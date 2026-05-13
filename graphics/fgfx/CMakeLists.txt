# ##############################################################################
# apps/graphics/fgfx/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_GRAPHICS_FGFX)
  set(FGFX_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../stacks/fgfx)

  # Keep this source list aligned with stacks/fgfx/CMakeLists.txt.  This apps
  # target only compiles the existing C implementation into NuttX apps.

  set(
    SRCS
    ${FGFX_ROOT}/clang/src/core/fgfx_display_list.c
    ${FGFX_ROOT}/clang/src/core/fgfx_display_list_2d.c
    ${FGFX_ROOT}/clang/src/core/fgfx_display_list_3d.c
    ${FGFX_ROOT}/clang/src/core/fgfx_display_list_state.c
    ${FGFX_ROOT}/clang/src/core/fgfx_transform.c
    ${FGFX_ROOT}/clang/src/core/fgfx_context.c
    ${FGFX_ROOT}/clang/src/core/fgfx_fallback_cache.c
    ${FGFX_ROOT}/clang/src/core/fgfx_encoder.c
    ${FGFX_ROOT}/clang/src/core/fgfx_frame_graph.c
    ${FGFX_ROOT}/clang/src/core/fgfx_geometry.c
    ${FGFX_ROOT}/clang/src/core/fgfx_planned_executor.c
    ${FGFX_ROOT}/clang/src/core/fgfx_planner.c
    ${FGFX_ROOT}/clang/src/core/fgfx_planner_dirty.c
    ${FGFX_ROOT}/clang/src/core/fgfx_planner_pass.c
    ${FGFX_ROOT}/clang/src/core/fgfx_resource.c
    ${FGFX_ROOT}/clang/src/core/fgfx_resource_cache.c
    ${FGFX_ROOT}/clang/src/core/fgfx_surface_state.c
    ${FGFX_ROOT}/clang/src/core/fgfx_text.c
    ${FGFX_ROOT}/clang/src/backends/soft/fgfx_soft.c
    ${FGFX_ROOT}/clang/src/backends/soft/fgfx_soft/common.c
    ${FGFX_ROOT}/clang/src/backends/soft/fgfx_soft/ops2d.c
    ${FGFX_ROOT}/clang/src/backends/soft/fgfx_soft/ops3d.c
    ${FGFX_ROOT}/clang/src/backends/blitter2d/fgfx_blitter2d.c
    ${FGFX_ROOT}/clang/src/backends/nema/fgfx_nema.c
    ${FGFX_ROOT}/clang/src/diagnostics/trace/fgfx_trace.c)

  nuttx_add_library(fgfx STATIC)
  target_sources(fgfx PRIVATE ${SRCS})
  target_include_directories(
    fgfx
    PUBLIC ${FGFX_ROOT}/clang/include
    PRIVATE ${FGFX_ROOT}/clang/src/core
            ${FGFX_ROOT}/clang/src/backends/soft
            ${FGFX_ROOT}/clang/src/backends/soft/fgfx_soft)

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${FGFX_ROOT}/clang/include)
endif()
