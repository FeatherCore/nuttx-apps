############################################################################
# apps/graphics/lvgl/Makefile
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

# LVGL graphic library

LVGL_DIR = .
LVGL_DIR_NAME = lvgl

-include ./lvgl/lvgl.mk

ifeq ($(CONFIG_LV_USE_DRAW_ARM2D_SYNC),y)
ARM2D_DIR       := $(CURDIR)/arm-2d
ARM2D_SRC_DIR   := $(ARM2D_DIR)/Library/Source
ARM2D_INC_DIR   := $(ARM2D_DIR)/Library/Include
ARM2D_PORT_DIR  := $(CURDIR)/arm2d-port
ARM2D_CMSIS_DIR := $(ARM2D_DIR)/examples/[template][pc][vscode]/platform/math

ifneq ($(wildcard $(ARM2D_INC_DIR)/arm_2d.h),)
CFLAGS   += ${INCDIR_PREFIX}$(ARM2D_PORT_DIR)
CFLAGS   += ${INCDIR_PREFIX}$(ARM2D_INC_DIR)
CFLAGS   += ${INCDIR_PREFIX}$(ARM2D_INC_DIR)/template
CFLAGS   += ${INCDIR_PREFIX}$(ARM2D_CMSIS_DIR)
CFLAGS   += -flax-vector-conversions
CFLAGS   += -include $(ARM2D_PORT_DIR)/arm2d_gcc_mve_compat.h
CFLAGS   += -D__ARM_2D_SUPPRESS_GCC_HELIUM_PERFORMANCE_WARNING__
CXXFLAGS += ${INCDIR_PREFIX}$(ARM2D_PORT_DIR)
CXXFLAGS += ${INCDIR_PREFIX}$(ARM2D_INC_DIR)
CXXFLAGS += ${INCDIR_PREFIX}$(ARM2D_INC_DIR)/template
CXXFLAGS += ${INCDIR_PREFIX}$(ARM2D_CMSIS_DIR)
CXXFLAGS += -flax-vector-conversions
CXXFLAGS += -D__ARM_2D_SUPPRESS_GCC_HELIUM_PERFORMANCE_WARNING__

CSRCS += $(wildcard $(ARM2D_SRC_DIR)/*.c)
CSRCS += $(ARM2D_CMSIS_DIR)/arm_common_tables.c
CSRCS += $(ARM2D_CMSIS_DIR)/arm_cos_f32.c
CSRCS += $(ARM2D_CMSIS_DIR)/arm_cos_q31.c
CSRCS += $(ARM2D_CMSIS_DIR)/arm_sin_f32.c
CSRCS += $(ARM2D_CMSIS_DIR)/arm_sin_q31.c
else
$(warning CONFIG_LV_USE_DRAW_ARM2D_SYNC requires Arm-2D in $(ARM2D_DIR); run tools/firmware/stm32n6570-dk/build-lvgl.sh)
endif
endif

ifneq ($(CONFIG_LV_BUILD_EXAMPLES),y)
CSRCS := $(filter-out $(LVGL_PATH)/examples/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_BENCHMARK),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/benchmark/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_FLEX_LAYOUT),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/flex_layout/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/keypad_encoder/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_MULTILANG),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/multilang/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_MUSIC),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/music/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_RENDER),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/render/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_SCROLL),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/scroll/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_STRESS),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/stress/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_TRANSFORM),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/transform/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_VECTOR_GRAPHIC),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/vector_graphic/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_DEMO_WIDGETS),y)
CSRCS := $(filter-out $(LVGL_PATH)/demos/widgets/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_BARCODE),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/barcode/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_BMP),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/bmp/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_FFMPEG),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/ffmpeg/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_FREETYPE),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/freetype/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_FS_ARDUINO_ESP_LITTLEFS),y)
CXXSRCS := $(filter-out $(LVGL_PATH)/src/libs/fsdrv/lv_fs_arduino_esp_littlefs.cpp,$(CXXSRCS))
endif

ifneq ($(CONFIG_LV_USE_FS_ARDUINO_SD),y)
CXXSRCS := $(filter-out $(LVGL_PATH)/src/libs/fsdrv/lv_fs_arduino_sd.cpp,$(CXXSRCS))
endif

ifneq ($(CONFIG_LV_USE_GIF),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/gif/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_LIBJPEG_TURBO),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/libjpeg_turbo/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_LIBPNG),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/libpng/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_LODEPNG),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/lodepng/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_LZ4),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/lz4/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_QRCODE),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/qrcode/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_RLE),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/rle/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_RLOTTIE),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/rlottie/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_THORVG),y)
CXXSRCS := $(filter-out $(LVGL_PATH)/src/libs/thorvg/%.cpp,$(CXXSRCS))
endif

ifneq ($(CONFIG_LV_USE_TINY_TTF),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/tiny_ttf/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_TJPGD),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/libs/tjpgd/%.c,$(CSRCS))
endif

ifneq ($(CONFIG_LV_USE_VG_LITE_THORVG),y)
CSRCS := $(filter-out $(LVGL_PATH)/src/others/vg_lite_tvg/%.c,$(CSRCS))
CXXSRCS := $(filter-out $(LVGL_PATH)/src/others/vg_lite_tvg/%.cpp,$(CXXSRCS))
endif

ifneq ($(CONFIG_LV_ASSERT_HANDLER_INCLUDE), "")
CFLAGS += "-DLV_ASSERT_HANDLER=ASSERT(0);"
endif

ifeq ($(and $(CONFIG_LV_USE_PERF_MONITOR),$(CONFIG_SCHED_CPULOAD)),y)
CFLAGS += "-DLV_SYSMON_GET_IDLE=lv_nuttx_get_idle"
endif

ifeq ($(and $(CONFIG_SCHED_INSTRUMENTATION),$(CONFIG_LV_USE_PROFILER)),y)
CFLAGS += "-DLV_PROFILER_BEGIN=sched_note_beginex(NOTE_TAG_GRAPHICS, __func__)"
CFLAGS += "-DLV_PROFILER_END=sched_note_endex(NOTE_TAG_GRAPHICS, __func__)"

CFLAGS += "-DLV_PROFILER_BEGIN_TAG(str)=sched_note_beginex(NOTE_TAG_GRAPHICS, str)"
CFLAGS += "-DLV_PROFILER_END_TAG(str)=sched_note_endex(NOTE_TAG_GRAPHICS, str)"
endif

ifneq ($(CONFIG_LV_OPTLEVEL), "")
# Since multiple options need to be supported, subst needs to be used here to remove
# the redundant double quotes, otherwise it will cause command parsing errors.
CFLAGS   += $(subst ",, $(CONFIG_LV_OPTLEVEL))
CXXFLAGS += $(subst ",, $(CONFIG_LV_OPTLEVEL))
endif

# Set up build configuration and environment

WD := ${shell echo $(CURDIR) | sed -e 's/ /\\ /g'}

CONFIG_GRAPH_LVGL_URL ?= "https://github.com/lvgl/lvgl/archive/refs/tags"

_MAJ         := $(CONFIG_LVGL_VERSION_MAJOR)
_MIN         := $(CONFIG_LVGL_VERSION_MINOR)
_PAT         := $(CONFIG_LVGL_VERSION_PATCH)
LVGL_VERSION := $(_MAJ).$(_MIN).$(_PAT)
LVGL_TARBALL := v$(LVGL_VERSION).zip

LVGL_UNPACKNAME = lvgl
UNPACK ?= unzip -o $(if $(V),,-q)
CURL ?= curl -L -O $(if $(V),,-Ss)

LVGL_UNPACKDIR =  $(WD)/$(LVGL_UNPACKNAME)

$(LVGL_TARBALL):
	$(ECHO_BEGIN)"Downloading: $(LVGL_TARBALL)"
	$(Q) $(CURL) $(CONFIG_GRAPH_LVGL_URL)/$(LVGL_TARBALL)
	$(ECHO_END)

$(LVGL_UNPACKNAME): $(LVGL_TARBALL)
	$(ECHO_BEGIN)"Unpacking: $(LVGL_TARBALL) -> $(LVGL_UNPACKNAME)"
	$(Q) $(UNPACK) $(LVGL_TARBALL)
	$(Q) mv	lvgl-$(LVGL_VERSION) $(LVGL_UNPACKNAME)
	$(Q) touch $(LVGL_UNPACKNAME)
	$(ECHO_END)

# Download and unpack tarball if no git repo found
ifeq ($(wildcard $(LVGL_UNPACKNAME)/.git),)
context:: $(LVGL_UNPACKNAME)
endif

include $(APPDIR)/Application.mk

ifeq ($(wildcard $(LVGL_UNPACKNAME)/.git),)
distclean::
	$(call DELDIR, $(LVGL_UNPACKNAME))
	$(call DELFILE, $(LVGL_TARBALL))
endif
