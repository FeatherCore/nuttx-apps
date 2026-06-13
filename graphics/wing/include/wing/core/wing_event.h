/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_event.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_EVENT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_EVENT_H

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

int wing_gui_post_event(wing_gui_t *gui, wing_obj_t *target,
                        enum wing_event_code_e code, wing_context_t *ctx,
                        void *data);
int wing_gui_dispatch_events(wing_gui_t *gui);
void wing_event_stop_propagation(wing_event_t *event);
bool wing_event_is_stopped(const wing_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_EVENT_H */
