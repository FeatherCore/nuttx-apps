/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_theme.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_THEME_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_THEME_H

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

wing_color_t wing_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t wing_color_pack_rgba8888(wing_color_t color);

void wing_theme_init_default(wing_theme_t *theme);
void wing_theme_init_high_contrast(wing_theme_t *theme);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_THEME_H */
