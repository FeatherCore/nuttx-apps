/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_switch.c
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

static int wing_switch_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  wing_switch_t *sw;
  const wing_box_style_t *track_style;
  const wing_rect_t *bounds;
  wing_rect_t knob;
  bool checked;
  uint8_t knob_size;
  uint8_t padding;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  sw = (wing_switch_t *)wing_obj_get_user_data(obj);
  if (sw == NULL)
    {
      return -EINVAL;
    }

  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL)
    {
      return -EINVAL;
    }

  checked = (wing_obj_get_state(obj) & WING_OBJ_STATE_CHECKED) != 0;
  track_style = checked ? &sw->on_style : &sw->off_style;
  wing_widget_draw_style_for_obj(ctx, obj, bounds, track_style);

  padding = sw->padding;
  if (bounds->w <= (uint16_t)(padding * 2) ||
      bounds->h <= (uint16_t)(padding * 2))
    {
      return 0;
    }

  knob_size = sw->knob_size;
  if (knob_size == 0)
    {
      knob_size = (uint8_t)(bounds->h - padding * 2);
    }

  if (knob_size > (uint8_t)(bounds->h - padding * 2))
    {
      knob_size = (uint8_t)(bounds->h - padding * 2);
    }

  if (knob_size > (uint8_t)(bounds->w - padding * 2))
    {
      knob_size = (uint8_t)(bounds->w - padding * 2);
    }

  knob.y = (int16_t)(bounds->y + padding);
  knob.w = knob_size;
  knob.h = knob_size;
  if (checked)
    {
      knob.x = (int16_t)(bounds->x + bounds->w - padding - knob_size);
    }
  else
    {
      knob.x = (int16_t)(bounds->x + padding);
    }

  wing_widget_draw_style_for_obj(ctx, obj, &knob, &sw->knob_style);

  return 0;
}

static int wing_switch_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_switch_t *sw;
  bool handled;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  sw = (wing_switch_t *)wing_obj_get_user_data(obj);
  if (sw == NULL)
    {
      return -EINVAL;
    }

  handled = wing_widget_handle_pointer_lifecycle(obj, event);
  if (!handled && event->code == WING_EVENT_CLICK)
    {
      wing_switch_set_checked(sw, !sw->checked);
      wing_event_stop_propagation(event);
    }
  else if (!handled && event->code == WING_EVENT_KEY_DOWN)
    {
      (void)wing_widget_stop_activation_key(event);
    }

  if (sw->event != NULL)
    {
      return sw->event(sw, event, sw->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_switch_init(wing_switch_t *sw, const wing_rect_t *bounds,
                      const wing_box_style_t *off_style,
                      const wing_box_style_t *on_style,
                      const wing_box_style_t *knob_style, bool checked)
{
  if (sw == NULL)
    {
      return;
    }

  wing_widget_init_obj(&sw->obj, bounds, wing_switch_draw,
                       wing_switch_event, sw, true);

  wing_box_style_init(&sw->off_style);
  wing_box_style_init(&sw->on_style);
  wing_box_style_init(&sw->knob_style);

  if (off_style != NULL)
    {
      sw->off_style = *off_style;
    }

  if (on_style != NULL)
    {
      sw->on_style = *on_style;
    }

  if (knob_style != NULL)
    {
      sw->knob_style = *knob_style;
    }

  sw->event = NULL;
  sw->event_arg = NULL;
  sw->checked = checked;
  sw->padding = 0;
  sw->knob_size = 0;
  if (checked)
    {
      wing_obj_set_state(&sw->obj,
                         (uint16_t)(wing_obj_get_state(&sw->obj) |
                                    WING_OBJ_STATE_CHECKED));
    }
}

wing_obj_t *wing_switch_obj(wing_switch_t *sw)
{
  return sw == NULL ? NULL : &sw->obj;
}

void wing_switch_set_checked(wing_switch_t *sw, bool checked)
{
  wing_value_event_t payload;
  uint16_t state;

  if (sw == NULL)
    {
      return;
    }

  if (!wing_value_update_bool(&sw->checked, checked, &payload))
    {
      return;
    }

  state = wing_obj_get_state(&sw->obj);
  if (checked)
    {
      state |= WING_OBJ_STATE_CHECKED;
    }
  else
    {
      state &= (uint16_t)~WING_OBJ_STATE_CHECKED;
    }

  wing_obj_set_state(&sw->obj, state);
  (void)wing_obj_send_event(&sw->obj, WING_EVENT_VALUE_CHANGED, NULL,
                            &payload);
}

bool wing_switch_get_checked(const wing_switch_t *sw)
{
  return sw != NULL && sw->checked;
}

void wing_switch_set_off_style(wing_switch_t *sw,
                               const wing_box_style_t *style)
{
  if (sw == NULL || style == NULL)
    {
      return;
    }

  sw->off_style = *style;
  wing_obj_invalidate(&sw->obj);
}

void wing_switch_set_on_style(wing_switch_t *sw,
                              const wing_box_style_t *style)
{
  if (sw == NULL || style == NULL)
    {
      return;
    }

  sw->on_style = *style;
  wing_obj_invalidate(&sw->obj);
}

void wing_switch_set_knob_style(wing_switch_t *sw,
                                const wing_box_style_t *style)
{
  if (sw == NULL || style == NULL)
    {
      return;
    }

  sw->knob_style = *style;
  wing_obj_invalidate(&sw->obj);
}

void wing_switch_set_padding(wing_switch_t *sw, uint8_t padding)
{
  if (sw == NULL)
    {
      return;
    }

  sw->padding = padding;
  wing_obj_invalidate(&sw->obj);
}

void wing_switch_set_knob_size(wing_switch_t *sw, uint8_t knob_size)
{
  if (sw == NULL)
    {
      return;
    }

  sw->knob_size = knob_size;
  wing_obj_invalidate(&sw->obj);
}

void wing_switch_set_event_cb(wing_switch_t *sw,
                              wing_switch_event_fn_t event, void *arg)
{
  if (sw == NULL)
    {
      return;
    }

  sw->event = event;
  sw->event_arg = arg;
}
