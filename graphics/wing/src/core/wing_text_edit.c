/****************************************************************************
 * apps/graphics/wing/src/core/wing_text_edit.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <string.h>

#include <wing/core/wing_text_edit.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint16_t wing_text_edit_min(uint16_t a, uint16_t b)
{
  return a < b ? a : b;
}

static uint16_t wing_text_edit_max(uint16_t a, uint16_t b)
{
  return a > b ? a : b;
}

static bool wing_text_edit_delete_selection(wing_text_edit_t *edit)
{
  uint16_t end;
  uint16_t start;
  uint16_t tail;

  if (!wing_text_edit_has_selection(edit) || edit->buffer == NULL)
    {
      return false;
    }

  start = wing_text_edit_min(edit->selection_start, edit->selection_end);
  end = wing_text_edit_max(edit->selection_start, edit->selection_end);
  tail = (uint16_t)(edit->length - end + 1);
  memmove(&edit->buffer[start], &edit->buffer[end], tail);
  edit->length = (uint16_t)(edit->length - (end - start));
  edit->cursor_index = start;
  edit->selection_start = start;
  edit->selection_end = start;
  return true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_text_edit_init(wing_text_edit_t *edit, char *buffer,
                         uint16_t capacity)
{
  if (edit == NULL)
    {
      return;
    }

  memset(edit, 0, sizeof(*edit));
  edit->buffer = buffer;
  edit->capacity = capacity;

  if (buffer != NULL && capacity > 0)
    {
      buffer[capacity - 1] = '\0';
      edit->length = (uint16_t)strlen(buffer);
      edit->cursor_index = edit->length;
      edit->selection_start = edit->cursor_index;
      edit->selection_end = edit->cursor_index;
    }
}

const char *wing_text_edit_get_text(const wing_text_edit_t *edit)
{
  return edit != NULL && edit->buffer != NULL ? edit->buffer : "";
}

uint16_t wing_text_edit_get_capacity(const wing_text_edit_t *edit)
{
  return edit != NULL ? edit->capacity : 0;
}

uint16_t wing_text_edit_get_length(const wing_text_edit_t *edit)
{
  return edit != NULL ? edit->length : 0;
}

uint16_t wing_text_edit_get_cursor(const wing_text_edit_t *edit)
{
  return edit != NULL ? edit->cursor_index : 0;
}

bool wing_text_edit_has_selection(const wing_text_edit_t *edit)
{
  return edit != NULL && edit->selection_start != edit->selection_end;
}

void wing_text_edit_get_selection(const wing_text_edit_t *edit,
                                  uint16_t *start, uint16_t *end)
{
  uint16_t local_end;
  uint16_t local_start;

  local_start = 0;
  local_end = 0;
  if (edit != NULL)
    {
      local_start = wing_text_edit_min(edit->selection_start,
                                       edit->selection_end);
      local_end = wing_text_edit_max(edit->selection_start,
                                     edit->selection_end);
    }

  if (start != NULL)
    {
      *start = local_start;
    }

  if (end != NULL)
    {
      *end = local_end;
    }
}

bool wing_text_edit_set_text(wing_text_edit_t *edit, const char *text)
{
  if (edit == NULL || edit->buffer == NULL || edit->capacity == 0)
    {
      return false;
    }

  if (text == NULL)
    {
      text = "";
    }

  strncpy(edit->buffer, text, edit->capacity - 1);
  edit->buffer[edit->capacity - 1] = '\0';
  edit->length = (uint16_t)strlen(edit->buffer);
  edit->cursor_index = edit->length;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_set_cursor(wing_text_edit_t *edit, uint16_t cursor)
{
  if (edit == NULL)
    {
      return false;
    }

  if (cursor > edit->length)
    {
      cursor = edit->length;
    }

  if (edit->cursor_index == cursor && !wing_text_edit_has_selection(edit))
    {
      return false;
    }

  edit->cursor_index = cursor;
  edit->selection_start = cursor;
  edit->selection_end = cursor;
  return true;
}

bool wing_text_edit_set_selection(wing_text_edit_t *edit, uint16_t start,
                                  uint16_t end)
{
  if (edit == NULL)
    {
      return false;
    }

  if (start > edit->length)
    {
      start = edit->length;
    }

  if (end > edit->length)
    {
      end = edit->length;
    }

  if (edit->selection_start == start && edit->selection_end == end &&
      edit->cursor_index == end)
    {
      return false;
    }

  edit->selection_start = start;
  edit->selection_end = end;
  edit->cursor_index = end;
  return true;
}

bool wing_text_edit_select_all(wing_text_edit_t *edit)
{
  if (edit == NULL)
    {
      return false;
    }

  return wing_text_edit_set_selection(edit, 0, edit->length);
}

bool wing_text_edit_clear_selection(wing_text_edit_t *edit)
{
  if (!wing_text_edit_has_selection(edit))
    {
      return false;
    }

  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_move_left(wing_text_edit_t *edit)
{
  uint16_t start;

  if (edit == NULL)
    {
      return false;
    }

  if (wing_text_edit_has_selection(edit))
    {
      start = wing_text_edit_min(edit->selection_start,
                                 edit->selection_end);
      edit->cursor_index = start;
      edit->selection_start = start;
      edit->selection_end = start;
      return true;
    }

  if (edit->cursor_index == 0)
    {
      return false;
    }

  edit->cursor_index--;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_move_right(wing_text_edit_t *edit)
{
  uint16_t end;

  if (edit == NULL)
    {
      return false;
    }

  if (wing_text_edit_has_selection(edit))
    {
      end = wing_text_edit_max(edit->selection_start, edit->selection_end);
      edit->cursor_index = end;
      edit->selection_start = end;
      edit->selection_end = end;
      return true;
    }

  if (edit->cursor_index >= edit->length)
    {
      return false;
    }

  edit->cursor_index++;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_insert_char(wing_text_edit_t *edit, uint16_t key)
{
  uint16_t selected;
  uint16_t tail;

  if (edit == NULL || edit->buffer == NULL || edit->capacity == 0 ||
      key < 32 || key >= 127)
    {
      return false;
    }

  selected = 0;
  if (wing_text_edit_has_selection(edit))
    {
      selected = (uint16_t)(wing_text_edit_max(edit->selection_start,
                                               edit->selection_end) -
                            wing_text_edit_min(edit->selection_start,
                                               edit->selection_end));
    }

  if ((uint16_t)(edit->length - selected + 1) >= edit->capacity)
    {
      return false;
    }

  (void)wing_text_edit_delete_selection(edit);
  tail = (uint16_t)(edit->length - edit->cursor_index + 1);
  memmove(&edit->buffer[edit->cursor_index + 1],
          &edit->buffer[edit->cursor_index], tail);
  edit->buffer[edit->cursor_index] = (char)key;
  edit->cursor_index++;
  edit->length++;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_backspace(wing_text_edit_t *edit)
{
  uint16_t tail;

  if (edit == NULL || edit->buffer == NULL)
    {
      return false;
    }

  if (wing_text_edit_delete_selection(edit))
    {
      return true;
    }

  if (edit->cursor_index == 0)
    {
      return false;
    }

  tail = (uint16_t)(edit->length - edit->cursor_index + 1);
  memmove(&edit->buffer[edit->cursor_index - 1],
          &edit->buffer[edit->cursor_index], tail);
  edit->cursor_index--;
  edit->length--;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}

bool wing_text_edit_delete(wing_text_edit_t *edit)
{
  uint16_t tail;

  if (edit == NULL || edit->buffer == NULL)
    {
      return false;
    }

  if (wing_text_edit_delete_selection(edit))
    {
      return true;
    }

  if (edit->cursor_index >= edit->length)
    {
      return false;
    }

  tail = (uint16_t)(edit->length - edit->cursor_index);
  memmove(&edit->buffer[edit->cursor_index],
          &edit->buffer[edit->cursor_index + 1], tail);
  edit->length--;
  edit->selection_start = edit->cursor_index;
  edit->selection_end = edit->cursor_index;
  return true;
}
