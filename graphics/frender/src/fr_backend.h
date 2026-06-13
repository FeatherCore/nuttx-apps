/****************************************************************************
 * apps/graphics/frender/src/fr_backend.h
 *
 * Internal: backend vtable definition.
 * Each backend implementation provides one instance of fr_backend_ops_s.
 * FRender core dispatches commands through the vtable.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_FRENDER_SRC_FR_BACKEND_H
#define __APPS_GRAPHICS_FRENDER_SRC_FR_BACKEND_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include <frender/frender.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Backend operations vtable.
 *
 * Lifecycle: open() allocates backend-private state, close() frees it.
 *
 * Drawing commands: each receives the current effective clip rect
 * (maintained by fr_execute()'s clip stack). The backend renders the
 * primitive clipped to that rect.
 *
 * present(): called after the full command list is executed. The damage
 * rect may be NULL (full update). For framebuffer backend this issues
 * FBIO_UPDATE; for software backend it copies surface→fbmem; for NX
 * backend it's a no-op.
 */

struct fr_backend_ops_s
{
  const char *name;

  int  (*open)(void **priv, const void *config);
  void (*close)(void *priv);
  fr_rect_t (*get_bounds)(void *priv);

  int (*cmd_clear)(void *priv, const fr_rect_t *clip, fr_color_t color);
  int (*cmd_fill_rect)(void *priv, const fr_rect_t *clip,
                       const fr_rect_t *rect, fr_color_t color);
  int (*cmd_stroke_rect)(void *priv, const fr_rect_t *clip,
                         const fr_rect_t *rect, uint16_t thickness,
                         fr_color_t color);
  int (*cmd_fill_quad)(void *priv, const fr_rect_t *clip,
                       const fr_quad_t *quad, fr_color_t color);
  int (*cmd_fill_triangle)(void *priv, const fr_rect_t *clip,
                           const fr_triangle_t *triangle, fr_color_t color);
  int (*cmd_stroke_quad)(void *priv, const fr_rect_t *clip,
                         const fr_quad_t *quad, uint16_t thickness,
                         fr_color_t color);
  int (*cmd_blit)(void *priv, const fr_rect_t *clip,
                  const fr_surface_t *source, const fr_rect_t *src_rect,
                  const fr_rect_t *dst_rect, uint8_t global_alpha);
  int (*cmd_blit_quad)(void *priv, const fr_rect_t *clip,
                       const fr_surface_t *source, const fr_rect_t *src_rect,
                       const fr_quad_t *dst_quad, uint8_t global_alpha);

  int (*present)(void *priv, const fr_rect_t *damage);
};

/* Runtime backend instance: vtable + private state */

/* fr_backend_instance_t is defined in frender.h (public type) */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Returns the ops vtable for each built-in backend.
 * Callers don't need to include backend-specific headers. */

const struct fr_backend_ops_s *fr_backend_ops_framebuffer(void);
const struct fr_backend_ops_s *fr_backend_ops_nx(void);

#endif /* __APPS_GRAPHICS_FRENDER_SRC_FR_BACKEND_H */
