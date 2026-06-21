/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_value_input.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "wing_value_input.h"

#include "../../core/wing_value.h"
#include "wing_widget.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool wing_widget_value_key_to_direction(
  uint16_t key, enum wing_widget_value_keymap_e keymap, bool *increase)
{
  if (increase == NULL)
    {
      return false;
    }

  if (key == WING_KEY_RIGHT)
    {
      *increase = true;
      return true;
    }

  if (key == WING_KEY_LEFT)
    {
      *increase = false;
      return true;
    }

  if (key == WING_KEY_UP)
    {
      *increase = keymap == WING_WIDGET_VALUE_KEYMAP_UP_INCREASE;
      return true;
    }

  if (key == WING_KEY_DOWN)
    {
      *increase = keymap == WING_WIDGET_VALUE_KEYMAP_DOWN_INCREASE;
      return true;
    }

  return false;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

bool wing_widget_value_handle_step_input(
  wing_obj_t *obj, wing_value_model_t *value, wing_event_t *event,
  wing_widget_value_apply_fn_t apply, void *arg,
  enum wing_widget_value_keymap_e keymap)
{
  const wing_input_event_t *input;
  int16_t delta;
  bool increase;

  if (obj == NULL || value == NULL || event == NULL || apply == NULL)
    {
      return false;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      input = (const wing_input_event_t *)event->data;
      if (input != NULL &&
          wing_widget_value_key_to_direction(input->key, keymap,
                                             &increase))
        {
          apply(arg, wing_value_model_step_value(value, increase));
          wing_event_stop_propagation(event);
          return true;
        }
    }
  else if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      input = (const wing_input_event_t *)event->data;
      delta = input == NULL ? 0 : input->encoder_delta;
      if (delta != 0)
        {
          while (delta != 0)
            {
              apply(arg, wing_value_model_step_value(value, delta > 0));
              delta += delta > 0 ? -1 : 1;
            }

          wing_event_stop_propagation(event);
          return true;
        }
    }

  return false;
}

bool wing_widget_value_handle_pointer_drag(
  wing_obj_t *obj, wing_event_t *event,
  wing_widget_value_pointer_update_fn_t update, void *arg)
{
  const wing_input_event_t *input;
  bool dragging;
  bool update_value;

  if (obj == NULL || event == NULL || update == NULL)
    {
      return false;
    }

  if (event->code != WING_EVENT_POINTER_DOWN &&
      event->code != WING_EVENT_POINTER_MOVE &&
      event->code != WING_EVENT_POINTER_UP &&
      event->code != WING_EVENT_POINTER_CANCELLED)
    {
      return false;
    }

  if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      wing_widget_set_pressed(obj, false);
      wing_event_stop_propagation(event);
      return true;
    }

  dragging = (wing_obj_get_state(obj) & WING_OBJ_STATE_PRESSED) != 0;
  update_value = event->code == WING_EVENT_POINTER_DOWN ||
                 (dragging &&
                  (event->code == WING_EVENT_POINTER_MOVE ||
                   event->code == WING_EVENT_POINTER_UP));

  input = (const wing_input_event_t *)event->data;
  if (update_value && input != NULL)
    {
      update(arg, input);
    }

  if (event->code == WING_EVENT_POINTER_DOWN)
    {
      wing_widget_set_pressed(obj, true);
      wing_event_stop_propagation(event);
      return true;
    }

  if (dragging || event->code == WING_EVENT_POINTER_UP)
    {
      wing_widget_set_pressed(obj, event->code != WING_EVENT_POINTER_UP);
      wing_event_stop_propagation(event);
      return true;
    }

  return false;
}
