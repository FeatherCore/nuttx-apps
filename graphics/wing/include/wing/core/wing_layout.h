/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_layout.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_LAYOUT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_LAYOUT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

#include <wing/wing.h>

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void wing_obj_set_layout(wing_obj_t *obj, enum wing_layout_type_e layout,
                         uint8_t padding, uint8_t spacing);
int wing_obj_layout_tree(wing_obj_t *root);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_LAYOUT_H */
