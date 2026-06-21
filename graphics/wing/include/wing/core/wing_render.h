/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_render.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_surface_init(wing_surface_t *surface, void *pixels,
                      uint16_t width, uint16_t height, uint16_t stride,
                      enum wing_pixel_format_e format);
int wing_gui_init(wing_context_t *ctx, wing_surface_t *surface,
                  fr_command_list_t *commands);
void wing_gui_deinit(wing_context_t *ctx);
int wing_gui_begin(wing_context_t *ctx);
int wing_gui_end(wing_context_t *ctx);
int wing_gui_set_clip(wing_context_t *ctx, const wing_rect_t *clip);
int wing_gui_reset_clip(wing_context_t *ctx);
int wing_gui_clear(wing_context_t *ctx, wing_color_t color);
int wing_gui_fill_rect(wing_context_t *ctx, const wing_rect_t *rect,
                       wing_color_t color);
int wing_gui_fill_quad(wing_context_t *ctx, const wing_quad2d_t *quad,
                       wing_color_t color);
int wing_gui_fill_triangle(wing_context_t *ctx,
                           const wing_triangle2d_t *triangle,
                           wing_color_t color);
int wing_gui_draw_quad(wing_context_t *ctx, const wing_quad2d_t *quad,
                       uint16_t thickness, wing_color_t color);
wing_render_material_t wing_render_material_color(wing_color_t color);
wing_color_t wing_render_material_resolve_color(
  const wing_render_material_t *material);
int wing_gui_submit_render_node(wing_context_t *ctx,
                                const wing_render_node_t *node);
int wing_gui_blit(wing_context_t *ctx, const wing_surface_t *source,
                  const wing_rect_t *src_rect,
                  const wing_rect_t *dst_rect);
int wing_gui_blit_alpha(wing_context_t *ctx, const wing_surface_t *source,
                        const wing_rect_t *src_rect,
                        const wing_rect_t *dst_rect,
                        uint8_t global_alpha);
int wing_gui_blit_quad_alpha(wing_context_t *ctx,
                             const wing_surface_t *source,
                             const wing_rect_t *src_rect,
                             const wing_quad2d_t *dst_quad,
                             uint8_t global_alpha);
int wing_gui_draw_rect(wing_context_t *ctx, const wing_rect_t *rect,
                       uint16_t thickness, wing_color_t color);
int wing_gui_flush(wing_gui_t *gui, fr_backend_instance_t *backend);
uint16_t wing_gui_render_command_count(const wing_gui_t *gui);
uint32_t wing_surface_checksum_rgba8888(const wing_surface_t *surface);
int wing_gui_render_dirty(wing_gui_t *gui);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_RENDER_H */
