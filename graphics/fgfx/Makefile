############################################################################
# apps/graphics/fgfx/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

FGFX_ROOT = $(APPDIR)/../stacks/fgfx

# Keep this source list aligned with stacks/fgfx/CMakeLists.txt.  The apps
# integration is a thin build-system wrapper; FGFX behavior still lives in
# stacks/fgfx.

CSRCS += fgfx_display_list.c
CSRCS += fgfx_display_list_2d.c
CSRCS += fgfx_display_list_3d.c
CSRCS += fgfx_display_list_state.c
CSRCS += fgfx_transform.c
CSRCS += fgfx_context.c
CSRCS += fgfx_fallback_cache.c
CSRCS += fgfx_encoder.c
CSRCS += fgfx_frame_graph.c
CSRCS += fgfx_geometry.c
CSRCS += fgfx_planned_executor.c
CSRCS += fgfx_planner.c
CSRCS += fgfx_planner_dirty.c
CSRCS += fgfx_planner_pass.c
CSRCS += fgfx_resource.c
CSRCS += fgfx_resource_cache.c
CSRCS += fgfx_surface_state.c
CSRCS += fgfx_text.c

CSRCS += fgfx_soft.c
CSRCS += common.c
CSRCS += ops2d.c
CSRCS += ops3d.c

CSRCS += fgfx_blitter2d.c
CSRCS += fgfx_nema.c
CSRCS += fgfx_trace.c

VPATH += $(FGFX_ROOT)/clang/src/core
VPATH += $(FGFX_ROOT)/clang/src/backends/soft
VPATH += $(FGFX_ROOT)/clang/src/backends/soft/fgfx_soft
VPATH += $(FGFX_ROOT)/clang/src/backends/blitter2d
VPATH += $(FGFX_ROOT)/clang/src/backends/nema
VPATH += $(FGFX_ROOT)/clang/src/diagnostics/trace

# Private include paths for quoted internal headers when NuttX builds sources
# through VPATH.  Public users only need clang/include from Make.defs.

CFLAGS += ${INCDIR_PREFIX}$(FGFX_ROOT)/clang/src/core
CFLAGS += ${INCDIR_PREFIX}$(FGFX_ROOT)/clang/src/backends/soft
CFLAGS += ${INCDIR_PREFIX}$(FGFX_ROOT)/clang/src/backends/soft/fgfx_soft

include $(APPDIR)/Application.mk
