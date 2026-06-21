/****************************************************************************
 * apps/graphics/frender/src/fr_backend_framebuffer.c
 *
 * Framebuffer backend — pure dispatch layer.
 * ALL drawing is delegated to nuttx/graphics (fb_draw_* functions).
 * This file contains ZERO pixel manipulation code.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include "fr_backend.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <nuttx/video/fb.h>
#include <nuttx/nx/nxglib.h>

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fr_fb_priv_s
{
  struct fb_planeinfo_s pinfo;  /* Passed to fb_draw_* */
  int      fd;
  size_t   fblen;
  uint16_t xres;
  uint16_t yres;
  int      bpp;        /* 16, 24, or 32 */
  bool     mapped;
};

struct fr_fb_config_s
{
  const char *fb_path;
};

/****************************************************************************
 * Private Helpers
 ****************************************************************************/

/* Pack fr_color_t → RGBA8888 uint32_t (0xRRGGBBAA) */

static uint32_t fr_fb_pack_rgba(fr_color_t c)
{
  return ((uint32_t)c.r << 24) | ((uint32_t)c.g << 16) |
         ((uint32_t)c.b << 8) | c.a;
}

/* Convert fr_rect_t (x,y,w,h) → nxgl_rect_s (pt1, pt2), clipped */

static bool fr_fb_clip(const fr_rect_t *clip, const fr_rect_t *rect,
                        struct nxgl_rect_s *out)
{
  int32_t cx1, cy1, cx2, cy2, rx1, ry1, rx2, ry2, x1, y1, x2, y2;

  cx1 = clip->x;  cy1 = clip->y;
  cx2 = cx1 + clip->w;  cy2 = cy1 + clip->h;
  rx1 = rect->x;  ry1 = rect->y;
  rx2 = rx1 + rect->w;  ry2 = ry1 + rect->h;

  x1 = rx1 > cx1 ? rx1 : cx1;
  y1 = ry1 > cy1 ? ry1 : cy1;
  x2 = rx2 < cx2 ? rx2 : cx2;
  y2 = ry2 < cy2 ? ry2 : cy2;

  if (x2 <= x1 || y2 <= y1)
    {
      out->pt1.x = 0; out->pt1.y = 0;
      out->pt2.x = -1; out->pt2.y = -1;
      return false;
    }

  out->pt1.x = (nxgl_coord_t)x1;
  out->pt1.y = (nxgl_coord_t)y1;
  out->pt2.x = (nxgl_coord_t)(x2 - 1);
  out->pt2.y = (nxgl_coord_t)(y2 - 1);
  return true;
}

/* Convert fr_point_t → nxgl_point_s */

static void fr_fb_pt(const fr_point_t *fp, struct nxgl_point_s *np)
{
  np->x = fp->x; np->y = fp->y;
}

/* Convert fr_quad_t → nxgl_point_s[4] */

static void fr_fb_quad(const fr_quad_t *q, struct nxgl_point_s nq[4])
{
  int i;
  for (i = 0; i < 4; i++)
    { nq[i].x = q->points[i].x; nq[i].y = q->points[i].y; }
}

/* Fill trapezoid via fb_draw — decompose triangle into 1-2 traps */

static void fr_fb_fill_triangle_via_trap(struct fb_planeinfo_s *pinfo,
                                          int bpp,
                                          const fr_point_t tri[3],
                                          uint32_t rgba)
{
  fr_point_t v[3], tmp;
  struct nxgl_trapezoid_s trap;
  struct nxgl_rect_s bounds;

  v[0] = tri[0]; v[1] = tri[1]; v[2] = tri[2];

  /* Sort by Y ascending */
  if (v[0].y > v[1].y) { tmp = v[0]; v[0] = v[1]; v[1] = tmp; }
  if (v[1].y > v[2].y) { tmp = v[1]; v[1] = v[2]; v[2] = tmp; }
  if (v[0].y > v[1].y) { tmp = v[0]; v[0] = v[1]; v[1] = tmp; }

  bounds.pt1.x = 0; bounds.pt1.y = 0;
  bounds.pt2.x = 32767; bounds.pt2.y = 32767;

  /* Top → middle trapezoid */
  if (v[1].y > v[0].y)
    {
      int32_t d_full = v[2].y - v[0].y;
      int32_t lx_mid, rx_mid;

      if (d_full > 0)
        lx_mid = v[0].x + ((v[2].x - v[0].x) * (v[1].y - v[0].y)) / d_full;
      else
        lx_mid = v[0].x;
      rx_mid = v[1].x;

      if (lx_mid > rx_mid) { int32_t t = lx_mid; lx_mid = rx_mid; rx_mid = t; }

      trap.top.x1 = itob16(v[0].x);
      trap.top.x2 = itob16(v[0].x);
      trap.top.y  = v[0].y;
      trap.bot.x1 = itob16(lx_mid);
      trap.bot.x2 = itob16(rx_mid);
      trap.bot.y  = v[1].y;
      nxgl_filltrapezoid_blend(pinfo, &trap, &bounds, rgba, bpp);
    }

  /* Middle → bottom trapezoid */
  if (v[2].y > v[1].y)
    {
      int32_t d_full = v[2].y - v[0].y;
      int32_t lx_mid, rx_mid;

      if (d_full > 0)
        lx_mid = v[0].x + ((v[2].x - v[0].x) * (v[1].y - v[0].y)) / d_full;
      else
        lx_mid = v[1].x;
      rx_mid = v[1].x;

      if (lx_mid > rx_mid) { int32_t t = lx_mid; lx_mid = rx_mid; rx_mid = t; }

      trap.top.x1 = itob16(lx_mid);
      trap.top.x2 = itob16(rx_mid);
      trap.top.y  = v[1].y;
      trap.bot.x1 = itob16(v[2].x);
      trap.bot.x2 = itob16(v[2].x);
      trap.bot.y  = v[2].y;
      nxgl_filltrapezoid_blend(pinfo, &trap, &bounds, rgba, bpp);
    }
}

/****************************************************************************
 * Backend Vtable
 ****************************************************************************/

static int fr_fb_open(void **priv, const void *config)
{
#if defined(CONFIG_VIDEO_FB)
  const struct fr_fb_config_s *cfg = config;
  struct fr_fb_priv_s *fb;
  struct fb_videoinfo_s vinfo;
  const char *fbpath;
  void *fbmem;
  int fd, ret;

  fbpath = (cfg && cfg->fb_path) ? cfg->fb_path : "/dev/fb0";

  fb = calloc(1, sizeof(*fb));
  if (!fb) return -ENOMEM;
  fb->fd = -1;

  fd = open(fbpath, O_RDWR);
  if (fd < 0) { ret = -errno; free(fb); return ret; }

  ret = ioctl(fd, FBIOGET_VIDEOINFO, (unsigned long)&vinfo);
  if (ret < 0) { ret = -errno; close(fd); free(fb); return ret; }

  memset(&fb->pinfo, 0, sizeof(fb->pinfo));
  ret = ioctl(fd, FBIOGET_PLANEINFO, (unsigned long)&fb->pinfo);
  if (ret < 0) { ret = -errno; close(fd); free(fb); return ret; }

  fbmem = mmap(NULL, fb->pinfo.fblen, PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);
  if (fbmem == MAP_FAILED)
    {
      if (!fb->pinfo.fbmem)
        { ret = -errno; close(fd); free(fb); return ret; }
      fbmem = fb->pinfo.fbmem;
      fb->mapped = false;
    }
  else fb->mapped = true;

  fb->pinfo.fbmem = fbmem;
  fb->fd = fd;
  fb->fblen = fb->pinfo.fblen;
  fb->xres = vinfo.xres;
  fb->yres = vinfo.yres;
  fb->bpp = (int)fb->pinfo.bpp;
  if (fb->bpp == 0) fb->bpp = 32;

  *priv = fb;
  return 0;
#else
  return -ENOSYS;
#endif
}

static void fr_fb_close(void *priv)
{
#if defined(CONFIG_VIDEO_FB)
  struct fr_fb_priv_s *fb = priv;
  if (!fb) return;
  if (fb->mapped && fb->pinfo.fbmem)
    munmap(fb->pinfo.fbmem, fb->fblen);
  if (fb->fd >= 0) close(fb->fd);
  free(fb);
#endif
}

static fr_rect_t fr_fb_get_bounds(void *priv)
{
  struct fr_fb_priv_s *fb = priv;
  fr_rect_t r = {0, 0, fb->xres, fb->yres};
  return r;
}

/* --- Each cmd_* function is pure dispatch to nuttx/graphics --- */

static int fr_fb_cmd_clear(void *priv, const fr_rect_t *clip, fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_rect_s r;
  r.pt1.x = clip->x; r.pt1.y = clip->y;
  r.pt2.x = clip->x + clip->w - 1;
  r.pt2.y = clip->y + clip->h - 1;
  nxgl_fillrectangle_blend(&fb->pinfo, &r, fr_fb_pack_rgba(c), fb->bpp);
  return 0;
}

static int fr_fb_cmd_fill_rect(void *priv, const fr_rect_t *clip,
                                const fr_rect_t *rect, fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_rect_s r;
  if (!fr_fb_clip(clip, rect, &r)) return 0;
  nxgl_fillrectangle_blend(&fb->pinfo, &r, fr_fb_pack_rgba(c), fb->bpp);
  return 0;
}

static int fr_fb_cmd_stroke_rect(void *priv, const fr_rect_t *clip,
                                  const fr_rect_t *rect, uint16_t t,
                                  fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  uint32_t rgba = fr_fb_pack_rgba(c);
  struct nxgl_rect_s r;
  fr_rect_t part;
  int bpp = fb->bpp;

  if (t == 0 || rect->w == 0 || rect->h == 0) return 0;
  if (t > rect->w) t = rect->w;
  if (t > rect->h) t = rect->h;

  part = *rect; part.h = t;
  if (fr_fb_clip(clip, &part, &r))
    nxgl_fillrectangle_blend(&fb->pinfo, &r, rgba, bpp);

  part.y = (int16_t)(rect->y + rect->h - t);
  if (fr_fb_clip(clip, &part, &r))
    nxgl_fillrectangle_blend(&fb->pinfo, &r, rgba, bpp);

  part = *rect; part.w = t;
  if (fr_fb_clip(clip, &part, &r))
    nxgl_fillrectangle_blend(&fb->pinfo, &r, rgba, bpp);

  part.x = (int16_t)(rect->x + rect->w - t);
  if (fr_fb_clip(clip, &part, &r))
    nxgl_fillrectangle_blend(&fb->pinfo, &r, rgba, bpp);

  return 0;
}

static int fr_fb_cmd_fill_quad(void *priv, const fr_rect_t *clip,
                                const fr_quad_t *quad, fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_point_s nq[4];
  uint32_t rgba = fr_fb_pack_rgba(c);

  fr_fb_quad(quad, nq);
  nxgl_fillpolygon_blend(&fb->pinfo, nq, 4, rgba, fb->bpp);
  return 0;
}

static int fr_fb_cmd_fill_triangle(void *priv, const fr_rect_t *clip,
                                    const fr_triangle_t *tri, fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_point_s nv[3];
  uint32_t rgba = fr_fb_pack_rgba(c);
  int i;

  for (i = 0; i < 3; i++)
    { nv[i].x = tri->points[i].x; nv[i].y = tri->points[i].y; }

  nxgl_fillpolygon_blend(&fb->pinfo, nv, 3, rgba, fb->bpp);
  return 0;
}

static int fr_fb_cmd_stroke_quad(void *priv, const fr_rect_t *clip,
                                  const fr_quad_t *quad, uint16_t t,
                                  fr_color_t c)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_vector_s vec;
  uint32_t rgba = fr_fb_pack_rgba(c);
  int i;

  if (t == 0) return 0;

  for (i = 0; i < 4; i++)
    {
      int j = (i + 1) & 3;
      vec.pt1.x = quad->points[i].x;
      vec.pt1.y = quad->points[i].y;
      vec.pt2.x = quad->points[j].x;
      vec.pt2.y = quad->points[j].y;
      nxgl_drawline_blend(&fb->pinfo, &vec, (nxgl_coord_t)t, rgba, fb->bpp);
    }
  return 0;
}

static int fr_fb_cmd_blit(void *priv, const fr_rect_t *clip,
                           const fr_surface_t *src, const fr_rect_t *sr,
                           const fr_rect_t *dr, uint8_t ga)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_rect_s dest;

  /* Build NX rect for destination */
  dest.pt1.x = dr->x; dest.pt1.y = dr->y;
  dest.pt2.x = dr->x + dr->w - 1;
  dest.pt2.y = dr->y + dr->h - 1;

  nxgl_blit_scale(&fb->pinfo, &dest,
                     src->pixels,
                     (int)sr->w, (int)sr->h,
                     (int)src->stride,
                     fb->bpp, ga);
  return 0;
}

static int fr_fb_cmd_blit_quad(void *priv, const fr_rect_t *clip,
                                const fr_surface_t *src,
                                const fr_rect_t *sr,
                                const fr_quad_t *q, uint8_t ga)
{
  struct fr_fb_priv_s *fb = priv;
  struct nxgl_point_s nq[4];

  fr_fb_quad(q, nq);
  nxgl_blit_quad(&fb->pinfo, src->pixels,
                    (int)sr->w, (int)sr->h, (int)src->stride,
                    nq, fb->bpp, ga);
  return 0;
}

static int fr_fb_present(void *priv, const fr_rect_t *damage)
{
#if defined(CONFIG_FB_UPDATE)
  struct fr_fb_priv_s *fb = priv;
  struct fb_area_s area;

  if (damage)
    {
      area.x = damage->x < 0 ? 0 : damage->x;
      area.y = damage->y < 0 ? 0 : damage->y;
      area.w = damage->w;
      area.h = damage->h;
    }
  else
    { area.x = 0; area.y = 0; area.w = fb->xres; area.h = fb->yres; }

  if (ioctl(fb->fd, FBIO_UPDATE, (unsigned long)&area) < 0)
    return -errno;
#endif
  return 0;
}

static const struct fr_backend_ops_s g_fr_fb_ops =
{
  .name              = "framebuffer",
  .open              = fr_fb_open,
  .close             = fr_fb_close,
  .get_bounds        = fr_fb_get_bounds,
  .cmd_clear         = fr_fb_cmd_clear,
  .cmd_fill_rect     = fr_fb_cmd_fill_rect,
  .cmd_stroke_rect   = fr_fb_cmd_stroke_rect,
  .cmd_fill_quad     = fr_fb_cmd_fill_quad,
  .cmd_fill_triangle = fr_fb_cmd_fill_triangle,
  .cmd_stroke_quad   = fr_fb_cmd_stroke_quad,
  .cmd_blit          = fr_fb_cmd_blit,
  .cmd_blit_quad     = fr_fb_cmd_blit_quad,
  .present           = fr_fb_present,
};

const struct fr_backend_ops_s *fr_backend_ops_framebuffer(void)
{
  return &g_fr_fb_ops;
}
