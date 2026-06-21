/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_text_edit.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TEXT_EDIT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TEXT_EDIT_H

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

void wing_text_edit_init(wing_text_edit_t *edit, char *buffer,
                         uint16_t capacity);
const char *wing_text_edit_get_text(const wing_text_edit_t *edit);
uint16_t wing_text_edit_get_capacity(const wing_text_edit_t *edit);
uint16_t wing_text_edit_get_length(const wing_text_edit_t *edit);
uint16_t wing_text_edit_get_cursor(const wing_text_edit_t *edit);
bool wing_text_edit_has_selection(const wing_text_edit_t *edit);
void wing_text_edit_get_selection(const wing_text_edit_t *edit,
                                  uint16_t *start, uint16_t *end);
bool wing_text_edit_set_text(wing_text_edit_t *edit, const char *text);
bool wing_text_edit_set_cursor(wing_text_edit_t *edit, uint16_t cursor);
bool wing_text_edit_set_selection(wing_text_edit_t *edit, uint16_t start,
                                  uint16_t end);
bool wing_text_edit_select_all(wing_text_edit_t *edit);
bool wing_text_edit_clear_selection(wing_text_edit_t *edit);
bool wing_text_edit_move_left(wing_text_edit_t *edit);
bool wing_text_edit_move_right(wing_text_edit_t *edit);
bool wing_text_edit_insert_char(wing_text_edit_t *edit, uint16_t key);
bool wing_text_edit_backspace(wing_text_edit_t *edit);
bool wing_text_edit_delete(wing_text_edit_t *edit);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_TEXT_EDIT_H */
