/****************************************************************************
 * apps/graphics/wing/src/core/wing_event.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_gui_post_event(wing_gui_t *gui, wing_obj_t *target,
                        enum wing_event_code_e code, wing_context_t *ctx,
                        void *data)
{
  wing_queued_event_t *queued;

  if (gui == NULL || target == NULL)
    {
      return -EINVAL;
    }

  if (gui->event_count >= WING_GUI_EVENT_QUEUE_SIZE)
    {
      return -ENOSPC;
    }

  queued = &gui->event_queue[gui->event_head];
  queued->target = target;
  queued->code = code;
  queued->context = ctx;
  queued->data = data;
  queued->has_input = false;

  gui->event_head = (uint8_t)((gui->event_head + 1) %
                              WING_GUI_EVENT_QUEUE_SIZE);
  gui->event_count++;

  return 0;
}

int wing_gui_dispatch_events(wing_gui_t *gui)
{
  wing_queued_event_t queued;
  void *data;
  int ret;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  while (gui->event_count > 0)
    {
      queued = gui->event_queue[gui->event_tail];
      gui->event_tail = (uint8_t)((gui->event_tail + 1) %
                                  WING_GUI_EVENT_QUEUE_SIZE);
      gui->event_count--;

      data = queued.has_input ? (void *)&queued.input : queued.data;
      ret = wing_obj_bubble_event(queued.target, queued.code,
                                  queued.context, data);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

void wing_event_stop_propagation(wing_event_t *event)
{
  if (event != NULL)
    {
      event->stopped = true;
    }
}

bool wing_event_is_stopped(const wing_event_t *event)
{
  return event != NULL && event->stopped;
}
