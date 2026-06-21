/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_scroll_view.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <errno.h>

#include <wing/wing.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int16_t wing_scroll_view_max_offset_x(const wing_scroll_view_t *view)
{
  const wing_rect_t *bounds;

  if (view == NULL)
    {
      return 0;
    }

  bounds = wing_obj_get_bounds(&((wing_scroll_view_t *)view)->box.obj);
  if (bounds == NULL || view->content_width <= bounds->w)
    {
      return 0;
    }

  return (int16_t)(view->content_width - bounds->w);
}

static int16_t wing_scroll_view_max_offset_y(const wing_scroll_view_t *view)
{
  const wing_rect_t *bounds;

  if (view == NULL)
    {
      return 0;
    }

  bounds = wing_obj_get_bounds(&((wing_scroll_view_t *)view)->box.obj);
  if (bounds == NULL || view->content_height <= bounds->h)
    {
      return 0;
    }

  return (int16_t)(view->content_height - bounds->h);
}

static int16_t wing_scroll_view_clamp_offset(int16_t offset,
                                             int16_t max_offset)
{
  if (offset < 0)
    {
      return 0;
    }

  return offset > max_offset ? max_offset : offset;
}

static int16_t wing_scroll_view_step_delta(uint16_t step)
{
  return step == 0 ? 1 : (int16_t)step;
}

static void wing_scroll_view_step(wing_scroll_view_t *view,
                                  int16_t dx, int16_t dy)
{
  if (view == NULL)
    {
      return;
    }

  wing_scroll_view_set_offset(view, (int16_t)(view->offset_x + dx),
                              (int16_t)(view->offset_y + dy));
}

static int wing_scroll_view_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_scroll_view_t *view;
  const wing_input_event_t *input;
  bool handled;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  view = (wing_scroll_view_t *)wing_obj_get_user_data(obj);
  if (view == NULL)
    {
      return -EINVAL;
    }

  handled = false;
  if (event->code == WING_EVENT_KEY_DOWN)
    {
      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          if (input->key == WING_KEY_RIGHT)
            {
              wing_scroll_view_step(
                view, wing_scroll_view_step_delta(view->step_x), 0);
              handled = true;
            }
          else if (input->key == WING_KEY_LEFT)
            {
              wing_scroll_view_step(
                view, -wing_scroll_view_step_delta(view->step_x), 0);
              handled = true;
            }
          else if (input->key == WING_KEY_DOWN)
            {
              wing_scroll_view_step(
                view, 0, wing_scroll_view_step_delta(view->step_y));
              handled = true;
            }
          else if (input->key == WING_KEY_UP)
            {
              wing_scroll_view_step(
                view, 0, -wing_scroll_view_step_delta(view->step_y));
              handled = true;
            }
        }

      if (handled)
        {
          wing_event_stop_propagation(event);
        }
    }
  else if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      int16_t delta;

      input = (const wing_input_event_t *)event->data;
      delta = input == NULL ? 0 : input->encoder_delta;
      while (delta != 0)
        {
          if (wing_scroll_view_max_offset_y(view) > 0)
            {
              wing_scroll_view_step(
                view, 0, delta > 0 ?
                wing_scroll_view_step_delta(view->step_y) :
                -wing_scroll_view_step_delta(view->step_y));
            }
          else
            {
              wing_scroll_view_step(
                view, delta > 0 ?
                wing_scroll_view_step_delta(view->step_x) :
                -wing_scroll_view_step_delta(view->step_x), 0);
            }

          delta += delta > 0 ? -1 : 1;
          handled = true;
        }

      if (handled)
        {
          wing_event_stop_propagation(event);
        }
    }

  if (event->code == WING_EVENT_CLICK)
    {
      wing_event_stop_propagation(event);
    }

  if (view->event != NULL)
    {
      return view->event(view, event, view->event_arg);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_scroll_view_init(wing_scroll_view_t *view,
                           const wing_rect_t *bounds,
                           const wing_box_style_t *style)
{
  if (view == NULL)
    {
      return;
    }

  wing_box_init(&view->box, bounds, style);
  wing_obj_set_event_cb(wing_box_obj(&view->box), wing_scroll_view_event);
  wing_obj_set_user_data(wing_box_obj(&view->box), view);
  wing_obj_set_flags(wing_box_obj(&view->box),
                     wing_obj_get_flags(wing_box_obj(&view->box)) |
                     WING_OBJ_FLAG_FOCUSABLE);
  view->event = NULL;
  view->event_arg = NULL;
  view->content_width = bounds == NULL ? 0 : bounds->w;
  view->content_height = bounds == NULL ? 0 : bounds->h;
  view->step_x = 1;
  view->step_y = 1;
  view->offset_x = 0;
  view->offset_y = 0;
  wing_obj_set_clip_children(wing_box_obj(&view->box), true);
}

wing_obj_t *wing_scroll_view_obj(wing_scroll_view_t *view)
{
  return view != NULL ? wing_box_obj(&view->box) : NULL;
}

wing_box_t *wing_scroll_view_box(wing_scroll_view_t *view)
{
  return view != NULL ? &view->box : NULL;
}

void wing_scroll_view_set_style(wing_scroll_view_t *view,
                                const wing_box_style_t *style)
{
  if (view == NULL)
    {
      return;
    }

  wing_box_set_style(&view->box, style);
}

void wing_scroll_view_set_state_style(wing_scroll_view_t *view,
                                      uint16_t state,
                                      const wing_box_style_t *style)
{
  if (view == NULL)
    {
      return;
    }

  wing_box_set_state_style(&view->box, state, style);
}

void wing_scroll_view_set_offset(wing_scroll_view_t *view,
                                 int16_t offset_x, int16_t offset_y)
{
  wing_obj_t *child;
  int16_t dx;
  int16_t dy;
  int16_t max_offset_x;
  int16_t max_offset_y;
  int16_t old_offset_x;
  int16_t old_offset_y;
  wing_scroll_event_t payload;

  if (view == NULL)
    {
      return;
    }

  max_offset_x = wing_scroll_view_max_offset_x(view);
  max_offset_y = wing_scroll_view_max_offset_y(view);
  offset_x = wing_scroll_view_clamp_offset(offset_x, max_offset_x);
  offset_y = wing_scroll_view_clamp_offset(offset_y, max_offset_y);
  if (view->offset_x == offset_x && view->offset_y == offset_y)
    {
      return;
    }

  old_offset_x = view->offset_x;
  old_offset_y = view->offset_y;
  dx = (int16_t)(view->offset_x - offset_x);
  dy = (int16_t)(view->offset_y - offset_y);
  wing_obj_invalidate(wing_scroll_view_obj(view));

  view->offset_x = offset_x;
  view->offset_y = offset_y;

  for (child = view->box.obj.first_child; child != NULL;
       child = child->next_sibling)
    {
      const wing_rect_t *bounds;
      wing_rect_t next;

      bounds = wing_obj_get_bounds(child);
      if (bounds == NULL)
        {
          continue;
        }

      next = *bounds;
      next.x = (int16_t)(next.x + dx);
      next.y = (int16_t)(next.y + dy);
      (void)wing_obj_set_bounds(child, &next);
    }

  payload.old_offset_x = old_offset_x;
  payload.old_offset_y = old_offset_y;
  payload.offset_x = offset_x;
  payload.offset_y = offset_y;
  payload.max_offset_x = max_offset_x;
  payload.max_offset_y = max_offset_y;
  (void)wing_obj_send_event(wing_scroll_view_obj(view),
                            WING_EVENT_SCROLL_CHANGED, NULL, &payload);
  wing_obj_invalidate(wing_scroll_view_obj(view));
}

void wing_scroll_view_get_offset(const wing_scroll_view_t *view,
                                 int16_t *offset_x, int16_t *offset_y)
{
  if (offset_x != NULL)
    {
      *offset_x = view != NULL ? view->offset_x : 0;
    }

  if (offset_y != NULL)
    {
      *offset_y = view != NULL ? view->offset_y : 0;
    }
}

void wing_scroll_view_scroll_by(wing_scroll_view_t *view,
                                int16_t delta_x, int16_t delta_y)
{
  if (view == NULL)
    {
      return;
    }

  wing_scroll_view_set_offset(view, (int16_t)(view->offset_x + delta_x),
                              (int16_t)(view->offset_y + delta_y));
}

void wing_scroll_view_get_max_offset(const wing_scroll_view_t *view,
                                     int16_t *max_offset_x,
                                     int16_t *max_offset_y)
{
  if (max_offset_x != NULL)
    {
      *max_offset_x = wing_scroll_view_max_offset_x(view);
    }

  if (max_offset_y != NULL)
    {
      *max_offset_y = wing_scroll_view_max_offset_y(view);
    }
}

void wing_scroll_view_set_content_size(wing_scroll_view_t *view,
                                       uint16_t width, uint16_t height)
{
  if (view == NULL)
    {
      return;
    }

  view->content_width = width;
  view->content_height = height;
  wing_scroll_view_set_offset(view, view->offset_x, view->offset_y);
  wing_obj_invalidate(wing_scroll_view_obj(view));
}

void wing_scroll_view_get_content_size(const wing_scroll_view_t *view,
                                       uint16_t *width, uint16_t *height)
{
  if (width != NULL)
    {
      *width = view == NULL ? 0 : view->content_width;
    }

  if (height != NULL)
    {
      *height = view == NULL ? 0 : view->content_height;
    }
}

void wing_scroll_view_set_step(wing_scroll_view_t *view,
                               uint16_t step_x, uint16_t step_y)
{
  if (view == NULL)
    {
      return;
    }

  view->step_x = step_x == 0 ? 1 : step_x;
  view->step_y = step_y == 0 ? 1 : step_y;
}

void wing_scroll_view_get_step(const wing_scroll_view_t *view,
                               uint16_t *step_x, uint16_t *step_y)
{
  if (step_x != NULL)
    {
      *step_x = view == NULL ? 0 : view->step_x;
    }

  if (step_y != NULL)
    {
      *step_y = view == NULL ? 0 : view->step_y;
    }
}

void wing_scroll_view_set_event_cb(wing_scroll_view_t *view,
                                   wing_scroll_view_event_fn_t event,
                                   void *arg)
{
  if (view == NULL)
    {
      return;
    }

  view->event = event;
  view->event_arg = arg;
}
