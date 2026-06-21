/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_widget_value.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/core/wing_widget_value.h>

#include "../../core/wing_value.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

bool wing_widget_update_value(wing_obj_t *obj, wing_value_model_t *value,
                              uint16_t next)
{
  wing_value_event_t payload;

  if (obj == NULL || value == NULL)
    {
      return false;
    }

  if (!wing_value_model_set_value(value, next, &payload))
    {
      return false;
    }

  wing_obj_invalidate(obj);
  (void)wing_obj_send_event(obj, WING_EVENT_VALUE_CHANGED, NULL, &payload);
  return true;
}

bool wing_widget_update_bool(wing_obj_t *obj, bool *value, bool next)
{
  wing_value_event_t payload;

  if (obj == NULL || value == NULL)
    {
      return false;
    }

  if (!wing_value_update_bool(value, next, &payload))
    {
      return false;
    }

  wing_obj_invalidate(obj);
  (void)wing_obj_send_event(obj, WING_EVENT_VALUE_CHANGED, NULL, &payload);
  return true;
}
