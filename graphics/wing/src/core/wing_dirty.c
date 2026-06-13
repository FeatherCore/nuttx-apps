/****************************************************************************
 * apps/graphics/wing/src/core/wing_dirty.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/core/wing_dirty.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool wing_dirty_surface_is_valid(const wing_surface_t *surface)
{
  return surface != NULL && surface->render_surface.pixels != NULL &&
         surface->render_surface.width > 0 &&
         surface->render_surface.height > 0 &&
         surface->render_surface.stride >= surface->render_surface.width &&
         surface->render_surface.format == FR_FORMAT_RGBA8888;
}

static wing_rect_t wing_dirty_surface_rect(const wing_surface_t *surface)
{
  wing_rect_t rect;

  rect.x = 0;
  rect.y = 0;
  rect.w = surface->render_surface.width;
  rect.h = surface->render_surface.height;

  return rect;
}

static bool wing_dirty_rect_intersect(const wing_rect_t *a,
                                      const wing_rect_t *b,
                                      wing_rect_t *out)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;

  if (a == NULL || b == NULL || out == NULL || a->w == 0 || a->h == 0 ||
      b->w == 0 || b->h == 0)
    {
      return false;
    }

  ax2 = (int32_t)a->x + a->w;
  ay2 = (int32_t)a->y + a->h;
  bx2 = (int32_t)b->x + b->w;
  by2 = (int32_t)b->y + b->h;

  x1 = a->x > b->x ? a->x : b->x;
  y1 = a->y > b->y ? a->y : b->y;
  x2 = ax2 < bx2 ? ax2 : bx2;
  y2 = ay2 < by2 ? ay2 : by2;

  if (x2 <= x1 || y2 <= y1)
    {
      return false;
    }

  out->x = (int16_t)x1;
  out->y = (int16_t)y1;
  out->w = (uint16_t)(x2 - x1);
  out->h = (uint16_t)(y2 - y1);

  return true;
}

static void wing_dirty_rect_union(const wing_rect_t *a,
                                  const wing_rect_t *b,
                                  wing_rect_t *out)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;

  ax2 = (int32_t)a->x + a->w;
  ay2 = (int32_t)a->y + a->h;
  bx2 = (int32_t)b->x + b->w;
  by2 = (int32_t)b->y + b->h;

  x1 = a->x < b->x ? a->x : b->x;
  y1 = a->y < b->y ? a->y : b->y;
  x2 = ax2 > bx2 ? ax2 : bx2;
  y2 = ay2 > by2 ? ay2 : by2;

  out->x = (int16_t)x1;
  out->y = (int16_t)y1;
  out->w = (uint16_t)(x2 - x1);
  out->h = (uint16_t)(y2 - y1);
}

static bool wing_dirty_rect_can_merge(const wing_rect_t *a,
                                      const wing_rect_t *b)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;

  if (a == NULL || b == NULL || a->w == 0 || a->h == 0 ||
      b->w == 0 || b->h == 0)
    {
      return false;
    }

  ax2 = (int32_t)a->x + a->w;
  ay2 = (int32_t)a->y + a->h;
  bx2 = (int32_t)b->x + b->w;
  by2 = (int32_t)b->y + b->h;

  return a->x <= bx2 && ax2 >= b->x && a->y <= by2 && ay2 >= b->y;
}

static void wing_gui_store_dirty_rect(wing_gui_t *gui,
                                      const wing_rect_t *rect)
{
  uint8_t i;

  if (gui == NULL || rect == NULL || rect->w == 0 || rect->h == 0)
    {
      return;
    }

  for (i = 0; i < gui->dirty_rect_count; i++)
    {
      if (wing_dirty_rect_can_merge(&gui->dirty_rects[i], rect))
        {
          wing_dirty_rect_union(&gui->dirty_rects[i], rect,
                                &gui->dirty_rects[i]);
          gui->dirty_merge_count++;
          return;
        }
    }

  if (gui->dirty_rect_count < WING_GUI_DIRTY_RECT_MAX)
    {
      gui->dirty_rects[gui->dirty_rect_count++] = *rect;
      return;
    }

  gui->dirty_rects[0] = gui->dirty_rect;
  gui->dirty_rect_count = 1;
  gui->dirty_merge_count++;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_gui_invalidate(wing_gui_t *gui)
{
  if (gui != NULL)
    {
      if (wing_dirty_surface_is_valid(gui->surface))
        {
          gui->dirty_rect = wing_dirty_surface_rect(gui->surface);
          gui->has_dirty_rect = true;
          gui->dirty_rects[0] = gui->dirty_rect;
          gui->dirty_rect_count = 1;
          gui->dirty_merge_count = 0;
        }

      gui->dirty = true;
    }
}

void wing_gui_invalidate_rect(wing_gui_t *gui, const wing_rect_t *rect)
{
  wing_rect_t bounds;
  wing_rect_t clipped;

  if (gui == NULL || rect == NULL ||
      !wing_dirty_surface_is_valid(gui->surface))
    {
      return;
    }

  bounds = wing_dirty_surface_rect(gui->surface);
  if (!wing_dirty_rect_intersect(&bounds, rect, &clipped))
    {
      return;
    }

  if (gui->has_dirty_rect)
    {
      wing_dirty_rect_union(&gui->dirty_rect, &clipped, &gui->dirty_rect);
    }
  else
    {
      gui->dirty_rect = clipped;
      gui->has_dirty_rect = true;
    }

  wing_gui_store_dirty_rect(gui, &clipped);
  gui->dirty = true;
}

bool wing_gui_get_dirty_rect(const wing_gui_t *gui, wing_rect_t *rect)
{
  if (gui == NULL || rect == NULL || !gui->has_dirty_rect)
    {
      return false;
    }

  *rect = gui->dirty_rect;
  return true;
}

uint8_t wing_gui_get_dirty_rect_count(const wing_gui_t *gui)
{
  if (gui == NULL || !gui->has_dirty_rect)
    {
      return 0;
    }

  return gui->dirty_rect_count;
}

bool wing_gui_get_dirty_rect_at(const wing_gui_t *gui, uint8_t index,
                                wing_rect_t *rect)
{
  if (gui == NULL || rect == NULL || !gui->has_dirty_rect ||
      index >= gui->dirty_rect_count)
    {
      return false;
    }

  *rect = gui->dirty_rects[index];
  return true;
}

uint16_t wing_gui_get_dirty_merge_count(const wing_gui_t *gui)
{
  return gui == NULL || !gui->has_dirty_rect ? 0 : gui->dirty_merge_count;
}

void wing_gui_clear_dirty(wing_gui_t *gui)
{
  if (gui == NULL)
    {
      return;
    }

  gui->dirty_rect.x = 0;
  gui->dirty_rect.y = 0;
  gui->dirty_rect.w = 0;
  gui->dirty_rect.h = 0;
  gui->dirty_rect_count = 0;
  gui->dirty_merge_count = 0;
  gui->has_dirty_rect = false;
  gui->dirty = false;
}
