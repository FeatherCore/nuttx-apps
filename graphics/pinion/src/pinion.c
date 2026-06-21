/****************************************************************************
 * apps/graphics/pinion/src/pinion.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <pinion/pinion.h>

#include <errno.h>
#include <string.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t pinion_pack_rgba(pinion_color_t color)
{
  return ((uint32_t)color.r << 0) |
         ((uint32_t)color.g << 8) |
         ((uint32_t)color.b << 16) |
         ((uint32_t)color.a << 24);
}

static void pinion_putpixel(pinion_surface_t *surface, int32_t x, int32_t y,
                            pinion_color_t color)
{
  uint32_t packed;
  uint8_t *row;

  if (surface == NULL || surface->pixels == NULL ||
      x < 0 || y < 0 ||
      (uint32_t)x >= surface->width ||
      (uint32_t)y >= surface->height)
    {
      return;
    }

  packed = pinion_pack_rgba(color);
  row = surface->pixels + (uint32_t)y * surface->stride;
  memcpy(row + (uint32_t)x * 4u, &packed, sizeof(packed));
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

pinion_color_t pinion_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  pinion_color_t color =
    {
      r, g, b, a
    };

  return color;
}

int pinion_engine_init(pinion_engine_t *engine, uint32_t target_fps)
{
  if (engine == NULL || target_fps == 0)
    {
      return -EINVAL;
    }

  engine->frame = 0;
  engine->target_fps = target_fps;
  engine->frame_time_us = 1000000u / target_fps;
  return 0;
}

void pinion_engine_begin_frame(pinion_engine_t *engine)
{
  (void)engine;
}

void pinion_engine_end_frame(pinion_engine_t *engine)
{
  if (engine != NULL)
    {
      engine->frame++;
    }
}

int pinion_surface_init(pinion_surface_t *surface, uint32_t width,
                        uint32_t height, uint32_t stride, uint8_t *pixels)
{
  if (surface == NULL || pixels == NULL || width == 0 || height == 0 ||
      stride < width * 4u)
    {
      return -EINVAL;
    }

  surface->width = width;
  surface->height = height;
  surface->stride = stride;
  surface->pixels = pixels;
  return 0;
}

void pinion_surface_clear(pinion_surface_t *surface, pinion_color_t color)
{
  uint32_t packed;
  uint32_t x;
  uint32_t y;

  if (surface == NULL || surface->pixels == NULL)
    {
      return;
    }

  packed = pinion_pack_rgba(color);

  for (y = 0; y < surface->height; y++)
    {
      uint8_t *row = surface->pixels + y * surface->stride;

      for (x = 0; x < surface->width; x++)
        {
          memcpy(row + x * 4u, &packed, sizeof(packed));
        }
    }
}

void pinion_draw_rect(pinion_surface_t *surface, int32_t x, int32_t y,
                      int32_t w, int32_t h, pinion_color_t color)
{
  int32_t x0;
  int32_t y0;
  int32_t x1;
  int32_t y1;
  int32_t py;

  if (surface == NULL || surface->pixels == NULL || w <= 0 || h <= 0)
    {
      return;
    }

  x0 = x < 0 ? 0 : x;
  y0 = y < 0 ? 0 : y;
  x1 = x + w;
  y1 = y + h;

  if (x1 > (int32_t)surface->width)
    {
      x1 = (int32_t)surface->width;
    }

  if (y1 > (int32_t)surface->height)
    {
      y1 = (int32_t)surface->height;
    }

  for (py = y0; py < y1; py++)
    {
      int32_t px;

      for (px = x0; px < x1; px++)
        {
          pinion_putpixel(surface, px, py, color);
        }
    }
}

void pinion_draw_circle(pinion_surface_t *surface, int32_t cx, int32_t cy,
                        int32_t radius, pinion_color_t color)
{
  int32_t r2;
  int32_t y;

  if (surface == NULL || surface->pixels == NULL || radius <= 0)
    {
      return;
    }

  r2 = radius * radius;

  for (y = -radius; y <= radius; y++)
    {
      int32_t x;

      for (x = -radius; x <= radius; x++)
        {
          if (x * x + y * y <= r2)
            {
              pinion_putpixel(surface, cx + x, cy + y, color);
            }
        }
    }
}

void pinion_blit(pinion_surface_t *dst, const pinion_surface_t *src,
                 int32_t dst_x, int32_t dst_y)
{
  uint32_t y;

  if (dst == NULL || src == NULL || dst->pixels == NULL ||
      src->pixels == NULL)
    {
      return;
    }

  for (y = 0; y < src->height; y++)
    {
      uint32_t x;

      for (x = 0; x < src->width; x++)
        {
          uint8_t color_bytes[4];
          pinion_color_t color;
          const uint8_t *spixel = src->pixels + y * src->stride + x * 4u;

          memcpy(color_bytes, spixel, sizeof(color_bytes));
          color = pinion_rgba(color_bytes[0], color_bytes[1],
                              color_bytes[2], color_bytes[3]);

          if (color.a != 0)
            {
              pinion_putpixel(dst, dst_x + (int32_t)x, dst_y + (int32_t)y,
                              color);
            }
        }
    }
}
