/*
 * Copyright (C) 2016-2023 C-SKY Microsystems Co., Ltd. All rights reserved.
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

#include "rvv/cap.h"
#include "rvv/perf.h"
#include "rvv/rvv.h"

#define RVV_OP_PATTERN_MAX 120
static struct shl_cb_table shl_rvv_cb_table[RVV_OP_PATTERN_MAX];

void shl_rvv_reg_op(enum csinn_dtype_enum dtype, enum csinn_op_enum op_name, void *init, void *exec,
                    void *est, void *cap, void *perf)
{
    static int i = 0;
    if (i >= RVV_OP_PATTERN_MAX) {
        shl_debug_error("RVV callback length is greater than RVV_OP_PATTERN_MAX!\n");
    }
    shl_rvv_cb_table[i].shl_cb_key = op_name * CSINN_DTYPE_SIZE + dtype;
    shl_rvv_cb_table[i].shl_cb_value.init = init;
    shl_rvv_cb_table[i].shl_cb_value.exec = exec;
    shl_rvv_cb_table[i].shl_cb_value.est = est;
    shl_rvv_cb_table[i].shl_cb_value.caps = cap;
    shl_rvv_cb_table[i].shl_cb_value.perf = perf;
    i++;
}

struct csinn_callback *shl_cb_map_ref(int op, int dtype);
struct csinn_callback *shl_cb_map_rvv(int op, int dtype)
{
    struct csinn_callback *cb = NULL;
    for (int i = 0; i < RVV_OP_PATTERN_MAX; i++) {
        if (shl_rvv_cb_table[i].shl_cb_key == (op * CSINN_DTYPE_SIZE + dtype)) {
            cb = &(shl_rvv_cb_table[i].shl_cb_value);
            break;
        }
    }
    if ((cb == NULL) || (cb->est == NULL && (cb->init == NULL || cb->exec == NULL))) {
        cb = shl_cb_map_ref(op, dtype);
    }
    return cb;
}

struct shl_rvv_option *shl_rvv_get_graph_option(struct csinn_session *sess)
{
    struct shl_gref_target_data *gref_td = sess->td;
    if (gref_td) {
        return (struct shl_rvv_option *)(gref_td->cpu_option);
    } else {
        return NULL;
    }
}

void __attribute__((weak)) shl_target_init_rvv()
{
#ifndef CONFIG_THEAD_RVV_CONVOLUTION_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_CONV2D, shl_rvv_conv2d_init_fp32, NULL,
                   shl_gref_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_GROUP_CONV2D, shl_rvv_conv2d_init_fp32, NULL,
                   shl_gref_group_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONVOLUTION_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_CONV2D, shl_rvv_conv2d_init_fp16, NULL,
                   shl_gref_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_GROUP_CONV2D, shl_rvv_conv2d_init_fp16, NULL,
                   shl_gref_group_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DEPTHWISE_CONVOLUTION_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_DEPTHWISE_CONV2D,
                   shl_rvv_depthwise_conv2d_init_fp32, NULL, shl_gref_depthwise_conv2d,
                   shl_rvv_depthwise_conv2d_cap, shl_rvv_depthwise_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DEPTHWISE_CONVOLUTION_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_DEPTHWISE_CONV2D,
                   shl_rvv_depthwise_conv2d_init_fp16, NULL, shl_gref_depthwise_conv2d,
                   shl_rvv_depthwise_conv2d_cap, shl_rvv_depthwise_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DEPTHWISE_CONVOLUTION_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_DEPTHWISE_CONV2D, shl_rvv_depthwise_conv2d_init_int8,
                   NULL, shl_gref_depthwise_conv2d, shl_rvv_depthwise_conv2d_cap,
                   shl_rvv_depthwise_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_DEPTHWISE_CONV2D_RELU,
                   shl_rvv_depthwise_conv2d_init_int8, NULL, shl_gref_depthwise_conv2d_relu,
                   shl_rvv_depthwise_conv2d_cap, shl_rvv_depthwise_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_DEPTHWISE_CONV2D_RELU6,
                   shl_rvv_depthwise_conv2d_init_int8, NULL, shl_gref_depthwise_conv2d_relu6,
                   shl_rvv_depthwise_conv2d_cap, shl_rvv_depthwise_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DECONVOLUTION_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_DECONV2D, shl_rvv_deconv2d_init_fp32, NULL,
                   shl_gref_deconv2d, shl_rvv_deconv2d_cap, shl_rvv_deconv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_GROUP_DECONV2D, shl_rvv_deconv2d_init_fp32, NULL,
                   shl_gref_group_deconv2d, shl_rvv_deconv2d_cap, shl_rvv_deconv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DECONVOLUTION_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_DECONV2D, shl_rvv_deconv2d_init_fp16, NULL,
                   shl_gref_deconv2d, shl_rvv_deconv2d_cap, shl_rvv_deconv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_GROUP_DECONV2D, shl_rvv_deconv2d_init_fp16, NULL,
                   shl_gref_group_deconv2d, shl_rvv_deconv2d_cap, shl_rvv_deconv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MAXPOOL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_MAXPOOL2D, shl_rvv_maxpool2d_init_fp32, NULL,
                   shl_gref_maxpool2d, shl_rvv_maxpool2d_cap, shl_rvv_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MAXPOOL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_MAXPOOL2D, shl_rvv_maxpool2d_init_fp16, NULL,
                   shl_gref_maxpool2d, shl_rvv_maxpool2d_cap, shl_rvv_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MAXPOOL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_MAXPOOL2D, shl_rvv_maxpool2d_init_int8, NULL,
                   shl_gref_maxpool2d, shl_rvv_maxpool2d_cap, shl_rvv_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_AVERAGEPOOL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_AVGPOOL2D, shl_rvv_avgpool2d_init_fp32, NULL,
                   shl_gref_avgpool2d, shl_rvv_avgpool2d_cap, shl_rvv_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_AVERAGEPOOL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_AVGPOOL2D, shl_rvv_avgpool2d_init_fp16, NULL,
                   shl_gref_avgpool2d, shl_rvv_avgpool2d_cap, shl_rvv_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_AVERAGEPOOL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_AVGPOOL2D, shl_rvv_avgpool2d_init_int8, NULL,
                   shl_gref_avgpool2d, shl_rvv_avgpool2d_cap, shl_rvv_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_FULLYCONNECTED_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_FULLYCONNECTED, shl_rvv_fullyconnected_init_fp32,
                   NULL, shl_gref_fullyconnected, shl_rvv_fullyconnected_cap,
                   shl_rvv_fullyconnected_perf);
#endif
#ifndef CONFIG_THEAD_RVV_FULLYCONNECTED_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_FULLYCONNECTED, shl_rvv_fullyconnected_init_fp16,
                   NULL, shl_gref_fullyconnected, shl_rvv_fullyconnected_cap,
                   shl_rvv_fullyconnected_perf);
#endif
#ifndef CONFIG_THEAD_RVV_FULLYCONNECTED_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_FULLYCONNECTED, shl_rvv_fullyconnected_init_int8,
                   NULL, shl_gref_fullyconnected, shl_rvv_fullyconnected_cap,
                   shl_rvv_fullyconnected_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ADD_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_ADD, NULL, shl_rvv_add_fp32, shl_gref_add,
                   shl_rvv_add_cap, shl_rvv_add_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ADD_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_ADD, NULL, shl_rvv_add_fp16, shl_gref_add,
                   shl_rvv_add_cap, shl_rvv_add_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ADD_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_ADD, NULL, shl_rvv_add_int8, shl_gref_add,
                   shl_rvv_add_cap, shl_rvv_add_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SUB_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SUB, NULL, shl_rvv_sub_fp32, shl_gref_sub,
                   shl_rvv_sub_cap, shl_rvv_sub_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SUB_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SUB, NULL, shl_rvv_sub_fp16, shl_gref_sub,
                   shl_rvv_sub_cap, shl_rvv_sub_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SUB_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_SUB, NULL, shl_rvv_sub_int8, shl_gref_sub,
                   shl_rvv_sub_cap, shl_rvv_sub_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MUL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_MUL, NULL, shl_rvv_mul_fp32, shl_gref_mul,
                   shl_rvv_mul_cap, shl_rvv_mul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MUL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_MUL, NULL, shl_rvv_mul_fp16, shl_gref_mul,
                   shl_rvv_mul_cap, shl_rvv_mul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MUL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_MUL, NULL, shl_rvv_mul_int8, shl_gref_mul,
                   shl_rvv_mul_cap, shl_rvv_mul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DIV_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_DIV, NULL, shl_rvv_div_fp32, shl_gref_div,
                   shl_rvv_div_cap, shl_rvv_div_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DIV_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_DIV, NULL, shl_rvv_div_fp16, shl_gref_div,
                   shl_rvv_div_cap, shl_rvv_div_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DIV_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_DIV, NULL, shl_rvv_div_int8, shl_gref_div,
                   shl_rvv_div_cap, shl_rvv_div_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONCAT_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_CONCAT, NULL, shl_rvv_concat_fp32, shl_gref_concat,
                   shl_rvv_concat_cap, shl_rvv_concat_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONCAT_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_CONCAT, NULL, shl_rvv_concat_fp16, shl_gref_concat,
                   shl_rvv_concat_cap, shl_rvv_concat_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONCAT_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CONCAT, NULL, shl_rvv_concat_int8, shl_gref_concat,
                   shl_rvv_concat_cap, shl_rvv_concat_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LEAKY_RELU_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_LEAKY_RELU, NULL, shl_rvv_leaky_relu_fp32,
                   shl_gref_leaky_relu, shl_rvv_leaky_relu_cap, shl_rvv_leaky_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LEAKY_RELU_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_LEAKY_RELU, NULL, shl_rvv_leaky_relu_fp16,
                   shl_gref_leaky_relu, shl_rvv_leaky_relu_cap, shl_rvv_leaky_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LEAKY_RELU_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_LEAKY_RELU, NULL, shl_rvv_leaky_relu_int8,
                   shl_gref_leaky_relu, shl_rvv_leaky_relu_cap, shl_rvv_leaky_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_RELU, NULL, shl_rvv_relu_fp32, shl_gref_relu,
                   shl_rvv_relu_cap, shl_rvv_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_RELU, NULL, shl_rvv_relu_fp16, shl_gref_relu,
                   shl_rvv_relu_cap, shl_rvv_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_RELU, NULL, shl_rvv_relu_int8, shl_gref_relu,
                   shl_rvv_relu_cap, shl_rvv_relu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU6_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_RELU6, NULL, shl_rvv_relu6_fp32, shl_gref_relu6,
                   shl_rvv_relu6_cap, shl_rvv_relu6_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU6_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_RELU6, NULL, shl_rvv_relu6_fp16, shl_gref_relu6,
                   shl_rvv_relu6_cap, shl_rvv_relu6_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RELU6_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_RELU6, NULL, shl_rvv_relu6_int8, shl_gref_relu6,
                   shl_rvv_relu6_cap, shl_rvv_relu6_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_AVERAGEPOOL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_GLOBAL_AVGPOOL2D,
                   shl_rvv_global_avgpool2d_init_fp32, NULL, shl_gref_global_avgpool2d,
                   shl_rvv_global_avgpool2d_cap, shl_rvv_global_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_AVERAGEPOOL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_GLOBAL_AVGPOOL2D,
                   shl_rvv_global_avgpool2d_init_fp16, NULL, shl_gref_global_avgpool2d,
                   shl_rvv_global_avgpool2d_cap, shl_rvv_global_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_AVERAGEPOOL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_GLOBAL_AVGPOOL2D, shl_rvv_global_avgpool2d_init_int8,
                   NULL, shl_gref_global_avgpool2d, shl_rvv_global_avgpool2d_cap,
                   shl_rvv_global_avgpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_MAXPOOL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_GLOBAL_MAXPOOL2D,
                   shl_rvv_global_maxpool2d_init_fp32, NULL, shl_gref_global_maxpool2d,
                   shl_rvv_global_maxpool2d_cap, shl_rvv_global_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_MAXPOOL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_GLOBAL_MAXPOOL2D,
                   shl_rvv_global_maxpool2d_init_fp16, NULL, shl_gref_global_maxpool2d,
                   shl_rvv_global_maxpool2d_cap, shl_rvv_global_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GLOBAL_MAXPOOL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_GLOBAL_MAXPOOL2D, shl_rvv_global_maxpool2d_init_int8,
                   NULL, shl_gref_global_maxpool2d, shl_rvv_global_maxpool2d_cap,
                   shl_rvv_global_maxpool2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RESHAPE_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_RESHAPE, NULL, shl_rvv_reshape_fp32,
                   shl_gref_reshape, shl_rvv_reshape_cap, shl_rvv_reshape_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RESHAPE_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_RESHAPE, NULL, shl_rvv_reshape_fp16,
                   shl_gref_reshape, shl_rvv_reshape_cap, shl_rvv_reshape_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RESHAPE_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_RESHAPE, NULL, shl_rvv_reshape_int8, shl_gref_reshape,
                   shl_rvv_reshape_cap, shl_rvv_reshape_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SIGMOID_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SIGMOID, NULL, shl_rvv_sigmoid_fp32,
                   shl_gref_sigmoid, shl_rvv_sigmoid_cap, shl_rvv_sigmoid_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SIGMOID_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SIGMOID, NULL, shl_rvv_sigmoid_fp16,
                   shl_gref_sigmoid, shl_rvv_sigmoid_cap, shl_rvv_sigmoid_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SIGMOID_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_SIGMOID, NULL, shl_rvv_sigmoid_int8, shl_gref_sigmoid,
                   shl_rvv_sigmoid_cap, shl_rvv_sigmoid_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SOFTMAX_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SOFTMAX, NULL, shl_rvv_softmax_fp32,
                   shl_gref_softmax, shl_rvv_softmax_cap, shl_rvv_softmax_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SOFTMAX_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SOFTMAX, NULL, shl_rvv_softmax_fp16,
                   shl_gref_softmax, shl_rvv_softmax_cap, shl_rvv_softmax_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SOFTMAX_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_SOFTMAX, NULL, shl_rvv_softmax_int8, shl_gref_softmax,
                   shl_rvv_softmax_cap, shl_rvv_softmax_perf);
#endif
#ifndef CONFIG_THEAD_RVV_REDUCE_SUM_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_REDUCE_SUM, NULL, shl_rvv_reduce_sum_int8,
                   shl_gref_reduce_sum, shl_rvv_reduce_sum_cap, shl_rvv_reduce_sum_perf);
#endif
#ifndef CONFIG_THEAD_RVV_PRELU_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_PRELU, NULL, shl_rvv_prelu_fp32, shl_gref_prelu,
                   shl_rvv_prelu_cap, shl_rvv_prelu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_PRELU_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_PRELU, NULL, shl_rvv_prelu_fp16, shl_gref_prelu,
                   shl_rvv_prelu_cap, shl_rvv_prelu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_PRELU_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_PRELU, NULL, shl_rvv_prelu_int8, shl_gref_prelu,
                   shl_rvv_prelu_cap, shl_rvv_prelu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LAYER_NORM_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_LAYER_NORM, NULL, shl_rvv_layer_norm_fp32,
                   shl_gref_layer_norm, shl_rvv_layer_norm_cap, shl_rvv_layer_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LAYER_NORM_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_LAYER_NORM, NULL, shl_rvv_layer_norm_fp16,
                   shl_gref_layer_norm, shl_rvv_layer_norm_cap, shl_rvv_layer_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LAYER_NORM_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_LAYER_NORM, NULL, shl_rvv_layer_norm_int8,
                   shl_gref_layer_norm, shl_rvv_layer_norm_cap, shl_rvv_layer_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CLIP_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_CLIP, NULL, shl_rvv_clip_fp32, shl_gref_clip,
                   shl_rvv_clip_cap, shl_rvv_clip_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CLIP_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_CLIP, NULL, shl_rvv_clip_fp16, shl_gref_clip,
                   shl_rvv_clip_cap, shl_rvv_clip_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CLIP_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CLIP, NULL, shl_rvv_clip_int8, shl_gref_clip,
                   shl_rvv_clip_cap, shl_rvv_clip_perf);
#endif

#ifndef CONFIG_THEAD_RVV_CONVOLUTION1D_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_CONV1D, shl_rvv_conv1d_init_fp32, NULL,
                   shl_gref_conv1d, shl_rvv_conv1d_cap, shl_rvv_conv1d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONVOLUTION1D_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_CONV1D, shl_rvv_conv1d_init_fp16, NULL,
                   shl_gref_conv1d, shl_rvv_conv1d_cap, shl_rvv_conv1d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONVOLUTION1D_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CONV1D, shl_rvv_conv1d_init_int8, NULL,
                   shl_gref_conv1d, shl_rvv_conv1d_cap, shl_rvv_conv1d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DEPTHWISE_CONVOLUTION1D_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_DEPTHWISE_CONV1D, shl_rvv_conv1d_init_int8, NULL,
                   shl_gref_depthwise_conv1d, shl_rvv_conv1d_cap, shl_rvv_conv1d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_CONVOLUTION_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CONV2D, shl_rvv_conv2d_init_int8, NULL,
                   shl_gref_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_GROUP_CONV2D, shl_rvv_conv2d_init_int8, NULL,
                   shl_gref_group_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CONV2D_RELU, shl_rvv_conv2d_init_int8, NULL,
                   shl_gref_conv2d_relu, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_GROUP_CONV2D_RELU, shl_rvv_conv2d_init_int8, NULL,
                   shl_gref_group_conv2d_relu, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_CONV2D_RELU6, shl_rvv_conv2d_init_int8, NULL,
                   shl_gref_conv2d_relu6, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_TRANSPOSE_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_TRANSPOSE, NULL, shl_rvv_transpose_fp32,
                   shl_gref_transpose, shl_rvv_transpose_cap, shl_rvv_transpose_perf);
#endif
#ifndef CONFIG_THEAD_RVV_TRANSPOSE_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_TRANSPOSE, NULL, shl_rvv_transpose_fp16,
                   shl_gref_transpose, shl_rvv_transpose_cap, shl_rvv_transpose_perf);
#endif
#ifndef CONFIG_THEAD_RVV_TRANSPOSE_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_TRANSPOSE, NULL, shl_rvv_transpose_int8,
                   shl_gref_transpose, shl_rvv_transpose_cap, shl_rvv_transpose_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MATMUL_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_MATMUL, shl_rvv_matmul_init_fp32, NULL,
                   shl_gref_matmul, shl_rvv_matmul_cap, shl_rvv_matmul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MATMUL_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_MATMUL, shl_rvv_matmul_init_fp16, NULL,
                   shl_gref_matmul, shl_rvv_matmul_cap, shl_rvv_matmul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_MATMUL_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_MATMUL, shl_rvv_matmul_init_int8, NULL,
                   shl_gref_matmul, shl_rvv_matmul_cap, shl_rvv_matmul_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GATHER_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_GATHER, NULL, shl_rvv_gather_fp32, shl_gref_gather,
                   shl_rvv_gather_cap, shl_rvv_gather_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GATHER_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_GATHER, NULL, shl_rvv_gather_fp16, shl_gref_gather,
                   shl_rvv_gather_cap, shl_rvv_gather_perf);
#endif
#ifndef CONFIG_THEAD_RVV_GATHER_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_GATHER, NULL, shl_rvv_gather_int8, shl_gref_gather,
                   shl_rvv_gather_cap, shl_rvv_gather_perf);
#endif
#ifndef CONFIG_THEAD_RVV_STRIDED_SLICE_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_STRIDED_SLICE, NULL, shl_rvv_strided_slice_fp16,
                   shl_gref_strided_slice, shl_rvv_strided_slice_cap, shl_rvv_strided_slice_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ERF_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_ERF, NULL, shl_rvv_erf_fp32, shl_gref_erf,
                   shl_rvv_erf_cap, shl_rvv_erf_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ERF_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_ERF, NULL, shl_rvv_erf_fp16, shl_gref_erf,
                   shl_rvv_erf_cap, shl_rvv_erf_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ERF_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_ERF, NULL, shl_rvv_erf_int8, shl_gref_erf,
                   shl_rvv_erf_cap, shl_rvv_erf_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SPLIT_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SPLIT, NULL, shl_rvv_split_fp32, shl_gref_split,
                   shl_rvv_split_cap, shl_rvv_split_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SPLIT_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SPLIT, NULL, shl_rvv_split_fp16, shl_gref_split,
                   shl_rvv_split_cap, shl_rvv_split_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SPLIT_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_SPLIT, NULL, shl_rvv_split_int8, shl_gref_split,
                   shl_rvv_split_cap, shl_rvv_split_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SILU_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SILU, NULL, shl_rvv_silu_fp32, shl_gref_silu,
                   shl_rvv_silu_cap, shl_rvv_silu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SILU_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SILU, NULL, shl_rvv_silu_fp16, shl_gref_silu,
                   shl_rvv_silu_cap, shl_rvv_silu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SILU_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_SILU, NULL, shl_rvv_silu_int8, shl_gref_silu,
                   shl_rvv_silu_cap, shl_rvv_silu_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RMS_NORM_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_RMS_NORM, NULL, shl_rvv_rms_norm_fp32,
                   shl_gref_rms_norm, shl_rvv_rms_norm_cap, shl_rvv_rms_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RMS_NORM_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_RMS_NORM, NULL, shl_rvv_rms_norm_fp16,
                   shl_gref_rms_norm, shl_rvv_rms_norm_cap, shl_rvv_rms_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_RMS_NORM_INT8_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT8, CSINN_OP_RMS_NORM, NULL, shl_rvv_rms_norm_int8,
                   shl_gref_rms_norm, shl_rvv_rms_norm_cap, shl_rvv_rms_norm_perf);
#endif
#ifndef CONFIG_THEAD_RVV_EMBEDDING_INT32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT32, CSINN_OP_EMBEDDING, NULL, shl_rvv_embedding_int32,
                   shl_gref_embedding, shl_rvv_embedding_cap, shl_rvv_embedding_perf);
#endif
#ifndef CONFIG_THEAD_RVV_EXPAND_DIMS_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_EXPAND_DIMS, NULL, shl_rvv_expand_dims_fp32,
                   shl_gref_expand_dims, shl_rvv_expand_dims_cap, shl_rvv_expand_dims_perf);
#endif
#ifndef CONFIG_THEAD_RVV_EXPAND_DIMS_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_EXPAND_DIMS, NULL, shl_rvv_expand_dims_fp16,
                   shl_gref_expand_dims, shl_rvv_expand_dims_cap, shl_rvv_expand_dims_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ROPE_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_ROPE, NULL, shl_rvv_rope_fp32, shl_gref_rope,
                   shl_rvv_rope_cap, shl_rvv_rope_perf);
#endif
#ifndef CONFIG_THEAD_RVV_ROPE_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_ROPE, NULL, shl_rvv_rope_fp16, shl_gref_rope,
                   shl_rvv_rope_cap, shl_rvv_rope_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SCALED_DOT_PRODUCT_ATTENTION_FP32_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT32, CSINN_OP_SCALED_DOT_PRODUCT_ATTENTION, NULL,
                   shl_rvv_scaled_dot_product_attention_fp32, shl_gref_scaled_dot_product_attention,
                   shl_rvv_scaled_dot_product_attention_cap,
                   shl_rvv_scaled_dot_product_attention_perf);
#endif
#ifndef CONFIG_THEAD_RVV_SCALED_DOT_PRODUCT_ATTENTION_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_SCALED_DOT_PRODUCT_ATTENTION, NULL,
                   shl_rvv_scaled_dot_product_attention_fp16, shl_gref_scaled_dot_product_attention,
                   shl_rvv_scaled_dot_product_attention_cap,
                   shl_rvv_scaled_dot_product_attention_perf);
#endif
#ifndef CONFIG_THEAD_RVV_LLM_POS_FP16_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_FLOAT16, CSINN_OP_LLM_POS, NULL, shl_rvv_llm_pos_fp16,
                   shl_gref_llm_pos, shl_rvv_llm_pos_cap, shl_rvv_llm_pos_perf);
#endif

#ifdef SHL_USE_DOT_INT4
#ifndef CONFIG_THEAD_RVV_CONVOLUTION_INT4_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_CONV2D, shl_rvv_conv2d_init_int4, NULL,
                   shl_gref_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_GROUP_CONV2D, shl_rvv_conv2d_init_int4, NULL,
                   shl_gref_group_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_CONV2D_RELU, shl_rvv_conv2d_init_int4, NULL,
                   shl_gref_conv2d_relu, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_GROUP_CONV2D_RELU, shl_rvv_conv2d_init_int4, NULL,
                   shl_gref_group_conv2d_relu, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
#endif
#ifndef CONFIG_THEAD_RVV_DEPTHWISE_CONVOLUTION_INT4_DISABLED
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_DEPTHWISE_CONV2D, shl_rvv_depthwise_conv2d_init_int4,
                   NULL, shl_gref_depthwise_conv2d, shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
    shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_DEPTHWISE_CONV2D_RELU,
                   shl_rvv_depthwise_conv2d_init_int4, NULL, shl_gref_depthwise_conv2d_relu,
                   shl_rvv_conv2d_cap, shl_rvv_conv2d_perf);
#endif
    // shl_rvv_reg_op(CSINN_DTYPE_INT4, CSINN_OP_FULLYCONNECTED, shl_rvv_fullyconnected_init, NULL,
    //                shl_gref_fullyconnected);
#endif

    shl_register_runtime_callback(CSINN_RVV, NULL);
    shl_register_op_callback(CSINN_RVV, shl_cb_map_rvv);
    shl_register_runtime_callback(CSINN_RVV, shl_gref_runtime_callback);
}
