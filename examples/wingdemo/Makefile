############################################################################
# apps/examples/wingdemo/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME = wingdemo
PRIORITY = $(CONFIG_EXAMPLES_WINGDEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_WINGDEMO_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_WINGDEMO)

MAINSRC = wingdemo_main.c

CSRCS += lv_demo_widgets.c
CSRCS += lv_demo_widgets_components.c
CSRCS += lv_demo_widgets_profile.c
CSRCS += lv_demo_widgets_analytics.c
CSRCS += lv_demo_widgets_shop.c
CSRCS += img_demo_widgets_avatar.c
CSRCS += img_demo_widgets_needle.c
CSRCS += img_lvgl_logo.c
CSRCS += img_clothes.c

VPATH += lvgl/demos/widgets
VPATH += lvgl/demos/widgets/assets

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/examples/wingdemo/lvgl
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/examples/wingdemo/lvgl/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/examples/wingdemo/lvgl/demos
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/examples/wingdemo/lvgl/demos/widgets

include $(APPDIR)/Application.mk
