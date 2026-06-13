/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_space.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_SPACE_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_SPACE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* WING space is the default object coordinate space.
 *
 * It is not an optional 3D view layer: ordinary 2D widgets are identity
 * transform objects in this same space, while cards, app switchers, previews,
 * and future mesh/shader paths consume the same camera/transform projection
 * data.
 */

#ifdef __cplusplus
extern "C"
{
#endif

void wing_viewport_init(wing_viewport_t *viewport, int16_t x, int16_t y,
                        uint16_t w, uint16_t h);
void wing_camera_init_default(wing_camera_t *camera,
                              const wing_viewport_t *viewport);
bool wing_camera_equal(const wing_camera_t *a, const wing_camera_t *b);
void wing_space_transform_init(wing_space_transform_t *transform);
bool wing_space_transform_is_identity(
  const wing_space_transform_t *transform);
bool wing_space_transform_is_default_2d(
  const wing_space_transform_t *transform);
bool wing_space_transform_equal(const wing_space_transform_t *a,
                                const wing_space_transform_t *b);
int wing_space_transform_compose(const wing_space_transform_t *parent,
                                 const wing_space_transform_t *local,
                                 wing_space_transform_t *out);
int wing_space_transform_apply_point(const wing_space_transform_t *transform,
                                     const wing_vec3_t *local,
                                     wing_vec3_t *world);
int wing_project_point(const wing_camera_t *camera,
                       const wing_space_transform_t *transform,
                       const wing_vec3_t *local,
                       wing_point_t *screen);
int wing_project_point_with_depth(const wing_camera_t *camera,
                                  const wing_space_transform_t *transform,
                                  const wing_vec3_t *local,
                                  wing_point_t *screen,
                                  int32_t *projected_depth);
int wing_project_rect_quad(const wing_camera_t *camera,
                           const wing_space_transform_t *transform,
                           const wing_rect_t *rect,
                           wing_quad2d_t *quad);
int wing_project_rect_projected_quad(const wing_camera_t *camera,
                                     const wing_space_transform_t *transform,
                                     const wing_rect_t *rect,
                                     wing_projected_quad_t *quad);
int wing_project_triangle(const wing_camera_t *camera,
                          const wing_space_transform_t *transform,
                          const wing_vec3_t *local_points,
                          wing_triangle2d_t *triangle);
int wing_project_projected_triangle(const wing_camera_t *camera,
                                    const wing_space_transform_t *transform,
                                    const wing_vec3_t *local_points,
                                    wing_projected_triangle_t *triangle);
int wing_projected_quad_average_depth(const wing_projected_quad_t *quad,
                                      int32_t *average_depth);
int wing_projected_triangle_average_depth(
  const wing_projected_triangle_t *triangle, int32_t *average_depth);
int wing_quad2d_get_bounds(const wing_quad2d_t *quad, wing_rect_t *bounds);
int wing_triangle2d_get_bounds(const wing_triangle2d_t *triangle,
                               wing_rect_t *bounds);
int wing_projected_triangle_get_bounds(
  const wing_projected_triangle_t *triangle, wing_rect_t *bounds);
bool wing_quad2d_contains_point(const wing_quad2d_t *quad,
                                wing_point_t point);
bool wing_triangle2d_contains_point(const wing_triangle2d_t *triangle,
                                    wing_point_t point);
bool wing_projected_triangle_contains_point(
  const wing_projected_triangle_t *triangle, wing_point_t point);
int wing_project_rect_bounds(const wing_camera_t *camera,
                             const wing_space_transform_t *transform,
                             const wing_rect_t *rect,
                             wing_rect_t *bounds);
int wing_project_triangle_bounds(const wing_camera_t *camera,
                                 const wing_space_transform_t *transform,
                                 const wing_vec3_t *local_points,
                                 wing_rect_t *bounds);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_SPACE_H */
