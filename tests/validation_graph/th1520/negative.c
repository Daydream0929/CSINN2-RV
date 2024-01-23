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

int main(int argc, char **argv)
{
    init_testsuite("Testing function of negative(graph).\n");

    int *buffer = read_input_data_f32(argv[1]);

    struct csinn_tensor *reference = csinn_alloc_tensor(NULL);
    int in_size = 1, out_size = 1;
    enum csinn_dtype_enum test_dtype = CSINN_TEST_DTYPE;
    /* session configuration */
    struct csinn_session *sess = csinn_alloc_session();
    sess->base_api = CSINN_API;
    csinn_session_init(sess);
    csinn_set_input_number(1, sess);
    csinn_set_output_number(1, sess);

    /* input tensor configuration */
    struct csinn_tensor *input = csinn_alloc_tensor(sess);
    input->dim_count = buffer[0];
    for (int i = 0; i < input->dim_count; i++) {
        input->dim[i] = buffer[1 + i];
        in_size *= input->dim[i];
    }
    input->name = "input";
    float *input_data = (float *)(buffer + 1 + input->dim_count);
    input->data = input_data;
    get_quant_info(input);
    input->dtype = CSINN_DTYPE_FLOAT32;

    /* output tensor configuration */
    struct csinn_tensor *output = csinn_alloc_tensor(sess);
    output->dim_count = input->dim_count;
    for (int i = 0; i < output->dim_count; i++) {
        output->dim[i] = input->dim[i];
        out_size *= output->dim[i];
    }
    reference->data = (float *)(buffer + 1 + input->dim_count + in_size);
    output->data = reference->data;
    output->name = "output";
    get_quant_info(output);

    /* operator parameter configuration */
    struct csinn_siso_params *params = csinn_alloc_params(sizeof(struct csinn_siso_params), NULL);
    params->base.api = CSINN_API;
    params->base.name = "params";
    params->base.layout = CSINN_LAYOUT_NCHW;

    struct csinn_tensor *input_tensor = convert_input(input, test_dtype);
    input->dtype = sess->base_dtype;
    /* th1520: unsupport negative now*/
    if (csinn_negative_init(input, output, params) != CSINN_TRUE) {
        printf("negative init fail.\n\t");
        return -1;
    }

    csinn_set_tensor_entry(input, sess);
    csinn_set_input(0, input, sess);

    csinn_negative(input, output, params);

    csinn_set_output(0, output, sess);
    csinn_session_setup(sess);

    csinn_update_input(0, input_tensor, sess);
    csinn_session_run(sess);

    struct csinn_tensor *output_tensor = csinn_alloc_tensor(NULL);
    output_tensor->data = NULL;
    output_tensor->dtype = sess->base_dtype;
    output_tensor->is_const = 0;
    int output_num = csinn_get_output_number(sess);
    printf("output_num = %d\n", output_num);
    csinn_get_output(0, output_tensor, sess);
    memcpy(output_tensor->qinfo, output->qinfo, sizeof(struct csinn_quant_info));

    /* verify result */
    float difference = argc > 2 ? atof(argv[2]) : 1e-4;
    if (sess->base_dtype == CSINN_DTYPE_UINT8 || sess->base_dtype == CSINN_DTYPE_INT8) {
        result_verify_8(reference->data, output_tensor, input->data, difference, out_size, false);
    } else if (sess->base_dtype == CSINN_DTYPE_FLOAT32 &&
               output_tensor->dtype == CSINN_DTYPE_INT8) {
        struct csinn_tensor *foutput = shl_ref_tensor_transform_f32(output_tensor);
        result_verify_f32(reference->data, foutput->data, input->data, difference, out_size, false);
    }

    /* free alloced memory */
    free(buffer);
    free(input_tensor->qinfo);
    free(input_tensor);
    free(output_tensor->qinfo);
    free(output_tensor);
    free(reference->qinfo);
    free(reference);

    csinn_session_deinit(sess);
    csinn_free_session(sess);
    return done_testing();
}
