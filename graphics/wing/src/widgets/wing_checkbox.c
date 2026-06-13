/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_checkbox.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>

#include <wing/wing.h>

#include "../core/wing_value.h"
#include "internal/wing_widget.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_checkbox_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  wing_checkbox_t *checkbox;
  const wing_box_style_t *box_style;
  const wing_rect_t *bounds;
  wing_rect_t mark;
  bool checked;
  uint8_t padding;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  checkbox = (wing_checkbox_t *)wing_obj_get_user_data(obj);
  if (checkbox == NULL)
    {
      return -EINVAL;
    }

  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL)
    {
      return -EINVAL;
    }

  checked = (wing_obj_get_state(obj) & WING_OBJ_STATE_CHECKED) != 0;
  box_style = checked ? &checkbox->checked_style : &checkbox->box_style;

  wing_widget_draw_style_for_obj(ctx, obj, bounds, box_style);

  if (!checked)
    {
      return 0;
    }

  padding = checkbox->padding;
  if (bounds->w <= (uint16_t)(padding * 2) ||
      bounds->h <= (uint16_t)(padding * 2))
    {
      return 0;
    }

  mark.x = (int16_t)(bounds->x + padding);
  mark.y = (int16_t)(bounds->y + padding);
  mark.w = (uint16_t)(bounds->w - padding * 2);
  mark.h = (uint16_t)(bounds->h - padding * 2);

  wing_widget_draw_style_for_obj(ctx, obj, &mark, &checkbox->mark_style);

  return 0;
}

static int wing_checkbox_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_checkbox_t *checkbox;
  bool handled;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  checkbox = (wing_checkbox_t *)wing_obj_get_user_data(obj);
  if (checkbox == NULL)
    {
      return -EINVAL;
    }

  handled = wing_widget_handle_pointer_lifecycle(obj, event);
  if (!handled && event->code == WING_EVENT_CLICK)
    {
      wing_checkbox_set_checked(checkbox, !checkbox->checked);
      wing_event_stop_propagation(event);
    }
  else if (!handled && event->code == WING_EVENT_KEY_DOWN)
    {
      (void)wing_widget_stop_activation_key(event);
    }

  if (checkbox->event != NULL)
    {
      return checkbox->event(checkbox, event, checkbox->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_checkbox_init(wing_checkbox_t *checkbox,
                        const wing_rect_t *bounds,
                        const wing_box_style_t *box_style,
                        const wing_box_style_t *checked_style,
                        const wing_box_style_t *mark_style,
                        bool checked)
{
  if (checkbox == NULL)
    {
      return;
    }

  wing_widget_init_obj(&checkbox->obj, bounds, wing_checkbox_draw,
                       wing_checkbox_event, checkbox, true);

  wing_box_style_init(&checkbox->box_style);
  wing_box_style_init(&checkbox->checked_style);
  wing_box_style_init(&checkbox->mark_style);

  if (box_style != NULL)
    {
      checkbox->box_style = *box_style;
    }

  if (checked_style != NULL)
    {
      checkbox->checked_style = *checked_style;
    }

  if (mark_style != NULL)
    {
      checkbox->mark_style = *mark_style;
    }

  checkbox->event = NULL;
  checkbox->event_arg = NULL;
  checkbox->checked = checked;
  checkbox->padding = 0;
  if (checked)
    {
      wing_obj_set_state(&checkbox->obj,
                         (uint16_t)(wing_obj_get_state(&checkbox->obj) |
                                    WING_OBJ_STATE_CHECKED));
    }
}

wing_obj_t *wing_checkbox_obj(wing_checkbox_t *checkbox)
{
  return checkbox == NULL ? NULL : &checkbox->obj;
}

void wing_checkbox_set_checked(wing_checkbox_t *checkbox, bool checked)
{
  wing_value_event_t payload;
  uint16_t state;

  if (checkbox == NULL)
    {
      return;
    }

  if (!wing_value_update_bool(&checkbox->checked, checked, &payload))
    {
      return;
    }

  state = wing_obj_get_state(&checkbox->obj);
  if (checked)
    {
      state |= WING_OBJ_STATE_CHECKED;
    }
  else
    {
      state &= (uint16_t)~WING_OBJ_STATE_CHECKED;
    }

  wing_obj_set_state(&checkbox->obj, state);
  (void)wing_obj_send_event(&checkbox->obj, WING_EVENT_VALUE_CHANGED,
                            NULL, &payload);
}

bool wing_checkbox_get_checked(const wing_checkbox_t *checkbox)
{
  return checkbox != NULL && checkbox->checked;
}

void wing_checkbox_set_box_style(wing_checkbox_t *checkbox,
                                 const wing_box_style_t *style)
{
  if (checkbox == NULL || style == NULL)
    {
      return;
    }

  checkbox->box_style = *style;
  wing_obj_invalidate(&checkbox->obj);
}

void wing_checkbox_set_checked_style(wing_checkbox_t *checkbox,
                                     const wing_box_style_t *style)
{
  if (checkbox == NULL || style == NULL)
    {
      return;
    }

  checkbox->checked_style = *style;
  wing_obj_invalidate(&checkbox->obj);
}

void wing_checkbox_set_mark_style(wing_checkbox_t *checkbox,
                                  const wing_box_style_t *style)
{
  if (checkbox == NULL || style == NULL)
    {
      return;
    }

  checkbox->mark_style = *style;
  wing_obj_invalidate(&checkbox->obj);
}

void wing_checkbox_set_padding(wing_checkbox_t *checkbox, uint8_t padding)
{
  if (checkbox == NULL)
    {
      return;
    }

  checkbox->padding = padding;
  wing_obj_invalidate(&checkbox->obj);
}

void wing_checkbox_set_event_cb(wing_checkbox_t *checkbox,
                                wing_checkbox_event_fn_t event, void *arg)
{
  if (checkbox == NULL)
    {
      return;
    }

  checkbox->event = event;
  checkbox->event_arg = arg;
}
