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

#ifndef __ST_PPL_KERNEL_RISCV_FP16_ARITHMETIC_ARITHMETIC_KERNEL_FP16_H_
#define __ST_PPL_KERNEL_RISCV_FP16_ARITHMETIC_ARITHMETIC_KERNEL_FP16_H_

#include <riscv-vector.h>

#include "ppl/kernel/riscv/common/arithmetic/arithmetic_common.h"
#include "ppl/kernel/riscv/common/internal_include.h"
namespace ppl { namespace kernel { namespace riscv {

template <arithmetic_op_type_t _op>
inline float16xm1_t arithmetic_vector_kernel_fp16(float16xm1_t a, float16xm1_t b);

template <>
inline float16xm1_t arithmetic_vector_kernel_fp16<ARITHMETIC_ADD>(float16xm1_t a, float16xm1_t b) {
    return vfaddvv_float16xm1(a, b, vsetvli(8, RVV_E16, RVV_M1));
}
template <>
inline float16xm1_t arithmetic_vector_kernel_fp16<ARITHMETIC_SUB>(float16xm1_t a, float16xm1_t b) {
    return vfsubvv_float16xm1(a, b, vsetvli(8, RVV_E16, RVV_M1));
}
template <>
inline float16xm1_t arithmetic_vector_kernel_fp16<ARITHMETIC_MUL>(float16xm1_t a, float16xm1_t b) {
    return vfmulvv_float16xm1(a, b, vsetvli(8, RVV_E16, RVV_M1));
}
template <>
inline float16xm1_t arithmetic_vector_kernel_fp16<ARITHMETIC_DIV>(float16xm1_t a, float16xm1_t b) {
    return vfdivvv_float16xm1(a, b, vsetvli(8, RVV_E16, RVV_M1));
}

template <arithmetic_op_type_t _op>
inline __fp16 arithmetic_scalar_kernel_fp16(__fp16 a, __fp16 b);

template <>
inline __fp16 arithmetic_scalar_kernel_fp16<ARITHMETIC_ADD>(__fp16 a, __fp16 b) {
    return a + b;
}
template <>
inline __fp16 arithmetic_scalar_kernel_fp16<ARITHMETIC_SUB>(__fp16 a, __fp16 b) {
    return a - b;
}
template <>
inline __fp16 arithmetic_scalar_kernel_fp16<ARITHMETIC_MUL>(__fp16 a, __fp16 b) {
    return a * b;
}
template <>
inline __fp16 arithmetic_scalar_kernel_fp16<ARITHMETIC_DIV>(__fp16 a, __fp16 b) {
    return a / b;
}

struct parallel_block {
    int64_t id;
    int64_t start[PPL_RISCV_TENSOR_MAX_DIMS()];
    int64_t end[PPL_RISCV_TENSOR_MAX_DIMS()];
    int64_t idx[PPL_RISCV_TENSOR_MAX_DIMS()];
};

inline void pad_shape(
    const ppl::nn::TensorShape* shape,
    const int64_t padded_dim_count,
    int64_t* padded_shape)
{
    const int64_t dim_diff = padded_dim_count - shape->GetRealDimCount();
    for (int64_t i = 0; i < dim_diff; i++) {
        padded_shape[i] = 1;
    }
    for (int64_t i = dim_diff; i < padded_dim_count; i++) {
        padded_shape[i] = shape->GetDim(i - dim_diff);
    }
}

inline void idx2dims(
    const int64_t idx,
    const int64_t* shape,
    const int64_t dim_count,
    int64_t* dims) 
{
    int64_t _idx = idx;
    for (int64_t i = dim_count - 1; i >= 0; i--) {
        dims[i] = _idx % shape[i];
        _idx /= shape[i];
    }
}

inline bool is_first_dim(
    parallel_block* block,
    const int64_t dim_idx)
{
    bool is_first = true;
    for (int64_t i = 0; i < dim_idx; i++) {
        if (block->idx[i] != block->start[i]) {
            is_first = false;
            break;
        }
    }
    return is_first;
}

inline bool is_last_dim(
    parallel_block* block,
    const int64_t dim_idx)
{
    bool is_last = true;
    for (int64_t i = 0; i < dim_idx; i++) {
        if (block->idx[i] != block->end[i]) {
            is_last = false;
            break;
        }
    }
    return is_last;
}

}}}; //  namespace ppl::kernel::riscv

#endif //  __ST_PPL_KERNEL_RISCV_FP16_ARITHMETIC_ARITHMETIC_KERNEL_FP16_H_