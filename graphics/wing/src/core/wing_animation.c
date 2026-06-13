/****************************************************************************
 * apps/graphics/wing/src/core/wing_animation.c
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

int wing_gui_anim_start(wing_gui_t *gui, int32_t start_value,
                        int32_t end_value, uint32_t duration_ms,
                        wing_anim_apply_cb_t apply,
                        wing_anim_done_cb_t done, void *arg,
                        uint8_t *anim_id)
{
  return wing_gui_anim_start_path(gui, start_value, end_value, duration_ms,
                                  WING_ANIM_PATH_LINEAR, apply, done, arg,
                                  anim_id);
}

int wing_gui_anim_start_path(wing_gui_t *gui, int32_t start_value,
                             int32_t end_value, uint32_t duration_ms,
                             enum wing_anim_path_e path,
                             wing_anim_apply_cb_t apply,
                             wing_anim_done_cb_t done, void *arg,
                             uint8_t *anim_id)
{
  uint8_t i;

  if (gui == NULL || duration_ms == 0 || apply == NULL)
    {
      return -EINVAL;
    }

  for (i = 0; i < WING_GUI_ANIM_MAX; i++)
    {
      if (!gui->animations[i].active)
        {
          gui->animations[i].apply = apply;
          gui->animations[i].done = done;
          gui->animations[i].arg = arg;
          gui->animations[i].start_value = start_value;
          gui->animations[i].end_value = end_value;
          gui->animations[i].duration_ms = duration_ms;
          gui->animations[i].elapsed_ms = 0;
          gui->animations[i].path = path;
          gui->animations[i].active = true;

          apply(gui, start_value, arg);

          if (anim_id != NULL)
            {
              *anim_id = i;
            }

          return 0;
        }
    }

  return -ENOSPC;
}

int wing_gui_anim_stop(wing_gui_t *gui, uint8_t anim_id)
{
  if (gui == NULL || anim_id >= WING_GUI_ANIM_MAX)
    {
      return -EINVAL;
    }

  gui->animations[anim_id].apply = NULL;
  gui->animations[anim_id].done = NULL;
  gui->animations[anim_id].arg = NULL;
  gui->animations[anim_id].start_value = 0;
  gui->animations[anim_id].end_value = 0;
  gui->animations[anim_id].duration_ms = 0;
  gui->animations[anim_id].elapsed_ms = 0;
  gui->animations[anim_id].path = WING_ANIM_PATH_LINEAR;
  gui->animations[anim_id].active = false;

  return 0;
}
