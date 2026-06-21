/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_tick.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TICK_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TICK_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct wing_gui_s;

typedef struct wing_gui_s wing_gui_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_gui_tick(wing_gui_t *gui, uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TICK_H */
