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

#include "testutil.h"

int main(int argc, char **argv)
{
    init_testsuite("Testing function of fullyconnected(layer).\n");

    struct csinn_session *sess = csinn_alloc_session();
    sess->base_run_mode = CSINN_RM_CPU_GRAPH;
    sess->model.save_mode = CSINN_RUN_ONLY;
    sess->dynamic_shape = CSINN_FALSE;
    struct csinn_tensor *input = csinn_alloc_tensor(sess);
    struct csinn_tensor *output = csinn_alloc_tensor(sess);
    struct csinn_tensor *reference = csinn_alloc_tensor(sess);
    struct csinn_tensor *weight = csinn_alloc_tensor(sess);
    struct csinn_tensor *bias = csinn_alloc_tensor(sess);
    struct csinn_fc_params *params =
        (csinn_fc_params *)csinn_alloc_params(sizeof(struct csinn_fc_params), sess);
    int in_size0, in_size1, out_size;

    int *buffer = read_input_data_f32(argv[1]);
    input->dim[0] = buffer[0];   // batch
    input->dim[1] = buffer[1];   // in_size
    weight->dim[0] = buffer[2];  // out_size
    weight->dim[1] = buffer[1];  // in_size
    bias->dim[0] = buffer[2];
    output->dim[0] = buffer[0];
    output->dim[1] = buffer[2];
    input->dim_count = 2;
    weight->dim_count = 2;
    bias->dim_count = 1;
    output->dim_count = 2;
    in_size0 = input->dim[0] * input->dim[1];
    in_size1 = weight->dim[0] * weight->dim[1];
    out_size = output->dim[0] * output->dim[1];
    input->dtype = CSINN_DTYPE_FLOAT32;
    input->layout = CSINN_LAYOUT_NC;
    input->is_const = 0;
    input->quant_channel = 1;
    weight->dtype = CSINN_DTYPE_FLOAT32;
    weight->layout = CSINN_LAYOUT_OI;
    weight->is_const = 1;
    weight->quant_channel = 1;

    bias->dtype = CSINN_DTYPE_FLOAT32;
    bias->layout = CSINN_LAYOUT_O;
    bias->is_const = 1;
    bias->quant_channel = 1;

    output->dtype = CSINN_DTYPE_FLOAT32;
    output->layout = CSINN_LAYOUT_NC;
    output->is_const = 0;
    output->quant_channel = 1;
    params->base.api = CSINN_API;

    input->data = (float *)(buffer + 3);
    weight->data = (float *)(buffer + 3 + in_size0);
    bias->data = (float *)(buffer + 3 + in_size0 + in_size1);
    reference->data = (float *)(buffer + 3 + in_size0 + in_size1 + buffer[2]);
    output->data = reference->data;
    float difference = argc > 2 ? atof(argv[2]) : 0.99;

#if (DTYPE == 32)
    test_fully_op(input, output, weight, bias, params, CSINN_DTYPE_FLOAT32, CSINN_QUANT_FLOAT32,
                  sess, csinn_fullyconnected_init, csinn_fullyconnected, &difference);
#elif (DTYPE == 16)
    test_fully_op(input, output, weight, bias, params, CSINN_DTYPE_FLOAT16, CSINN_QUANT_FLOAT16,
                  sess, csinn_fullyconnected_init, csinn_fullyconnected, &difference);
#elif (DTYPE == 8)
    test_fully_op(input, output, weight, bias, params, CSINN_DTYPE_INT8,
                  CSINN_QUANT_INT8_ASYM_W_SYM, sess, csinn_fullyconnected_init,
                  csinn_fullyconnected, &difference);
#elif (DTYPE == 168)
    test_fully_op(input, output, weight, bias, params, CSINN_DTYPE_FLOAT16,
                  CSINN_QUANT_FLOAT16_W_INT8, sess, csinn_fullyconnected_init, csinn_fullyconnected,
                  &difference);
#elif (DTYPE == 0x168C)
    csinn_realloc_quant_info(weight, weight->dim[0]);
    test_fully_op(input, output, weight, bias, params, CSINN_DTYPE_FLOAT16,
                  CSINN_QUANT_FLOAT16_W_INT8, sess, csinn_fullyconnected_init, csinn_fullyconnected,
                  &difference);
#endif

    return done_testing();
}
