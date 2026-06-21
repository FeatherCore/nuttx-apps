/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_widget.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "wing_widget.h"

#include <stddef.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_widget_init_obj(wing_obj_t *obj, const wing_rect_t *bounds,
                          wing_obj_draw_fn_t draw,
                          wing_obj_event_fn_t event, void *user_data,
                          bool focusable)
{
  uint16_t flags;

  if (obj == NULL)
    {
      return;
    }

  wing_obj_init(obj, bounds);
  wing_obj_set_draw_cb(obj, draw);
  wing_obj_set_event_cb(obj, event);
  wing_obj_set_user_data(obj, user_data);

  if (focusable)
    {
      flags = wing_obj_get_flags(obj);
      wing_obj_set_flags(obj, (uint16_t)(flags | WING_OBJ_FLAG_FOCUSABLE));
    }
}
