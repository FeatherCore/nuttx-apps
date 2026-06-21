/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_timer.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TIMER_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TIMER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct wing_gui_s;

typedef struct wing_gui_s wing_gui_t;
typedef void (*wing_timer_cb_t)(wing_gui_t *gui, void *arg);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_gui_timer_start(wing_gui_t *gui, uint32_t period_ms, bool repeat,
                         wing_timer_cb_t callback, void *arg,
                         uint8_t *timer_id);
int wing_gui_timer_stop(wing_gui_t *gui, uint8_t timer_id);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TIMER_H */
