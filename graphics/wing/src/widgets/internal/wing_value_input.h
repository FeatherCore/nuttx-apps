/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_value_input.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_VALUE_INPUT_H
#define __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_VALUE_INPUT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum wing_widget_value_keymap_e
{
  WING_WIDGET_VALUE_KEYMAP_UP_INCREASE = 0,
  WING_WIDGET_VALUE_KEYMAP_DOWN_INCREASE
};

typedef void (*wing_widget_value_apply_fn_t)(void *arg, uint16_t value);
typedef void (*wing_widget_value_pointer_update_fn_t)(
  void *arg, const wing_input_event_t *input);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

bool wing_widget_value_handle_step_input(
  wing_obj_t *obj, wing_value_model_t *value, wing_event_t *event,
  wing_widget_value_apply_fn_t apply, void *arg,
  enum wing_widget_value_keymap_e keymap);
bool wing_widget_value_handle_pointer_drag(
  wing_obj_t *obj, wing_event_t *event,
  wing_widget_value_pointer_update_fn_t update, void *arg);

#endif /* __APPS_GRAPHICS_WING_SRC_WIDGETS_INTERNAL_WING_VALUE_INPUT_H */
