/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_runtime.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RUNTIME_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RUNTIME_H

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

int wing_gui_create(wing_gui_t *gui, wing_surface_t *surface,
                    fr_command_list_t *commands,
                    wing_gui_render_fn_t render, void *render_arg);
void wing_gui_destroy(wing_gui_t *gui);
void wing_gui_set_theme(wing_gui_t *gui, const wing_theme_t *theme);
const wing_theme_t *wing_gui_get_theme(const wing_gui_t *gui);
void wing_gui_set_camera(wing_gui_t *gui, const wing_camera_t *camera);
const wing_camera_t *wing_gui_get_camera(const wing_gui_t *gui);
int wing_gui_set_root(wing_gui_t *gui, wing_obj_t *root);
void wing_gui_set_frame_interval(wing_gui_t *gui, uint32_t interval_ms);
void wing_gui_invalidate(wing_gui_t *gui);
void wing_gui_invalidate_rect(wing_gui_t *gui, const wing_rect_t *rect);
bool wing_gui_get_dirty_rect(const wing_gui_t *gui, wing_rect_t *rect);
uint8_t wing_gui_get_dirty_rect_count(const wing_gui_t *gui);
bool wing_gui_get_dirty_rect_at(const wing_gui_t *gui, uint8_t index,
                                wing_rect_t *rect);
uint16_t wing_gui_get_dirty_merge_count(const wing_gui_t *gui);
void wing_gui_request_stop(wing_gui_t *gui);
bool wing_gui_is_running(const wing_gui_t *gui);
int wing_gui_step(wing_gui_t *gui);
int wing_gui_handle(wing_gui_t *gui, uint32_t elapsed_ms,
                    wing_gui_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RUNTIME_H */
