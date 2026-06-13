/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_font.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FONT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FONT_H

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

const wing_font_t *wing_font_builtin_5x7(void);
int wing_text_next_codepoint(const char **text, uint32_t *codepoint);
bool wing_font_get_glyph(const wing_font_t *font, uint32_t codepoint,
                         wing_bitmap_glyph_t *glyph);
int wing_font_measure_text(const wing_font_t *font, const char *text,
                           uint8_t scale, uint16_t *width,
                           uint16_t *height);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_FONT_H */
