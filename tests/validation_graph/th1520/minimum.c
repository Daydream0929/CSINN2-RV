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

#include "csi_nn.h"
#include "test_utils.h"

void op_test_run(struct csinn_tensor *input0, struct csinn_tensor *input1,
                 struct csinn_tensor *output, struct csinn_diso_params *params,
                 struct csinn_session *sess, struct csinn_tensor *real_input0,
                 struct csinn_tensor *real_input1, float *output_data, float diff);

void test_i8_sym(struct csinn_tensor *input0, struct csinn_tensor *input1,
                 struct csinn_tensor *output, struct csinn_diso_params *params, float difference)
{
    printf("test minimum i8 sym\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_TH1520;
    sess->base_quant_type = CSINN_QUANT_INT8_SYM;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_FLOAT32;

    struct csinn_tensor *qinput0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *qinput1 = convert_f32_input(input1, test_dtype, sess);
    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);
    struct csinn_tensor *real_input0 = convert_f32_input(input0, CSINN_DTYPE_INT8, sess);
    struct csinn_tensor *real_input1 = convert_f32_input(input1, CSINN_DTYPE_INT8, sess);
    op_test_run(qinput0, qinput1, qoutput, params, sess, real_input0, real_input1, output->data,
                difference);
}

void test_i16_sym(struct csinn_tensor *input0, struct csinn_tensor *input1,
                  struct csinn_tensor *output, struct csinn_diso_params *params, float difference)
{
    printf("test minimum i16 sym\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_TH1520;
    sess->base_quant_type = CSINN_QUANT_INT16_SYM;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_FLOAT32;

    struct csinn_tensor *qinput0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *qinput1 = convert_f32_input(input1, test_dtype, sess);
    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);
    struct csinn_tensor *real_input0 = convert_f32_input(input0, CSINN_DTYPE_INT16, sess);
    struct csinn_tensor *real_input1 = convert_f32_input(input1, CSINN_DTYPE_INT16, sess);
    op_test_run(qinput0, qinput1, qoutput, params, sess, real_input0, real_input1, output->data,
                difference);
}

void test_i8_asym(struct csinn_tensor *input0, struct csinn_tensor *input1,
                  struct csinn_tensor *output, struct csinn_diso_params *params, float difference)
{
    printf("test minimum i8 asym\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_TH1520;
    sess->base_quant_type = CSINN_QUANT_INT8_ASYM;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_INT8;

    struct csinn_tensor *qinput0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *qinput1 = convert_f32_input(input1, test_dtype, sess);
    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);
    struct csinn_tensor *real_input0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *real_input1 = convert_f32_input(input1, test_dtype, sess);
    op_test_run(qinput0, qinput1, qoutput, params, sess, real_input0, real_input1, output->data,
                difference);
}

void test_u8_asym(struct csinn_tensor *input0, struct csinn_tensor *input1,
                  struct csinn_tensor *output, struct csinn_diso_params *params, float difference)
{
    printf("test minimum u8 asym\n");
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_TH1520;
    sess->base_quant_type = CSINN_QUANT_UINT8_ASYM;
    // sess->debug_level = CSINN_DEBUG_LEVEL_INFO;
    enum csinn_dtype_enum test_dtype = CSINN_DTYPE_UINT8;

    struct csinn_tensor *qinput0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *qinput1 = convert_f32_input(input1, test_dtype, sess);
    struct csinn_tensor *qoutput = convert_f32_input(output, test_dtype, sess);
    struct csinn_tensor *real_input0 = convert_f32_input(input0, test_dtype, sess);
    struct csinn_tensor *real_input1 = convert_f32_input(input1, test_dtype, sess);
    op_test_run(qinput0, qinput1, qoutput, params, sess, real_input0, real_input1, output->data,
                difference);
}

void test_minimum(struct csinn_tensor *input0, struct csinn_tensor *input1,
                  struct csinn_tensor *output, struct csinn_diso_params *params, float difference)
{
    params->base.api = CSINN_TH1520;

    test_i8_sym(input0, input1, output, params, difference);
    test_i16_sym(input0, input1, output, params, difference);
    /* unsupport yet */
    // test_i8_asym(input0, input1, output, params, difference);
    // test_u8_asym(input0, input1, output, params, difference);
}
