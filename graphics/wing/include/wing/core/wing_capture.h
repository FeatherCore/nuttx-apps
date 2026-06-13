/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_capture.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_CAPTURE_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_CAPTURE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_gui_capture_pointer(wing_gui_t *gui, wing_obj_t *obj);
int wing_gui_release_pointer(wing_gui_t *gui, wing_obj_t *obj);
int wing_gui_cancel_pointer_capture(wing_gui_t *gui);
wing_obj_t *wing_gui_get_pointer_capture(const wing_gui_t *gui);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_CAPTURE_H */
