// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <cstring>
#include "ppl/common/log.h"
#include "ppl/kernel/riscv/common/math.h"
#include "ppl/kernel/riscv/fp32/conv2d/wg/vec128/common/wg_offline.h"
#include "ppl/kernel/riscv/fp32/conv2d/wg/vec128/conv2d_n4cx_wg_b6f3_fp32.h"

namespace ppl { namespace kernel { namespace riscv {

inline void wb_b6f3s1_cvt_filter_blk_kernel(
    const float* filter,
    const float* trans_mat, // TODO: should be removed
    int64_t filter_out_stride,

    float* filter_cvt,
    int64_t filter_cvt_wg_tile_stride)
{
    float tmp[8][3];
    for (int64_t i = 0; i < 8; i++) {
        tmp[i][0] = trans_mat[i * 3 + 0] * filter[0] + trans_mat[i * 3 + 1] * filter[3] + trans_mat[i * 3 + 2] * filter[6];
        tmp[i][1] = trans_mat[i * 3 + 0] * filter[1] + trans_mat[i * 3 + 1] * filter[4] + trans_mat[i * 3 + 2] * filter[7];
        tmp[i][2] = trans_mat[i * 3 + 0] * filter[2] + trans_mat[i * 3 + 1] * filter[5] + trans_mat[i * 3 + 2] * filter[8];
    }
    for (int64_t i = 0; i < 8; i++) {
        for (int64_t j = 0; j < 8; j++) {
            int64_t flt_cvt_idx = 0;
            flt_cvt_idx += (i * 8 + j) * filter_cvt_wg_tile_stride;
            filter_cvt[flt_cvt_idx] = tmp[i][0] * trans_mat[j * 3 + 0] + tmp[i][1] * trans_mat[j * 3 + 1] + tmp[i][2] * trans_mat[j * 3 + 2];
        }
    }
}

uint64_t conv2d_n4cx_wg_b6f3_fp32_runtime_executor::cal_temp_buffer_size() {
    LOG(DEBUG) << "n4cx wg b6f3: cal temp buffer size";

    size_t temp_buffer_size = conv_wg_bxfxs1_get_temp_buffer_size_fp32<6, 3>(
        src_shape_->GetDim(2), // src_h,
        src_shape_->GetDim(3), // src_w,
        conv_param_->channels, // channels,
        conv_param_->num_output, // num_outs,
        conv_param_->group, // group,
        conv_param_->pad_h, // padding_h,
        conv_param_->pad_w, // padding_w,
        tunning_param_.oh_blk, // blk_dst_h,
        tunning_param_.ow_blk, // blk_dst_w,
        tunning_param_.ic_blk, // blk_channels,
        tunning_param_.oc_blk); // blk_num_outs

    return temp_buffer_size;
}

bool conv2d_n4cx_wg_b6f3_fp32_offline_manager::is_supported() {
    return true;
}

ppl::common::RetCode conv2d_n4cx_wg_b6f3_fp32_offline_manager::fast_init_tunning_param() {
    tunning_param_.oh_blk = 16;
    tunning_param_.ow_blk = 16;
    tunning_param_.ic_blk = 256;
    tunning_param_.oc_blk = 256 / 16 / 16 * 128;

    return ppl::common::RC_SUCCESS;
}

ppl::common::RetCode conv2d_n4cx_wg_b6f3_fp32_offline_manager::pick_best_tunning_param(
    const float* src, const float* filter, float* dst, ppl::nn::TensorShape& src_shape,
    ppl::nn::TensorShape& dst_shape) {
    LOG(DEBUG) << "n4cx wg b6f3: pick best tunning param";

    return ppl::common::RC_SUCCESS;
}

ppl::common::RetCode conv2d_n4cx_wg_b6f3_fp32_offline_manager::gen_cvt_weights(const float* filter, const float* bias) {
    if (cvt_filter_ != nullptr || cvt_bias_ != nullptr) {
        return ppl::common::RC_PERMISSION_DENIED;
    }
    LOG(DEBUG) << "n4cx wg b6f3: gen cvt weight";

    const int64_t num_output = param_.num_output;
    const int64_t channels = param_.channels;
    const int64_t kernel_h = param_.kernel_h;
    const int64_t kernel_w = param_.kernel_w;
    const int64_t group = param_.group;

    {
        cvt_bias_size_ = round_up(num_output, 8);
        cvt_bias_ = (float*)allocator_->Alloc(cvt_bias_size_ * sizeof(float));
        memcpy(cvt_bias_, bias, num_output * sizeof(float));
        memset(cvt_bias_ + num_output, 0.0f, (cvt_bias_size_ - num_output) * sizeof(float));
    }
    {
        cvt_filter_size_ = conv_wg_bxfxs1_get_cvt_filter_size_fp32<6, 3>(channels, num_output, group);
        cvt_filter_ = (float*)allocator_->Alloc(cvt_filter_size_);

        const float trans_mat[8][3] = {
            {1.0f, 0.0f, 0.0f},
            {-2.0f / 9, -2.0f / 9, -2.0f / 9},
            {-2.0f / 9, 2.0f / 9, -2.0f / 9},
            {1.0f / 90, 1.0f / 45, 2.0f / 45},
            {1.0f / 90, -1.0f / 45, 2.0f / 45},
            {1.0f / 45, 1.0f / 90, 1.0f / 180},
            {1.0f / 45, -1.0f / 90, 1.0f / 180},
            {0.0f, 0.0f, 1.0f}};

        const float* trans_mat_ = (const float*)trans_mat;
        conv_wg_bxfxs1_cvt_filter_fp32<6, 3, wb_b6f3s1_cvt_filter_blk_kernel>(
            filter, // filter,
            trans_mat_, // trans_mat_,
            channels, // channels,
            num_output, // num_outs,
            group, // group,
            tunning_param_.ic_blk, // blk_channels,
            tunning_param_.oc_blk, // blk_num_outs,
            cvt_filter_); // filter_cvt
    }

    return ppl::common::RC_SUCCESS;
}

}}}; //  namespace ppl::kernel::riscv