/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_object.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_OBJECT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_OBJECT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

void wing_obj_init(wing_obj_t *obj, const wing_rect_t *bounds);
int wing_obj_add_child(wing_obj_t *parent, wing_obj_t *child);
int wing_obj_remove_child(wing_obj_t *child);
int wing_obj_destroy_tree(wing_obj_t *obj);
void wing_obj_set_draw_cb(wing_obj_t *obj, wing_obj_draw_fn_t draw);
void wing_obj_set_event_cb(wing_obj_t *obj, wing_obj_event_fn_t event);
void wing_obj_set_geometry_cb(wing_obj_t *obj, wing_obj_draw_fn_t draw,
                              wing_obj_screen_bounds_fn_t bounds,
                              wing_obj_contains_point_fn_t contains);
void wing_obj_set_screen_bounds_cb(wing_obj_t *obj,
                                   wing_obj_screen_bounds_fn_t bounds);
void wing_obj_set_contains_point_cb(wing_obj_t *obj,
                                    wing_obj_contains_point_fn_t contains);
void wing_obj_set_user_data(wing_obj_t *obj, void *user_data);
void *wing_obj_get_user_data(const wing_obj_t *obj);
const wing_rect_t *wing_obj_get_bounds(const wing_obj_t *obj);
void wing_obj_set_z_index(wing_obj_t *obj, int16_t z_index);
int16_t wing_obj_get_z_index(const wing_obj_t *obj);
void wing_obj_set_opacity(wing_obj_t *obj, uint8_t opacity);
uint8_t wing_obj_get_opacity(const wing_obj_t *obj);
uint8_t wing_obj_get_effective_opacity(const wing_obj_t *obj);
int wing_obj_compare_space_order(const wing_obj_t *a, const wing_obj_t *b);
int wing_obj_project_quad(const wing_obj_t *obj, wing_projected_quad_t *quad);
int wing_obj_get_projected_depth(const wing_obj_t *obj,
                                 int32_t *average_depth);
int wing_obj_project_quad2d(const wing_obj_t *obj, wing_quad2d_t *quad);
bool wing_obj_get_screen_bounds(const wing_obj_t *obj, wing_rect_t *bounds);
bool wing_obj_contains_point(const wing_obj_t *obj, wing_point_t point);
int wing_obj_set_bounds(wing_obj_t *obj, const wing_rect_t *bounds);
void wing_obj_set_space_transform(wing_obj_t *obj,
                                  const wing_space_transform_t *transform);
const wing_space_transform_t *wing_obj_get_space_transform(
  const wing_obj_t *obj);
int wing_obj_get_world_space_transform(const wing_obj_t *obj,
                                       wing_space_transform_t *transform);
bool wing_obj_space_transform_is_identity(const wing_obj_t *obj);
bool wing_obj_is_default_2d(const wing_obj_t *obj);
void wing_obj_reset_space_transform(wing_obj_t *obj);
void wing_obj_set_state(wing_obj_t *obj, uint16_t state);
uint16_t wing_obj_get_state(const wing_obj_t *obj);
void wing_obj_set_flags(wing_obj_t *obj, uint16_t flags);
uint16_t wing_obj_get_flags(const wing_obj_t *obj);
void wing_obj_set_visible(wing_obj_t *obj, bool visible);
bool wing_obj_is_visible(const wing_obj_t *obj);
void wing_obj_set_enabled(wing_obj_t *obj, bool enabled);
bool wing_obj_is_enabled(const wing_obj_t *obj);
void wing_obj_set_clip_children(wing_obj_t *obj, bool clip_children);
bool wing_obj_get_clip_children(const wing_obj_t *obj);
void wing_obj_set_selected(wing_obj_t *obj, bool selected);
bool wing_obj_is_selected(const wing_obj_t *obj);
void wing_obj_set_active(wing_obj_t *obj, bool active);
bool wing_obj_is_active(const wing_obj_t *obj);
void wing_obj_set_layout(wing_obj_t *obj, enum wing_layout_type_e layout,
                         uint8_t padding, uint8_t spacing);
void wing_obj_invalidate(wing_obj_t *obj);
int wing_obj_send_event(wing_obj_t *obj, enum wing_event_code_e code,
                        wing_context_t *ctx, void *data);
int wing_obj_bubble_event(wing_obj_t *target, enum wing_event_code_e code,
                          wing_context_t *ctx, void *data);
int wing_obj_layout_tree(wing_obj_t *root);
int wing_obj_draw_tree(wing_obj_t *root, wing_context_t *ctx,
                       const wing_rect_t *dirty);
wing_obj_t *wing_obj_hit_test(wing_obj_t *root, wing_point_t point);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_OBJECT_H */
