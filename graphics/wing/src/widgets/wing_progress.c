/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_progress.c
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

static uint16_t wing_progress_point_to_value(const wing_progress_t *progress,
                                             const wing_rect_t *bounds,
                                             int16_t point_x)
{
  uint16_t offset;
  uint16_t inner_width;
  int16_t left;

  if (bounds->w <= (uint16_t)(progress->padding * 2))
    {
      return progress->value.min;
    }

  left = (int16_t)(bounds->x + progress->padding);
  inner_width = (uint16_t)(bounds->w - progress->padding * 2);

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

  return wing_value_model_from_offset(&progress->value, offset,
                                      inner_width);
}

static void wing_progress_apply_value(void *arg, uint16_t value)
{
  wing_progress_set_value((wing_progress_t *)arg, value);
}

static void wing_progress_apply_pointer(void *arg,
                                        const wing_input_event_t *input)
{
  wing_progress_t *progress;
  const wing_rect_t *bounds;

  progress = (wing_progress_t *)arg;
  if (progress == NULL || input == NULL)
    {
      return;
    }

  bounds = wing_obj_get_bounds(&progress->obj);
  if (bounds != NULL)
    {
      wing_progress_set_value(
        progress,
        wing_progress_point_to_value(progress, bounds, input->point.x));
    }
}

static int wing_progress_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  wing_progress_t *progress;
  const wing_rect_t *bounds;
  wing_rect_t fill;
  wing_rect_t inner;
  uint8_t padding;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  progress = (wing_progress_t *)wing_obj_get_user_data(obj);
  if (progress == NULL)
    {
      return -EINVAL;
    }

  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL)
    {
      return -EINVAL;
    }

  wing_widget_draw_style_for_obj(ctx, obj, bounds,
                                 &progress->frame_style);

  padding = progress->padding;
  if (bounds->w <= (uint16_t)(padding * 2) ||
      bounds->h <= (uint16_t)(padding * 2))
    {
      return 0;
    }

  inner.x = (int16_t)(bounds->x + padding);
  inner.y = (int16_t)(bounds->y + padding);
  inner.w = (uint16_t)(bounds->w - padding * 2);
  inner.h = (uint16_t)(bounds->h - padding * 2);

  fill = inner;
  fill.w = wing_value_model_to_offset(&progress->value, inner.w);
  if (fill.w == 0)
    {
      return 0;
    }

  if (fill.w > 0)
    {
      wing_widget_draw_style_for_obj(ctx, obj, &fill,
                                     &progress->fill_style);
    }

  return 0;
}

static int wing_progress_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_progress_t *progress;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  progress = (wing_progress_t *)wing_obj_get_user_data(obj);
  if (progress == NULL)
    {
      return -EINVAL;
    }

  if (event->code == WING_EVENT_POINTER_DOWN ||
      event->code == WING_EVENT_POINTER_MOVE ||
      event->code == WING_EVENT_POINTER_UP)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_progress_apply_pointer, progress);
    }
  else if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      (void)wing_widget_value_handle_pointer_drag(
        obj, event, wing_progress_apply_pointer, progress);
    }

  if (event->code == WING_EVENT_CLICK)
    {
      wing_event_stop_propagation(event);
    }
  else if (event->code == WING_EVENT_KEY_DOWN)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &progress->value, event, wing_progress_apply_value, progress,
        WING_WIDGET_VALUE_KEYMAP_UP_INCREASE);
    }
  else if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      (void)wing_widget_value_handle_step_input(
        obj, &progress->value, event, wing_progress_apply_value, progress,
        WING_WIDGET_VALUE_KEYMAP_UP_INCREASE);
    }

  if (progress->event != NULL)
    {
      return progress->event(progress, event, progress->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_progress_init(wing_progress_t *progress,
                        const wing_rect_t *bounds,
                        const wing_box_style_t *frame_style,
                        const wing_box_style_t *fill_style,
                        uint16_t min, uint16_t max, uint16_t value)
{
  if (progress == NULL)
    {
      return;
    }

  wing_widget_init_obj(&progress->obj, bounds, wing_progress_draw,
                       wing_progress_event, progress, true);

  wing_box_style_init(&progress->frame_style);
  wing_box_style_init(&progress->fill_style);

  if (frame_style != NULL)
    {
      progress->frame_style = *frame_style;
    }

  if (fill_style != NULL)
    {
      progress->fill_style = *fill_style;
    }

  wing_value_model_init(&progress->value, min, max, value, 1);
  progress->event = NULL;
  progress->event_arg = NULL;
  progress->padding = 0;
}

wing_obj_t *wing_progress_obj(wing_progress_t *progress)
{
  return progress == NULL ? NULL : &progress->obj;
}

void wing_progress_set_value(wing_progress_t *progress, uint16_t value)
{
  if (progress == NULL)
    {
      return;
    }

  (void)wing_widget_update_value(&progress->obj, &progress->value, value);
}

uint16_t wing_progress_get_value(const wing_progress_t *progress)
{
  return progress == NULL ? 0 : progress->value.value;
}

void wing_progress_get_range(const wing_progress_t *progress,
                             uint16_t *min, uint16_t *max)
{
  if (min != NULL)
    {
      *min = progress == NULL ? 0 : progress->value.min;
    }

  if (max != NULL)
    {
      *max = progress == NULL ? 0 : progress->value.max;
    }
}

void wing_progress_set_range(wing_progress_t *progress,
                             uint16_t min, uint16_t max)
{
  if (progress == NULL)
    {
      return;
    }

  wing_value_model_set_range(&progress->value, min, max);
  wing_obj_invalidate(&progress->obj);
}

void wing_progress_set_step(wing_progress_t *progress, uint16_t step)
{
  if (progress == NULL)
    {
      return;
    }

  wing_value_model_set_step(&progress->value, step);
}

uint16_t wing_progress_get_step(const wing_progress_t *progress)
{
  return progress == NULL ? 0 : progress->value.step;
}

void wing_progress_set_frame_style(wing_progress_t *progress,
                                   const wing_box_style_t *style)
{
  if (progress == NULL || style == NULL)
    {
      return;
    }

  progress->frame_style = *style;
  wing_obj_invalidate(&progress->obj);
}

void wing_progress_set_fill_style(wing_progress_t *progress,
                                  const wing_box_style_t *style)
{
  if (progress == NULL || style == NULL)
    {
      return;
    }

  progress->fill_style = *style;
  wing_obj_invalidate(&progress->obj);
}

void wing_progress_set_padding(wing_progress_t *progress, uint8_t padding)
{
  if (progress == NULL)
    {
      return;
    }

  progress->padding = padding;
  wing_obj_invalidate(&progress->obj);
}

void wing_progress_set_event_cb(wing_progress_t *progress,
                                wing_progress_event_fn_t event, void *arg)
{
  if (progress == NULL)
    {
      return;
    }

  progress->event = event;
  progress->event_arg = arg;
}
