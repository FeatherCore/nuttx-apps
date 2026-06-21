/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_widget_input.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/core/wing_widget_input.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_widget_set_pressed(wing_obj_t *obj, bool pressed)
{
  uint16_t state;

  if (obj == NULL)
    {
      return;
    }

  state = wing_obj_get_state(obj);
  if (pressed)
    {
      state |= WING_OBJ_STATE_PRESSED;
    }
  else
    {
      state &= (uint16_t)~WING_OBJ_STATE_PRESSED;
    }

  wing_obj_set_state(obj, state);
}

bool wing_widget_handle_pointer_lifecycle(wing_obj_t *obj,
                                           wing_event_t *event)
{
  if (obj == NULL || event == NULL)
    {
      return false;
    }

  if (event->code == WING_EVENT_POINTER_DOWN)
    {
      wing_widget_set_pressed(obj, true);
      wing_event_stop_propagation(event);
      return true;
    }

  if (event->code == WING_EVENT_POINTER_UP ||
      event->code == WING_EVENT_POINTER_CANCELLED)
    {
      wing_widget_set_pressed(obj, false);
      wing_event_stop_propagation(event);
      return true;
    }

  return false;
}

bool wing_widget_stop_activation_key(wing_event_t *event)
{
  const wing_input_event_t *input;

  if (event == NULL || event->code != WING_EVENT_KEY_DOWN)
    {
      return false;
    }

  input = (const wing_input_event_t *)event->data;
  if (input == NULL ||
      (input->key != WING_KEY_ENTER && input->key != WING_KEY_SPACE))
    {
      return false;
    }

  wing_event_stop_propagation(event);
  return true;
}
