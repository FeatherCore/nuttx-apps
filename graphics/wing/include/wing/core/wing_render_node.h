/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_render_node.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_NODE_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_NODE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_WING_H
#  include <wing/wing.h>
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Render nodes are WING render-frontend descriptions.
 *
 * They are intentionally above FRender commands and below object/widget draw
 * callbacks: object space, dirty culling and widget state stay in WING, while
 * execution still goes through the unified FRender backend.
 */

enum wing_render_node_type_e
{
  WING_RENDER_NODE_FILL_RECT = 0,
  WING_RENDER_NODE_FILL_QUAD,
  WING_RENDER_NODE_FILL_TRIANGLE,
  WING_RENDER_NODE_DRAW_QUAD,
  WING_RENDER_NODE_BLIT,
  WING_RENDER_NODE_BLIT_QUAD
};

struct wing_render_material_s
{
  wing_color_t color;
  uint8_t opacity;
};

struct wing_render_node_s
{
  enum wing_render_node_type_e type;
  struct wing_render_material_s material;
  uint16_t thickness;
  const wing_surface_t *source;
  wing_rect_t src_rect;
  union
  {
    wing_rect_t rect;
    wing_quad2d_t quad;
    wing_triangle2d_t triangle;
  } geometry;
};

typedef enum wing_render_node_type_e wing_render_node_type_t;
typedef struct wing_render_material_s wing_render_material_t;
typedef struct wing_render_node_s wing_render_node_t;

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_NODE_H */
