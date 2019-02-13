// Copyright 2016 The TensorFlow Authors. All Rights Reserved.
// Modifications copyright (C) 2019 Uber Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#ifndef HOROVOD_DDL_OPERATIONS_H
#define HOROVOD_DDL_OPERATIONS_H

#include <ddl.hpp>

#include "cuda_operations.h"

namespace horovod {
namespace common {

class DDLAllreduce : public CUDACustomAllreduce {
public:
  DDLAllreduce(CUDAContext* cuda_context,
               CommunicationContext* comm_context,
               HorovodGlobalState* global_state);

protected:
  void InitComm(std::vector<TensorTableEntry>& entries, const std::vector<int32_t>& devices) override;
  void CustomAllreduce(std::vector<TensorTableEntry>& entries,
                       cudaStream_t& stream, std::queue<std::pair<std::string, cudaEvent_t>>& event_queue,
                       const void* fused_input_data, void* buffer_data,
                       int64_t& num_elements, size_t& buffer_len, void* host_buffer) override;
};

} // namespace common
} // namespace horovod

#endif //HOROVOD_DDL_OPERATIONS_H
