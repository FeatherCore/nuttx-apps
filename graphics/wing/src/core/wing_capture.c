/****************************************************************************
 * apps/graphics/wing/src/core/wing_capture.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <wing/core/wing_capture.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_gui_capture_pointer(wing_gui_t *gui, wing_obj_t *obj)
{
  wing_obj_t *old_capture;
  int ret;

  if (gui == NULL || obj == NULL || obj->gui != gui)
    {
      return -EINVAL;
    }

  if (gui->captured_obj == obj)
    {
      return 0;
    }

  old_capture = gui->captured_obj;
  gui->captured_obj = obj;

  if (old_capture != NULL)
    {
      ret = wing_gui_post_event(gui, old_capture,
                                WING_EVENT_POINTER_RELEASED, NULL, NULL);
      if (ret < 0)
        {
          return ret;
        }
    }

  return wing_gui_post_event(gui, obj, WING_EVENT_POINTER_CAPTURED,
                             NULL, NULL);
}

int wing_gui_release_pointer(wing_gui_t *gui, wing_obj_t *obj)
{
  wing_obj_t *old_capture;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  old_capture = gui->captured_obj;
  if (old_capture == NULL)
    {
      return 0;
    }

  if (obj != NULL && obj != old_capture)
    {
      return -EPERM;
    }

  gui->captured_obj = NULL;
  return wing_gui_post_event(gui, old_capture, WING_EVENT_POINTER_RELEASED,
                             NULL, NULL);
}

int wing_gui_cancel_pointer_capture(wing_gui_t *gui)
{
  wing_obj_t *old_capture;
  uint16_t state;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  old_capture = gui->captured_obj;
  if (old_capture == NULL)
    {
      gui->pressed_obj = NULL;
      return 0;
    }

  gui->captured_obj = NULL;
  gui->pressed_obj = NULL;
  state = wing_obj_get_state(old_capture);
  wing_obj_set_state(old_capture,
                     (uint16_t)(state & ~WING_OBJ_STATE_PRESSED));

  return wing_gui_post_event(gui, old_capture,
                             WING_EVENT_POINTER_CANCELLED, NULL, NULL);
}

wing_obj_t *wing_gui_get_pointer_capture(const wing_gui_t *gui)
{
  return gui == NULL ? NULL : gui->captured_obj;
}
