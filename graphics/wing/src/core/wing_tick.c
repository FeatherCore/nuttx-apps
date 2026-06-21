/****************************************************************************
 * apps/graphics/wing/src/core/wing_tick.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdint.h>

#include <wing/core/wing_tick.h>
#include <wing/wing.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t wing_anim_ease_progress(enum wing_anim_path_e path,
                                        uint32_t progress)
{
  uint64_t t;
  uint64_t inv;

  if (progress > 65535)
    {
      progress = 65535;
    }

  t = progress;
  switch (path)
    {
      case WING_ANIM_PATH_EASE_IN:
        return (uint32_t)((t * t) / 65535);

      case WING_ANIM_PATH_EASE_OUT:
        inv = 65535 - t;
        return (uint32_t)(65535 - ((inv * inv) / 65535));

      case WING_ANIM_PATH_EASE_IN_OUT:
        if (t < 32768)
          {
            return (uint32_t)((2 * t * t) / 65535);
          }

        inv = 65535 - t;
        return (uint32_t)(65535 - ((2 * inv * inv) / 65535));

      case WING_ANIM_PATH_LINEAR:
      default:
        return progress;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_gui_tick(wing_gui_t *gui, uint32_t elapsed_ms)
{
  wing_anim_apply_cb_t apply;
  wing_anim_done_cb_t done;
  wing_timer_cb_t callback;
  void *arg;
  int32_t delta;
  int32_t value;
  uint32_t progress;
  uint32_t eased;
  uint8_t i;

  if (gui == NULL || !gui->running)
    {
      return -EINVAL;
    }

  gui->tick_ms += elapsed_ms;

  for (i = 0; i < WING_GUI_TIMER_MAX; i++)
    {
      if (!gui->timers[i].active || gui->timers[i].callback == NULL)
        {
          continue;
        }

      gui->timers[i].elapsed_ms += elapsed_ms;
      if (gui->timers[i].elapsed_ms < gui->timers[i].period_ms)
        {
          continue;
        }

      callback = gui->timers[i].callback;
      arg = gui->timers[i].arg;

      if (gui->timers[i].repeat)
        {
          gui->timers[i].elapsed_ms -= gui->timers[i].period_ms;
        }
      else
        {
          gui->timers[i].active = false;
          gui->timers[i].elapsed_ms = 0;
        }

      callback(gui, arg);
    }

  for (i = 0; i < WING_GUI_ANIM_MAX; i++)
    {
      if (!gui->animations[i].active || gui->animations[i].apply == NULL)
        {
          continue;
        }

      if (gui->animations[i].elapsed_ms + elapsed_ms >=
          gui->animations[i].duration_ms)
        {
          gui->animations[i].elapsed_ms = gui->animations[i].duration_ms;
        }
      else
        {
          gui->animations[i].elapsed_ms += elapsed_ms;
        }

      delta = gui->animations[i].end_value - gui->animations[i].start_value;
      progress = (uint32_t)(((uint64_t)gui->animations[i].elapsed_ms *
                             65535) / gui->animations[i].duration_ms);
      eased = wing_anim_ease_progress(gui->animations[i].path, progress);
      value = gui->animations[i].start_value +
              (int32_t)(((int64_t)delta * eased) / 65535);

      apply = gui->animations[i].apply;
      done = gui->animations[i].done;
      arg = gui->animations[i].arg;
      apply(gui, value, arg);

      if (gui->animations[i].elapsed_ms >= gui->animations[i].duration_ms)
        {
          gui->animations[i].active = false;
          gui->animations[i].elapsed_ms = 0;

          if (done != NULL)
            {
              done(gui, arg);
            }
        }
    }

  return 0;
}
