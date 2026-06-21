/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_image.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>

#include <wing/wing.h>
#include <wing/core/wing_object.h>
#include <wing/core/wing_render.h>
#include <wing/core/wing_space.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_image_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  const wing_rect_t *bounds;
  wing_image_t *image;
  const wing_image_resource_t *resource;
  wing_rect_t src;
  wing_quad2d_t quad;
  wing_surface_t source;
  wing_space_transform_t world_transform;
  uint8_t opacity;
  int ret;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  image = (wing_image_t *)obj;
  resource = image->resource;
  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL || resource == NULL || resource->pixels == NULL ||
      resource->width == 0 || resource->height == 0 ||
      resource->stride == 0)
    {
      return 0;
    }

  if (resource->format != WING_PIXEL_FORMAT_RGBA8888)
    {
      return -ENOSYS;
    }

  opacity = wing_obj_get_effective_opacity(obj);
  if (opacity == 0)
    {
      return 0;
    }

  ret = wing_surface_init(&source, (void *)resource->pixels,
                          resource->width, resource->height,
                          resource->stride, resource->format);
  if (ret < 0)
    {
      return ret;
    }

  src.x = 0;
  src.y = 0;
  src.w = resource->width;
  src.h = resource->height;

  if (wing_obj_get_world_space_transform(obj, &world_transform) == 0 &&
      !wing_space_transform_is_identity(&world_transform) &&
      wing_obj_project_quad2d(obj, &quad) == 0)
    {
      return wing_gui_blit_quad_alpha(ctx, &source, &src, &quad, opacity);
    }

  return wing_gui_blit_alpha(ctx, &source, &src, bounds, opacity);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_image_init(wing_image_t *image, const wing_rect_t *bounds,
                     const wing_color_t *pixels, uint16_t width,
                     uint16_t height, uint16_t stride, uint8_t scale)
{
  if (image == NULL)
    {
      return;
    }

  image->inline_resource.pixels = pixels;
  image->inline_resource.width = width;
  image->inline_resource.height = height;
  image->inline_resource.stride = stride == 0 ? width : stride;
  image->inline_resource.format = WING_PIXEL_FORMAT_RGBA8888;
  wing_image_init_resource(image, bounds, &image->inline_resource, scale);
}

void wing_image_init_resource(wing_image_t *image, const wing_rect_t *bounds,
                              const wing_image_resource_t *resource,
                              uint8_t scale)
{
  if (image == NULL)
    {
      return;
    }

  wing_obj_init(&image->obj, bounds);
  image->resource = resource;
  image->scale = scale == 0 ? 1 : scale;
  wing_obj_set_draw_cb(&image->obj, wing_image_draw);
}

wing_obj_t *wing_image_obj(wing_image_t *image)
{
  return image == NULL ? NULL : &image->obj;
}

void wing_image_set_source(wing_image_t *image,
                           const wing_color_t *pixels, uint16_t width,
                           uint16_t height, uint16_t stride)
{
  if (image == NULL)
    {
      return;
    }

  image->inline_resource.pixels = pixels;
  image->inline_resource.width = width;
  image->inline_resource.height = height;
  image->inline_resource.stride = stride == 0 ? width : stride;
  image->inline_resource.format = WING_PIXEL_FORMAT_RGBA8888;
  wing_image_set_resource(image, &image->inline_resource);
}

void wing_image_set_resource(wing_image_t *image,
                             const wing_image_resource_t *resource)
{
  if (image == NULL)
    {
      return;
    }

  image->resource = resource;
  wing_obj_invalidate(&image->obj);
}

void wing_image_set_scale(wing_image_t *image, uint8_t scale)
{
  if (image == NULL)
    {
      return;
    }

  image->scale = scale == 0 ? 1 : scale;
  wing_obj_invalidate(&image->obj);
}
