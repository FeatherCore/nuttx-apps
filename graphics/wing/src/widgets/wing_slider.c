/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_slider.c
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
#include "internal/wing_value_input.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint16_t wing_slider_point_to_value(const wing_slider_t *slider,
                                           const wing_rect_t *bounds,
                                           int16_t point_x)
{
  uint16_t offset;
  uint16_t inner_width;
  int16_t left;

  if (bounds->w <= (uint16_t)(slider->padding * 2))
    {
      return slider->value.min;
    }

  left = (int16_t)(bounds->x + slider->padding);
  inner_width = (uint16_t)(bounds->w - slider->padding * 2);

  if (point_x <= left)
    {
      offset = 0;
    }
  else if (point_x >= (int16_t)(left + inner_width))
    {
      offset = inner_width;
    }
  else
    {
      offset = (uint16_t)(point_x - left);
    }

  return wing_value_model_from_offset(&slider->value, offset, inner_width);
}

static void wing_slider_apply_value(void *arg, uint16_t value)
{
  wing_slider_set_value((wing_slider_t *)arg, value);
}

static void wing_slider_apply_pointer(void *arg,
                                      const wing_input_event_t *input)
{
  wing_slider_t *slider;
  const wing_rect_t *bounds;

  slider = (wing_slider_t *)arg;
  if (slider == NULL || input == NULL)
    {
      return;
    }

  bounds = wing_obj_get_bounds(&slider->obj);
  if (bounds != NULL)
    {
      wing_slider_set_value(
        slider,
        wing_slider_point_to_value(slider, bounds, input->point.x));
    }
}

static int wing_slider_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  wing_slider_t *slider;
  const wing_rect_t *bounds;
  const wing_box_style_t *state_style;
  wing_rect_t fill;
  wing_rect_t knob;
  wing_rect_t track;
  uint16_t offset;
  uint8_t knob_size;
  uint8_t track_height;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  slider = (wing_slider_t *)wing_obj_get_user_data(obj);
  if (slider == NULL)
    {
      return -EINVAL;
    }

  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL || bounds->w <= (uint16_t)(slider->padding * 2) ||
      bounds->h <= (uint16_t)(slider->padding * 2))
    {
      return 0;
    }

  track_height = slider->track_height;
  if (track_height == 0)
    {
      track_height = (uint8_t)(bounds->h / 4);
    }

  if (track_height == 0)
    {
      track_height = 1;
    }

  if (track_height > bounds->h)
    {
      track_height = (uint8_t)bounds->h;
    }

  track.x = (int16_t)(bounds->x + slider->padding);
  track.y = (int16_t)(bounds->y + (bounds->h - track_height) / 2);
  track.w = (uint16_t)(bounds->w - slider->padding * 2);
  track.h = track_height;

  state_style = wing_widget_select_style(obj, NULL,
                                         &slider->state_style);
  if (state_style != NULL)
    {
      wing_widget_draw_style_background_for_obj(ctx, obj, bounds,
                                                state_style);
    }

  wing_widget_draw_style_for_obj(ctx, obj, &track, &slider->track_style);

  offset = wing_value_model_to_offset(&slider->value, track.w);
  fill = track;
  fill.w = offset;
  if (fill.w > 0)
    {
      wing_widget_draw_style_for_obj(ctx, obj, &fill,
                                     &slider->fill_style);
    }

  knob_size = slider->knob_size;
  if (knob_size == 0)
    {
      knob_size = (uint8_t)(bounds->h - slider->padding * 2);
      if (knob_size == 0)
        {
          knob_size = 1;
        }
    }

  knob.x = (int16_t)(track.x + offset - knob_size / 2);
  knob.y = (int16_t)(bounds->y + (bounds->h - knob_size) / 2);
  knob.w = knob_size;
  knob.h = knob_size;

  if (knob.x < bounds->x)
    {
      knob.x = bounds->x;
    }

  if ((int16_t)(knob.x + knob.w) > (int16_t)(bounds->x + bounds->w))
    {
      knob.x = (int16_t)(bounds->x + bounds->w - knob.w);
    }

  wing_widget_draw_style_for_obj(ctx, obj, &knob, &slider->knob_style);

  if (state_style != NULL)
    {
      wing_widget_draw_style_overlay_for_obj(ctx, obj, bounds,
                                             state_style);
    }

  return 0;
}

static int wing_slider_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_slider_t *slider;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  slider = (wing_slider_t *)wing_obj_get_user_data(obj);
  if (slider == NULL)
    {
      return -EINVAL;
    }

  if (event->code == WING_EVENT_POINTER_DOWN ||
      event->code == WING_EVENT_POINTER_MOVE ||
      event->code == WING_EVENT_POINTER_UP)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_slider_apply_pointer, slider);
    }
  else if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_slider_apply_pointer, slider);
    }

  if (event->code == WING_EVENT_CLICK)
    {
      wing_event_stop_propagation(event);
    }
  else if (event->code == WING_EVENT_KEY_DOWN)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &slider->value, event, wing_slider_apply_value, slider,
        WING_WIDGET_VALUE_KEYMAP_UP_INCREASE);
    }
  else if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &slider->value, event, wing_slider_apply_value, slider,
        WING_WIDGET_VALUE_KEYMAP_UP_INCREASE);
    }

  if (slider->event != NULL)
    {
      return slider->event(slider, event, slider->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_slider_init(wing_slider_t *slider,
                      const wing_rect_t *bounds,
                      const wing_box_style_t *track_style,
                      const wing_box_style_t *fill_style,
                      const wing_box_style_t *knob_style,
                      uint16_t min, uint16_t max, uint16_t value)
{
  if (slider == NULL)
    {
      return;
    }

  wing_widget_init_obj(&slider->obj, bounds, wing_slider_draw,
                       wing_slider_event, slider, true);

  wing_box_style_init(&slider->track_style);
  wing_box_style_init(&slider->fill_style);
  wing_box_style_init(&slider->knob_style);
  wing_widget_state_style_init(&slider->state_style);

  if (track_style != NULL)
    {
      slider->track_style = *track_style;
    }

  if (fill_style != NULL)
    {
      slider->fill_style = *fill_style;
    }

  if (knob_style != NULL)
    {
      slider->knob_style = *knob_style;
    }

  slider->event = NULL;
  slider->event_arg = NULL;
  wing_value_model_init(&slider->value, min, max, value, 5);
  slider->padding = 0;
  slider->knob_size = 0;
  slider->track_height = 0;
}

wing_obj_t *wing_slider_obj(wing_slider_t *slider)
{
  return slider == NULL ? NULL : &slider->obj;
}

void wing_slider_set_value(wing_slider_t *slider, uint16_t value)
{
  if (slider == NULL)
    {
      return;
    }

  (void)wing_widget_update_value(&slider->obj, &slider->value, value);
}

uint16_t wing_slider_get_value(const wing_slider_t *slider)
{
  return slider == NULL ? 0 : slider->value.value;
}

void wing_slider_get_range(const wing_slider_t *slider,
                           uint16_t *min, uint16_t *max)
{
  if (min != NULL)
    {
      *min = slider == NULL ? 0 : slider->value.min;
    }

  if (max != NULL)
    {
      *max = slider == NULL ? 0 : slider->value.max;
    }
}

void wing_slider_set_range(wing_slider_t *slider,
                           uint16_t min, uint16_t max)
{
  if (slider == NULL)
    {
      return;
    }

  wing_value_model_set_range(&slider->value, min, max);
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_step(wing_slider_t *slider, uint16_t step)
{
  if (slider == NULL)
    {
      return;
    }

  wing_value_model_set_step(&slider->value, step);
}

uint16_t wing_slider_get_step(const wing_slider_t *slider)
{
  return slider == NULL ? 0 : slider->value.step;
}

void wing_slider_set_track_style(wing_slider_t *slider,
                                 const wing_box_style_t *style)
{
  if (slider == NULL || style == NULL)
    {
      return;
    }

  slider->track_style = *style;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_fill_style(wing_slider_t *slider,
                                const wing_box_style_t *style)
{
  if (slider == NULL || style == NULL)
    {
      return;
    }

  slider->fill_style = *style;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_knob_style(wing_slider_t *slider,
                                const wing_box_style_t *style)
{
  if (slider == NULL || style == NULL)
    {
      return;
    }

  slider->knob_style = *style;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_state_style(wing_slider_t *slider, uint16_t state,
                                 const wing_box_style_t *style)
{
  if (slider == NULL || style == NULL)
    {
      return;
    }

  (void)wing_widget_set_state_style(&slider->obj, &slider->state_style,
                                    state, style);
}

void wing_slider_set_padding(wing_slider_t *slider, uint8_t padding)
{
  if (slider == NULL)
    {
      return;
    }

  slider->padding = padding;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_knob_size(wing_slider_t *slider, uint8_t knob_size)
{
  if (slider == NULL)
    {
      return;
    }

  slider->knob_size = knob_size;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_track_height(wing_slider_t *slider,
                                  uint8_t track_height)
{
  if (slider == NULL)
    {
      return;
    }

  slider->track_height = track_height;
  wing_obj_invalidate(&slider->obj);
}

void wing_slider_set_event_cb(wing_slider_t *slider,
                              wing_slider_event_fn_t event, void *arg)
{
  if (slider == NULL)
    {
      return;
    }

  slider->event = event;
  slider->event_arg = arg;
}
