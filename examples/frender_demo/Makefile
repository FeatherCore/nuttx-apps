############################################################################
# apps/examples/frender_demo/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = $(CONFIG_EXAMPLES_FRENDER_DEMO_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_FRENDER_DEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_FRENDER_DEMO_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_FRENDER_DEMO)

MAINSRC = frender_demo_main.c

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/graphics/frender/include

include $(APPDIR)/Application.mk
