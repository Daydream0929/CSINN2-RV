/*
 * Copyright (C) 2016-2023 T-Head Semiconductor Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "llm/shl_llm.h"
struct shl_llm_model *load_shl_model(void *base);

// calculate cosine similarity
static float compute_cs(float *a, float *b, uint32_t size)
{
    double dot_sum = 0.0;
    double a_norm = 0.0;
    double b_norm = 0.0;
    float res = 0.0;

    for (int i = 0; i < size; i++) {
        dot_sum += (a[i] * b[i]);
        a_norm += (a[i] * a[i]);
        b_norm += (b[i] * b[i]);
    }
    res = dot_sum / (sqrt(a_norm * b_norm));
    return res;
}

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
static void *shl_mmap(char *path)
{
    int fd = open(path, O_RDWR);
    struct stat sb;
    fstat(fd, &sb);
    void *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        printf("mmap error\n");
        return NULL;
    }
    return addr;
}

void main()
{
    struct llama_config *config = shl_mem_alloc(sizeof(struct llama_config));
    config->dim = 4096;
    config->multiple_of = 256;
    config->n_heads = 32;
    config->n_layers = 32;
    config->nor_eps = 1e-05;
    config->vocab_size = -1;
    char *path = "/mnt/ssd/zhangwm/git/Llama-2-7b/ggml-model-f16.shl.llm.bm";
    void *base_addr = shl_mmap(path);

    config->shl_model = load_shl_model(base_addr);

    struct shl_llm_ctx *ctx = llama2_build(config);

    /*
     * prompt: "Building a website can be done in 10 simple steps:\nStep 1:"
     */
    int32_t token[] = {1,     17166, 263,  4700,  508, 367,   2309,  297,   29871, 29896,
                       29900, 2560,  6576, 29901, 13,  14448, 29871, 29896, 29901};

    struct shl_llm_input *embd =
        (struct shl_llm_input *)shl_mem_alloc(sizeof(struct shl_llm_input));
    embd->n_tokens = 19;
    embd->token = token;
    embd->pos = (int32_t *)malloc(4 * 19);
    for (int i = 0; i < 19; i++) {
        embd->pos[i] = i;
    }

    llm_run(ctx, embd);

    // check prefill result
    float *result = (float *)ctx->output_session->output[0]->data;
    // last logits
    result += 32000 * 18;
    float reference_result[] = {
        -5.71030331,  -6.5068779,   4.49947596,   1.61511719,   2.1548543,     0.0926032066,
        2.82565427,   0.221694469,  -0.802444339, 0.397152185,  3.21004057,    2.24275088,
        10.127943,    7.96558952,   0.993502378,  -0.401111603, -5.71008682,   -1.67284417,
        -0.722307682, 1.00253534,   -0.748121202, -1.11147189,  -0.527304411,  -0.988370299,
        -1.5118947,   -1.75848043,  -0.597546458, -0.898284316, -1.02883792,   -0.916219473,
        0.592717409,  -0.389472723, -1.51692116,  -1.74400616,  -0.0866698027, -5.71152353,
        -5.71036053,  -5.70992756,  -5.7100029,   -5.71010208,  -5.71188021,   -5.710711,
    };
    float cs_ret = compute_cs(result, reference_result, 42);
    printf("first five: %f, %f, %f, %f, %f\n", result[0], result[1], result[2], result[3],
           result[4]);
    printf("result cos = %f\n", cs_ret);

    /* temperature = 0, greedy sampling */
    float prob[5];
    uint32_t index[5];
    shl_get_top5(result, 32000, prob, index);
    printf("Next id: %d = %f\n", index[0], prob[0]);

    /*
     * --temp 0 --repeat-penalty 1.0
     * next decode reference results:
     *  Cho  ose  a    domain name .
     * 14542,852,263, 5354,  1024, 29889, 13
     * Step        2     :      Cho  ose  a   hosting provider.
     * 14448,29871,29906,29901,14542,852,263,23376,  13113,   29889
     */
    for (int i = 19; i < 19 + 16; i++) {
        embd->n_tokens = 1;
        embd->token[0] = index[0];
        embd->pos[0] = i;
        llm_run(ctx, embd);
        result = (float *)ctx->output_session->output[0]->data;
        shl_get_top5(result, 32000, prob, index);
        printf("Next id: %d = %f\n", index[0], prob[0]);
    }
}
