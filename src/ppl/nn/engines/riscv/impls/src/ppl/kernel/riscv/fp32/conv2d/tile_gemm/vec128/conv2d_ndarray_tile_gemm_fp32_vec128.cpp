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

#include "ppl/common/log.h"
#include "ppl/kernel/riscv/common/math.h"
#include "ppl/kernel/riscv/fp32/conv2d/tile_gemm/vec128/conv2d_ndarray_tile_gemm_fp32_vec128.h"
#include "ppl/kernel/riscv/fp32/conv2d/tile_gemm/conv2d_generic_tile_gemm_fp32_vec128.h"
#include "ppl/kernel/riscv/fp32/conv2d/common/conv2d_shell_fp32.h"

namespace ppl { namespace kernel { namespace riscv {

uint64_t conv2d_ndarray_tile_gemm_fp32_runtime_executor::cal_temp_buffer_size() {
    size_t temp_buffer_size = conv2d_nxcx_tile_gemm_get_temp_buffer_size_fp32_vec128<1>(
        src_shape_->GetDim(2), // src_h
        src_shape_->GetDim(3), // src_w
        conv_param_->pad_h,
        conv_param_->pad_w,
        conv_param_->stride_h,
        conv_param_->stride_w,
        conv_param_->kernel_h,
        conv_param_->kernel_w,
        conv_param_->dilation_h,
        conv_param_->dilation_w,
        conv_param_->channels,
        conv_param_->group,
        conv_param_->num_output,
        tunning_param_.m_blk,
        tunning_param_.oh_blk,
        tunning_param_.ow_blk,
        tunning_param_.num_thread
    );

    return temp_buffer_size;
}

void conv2d_ndarray_tile_gemm_fp32_runtime_executor::adjust_tunning_param() {
    auto dst_h = dst_shape_->GetDim(2);
    auto dst_w = dst_shape_->GetDim(3);
    const int64_t num_outs_per_group = conv_param_->num_output / conv_param_->group;

    tunning_param_.oh_blk = min(dst_h, tunning_param_.oh_blk);
    tunning_param_.ow_blk = min(dst_w, tunning_param_.ow_blk);
    tunning_param_.m_blk = min(tunning_param_.m_blk, round_up(num_outs_per_group, 4));
}

ppl::common::RetCode conv2d_ndarray_tile_gemm_fp32_runtime_executor::prepare() {
    if (!conv_param_ || !src_shape_ || !dst_shape_) {
        return ppl::common::RC_INVALID_VALUE;
    }

    adjust_tunning_param();
    return ppl::common::RC_SUCCESS;
}

static int64_t conv2d_tile_gemm_get_real_filter_size(const int64_t flt) {
    return flt;
}

ppl::common::RetCode conv2d_ndarray_tile_gemm_fp32_runtime_executor::execute() {
    const conv2d_common_param& cp = *conv_param_;

    if (src_ == nullptr || cvt_bias_ == nullptr || cvt_filter_ == nullptr || temp_buffer_ == nullptr ||
        dst_ == nullptr) {
        return ppl::common::RC_INVALID_VALUE;
    }

    conv2d_shell_fp32<conv2d_nxcx_conv_tile_gemm_tunning_info, 1, conv2d_tile_gemm_get_real_filter_size,
                      conv2d_nxcx_conv_tile_gemm_riscv_per_group_fp32_vec128<1>>(
        src_,
        cvt_filter_,
        cvt_bias_,
        (float*)temp_buffer_,
        dst_,
        src_shape_->GetDim(2), // src_h
        src_shape_->GetDim(3), // src_w
        conv_param_->pad_h,
        conv_param_->pad_w,
        conv_param_->kernel_h,
        conv_param_->kernel_w,
        conv_param_->stride_h,
        conv_param_->stride_w,
        conv_param_->dilation_h,
        conv_param_->dilation_w,
        conv_param_->channels,
        conv_param_->num_output,
        conv_param_->group,
        src_shape_->GetDim(0), // batch
        {
            tunning_param_.m_blk,
            tunning_param_.k_blk,
            tunning_param_.oh_blk,
            tunning_param_.ow_blk,
            tunning_param_.num_thread
        }
    );

    return ppl::common::RC_SUCCESS;
}

bool conv2d_ndarray_tile_gemm_fp32_offline_manager::is_supported() {
    return true;
}

ppl::common::RetCode conv2d_ndarray_tile_gemm_fp32_offline_manager::fast_init_tunning_param() {
    const int64_t channels_per_group = param_.channels / param_.group;
    const int64_t num_outs_per_group = param_.num_output / param_.group;

    tunning_param_.k_blk = channels_per_group * param_.kernel_h * param_.kernel_w;
    tunning_param_.m_blk = 16;
    tunning_param_.m_blk = min(tunning_param_.m_blk, round_up(num_outs_per_group, 4));

    tunning_param_.oh_blk = 3;
    tunning_param_.ow_blk = 7;
    tunning_param_.num_thread = 1;
    return ppl::common::RC_SUCCESS;
}

ppl::common::RetCode conv2d_ndarray_tile_gemm_fp32_offline_manager::pick_best_tunning_param(
    const float* src, const float* filter, float* dst, ppl::nn::TensorShape& src_shape,
    ppl::nn::TensorShape& dst_shape) {
    const int64_t atom_c = 4;
    auto best_tunnig_param = tunning_param_;
    profile_tunning_param(src, filter, dst, src_shape, dst_shape);
    double best_time = profile_tunning_param(src, filter, dst, src_shape, dst_shape);

    const int64_t num_outs_per_group = param_.num_output / param_.group;

    for (tunning_param_.m_blk = 16; tunning_param_.m_blk <= round_up(num_outs_per_group, atom_c);
         tunning_param_.m_blk *= 2) {
        for (tunning_param_.oh_blk = 4; tunning_param_.oh_blk <= dst_shape.GetDim(2); tunning_param_.oh_blk += 4) {
            double inner_prev_time = DBL_MAX;
            for (tunning_param_.ow_blk = 7; tunning_param_.ow_blk <= dst_shape.GetDim(3); tunning_param_.ow_blk += 7) {
                double this_time = profile_tunning_param(src, filter, dst, src_shape, dst_shape);
                if (this_time < best_time) {
                    best_time = this_time;
                    best_tunnig_param = tunning_param_;
                }
                if (this_time < inner_prev_time) {
                    inner_prev_time = this_time;
                } else {
                    break;
                }
            }
        }
    }
    tunning_param_ = best_tunnig_param;

    LOG(DEBUG) << "tile gemm best tunning " << best_tunnig_param.m_blk << " " << best_tunnig_param.oh_blk << " "
               << best_tunnig_param.ow_blk;
    return ppl::common::RC_SUCCESS;
}

ppl::common::RetCode conv2d_ndarray_tile_gemm_fp32_offline_manager::gen_cvt_weights(const float* filter,
                                                                                    const float* bias) {
    if (cvt_bias_ != nullptr || cvt_filter_ != nullptr) {
        return ppl::common::RC_PERMISSION_DENIED;
    }

    const int64_t num_output = param_.num_output;
    const int64_t channels = param_.channels;
    const int64_t kernel_h = param_.kernel_h;
    const int64_t kernel_w = param_.kernel_w;
    const int64_t num_group = param_.group;

    {
        cvt_bias_size_ = round_up(num_output, 4);
        cvt_bias_ = (float*)allocator_->Alloc(cvt_bias_size_ * sizeof(float));
        memcpy(cvt_bias_, bias, num_output * sizeof(float));
        memset(cvt_bias_ + num_output, 0.f, (cvt_bias_size_ - num_output) * sizeof(float));
    }

    {
        cvt_filter_size_ = conv2d_nxcx_conv_tile_gemm_get_cvt_filter_size_fp32_vec128<1>(
            kernel_h,
            kernel_w,
            channels,
            num_output,
            num_group
        );

        cvt_filter_ = (float*)allocator_->Alloc(cvt_filter_size_);
        conv2d_nxcx_conv_tile_gemm_cvt_filter_fp32_vec128<1>(
            filter,
            kernel_h,
            kernel_w,
            num_output,
            channels,
            num_group,
            tunning_param_.m_blk,
            tunning_param_.k_blk,
            cvt_filter_
        );
    }

    return ppl::common::RC_SUCCESS;
}

}}}; // namespace ppl::kernel::riscv