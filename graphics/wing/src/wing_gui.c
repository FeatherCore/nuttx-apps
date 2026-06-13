/****************************************************************************
 * apps/graphics/wing/src/wing_gui.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdint.h>

#include <wing/wing.h>
#include <wing/core/wing_dirty.h>
#include <wing/core/wing_render.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WING_GUI_DEFAULT_FRAME_INTERVAL_MS 33

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static wing_rect_t wing_surface_rect(const wing_surface_t *surface)
{
  wing_rect_t rect;

  rect.x = 0;
  rect.y = 0;
  rect.w = surface->render_surface.width;
  rect.h = surface->render_surface.height;

  return rect;
}

static wing_rect_t wing_gui_union_rect(wing_rect_t a, wing_rect_t b)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;
  wing_rect_t out;

  if (a.w == 0 || a.h == 0)
    {
      return b;
    }

  if (b.w == 0 || b.h == 0)
    {
      return a;
    }

  ax2 = (int32_t)a.x + a.w;
  ay2 = (int32_t)a.y + a.h;
  bx2 = (int32_t)b.x + b.w;
  by2 = (int32_t)b.y + b.h;

  x1 = a.x < b.x ? a.x : b.x;
  y1 = a.y < b.y ? a.y : b.y;
  x2 = ax2 > bx2 ? ax2 : bx2;
  y2 = ay2 > by2 ? ay2 : by2;

  out.x = (int16_t)x1;
  out.y = (int16_t)y1;
  out.w = (uint16_t)(x2 - x1);
  out.h = (uint16_t)(y2 - y1);

  return out;
}

int wing_gui_create(wing_gui_t *gui, wing_surface_t *surface,
                    fr_command_list_t *commands,
                    wing_gui_render_fn_t render, void *render_arg)
{
  wing_viewport_t viewport;
  uint8_t i;
  int ret;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  ret = wing_gui_init(&gui->context, surface, commands);
  if (ret < 0)
    {
      return ret;
    }

  gui->surface = surface;
  gui->commands = commands;
  gui->root = NULL;
  gui->pressed_obj = NULL;
  gui->captured_obj = NULL;
  gui->focused_obj = NULL;
  gui->hovered_obj = NULL;
  gui->render = render;
  gui->render_arg = render_arg;
  gui->theme = NULL;
  gui->input_read = NULL;
  gui->input_read_arg = NULL;
  gui->dirty_rect = wing_surface_rect(surface);
  wing_viewport_init(&viewport, 0, 0, gui->dirty_rect.w, gui->dirty_rect.h);
  wing_camera_init_default(&gui->camera, &viewport);
  gui->tick_ms = 0;
  gui->last_frame_ms = 0;
  gui->frame_interval_ms = WING_GUI_DEFAULT_FRAME_INTERVAL_MS;
  gui->input_head = 0;
  gui->input_tail = 0;
  gui->input_count = 0;
  gui->event_head = 0;
  gui->event_tail = 0;
  gui->event_count = 0;
  gui->dirty_rects[0] = gui->dirty_rect;
  gui->dirty_rect_count = 1;
  gui->last_planned_redraw_count = 0;
  gui->last_redraw_count = 0;
  for (i = 0; i < WING_GUI_DIRTY_RECT_MAX; i++)
    {
      gui->last_redraw_rects[i] = (wing_rect_t){0, 0, 0, 0};
    }

  gui->dirty_merge_count = 0;
  gui->last_command_capacity_fallback = false;
  gui->last_redraw_cost_fallback = false;
  for (i = 0; i < WING_GUI_TIMER_MAX; i++)
    {
      gui->timers[i].callback = NULL;
      gui->timers[i].arg = NULL;
      gui->timers[i].period_ms = 0;
      gui->timers[i].elapsed_ms = 0;
      gui->timers[i].repeat = false;
      gui->timers[i].active = false;
    }

  for (i = 0; i < WING_GUI_ANIM_MAX; i++)
    {
      gui->animations[i].apply = NULL;
      gui->animations[i].done = NULL;
      gui->animations[i].arg = NULL;
      gui->animations[i].start_value = 0;
      gui->animations[i].end_value = 0;
      gui->animations[i].duration_ms = 0;
      gui->animations[i].elapsed_ms = 0;
      gui->animations[i].path = WING_ANIM_PATH_LINEAR;
      gui->animations[i].active = false;
    }

  gui->has_dirty_rect = true;
  gui->dirty = true;
  gui->running = true;

  return 0;
}

void wing_gui_set_theme(wing_gui_t *gui, const wing_theme_t *theme)
{
  if (gui != NULL)
    {
      gui->theme = theme;
    }
}

const wing_theme_t *wing_gui_get_theme(const wing_gui_t *gui)
{
  return gui == NULL ? NULL : gui->theme;
}

void wing_gui_set_camera(wing_gui_t *gui, const wing_camera_t *camera)
{
  wing_camera_event_t payload;

  if (gui == NULL || camera == NULL)
    {
      return;
    }

  if (wing_camera_equal(&gui->camera, camera))
    {
      return;
    }

  payload.old_camera = gui->camera;
  payload.camera = *camera;

  wing_gui_invalidate(gui);
  gui->camera = *camera;
  wing_gui_invalidate(gui);

  if (gui->root != NULL)
    {
      (void)wing_obj_send_event(gui->root, WING_EVENT_CAMERA_CHANGED, NULL,
                                &payload);
    }
}

const wing_camera_t *wing_gui_get_camera(const wing_gui_t *gui)
{
  return gui == NULL ? NULL : &gui->camera;
}

void wing_gui_destroy(wing_gui_t *gui)
{
  uint8_t i;

  if (gui != NULL)
    {
      wing_gui_deinit(&gui->context);
      gui->surface = NULL;
      gui->commands = NULL;
      gui->root = NULL;
      gui->pressed_obj = NULL;
      gui->captured_obj = NULL;
      gui->focused_obj = NULL;
      gui->hovered_obj = NULL;
      gui->render = NULL;
      gui->render_arg = NULL;
      wing_viewport_init(&gui->camera.viewport, 0, 0, 0, 0);
      gui->camera.position.x = 0;
      gui->camera.position.y = 0;
      gui->camera.position.z = 0;
      gui->camera.target.x = 0;
      gui->camera.target.y = 0;
      gui->camera.target.z = 0;
      gui->camera.near_z = 0;
      gui->camera.far_z = 0;
      gui->camera.focal_length = 0;
      gui->dirty_rect.x = 0;
      gui->dirty_rect.y = 0;
      gui->dirty_rect.w = 0;
      gui->dirty_rect.h = 0;
      gui->dirty_rect_count = 0;
      gui->last_planned_redraw_count = 0;
      gui->last_redraw_count = 0;
      for (i = 0; i < WING_GUI_DIRTY_RECT_MAX; i++)
        {
          gui->last_redraw_rects[i] = (wing_rect_t){0, 0, 0, 0};
        }

      gui->dirty_merge_count = 0;
      gui->last_command_capacity_fallback = false;
      gui->last_redraw_cost_fallback = false;
      gui->tick_ms = 0;
      gui->last_frame_ms = 0;
      gui->frame_interval_ms = 0;
      gui->input_head = 0;
      gui->input_tail = 0;
      gui->input_count = 0;
      gui->event_head = 0;
      gui->event_tail = 0;
      gui->event_count = 0;
      for (i = 0; i < WING_GUI_TIMER_MAX; i++)
        {
          gui->timers[i].callback = NULL;
          gui->timers[i].arg = NULL;
          gui->timers[i].period_ms = 0;
          gui->timers[i].elapsed_ms = 0;
          gui->timers[i].repeat = false;
          gui->timers[i].active = false;
        }

      for (i = 0; i < WING_GUI_ANIM_MAX; i++)
        {
          gui->animations[i].apply = NULL;
          gui->animations[i].done = NULL;
          gui->animations[i].arg = NULL;
          gui->animations[i].start_value = 0;
          gui->animations[i].end_value = 0;
          gui->animations[i].duration_ms = 0;
          gui->animations[i].elapsed_ms = 0;
          gui->animations[i].path = WING_ANIM_PATH_LINEAR;
          gui->animations[i].active = false;
        }

      gui->has_dirty_rect = false;
      gui->dirty = false;
      gui->running = false;
    }
}

void wing_gui_set_frame_interval(wing_gui_t *gui, uint32_t interval_ms)
{
  if (gui != NULL)
    {
      gui->frame_interval_ms = interval_ms;
    }
}

void wing_gui_request_stop(wing_gui_t *gui)
{
  if (gui != NULL)
    {
      gui->running = false;
    }
}

bool wing_gui_is_running(const wing_gui_t *gui)
{
  return gui != NULL && gui->running;
}


int wing_gui_step(wing_gui_t *gui)
{
  int ret;

  if (gui == NULL || !gui->running)
    {
      return -EINVAL;
    }

  if (gui->root != NULL)
    {
      ret = wing_obj_layout_tree(gui->root);
      if (ret < 0)
        {
          return ret;
        }
    }

  while (gui->input_count > 0)
    {
      ret = wing_gui_dispatch_input(gui, &gui->input_queue[gui->input_tail]);
      if (ret < 0)
        {
          return ret;
        }

      ret = wing_gui_dispatch_events(gui);
      if (ret < 0)
        {
          return ret;
        }

      gui->input_tail = (uint8_t)((gui->input_tail + 1) %
                                  WING_GUI_INPUT_QUEUE_SIZE);
      gui->input_count--;
    }

  ret = wing_gui_dispatch_events(gui);
  if (ret < 0)
    {
      return ret;
    }

  if (!gui->running)
    {
      gui->last_redraw_count = 0;
      return 0;
    }

  return wing_gui_render_dirty(gui);
}

int wing_gui_handle(wing_gui_t *gui, uint32_t elapsed_ms,
                    wing_gui_frame_t *frame)
{
  uint8_t i;
  int ret;

  if (frame != NULL)
    {
      frame->tick_ms = 0;
      frame->step_result = 0;
      frame->input_polled = 0;
      frame->dirty_before.x = 0;
      frame->dirty_before.y = 0;
      frame->dirty_before.w = 0;
      frame->dirty_before.h = 0;
      frame->dirty_after_tick = frame->dirty_before;
      frame->dirty_after_step = frame->dirty_before;
      frame->present_rect = frame->dirty_before;
      for (i = 0; i < WING_GUI_DIRTY_RECT_MAX; i++)
        {
          frame->present_rects[i] = frame->dirty_before;
        }

      frame->dirty_count_before = 0;
      frame->dirty_count_after_tick = 0;
      frame->dirty_count_after_step = 0;
      frame->present_rect_count = 0;
      frame->planned_redraw_count = 0;
      frame->redraw_count = 0;
      frame->dirty_merge_count_before = 0;
      frame->dirty_merge_count_after_tick = 0;
      frame->dirty_merge_count_after_step = 0;
      frame->has_dirty_before = false;
      frame->has_dirty_after_tick = false;
      frame->has_dirty_after_step = false;
      frame->has_present_rect = false;
      frame->command_capacity_fallback = false;
      frame->redraw_cost_fallback = false;
    }

  if (gui == NULL)
    {
      return -EINVAL;
    }

  if (frame != NULL)
    {
      frame->tick_ms = gui->tick_ms;
      frame->has_dirty_before =
        wing_gui_get_dirty_rect(gui, &frame->dirty_before);
      frame->dirty_count_before = wing_gui_get_dirty_rect_count(gui);
      frame->dirty_merge_count_before = wing_gui_get_dirty_merge_count(gui);
    }

  /* Pump platform input into the WING runtime before advancing timers and
   * animations.  Input, timers and animations are all frame sources; none of
   * them should starve another source.  The runtime collects them first and
   * then wing_gui_step() dispatches events and commits rendering once.
   */

  ret = wing_gui_poll_input(gui, WING_GUI_INPUT_QUEUE_SIZE);
  if (ret < 0)
    {
      return ret;
    }

  if (frame != NULL)
    {
      frame->input_polled = (uint8_t)ret;
    }

  ret = wing_gui_tick(gui, elapsed_ms);
  if (ret < 0)
    {
      return ret;
    }

  if (frame != NULL)
    {
      frame->tick_ms = gui->tick_ms;
      frame->has_dirty_after_tick =
        wing_gui_get_dirty_rect(gui, &frame->dirty_after_tick);
      frame->dirty_count_after_tick = wing_gui_get_dirty_rect_count(gui);
      frame->dirty_merge_count_after_tick =
        wing_gui_get_dirty_merge_count(gui);
    }

  ret = wing_gui_step(gui);
  if (frame != NULL)
    {
      frame->step_result = ret;
      frame->planned_redraw_count = gui->last_planned_redraw_count;
      frame->redraw_count = gui->last_redraw_count;
      frame->command_capacity_fallback =
        gui->last_command_capacity_fallback;
      frame->redraw_cost_fallback = gui->last_redraw_cost_fallback;
      if (ret > 0 && gui->last_redraw_count > 0)
        {
          frame->present_rect_count = gui->last_redraw_count;
          if (frame->present_rect_count > WING_GUI_DIRTY_RECT_MAX)
            {
              frame->present_rect_count = WING_GUI_DIRTY_RECT_MAX;
            }

          for (i = 0; i < frame->present_rect_count; i++)
            {
              frame->present_rects[i] = gui->last_redraw_rects[i];
            }

          frame->present_rect = frame->present_rects[0];
          for (i = 1; i < frame->present_rect_count; i++)
            {
              frame->present_rect =
                wing_gui_union_rect(frame->present_rect,
                                    frame->present_rects[i]);
            }

          frame->has_present_rect = true;
        }

      frame->has_dirty_after_step =
        wing_gui_get_dirty_rect(gui, &frame->dirty_after_step);
      frame->dirty_count_after_step = wing_gui_get_dirty_rect_count(gui);
      frame->dirty_merge_count_after_step =
        wing_gui_get_dirty_merge_count(gui);
    }

  return ret;
}
