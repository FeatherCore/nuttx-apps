/****************************************************************************
 * apps/graphics/pinion/include/pinion/pinion.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __PINION_PINION_H
#define __PINION_PINION_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct pinion_color_s
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pinion_color_t;

typedef struct pinion_surface_s
{
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint8_t *pixels;
} pinion_surface_t;

typedef struct pinion_engine_s
{
  uint32_t frame;
  uint32_t target_fps;
  uint32_t frame_time_us;
} pinion_engine_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

pinion_color_t pinion_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

int pinion_engine_init(pinion_engine_t *engine, uint32_t target_fps);
void pinion_engine_begin_frame(pinion_engine_t *engine);
void pinion_engine_end_frame(pinion_engine_t *engine);

int pinion_surface_init(pinion_surface_t *surface, uint32_t width,
                        uint32_t height, uint32_t stride, uint8_t *pixels);

void pinion_surface_clear(pinion_surface_t *surface, pinion_color_t color);
void pinion_draw_rect(pinion_surface_t *surface, int32_t x, int32_t y,
                      int32_t w, int32_t h, pinion_color_t color);
void pinion_draw_circle(pinion_surface_t *surface, int32_t cx, int32_t cy,
                        int32_t radius, pinion_color_t color);
void pinion_blit(pinion_surface_t *dst, const pinion_surface_t *src,
                 int32_t dst_x, int32_t dst_y);

#ifdef __cplusplus
}
#endif

#endif /* __PINION_PINION_H */
