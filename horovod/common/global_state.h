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

#ifndef HOROVOD_GLOBAL_STATE_H
#define HOROVOD_GLOBAL_STATE_H

#include <thread>

#include "fusion_buffer_manager.h"
#include "parameter_manager.h"
#include "timeline.h"

namespace horovod {
namespace common {

// The global state required for the MPI ops.
//
// MPI is a library that stores a lot of global per-program state and often
// requires running on a single thread. As a result, we have to have a single
// background thread responsible for all MPI operations, and communicate with
// that background thread through global state.
struct HorovodGlobalState {
  // An atomic boolean which is set to true when background thread is started.
  // This ensures that only one background thread is spawned.
  std::atomic_flag initialize_flag = ATOMIC_FLAG_INIT;

  // A mutex that needs to be used whenever MPI operations are done.
  std::mutex mutex;

  // Tensors waiting to be allreduced or allgathered.
  TensorTable tensor_table;

  // Background thread running MPI communication.
  std::thread background_thread;

  // Whether the background thread should shutdown.
  std::atomic_bool shut_down {false};

  // Whether Horovod should finalize MPI (only if it has initialized it).
  bool should_finalize = false;

  // Time point when coordinator last checked for stalled tensors.
  std::chrono::steady_clock::time_point last_stall_check;

  // Flag indicating whether to perform stall tensor check.
  bool perform_stall_check = true;

  // Timeline writer.
  Timeline timeline;

  // Flag indicating whether to mark cycles in the timeline.
  bool mark_cycles_in_timeline = false;

  ParameterManager param_manager;

  // Encapsulates the fusion buffers, handles resizing and auto-tuning of buffer size.
  FusionBufferManager fusion_buffer;

  // Time point when last cycle started.
  std::chrono::steady_clock::time_point last_cycle_start;

  // Whether MPI_Init has been completed on the background thread.
  std::atomic_bool initialization_done {false};

  // The MPI rank, local rank, size, local size, flag indicating whether MPI
  // multi-threading is supported, ranks from which the MPI communicator will
  // be made and the communicator itself.
  int rank = 0;
  int local_rank = 0;
  int cross_rank = 0;
  int size = 1;
  int local_size = 1;
  int cross_size = 1;
  bool mpi_threads_supported = false;
  bool is_homogeneous = false;
  std::vector<int> ranks;

  // COMM_WORLD ranks of processes running on this node.
  std::vector<int> local_comm_ranks;

  // Numbers of ranks running per node
  std::vector<int> local_sizes;

  // Pointer to shared buffer for allgather
  void* shared_buffer = nullptr;

  // Current shared buffer size
  int64_t shared_buffer_size = 0;

  // Will be set to true after initialization when ddl is used
  bool ddl_initialized = false;
  int32_t ddl_local_device_id = 0;

  ~HorovodGlobalState() {
    // Make sure that the destructor of the background thread is safe to
    // call. If a thread is still joinable (not detached or complete) its
    // destructor cannot be called.
    if (background_thread.joinable()) {
      shut_down = true;
      background_thread.join();
    }
  }
};

} // namespace common
} // namespace horovod

#endif //HOROVOD_GLOBAL_STATE_H
