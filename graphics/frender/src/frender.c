/****************************************************************************
 * apps/graphics/frender/src/frender.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <nuttx/video/fb.h>

#include <frender/frender.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FR_CLIP_STACK_DEPTH 8
#define FR_BACKEND_REGISTRY_MAX 8

/****************************************************************************
 * Private Data
 ****************************************************************************/

static fr_backend_caps_t g_backend_registry[FR_BACKEND_REGISTRY_MAX];
static uint8_t g_backend_registry_count;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool fr_surface_valid(const fr_surface_t *surface)
{
  return surface != NULL && surface->pixels != NULL &&
         surface->width > 0 && surface->height > 0 &&
         surface->stride >= surface->width &&
         surface->format == FR_FORMAT_RGBA8888;
}

static fr_rect_t fr_surface_bounds(const fr_surface_t *surface)
{
  fr_rect_t rect;

  rect.x = 0;
  rect.y = 0;
  rect.w = surface->width;
  rect.h = surface->height;

  return rect;
}

static bool fr_rect_intersect(const fr_rect_t *a, const fr_rect_t *b,
                              fr_rect_t *out)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;
  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;

  if (a == NULL || b == NULL || out == NULL ||
      a->w == 0 || a->h == 0 || b->w == 0 || b->h == 0)
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

static uint8_t fr_blend_u8(uint8_t src, uint8_t dst, uint8_t alpha)
{
  uint16_t value;

  value = (uint16_t)src * alpha + (uint16_t)dst * (255 - alpha) + 127;
  return (uint8_t)(value / 255);
}

static int32_t fr_abs_i32(int32_t value)
{
  return value < 0 ? -value : value;
}

static uint32_t fr_color_blend_rgba8888(fr_color_t color, uint32_t dst)
{
  uint8_t dst_r;
  uint8_t dst_g;
  uint8_t dst_b;
  uint8_t dst_a;
  uint8_t out_a;

  if (color.a == 255)
    {
      return ((uint32_t)color.r << 24) | ((uint32_t)color.g << 16) |
             ((uint32_t)color.b << 8) | color.a;
    }

  if (color.a == 0)
    {
      return dst;
    }

  dst_r = (uint8_t)(dst >> 24);
  dst_g = (uint8_t)(dst >> 16);
  dst_b = (uint8_t)(dst >> 8);
  dst_a = (uint8_t)dst;
  out_a = (uint8_t)(color.a + (((uint16_t)dst_a * (255 - color.a) +
                                127) / 255));

  return ((uint32_t)fr_blend_u8(color.r, dst_r, color.a) << 24) |
         ((uint32_t)fr_blend_u8(color.g, dst_g, color.a) << 16) |
         ((uint32_t)fr_blend_u8(color.b, dst_b, color.a) << 8) |
         out_a;
}

static int fr_command_append(fr_command_list_t *list,
                             const fr_command_t *command)
{
  if (list == NULL || command == NULL || list->commands == NULL)
    {
      return -EINVAL;
    }

  if (list->count >= list->capacity)
    {
      list->overflowed = true;
      return -ENOSPC;
    }

  list->commands[list->count] = *command;
  list->count++;
  return 0;
}

static int fr_fill_rect_clipped(fr_surface_t *surface, const fr_rect_t *clip,
                                const fr_rect_t *rect, fr_color_t color)
{
  fr_rect_t bounds;
  fr_rect_t clipped;
  uint32_t *pixels;
  uint32_t *row;
  uint16_t x;
  uint16_t y;

  bounds = fr_surface_bounds(surface);
  if (!fr_rect_intersect(&bounds, rect, &clipped) ||
      !fr_rect_intersect(clip, &clipped, &clipped))
    {
      return 0;
    }

  pixels = (uint32_t *)surface->pixels;

  for (y = 0; y < clipped.h; y++)
    {
      row = pixels + ((uint32_t)(clipped.y + y) * surface->stride) +
            clipped.x;

      for (x = 0; x < clipped.w; x++)
        {
          row[x] = fr_color_blend_rgba8888(color, row[x]);
        }
    }

  return 0;
}

static int fr_stroke_rect_clipped(fr_surface_t *surface, const fr_rect_t *clip,
                                  const fr_rect_t *rect, uint16_t thickness,
                                  fr_color_t color)
{
  fr_rect_t part;
  int ret;

  if (rect->w == 0 || rect->h == 0 || thickness == 0)
    {
      return 0;
    }

  if (thickness > rect->w)
    {
      thickness = rect->w;
    }

  if (thickness > rect->h)
    {
      thickness = rect->h;
    }

  part.x = rect->x;
  part.y = rect->y;
  part.w = rect->w;
  part.h = thickness;
  ret = fr_fill_rect_clipped(surface, clip, &part, color);
  if (ret < 0)
    {
      return ret;
    }

  part.y = (int16_t)(rect->y + rect->h - thickness);
  ret = fr_fill_rect_clipped(surface, clip, &part, color);
  if (ret < 0)
    {
      return ret;
    }

  part.x = rect->x;
  part.y = rect->y;
  part.w = thickness;
  part.h = rect->h;
  ret = fr_fill_rect_clipped(surface, clip, &part, color);
  if (ret < 0)
    {
      return ret;
    }

  part.x = (int16_t)(rect->x + rect->w - thickness);
  return fr_fill_rect_clipped(surface, clip, &part, color);
}

static bool fr_quad_edge_intersect_y(const fr_point_t *a,
                                     const fr_point_t *b,
                                     int32_t y, int32_t *x)
{
  int32_t dy;

  if (a == NULL || b == NULL || x == NULL)
    {
      return false;
    }

  if (a->y == b->y)
    {
      return false;
    }

  if ((y < a->y && y < b->y) || (y >= a->y && y >= b->y))
    {
      return false;
    }

  dy = (int32_t)b->y - a->y;
  *x = a->x + (((int32_t)b->x - a->x) * (y - a->y)) / dy;
  return true;
}

static int fr_fill_quad_clipped(fr_surface_t *surface, const fr_rect_t *clip,
                                const fr_quad_t *quad, fr_color_t color)
{
  fr_rect_t line;
  int32_t min_y;
  int32_t max_y;
  int32_t xs[4];
  int32_t left;
  int32_t right;
  int32_t y;
  uint8_t count;
  uint8_t i;
  uint8_t j;
  int ret;

  if (quad == NULL)
    {
      return -EINVAL;
    }

  min_y = quad->points[0].y;
  max_y = quad->points[0].y;
  for (i = 1; i < 4; i++)
    {
      if (quad->points[i].y < min_y)
        {
          min_y = quad->points[i].y;
        }

      if (quad->points[i].y > max_y)
        {
          max_y = quad->points[i].y;
        }
    }

  for (y = min_y; y <= max_y; y++)
    {
      count = 0;
      for (i = 0; i < 4; i++)
        {
          j = (uint8_t)((i + 1) & 3);
          if (fr_quad_edge_intersect_y(&quad->points[i], &quad->points[j],
                                       y, &xs[count]))
            {
              count++;
            }
        }

      if (count < 2)
        {
          continue;
        }

      left = xs[0];
      right = xs[0];
      for (i = 1; i < count; i++)
        {
          if (xs[i] < left)
            {
              left = xs[i];
            }

          if (xs[i] > right)
            {
              right = xs[i];
            }
        }

      if (right < left)
        {
          int32_t tmp;

          tmp = left;
          left = right;
          right = tmp;
        }

      line.x = (int16_t)left;
      line.y = (int16_t)y;
      line.w = (uint16_t)(right - left + 1);
      line.h = 1;

      ret = fr_fill_rect_clipped(surface, clip, &line, color);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static int fr_fill_triangle_clipped(fr_surface_t *surface,
                                    const fr_rect_t *clip,
                                    const fr_triangle_t *triangle,
                                    fr_color_t color)
{
  fr_rect_t line;
  int32_t min_y;
  int32_t max_y;
  int32_t xs[3];
  int32_t left;
  int32_t right;
  int32_t y;
  uint8_t count;
  uint8_t i;
  uint8_t j;
  int ret;

  if (triangle == NULL)
    {
      return -EINVAL;
    }

  min_y = triangle->points[0].y;
  max_y = triangle->points[0].y;
  for (i = 1; i < 3; i++)
    {
      if (triangle->points[i].y < min_y)
        {
          min_y = triangle->points[i].y;
        }

      if (triangle->points[i].y > max_y)
        {
          max_y = triangle->points[i].y;
        }
    }

  for (y = min_y; y <= max_y; y++)
    {
      count = 0;
      for (i = 0; i < 3; i++)
        {
          j = (uint8_t)((i + 1) % 3);
          if (fr_quad_edge_intersect_y(&triangle->points[i],
                                       &triangle->points[j], y,
                                       &xs[count]))
            {
              count++;
            }
        }

      if (count < 2)
        {
          continue;
        }

      left = xs[0];
      right = xs[0];
      for (i = 1; i < count; i++)
        {
          if (xs[i] < left)
            {
              left = xs[i];
            }

          if (xs[i] > right)
            {
              right = xs[i];
            }
        }

      if (right < left)
        {
          int32_t tmp;

          tmp = left;
          left = right;
          right = tmp;
        }

      line.x = (int16_t)left;
      line.y = (int16_t)y;
      line.w = (uint16_t)(right - left + 1);
      line.h = 1;

      ret = fr_fill_rect_clipped(surface, clip, &line, color);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static int fr_stroke_line_clipped(fr_surface_t *surface,
                                  const fr_rect_t *clip,
                                  const fr_point_t *a,
                                  const fr_point_t *b,
                                  uint16_t thickness,
                                  fr_color_t color)
{
  fr_rect_t dot;
  int32_t dx;
  int32_t dy;
  int32_t err;
  int32_t e2;
  int32_t sx;
  int32_t sy;
  int32_t x;
  int32_t y;
  int32_t half;
  int ret;

  if (a == NULL || b == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  x = a->x;
  y = a->y;
  dx = fr_abs_i32((int32_t)b->x - a->x);
  dy = -fr_abs_i32((int32_t)b->y - a->y);
  sx = a->x < b->x ? 1 : -1;
  sy = a->y < b->y ? 1 : -1;
  err = dx + dy;
  half = thickness / 2;

  while (true)
    {
      dot.x = (int16_t)(x - half);
      dot.y = (int16_t)(y - half);
      dot.w = thickness;
      dot.h = thickness;

      ret = fr_fill_rect_clipped(surface, clip, &dot, color);
      if (ret < 0)
        {
          return ret;
        }

      if (x == b->x && y == b->y)
        {
          break;
        }

      e2 = err * 2;
      if (e2 >= dy)
        {
          err += dy;
          x += sx;
        }

      if (e2 <= dx)
        {
          err += dx;
          y += sy;
        }
    }

  return 0;
}

static int fr_stroke_quad_clipped(fr_surface_t *surface,
                                  const fr_rect_t *clip,
                                  const fr_quad_t *quad,
                                  uint16_t thickness,
                                  fr_color_t color)
{
  uint8_t i;
  uint8_t next;
  int ret;

  if (quad == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < 4; i++)
    {
      next = (uint8_t)((i + 1) & 3);
      ret = fr_stroke_line_clipped(surface, clip,
                                   &quad->points[i],
                                   &quad->points[next],
                                   thickness, color);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

static bool fr_source_rect_valid(const fr_surface_t *surface,
                                 const fr_rect_t *rect)
{
  int32_t x2;
  int32_t y2;

  if (!fr_surface_valid(surface) || rect == NULL || rect->w == 0 ||
      rect->h == 0 || rect->x < 0 || rect->y < 0)
    {
      return false;
    }

  x2 = (int32_t)rect->x + rect->w;
  y2 = (int32_t)rect->y + rect->h;
  return x2 <= surface->width && y2 <= surface->height;
}

static int fr_blit_clipped(fr_surface_t *surface, const fr_rect_t *clip,
                           const fr_surface_t *source,
                           const fr_rect_t *src_rect,
                           const fr_rect_t *dst_rect,
                           uint8_t global_alpha)
{
  const uint32_t *src_pixels;
  uint32_t *dst_pixels;
  uint32_t *dst_row;
  fr_color_t color;
  fr_rect_t bounds;
  fr_rect_t clipped;
  uint32_t src_pixel;
  uint16_t x;
  uint16_t y;
  int32_t dst_x;
  int32_t dst_y;
  int32_t src_x;
  int32_t src_y;

  if (!fr_surface_valid(surface) || !fr_source_rect_valid(source, src_rect) ||
      dst_rect == NULL || dst_rect->w == 0 || dst_rect->h == 0)
    {
      return -EINVAL;
    }

  bounds = fr_surface_bounds(surface);
  if (!fr_rect_intersect(&bounds, dst_rect, &clipped) ||
      !fr_rect_intersect(clip, &clipped, &clipped))
    {
      return 0;
    }

  src_pixels = (const uint32_t *)source->pixels;
  dst_pixels = (uint32_t *)surface->pixels;

  for (y = 0; y < clipped.h; y++)
    {
      dst_y = clipped.y + y;
      src_y = src_rect->y +
              (((dst_y - dst_rect->y) * (int32_t)src_rect->h) /
               dst_rect->h);
      dst_row = dst_pixels + ((uint32_t)dst_y * surface->stride) +
                clipped.x;

      for (x = 0; x < clipped.w; x++)
        {
          dst_x = clipped.x + x;
          src_x = src_rect->x +
                  (((dst_x - dst_rect->x) * (int32_t)src_rect->w) /
                   dst_rect->w);
          src_pixel = src_pixels[((uint32_t)src_y * source->stride) +
                                 src_x];

          color.r = (uint8_t)(src_pixel >> 24);
          color.g = (uint8_t)(src_pixel >> 16);
          color.b = (uint8_t)(src_pixel >> 8);
          color.a = (uint8_t)src_pixel;
          color.a = (uint8_t)(((uint16_t)color.a * global_alpha + 127) /
                              255);
          dst_row[x] = fr_color_blend_rgba8888(color, dst_row[x]);
        }
    }

  return 0;
}

static int fr_blit_quad_clipped(fr_surface_t *surface, const fr_rect_t *clip,
                                const fr_surface_t *source,
                                const fr_rect_t *src_rect,
                                const fr_quad_t *dst_quad,
                                uint8_t global_alpha)
{
  const uint32_t *src_pixels;
  uint32_t *dst_pixels;
  fr_color_t color;
  fr_rect_t bounds;
  int32_t min_y;
  int32_t max_y;
  int32_t xs[4];
  int32_t left;
  int32_t right;
  int32_t src_x;
  int32_t src_y;
  int32_t y_span;
  int32_t x_span;
  uint32_t src_pixel;
  uint8_t count;
  uint8_t i;
  uint8_t j;
  int32_t x;
  int32_t y;

  if (!fr_surface_valid(surface) ||
      !fr_source_rect_valid(source, src_rect) || dst_quad == NULL)
    {
      return -EINVAL;
    }

  src_pixels = (const uint32_t *)source->pixels;
  dst_pixels = (uint32_t *)surface->pixels;
  bounds = fr_surface_bounds(surface);
  min_y = dst_quad->points[0].y;
  max_y = dst_quad->points[0].y;
  for (i = 1; i < 4; i++)
    {
      if (dst_quad->points[i].y < min_y)
        {
          min_y = dst_quad->points[i].y;
        }

      if (dst_quad->points[i].y > max_y)
        {
          max_y = dst_quad->points[i].y;
        }
    }

  y_span = max_y - min_y;
  if (y_span <= 0)
    {
      y_span = 1;
    }

  for (y = min_y; y <= max_y; y++)
    {
      count = 0;
      for (i = 0; i < 4; i++)
        {
          j = (uint8_t)((i + 1) & 3);
          if (fr_quad_edge_intersect_y(&dst_quad->points[i],
                                       &dst_quad->points[j], y,
                                       &xs[count]))
            {
              count++;
            }
        }

      if (count < 2 || y < bounds.y || y >= (int32_t)bounds.y + bounds.h ||
          y < clip->y || y >= (int32_t)clip->y + clip->h)
        {
          continue;
        }

      left = xs[0];
      right = xs[0];
      for (i = 1; i < count; i++)
        {
          if (xs[i] < left)
            {
              left = xs[i];
            }

          if (xs[i] > right)
            {
              right = xs[i];
            }
        }

      if (right < left)
        {
          int32_t tmp;

          tmp = left;
          left = right;
          right = tmp;
        }

      if (left < bounds.x)
        {
          left = bounds.x;
        }

      if (left < clip->x)
        {
          left = clip->x;
        }

      if (right >= (int32_t)bounds.x + bounds.w)
        {
          right = (int32_t)bounds.x + bounds.w - 1;
        }

      if (right >= (int32_t)clip->x + clip->w)
        {
          right = (int32_t)clip->x + clip->w - 1;
        }

      x_span = right - left;
      if (x_span <= 0)
        {
          x_span = 1;
        }

      src_y = src_rect->y +
              (((y - min_y) * (int32_t)(src_rect->h - 1)) / y_span);
      for (x = left; x <= right; x++)
        {
          src_x = src_rect->x +
                  (((x - left) * (int32_t)(src_rect->w - 1)) / x_span);
          src_pixel = src_pixels[((uint32_t)src_y * source->stride) +
                                 src_x];

          color.r = (uint8_t)(src_pixel >> 24);
          color.g = (uint8_t)(src_pixel >> 16);
          color.b = (uint8_t)(src_pixel >> 8);
          color.a = (uint8_t)src_pixel;
          color.a = (uint8_t)(((uint16_t)color.a * global_alpha + 127) /
                              255);
          dst_pixels[((uint32_t)y * surface->stride) + x] =
            fr_color_blend_rgba8888(
              color, dst_pixels[((uint32_t)y * surface->stride) + x]);
        }
    }

  return 0;
}

static uint16_t fr_color_to_rgb565(uint32_t rgba)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;

  r = (uint8_t)(rgba >> 24);
  g = (uint8_t)(rgba >> 16);
  b = (uint8_t)(rgba >> 8);

  return (uint16_t)(((uint16_t)(r & 0xf8) << 8) |
                    ((uint16_t)(g & 0xfc) << 3) |
                    ((uint16_t)b >> 3));
}

static uint32_t fr_color_to_rgb32(uint32_t rgba)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;

  r = (uint8_t)(rgba >> 24);
  g = (uint8_t)(rgba >> 16);
  b = (uint8_t)(rgba >> 8);

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static uint32_t fr_color_to_rgba32(uint32_t rgba)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  r = (uint8_t)(rgba >> 24);
  g = (uint8_t)(rgba >> 16);
  b = (uint8_t)(rgba >> 8);
  a = (uint8_t)rgba;

  return ((uint32_t)a << 24) | ((uint32_t)r << 16) |
         ((uint32_t)g << 8) | b;
}

static int fr_fb_present_pixel(uint8_t *dst, uint8_t fmt, uint32_t rgba)
{
  switch (fmt)
    {
      case FB_FMT_RGB16_565:
        *((uint16_t *)dst) = fr_color_to_rgb565(rgba);
        return 0;

      case FB_FMT_RGB24:
        dst[0] = (uint8_t)(rgba >> 24);
        dst[1] = (uint8_t)(rgba >> 16);
        dst[2] = (uint8_t)(rgba >> 8);
        return 0;

      case FB_FMT_RGB32:
      case FB_FMT_RGBT32:
        *((uint32_t *)dst) = fr_color_to_rgb32(rgba);
        return 0;

      case FB_FMT_RGBA32:
        *((uint32_t *)dst) = fr_color_to_rgba32(rgba);
        return 0;

      default:
        return -ENOSYS;
    }
}

static uint8_t fr_fb_bytes_per_pixel(const fr_fb_presenter_t *presenter)
{
  if (presenter->bpp == 0)
    {
      return 0;
    }

  return (uint8_t)((presenter->bpp + 7) / 8);
}

static uint32_t fr_fb_format_caps(uint8_t fmt)
{
  switch (fmt)
    {
      case FB_FMT_RGB16_565:
        return FR_FORMAT_CAP_RGB565;

      case FB_FMT_RGB24:
        return FR_FORMAT_CAP_RGB24;

      case FB_FMT_RGB32:
        return FR_FORMAT_CAP_RGB32;

      case FB_FMT_RGBT32:
        return FR_FORMAT_CAP_RGBT32;

      case FB_FMT_RGBA32:
        return FR_FORMAT_CAP_RGBA32;

      default:
        return FR_FORMAT_CAP_NONE;
    }
}

#if defined(CONFIG_STM32_DMA2D) || defined(CONFIG_STM32F7_DMA2D) || \
    defined(CONFIG_STM32H7_DMA2D) || defined(CONFIG_STM32L4_DMA2D) || \
    defined(CONFIG_STM32U5_DMA2D)
static uint32_t fr_nuttx_dma2d_format_caps(void)
{
  uint32_t caps = FR_FORMAT_CAP_NONE;

#if defined(CONFIG_STM32_DMA2D_RGB565) || \
    defined(CONFIG_STM32F7_DMA2D_RGB565) || \
    defined(CONFIG_STM32H7_DMA2D_RGB565) || \
    defined(CONFIG_STM32L4_DMA2D_RGB565) || \
    defined(CONFIG_STM32U5_DMA2D)
  caps |= FR_FORMAT_CAP_RGB565;
#endif

#if defined(CONFIG_STM32_DMA2D_RGB888) || \
    defined(CONFIG_STM32F7_DMA2D_RGB888) || \
    defined(CONFIG_STM32H7_DMA2D_RGB888) || \
    defined(CONFIG_STM32L4_DMA2D_RGB888) || \
    defined(CONFIG_STM32U5_DMA2D)
  caps |= FR_FORMAT_CAP_RGB24;
#endif

#if defined(CONFIG_STM32_DMA2D_ARGB8888) || \
    defined(CONFIG_STM32F7_DMA2D_ARGB8888) || \
    defined(CONFIG_STM32H7_DMA2D_ARGB8888) || \
    defined(CONFIG_STM32L4_DMA2D_ARGB8888) || \
    defined(CONFIG_STM32U5_DMA2D)
  caps |= FR_FORMAT_CAP_RGBA32;
#endif

  return caps;
}
#endif

#ifdef CONFIG_NX
static uint32_t fr_nuttx_nx_format_caps(void)
{
  uint32_t caps = FR_FORMAT_CAP_NONE;

#  ifndef CONFIG_NX_DISABLE_16BPP
  caps |= FR_FORMAT_CAP_RGB565;
#  endif

#  ifndef CONFIG_NX_DISABLE_24BPP
  caps |= FR_FORMAT_CAP_RGB24;
#  endif

#  ifndef CONFIG_NX_DISABLE_32BPP
  caps |= FR_FORMAT_CAP_RGB32 | FR_FORMAT_CAP_RGBT32 |
          FR_FORMAT_CAP_RGBA32;
#  endif

  return caps;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

fr_color_t fr_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  fr_color_t color;

  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;

  return color;
}

uint32_t fr_color_pack_rgba8888(fr_color_t color)
{
  return ((uint32_t)color.r << 24) | ((uint32_t)color.g << 16) |
         ((uint32_t)color.b << 8) | color.a;
}

int fr_surface_init(fr_surface_t *surface, void *pixels,
                    uint16_t width, uint16_t height, uint16_t stride,
                    enum fr_format_e format)
{
  if (surface == NULL || pixels == NULL || width == 0 || height == 0 ||
      stride < width || format != FR_FORMAT_RGBA8888)
    {
      return -EINVAL;
    }

  surface->pixels = pixels;
  surface->width = width;
  surface->height = height;
  surface->stride = stride;
  surface->format = format;
  return 0;
}

void fr_command_list_init(fr_command_list_t *list, fr_command_t *commands,
                          uint16_t capacity)
{
  if (list != NULL)
    {
      list->commands = commands;
      list->capacity = capacity;
      list->count = 0;
      list->overflowed = false;
    }
}

void fr_command_list_reset(fr_command_list_t *list)
{
  if (list != NULL)
    {
      list->count = 0;
      list->overflowed = false;
    }
}

bool fr_command_list_overflowed(const fr_command_list_t *list)
{
  return list != NULL && list->overflowed;
}

uint16_t fr_command_list_count(const fr_command_list_t *list)
{
  return list != NULL ? list->count : 0;
}

int fr_cmd_clear(fr_command_list_t *list, fr_color_t color)
{
  fr_command_t command;

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_CLEAR;
  command.color = color;

  return fr_command_append(list, &command);
}

int fr_cmd_fill_rect(fr_command_list_t *list, const fr_rect_t *rect,
                     fr_color_t color)
{
  fr_command_t command;

  if (rect == NULL)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_FILL_RECT;
  command.rect = *rect;
  command.color = color;

  return fr_command_append(list, &command);
}

int fr_cmd_stroke_rect(fr_command_list_t *list, const fr_rect_t *rect,
                       uint16_t thickness, fr_color_t color)
{
  fr_command_t command;

  if (rect == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_STROKE_RECT;
  command.rect = *rect;
  command.color = color;
  command.thickness = thickness;

  return fr_command_append(list, &command);
}

int fr_cmd_fill_quad(fr_command_list_t *list, const fr_quad_t *quad,
                     fr_color_t color)
{
  fr_command_t command;

  if (quad == NULL)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_FILL_QUAD;
  command.quad = *quad;
  command.color = color;
  command.thickness = 0;

  return fr_command_append(list, &command);
}

int fr_cmd_fill_triangle(fr_command_list_t *list,
                         const fr_triangle_t *triangle,
                         fr_color_t color)
{
  fr_command_t command;

  if (triangle == NULL)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_FILL_TRIANGLE;
  command.triangle = *triangle;
  command.color = color;

  return fr_command_append(list, &command);
}

int fr_cmd_stroke_quad(fr_command_list_t *list, const fr_quad_t *quad,
                       uint16_t thickness, fr_color_t color)
{
  fr_command_t command;

  if (quad == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_STROKE_QUAD;
  command.quad = *quad;
  command.color = color;
  command.thickness = thickness;

  return fr_command_append(list, &command);
}

int fr_cmd_blit(fr_command_list_t *list, const fr_surface_t *source,
                const fr_rect_t *src_rect, const fr_rect_t *dst_rect)
{
  return fr_cmd_blit_alpha(list, source, src_rect, dst_rect, 255);
}

int fr_cmd_blit_alpha(fr_command_list_t *list, const fr_surface_t *source,
                      const fr_rect_t *src_rect, const fr_rect_t *dst_rect,
                      uint8_t global_alpha)
{
  fr_command_t command;

  if (!fr_source_rect_valid(source, src_rect) || dst_rect == NULL ||
      dst_rect->w == 0 || dst_rect->h == 0)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_BLIT;
  command.source = *source;
  command.src_rect = *src_rect;
  command.rect = *dst_rect;
  command.global_alpha = global_alpha;

  return fr_command_append(list, &command);
}

int fr_cmd_blit_quad_alpha(fr_command_list_t *list,
                           const fr_surface_t *source,
                           const fr_rect_t *src_rect,
                           const fr_quad_t *dst_quad,
                           uint8_t global_alpha)
{
  fr_command_t command;

  if (!fr_source_rect_valid(source, src_rect) || dst_quad == NULL)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_BLIT_QUAD;
  command.source = *source;
  command.src_rect = *src_rect;
  command.quad = *dst_quad;
  command.global_alpha = global_alpha;

  return fr_command_append(list, &command);
}

int fr_cmd_push_clip(fr_command_list_t *list, const fr_rect_t *rect)
{
  fr_command_t command;

  if (rect == NULL)
    {
      return -EINVAL;
    }

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_PUSH_CLIP;
  command.rect = *rect;

  return fr_command_append(list, &command);
}

int fr_cmd_pop_clip(fr_command_list_t *list)
{
  fr_command_t command;

  memset(&command, 0, sizeof(command));
  command.kind = FR_CMD_POP_CLIP;

  return fr_command_append(list, &command);
}

fr_backend_caps_t fr_backend_caps_none(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "none";
  caps.kind = FR_BACKEND_KIND_SOFTWARE;
  caps.caps = FR_CAP_NONE;
  caps.preferred_format = FR_FORMAT_RGBA8888;
  return caps;
}

fr_backend_caps_t fr_backend_caps_software(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "software";
  caps.kind = FR_BACKEND_KIND_SOFTWARE;
  caps.caps = FR_CAP_SOFTWARE | FR_CAP_COMMAND_LIST | FR_CAP_CLEAR |
              FR_CAP_FILL_RECT | FR_CAP_STROKE_RECT | FR_CAP_CLIP |
              FR_CAP_RGBA8888_TARGET | FR_CAP_FILL_QUAD |
              FR_CAP_FILL_TRIANGLE | FR_CAP_STROKE_QUAD | FR_CAP_BLIT |
              FR_CAP_BLIT_QUAD;
  caps.draw_caps = FR_DRAW_CAP_COMMANDS | FR_DRAW_CAP_CLEAR |
                   FR_DRAW_CAP_FILL_RECT | FR_DRAW_CAP_STROKE_RECT |
                   FR_DRAW_CAP_CLIP | FR_DRAW_CAP_FILL_QUAD |
                   FR_DRAW_CAP_FILL_TRIANGLE | FR_DRAW_CAP_STROKE_QUAD |
                   FR_DRAW_CAP_BLIT | FR_DRAW_CAP_BLIT_QUAD;
  caps.memory_caps = FR_MEMORY_CAP_SURFACE | FR_MEMORY_CAP_PIXEL_WRITE;
  caps.blend_caps = FR_BLEND_CAP_GLOBAL_ALPHA | FR_BLEND_CAP_PIXEL_ALPHA;
  caps.format_caps = FR_FORMAT_CAP_RGBA8888;
  caps.preferred_format = FR_FORMAT_RGBA8888;
  caps.max_width = 0xffff;
  caps.max_height = 0xffff;
  caps.max_commands = 0xffff;
  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_graphics(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-graphics";
  caps.kind = FR_BACKEND_KIND_COMPOSITOR;
  caps.caps = FR_CAP_NONE;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT)
  caps.caps |= FR_CAP_FB_PRESENT;
  caps.present_caps |= FR_PRESENT_CAP_FRAMEBUFFER;
  caps.memory_caps |= FR_MEMORY_CAP_MMAP | FR_MEMORY_CAP_DIRECT_PTR |
                      FR_MEMORY_CAP_PIXEL_WRITE;
  caps.format_caps |= FR_FORMAT_CAP_RGB565 | FR_FORMAT_CAP_RGB24 |
                      FR_FORMAT_CAP_RGB32 | FR_FORMAT_CAP_RGBT32 |
                      FR_FORMAT_CAP_RGBA32;
#endif

#ifdef CONFIG_FB_UPDATE
  caps.present_caps |= FR_PRESENT_CAP_UPDATE_RECT;
  caps.sync_caps |= FR_SYNC_CAP_UPDATE_RECT;
#endif

#ifdef CONFIG_NX
  caps.caps |= FR_CAP_NX_PRESENT;
  caps.present_caps |= FR_PRESENT_CAP_NX;
#endif

  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_framebuffer(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-framebuffer-config";
  caps.kind = FR_BACKEND_KIND_PRESENT;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT)
  caps.caps = FR_CAP_FB_PRESENT;
  caps.present_caps = FR_PRESENT_CAP_FRAMEBUFFER;
  caps.memory_caps = FR_MEMORY_CAP_MMAP | FR_MEMORY_CAP_DIRECT_PTR |
                     FR_MEMORY_CAP_PIXEL_WRITE;
  caps.format_caps = FR_FORMAT_CAP_RGB565 | FR_FORMAT_CAP_RGB24 |
                     FR_FORMAT_CAP_RGB32 | FR_FORMAT_CAP_RGBT32 |
                     FR_FORMAT_CAP_RGBA32;
#endif

#ifdef CONFIG_FB_UPDATE
  caps.present_caps |= FR_PRESENT_CAP_UPDATE_RECT;
  caps.sync_caps |= FR_SYNC_CAP_UPDATE_RECT;
#endif

  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_lcd(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-lcd";
  caps.kind = FR_BACKEND_KIND_PRESENT;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#ifdef CONFIG_LCD_DEV
  caps.present_caps = FR_PRESENT_CAP_LCD;
  caps.memory_caps = FR_MEMORY_CAP_PIXEL_WRITE;
  caps.format_caps = FR_FORMAT_CAP_RGB565 | FR_FORMAT_CAP_RGB24 |
                     FR_FORMAT_CAP_RGB32 | FR_FORMAT_CAP_RGBA32;
#endif

  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_nx(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-nx";
  caps.kind = FR_BACKEND_KIND_COMPOSITOR;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#ifdef CONFIG_NX
  caps.caps = FR_CAP_NX_PRESENT;
  caps.draw_caps = FR_DRAW_CAP_FILL_RECT | FR_DRAW_CAP_STROKE_RECT |
                   FR_DRAW_CAP_CLIP | FR_DRAW_CAP_BLIT;
  caps.present_caps = FR_PRESENT_CAP_NX;
  caps.format_caps = fr_nuttx_nx_format_caps();

#  ifdef CONFIG_NX_RAMBACKED
  caps.memory_caps |= FR_MEMORY_CAP_SURFACE | FR_MEMORY_CAP_PIXEL_WRITE;
#  endif

#  ifdef CONFIG_NX_UPDATE
  caps.present_caps |= FR_PRESENT_CAP_UPDATE_RECT;
  caps.sync_caps |= FR_SYNC_CAP_UPDATE_RECT;
#  endif

#  ifdef CONFIG_NXFONTS
  caps.draw_caps |= FR_DRAW_CAP_TEXT;
#  endif
#endif

  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_dma2d(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-dma2d";
  caps.kind = FR_BACKEND_KIND_ACCELERATOR;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#if defined(CONFIG_STM32_DMA2D) || defined(CONFIG_STM32F7_DMA2D) || \
    defined(CONFIG_STM32H7_DMA2D) || defined(CONFIG_STM32L4_DMA2D) || \
    defined(CONFIG_STM32U5_DMA2D)
  caps.caps = FR_CAP_HARDWARE_ACCELERATOR;
  caps.draw_caps = FR_DRAW_CAP_FILL_RECT;
  caps.memory_caps = FR_MEMORY_CAP_PIXEL_WRITE;
  caps.format_caps = fr_nuttx_dma2d_format_caps();
  caps.sync_caps = FR_SYNC_CAP_FENCE;

#  if defined(CONFIG_FB_OVERLAY_BLIT)
  caps.draw_caps |= FR_DRAW_CAP_BLIT;
#  endif

#  if defined(CONFIG_FB_OVERLAY_BLEND)
  caps.blend_caps |= FR_BLEND_CAP_GLOBAL_ALPHA | FR_BLEND_CAP_PIXEL_ALPHA;
#  endif
#endif

  return caps;
}

fr_backend_caps_t fr_backend_caps_nuttx_gpu2d(void)
{
  fr_backend_caps_t caps;

  memset(&caps, 0, sizeof(caps));
  caps.name = "nuttx-gpu2d";
  caps.kind = FR_BACKEND_KIND_ACCELERATOR;
  caps.preferred_format = FR_FORMAT_RGBA8888;

#if defined(CONFIG_STM32U5_GPU2D) || defined(CONFIG_STM32N6_GPU2D) || \
    defined(CONFIG_STM32H7RS_GPU2D)
  caps.caps = FR_CAP_HARDWARE_ACCELERATOR | FR_CAP_COMMAND_LIST;
  caps.draw_caps = FR_DRAW_CAP_COMMANDS | FR_DRAW_CAP_CLEAR |
                   FR_DRAW_CAP_FILL_RECT | FR_DRAW_CAP_BLIT |
                   FR_DRAW_CAP_CLIP;
  caps.blend_caps = FR_BLEND_CAP_GLOBAL_ALPHA | FR_BLEND_CAP_PIXEL_ALPHA |
                    FR_BLEND_CAP_COLOR_KEY;
  caps.transform_caps = FR_TRANSFORM_CAP_SCALE;
  caps.memory_caps = FR_MEMORY_CAP_SURFACE | FR_MEMORY_CAP_PIXEL_WRITE;
  caps.sync_caps = FR_SYNC_CAP_FENCE;
  caps.format_caps = FR_FORMAT_CAP_RGBA8888 | FR_FORMAT_CAP_RGB565 |
                     FR_FORMAT_CAP_RGB24 | FR_FORMAT_CAP_RGB32 |
                     FR_FORMAT_CAP_RGBA32;
  caps.max_commands = 0xffff;
#endif

  return caps;
}

int fr_backend_caps_from_fb_presenter(const fr_fb_presenter_t *presenter,
                                      fr_backend_caps_t *caps)
{
  if (presenter == NULL || caps == NULL || !presenter->open)
    {
      return -EINVAL;
    }

  memset(caps, 0, sizeof(*caps));
  caps->name = "nuttx-framebuffer";
  caps->kind = FR_BACKEND_KIND_PRESENT;
  caps->caps = FR_CAP_FB_PRESENT;
  caps->present_caps = FR_PRESENT_CAP_FRAMEBUFFER;
  caps->memory_caps = FR_MEMORY_CAP_PIXEL_WRITE;
  caps->format_caps = fr_fb_format_caps(presenter->fmt);
  caps->preferred_format = FR_FORMAT_RGBA8888;
  caps->max_width = presenter->xres;
  caps->max_height = presenter->yres;
  caps->max_commands = 0;

  if (presenter->mapped)
    {
      caps->memory_caps |= FR_MEMORY_CAP_MMAP;
    }
  else if (presenter->fbmem != NULL)
    {
      caps->memory_caps |= FR_MEMORY_CAP_DIRECT_PTR;
    }

#ifdef CONFIG_FB_UPDATE
  caps->present_caps |= FR_PRESENT_CAP_UPDATE_RECT;
  caps->sync_caps |= FR_SYNC_CAP_UPDATE_RECT;
#endif

  return 0;
}

bool fr_backend_supports(const fr_backend_caps_t *caps,
                         enum fr_command_kind_e kind)
{
  uint32_t required;
  uint32_t draw_required;

  if (caps == NULL)
    {
      return false;
    }

  switch (kind)
    {
      case FR_CMD_CLEAR:
        required = FR_CAP_CLEAR;
        draw_required = FR_DRAW_CAP_CLEAR;
        break;

      case FR_CMD_FILL_RECT:
        required = FR_CAP_FILL_RECT;
        draw_required = FR_DRAW_CAP_FILL_RECT;
        break;

      case FR_CMD_STROKE_RECT:
        required = FR_CAP_STROKE_RECT;
        draw_required = FR_DRAW_CAP_STROKE_RECT;
        break;

      case FR_CMD_FILL_QUAD:
        required = FR_CAP_FILL_QUAD;
        draw_required = FR_DRAW_CAP_FILL_QUAD;
        break;

      case FR_CMD_FILL_TRIANGLE:
        required = FR_CAP_FILL_TRIANGLE;
        draw_required = FR_DRAW_CAP_FILL_TRIANGLE;
        break;

      case FR_CMD_STROKE_QUAD:
        required = FR_CAP_STROKE_QUAD;
        draw_required = FR_DRAW_CAP_STROKE_QUAD;
        break;

      case FR_CMD_BLIT:
        required = FR_CAP_BLIT;
        draw_required = FR_DRAW_CAP_BLIT;
        break;

      case FR_CMD_BLIT_QUAD:
        required = FR_CAP_BLIT_QUAD;
        draw_required = FR_DRAW_CAP_BLIT_QUAD;
        break;

      case FR_CMD_PUSH_CLIP:
      case FR_CMD_POP_CLIP:
        required = FR_CAP_CLIP;
        draw_required = FR_DRAW_CAP_CLIP;
        break;

      default:
        return false;
    }

  return (caps->caps & required) != 0 ||
         (caps->draw_caps & draw_required) != 0;
}

bool fr_backend_supports_draw(const fr_backend_caps_t *caps,
                              uint32_t draw_caps)
{
  return caps != NULL && (caps->draw_caps & draw_caps) == draw_caps;
}

bool fr_backend_supports_present(const fr_backend_caps_t *caps,
                                 uint32_t present_caps)
{
  return caps != NULL && (caps->present_caps & present_caps) == present_caps;
}

bool fr_backend_supports_memory(const fr_backend_caps_t *caps,
                                uint32_t memory_caps)
{
  return caps != NULL && (caps->memory_caps & memory_caps) == memory_caps;
}

bool fr_backend_supports_blend(const fr_backend_caps_t *caps,
                               uint32_t blend_caps)
{
  return caps != NULL && (caps->blend_caps & blend_caps) == blend_caps;
}

bool fr_backend_supports_transform(const fr_backend_caps_t *caps,
                                   uint32_t transform_caps)
{
  return caps != NULL &&
         (caps->transform_caps & transform_caps) == transform_caps;
}

bool fr_backend_supports_sync(const fr_backend_caps_t *caps,
                              uint32_t sync_caps)
{
  return caps != NULL && (caps->sync_caps & sync_caps) == sync_caps;
}

bool fr_backend_supports_format(const fr_backend_caps_t *caps,
                                uint32_t format_caps)
{
  return caps != NULL && (caps->format_caps & format_caps) == format_caps;
}

void fr_backend_registry_reset(void)
{
  memset(g_backend_registry, 0, sizeof(g_backend_registry));
  g_backend_registry_count = 0;
}

int fr_backend_register(const fr_backend_caps_t *caps)
{
  uint8_t i;

  if (caps == NULL || caps->name == NULL)
    {
      return -EINVAL;
    }

  for (i = 0; i < g_backend_registry_count; i++)
    {
      if (strcmp(g_backend_registry[i].name, caps->name) == 0)
        {
          g_backend_registry[i] = *caps;
          return 0;
        }
    }

  if (g_backend_registry_count >= FR_BACKEND_REGISTRY_MAX)
    {
      return -ENOSPC;
    }

  g_backend_registry[g_backend_registry_count] = *caps;
  g_backend_registry_count++;
  return 0;
}

int fr_backend_register_builtin(void)
{
  fr_backend_caps_t caps;
  int ret;

  caps = fr_backend_caps_software();
  ret = fr_backend_register(&caps);
  if (ret < 0)
    {
      return ret;
    }

  return fr_backend_register_nuttx_graphics();
}

int fr_backend_register_nuttx_graphics(void)
{
  fr_backend_caps_t caps;
  int ret;

  caps = fr_backend_caps_nuttx_graphics();
  if (caps.caps != FR_CAP_NONE || caps.present_caps != FR_PRESENT_CAP_NONE ||
      caps.draw_caps != FR_DRAW_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  caps = fr_backend_caps_nuttx_framebuffer();
  if (caps.caps != FR_CAP_NONE || caps.present_caps != FR_PRESENT_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  caps = fr_backend_caps_nuttx_lcd();
  if (caps.present_caps != FR_PRESENT_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  caps = fr_backend_caps_nuttx_nx();
  if (caps.caps != FR_CAP_NONE || caps.present_caps != FR_PRESENT_CAP_NONE ||
      caps.draw_caps != FR_DRAW_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  caps = fr_backend_caps_nuttx_dma2d();
  if (caps.caps != FR_CAP_NONE || caps.draw_caps != FR_DRAW_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  caps = fr_backend_caps_nuttx_gpu2d();
  if (caps.caps != FR_CAP_NONE || caps.draw_caps != FR_DRAW_CAP_NONE)
    {
      ret = fr_backend_register(&caps);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

int fr_backend_register_fb_presenter(const fr_fb_presenter_t *presenter)
{
  fr_backend_caps_t caps;
  int ret;

  ret = fr_backend_caps_from_fb_presenter(presenter, &caps);
  if (ret < 0)
    {
      return ret;
    }

  return fr_backend_register(&caps);
}

uint8_t fr_backend_registry_count(void)
{
  return g_backend_registry_count;
}

const fr_backend_caps_t *fr_backend_registry_get(uint8_t index)
{
  if (index >= g_backend_registry_count)
    {
      return NULL;
    }

  return &g_backend_registry[index];
}

const fr_backend_caps_t *fr_backend_registry_find(const char *name)
{
  uint8_t i;

  if (name == NULL)
    {
      return NULL;
    }

  for (i = 0; i < g_backend_registry_count; i++)
    {
      if (strcmp(g_backend_registry[i].name, name) == 0)
        {
          return &g_backend_registry[i];
        }
    }

  return NULL;
}

int fr_execute_software(fr_surface_t *surface,
                        const fr_command_list_t *list)
{
  fr_rect_t clip_stack[FR_CLIP_STACK_DEPTH];
  fr_rect_t current_clip;
  fr_rect_t bounds;
  uint8_t clip_depth;
  uint16_t i;
  int ret;

  if (!fr_surface_valid(surface) || list == NULL || list->commands == NULL)
    {
      return -EINVAL;
    }

  bounds = fr_surface_bounds(surface);
  current_clip = bounds;
  clip_depth = 0;

  for (i = 0; i < list->count; i++)
    {
      const fr_command_t *command = &list->commands[i];

      switch (command->kind)
        {
          case FR_CMD_CLEAR:
            ret = fr_fill_rect_clipped(surface, &current_clip, &bounds,
                                       command->color);
            break;

          case FR_CMD_FILL_RECT:
            ret = fr_fill_rect_clipped(surface, &current_clip,
                                       &command->rect, command->color);
            break;

          case FR_CMD_STROKE_RECT:
            ret = fr_stroke_rect_clipped(surface, &current_clip,
                                         &command->rect, command->thickness,
                                         command->color);
            break;

          case FR_CMD_FILL_QUAD:
            ret = fr_fill_quad_clipped(surface, &current_clip,
                                       &command->quad, command->color);
            break;

          case FR_CMD_FILL_TRIANGLE:
            ret = fr_fill_triangle_clipped(surface, &current_clip,
                                          &command->triangle,
                                          command->color);
            break;

          case FR_CMD_STROKE_QUAD:
            ret = fr_stroke_quad_clipped(surface, &current_clip,
                                         &command->quad,
                                         command->thickness,
                                         command->color);
            break;

          case FR_CMD_BLIT:
            ret = fr_blit_clipped(surface, &current_clip,
                                  &command->source, &command->src_rect,
                                  &command->rect, command->global_alpha);
            break;

          case FR_CMD_BLIT_QUAD:
            ret = fr_blit_quad_clipped(surface, &current_clip,
                                       &command->source,
                                       &command->src_rect,
                                       &command->quad,
                                       command->global_alpha);
            break;

          case FR_CMD_PUSH_CLIP:
            if (clip_depth >= FR_CLIP_STACK_DEPTH)
              {
                return -ENOSPC;
              }

            clip_stack[clip_depth] = current_clip;
            clip_depth++;

            if (!fr_rect_intersect(&current_clip, &command->rect,
                                   &current_clip))
              {
                current_clip.x = 0;
                current_clip.y = 0;
                current_clip.w = 0;
                current_clip.h = 0;
              }

            ret = 0;
            break;

          case FR_CMD_POP_CLIP:
            if (clip_depth == 0)
              {
                return -EINVAL;
              }

            clip_depth--;
            current_clip = clip_stack[clip_depth];
            ret = 0;
            break;

          default:
            return -ENOSYS;
        }

      if (ret < 0)
        {
          return ret;
        }
    }

  return list->overflowed ? -ENOSPC : 0;
}

int fr_fb_presenter_open(fr_fb_presenter_t *presenter, const char *path)
{
#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT)
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
  const char *fbpath;
  void *fbmem;
  int fd;
  int ret;

  if (presenter == NULL)
    {
      return -EINVAL;
    }

  memset(presenter, 0, sizeof(*presenter));
  presenter->fd = -1;

  fbpath = path != NULL ? path : "/dev/fb0";
  fd = open(fbpath, O_RDWR);
  if (fd < 0)
    {
      return -errno;
    }

  ret = ioctl(fd, FBIOGET_VIDEOINFO, (unsigned long)&vinfo);
  if (ret < 0)
    {
      ret = -errno;
      close(fd);
      return ret;
    }

  memset(&pinfo, 0, sizeof(pinfo));
  ret = ioctl(fd, FBIOGET_PLANEINFO, (unsigned long)&pinfo);
  if (ret < 0)
    {
      ret = -errno;
      close(fd);
      return ret;
    }

  fbmem = mmap(NULL, pinfo.fblen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (fbmem == MAP_FAILED)
    {
      if (pinfo.fbmem == NULL)
        {
          ret = -errno;
          close(fd);
          return ret;
        }

      fbmem = pinfo.fbmem;
      presenter->mapped = false;
    }
  else
    {
      presenter->mapped = true;
    }

  presenter->fd = fd;
  presenter->fbmem = fbmem;
  presenter->fblen = pinfo.fblen;
  presenter->xres = vinfo.xres;
  presenter->yres = vinfo.yres;
  presenter->stride = pinfo.stride;
  presenter->fmt = vinfo.fmt;
  presenter->bpp = pinfo.bpp;
  presenter->open = true;
  return 0;
#else
  return -ENOSYS;
#endif
}

int fr_fb_presenter_present(fr_fb_presenter_t *presenter,
                            const fr_surface_t *surface)
{
  fr_rect_t rect;

  if (surface == NULL)
    {
      return -EINVAL;
    }

  rect.x = 0;
  rect.y = 0;
  rect.w = surface->width;
  rect.h = surface->height;
  return fr_fb_presenter_present_rect(presenter, surface, &rect);
}

int fr_fb_presenter_present_rect(fr_fb_presenter_t *presenter,
                                 const fr_surface_t *surface,
                                 const fr_rect_t *rect)
{
#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT)
  fr_rect_t bounds;
  fr_rect_t fbounds;
  fr_rect_t clipped;
  const uint32_t *src_pixels;
  const uint32_t *src_row;
  uint8_t *dst_row;
  uint8_t *dst;
  uint8_t bpp;
  uint16_t x;
  uint16_t y;
  int ret;

  if (presenter == NULL || !presenter->open || presenter->fbmem == NULL ||
      !fr_surface_valid(surface) || rect == NULL)
    {
      return -EINVAL;
    }

  bpp = fr_fb_bytes_per_pixel(presenter);
  if (bpp == 0)
    {
      return -EINVAL;
    }

  bounds = fr_surface_bounds(surface);
  fbounds.x = 0;
  fbounds.y = 0;
  fbounds.w = presenter->xres;
  fbounds.h = presenter->yres;

  if (!fr_rect_intersect(&bounds, rect, &clipped) ||
      !fr_rect_intersect(&fbounds, &clipped, &clipped))
    {
      return 0;
    }

  src_pixels = (const uint32_t *)surface->pixels;

  for (y = 0; y < clipped.h; y++)
    {
      src_row = src_pixels +
                ((uint32_t)(clipped.y + y) * surface->stride) + clipped.x;
      dst_row = (uint8_t *)presenter->fbmem +
                ((uint32_t)(clipped.y + y) * presenter->stride) +
                ((uint32_t)clipped.x * bpp);

      for (x = 0; x < clipped.w; x++)
        {
          dst = dst_row + ((uint32_t)x * bpp);
          ret = fr_fb_present_pixel(dst, presenter->fmt, src_row[x]);
          if (ret < 0)
            {
              return ret;
            }
        }
    }

  return 0;
#else
  return -ENOSYS;
#endif
}

int fr_fb_presenter_update_rect(fr_fb_presenter_t *presenter,
                                const fr_rect_t *rect)
{
#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT) && \
    defined(CONFIG_FB_UPDATE)
  struct fb_area_s area;

  if (presenter == NULL || !presenter->open || rect == NULL)
    {
      return -EINVAL;
    }

  area.x = rect->x < 0 ? 0 : rect->x;
  area.y = rect->y < 0 ? 0 : rect->y;
  area.w = rect->w;
  area.h = rect->h;

  if (ioctl(presenter->fd, FBIO_UPDATE, (unsigned long)&area) < 0)
    {
      return -errno;
    }

  return 0;
#else
  return 0;
#endif
}

int fr_fb_presenter_poll_input(fr_fb_presenter_t *presenter,
                               fr_input_event_t *input)
{
#if defined(CONFIG_SIM_X11FB)
  extern int sim_x11pollinput(int *type, int16_t *x, int16_t *y,
                              uint16_t *key, int16_t *encoder_delta,
                              uint8_t *button);
  int type;
  int ret;

  if (presenter == NULL || !presenter->open || input == NULL)
    {
      return -EINVAL;
    }

  input->type = FR_INPUT_NONE;
  input->x = 0;
  input->y = 0;
  input->key = FR_KEY_UNKNOWN;
  input->encoder_delta = 0;
  input->button = 0;

  ret = sim_x11pollinput(&type, &input->x, &input->y, &input->key,
                         &input->encoder_delta, &input->button);
  if (ret <= 0)
    {
      return ret;
    }

  switch (type)
    {
      case 1:
        input->type = FR_INPUT_POINTER_DOWN;
        break;
      case 2:
        input->type = FR_INPUT_POINTER_MOVE;
        break;
      case 3:
        input->type = FR_INPUT_POINTER_UP;
        break;
      case 4:
        input->type = FR_INPUT_KEY_DOWN;
        break;
      case 5:
        input->type = FR_INPUT_KEY_UP;
        break;
      case 6:
        input->type = FR_INPUT_ENCODER_ROTATE;
        break;
      default:
        input->type = FR_INPUT_NONE;
        return 0;
    }

  return 1;
#else
  (void)presenter;
  (void)input;
  return 0;
#endif
}

void fr_fb_presenter_close(fr_fb_presenter_t *presenter)
{
#if defined(CONFIG_VIDEO_FB) && defined(CONFIG_GRAPHICS_FRENDER_FB_PRESENT)
  if (presenter != NULL && presenter->open)
    {
      if (presenter->mapped && presenter->fbmem != NULL)
        {
          munmap(presenter->fbmem, presenter->fblen);
        }

      if (presenter->fd >= 0)
        {
          close(presenter->fd);
        }

      memset(presenter, 0, sizeof(*presenter));
      presenter->fd = -1;
    }
#else
  (void)presenter;
#endif
}

bool fr_fb_presenter_window_closed(fr_fb_presenter_t *presenter)
{
#if defined(CONFIG_SIM_X11FB)
  extern bool sim_x11pollwindowclosed(void);

  if (presenter == NULL || !presenter->open)
    {
      return true;
    }

  return sim_x11pollwindowclosed();
#else
  return presenter == NULL || !presenter->open;
#endif
}

uint32_t fr_surface_checksum_rgba8888(const fr_surface_t *surface)
{
  const uint32_t *pixels;
  uint32_t hash;
  size_t count;
  size_t i;

  if (!fr_surface_valid(surface))
    {
      return 0;
    }

  pixels = (const uint32_t *)surface->pixels;
  count = (size_t)surface->width * surface->height;
  hash = 2166136261u;

  for (i = 0; i < count; i++)
    {
      hash ^= pixels[i];
      hash *= 16777619u;
    }

  return hash;
}
