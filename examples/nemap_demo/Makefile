############################################################################
# apps/examples/nemap_demo/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME = $(CONFIG_EXAMPLES_NEMAP_DEMO_PROGNAME)
PRIORITY = $(CONFIG_EXAMPLES_NEMAP_DEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_NEMAP_DEMO_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_NEMAP_DEMO)

MAINSRC = nemap_demo_main.c

ifeq ($(CONFIG_EXAMPLES_NEMAP_DEMO_STM32U5),y)
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32u5
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32u5/hardware
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/common
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/armv8-m
endif

ifeq ($(CONFIG_EXAMPLES_NEMAP_DEMO_STM32N6),y)
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32n6
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32n6/hardware
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/common
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/armv8-m
endif

ifeq ($(CONFIG_EXAMPLES_NEMAP_DEMO_STM32H7RS),y)
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32h7rs
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/stm32h7rs/hardware
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/common
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)/arch/arm/src/armv7-m
endif

include $(APPDIR)/Application.mk
