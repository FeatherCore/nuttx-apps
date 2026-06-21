/****************************************************************************
 * apps/graphics/frender/src/fr_backend_nx.c
 *
 * NX backend: translates FRender commands to NX server drawing API calls.
 * Requires an already-running NX server and an open window.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_NX

#include "fr_backend.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nx.h>

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fr_nx_priv_s
{
  NXWINDOW         hwnd;     /* NX window handle */
  uint16_t         width;    /* Window width */
  uint16_t         height;   /* Window height */
  uint8_t          bpp;      /* Bits per pixel of the underlying fb */
};

/* Config passed through open() */
struct fr_nx_config_s
{
  NXWINDOW  hwnd;
  uint16_t  width;
  uint16_t  height;
  uint8_t   bpp;
};

/****************************************************************************
 * Private Helpers
 ****************************************************************************/

/* Convert FRender rect (x,y,w,h) to NX rect (pt1, pt2) */
static void fr_nx_convert_rect(const fr_rect_t *src, struct nxgl_rect_s *dst)
{
  dst->pt1.x = src->x;
  dst->pt1.y = src->y;
  dst->pt2.x = src->x + src->w - 1;
  dst->pt2.y = src->y + src->h - 1;
}

/* Clip two rects before converting */
static void fr_nx_convert_clipped_rect(const fr_rect_t *clip,
                                        const fr_rect_t *rect,
                                        struct nxgl_rect_s *dst)
{
  fr_rect_t clipped;
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;

  /* Compute intersection of clip and rect inline */
  ax2 = (int32_t)clip->x + clip->w;
  ay2 = (int32_t)clip->y + clip->h;
  bx2 = (int32_t)rect->x + rect->w;
  by2 = (int32_t)rect->y + rect->h;

  x1 = clip->x > rect->x ? clip->x : rect->x;
  y1 = clip->y > rect->y ? clip->y : rect->y;
  x2 = ax2 < bx2 ? ax2 : bx2;
  y2 = ay2 < by2 ? ay2 : by2;

  if (x2 <= x1 || y2 <= y1)
    {
      /* Empty rect: set to degenerate */
      dst->pt1.x = 0;
      dst->pt1.y = 0;
      dst->pt2.x = -1;
      dst->pt2.y = -1;
      return;
    }

  dst->pt1.x = (nxgl_coord_t)x1;
  dst->pt1.y = (nxgl_coord_t)y1;
  dst->pt2.x = (nxgl_coord_t)(x2 - 1);
  dst->pt2.y = (nxgl_coord_t)(y2 - 1);
}

/* Convert fr_color_t to nxgl_mxpixel_t. For single-plane, the color is
 * packed into the format expected by the underlying framebuffer.
 * We pack as RGBA8888 for simplicity. */
static void fr_nx_color(fr_color_t color, nxgl_mxpixel_t nx_color[1])
{
  nx_color[0] = ((nxgl_mxpixel_t)color.r << 16) |
                ((nxgl_mxpixel_t)color.g << 8) |
                (nxgl_mxpixel_t)color.b;
}

/****************************************************************************
 * Backend vtable implementations
 ****************************************************************************/

static int fr_nx_open(void **priv, const void *config)
{
  const struct fr_nx_config_s *cfg;
  struct fr_nx_priv_s *nx;

  cfg = (const struct fr_nx_config_s *)config;
  if (cfg == NULL || cfg->hwnd == NULL)
    {
      return -EINVAL;
    }

  nx = (struct fr_nx_priv_s *)calloc(1, sizeof(*nx));
  if (nx == NULL)
    {
      return -ENOMEM;
    }

  nx->hwnd = cfg->hwnd;
  nx->width = cfg->width;
  nx->height = cfg->height;
  nx->bpp = cfg->bpp;

  *priv = nx;
  return 0;
}

static void fr_nx_close(void *priv)
{
  free(priv);
}

static fr_rect_t fr_nx_get_bounds(void *priv)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  fr_rect_t bounds;

  bounds.x = 0;
  bounds.y = 0;
  bounds.w = nx->width;
  bounds.h = nx->height;
  return bounds;
}

static int fr_nx_cmd_clear(void *priv, const fr_rect_t *clip, fr_color_t color)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_rect_s nx_rect;
  nxgl_mxpixel_t nx_color[1];

  nx_rect.pt1.x = clip->x;
  nx_rect.pt1.y = clip->y;
  nx_rect.pt2.x = clip->x + clip->w - 1;
  nx_rect.pt2.y = clip->y + clip->h - 1;

  fr_nx_color(color, nx_color);
  return nx_fill(nx->hwnd, &nx_rect, nx_color);
}

static int fr_nx_cmd_fill_rect(void *priv, const fr_rect_t *clip,
                                const fr_rect_t *rect, fr_color_t color)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_rect_s nx_rect;
  nxgl_mxpixel_t nx_color[1];

  fr_nx_convert_clipped_rect(clip, rect, &nx_rect);
  if (nx_rect.pt2.x < nx_rect.pt1.x || nx_rect.pt2.y < nx_rect.pt1.y)
    {
      return 0; /* Empty intersection */
    }

  fr_nx_color(color, nx_color);
  return nx_fill(nx->hwnd, &nx_rect, nx_color);
}

static int fr_nx_cmd_stroke_rect(void *priv, const fr_rect_t *clip,
                                  const fr_rect_t *rect, uint16_t thickness,
                                  fr_color_t color)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_vector_s vec;
  nxgl_mxpixel_t nx_color[1];
  uint8_t caps;
  int ret;

  if (rect->w == 0 || rect->h == 0 || thickness == 0)
    {
      return 0;
    }

  fr_nx_color(color, nx_color);
  caps = NX_LINECAP_NONE;

  /* Top edge: (x, y) -> (x + w - 1, y) */
  vec.pt1.x = rect->x;
  vec.pt1.y = rect->y;
  vec.pt2.x = rect->x + rect->w - 1;
  vec.pt2.y = rect->y;
  ret = nx_drawline(nx->hwnd, &vec, (nxgl_coord_t)thickness, nx_color, caps);
  if (ret < 0) return ret;

  /* Bottom edge */
  vec.pt1.x = rect->x;
  vec.pt1.y = rect->y + rect->h - 1;
  vec.pt2.x = rect->x + rect->w - 1;
  vec.pt2.y = rect->y + rect->h - 1;
  ret = nx_drawline(nx->hwnd, &vec, (nxgl_coord_t)thickness, nx_color, caps);
  if (ret < 0) return ret;

  /* Left edge */
  vec.pt1.x = rect->x;
  vec.pt1.y = rect->y;
  vec.pt2.x = rect->x;
  vec.pt2.y = rect->y + rect->h - 1;
  ret = nx_drawline(nx->hwnd, &vec, (nxgl_coord_t)thickness, nx_color, caps);
  if (ret < 0) return ret;

  /* Right edge */
  vec.pt1.x = rect->x + rect->w - 1;
  vec.pt1.y = rect->y;
  vec.pt2.x = rect->x + rect->w - 1;
  vec.pt2.y = rect->y + rect->h - 1;
  return nx_drawline(nx->hwnd, &vec, (nxgl_coord_t)thickness, nx_color, caps);
}

static int fr_nx_cmd_stroke_quad(void *priv, const fr_rect_t *clip,
                                  const fr_quad_t *quad, uint16_t thickness,
                                  fr_color_t color)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_vector_s vec;
  nxgl_mxpixel_t nx_color[1];
  uint8_t caps;
  uint8_t i;
  uint8_t next;
  int ret;

  if (quad == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  fr_nx_color(color, nx_color);
  caps = NX_LINECAP_NONE;

  for (i = 0; i < 4; i++)
    {
      next = (uint8_t)((i + 1) & 3);
      vec.pt1.x = quad->points[i].x;
      vec.pt1.y = quad->points[i].y;
      vec.pt2.x = quad->points[next].x;
      vec.pt2.y = quad->points[next].y;

      ret = nx_drawline(nx->hwnd, &vec, (nxgl_coord_t)thickness,
                        nx_color, caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static int fr_nx_cmd_fill_quad(void *priv, const fr_rect_t *clip,
                                const fr_quad_t *quad, fr_color_t color)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_trapezoid_s trap;
  struct nxgl_rect_s nx_bounds;
  nxgl_mxpixel_t nx_color[1];
  int32_t top_y;
  int32_t bot_y;
  int32_t mid_y;
  int32_t top_xs[4];
  int32_t bot_xs[4];
  int32_t mid_xs[4];
  uint8_t top_count;
  uint8_t bot_count;
  uint8_t mid_count;
  uint8_t i;
  uint8_t j;
  int ret;

  if (quad == NULL)
    {
      return -EINVAL;
    }

  /* Find top, middle, bottom Y and get intersection X values */
  top_y = quad->points[0].y;
  bot_y = quad->points[0].y;
  for (i = 1; i < 4; i++)
    {
      if (quad->points[i].y < top_y) top_y = quad->points[i].y;
      if (quad->points[i].y > bot_y) bot_y = quad->points[i].y;
    }

  mid_y = (top_y + bot_y) / 2;
  if (mid_y == top_y) mid_y++;
  if (mid_y == bot_y) mid_y = bot_y - 1;

  /* Get X intersections at top, middle, and bottom scanlines */
  top_count = 0;
  for (i = 0; i < 4; i++)
    {
      j = (uint8_t)((i + 1) & 3);
      if (fr_pixel_write /* edge intersect: copied inline for simplicity */)
        {
          /* We need the edge_intersect logic here. For NX backend,
           * decompose to 2 trapezoids using nxgl_splitline-like logic.
           * Simple approach: convert quad into two triangles, fill each
           * as a trapezoid. */
        }
    }

  /* Simplified: decompose convex quad into 2 trapezoids at mid_y.
   * Top trapezoid: top_y..mid_y. Bottom trapezoid: mid_y..bot_y. */
  fr_nx_color(color, nx_color);
  fr_nx_convert_rect(clip, &nx_bounds);

  /* Get left/right edges at top, middle, bottom scanlines */
  top_count = 0; bot_count = 0; mid_count = 0;
  for (i = 0; i < 4; i++)
    {
      j = (uint8_t)((i + 1) & 3);
      int32_t x_val;

      /* Check top_y intersection */
      if (quad->points[i].y == top_y || quad->points[j].y == top_y)
        {
          if (quad->points[i].y <= top_y && quad->points[j].y > top_y)
            {
              int32_t dy = quad->points[j].y - quad->points[i].y;
              x_val = quad->points[i].x +
                      ((quad->points[j].x - quad->points[i].x) *
                       (top_y - quad->points[i].y)) / dy;
              top_xs[top_count++] = x_val;
            }
          if (quad->points[i].y == top_y) top_xs[top_count++] =
            quad->points[i].x;
        }

      /* Check bot_y */
      /* ... similar logic ... */
    }

  /* For now: return -ENOSYS for complex quad operations.
   * The NX backend is mainly for FILL_RECT/STROKE_RECT/BLIT.
   * Quad/triangle operations should use the framebuffer backend. */
  (void)top_xs;
  (void)bot_xs;
  (void)mid_xs;
  (void)mid_count;
  (void)trap;
  (void)ret;
  return -ENOSYS;
}

static int fr_nx_cmd_fill_triangle(void *priv, const fr_rect_t *clip,
                                    const fr_triangle_t *triangle,
                                    fr_color_t color)
{
  /* Decompose triangle into 1-2 trapezoids, call nx_filltrapezoid().
   * Not yet implemented — use framebuffer backend for triangles. */
  (void)priv;
  (void)clip;
  (void)triangle;
  (void)color;
  return -ENOSYS;
}

static int fr_nx_cmd_blit(void *priv, const fr_rect_t *clip,
                           const fr_surface_t *source,
                           const fr_rect_t *src_rect,
                           const fr_rect_t *dst_rect, uint8_t global_alpha)
{
  struct fr_nx_priv_s *nx = (struct fr_nx_priv_s *)priv;
  struct nxgl_rect_s nx_dest;
  struct nxgl_point_s nx_origin;
  const void *src_plane[1];

  (void)clip;
  (void)global_alpha; /* NX nx_bitmap does not support per-call alpha */

  if (source == NULL || !source->pixels)
    {
      return -EINVAL;
    }

  fr_nx_convert_rect(dst_rect, &nx_dest);
  nx_origin.x = src_rect->x;
  nx_origin.y = src_rect->y;

  src_plane[0] = source->pixels;
  return nx_bitmap(nx->hwnd, &nx_dest, src_plane, &nx_origin,
                   (unsigned int)(source->stride * 4));
}

static int fr_nx_cmd_blit_quad(void *priv, const fr_rect_t *clip,
                                const fr_surface_t *source,
                                const fr_rect_t *src_rect,
                                const fr_quad_t *dst_quad,
                                uint8_t global_alpha)
{
  /* NX has no perspective blit. */
  (void)priv;
  (void)clip;
  (void)source;
  (void)src_rect;
  (void)dst_quad;
  (void)global_alpha;
  return -ENOSYS;
}

static int fr_nx_present(void *priv, const fr_rect_t *damage)
{
  /* NX compositor handles presentation. */
  (void)priv;
  (void)damage;
  return 0;
}

/****************************************************************************
 * Public Data
 ****************************************************************************/

static const struct fr_backend_ops_s g_fr_nx_backend_ops =
{
  .name              = "nx",
  .open              = fr_nx_open,
  .close             = fr_nx_close,
  .get_bounds        = fr_nx_get_bounds,
  .cmd_clear         = fr_nx_cmd_clear,
  .cmd_fill_rect     = fr_nx_cmd_fill_rect,
  .cmd_stroke_rect   = fr_nx_cmd_stroke_rect,
  .cmd_fill_quad     = fr_nx_cmd_fill_quad,
  .cmd_fill_triangle = fr_nx_cmd_fill_triangle,
  .cmd_stroke_quad   = fr_nx_cmd_stroke_quad,
  .cmd_blit          = fr_nx_cmd_blit,
  .cmd_blit_quad     = fr_nx_cmd_blit_quad,
  .present           = fr_nx_present,
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

const struct fr_backend_ops_s *fr_backend_ops_nx(void)
{
  return &g_fr_nx_backend_ops;
}

#endif /* CONFIG_NX */
