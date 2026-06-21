############################################################################
# apps/graphics/wing/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

include $(APPDIR)/Make.defs

WING_ROOT = $(APPDIR)/graphics/wing

CSRCS += wing_gui.c
CSRCS += wing_box.c
CSRCS += wing_button.c
CSRCS += wing_label.c
CSRCS += wing_image.c
CSRCS += wing_panel.c
CSRCS += wing_card.c
CSRCS += wing_scroll_view.c
CSRCS += wing_progress.c
CSRCS += wing_slider.c
CSRCS += wing_scrollbar.c
CSRCS += wing_switch.c
CSRCS += wing_checkbox.c
CSRCS += wing_text_input.c
CSRCS += wing_animation.c
CSRCS += wing_capture.c
CSRCS += wing_space.c
CSRCS += wing_dirty.c
CSRCS += wing_event.c
CSRCS += wing_focus.c
CSRCS += wing_font.c
CSRCS += wing_input.c
CSRCS += wing_layout.c
CSRCS += wing_object.c
CSRCS += wing_render.c
CSRCS += wing_theme.c
CSRCS += wing_tick.c
CSRCS += wing_text_edit.c
CSRCS += wing_timer.c
CSRCS += wing_value.c
CSRCS += wing_widget_input.c
CSRCS += wing_widget_style.c
CSRCS += wing_value_input.c
CSRCS += wing_widget_value.c
CSRCS += wing_widget.c

VPATH += $(WING_ROOT)/src
VPATH += $(WING_ROOT)/src/core
VPATH += $(WING_ROOT)/src/widgets
VPATH += $(WING_ROOT)/src/widgets/internal

CFLAGS += ${INCDIR_PREFIX}$(WING_ROOT)/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/graphics/frender/include

include $(APPDIR)/Application.mk
