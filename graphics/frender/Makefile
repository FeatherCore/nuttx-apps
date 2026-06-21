############################################################################
# apps/graphics/frender/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

FRENDER_ROOT = $(APPDIR)/graphics/frender

CSRCS += frender.c

ifeq ($(CONFIG_GRAPHICS_FRENDER_FB),y)
CSRCS += fr_backend_framebuffer.c
endif

ifeq ($(CONFIG_GRAPHICS_FRENDER_NX),y)
CSRCS += fr_backend_nx.c
endif

VPATH += $(FRENDER_ROOT)/src

CFLAGS += ${INCDIR_PREFIX}$(FRENDER_ROOT)/include

include $(APPDIR)/Application.mk
