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

#include "pybind11/pybind11.h"
#include "py_type_creator_manager.h"
#include "ppl/nn/common/logger.h"
using namespace ppl::common;

namespace ppl { namespace nn { namespace python {

#ifdef PPLNN_USE_CUDA
void RegisterCudaEngineFactory(pybind11::module*);
void RegisterCudaEngineOptions(pybind11::module*);
void RegisterCudaEngine(pybind11::module*);
#endif

#ifdef PPLNN_USE_X86
void RegisterX86EngineFactory(pybind11::module*);
void RegisterX86EngineOptions(pybind11::module*);
void RegisterX86Engine(pybind11::module*);
#endif

#ifdef PPLNN_USE_RISCV
void RegisterRiscvEngineFactory(pybind11::module*);
void RegisterRiscvEngineOptions(pybind11::module*);
void RegisterRiscvEngine(pybind11::module*);
#endif

#ifdef PPLNN_USE_ARM
void RegisterArmEngineFactory(pybind11::module*);
void RegisterArmEngineOptions(pybind11::module*);
void RegisterArmEngine(pybind11::module*);
#endif

void RegisterTensorShape(pybind11::module*);
void RegisterTensor(pybind11::module*);
void RegisterNdArray(pybind11::module*);
void RegisterEngine(pybind11::module*);
void RegisterOnnxRuntimeBuilderFactory(pybind11::module*);
void RegisterRuntimeBuilder(pybind11::module*);
void RegisterDeviceContext(pybind11::module*);
void RegisterRuntime(pybind11::module*);
void RegisterGetVersionString(pybind11::module*);

PYBIND11_MODULE(nn, m) {
    RegisterTensorShape(&m);
    RegisterTensor(&m);
    RegisterNdArray(&m);
    RegisterEngine(&m);
    RegisterOnnxRuntimeBuilderFactory(&m);
    RegisterRuntimeBuilder(&m);
    RegisterDeviceContext(&m);
    RegisterRuntime(&m);
    RegisterGetVersionString(&m);

    auto mgr = PyTypeCreatorManager::Instance();
    for (uint32_t i = 0; i < mgr->GetCreatorCount(); ++i) {
        auto creator = mgr->GetCreator(i);
        auto status = creator->Register(&m);
        if (status != RC_SUCCESS) {
            LOG(ERROR) << "register python type failed.";
            return;
        }
    }

#ifdef PPLNN_USE_CUDA
    RegisterCudaEngineFactory(&m);
    RegisterCudaEngineOptions(&m);
    RegisterCudaEngine(&m);
#endif

#ifdef PPLNN_USE_X86
    RegisterX86EngineFactory(&m);
    RegisterX86EngineOptions(&m);
    RegisterX86Engine(&m);
#endif

#ifdef PPLNN_USE_RISCV
    RegisterRiscvEngineFactory(&m);
    RegisterRiscvEngineOptions(&m);
    RegisterRiscvEngine(&m);
#endif

#ifdef PPLNN_USE_ARM
    RegisterArmEngineFactory(&m);
    RegisterArmEngineOptions(&m);
    RegisterArmEngine(&m);
#endif
}

}}} // namespace ppl::nn::python
