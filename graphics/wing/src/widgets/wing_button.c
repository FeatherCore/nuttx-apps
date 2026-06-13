/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_button.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>

#include <wing/wing.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_button_event(wing_obj_t *obj, wing_event_t *event)
{
  wing_button_t *button;

  if (obj == NULL || event == NULL)
    {
      return -EINVAL;
    }

  button = (wing_button_t *)obj;
  if (button->event == NULL)
    {
      return 0;
    }

  return button->event(button, event, button->event_arg);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_button_init(wing_button_t *button, const wing_rect_t *bounds,
                      const wing_box_style_t *style)
{
  uint16_t flags;

  if (button == NULL)
    {
      return;
    }

  wing_box_init(&button->box, bounds, style);
  button->event = NULL;
  button->event_arg = NULL;

  flags = wing_obj_get_flags(wing_box_obj(&button->box));
  wing_obj_set_flags(wing_box_obj(&button->box),
                     (uint16_t)(flags | WING_OBJ_FLAG_FOCUSABLE));
  wing_obj_set_event_cb(wing_box_obj(&button->box), wing_button_event);
}

wing_obj_t *wing_button_obj(wing_button_t *button)
{
  return button != NULL ? wing_box_obj(&button->box) : NULL;
}

wing_box_t *wing_button_box(wing_button_t *button)
{
  return button != NULL ? &button->box : NULL;
}

void wing_button_set_style(wing_button_t *button,
                           const wing_box_style_t *style)
{
  if (button == NULL)
    {
      return;
    }

  wing_box_set_style(&button->box, style);
}

void wing_button_set_state_style(wing_button_t *button, uint16_t state,
                                 const wing_box_style_t *style)
{
  if (button == NULL)
    {
      return;
    }

  wing_box_set_state_style(&button->box, state, style);
}

void wing_button_set_event_cb(wing_button_t *button,
                              wing_button_event_fn_t event, void *arg)
{
  if (button == NULL)
    {
      return;
    }

  button->event = event;
  button->event_arg = arg;
}
