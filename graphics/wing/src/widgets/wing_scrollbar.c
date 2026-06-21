/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_scrollbar.c
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

static uint16_t wing_scrollbar_track_length(const wing_scrollbar_t *scrollbar,
                                            const wing_rect_t *track)
{
  return scrollbar->axis == WING_AXIS_VERTICAL ? track->h : track->w;
}

static uint16_t wing_scrollbar_thumb_length(const wing_scrollbar_t *scrollbar,
                                            uint16_t track_length)
{
  uint32_t range;
  uint32_t visible;
  uint32_t total;
  uint16_t thumb;

  range = (uint32_t)(scrollbar->value.max - scrollbar->value.min);
  visible = scrollbar->page_size == 0 ? 1 : scrollbar->page_size;
  total = range + visible;
  if (total == 0)
    {
      total = 1;
    }

  thumb = (uint16_t)((uint32_t)track_length * visible / total);
  if (thumb < scrollbar->min_thumb_length &&
      track_length >= scrollbar->min_thumb_length)
    {
      thumb = scrollbar->min_thumb_length;
    }

  if (thumb > track_length)
    {
      thumb = track_length;
    }

  return thumb;
}

static uint16_t wing_scrollbar_thumb_offset(const wing_scrollbar_t *scrollbar,
                                            uint16_t track_length,
                                            uint16_t thumb_length)
{
  uint16_t travel;

  if (track_length <= thumb_length)
    {
      return 0;
    }

  travel = (uint16_t)(track_length - thumb_length);
  return wing_value_model_to_offset(&scrollbar->value, travel);
}

static uint16_t wing_scrollbar_point_to_value(
  const wing_scrollbar_t *scrollbar, const wing_rect_t *track,
  int16_t point)
{
  uint16_t thumb_length;
  uint16_t track_length;
  uint16_t travel;
  int16_t origin;
  int32_t offset;

  track_length = wing_scrollbar_track_length(scrollbar, track);
  thumb_length = wing_scrollbar_thumb_length(scrollbar, track_length);
  if (track_length <= thumb_length)
    {
      return scrollbar->value.min;
    }

  origin = scrollbar->axis == WING_AXIS_VERTICAL ? track->y : track->x;
  travel = (uint16_t)(track_length - thumb_length);
  offset = (int32_t)point - origin - thumb_length / 2;
  if (offset < 0)
    {
      offset = 0;
    }

  if (offset > travel)
    {
      offset = travel;
    }

  return wing_value_model_from_offset(&scrollbar->value, (uint16_t)offset,
                                      travel);
}

static bool wing_scrollbar_make_track(const wing_scrollbar_t *scrollbar,
                                      const wing_rect_t *bounds,
                                      wing_rect_t *track);

static void wing_scrollbar_apply_value(void *arg, uint16_t value)
{
  wing_scrollbar_set_value((wing_scrollbar_t *)arg, value);
}

static void wing_scrollbar_apply_pointer(void *arg,
                                         const wing_input_event_t *input)
{
  wing_scrollbar_t *scrollbar;
  const wing_rect_t *bounds;
  wing_rect_t track;

  scrollbar = (wing_scrollbar_t *)arg;
  if (scrollbar == NULL || input == NULL)
    {
      return;
    }

  bounds = wing_obj_get_bounds(&scrollbar->obj);
  if (bounds != NULL && wing_scrollbar_make_track(scrollbar, bounds,
                                                  &track))
    {
      wing_scrollbar_set_value(
        scrollbar,
        wing_scrollbar_point_to_value(
          scrollbar, &track,
          scrollbar->axis == WING_AXIS_VERTICAL ? input->point.y :
                                                  input->point.x));
    }
}

static bool wing_scrollbar_make_track(const wing_scrollbar_t *scrollbar,
                                      const wing_rect_t *bounds,
                                      wing_rect_t *track)
{
  uint8_t padding;

  padding = scrollbar->padding;
  if (bounds->w <= (uint16_t)(padding * 2) ||
      bounds->h <= (uint16_t)(padding * 2))
    {
      return false;
    }

  track->x = (int16_t)(bounds->x + padding);
  track->y = (int16_t)(bounds->y + padding);
  track->w = (uint16_t)(bounds->w - padding * 2);
  track->h = (uint16_t)(bounds->h - padding * 2);
  return true;
}

static int wing_scrollbar_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  wing_scrollbar_t *scrollbar;
  const wing_rect_t *bounds;
  const wing_box_style_t *state_style;
  wing_rect_t thumb;
  wing_rect_t track;
  uint16_t offset;
  uint16_t thumb_length;
  uint16_t track_length;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  scrollbar = (wing_scrollbar_t *)wing_obj_get_user_data(obj);
  if (scrollbar == NULL)
    {
      return -EINVAL;
    }

  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL || !wing_scrollbar_make_track(scrollbar, bounds, &track))
    {
      return 0;
    }

  state_style = wing_widget_select_style(obj, NULL,
                                         &scrollbar->state_style);
  if (state_style != NULL)
    {
      wing_widget_draw_style_background_for_obj(ctx, obj, bounds,
                                                state_style);
    }

  wing_widget_draw_style_for_obj(ctx, obj, &track,
                                 &scrollbar->track_style);

  track_length = wing_scrollbar_track_length(scrollbar, &track);
  thumb_length = wing_scrollbar_thumb_length(scrollbar, track_length);
  offset = wing_scrollbar_thumb_offset(scrollbar, track_length,
                                       thumb_length);

  thumb = track;
  if (scrollbar->axis == WING_AXIS_VERTICAL)
    {
      thumb.y = (int16_t)(track.y + offset);
      thumb.h = thumb_length;
    }
  else
    {
      thumb.x = (int16_t)(track.x + offset);
      thumb.w = thumb_length;
    }

  wing_widget_draw_style_for_obj(ctx, obj, &thumb,
                                 &scrollbar->thumb_style);

  if (state_style != NULL)
    {
      wing_widget_draw_style_overlay_for_obj(ctx, obj, bounds,
                                             state_style);
    }

  return 0;
}

static int wing_scrollbar_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_scrollbar_t *scrollbar;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  scrollbar = (wing_scrollbar_t *)wing_obj_get_user_data(obj);
  if (scrollbar == NULL)
    {
      return -EINVAL;
    }

  if (event->code == WING_EVENT_POINTER_DOWN ||
      event->code == WING_EVENT_POINTER_MOVE ||
      event->code == WING_EVENT_POINTER_UP)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_scrollbar_apply_pointer, scrollbar);
    }
  else if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_scrollbar_apply_pointer, scrollbar);
    }

  if (event->code == WING_EVENT_CLICK)
    {
      wing_event_stop_propagation(event);
    }
  else if (event->code == WING_EVENT_KEY_DOWN)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &scrollbar->value, event, wing_scrollbar_apply_value,
        scrollbar, WING_WIDGET_VALUE_KEYMAP_DOWN_INCREASE);
    }
  else if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &scrollbar->value, event, wing_scrollbar_apply_value,
        scrollbar, WING_WIDGET_VALUE_KEYMAP_DOWN_INCREASE);
    }

  if (scrollbar->event != NULL)
    {
      return scrollbar->event(scrollbar, event, scrollbar->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_scrollbar_init(wing_scrollbar_t *scrollbar,
                         const wing_rect_t *bounds,
                         const wing_box_style_t *track_style,
                         const wing_box_style_t *thumb_style,
                         uint16_t min, uint16_t max, uint16_t value,
                         uint16_t page_size)
{
  if (scrollbar == NULL)
    {
      return;
    }

  wing_widget_init_obj(&scrollbar->obj, bounds, wing_scrollbar_draw,
                       wing_scrollbar_event, scrollbar, true);

  wing_box_style_init(&scrollbar->track_style);
  wing_box_style_init(&scrollbar->thumb_style);
  wing_widget_state_style_init(&scrollbar->state_style);

  if (track_style != NULL)
    {
      scrollbar->track_style = *track_style;
    }

  if (thumb_style != NULL)
    {
      scrollbar->thumb_style = *thumb_style;
    }

  scrollbar->event = NULL;
  scrollbar->event_arg = NULL;
  wing_value_model_init(&scrollbar->value, min, max, value, 5);
  scrollbar->page_size = page_size == 0 ? 1 : page_size;
  scrollbar->padding = 0;
  scrollbar->min_thumb_length = 1;
  scrollbar->axis = WING_AXIS_HORIZONTAL;
}

wing_obj_t *wing_scrollbar_obj(wing_scrollbar_t *scrollbar)
{
  return scrollbar == NULL ? NULL : &scrollbar->obj;
}

void wing_scrollbar_set_value(wing_scrollbar_t *scrollbar, uint16_t value)
{
  if (scrollbar == NULL)
    {
      return;
    }

  (void)wing_widget_update_value(&scrollbar->obj, &scrollbar->value, value);
}

uint16_t wing_scrollbar_get_value(const wing_scrollbar_t *scrollbar)
{
  return scrollbar == NULL ? 0 : scrollbar->value.value;
}

void wing_scrollbar_get_range(const wing_scrollbar_t *scrollbar,
                              uint16_t *min, uint16_t *max)
{
  if (min != NULL)
    {
      *min = scrollbar == NULL ? 0 : scrollbar->value.min;
    }

  if (max != NULL)
    {
      *max = scrollbar == NULL ? 0 : scrollbar->value.max;
    }
}

void wing_scrollbar_set_range(wing_scrollbar_t *scrollbar,
                              uint16_t min, uint16_t max)
{
  if (scrollbar == NULL)
    {
      return;
    }

  wing_value_model_set_range(&scrollbar->value, min, max);
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_page_size(wing_scrollbar_t *scrollbar,
                                  uint16_t page_size)
{
  if (scrollbar == NULL)
    {
      return;
    }

  scrollbar->page_size = page_size == 0 ? 1 : page_size;
  wing_obj_invalidate(&scrollbar->obj);
}

uint16_t wing_scrollbar_get_page_size(const wing_scrollbar_t *scrollbar)
{
  return scrollbar == NULL ? 0 : scrollbar->page_size;
}

void wing_scrollbar_set_step(wing_scrollbar_t *scrollbar, uint16_t step)
{
  if (scrollbar == NULL)
    {
      return;
    }

  wing_value_model_set_step(&scrollbar->value, step);
}

uint16_t wing_scrollbar_get_step(const wing_scrollbar_t *scrollbar)
{
  return scrollbar == NULL ? 0 : scrollbar->value.step;
}

void wing_scrollbar_set_track_style(wing_scrollbar_t *scrollbar,
                                    const wing_box_style_t *style)
{
  if (scrollbar == NULL || style == NULL)
    {
      return;
    }

  scrollbar->track_style = *style;
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_thumb_style(wing_scrollbar_t *scrollbar,
                                    const wing_box_style_t *style)
{
  if (scrollbar == NULL || style == NULL)
    {
      return;
    }

  scrollbar->thumb_style = *style;
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_state_style(wing_scrollbar_t *scrollbar,
                                    uint16_t state,
                                    const wing_box_style_t *style)
{
  if (scrollbar == NULL || style == NULL)
    {
      return;
    }

  (void)wing_widget_set_state_style(&scrollbar->obj,
                                    &scrollbar->state_style, state,
                                    style);
}

void wing_scrollbar_set_padding(wing_scrollbar_t *scrollbar,
                                uint8_t padding)
{
  if (scrollbar == NULL)
    {
      return;
    }

  scrollbar->padding = padding;
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_min_thumb_length(
  wing_scrollbar_t *scrollbar, uint8_t min_thumb_length)
{
  if (scrollbar == NULL)
    {
      return;
    }

  scrollbar->min_thumb_length = min_thumb_length == 0 ? 1 :
                                 min_thumb_length;
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_axis(wing_scrollbar_t *scrollbar,
                             enum wing_axis_e axis)
{
  if (scrollbar == NULL)
    {
      return;
    }

  scrollbar->axis = axis;
  wing_obj_invalidate(&scrollbar->obj);
}

void wing_scrollbar_set_event_cb(wing_scrollbar_t *scrollbar,
                                 wing_scrollbar_event_fn_t event,
                                 void *arg)
{
  if (scrollbar == NULL)
    {
      return;
    }

  scrollbar->event = event;
  scrollbar->event_arg = arg;
}
