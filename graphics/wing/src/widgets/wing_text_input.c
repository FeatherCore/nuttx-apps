/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_text_input.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <wing/core/wing_text_edit.h>
#include <wing/wing.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint16_t wing_text_input_measure_prefix(wing_text_input_t *input,
                                               uint16_t cursor)
{
  uint16_t prefix_height;
  uint16_t prefix_width;
  char *buffer;
  char saved;

  if (input == NULL)
    {
      return 0;
    }

  buffer = input->edit.buffer;
  if (buffer == NULL)
    {
      return 0;
    }

  if (cursor > wing_text_edit_get_length(&input->edit))
    {
      cursor = wing_text_edit_get_length(&input->edit);
    }

  prefix_width = 0;
  prefix_height = 0;
  saved = buffer[cursor];
  buffer[cursor] = '\0';
  (void)wing_label_get_text_size(&input->label, &prefix_width,
                                 &prefix_height);
  buffer[cursor] = saved;
  return prefix_width;
}

static bool wing_text_input_is_printable_key(uint16_t key)
{
  return key >= 32 && key < 127;
}

static void wing_text_input_sync_layout(wing_text_input_t *input)
{
  wing_rect_t cursor_bounds;
  wing_rect_t label_bounds;
  wing_rect_t selection_bounds;
  const wing_rect_t *bounds;
  uint16_t prefix_width;
  uint16_t selection_end;
  uint16_t selection_start;
  uint16_t selection_width;
  uint16_t cursor_index;

  if (input == NULL)
    {
      return;
    }

  bounds = wing_obj_get_bounds(wing_box_obj(&input->box));
  if (bounds == NULL)
    {
      return;
    }

  label_bounds.x = input->padding;
  label_bounds.y = 0;
  label_bounds.w = bounds->w > (uint16_t)(input->padding * 2) ?
                   (uint16_t)(bounds->w - input->padding * 2) : 0;
  label_bounds.h = bounds->h;
  wing_obj_set_bounds(wing_label_obj(&input->label), &label_bounds);

  cursor_index = wing_text_edit_get_cursor(&input->edit);
  prefix_width = wing_text_input_measure_prefix(input, cursor_index);

  cursor_bounds.x = (int16_t)(input->padding + prefix_width);
  cursor_bounds.y = 2;
  cursor_bounds.w = 1;
  cursor_bounds.h = bounds->h > 4 ? (uint16_t)(bounds->h - 4) : bounds->h;
  wing_obj_set_bounds(wing_box_obj(&input->cursor), &cursor_bounds);

  wing_text_edit_get_selection(&input->edit, &selection_start,
                               &selection_end);
  if (selection_start != selection_end)
    {
      prefix_width = wing_text_input_measure_prefix(input, selection_start);
      selection_width = (uint16_t)(
        wing_text_input_measure_prefix(input, selection_end) - prefix_width);
      selection_bounds.x = (int16_t)(input->padding + prefix_width);
      selection_bounds.y = 2;
      selection_bounds.w = selection_width > 0 ? selection_width : 1;
      selection_bounds.h = bounds->h > 4 ? (uint16_t)(bounds->h - 4) :
                                           bounds->h;
      wing_obj_set_bounds(wing_box_obj(&input->selection),
                          &selection_bounds);
      wing_obj_set_visible(wing_box_obj(&input->selection), true);
    }
  else
    {
      selection_bounds.x = input->padding;
      selection_bounds.y = 2;
      selection_bounds.w = 0;
      selection_bounds.h = bounds->h > 4 ? (uint16_t)(bounds->h - 4) :
                                           bounds->h;
      wing_obj_set_bounds(wing_box_obj(&input->selection),
                          &selection_bounds);
      wing_obj_set_visible(wing_box_obj(&input->selection), false);
    }
}

static void wing_text_input_emit_changed(wing_text_input_t *input,
                                         wing_event_t *event)
{
  (void)event;

  if (input == NULL)
    {
      return;
    }

  (void)wing_obj_send_event(wing_text_input_obj(input),
                            WING_EVENT_VALUE_CHANGED, NULL,
                            input->edit.buffer);
}

static void wing_text_input_sync_text(wing_text_input_t *input)
{
  if (input == NULL)
    {
      return;
    }

  wing_label_set_text(&input->label,
                      wing_text_edit_get_text(&input->edit));
  wing_text_input_sync_layout(input);
  wing_obj_invalidate(wing_text_input_obj(input));
}

static int wing_text_input_event(wing_obj_t *obj, wing_event_t *event)
{
  const wing_input_event_t *key;
  wing_text_input_t *input;
  bool changed;
  bool moved;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  input = (wing_text_input_t *)wing_obj_get_user_data(obj);
  if (input == NULL)
    {
      return -EINVAL;
    }

  changed = false;
  moved = false;
  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      wing_obj_set_visible(wing_box_obj(&input->cursor), true);
      wing_event_stop_propagation(event);
    }
  else if (event->code == WING_EVENT_FOCUS_LOST)
    {
      wing_obj_set_visible(wing_box_obj(&input->cursor), false);
      wing_event_stop_propagation(event);
    }
  else if (event->code == WING_EVENT_KEY_DOWN)
    {
      key = (const wing_input_event_t *)event->data;
      if (key == NULL)
        {
          return 0;
        }

      if (key->key == WING_KEY_LEFT)
        {
          moved = wing_text_edit_move_left(&input->edit);
        }
      else if (key->key == WING_KEY_RIGHT)
        {
          moved = wing_text_edit_move_right(&input->edit);
        }
      else if (key->key == WING_KEY_BACKSPACE)
        {
          changed = wing_text_edit_backspace(&input->edit);
        }
      else if (key->key == WING_KEY_DELETE)
        {
          changed = wing_text_edit_delete(&input->edit);
        }
      else if (wing_text_input_is_printable_key(key->key))
        {
          changed = wing_text_edit_insert_char(&input->edit, key->key);
        }

      if (changed || moved)
        {
          wing_text_input_sync_text(input);
        }

      if (changed)
        {
          wing_text_input_emit_changed(input, event);
        }

      wing_event_stop_propagation(event);
    }

  if (input->event != NULL)
    {
      return input->event(input, event, input->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_text_input_init(wing_text_input_t *input,
                          const wing_rect_t *bounds,
                          const wing_box_style_t *style,
                          const wing_box_style_t *cursor_style,
                          char *buffer, uint16_t capacity)
{
  wing_box_style_t local_cursor;
  wing_box_style_t local_selection;
  wing_color_t text_color;
  wing_rect_t child_bounds;
  uint16_t flags;

  if (input == NULL)
    {
      return;
    }

  memset(input, 0, sizeof(*input));
  wing_box_init(&input->box, bounds, style);
  wing_obj_set_user_data(wing_box_obj(&input->box), input);
  wing_obj_set_event_cb(wing_box_obj(&input->box), wing_text_input_event);
  flags = wing_obj_get_flags(wing_box_obj(&input->box));
  wing_obj_set_flags(wing_box_obj(&input->box),
                     (uint16_t)(flags | WING_OBJ_FLAG_FOCUSABLE));

  input->padding = 3;
  wing_text_edit_init(&input->edit, buffer, capacity);

  child_bounds.x = 0;
  child_bounds.y = 0;
  child_bounds.w = bounds != NULL ? bounds->w : 0;
  child_bounds.h = bounds != NULL ? bounds->h : 0;
  text_color = style != NULL ? style->stroke : (wing_color_t){0, 0, 0, 255};
  wing_label_init(&input->label, &child_bounds,
                  wing_text_edit_get_text(&input->edit),
                  text_color, 1);
  wing_label_set_text_mode(&input->label, WING_LABEL_TEXT_MODE_ELLIPSIS);
  wing_obj_set_enabled(wing_label_obj(&input->label), false);

  wing_box_style_init(&local_cursor);
  local_cursor.fill = text_color;
  local_cursor.has_fill = true;
  if (cursor_style != NULL)
    {
      local_cursor = *cursor_style;
    }

  wing_box_style_init(&local_selection);
  local_selection.fill = text_color;
  local_selection.fill.a = 96;
  local_selection.has_fill = true;

  wing_box_init(&input->selection, &child_bounds, &local_selection);
  wing_obj_set_enabled(wing_box_obj(&input->selection), false);
  wing_obj_set_visible(wing_box_obj(&input->selection), false);
  wing_box_init(&input->cursor, &child_bounds, &local_cursor);
  wing_obj_set_enabled(wing_box_obj(&input->cursor), false);
  wing_obj_set_visible(wing_box_obj(&input->cursor), false);
  (void)wing_obj_add_child(wing_box_obj(&input->box),
                           wing_box_obj(&input->selection));
  (void)wing_obj_add_child(wing_box_obj(&input->box),
                           wing_label_obj(&input->label));
  (void)wing_obj_add_child(wing_box_obj(&input->box),
                           wing_box_obj(&input->cursor));
  wing_text_input_sync_layout(input);
}

wing_obj_t *wing_text_input_obj(wing_text_input_t *input)
{
  return input != NULL ? wing_box_obj(&input->box) : NULL;
}

const char *wing_text_input_get_text(const wing_text_input_t *input)
{
  return input != NULL ? wing_text_edit_get_text(&input->edit) : "";
}

uint16_t wing_text_input_get_cursor(const wing_text_input_t *input)
{
  return input != NULL ? wing_text_edit_get_cursor(&input->edit) : 0;
}

bool wing_text_input_has_selection(const wing_text_input_t *input)
{
  return input != NULL && wing_text_edit_has_selection(&input->edit);
}

void wing_text_input_get_selection(const wing_text_input_t *input,
                                   uint16_t *start, uint16_t *end)
{
  if (input == NULL)
    {
      if (start != NULL)
        {
          *start = 0;
        }

      if (end != NULL)
        {
          *end = 0;
        }

      return;
    }

  wing_text_edit_get_selection(&input->edit, start, end);
}

void wing_text_input_set_text(wing_text_input_t *input, const char *text)
{
  if (input == NULL)
    {
      return;
    }

  if (wing_text_edit_set_text(&input->edit, text))
    {
      wing_text_input_sync_text(input);
    }
}

void wing_text_input_set_selection(wing_text_input_t *input,
                                   uint16_t start, uint16_t end)
{
  if (input == NULL)
    {
      return;
    }

  if (wing_text_edit_set_selection(&input->edit, start, end))
    {
      wing_text_input_sync_text(input);
    }
}

void wing_text_input_select_all(wing_text_input_t *input)
{
  if (input == NULL)
    {
      return;
    }

  if (wing_text_edit_select_all(&input->edit))
    {
      wing_text_input_sync_text(input);
    }
}

void wing_text_input_set_selection_style(wing_text_input_t *input,
                                         const wing_box_style_t *style)
{
  if (input == NULL || style == NULL)
    {
      return;
    }

  wing_box_set_style(&input->selection, style);
  wing_text_input_sync_layout(input);
  wing_obj_invalidate(wing_text_input_obj(input));
}

void wing_text_input_set_padding(wing_text_input_t *input, uint8_t padding)
{
  if (input == NULL)
    {
      return;
    }

  input->padding = padding;
  wing_text_input_sync_layout(input);
  wing_obj_invalidate(wing_text_input_obj(input));
}

void wing_text_input_set_event_cb(wing_text_input_t *input,
                                  wing_text_input_event_fn_t event,
                                  void *arg)
{
  if (input == NULL)
    {
      return;
    }

  input->event = event;
  input->event_arg = arg;
}
