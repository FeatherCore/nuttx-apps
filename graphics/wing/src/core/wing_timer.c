/****************************************************************************
 * apps/graphics/wing/src/core/wing_timer.c
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

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_gui_timer_start(wing_gui_t *gui, uint32_t period_ms, bool repeat,
                         wing_timer_cb_t callback, void *arg,
                         uint8_t *timer_id)
{
  uint8_t i;

  if (gui == NULL || period_ms == 0 || callback == NULL)
    {
      return -EINVAL;
    }

  for (i = 0; i < WING_GUI_TIMER_MAX; i++)
    {
      if (!gui->timers[i].active)
        {
          gui->timers[i].callback = callback;
          gui->timers[i].arg = arg;
          gui->timers[i].period_ms = period_ms;
          gui->timers[i].elapsed_ms = 0;
          gui->timers[i].repeat = repeat;
          gui->timers[i].active = true;

          if (timer_id != NULL)
            {
              *timer_id = i;
            }

          return 0;
        }
    }

  return -ENOSPC;
}

int wing_gui_timer_stop(wing_gui_t *gui, uint8_t timer_id)
{
  if (gui == NULL || timer_id >= WING_GUI_TIMER_MAX)
    {
      return -EINVAL;
    }

  gui->timers[timer_id].callback = NULL;
  gui->timers[timer_id].arg = NULL;
  gui->timers[timer_id].period_ms = 0;
  gui->timers[timer_id].elapsed_ms = 0;
  gui->timers[timer_id].repeat = false;
  gui->timers[timer_id].active = false;

  return 0;
}
