/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_widget.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_WIDGET_H
#define __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_WIDGET_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>

#include <wing/wing.h>
#include <wing/core/wing_widget_input.h>
#include <wing/core/wing_widget_style.h>
#include <wing/core/wing_widget_value.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void wing_widget_init_obj(wing_obj_t *obj, const wing_rect_t *bounds,
                          wing_obj_draw_fn_t draw,
                          wing_obj_event_fn_t event, void *user_data,
                          bool focusable);

#endif /* __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_WIDGET_H */
