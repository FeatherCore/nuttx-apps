############################################################################
# apps/examples/hwsim_testmode/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)common$(DELIM)include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)libnl-3.2.25$(DELIM)include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)libnl-3.2.25$(DELIM)lib
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)$(DELIM)wireless$(DELIM)ieee80211$(DELIM)include$(DELIM)uapi
CFLAGS += -include nuttx_libnl_compat.h

PROGNAME  = $(CONFIG_EXAMPLES_HWSIM_TESTMODE_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_HWSIM_TESTMODE_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_HWSIM_TESTMODE_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_HWSIM_TESTMODE)

MAINSRC = hwsim_testmode_main.c

include $(APPDIR)/Application.mk
