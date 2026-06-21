/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_widget_input.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_INPUT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_INPUT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>

#include <wing/wing.h>

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void wing_widget_set_pressed(wing_obj_t *obj, bool pressed);
bool wing_widget_handle_pointer_lifecycle(wing_obj_t *obj,
                                           wing_event_t *event);
bool wing_widget_stop_activation_key(wing_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_INPUT_H */
