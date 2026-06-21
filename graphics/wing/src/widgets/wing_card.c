/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_card.c
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
#include <wing/core/wing_widget_style.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int16_t wing_card_mid_i16(int16_t a, int16_t b)
{
  return (int16_t)(((int32_t)a + b) / 2);
}

static int wing_card_draw_card(wing_card_t *card, wing_context_t *ctx)
{
  const wing_box_style_t *fill_style;
  wing_quad2d_t quad;
  wing_quad2d_t ridge;
  wing_space_transform_t world_transform;
  int ret;

  ret = wing_obj_project_quad2d(&card->obj, &quad);
  if (ret < 0)
    {
      return ret;
    }

  ret = wing_obj_get_world_space_transform(&card->obj, &world_transform);
  if (ret < 0)
    {
      return ret;
    }

  fill_style = world_transform.rotation_y >= 0 ? &card->front_style :
                                                &card->back_style;

  if (fill_style->has_fill)
    {
      ret = wing_gui_fill_quad(ctx, &quad,
                               wing_widget_style_color_for_obj(
                                 &card->obj, fill_style,
                                 fill_style->fill));
      if (ret < 0)
        {
          return ret;
        }
    }

  if (card->edge_style.has_stroke)
    {
      ret = wing_gui_draw_quad(ctx, &quad, card->edge_style.stroke_width,
                               wing_widget_style_color_for_obj(
                                 &card->obj, &card->edge_style,
                                 card->edge_style.stroke));
      if (ret < 0)
        {
          return ret;
        }
    }

  if (card->edge_style.has_fill)
    {
      ridge.points[0].x = (int16_t)(wing_card_mid_i16(
                                    quad.points[0].x, quad.points[1].x) - 1);
      ridge.points[0].y = wing_card_mid_i16(quad.points[0].y,
                                            quad.points[1].y);
      ridge.points[1].x = (int16_t)(ridge.points[0].x + 2);
      ridge.points[1].y = ridge.points[0].y;
      ridge.points[2].x = (int16_t)(wing_card_mid_i16(
                                    quad.points[3].x, quad.points[2].x) + 1);
      ridge.points[2].y = wing_card_mid_i16(quad.points[3].y,
                                            quad.points[2].y);
      ridge.points[3].x = (int16_t)(ridge.points[2].x - 2);
      ridge.points[3].y = ridge.points[2].y;

      ret = wing_gui_fill_quad(ctx, &ridge,
                               wing_widget_style_color_for_obj(
                                 &card->obj, &card->edge_style,
                                 card->edge_style.fill));
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static int wing_card_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  const wing_rect_t *bounds;
  wing_card_t *card;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  card = (wing_card_t *)wing_obj_get_user_data(obj);
  bounds = wing_obj_get_bounds(obj);
  if (card == NULL || bounds == NULL || bounds->w == 0 || bounds->h == 0)
    {
      return -EINVAL;
    }

  return wing_card_draw_card(card, ctx);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_card_init(wing_card_t *card, const wing_rect_t *bounds,
                    const wing_box_style_t *front_style,
                    const wing_box_style_t *back_style,
                    const wing_box_style_t *edge_style)
{
  if (card == NULL)
    {
      return;
    }

  wing_obj_init(&card->obj, bounds);
  wing_box_style_init(&card->front_style);
  wing_box_style_init(&card->back_style);
  wing_box_style_init(&card->edge_style);

  if (front_style != NULL)
    {
      card->front_style = *front_style;
    }

  if (back_style != NULL)
    {
      card->back_style = *back_style;
    }

  if (edge_style != NULL)
    {
      card->edge_style = *edge_style;
    }

  wing_obj_set_user_data(&card->obj, card);
  wing_obj_set_draw_cb(&card->obj, wing_card_draw);
}

wing_obj_t *wing_card_obj(wing_card_t *card)
{
  return card == NULL ? NULL : &card->obj;
}

void wing_card_set_front_style(wing_card_t *card,
                               const wing_box_style_t *style)
{
  if (card == NULL || style == NULL)
    {
      return;
    }

  card->front_style = *style;
  wing_obj_invalidate(&card->obj);
}

void wing_card_set_back_style(wing_card_t *card,
                              const wing_box_style_t *style)
{
  if (card == NULL || style == NULL)
    {
      return;
    }

  card->back_style = *style;
  wing_obj_invalidate(&card->obj);
}

void wing_card_set_edge_style(wing_card_t *card,
                              const wing_box_style_t *style)
{
  if (card == NULL || style == NULL)
    {
      return;
    }

  card->edge_style = *style;
  wing_obj_invalidate(&card->obj);
}
