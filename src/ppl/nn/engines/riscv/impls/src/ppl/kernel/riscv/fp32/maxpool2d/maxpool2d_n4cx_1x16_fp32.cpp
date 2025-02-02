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

#include <riscv-vector.h>
#include "ppl/kernel/riscv/common/internal_include.h"
#include "ppl/nn/params/onnx/pooling_param.h"
#include "ppl/kernel/riscv/common/maxpool2d/maxpool2d_common.h"

namespace ppl { namespace kernel { namespace riscv {

#define C_BLK() ((int64_t)4)

template <int64_t w_len>
static void maxpool2d_n4cx_1x16_kernel_fp32(
    const float* src,
    float* dst,

    const maxpool2d_param* param,
    const int64_t oh,
    const int64_t ow,
    const int64_t ih_start_valid,
    const int64_t ih_end_valid)
{
    const int32_t& kernel_h = param->kernel_h;
    const int32_t& kernel_w = param->kernel_w;
    const int32_t& pad_w = param->pad_w;
    const int32_t& src_w = param->src_w;
    const int32_t& dst_w = param->dst_w;
    const int32_t& stride_w = param->stride_w;

    const int32_t iw_start_valid = -pad_w + ow * stride_w;
    const int32_t iw_end_valid = iw_start_valid + kernel_w;

    const auto vl = vsetvli(C_BLK(), RVV_E32, RVV_M1);
    float32xm1_t vfmax0, vfmax1, vfmax2, vfmax3;
    float32xm1_t vfmax4, vfmax5, vfmax6, vfmax7;
    float32xm1_t vfmax8, vfmax9, vfmax10, vfmax11;
    float32xm1_t vfmax12, vfmax13, vfmax14, vfmax15;

    if (w_len >= 1) vfmax0 = vfmvvf_float32xm1(-__FLT_MAX__, vl);
    if (w_len >= 2) vfmax1 = vfmax0;
    if (w_len >= 3) vfmax2 = vfmax0;
    if (w_len >= 4) vfmax3 = vfmax0;
    if (w_len >= 5) vfmax4 = vfmax0;
    if (w_len >= 6) vfmax5 = vfmax0;
    if (w_len >= 7) vfmax6 = vfmax0;
    if (w_len >= 8) vfmax7 = vfmax0;
    if (w_len >= 9) vfmax8 = vfmax0;
    if (w_len >= 10) vfmax9 = vfmax0;
    if (w_len >= 11) vfmax10 = vfmax0;
    if (w_len >= 12) vfmax11 = vfmax0;
    if (w_len >= 13) vfmax12 = vfmax0;
    if (w_len >= 14) vfmax13 = vfmax0;
    if (w_len >= 15) vfmax14 = vfmax0;
    if (w_len >= 16) vfmax15 = vfmax0;

    for (int64_t ih = ih_start_valid; ih < ih_end_valid; ih++) {
        for (int64_t iw = iw_start_valid; iw < iw_end_valid; iw++) {
            const float* src_p = src + (ih * src_w + iw) * C_BLK();
            if (w_len >= 1) vfmax0 = vfmaxvv_float32xm1(vfmax0, vlev_float32xm1(src_p + 0 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 2) vfmax1 = vfmaxvv_float32xm1(vfmax1, vlev_float32xm1(src_p + 1 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 3) vfmax2 = vfmaxvv_float32xm1(vfmax2, vlev_float32xm1(src_p + 2 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 4) vfmax3 = vfmaxvv_float32xm1(vfmax3, vlev_float32xm1(src_p + 3 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 5) vfmax4 = vfmaxvv_float32xm1(vfmax4, vlev_float32xm1(src_p + 4 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 6) vfmax5 = vfmaxvv_float32xm1(vfmax5, vlev_float32xm1(src_p + 5 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 7) vfmax6 = vfmaxvv_float32xm1(vfmax6, vlev_float32xm1(src_p + 6 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 8) vfmax7 = vfmaxvv_float32xm1(vfmax7, vlev_float32xm1(src_p + 7 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 9) vfmax8 = vfmaxvv_float32xm1(vfmax8, vlev_float32xm1(src_p + 8 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 10) vfmax9 = vfmaxvv_float32xm1(vfmax9, vlev_float32xm1(src_p + 9 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 11) vfmax10 = vfmaxvv_float32xm1(vfmax10, vlev_float32xm1(src_p + 10 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 12) vfmax11 = vfmaxvv_float32xm1(vfmax11, vlev_float32xm1(src_p + 11 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 13) vfmax12 = vfmaxvv_float32xm1(vfmax12, vlev_float32xm1(src_p + 12 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 14) vfmax13 = vfmaxvv_float32xm1(vfmax13, vlev_float32xm1(src_p + 13 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 15) vfmax14 = vfmaxvv_float32xm1(vfmax14, vlev_float32xm1(src_p + 14 * stride_w * C_BLK(), vl), vl);
            if (w_len >= 16) vfmax15 = vfmaxvv_float32xm1(vfmax15, vlev_float32xm1(src_p + 15 * stride_w * C_BLK(), vl), vl);
        }
    }

    float* dst_p = dst + (oh * dst_w + ow) * C_BLK();
    if (w_len >= 1) vsev_float32xm1(dst_p + 0 * C_BLK(), vfmax0, vl);
    if (w_len >= 2) vsev_float32xm1(dst_p + 1 * C_BLK(), vfmax1, vl);
    if (w_len >= 3) vsev_float32xm1(dst_p + 2 * C_BLK(), vfmax2, vl);
    if (w_len >= 4) vsev_float32xm1(dst_p + 3 * C_BLK(), vfmax3, vl);
    if (w_len >= 5) vsev_float32xm1(dst_p + 4 * C_BLK(), vfmax4, vl);
    if (w_len >= 6) vsev_float32xm1(dst_p + 5 * C_BLK(), vfmax5, vl);
    if (w_len >= 7) vsev_float32xm1(dst_p + 6 * C_BLK(), vfmax6, vl);
    if (w_len >= 8) vsev_float32xm1(dst_p + 7 * C_BLK(), vfmax7, vl);
    if (w_len >= 9) vsev_float32xm1(dst_p + 8 * C_BLK(), vfmax8, vl);
    if (w_len >= 10) vsev_float32xm1(dst_p + 9 * C_BLK(), vfmax9, vl);
    if (w_len >= 11) vsev_float32xm1(dst_p + 10 * C_BLK(), vfmax10, vl);
    if (w_len >= 12) vsev_float32xm1(dst_p + 11 * C_BLK(), vfmax11, vl);
    if (w_len >= 13) vsev_float32xm1(dst_p + 12 * C_BLK(), vfmax12, vl);
    if (w_len >= 14) vsev_float32xm1(dst_p + 13 * C_BLK(), vfmax13, vl);
    if (w_len >= 15) vsev_float32xm1(dst_p + 14 * C_BLK(), vfmax14, vl);
    if (w_len >= 16) vsev_float32xm1(dst_p + 15 * C_BLK(), vfmax15, vl);
}

typedef void (*maxpool2d_n4cx_kernel_fp32_func)(const float*, float*, const maxpool2d_param*, const int64_t,
                                                const int64_t, const int64_t, const int64_t);
static const maxpool2d_n4cx_kernel_fp32_func maxpool2d_n4cx_1x16_kernel_select[16]{
    maxpool2d_n4cx_1x16_kernel_fp32<1>, 
    maxpool2d_n4cx_1x16_kernel_fp32<2>,
    maxpool2d_n4cx_1x16_kernel_fp32<3>,
    maxpool2d_n4cx_1x16_kernel_fp32<4>,
    maxpool2d_n4cx_1x16_kernel_fp32<5>,
    maxpool2d_n4cx_1x16_kernel_fp32<6>,
    maxpool2d_n4cx_1x16_kernel_fp32<7>,
    maxpool2d_n4cx_1x16_kernel_fp32<8>,
    maxpool2d_n4cx_1x16_kernel_fp32<9>,
    maxpool2d_n4cx_1x16_kernel_fp32<10>,
    maxpool2d_n4cx_1x16_kernel_fp32<11>,
    maxpool2d_n4cx_1x16_kernel_fp32<12>,
    maxpool2d_n4cx_1x16_kernel_fp32<13>,
    maxpool2d_n4cx_1x16_kernel_fp32<14>,
    maxpool2d_n4cx_1x16_kernel_fp32<15>,
    maxpool2d_n4cx_1x16_kernel_fp32<16>};

static inline void maxpool2d_n4cx_border_fp32(
    const float* src,
    float* dst,

    const maxpool2d_param* param,
    const int64_t oh,
    const int64_t ow,
    const int64_t ih_start_valid,
    const int64_t ih_end_valid)
{
    const int32_t& kernel_h = param->kernel_h;
    const int32_t& kernel_w = param->kernel_w;
    const int32_t& stride_h = param->stride_h;
    const int32_t& stride_w = param->stride_w;
    const int32_t& pad_h = param->pad_h;
    const int32_t& pad_w = param->pad_w;

    const int32_t& src_h = param->src_h;
    const int32_t& src_w = param->src_w;
    const int32_t& dst_h = param->dst_h;
    const int32_t& dst_w = param->dst_w;

    int64_t iw_start = -pad_w + ow * stride_w;
    int64_t iw_end = iw_start + kernel_w;
    int64_t iw_start_valid = max(iw_start, int64_t(0));
    int64_t iw_end_valid = min(iw_end, int64_t(src_w));

    const auto vl = vsetvli(C_BLK(), RVV_E32, RVV_M1);
    float32xm1_t vfmax = vfmvvf_float32xm1(-__FLT_MAX__, vl);
    for (int64_t ih = ih_start_valid; ih < ih_end_valid; ih++) {
        for (int64_t iw = iw_start_valid; iw < iw_end_valid; iw++) {
            const float* src_p = src + (ih * src_w + iw) * C_BLK();
            float32xm1_t v0 = vlev_float32xm1(src_p, vl);
            vfmax = vfmaxvv_float32xm1(vfmax, v0, vl);
        }
    }
    float* dst_p = dst + (oh * dst_w + ow) * C_BLK();
    vsev_float32xm1(dst_p, vfmax, vl);
}

ppl::common::RetCode maxpool2d_n4cx_1x16_fp32(
    const ppl::nn::TensorShape* src_shape,
    const ppl::nn::TensorShape* dst_shape,
    const int32_t kernel_h,
    const int32_t kernel_w,
    const int32_t stride_h,
    const int32_t stride_w,
    const int32_t pad_h,
    const int32_t pad_w,

    const float* src,
    float* dst)
{
    const int32_t batch = src_shape->GetDim(0);
    const int32_t channels = src_shape->GetDim(1);
    const int32_t src_h = src_shape->GetDim(2);
    const int32_t src_w = src_shape->GetDim(3);
    const int32_t dst_h = dst_shape->GetDim(2);
    const int32_t dst_w = dst_shape->GetDim(3);

    const maxpool2d_param param = {kernel_h, kernel_w, stride_h, stride_w, pad_h, pad_w,
                                   batch,    channels, src_h,    src_w,    dst_h, dst_w};

    int32_t padded_channels = round_up(channels, C_BLK());
    int32_t dst_1x16_start_w = max((pad_w + stride_w - 1) / stride_w, 0);
    int32_t dst_1x16_end_w = min((src_w + pad_w - kernel_w) / stride_w + 1, dst_w);

    for (int64_t nc = 0; nc < batch * padded_channels; nc += C_BLK()) {
        const float* src_ = src + nc * src_h * src_w;
        float* dst_ = dst + nc * dst_h * dst_w;
        for (int64_t oh = 0; oh < dst_h; oh++) {
            int64_t ih_start = -pad_h + oh * stride_h;
            int64_t ih_end = ih_start + kernel_h;
            int64_t ih_start_valid = max(ih_start, int64_t(0));
            int64_t ih_end_valid = min(ih_end, int64_t(src_h));

            int64_t ow = 0;
            for (; ow < dst_1x16_start_w; ++ow) {
                maxpool2d_n4cx_border_fp32(src_, dst_, &param, oh, ow, ih_start_valid, ih_end_valid);
            }
            for (; ow + 16 <= dst_1x16_end_w; ow += 16) {
                maxpool2d_n4cx_1x16_kernel_select[15](src_, dst_, &param, oh, ow, ih_start_valid, ih_end_valid);
            }
            if (ow < dst_1x16_end_w) {
                maxpool2d_n4cx_1x16_kernel_select[dst_1x16_end_w - ow - 1](src_, dst_, &param, oh, ow, ih_start_valid, ih_end_valid);
                ow = dst_1x16_end_w;
            }
            for (; ow < dst_w; ++ow) {
                maxpool2d_n4cx_border_fp32(src_, dst_, &param, oh, ow, ih_start_valid, ih_end_valid);
            }
        }
    }
    return ppl::common::RC_SUCCESS;
}

}}}; // namespace ppl::kernel::riscv