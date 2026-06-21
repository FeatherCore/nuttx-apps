/****************************************************************************
 * apps/graphics/wing/src/core/wing_render.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdint.h>

#include <wing/core/wing_dirty.h>
#include <wing/core/wing_render.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WING_RENDER_COMMANDS_PER_DIRTY_CHUNK_RESERVE 256
#define WING_RENDER_UNION_REDRAW_COST_NUMERATOR 8
#define WING_RENDER_UNION_REDRAW_COST_DENOMINATOR 1

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_render_surface_is_valid(const wing_surface_t *surface)
{
  return surface != NULL && surface->render_surface.pixels != NULL &&
         surface->render_surface.width > 0 &&
         surface->render_surface.height > 0 &&
         surface->render_surface.stride >= surface->render_surface.width &&
         surface->render_surface.format == FR_FORMAT_RGBA8888;
}

static wing_rect_t wing_render_surface_rect(const wing_surface_t *surface)
{
  wing_rect_t rect;

  rect.x = 0;
  rect.y = 0;
  rect.w = surface->render_surface.width;
  rect.h = surface->render_surface.height;

  return rect;
}

static fr_rect_t wing_render_rect_to_fr(const wing_rect_t *rect)
{
  fr_rect_t out;

  out.x = rect->x;
  out.y = rect->y;
  out.w = rect->w;
  out.h = rect->h;

  return out;
}

static fr_quad_t wing_render_quad_to_fr(const wing_quad2d_t *quad)
{
  fr_quad_t out;
  uint8_t i;

  for (i = 0; i < 4; i++)
    {
      out.points[i].x = quad->points[i].x;
      out.points[i].y = quad->points[i].y;
    }

  return out;
}

static fr_triangle_t wing_render_triangle_to_fr(
  const wing_triangle2d_t *triangle)
{
  fr_triangle_t out;
  uint8_t i;

  for (i = 0; i < 3; i++)
    {
      out.points[i].x = triangle->points[i].x;
      out.points[i].y = triangle->points[i].y;
    }

  return out;
}

static fr_color_t wing_render_color_to_fr(wing_color_t color)
{
  return fr_color_rgba(color.r, color.g, color.b, color.a);
}

static bool wing_render_rect_intersect(const wing_rect_t *a,
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

static int wing_render_require_active(const wing_context_t *ctx)
{
  if (ctx == NULL || !wing_render_surface_is_valid(ctx->surface) ||
      ctx->commands == NULL)
    {
      return -EINVAL;
    }

  if (!ctx->active)
    {
      return -EPERM;
    }

  return 0;
}

static uint32_t wing_render_rect_area(const wing_rect_t *rect)
{
  if (rect == NULL)
    {
      return 0;
    }

  return (uint32_t)rect->w * rect->h;
}

static bool wing_render_should_fallback_to_union_by_cost(
  const wing_gui_t *gui)
{
  uint32_t dirty_area;
  uint32_t union_area;
  uint8_t i;

  if (gui == NULL || !gui->has_dirty_rect || gui->dirty_rect_count < 2)
    {
      return false;
    }

  dirty_area = 0;
  for (i = 0; i < gui->dirty_rect_count; i++)
    {
      dirty_area += wing_render_rect_area(&gui->dirty_rects[i]);
    }

  if (dirty_area == 0)
    {
      return true;
    }

  union_area = wing_render_rect_area(&gui->dirty_rect);
  return union_area * WING_RENDER_UNION_REDRAW_COST_DENOMINATOR <=
         dirty_area * WING_RENDER_UNION_REDRAW_COST_NUMERATOR;
}

static uint8_t
wing_render_max_dirty_chunks_for_commands(const fr_command_list_t *commands)
{
  uint16_t max_chunks;

  if (commands == NULL ||
      commands->capacity < WING_RENDER_COMMANDS_PER_DIRTY_CHUNK_RESERVE)
    {
      return 1;
    }

  max_chunks = commands->capacity /
               WING_RENDER_COMMANDS_PER_DIRTY_CHUNK_RESERVE;
  if (max_chunks == 0)
    {
      return 1;
    }

  if (max_chunks > WING_GUI_DIRTY_RECT_MAX)
    {
      return WING_GUI_DIRTY_RECT_MAX;
    }

  return (uint8_t)max_chunks;
}

static void wing_render_abort_pass(wing_context_t *ctx)
{
  if (ctx == NULL || ctx->surface == NULL)
    {
      return;
    }

  ctx->clip = wing_render_surface_rect(ctx->surface);
  ctx->clip_depth = 0;
  ctx->active = false;
}

static int wing_render_dirty_pass(wing_gui_t *gui, uint8_t redraw_count,
                                  bool use_dirty_list)
{
  wing_rect_t dirty;
  uint8_t i;
  int ret;

  ret = wing_gui_begin(&gui->context);
  for (i = 0; ret == 0 && i < redraw_count; i++)
    {
      if (gui->has_dirty_rect)
        {
          dirty = use_dirty_list && i < gui->dirty_rect_count ?
                  gui->dirty_rects[i] : gui->dirty_rect;
          ret = wing_gui_set_clip(&gui->context, &dirty);
          if (ret < 0)
            {
              break;
            }
        }

      if (gui->render != NULL)
        {
          ret = gui->render(&gui->context, gui->render_arg);
        }

      if (ret == 0 && gui->root != NULL)
        {
          ret = wing_obj_draw_tree(gui->root, &gui->context,
                                   gui->has_dirty_rect ? &dirty : NULL);
        }

      if (ret == 0 && gui->has_dirty_rect)
        {
          ret = wing_gui_reset_clip(&gui->context);
        }
    }

  if (ret == 0)
    {
      ret = wing_gui_end(&gui->context);
    }

  if (ret < 0)
    {
      wing_render_abort_pass(&gui->context);
    }

  return ret;
}

static void wing_render_store_last_redraw_rects(wing_gui_t *gui,
                                                uint8_t redraw_count,
                                                bool use_dirty_list)
{
  wing_rect_t rect;
  uint8_t i;

  if (gui == NULL)
    {
      return;
    }

  for (i = 0; i < WING_GUI_DIRTY_RECT_MAX; i++)
    {
      gui->last_redraw_rects[i] = (wing_rect_t){0, 0, 0, 0};
    }

  if (redraw_count > WING_GUI_DIRTY_RECT_MAX)
    {
      redraw_count = WING_GUI_DIRTY_RECT_MAX;
    }

  for (i = 0; i < redraw_count; i++)
    {
      if (gui->has_dirty_rect)
        {
          rect = use_dirty_list && i < gui->dirty_rect_count ?
                 gui->dirty_rects[i] : gui->dirty_rect;
        }
      else
        {
          rect = wing_render_surface_rect(gui->surface);
        }

      gui->last_redraw_rects[i] = rect;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_surface_init(wing_surface_t *surface, void *pixels,
                      uint16_t width, uint16_t height, uint16_t stride,
                      enum wing_pixel_format_e format)
{
  if (surface == NULL || format != WING_PIXEL_FORMAT_RGBA8888)
    {
      return -EINVAL;
    }

  return fr_surface_init(&surface->render_surface, pixels, width, height,
                         stride, FR_FORMAT_RGBA8888);
}

int wing_gui_init(wing_context_t *ctx, wing_surface_t *surface,
                  fr_command_list_t *commands)
{
  if (ctx == NULL || !wing_render_surface_is_valid(surface) ||
      commands == NULL)
    {
      return -EINVAL;
    }

  ctx->surface = surface;
  ctx->commands = commands;
  ctx->clip = wing_render_surface_rect(surface);
  ctx->clip_depth = 0;
  ctx->active = false;

  return 0;
}

void wing_gui_deinit(wing_context_t *ctx)
{
  if (ctx != NULL)
    {
      ctx->surface = NULL;
      ctx->commands = NULL;
      ctx->clip.x = 0;
      ctx->clip.y = 0;
      ctx->clip.w = 0;
      ctx->clip.h = 0;
      ctx->clip_depth = 0;
      ctx->active = false;
    }
}

int wing_gui_begin(wing_context_t *ctx)
{
  if (ctx == NULL || !wing_render_surface_is_valid(ctx->surface) ||
      ctx->commands == NULL)
    {
      return -EINVAL;
    }

  fr_command_list_reset(ctx->commands);
  ctx->clip = wing_render_surface_rect(ctx->surface);
  ctx->clip_depth = 0;
  ctx->active = true;

  return 0;
}

int wing_gui_end(wing_context_t *ctx)
{
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  while (ctx->clip_depth > 0)
    {
      ret = fr_cmd_pop_clip(ctx->commands);
      if (ret < 0)
        {
          return ret;
        }

      ctx->clip_depth--;
      ctx->clip = ctx->clip_stack[ctx->clip_depth];
    }

  ctx->clip = wing_render_surface_rect(ctx->surface);
  ctx->active = false;
  return 0;
}

int wing_gui_set_clip(wing_context_t *ctx, const wing_rect_t *clip)
{
  wing_rect_t bounds;
  wing_rect_t clipped;
  fr_rect_t fr_rect;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (clip == NULL)
    {
      return -EINVAL;
    }

  if (ctx->clip_depth >= WING_GUI_CLIP_STACK_MAX)
    {
      return -ENOSPC;
    }

  bounds = ctx->clip;
  if (!wing_render_rect_intersect(&bounds, clip, &clipped))
    {
      clipped.x = 0;
      clipped.y = 0;
      clipped.w = 0;
      clipped.h = 0;
    }

  fr_rect = wing_render_rect_to_fr(&clipped);
  ret = fr_cmd_push_clip(ctx->commands, &fr_rect);
  if (ret < 0)
    {
      return ret;
    }

  ctx->clip_stack[ctx->clip_depth] = ctx->clip;
  ctx->clip = clipped;
  ctx->clip_depth++;
  return 0;
}

int wing_gui_reset_clip(wing_context_t *ctx)
{
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (ctx->clip_depth > 0)
    {
      ret = fr_cmd_pop_clip(ctx->commands);
      if (ret < 0)
        {
          return ret;
        }

      ctx->clip_depth--;
      ctx->clip = ctx->clip_stack[ctx->clip_depth];
      return 0;
    }

  ctx->clip = wing_render_surface_rect(ctx->surface);
  return 0;
}

int wing_gui_clear(wing_context_t *ctx, wing_color_t color)
{
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  return fr_cmd_clear(ctx->commands, wing_render_color_to_fr(color));
}

int wing_gui_fill_rect(wing_context_t *ctx, const wing_rect_t *rect,
                       wing_color_t color)
{
  fr_rect_t fr_rect;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (rect == NULL)
    {
      return -EINVAL;
    }

  fr_rect = wing_render_rect_to_fr(rect);
  return fr_cmd_fill_rect(ctx->commands, &fr_rect,
                          wing_render_color_to_fr(color));
}

int wing_gui_fill_quad(wing_context_t *ctx, const wing_quad2d_t *quad,
                       wing_color_t color)
{
  fr_quad_t fr_quad;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (quad == NULL)
    {
      return -EINVAL;
    }

  fr_quad = wing_render_quad_to_fr(quad);
  return fr_cmd_fill_quad(ctx->commands, &fr_quad,
                          wing_render_color_to_fr(color));
}

int wing_gui_fill_triangle(wing_context_t *ctx,
                           const wing_triangle2d_t *triangle,
                           wing_color_t color)
{
  fr_triangle_t fr_triangle;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (triangle == NULL)
    {
      return -EINVAL;
    }

  fr_triangle = wing_render_triangle_to_fr(triangle);
  return fr_cmd_fill_triangle(ctx->commands, &fr_triangle,
                              wing_render_color_to_fr(color));
}

wing_render_material_t wing_render_material_color(wing_color_t color)
{
  wing_render_material_t material;

  material.color = color;
  material.opacity = color.a;
  return material;
}

wing_color_t wing_render_material_resolve_color(
  const wing_render_material_t *material)
{
  wing_color_t color;

  if (material == NULL)
    {
      return wing_color_rgba(0, 0, 0, 0);
    }

  color = material->color;
  color.a = (uint8_t)(((uint16_t)color.a * material->opacity) / 255);
  return color;
}

int wing_gui_submit_render_node(wing_context_t *ctx,
                                const wing_render_node_t *node)
{
  wing_color_t color;

  if (node == NULL)
    {
      return -EINVAL;
    }

  color = wing_render_material_resolve_color(&node->material);

  switch (node->type)
    {
      case WING_RENDER_NODE_FILL_RECT:
        return wing_gui_fill_rect(ctx, &node->geometry.rect, color);

      case WING_RENDER_NODE_FILL_QUAD:
        return wing_gui_fill_quad(ctx, &node->geometry.quad, color);

      case WING_RENDER_NODE_FILL_TRIANGLE:
        return wing_gui_fill_triangle(ctx, &node->geometry.triangle, color);

      case WING_RENDER_NODE_DRAW_QUAD:
        return wing_gui_draw_quad(ctx, &node->geometry.quad,
                                  node->thickness, color);

      case WING_RENDER_NODE_BLIT:
        return wing_gui_blit_alpha(ctx, node->source, &node->src_rect,
                                   &node->geometry.rect, color.a);

      case WING_RENDER_NODE_BLIT_QUAD:
        return wing_gui_blit_quad_alpha(ctx, node->source, &node->src_rect,
                                        &node->geometry.quad, color.a);

      default:
        return -EINVAL;
    }
}

int wing_gui_draw_quad(wing_context_t *ctx, const wing_quad2d_t *quad,
                       uint16_t thickness, wing_color_t color)
{
  fr_quad_t fr_quad;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (quad == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  fr_quad = wing_render_quad_to_fr(quad);
  return fr_cmd_stroke_quad(ctx->commands, &fr_quad, thickness,
                            wing_render_color_to_fr(color));
}

int wing_gui_blit(wing_context_t *ctx, const wing_surface_t *source,
                  const wing_rect_t *src_rect,
                  const wing_rect_t *dst_rect)
{
  return wing_gui_blit_alpha(ctx, source, src_rect, dst_rect, 255);
}

int wing_gui_blit_alpha(wing_context_t *ctx, const wing_surface_t *source,
                        const wing_rect_t *src_rect,
                        const wing_rect_t *dst_rect,
                        uint8_t global_alpha)
{
  fr_rect_t fr_src;
  fr_rect_t fr_dst;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (!wing_render_surface_is_valid(source) || src_rect == NULL ||
      dst_rect == NULL || src_rect->w == 0 || src_rect->h == 0 ||
      dst_rect->w == 0 || dst_rect->h == 0)
    {
      return -EINVAL;
    }

  fr_src = wing_render_rect_to_fr(src_rect);
  fr_dst = wing_render_rect_to_fr(dst_rect);
  return fr_cmd_blit_alpha(ctx->commands, &source->render_surface, &fr_src,
                           &fr_dst, global_alpha);
}

int wing_gui_blit_quad_alpha(wing_context_t *ctx,
                             const wing_surface_t *source,
                             const wing_rect_t *src_rect,
                             const wing_quad2d_t *dst_quad,
                             uint8_t global_alpha)
{
  fr_rect_t fr_src;
  fr_quad_t fr_quad;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (!wing_render_surface_is_valid(source) || src_rect == NULL ||
      dst_quad == NULL || src_rect->w == 0 || src_rect->h == 0)
    {
      return -EINVAL;
    }

  fr_src = wing_render_rect_to_fr(src_rect);
  fr_quad = wing_render_quad_to_fr(dst_quad);
  return fr_cmd_blit_quad_alpha(ctx->commands, &source->render_surface,
                                &fr_src, &fr_quad, global_alpha);
}

int wing_gui_draw_rect(wing_context_t *ctx, const wing_rect_t *rect,
                       uint16_t thickness, wing_color_t color)
{
  fr_rect_t fr_rect;
  int ret;

  ret = wing_render_require_active(ctx);
  if (ret < 0)
    {
      return ret;
    }

  if (rect == NULL || thickness == 0)
    {
      return -EINVAL;
    }

  fr_rect = wing_render_rect_to_fr(rect);
  return fr_cmd_stroke_rect(ctx->commands, &fr_rect, thickness,
                            wing_render_color_to_fr(color));
}

int wing_gui_flush(wing_gui_t *gui, fr_backend_instance_t *backend)
{
  if (gui == NULL || backend == NULL || gui->commands == NULL)
    {
      return -EINVAL;
    }

  return fr_execute(backend, gui->commands);
}

uint16_t wing_gui_render_command_count(const wing_gui_t *gui)
{
  if (gui == NULL || gui->commands == NULL)
    {
      return 0;
    }

  return fr_command_list_count(gui->commands);
}

uint32_t wing_surface_checksum_rgba8888(const wing_surface_t *surface)
{
  if (!wing_render_surface_is_valid(surface))
    {
      return 0;
    }

  return fr_surface_checksum_rgba8888(&surface->render_surface);
}

int wing_gui_render_dirty(wing_gui_t *gui)
{
  uint8_t max_redraw_count;
  uint8_t planned_redraw_count;
  uint8_t redraw_count;
  bool use_dirty_list;
  int ret;

  if (gui == NULL || !gui->running)
    {
      return -EINVAL;
    }

  if (!gui->dirty)
    {
      wing_render_store_last_redraw_rects(gui, 0, false);
      gui->last_planned_redraw_count = 0;
      gui->last_redraw_count = 0;
      gui->last_command_capacity_fallback = false;
      gui->last_redraw_cost_fallback = false;
      return 0;
    }

  planned_redraw_count = gui->has_dirty_rect && gui->dirty_rect_count > 0 ?
                         gui->dirty_rect_count : 1;
  redraw_count = planned_redraw_count;
  gui->last_command_capacity_fallback = false;
  gui->last_redraw_cost_fallback = false;

  max_redraw_count =
    wing_render_max_dirty_chunks_for_commands(gui->commands);
  if (planned_redraw_count > max_redraw_count)
    {
      redraw_count = 1;
      gui->last_command_capacity_fallback = true;
    }
  else if (wing_render_should_fallback_to_union_by_cost(gui))
    {
      redraw_count = 1;
      gui->last_redraw_cost_fallback = true;
    }

  use_dirty_list = gui->has_dirty_rect &&
                   gui->dirty_rect_count > 0 &&
                   !gui->last_command_capacity_fallback &&
                   !gui->last_redraw_cost_fallback;
  gui->last_planned_redraw_count = planned_redraw_count;
  gui->last_redraw_count = redraw_count;

  ret = wing_render_dirty_pass(gui, redraw_count, use_dirty_list);
  if (ret == -ENOSPC && use_dirty_list)
    {
      redraw_count = 1;
      use_dirty_list = false;
      gui->last_command_capacity_fallback = true;
      gui->last_redraw_count = redraw_count;

      ret = wing_render_dirty_pass(gui, redraw_count, use_dirty_list);
    }

  if (ret < 0)
    {
      return ret;
    }

  wing_render_store_last_redraw_rects(gui, redraw_count, use_dirty_list);
  wing_gui_clear_dirty(gui);
  gui->last_redraw_count = redraw_count;
  gui->last_frame_ms = gui->tick_ms;
  return 1;
}
