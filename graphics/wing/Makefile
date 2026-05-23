############################################################################
# apps/graphics/wing/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

WING_ROOT = $(APPDIR)/../stacks/wing

# Keep this source list aligned with stacks/wing/CMakeLists.txt.  This apps
# integration is only a build-system wrapper around the canonical stack.

CSRCS += wing_draw.c
CSRCS += wing_draw_sw.c
CSRCS += wing_reconcile.c
CSRCS += wing_lvgl.c
CSRCS += wing_world.c

VPATH += $(WING_ROOT)/src

CFLAGS += ${INCDIR_PREFIX}$(WING_ROOT)/include

include $(APPDIR)/Application.mk
