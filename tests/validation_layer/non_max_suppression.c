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

#include "testutil.h"

int main(int argc, char **argv)
{
    init_testsuite("Testing function of non_max_suppression(layer).\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_run_mode = CSINN_RM_LAYER;
    struct csinn_tensor *input1 = csinn_alloc_tensor(sess);
    struct csinn_tensor *input0 = csinn_alloc_tensor(sess);
    struct csinn_tensor *output = csinn_alloc_tensor(sess);
    struct csinn_tensor *reference = csinn_alloc_tensor(sess);
    struct csinn_non_max_suppression_params *params =
        csinn_alloc_params(sizeof(struct csinn_non_max_suppression_params), sess);
    int in_size = 1, out_size = 1;

    int *buffer = read_input_data_f32(argv[1]);
    input0->dim_count = 2;
    input1->dim_count = 1;
    input0->dim[0] = buffer[0];
    input0->dim[1] = 4;
    input1->dim[0] = buffer[0];

    params->max_output_size = buffer[1];
    params->iou_threshold = *((float *)buffer + 3);

    output->dim_count = 2;
    output->dim[0] = params->max_output_size;
    output->dim[1] = 4;

    in_size = input0->dim[0] * 4;
    out_size = buffer[2];

    input0->dtype = CSINN_DTYPE_FLOAT32;
    input0->layout = CSINN_LAYOUT_NCHW;
    input0->is_const = 0;
    input0->quant_channel = 1;
    input1->dtype = CSINN_DTYPE_FLOAT32;
    input1->layout = CSINN_LAYOUT_NCHW;
    input1->is_const = 0;
    input1->quant_channel = 1;
    output->dtype = CSINN_DTYPE_FLOAT32;
    output->layout = CSINN_LAYOUT_NCHW;
    output->is_const = 0;
    output->quant_channel = 1;
    params->base.api = CSINN_API;

    input0->data = (float *)(buffer + 4);
    input1->data = (float *)(buffer + 4 + in_size);
    reference->data = (int *)(buffer + 4 + in_size + in_size / 4);
    output->data = reference->data;
    float difference = argc > 2 ? atof(argv[2]) : 0.99;

    test_non_max_suppression_CSINN_QUANT_FLOAT32(input0, input1, output, params, &difference);
    test_non_max_suppression_CSINN_QUANT_UINT8_ASYM(input0, input1, output, params, &difference);
    test_non_max_suppression_CSINN_QUANT_INT8_SYM(input0, input1, output, params, &difference);

    return done_testing();
}
