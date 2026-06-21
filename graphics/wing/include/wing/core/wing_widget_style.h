/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_widget_style.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_STYLE_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_STYLE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include <wing/wing.h>

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void wing_widget_draw_style(wing_context_t *ctx, const wing_rect_t *rect,
                            const wing_box_style_t *style);
void wing_widget_draw_style_for_obj(wing_context_t *ctx,
                                    const wing_obj_t *obj,
                                    const wing_rect_t *rect,
                                    const wing_box_style_t *style);
void wing_widget_draw_style_background_for_obj(
  wing_context_t *ctx, const wing_obj_t *obj, const wing_rect_t *rect,
  const wing_box_style_t *style);
void wing_widget_draw_style_overlay_for_obj(
  wing_context_t *ctx, const wing_obj_t *obj, const wing_rect_t *rect,
  const wing_box_style_t *style);
int wing_widget_fill_rect_for_obj(wing_context_t *ctx,
                                  const wing_obj_t *obj,
                                  const wing_rect_t *rect,
                                  wing_color_t color);
wing_color_t wing_widget_style_color(const wing_box_style_t *style,
                                     wing_color_t color);
wing_color_t wing_widget_style_color_for_obj(const wing_obj_t *obj,
                                             const wing_box_style_t *style,
                                             wing_color_t color);
void wing_widget_state_style_init(wing_widget_state_style_t *state_style);
const wing_box_style_t *wing_widget_select_style(
  const wing_obj_t *obj, const wing_box_style_t *style,
  const wing_widget_state_style_t *state_style);
bool wing_widget_set_state_style(wing_obj_t *obj,
                                 wing_widget_state_style_t *state_style,
                                 uint16_t state,
                                 const wing_box_style_t *style);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_STYLE_H */
