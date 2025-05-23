# Copyright (c) 2020-2025 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5.0...3.31.3)

project(shortpath CXX)

include(../../common/cmake/common.cmake)

set_common_project_settings(tbb)

add_executable(shortpath shortpath.cpp)

target_link_libraries(shortpath TBB::tbb Threads::Threads)
target_compile_options(shortpath PRIVATE ${TBB_CXX_STD_FLAG})

set(EXECUTABLE "$<TARGET_FILE:shortpath>")
set(ARGS 4 N=1000 start=0 end=999 verbose)
set(PERF_ARGS auto N=1000 start=0 end=99 silent)

add_execution_target(run_shortpath shortpath ${EXECUTABLE} "${ARGS}")
add_execution_target(perf_run_shortpath shortpath ${EXECUTABLE} "${PERF_ARGS}")
