/*
 * Copyright (C) 2016-2021 C-SKY Limited. All rights reserved.
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

#include "test_utils.h"
#include "csi_nn.h"
#include "math_snr.h"
#include "csi_ovx.h"

int main(int argc, char** argv)
{
    init_testsuite("Testing function of equal (anole).\n");


    struct csi_session *sess = csi_alloc_session();
    sess->base_api = CSINN_ANOLE;
    sess->base_dtype = CSINN_DTYPE_UINT8;
    csi_session_init(sess);
    csi_set_input_number(2, sess);
    csi_set_output_number(1, sess);

    struct csi_tensor *input0 = csi_alloc_tensor(NULL);
    struct csi_tensor *input1 = csi_alloc_tensor(NULL);
    struct csi_tensor *output = csi_alloc_tensor(NULL);
    struct csi_tensor *reference = csi_alloc_tensor(NULL);
    struct diso_params params;
    int in_size = 1;
    int out_size = 1;
    int zp, quantized_multiplier, shift;
    float scale, min_value, max_value;
    float error[2] = {0};
    float max_error;

    int *buffer = read_input_data_f32(argv[1]);
    input0->dim_count = input1->dim_count = buffer[0];
    output->dim_count = input0->dim_count;

    for (int i = 0; i < input0->dim_count; i++ ) {
        input0->dim[i] = buffer[1 + i];
        input1->dim[i] = input0->dim[i];
        output->dim[i] = input0->dim[i];
        in_size = in_size * input0->dim[i];
    }

    out_size = in_size;

    input0->dtype = CSINN_DTYPE_UINT8;
    output->dtype = CSINN_DTYPE_UINT8;
    params.base.api = CSINN_API;
    params.base.name = "params";
    params.base.run_mode = CSINN_RM_NPU_GRAPH;


    uint8_t *input_tmp0 = malloc(in_size * sizeof(char));
    uint8_t *input_tmp1 = malloc(in_size * sizeof(char));
    float   *src_in0   = (float *)(buffer + 1 + input0->dim_count);
    float   *src_in1   = (float *)(buffer + 1 + input0->dim_count + in_size);
    float   *ref       = (float *)(buffer + 1 + input0->dim_count + 2*in_size);

    input0->qinfo = get_quant_info(src_in0, in_size);
    input0->name = "input0";

    for(int i = 0; i < in_size; i++) {
        input_tmp0[i] = csi_ref_quantize_f32_to_u8(src_in0[i], input0->qinfo);
    }

    input1->qinfo = get_quant_info(src_in1, in_size);
    input1->name = "input1";

    for(int i = 0; i < in_size; i++) {
        input_tmp1[i] = csi_ref_quantize_f32_to_u8(src_in1[i], input1->qinfo);
    }

    output->qinfo = get_quant_info(ref, out_size);
    output->name = "output";

    reference->data = ref;

    if (csi_equal_init(input0, input1, output, &params) != CSINN_TRUE) {
        printf("equal init fail.\n\t");
        return -1;
    }

    csi_ovx_set_tensor(input0, sess);
    csi_ovx_set_tensor(input1, sess);
    csi_set_input(0, input0, sess);
    csi_set_input(1, input1, sess);

    csi_equal(input0, input1, output, &params);

    csi_set_output(0, output, sess);
    csi_session_setup(sess);


    struct csi_tensor *input0_tensor = csi_alloc_tensor(NULL);
    struct csi_tensor *input1_tensor = csi_alloc_tensor(NULL);
    input0_tensor->data = input_tmp0;
    input1_tensor->data = input_tmp1;
    csi_update_input(0, input0_tensor, sess);
    csi_update_input(1, input1_tensor, sess);
    csi_session_run(sess);

    struct csi_tensor *output_tensor = csi_alloc_tensor(NULL);
    output_tensor->is_const = 0;
    int output_num = csi_get_output_number(sess);
    printf("output_num = %d\n", output_num);
    csi_get_output(0, output_tensor, sess);

    output_tensor->qinfo->multiplier = output->qinfo->multiplier; 
    output_tensor->qinfo->shift = output->qinfo->shift; 
    output_tensor->qinfo->zero_point = output->qinfo->zero_point;
    output_tensor->dtype == CSINN_DTYPE_UINT8;

    /* verify result */
    float difference = argc > 2 ? atof(argv[2]) : 1e-4;
    result_verify_8(reference->data, output_tensor, input0->data, difference, out_size, false);

    /* free alloced memory */
    free(buffer);
    free(input0_tensor->qinfo);
    free(input0_tensor);
    free(input1_tensor->qinfo);
    free(input1_tensor);
    free(output_tensor->qinfo);
    free(output_tensor);

    csi_session_deinit(sess);
    csi_free_session(sess);
    return done_testing();
}