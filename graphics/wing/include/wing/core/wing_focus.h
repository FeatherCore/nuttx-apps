/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_focus.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FOCUS_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FOCUS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_gui_set_focus(wing_gui_t *gui, wing_obj_t *obj);
int wing_gui_focus_next(wing_gui_t *gui, bool reverse);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FOCUS_H */
