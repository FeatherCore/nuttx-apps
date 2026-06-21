/****************************************************************************
 * apps/graphics/wing/src/core/wing_space.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <wing/core/wing_space.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int16_t wing_space_normalize_degrees(int16_t degrees)
{
  while (degrees > 180)
    {
      degrees = (int16_t)(degrees - 360);
    }

  while (degrees < -180)
    {
      degrees = (int16_t)(degrees + 360);
    }

  return degrees;
}

static int32_t wing_space_sin_q10(int16_t degrees)
{
  int16_t d;

  d = wing_space_normalize_degrees(degrees);
  if (d >= 0 && d <= 90)
    {
      return (int32_t)d * 1024 / 90;
    }

  if (d > 90)
    {
      return (int32_t)(180 - d) * 1024 / 90;
    }

  if (d >= -90)
    {
      return (int32_t)d * 1024 / 90;
    }

  return (int32_t)(-180 - d) * 1024 / 90;
}

static int32_t wing_space_cos_q10(int16_t degrees)
{
  return wing_space_sin_q10((int16_t)(degrees + 90));
}

static void wing_space_rotate_pair(int32_t *a, int32_t *b,
                                   int32_t sin_q10, int32_t cos_q10)
{
  int32_t next_a;
  int32_t next_b;

  next_a = (*a * cos_q10 - *b * sin_q10) / 1024;
  next_b = (*a * sin_q10 + *b * cos_q10) / 1024;

  *a = next_a;
  *b = next_b;
}

static int32_t wing_space_edge_cross(const wing_point_t *a,
                                     const wing_point_t *b,
                                     wing_point_t point)
{
  return ((int32_t)b->x - a->x) * ((int32_t)point.y - a->y) -
         ((int32_t)b->y - a->y) * ((int32_t)point.x - a->x);
}

static int32_t wing_space_clamp_depth(const wing_camera_t *camera,
                                      int32_t depth)
{
  if (camera == NULL)
    {
      return depth;
    }

  if (depth < camera->near_z)
    {
      depth = camera->near_z;
    }

  if (depth > camera->far_z)
    {
      depth = camera->far_z;
    }

  return depth;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_viewport_init(wing_viewport_t *viewport, int16_t x, int16_t y,
                        uint16_t w, uint16_t h)
{
  if (viewport == NULL)
    {
      return;
    }

  viewport->x = x;
  viewport->y = y;
  viewport->w = w;
  viewport->h = h;
}

void wing_camera_init_default(wing_camera_t *camera,
                              const wing_viewport_t *viewport)
{
  if (camera == NULL)
    {
      return;
    }

  camera->position.x = 0;
  camera->position.y = 0;
  camera->position.z = -256;
  camera->target.x = 0;
  camera->target.y = 0;
  camera->target.z = 0;
  camera->near_z = 1;
  camera->far_z = 32767;

  if (viewport != NULL)
    {
      camera->viewport = *viewport;
    }
  else
    {
      wing_viewport_init(&camera->viewport, 0, 0, 0, 0);
    }

  camera->focal_length = camera->viewport.w > 0 ? camera->viewport.w : 256;
}

bool wing_camera_equal(const wing_camera_t *a, const wing_camera_t *b)
{
  return a != NULL && b != NULL &&
         a->position.x == b->position.x &&
         a->position.y == b->position.y &&
         a->position.z == b->position.z &&
         a->target.x == b->target.x &&
         a->target.y == b->target.y &&
         a->target.z == b->target.z &&
         a->viewport.x == b->viewport.x &&
         a->viewport.y == b->viewport.y &&
         a->viewport.w == b->viewport.w &&
         a->viewport.h == b->viewport.h &&
         a->near_z == b->near_z &&
         a->far_z == b->far_z &&
         a->focal_length == b->focal_length;
}

void wing_space_transform_init(wing_space_transform_t *transform)
{
  if (transform == NULL)
    {
      return;
    }

  transform->translation.x = 0;
  transform->translation.y = 0;
  transform->translation.z = 0;
  transform->rotation_x = 0;
  transform->rotation_y = 0;
  transform->rotation_z = 0;
  transform->scale_q10 = 1024;
}

bool wing_space_transform_is_identity(
  const wing_space_transform_t *transform)
{
  return transform != NULL &&
         transform->translation.x == 0 &&
         transform->translation.y == 0 &&
         transform->translation.z == 0 &&
         transform->rotation_x == 0 &&
         transform->rotation_y == 0 &&
         transform->rotation_z == 0 &&
         transform->scale_q10 == 1024;
}

bool wing_space_transform_is_default_2d(
  const wing_space_transform_t *transform)
{
  return wing_space_transform_is_identity(transform);
}

bool wing_space_transform_equal(const wing_space_transform_t *a,
                                const wing_space_transform_t *b)
{
  return a != NULL && b != NULL &&
         a->translation.x == b->translation.x &&
         a->translation.y == b->translation.y &&
         a->translation.z == b->translation.z &&
         a->rotation_x == b->rotation_x &&
         a->rotation_y == b->rotation_y &&
         a->rotation_z == b->rotation_z &&
         a->scale_q10 == b->scale_q10;
}

int wing_space_transform_compose(const wing_space_transform_t *parent,
                                 const wing_space_transform_t *local,
                                 wing_space_transform_t *out)
{
  wing_space_transform_t identity;
  int32_t x;
  int32_t y;
  int32_t z;

  if (local == NULL || out == NULL)
    {
      return -EINVAL;
    }

  if (parent == NULL)
    {
      wing_space_transform_init(&identity);
      parent = &identity;
    }

  x = (int32_t)local->translation.x * parent->scale_q10 / 1024;
  y = (int32_t)local->translation.y * parent->scale_q10 / 1024;
  z = (int32_t)local->translation.z * parent->scale_q10 / 1024;

  wing_space_rotate_pair(&y, &z,
                         wing_space_sin_q10(parent->rotation_x),
                         wing_space_cos_q10(parent->rotation_x));
  wing_space_rotate_pair(&x, &z,
                         wing_space_sin_q10(parent->rotation_y),
                         wing_space_cos_q10(parent->rotation_y));
  wing_space_rotate_pair(&x, &y,
                         wing_space_sin_q10(parent->rotation_z),
                         wing_space_cos_q10(parent->rotation_z));

  out->translation.x = (int16_t)(parent->translation.x + x);
  out->translation.y = (int16_t)(parent->translation.y + y);
  out->translation.z = (int16_t)(parent->translation.z + z);
  out->rotation_x =
    wing_space_normalize_degrees((int16_t)(parent->rotation_x +
                                           local->rotation_x));
  out->rotation_y =
    wing_space_normalize_degrees((int16_t)(parent->rotation_y +
                                           local->rotation_y));
  out->rotation_z =
    wing_space_normalize_degrees((int16_t)(parent->rotation_z +
                                           local->rotation_z));
  out->scale_q10 =
    (uint16_t)((uint32_t)parent->scale_q10 * local->scale_q10 / 1024);

  return 0;
}

int wing_space_transform_apply_point(const wing_space_transform_t *transform,
                                     const wing_vec3_t *local,
                                     wing_vec3_t *world)
{
  wing_space_transform_t identity;
  int32_t x;
  int32_t y;
  int32_t z;

  if (local == NULL || world == NULL)
    {
      return -EINVAL;
    }

  if (transform == NULL)
    {
      wing_space_transform_init(&identity);
      transform = &identity;
    }

  x = (int32_t)local->x * transform->scale_q10 / 1024;
  y = (int32_t)local->y * transform->scale_q10 / 1024;
  z = (int32_t)local->z * transform->scale_q10 / 1024;

  wing_space_rotate_pair(&y, &z,
                         wing_space_sin_q10(transform->rotation_x),
                         wing_space_cos_q10(transform->rotation_x));
  wing_space_rotate_pair(&x, &z,
                         wing_space_sin_q10(transform->rotation_y),
                         wing_space_cos_q10(transform->rotation_y));
  wing_space_rotate_pair(&x, &y,
                         wing_space_sin_q10(transform->rotation_z),
                         wing_space_cos_q10(transform->rotation_z));

  world->x = (int16_t)(x + transform->translation.x);
  world->y = (int16_t)(y + transform->translation.y);
  world->z = (int16_t)(z + transform->translation.z);

  return 0;
}

int wing_project_point_with_depth(const wing_camera_t *camera,
                                  const wing_space_transform_t *transform,
                                  const wing_vec3_t *local,
                                  wing_point_t *screen,
                                  int32_t *projected_depth)
{
  wing_vec3_t world;
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t depth;
  int32_t cx;
  int32_t cy;

  if (camera == NULL || local == NULL || screen == NULL ||
      camera->viewport.w == 0 || camera->viewport.h == 0 ||
      camera->focal_length == 0)
    {
      return -EINVAL;
    }

  if (wing_space_transform_apply_point(transform, local, &world) < 0)
    {
      return -EINVAL;
    }

  x = (int32_t)world.x - camera->position.x;
  y = (int32_t)world.y - camera->position.y;
  z = (int32_t)world.z - camera->position.z;

  depth = wing_space_clamp_depth(camera, z);

  if (projected_depth != NULL)
    {
      *projected_depth = depth;
    }

  cx = (int32_t)camera->viewport.x + camera->viewport.w / 2;
  cy = (int32_t)camera->viewport.y + camera->viewport.h / 2;
  screen->x = (int16_t)(cx + x * camera->focal_length / depth);
  screen->y = (int16_t)(cy + y * camera->focal_length / depth);

  return 0;
}

int wing_project_point(const wing_camera_t *camera,
                       const wing_space_transform_t *transform,
                       const wing_vec3_t *local,
                       wing_point_t *screen)
{
  return wing_project_point_with_depth(camera, transform, local, screen,
                                       NULL);
}

int wing_project_rect_quad(const wing_camera_t *camera,
                           const wing_space_transform_t *transform,
                           const wing_rect_t *rect,
                           wing_quad2d_t *quad)
{
  wing_projected_quad_t projected;
  uint8_t i;

  if (quad == NULL)
    {
      return -EINVAL;
    }

  if (wing_project_rect_projected_quad(camera, transform, rect,
                                       &projected) < 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < 4; i++)
    {
      quad->points[i] = projected.vertices[i].screen;
    }

  return 0;
}

int wing_project_rect_projected_quad(const wing_camera_t *camera,
                                     const wing_space_transform_t *transform,
                                     const wing_rect_t *rect,
                                     wing_projected_quad_t *quad)
{
  wing_camera_t active_camera;
  wing_point_t point;
  int32_t depth;
  wing_space_transform_t active_transform;
  wing_space_transform_t identity;
  wing_vec3_t local[4];
  wing_viewport_t viewport;
  int32_t rect_cx;
  int32_t rect_cy;
  int32_t viewport_cx;
  int32_t viewport_cy;
  int16_t bottom;
  int16_t left;
  int16_t right;
  int16_t top;
  uint8_t i;

  if (rect == NULL || quad == NULL || rect->w == 0 || rect->h == 0)
    {
      return -EINVAL;
    }

  if (transform == NULL)
    {
      wing_space_transform_init(&identity);
      transform = &identity;
    }

  if (wing_space_transform_is_identity(transform))
    {
      depth = camera != NULL ?
              wing_space_clamp_depth(camera, -camera->position.z) : 0;
      quad->vertices[0].screen.x = rect->x;
      quad->vertices[0].screen.y = rect->y;
      quad->vertices[1].screen.x = (int16_t)(rect->x + rect->w);
      quad->vertices[1].screen.y = rect->y;
      quad->vertices[2].screen.x = quad->vertices[1].screen.x;
      quad->vertices[2].screen.y = (int16_t)(rect->y + rect->h);
      quad->vertices[3].screen.x = rect->x;
      quad->vertices[3].screen.y = quad->vertices[2].screen.y;

      for (i = 0; i < 4; i++)
        {
          quad->vertices[i].depth = depth;
        }

      return 0;
    }

  left = (int16_t)(-((int16_t)rect->w / 2));
  top = (int16_t)(-((int16_t)rect->h / 2));
  right = (int16_t)(left + rect->w);
  bottom = (int16_t)(top + rect->h);

  local[0].x = left;
  local[0].y = top;
  local[0].z = 0;
  local[1].x = right;
  local[1].y = top;
  local[1].z = 0;
  local[2].x = right;
  local[2].y = bottom;
  local[2].z = 0;
  local[3].x = left;
  local[3].y = bottom;
  local[3].z = 0;

  active_transform = *transform;
  if (camera != NULL)
    {
      active_camera = *camera;
      rect_cx = (int32_t)rect->x + rect->w / 2;
      rect_cy = (int32_t)rect->y + rect->h / 2;
      viewport_cx = (int32_t)active_camera.viewport.x +
                    active_camera.viewport.w / 2;
      viewport_cy = (int32_t)active_camera.viewport.y +
                    active_camera.viewport.h / 2;
      active_transform.translation.x =
        (int16_t)(active_transform.translation.x + rect_cx -
                  viewport_cx);
      active_transform.translation.y =
        (int16_t)(active_transform.translation.y + rect_cy -
                  viewport_cy);
    }
  else
    {
      wing_viewport_init(&viewport, rect->x, rect->y, rect->w, rect->h);
      wing_camera_init_default(&active_camera, &viewport);
      active_camera.position.z = -256;
      active_camera.focal_length = 256;
    }

  for (i = 0; i < 4; i++)
    {
      if (wing_project_point_with_depth(&active_camera, &active_transform,
                                        &local[i], &point, &depth) < 0)
        {
          return -EINVAL;
        }

      quad->vertices[i].screen = point;
      quad->vertices[i].depth = depth;
    }

  return 0;
}

int wing_projected_quad_average_depth(const wing_projected_quad_t *quad,
                                      int32_t *average_depth)
{
  int32_t sum;
  uint8_t i;

  if (quad == NULL || average_depth == NULL)
    {
      return -EINVAL;
    }

  sum = 0;
  for (i = 0; i < 4; i++)
    {
      sum += quad->vertices[i].depth;
    }

  *average_depth = sum / 4;
  return 0;
}

int wing_project_projected_triangle(const wing_camera_t *camera,
                                    const wing_space_transform_t *transform,
                                    const wing_vec3_t *local_points,
                                    wing_projected_triangle_t *triangle)
{
  wing_point_t point;
  int32_t depth;
  uint8_t i;

  if (camera == NULL || local_points == NULL || triangle == NULL)
    {
      return -EINVAL;
    }

  for (i = 0; i < 3; i++)
    {
      if (wing_project_point_with_depth(camera, transform,
                                        &local_points[i],
                                        &point, &depth) < 0)
        {
          return -EINVAL;
        }

      triangle->vertices[i].screen = point;
      triangle->vertices[i].depth = depth;
    }

  return 0;
}

int wing_project_triangle(const wing_camera_t *camera,
                          const wing_space_transform_t *transform,
                          const wing_vec3_t *local_points,
                          wing_triangle2d_t *triangle)
{
  wing_projected_triangle_t projected;
  uint8_t i;

  if (triangle == NULL)
    {
      return -EINVAL;
    }

  if (wing_project_projected_triangle(camera, transform, local_points,
                                      &projected) < 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < 3; i++)
    {
      triangle->points[i] = projected.vertices[i].screen;
    }

  return 0;
}

int wing_projected_triangle_average_depth(
  const wing_projected_triangle_t *triangle, int32_t *average_depth)
{
  int32_t sum;
  uint8_t i;

  if (triangle == NULL || average_depth == NULL)
    {
      return -EINVAL;
    }

  sum = 0;
  for (i = 0; i < 3; i++)
    {
      sum += triangle->vertices[i].depth;
    }

  *average_depth = sum / 3;
  return 0;
}

int wing_triangle2d_get_bounds(const wing_triangle2d_t *triangle,
                               wing_rect_t *bounds)
{
  int16_t max_x;
  int16_t max_y;
  int16_t min_x;
  int16_t min_y;
  uint8_t i;

  if (triangle == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  min_x = triangle->points[0].x;
  max_x = triangle->points[0].x;
  min_y = triangle->points[0].y;
  max_y = triangle->points[0].y;

  for (i = 1; i < 3; i++)
    {
      if (triangle->points[i].x < min_x)
        {
          min_x = triangle->points[i].x;
        }
      else if (triangle->points[i].x > max_x)
        {
          max_x = triangle->points[i].x;
        }

      if (triangle->points[i].y < min_y)
        {
          min_y = triangle->points[i].y;
        }
      else if (triangle->points[i].y > max_y)
        {
          max_y = triangle->points[i].y;
        }
    }

  bounds->x = (int16_t)(min_x - 1);
  bounds->y = (int16_t)(min_y - 1);
  bounds->w = (uint16_t)(max_x - min_x + 3);
  bounds->h = (uint16_t)(max_y - min_y + 3);
  return 0;
}

int wing_projected_triangle_get_bounds(
  const wing_projected_triangle_t *triangle, wing_rect_t *bounds)
{
  wing_triangle2d_t screen_triangle;
  uint8_t i;

  if (triangle == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  for (i = 0; i < 3; i++)
    {
      screen_triangle.points[i] = triangle->vertices[i].screen;
    }

  return wing_triangle2d_get_bounds(&screen_triangle, bounds);
}

int wing_quad2d_get_bounds(const wing_quad2d_t *quad, wing_rect_t *bounds)
{
  int16_t max_x;
  int16_t max_y;
  int16_t min_x;
  int16_t min_y;
  uint8_t i;

  if (quad == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  min_x = quad->points[0].x;
  max_x = quad->points[0].x;
  min_y = quad->points[0].y;
  max_y = quad->points[0].y;

  for (i = 1; i < 4; i++)
    {
      if (quad->points[i].x < min_x)
        {
          min_x = quad->points[i].x;
        }
      else if (quad->points[i].x > max_x)
        {
          max_x = quad->points[i].x;
        }

      if (quad->points[i].y < min_y)
        {
          min_y = quad->points[i].y;
        }
      else if (quad->points[i].y > max_y)
        {
          max_y = quad->points[i].y;
        }
    }

  bounds->x = (int16_t)(min_x - 1);
  bounds->y = (int16_t)(min_y - 1);
  bounds->w = (uint16_t)(max_x - min_x + 3);
  bounds->h = (uint16_t)(max_y - min_y + 3);
  return 0;
}

bool wing_quad2d_contains_point(const wing_quad2d_t *quad,
                                wing_point_t point)
{
  bool has_negative;
  bool has_positive;
  int32_t cross;
  uint8_t i;

  if (quad == NULL)
    {
      return false;
    }

  has_negative = false;
  has_positive = false;

  for (i = 0; i < 4; i++)
    {
      cross = wing_space_edge_cross(&quad->points[i],
                                    &quad->points[(i + 1) & 3],
                                    point);
      if (cross < 0)
        {
          has_negative = true;
        }
      else if (cross > 0)
        {
          has_positive = true;
        }

      if (has_negative && has_positive)
        {
          return false;
        }
    }

  return true;
}

bool wing_triangle2d_contains_point(const wing_triangle2d_t *triangle,
                                    wing_point_t point)
{
  bool has_negative;
  bool has_positive;
  int32_t cross;
  uint8_t i;

  if (triangle == NULL)
    {
      return false;
    }

  has_negative = false;
  has_positive = false;

  for (i = 0; i < 3; i++)
    {
      cross = wing_space_edge_cross(&triangle->points[i],
                                    &triangle->points[(i + 1) % 3],
                                    point);
      if (cross < 0)
        {
          has_negative = true;
        }
      else if (cross > 0)
        {
          has_positive = true;
        }

      if (has_negative && has_positive)
        {
          return false;
        }
    }

  return true;
}

bool wing_projected_triangle_contains_point(
  const wing_projected_triangle_t *triangle, wing_point_t point)
{
  wing_triangle2d_t screen_triangle;
  uint8_t i;

  if (triangle == NULL)
    {
      return false;
    }

  for (i = 0; i < 3; i++)
    {
      screen_triangle.points[i] = triangle->vertices[i].screen;
    }

  return wing_triangle2d_contains_point(&screen_triangle, point);
}

int wing_project_rect_bounds(const wing_camera_t *camera,
                             const wing_space_transform_t *transform,
                             const wing_rect_t *rect,
                             wing_rect_t *bounds)
{
  wing_space_transform_t identity;
  wing_quad2d_t quad;

  if (rect == NULL || bounds == NULL || rect->w == 0 || rect->h == 0)
    {
      return -EINVAL;
    }

  if (transform == NULL)
    {
      wing_space_transform_init(&identity);
      transform = &identity;
    }

  if (wing_space_transform_is_identity(transform))
    {
      *bounds = *rect;
      return 0;
    }

  if (wing_project_rect_quad(camera, transform, rect, &quad) < 0)
    {
      return -EINVAL;
    }

  return wing_quad2d_get_bounds(&quad, bounds);
}

int wing_project_triangle_bounds(const wing_camera_t *camera,
                                 const wing_space_transform_t *transform,
                                 const wing_vec3_t *local_points,
                                 wing_rect_t *bounds)
{
  wing_projected_triangle_t triangle;

  if (local_points == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  if (wing_project_projected_triangle(camera, transform, local_points,
                                      &triangle) < 0)
    {
      return -EINVAL;
    }

  return wing_projected_triangle_get_bounds(&triangle, bounds);
}
