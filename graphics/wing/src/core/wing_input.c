/****************************************************************************
 * apps/graphics/wing/src/core/wing_input.c
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
 * Private Functions
 ****************************************************************************/

static int wing_gui_post_input_event(wing_gui_t *gui, wing_obj_t *target,
                                     enum wing_event_code_e code,
                                     const wing_input_event_t *input)
{
  wing_queued_event_t *queued;

  if (gui == NULL || target == NULL || input == NULL)
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
  queued->context = NULL;
  queued->data = NULL;
  queued->input = *input;
  queued->has_input = true;

  gui->event_head = (uint8_t)((gui->event_head + 1) %
                              WING_GUI_EVENT_QUEUE_SIZE);
  gui->event_count++;

  return 0;
}

static bool wing_gui_input_supports_hover(const wing_input_event_t *input)
{
  if (input == NULL)
    {
      return false;
    }

  return input->source == WING_INPUT_SOURCE_MOUSE ||
         input->source == WING_INPUT_SOURCE_UNKNOWN;
}

static void wing_gui_normalize_input_source(wing_input_event_t *input)
{
  if (input == NULL || input->source != WING_INPUT_SOURCE_UNKNOWN)
    {
      return;
    }

  switch (input->type)
    {
      case WING_INPUT_KEY_DOWN:
      case WING_INPUT_KEY_UP:
        input->source = WING_INPUT_SOURCE_KEYBOARD;
        break;

      case WING_INPUT_ENCODER_ROTATE:
        input->source = WING_INPUT_SOURCE_ENCODER;
        break;

      case WING_INPUT_CLOSE_REQUEST:
        input->source = WING_INPUT_SOURCE_SYSTEM;
        break;

      default:
        break;
    }
}

static int wing_gui_update_hover(wing_gui_t *gui, wing_obj_t *target,
                                 const wing_input_event_t *input)
{
  wing_obj_t *old_hover;
  uint16_t state;
  int ret;

  if (gui == NULL || input == NULL)
    {
      return -EINVAL;
    }

  old_hover = gui->hovered_obj;
  if (old_hover == target)
    {
      return 0;
    }

  if (old_hover != NULL)
    {
      state = wing_obj_get_state(old_hover);
      wing_obj_set_state(old_hover,
                         (uint16_t)(state & ~WING_OBJ_STATE_HOVERED));

      ret = wing_gui_post_input_event(gui, old_hover,
                                      WING_EVENT_POINTER_LEAVE, input);
      if (ret < 0)
        {
          gui->hovered_obj = NULL;
          return ret;
        }
    }

  gui->hovered_obj = target;
  if (target != NULL)
    {
      state = wing_obj_get_state(target);
      wing_obj_set_state(target, state | WING_OBJ_STATE_HOVERED);

      ret = wing_gui_post_input_event(gui, target,
                                      WING_EVENT_POINTER_ENTER, input);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_gui_set_input_reader(wing_gui_t *gui,
                               wing_input_read_fn_t input_read, void *arg)
{
  if (gui != NULL)
    {
      gui->input_read = input_read;
      gui->input_read_arg = arg;
    }
}

void wing_input_adapter_init(wing_input_adapter_t *adapter)
{
  if (adapter == NULL)
    {
      return;
    }

  adapter->pending_input.type = WING_INPUT_POINTER_MOVE;
  adapter->pending_input.source = WING_INPUT_SOURCE_UNKNOWN;
  adapter->pending_input.point.x = 0;
  adapter->pending_input.point.y = 0;
  adapter->pending_input.key = 0;
  adapter->pending_input.encoder_delta = 0;
  adapter->pending_input.button = 0;
  adapter->coalesced_moves = 0;
  adapter->has_pending_input = false;
}

int wing_input_adapter_take_pending(wing_input_adapter_t *adapter,
                                    wing_input_event_t *input)
{
  if (adapter == NULL || input == NULL)
    {
      return -EINVAL;
    }

  if (!adapter->has_pending_input)
    {
      return 0;
    }

  *input = adapter->pending_input;
  adapter->has_pending_input = false;
  adapter->coalesced_moves = 0;
  return 1;
}

int wing_input_adapter_store_pending(wing_input_adapter_t *adapter,
                                     const wing_input_event_t *input)
{
  wing_input_event_t normalized;

  if (adapter == NULL || input == NULL)
    {
      return -EINVAL;
    }

  normalized = *input;
  wing_gui_normalize_input_source(&normalized);
  adapter->pending_input = normalized;
  adapter->has_pending_input = true;
  return 0;
}

bool wing_input_adapter_merge_pointer_move(wing_input_adapter_t *adapter,
                                           wing_input_event_t *current,
                                           const wing_input_event_t *next)
{
  if (adapter == NULL || current == NULL || next == NULL)
    {
      return false;
    }

  if (current->type != WING_INPUT_POINTER_MOVE ||
      next->type != WING_INPUT_POINTER_MOVE)
    {
      (void)wing_input_adapter_store_pending(adapter, next);
      return false;
    }

  *current = *next;
  adapter->coalesced_moves++;
  return true;
}

uint16_t wing_input_adapter_get_coalesced_moves(
  const wing_input_adapter_t *adapter)
{
  return adapter == NULL ? 0 : adapter->coalesced_moves;
}

int wing_gui_poll_input(wing_gui_t *gui, uint8_t max_events)
{
  wing_input_event_t input;
  uint8_t count;
  int ret;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  if (gui->input_read == NULL)
    {
      return 0;
    }

  count = 0;
  while (count < max_events &&
         gui->input_count < WING_GUI_INPUT_QUEUE_SIZE)
    {
      ret = gui->input_read(gui, &input, gui->input_read_arg);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0)
        {
          return count;
        }

      ret = wing_gui_enqueue_input(gui, &input);
      if (ret < 0)
        {
          return ret;
        }

      count++;
    }

  return count;
}

int wing_gui_enqueue_input(wing_gui_t *gui, const wing_input_event_t *input)
{
  wing_input_event_t normalized;

  if (gui == NULL || input == NULL)
    {
      return -EINVAL;
    }

  if (gui->input_count >= WING_GUI_INPUT_QUEUE_SIZE)
    {
      return -ENOSPC;
    }

  normalized = *input;
  wing_gui_normalize_input_source(&normalized);
  gui->input_queue[gui->input_head] = normalized;
  gui->input_head = (uint8_t)((gui->input_head + 1) %
                              WING_GUI_INPUT_QUEUE_SIZE);
  gui->input_count++;

  return 0;
}

int wing_gui_request_close(wing_gui_t *gui)
{
  wing_input_event_t input;

  if (gui == NULL)
    {
      return -EINVAL;
    }

  input.type = WING_INPUT_CLOSE_REQUEST;
  input.source = WING_INPUT_SOURCE_SYSTEM;
  input.point.x = 0;
  input.point.y = 0;
  input.key = 0;
  input.encoder_delta = 0;
  input.button = 0;

  return wing_gui_enqueue_input(gui, &input);
}

int wing_gui_dispatch_input(wing_gui_t *gui, const wing_input_event_t *input)
{
  wing_obj_t *captured;
  wing_obj_t *target;
  wing_obj_t *pressed;
  wing_input_event_t normalized;
  uint16_t state;
  int ret;

  if (gui == NULL || input == NULL || gui->root == NULL)
    {
      return -EINVAL;
    }

  normalized = *input;
  wing_gui_normalize_input_source(&normalized);
  input = &normalized;

  target = wing_obj_hit_test(gui->root, input->point);
  captured = wing_gui_get_pointer_capture(gui);

  switch (input->type)
    {
      case WING_INPUT_POINTER_DOWN:
        if (wing_gui_input_supports_hover(input))
          {
            ret = wing_gui_update_hover(gui, target, input);
            if (ret < 0)
              {
                return ret;
              }
          }

        if (captured != NULL && captured != target)
          {
            ret = wing_gui_cancel_pointer_capture(gui);
            if (ret < 0)
              {
                return ret;
              }
          }

        gui->pressed_obj = target;

        if (target == NULL)
          {
            ret = wing_gui_set_focus(gui, NULL);
            if (ret < 0)
              {
                return ret;
              }

            return 0;
          }

        ret = wing_gui_set_focus(gui, target);
        if (ret < 0)
          {
            return ret;
          }

        state = wing_obj_get_state(target);
        wing_obj_set_state(target, state | WING_OBJ_STATE_PRESSED);

        ret = wing_gui_capture_pointer(gui, target);
        if (ret < 0)
          {
            return ret;
          }

        return wing_gui_post_input_event(gui, target, WING_EVENT_POINTER_DOWN,
                                         input);

      case WING_INPUT_POINTER_MOVE:
        captured = wing_gui_get_pointer_capture(gui);
        if (captured != NULL)
          {
            return wing_gui_post_input_event(gui, captured,
                                             WING_EVENT_POINTER_MOVE, input);
          }

        if (wing_gui_input_supports_hover(input))
          {
            ret = wing_gui_update_hover(gui, target, input);
            if (ret < 0)
              {
                return ret;
              }
          }

        if (target != NULL)
          {
            return wing_gui_post_input_event(gui, target,
                                             WING_EVENT_POINTER_MOVE, input);
          }

        return 0;

      case WING_INPUT_POINTER_UP:
        pressed = gui->pressed_obj;
        gui->pressed_obj = NULL;
        captured = wing_gui_get_pointer_capture(gui);

        if (pressed == NULL && captured == NULL)
          {
            return 0;
          }

        if (pressed != NULL)
          {
            state = wing_obj_get_state(pressed);
            wing_obj_set_state(pressed,
                               (uint16_t)(state &
                                          ~WING_OBJ_STATE_PRESSED));
          }

        ret = wing_gui_post_input_event(gui,
                                        captured != NULL ? captured :
                                                           pressed,
                                        WING_EVENT_POINTER_UP, input);
        if (ret < 0)
          {
            return ret;
          }

        if (captured != NULL)
          {
            ret = wing_gui_release_pointer(gui, captured);
            if (ret < 0)
              {
              return ret;
            }
          }

        if (wing_gui_input_supports_hover(input))
          {
            ret = wing_gui_update_hover(gui, target, input);
            if (ret < 0)
              {
                return ret;
              }
          }

        if (pressed == target)
          {
            return wing_gui_post_input_event(gui, pressed, WING_EVENT_CLICK,
                                             input);
          }

        return 0;

      case WING_INPUT_KEY_DOWN:
        if (input->key == WING_KEY_TAB)
          {
            return wing_gui_focus_next(gui, false);
          }

        if (gui->focused_obj != NULL)
          {
            ret = wing_gui_post_input_event(gui, gui->focused_obj,
                                            WING_EVENT_KEY_DOWN, input);
            if (ret < 0)
              {
                return ret;
              }

            if (input->key == WING_KEY_ENTER || input->key == WING_KEY_SPACE)
              {
                return wing_gui_post_input_event(gui, gui->focused_obj,
                                                 WING_EVENT_CLICK, input);
              }

            return 0;
          }

        if (input->key == WING_KEY_RIGHT || input->key == WING_KEY_DOWN)
          {
            return wing_gui_focus_next(gui, false);
          }

        if (input->key == WING_KEY_LEFT || input->key == WING_KEY_UP)
          {
            return wing_gui_focus_next(gui, true);
          }

        return 0;

      case WING_INPUT_KEY_UP:
        if (gui->focused_obj == NULL)
          {
            return 0;
          }

        return wing_gui_post_input_event(gui, gui->focused_obj,
                                         WING_EVENT_KEY_UP, input);

      case WING_INPUT_ENCODER_ROTATE:
        if (gui->focused_obj == NULL || input->encoder_delta == 0)
          {
            return 0;
          }

        return wing_gui_post_input_event(gui, gui->focused_obj,
                                         WING_EVENT_ENCODER_ROTATE, input);

      case WING_INPUT_CLOSE_REQUEST:
        ret = wing_gui_post_input_event(gui, gui->root,
                                        WING_EVENT_CLOSE_REQUEST, input);
        if (ret < 0)
          {
            return ret;
          }

        wing_gui_request_stop(gui);
        return 0;

      default:
        return -EINVAL;
    }
}
