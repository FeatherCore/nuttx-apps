/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_dirty.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_DIRTY_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_DIRTY_H

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

void wing_gui_invalidate(wing_gui_t *gui);
void wing_gui_invalidate_rect(wing_gui_t *gui, const wing_rect_t *rect);
bool wing_gui_get_dirty_rect(const wing_gui_t *gui, wing_rect_t *rect);
uint8_t wing_gui_get_dirty_rect_count(const wing_gui_t *gui);
bool wing_gui_get_dirty_rect_at(const wing_gui_t *gui, uint8_t index,
                                wing_rect_t *rect);
uint16_t wing_gui_get_dirty_merge_count(const wing_gui_t *gui);
void wing_gui_clear_dirty(wing_gui_t *gui);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_DIRTY_H */
