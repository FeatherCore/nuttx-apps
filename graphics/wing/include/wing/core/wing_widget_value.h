/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_widget_value.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_VALUE_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_VALUE_H

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

bool wing_widget_update_value(wing_obj_t *obj, wing_value_model_t *value,
                              uint16_t next);
bool wing_widget_update_bool(wing_obj_t *obj, bool *value, bool next);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_WIDGET_VALUE_H */
