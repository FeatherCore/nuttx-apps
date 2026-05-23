############################################################################
# apps/testing/mm/sysresume/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

# Protected syscall resume test

PROGNAME = $(CONFIG_TESTING_SYSCALL_RESUME_PROGNAME)
PRIORITY = $(CONFIG_TESTING_SYSCALL_RESUME_PRIORITY)
STACKSIZE = $(CONFIG_TESTING_SYSCALL_RESUME_STACKSIZE)
MODULE = $(CONFIG_TESTING_SYSCALL_RESUME)

MAINSRC = sysresume_main.c

include $(APPDIR)/Application.mk
