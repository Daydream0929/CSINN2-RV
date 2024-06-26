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
    init_testsuite("Testing function of relu6(layer).\n");

    struct csinn_session *sess = csinn_alloc_session();
    sess->base_run_mode = CSINN_RM_CPU_GRAPH;
    sess->model.save_mode = CSINN_RUN_ONLY;
    sess->dynamic_shape = CSINN_FALSE;
    struct csinn_tensor *input = csinn_alloc_tensor(sess);
    struct csinn_tensor *output = csinn_alloc_tensor(sess);
    struct csinn_tensor *reference = csinn_alloc_tensor(sess);
    struct csinn_relu_params *params =
        (csinn_relu_params *)csinn_alloc_params(sizeof(struct csinn_relu_params), sess);
    int in_size = 1, out_size = 1;

    int *buffer = read_input_data_f32(argv[1]);
    input->dim_count = buffer[0];
    output->dim_count = input->dim_count;
    for (int i = 0; i < input->dim_count; i++) {
        input->dim[i] = buffer[i + 1];
        output->dim[i] = input->dim[i];
        in_size *= input->dim[i];
    }

    input->dtype = CSINN_DTYPE_FLOAT32;
    input->layout = CSINN_LAYOUT_NCHW;
    input->is_const = 0;
    input->quant_channel = 1;
    set_layout(input);

    output->dtype = CSINN_DTYPE_FLOAT32;
    output->layout = CSINN_LAYOUT_NCHW;
    output->is_const = 0;
    output->quant_channel = 1;
    set_layout(output);
    params->base.api = CSINN_API;

    input->data = (float *)(buffer + 1 + input->dim_count);
    reference->data = (float *)(buffer + 1 + input->dim_count + in_size);
    output->data = reference->data;
    float difference = argc > 2 ? atof(argv[2]) : 0.99;

#if (DTYPE == 32)
    test_unary_op(input, output, params, CSINN_DTYPE_FLOAT32, CSINN_QUANT_FLOAT32, sess,
                  csinn_relu6_init, csinn_relu6, &difference);
#elif (DTYPE == 16)
    test_unary_op(input, output, params, CSINN_DTYPE_FLOAT16, CSINN_QUANT_FLOAT16, sess,
                  csinn_relu6_init, csinn_relu6, &difference);
#elif (DTYPE == 8)
    test_unary_op(input, output, params, CSINN_DTYPE_INT8, CSINN_QUANT_INT8_ASYM, sess,
                  csinn_relu6_init, csinn_relu6, &difference);
#endif
    return done_testing();
}
