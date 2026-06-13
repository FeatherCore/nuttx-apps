/****************************************************************************
 * apps/graphics/wing/src/widgets/internal/wing_widget_style.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <wing/core/wing_object.h>
#include <wing/core/wing_space.h>
#include <wing/core/wing_widget_style.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool wing_widget_project_style_rect(const wing_obj_t *obj,
                                           const wing_rect_t *rect,
                                           wing_quad2d_t *quad,
                                           wing_rect_t *projected_bounds)
{
  wing_space_transform_t world_transform;

  if (obj == NULL || rect == NULL || quad == NULL)
    {
      return false;
    }

  if (wing_obj_get_world_space_transform(obj, &world_transform) < 0 ||
      wing_space_transform_is_identity(&world_transform))
    {
      return false;
    }

  if (wing_project_rect_quad(wing_gui_get_camera(obj->gui),
                             &world_transform, rect, quad) < 0)
    {
      return false;
    }

  if (projected_bounds != NULL &&
      wing_quad2d_get_bounds(quad, projected_bounds) < 0)
    {
      return false;
    }

  return true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_widget_draw_style(wing_context_t *ctx, const wing_rect_t *rect,
                            const wing_box_style_t *style)
{
  wing_widget_draw_style_for_obj(ctx, NULL, rect, style);
}

void wing_widget_draw_style_for_obj(wing_context_t *ctx,
                                    const wing_obj_t *obj,
                                    const wing_rect_t *rect,
                                    const wing_box_style_t *style)
{
  wing_color_t color;
  wing_quad2d_t quad;
  bool projected;

  if (ctx == NULL || rect == NULL || style == NULL)
    {
      return;
    }

  projected = wing_widget_project_style_rect(obj, rect, &quad, NULL);

  if (style->clear)
    {
      color = wing_widget_style_color_for_obj(obj, style, style->fill);
      (void)wing_gui_clear(ctx, color);
    }
  else if (style->has_fill)
    {
      color = wing_widget_style_color_for_obj(obj, style, style->fill);
      if (!projected)
        {
          (void)wing_widget_fill_rect_for_obj(ctx, obj, rect, color);
        }
      else
        {
          (void)wing_gui_fill_quad(ctx, &quad, color);
        }
    }

  if (style->has_stroke && style->stroke_width > 0)
    {
      color = wing_widget_style_color_for_obj(obj, style, style->stroke);
      if (projected)
        {
          (void)wing_gui_draw_quad(ctx, &quad, style->stroke_width, color);
        }
      else
        {
          (void)wing_gui_draw_rect(ctx, rect, style->stroke_width, color);
        }
    }
}

void wing_widget_draw_style_background_for_obj(
  wing_context_t *ctx, const wing_obj_t *obj, const wing_rect_t *rect,
  const wing_box_style_t *style)
{
  wing_box_style_t background;

  if (style == NULL)
    {
      return;
    }

  background = *style;
  background.has_stroke = false;
  background.stroke_width = 0;

  wing_widget_draw_style_for_obj(ctx, obj, rect, &background);
}

void wing_widget_draw_style_overlay_for_obj(
  wing_context_t *ctx, const wing_obj_t *obj, const wing_rect_t *rect,
  const wing_box_style_t *style)
{
  wing_box_style_t overlay;

  if (style == NULL)
    {
      return;
    }

  overlay = *style;
  overlay.clear = false;
  overlay.has_fill = false;

  wing_widget_draw_style_for_obj(ctx, obj, rect, &overlay);
}

int wing_widget_fill_rect_for_obj(wing_context_t *ctx,
                                  const wing_obj_t *obj,
                                  const wing_rect_t *rect,
                                  wing_color_t color)
{
  wing_quad2d_t quad;

  if (ctx == NULL || rect == NULL)
    {
      return -EINVAL;
    }

  if (wing_widget_project_style_rect(obj, rect, &quad, NULL))
    {
      return wing_gui_fill_quad(ctx, &quad, color);
    }

  return wing_gui_fill_rect(ctx, rect, color);
}

wing_color_t wing_widget_style_color(const wing_box_style_t *style,
                                     wing_color_t color)
{
  uint16_t alpha;

  if (style == NULL)
    {
      return color;
    }

  alpha = (uint16_t)color.a * style->opacity + 127;
  color.a = (uint8_t)(alpha / 255);

  return color;
}

wing_color_t wing_widget_style_color_for_obj(const wing_obj_t *obj,
                                             const wing_box_style_t *style,
                                             wing_color_t color)
{
  uint16_t alpha;

  color = wing_widget_style_color(style, color);
  if (obj == NULL)
    {
      return color;
    }

  alpha = (uint16_t)color.a * wing_obj_get_effective_opacity(obj) + 127;
  color.a = (uint8_t)(alpha / 255);
  return color;
}

void wing_widget_state_style_init(wing_widget_state_style_t *state_style)
{
  if (state_style == NULL)
    {
      return;
    }

  wing_box_style_init(&state_style->pressed);
  wing_box_style_init(&state_style->focused);
  wing_box_style_init(&state_style->disabled);
  wing_box_style_init(&state_style->hovered);
  wing_box_style_init(&state_style->checked);
  wing_box_style_init(&state_style->selected);
  wing_box_style_init(&state_style->active);
  state_style->mask = 0;
}

const wing_box_style_t *wing_widget_select_style(
  const wing_obj_t *obj, const wing_box_style_t *style,
  const wing_widget_state_style_t *state_style)
{
  uint16_t state;

  if (obj == NULL || state_style == NULL)
    {
      return style;
    }

  state = wing_obj_get_state(obj);
  if ((state & WING_OBJ_STATE_DISABLED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_DISABLED) != 0)
    {
      return &state_style->disabled;
    }

  if ((state & WING_OBJ_STATE_PRESSED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_PRESSED) != 0)
    {
      return &state_style->pressed;
    }

  if ((state & WING_OBJ_STATE_HOVERED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_HOVERED) != 0)
    {
      return &state_style->hovered;
    }

  if ((state & WING_OBJ_STATE_CHECKED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_CHECKED) != 0)
    {
      return &state_style->checked;
    }

  if ((state & WING_OBJ_STATE_ACTIVE) != 0 &&
      (state_style->mask & WING_OBJ_STATE_ACTIVE) != 0)
    {
      return &state_style->active;
    }

  if ((state & WING_OBJ_STATE_SELECTED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_SELECTED) != 0)
    {
      return &state_style->selected;
    }

  if ((state & WING_OBJ_STATE_FOCUSED) != 0 &&
      (state_style->mask & WING_OBJ_STATE_FOCUSED) != 0)
    {
      return &state_style->focused;
    }

  return style;
}

bool wing_widget_set_state_style(wing_obj_t *obj,
                                 wing_widget_state_style_t *state_style,
                                 uint16_t state,
                                 const wing_box_style_t *style)
{
  wing_box_style_t *target;
  uint16_t normalized;

  if (style == NULL || state_style == NULL)
    {
      return false;
    }

  target = NULL;
  normalized = 0;
  if ((state & WING_OBJ_STATE_DISABLED) != 0)
    {
      target = &state_style->disabled;
      normalized = WING_OBJ_STATE_DISABLED;
    }
  else if ((state & WING_OBJ_STATE_PRESSED) != 0)
    {
      target = &state_style->pressed;
      normalized = WING_OBJ_STATE_PRESSED;
    }
  else if ((state & WING_OBJ_STATE_FOCUSED) != 0)
    {
      target = &state_style->focused;
      normalized = WING_OBJ_STATE_FOCUSED;
    }
  else if ((state & WING_OBJ_STATE_HOVERED) != 0)
    {
      target = &state_style->hovered;
      normalized = WING_OBJ_STATE_HOVERED;
    }
  else if ((state & WING_OBJ_STATE_CHECKED) != 0)
    {
      target = &state_style->checked;
      normalized = WING_OBJ_STATE_CHECKED;
    }
  else if ((state & WING_OBJ_STATE_SELECTED) != 0)
    {
      target = &state_style->selected;
      normalized = WING_OBJ_STATE_SELECTED;
    }
  else if ((state & WING_OBJ_STATE_ACTIVE) != 0)
    {
      target = &state_style->active;
      normalized = WING_OBJ_STATE_ACTIVE;
    }

  if (target == NULL)
    {
      return false;
    }

  *target = *style;
  state_style->mask |= normalized;
  if (obj != NULL)
    {
      wing_obj_invalidate(obj);
    }

  return true;
}
