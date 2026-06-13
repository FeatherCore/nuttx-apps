/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_box.c
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

#include "internal/wing_widget.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_box_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  const wing_box_style_t *style;
  const wing_rect_t *bounds;
  wing_box_t *box;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  box = (wing_box_t *)obj;
  style = wing_box_get_active_style(box);
  bounds = wing_obj_get_bounds(obj);
  if (style == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  wing_widget_draw_style_for_obj(ctx, obj, bounds, style);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_box_style_init(wing_box_style_t *style)
{
  if (style == NULL)
    {
      return;
    }

  style->fill = wing_color_rgba(0, 0, 0, 0);
  style->stroke = wing_color_rgba(0, 0, 0, 0);
  style->stroke_width = 0;
  style->opacity = 255;
  style->clear = false;
  style->has_fill = false;
  style->has_stroke = false;
}

void wing_box_init(wing_box_t *box, const wing_rect_t *bounds,
                   const wing_box_style_t *style)
{
  if (box == NULL)
    {
      return;
    }

  wing_obj_init(&box->obj, bounds);
  wing_box_style_init(&box->style);
  wing_widget_state_style_init(&box->state_style);

  if (style != NULL)
    {
      box->style = *style;
    }

  wing_obj_set_draw_cb(&box->obj, wing_box_draw);
}

wing_obj_t *wing_box_obj(wing_box_t *box)
{
  return box != NULL ? &box->obj : NULL;
}

const wing_obj_t *wing_box_const_obj(const wing_box_t *box)
{
  return box != NULL ? &box->obj : NULL;
}

void wing_box_set_style(wing_box_t *box, const wing_box_style_t *style)
{
  if (box == NULL || style == NULL)
    {
      return;
    }

  box->style = *style;
  wing_obj_invalidate(&box->obj);
}

void wing_box_set_state_style(wing_box_t *box, uint16_t state,
                              const wing_box_style_t *style)
{
  if (box == NULL || style == NULL)
    {
      return;
    }

  (void)wing_widget_set_state_style(&box->obj, &box->state_style, state,
                                    style);
}

const wing_box_style_t *wing_box_get_style(const wing_box_t *box)
{
  return box != NULL ? &box->style : NULL;
}

const wing_box_style_t *wing_box_get_active_style(const wing_box_t *box)
{
  if (box == NULL)
    {
      return NULL;
    }

  return wing_widget_select_style(&box->obj, &box->style,
                                  &box->state_style);
}
