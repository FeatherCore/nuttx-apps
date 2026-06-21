/****************************************************************************
 * apps/graphics/wing/src/core/wing_object.c
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

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool wing_obj_rect_intersect(const wing_rect_t *a,
                                    const wing_rect_t *b)
{
  int32_t ax2;
  int32_t ay2;
  int32_t bx2;
  int32_t by2;

  if (a == NULL || b == NULL || a->w == 0 || a->h == 0 ||
      b->w == 0 || b->h == 0)
    {
      return false;
    }

  ax2 = (int32_t)a->x + a->w;
  ay2 = (int32_t)a->y + a->h;
  bx2 = (int32_t)b->x + b->w;
  by2 = (int32_t)b->y + b->h;

  return a->x < bx2 && ax2 > b->x && a->y < by2 && ay2 > b->y;
}

static bool wing_obj_rect_contains_point(const wing_rect_t *rect,
                                         wing_point_t point)
{
  int32_t x2;
  int32_t y2;

  if (rect == NULL || rect->w == 0 || rect->h == 0)
    {
      return false;
    }

  x2 = (int32_t)rect->x + rect->w;
  y2 = (int32_t)rect->y + rect->h;

  return point.x >= rect->x && point.x < x2 &&
         point.y >= rect->y && point.y < y2;
}

static void wing_obj_bind_gui(wing_obj_t *obj, wing_gui_t *gui)
{
  wing_obj_t *child;
  wing_gui_t *old_gui;

  if (obj == NULL)
    {
      return;
    }

  old_gui = obj->gui;
  obj->gui = gui;
  if (old_gui == NULL && gui != NULL)
    {
      (void)wing_obj_send_event(obj, WING_EVENT_CREATE, NULL, NULL);
    }

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      wing_obj_bind_gui(child, gui);
    }
}

static bool wing_obj_contains(const wing_obj_t *root,
                              const wing_obj_t *candidate)
{
  const wing_obj_t *child;

  if (root == NULL || candidate == NULL)
    {
      return false;
    }

  if (root == candidate)
    {
      return true;
    }

  for (child = root->first_child; child != NULL; child = child->next_sibling)
    {
      if (wing_obj_contains(child, candidate))
        {
          return true;
        }
    }

  return false;
}

static void wing_obj_clear_gui_refs(wing_gui_t *gui, wing_obj_t *root)
{
  if (gui == NULL || root == NULL)
    {
      return;
    }

  if (wing_obj_contains(root, gui->pressed_obj))
    {
      gui->pressed_obj = NULL;
    }

  if (wing_obj_contains(root, gui->captured_obj))
    {
      gui->captured_obj = NULL;
    }

  if (wing_obj_contains(root, gui->focused_obj))
    {
      gui->focused_obj = NULL;
    }

  if (wing_obj_contains(root, gui->hovered_obj))
    {
      gui->hovered_obj = NULL;
    }
}

static int wing_obj_send_delete_tree(wing_obj_t *obj)
{
  wing_obj_t *child;
  int ret;

  if (obj == NULL)
    {
      return -EINVAL;
    }

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      ret = wing_obj_send_delete_tree(child);
      if (ret < 0)
        {
          return ret;
        }
    }

  return wing_obj_send_event(obj, WING_EVENT_DELETE, NULL, NULL);
}

static void wing_obj_detach_from_parent(wing_obj_t *obj)
{
  wing_obj_t *parent;

  if (obj == NULL || obj->parent == NULL)
    {
      return;
    }

  parent = obj->parent;
  if (obj->prev_sibling != NULL)
    {
      obj->prev_sibling->next_sibling = obj->next_sibling;
    }
  else
    {
      parent->first_child = obj->next_sibling;
    }

  if (obj->next_sibling != NULL)
    {
      obj->next_sibling->prev_sibling = obj->prev_sibling;
    }
  else
    {
      parent->last_child = obj->prev_sibling;
    }

  obj->parent = NULL;
  obj->prev_sibling = NULL;
  obj->next_sibling = NULL;
}

static bool wing_obj_rect_equal(const wing_rect_t *a, const wing_rect_t *b)
{
  return a != NULL && b != NULL &&
         a->x == b->x &&
         a->y == b->y &&
         a->w == b->w &&
         a->h == b->h;
}

static int wing_obj_project_quad_from_bounds(
  const wing_rect_t *bounds, const wing_space_transform_t *transform,
  const wing_camera_t *camera, wing_quad2d_t *quad);

static bool wing_obj_project_bounds(const wing_rect_t *bounds,
                                    const wing_space_transform_t *transform,
                                    const wing_camera_t *camera,
                                    wing_rect_t *screen)
{
  if (bounds == NULL || transform == NULL || screen == NULL ||
      bounds->w == 0 || bounds->h == 0)
    {
      return false;
    }

  return wing_project_rect_bounds(camera, transform, bounds, screen) == 0;
}

static int wing_obj_project_quad_from_bounds(
  const wing_rect_t *bounds, const wing_space_transform_t *transform,
  const wing_camera_t *camera, wing_quad2d_t *quad)
{
  return wing_project_rect_quad(camera, transform, bounds, quad);
}

static void wing_obj_invalidate_subtree_screen_bounds(wing_obj_t *obj)
{
  wing_obj_t *child;
  wing_rect_t screen_bounds;

  if (obj == NULL)
    {
      return;
    }

  obj->flags |= WING_OBJ_FLAG_DIRTY;
  if (obj->gui != NULL &&
      wing_obj_get_screen_bounds(obj, &screen_bounds))
    {
      wing_gui_invalidate_rect(obj->gui, &screen_bounds);
    }

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      wing_obj_invalidate_subtree_screen_bounds(child);
    }
}

static bool wing_obj_find_min_child_z(wing_obj_t *obj, int16_t *z_index)
{
  wing_obj_t *child;
  int16_t best;
  bool found;

  if (obj == NULL || z_index == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (!found || child->z_index < best)
        {
          best = child->z_index;
          found = true;
        }
    }

  if (found)
    {
      *z_index = best;
    }

  return found;
}

static bool wing_obj_find_next_child_z(wing_obj_t *obj, int16_t current,
                                       int16_t *z_index)
{
  wing_obj_t *child;
  int16_t best;
  bool found;

  if (obj == NULL || z_index == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index > current &&
          (!found || child->z_index < best))
        {
          best = child->z_index;
          found = true;
        }
    }

  if (found)
    {
      *z_index = best;
    }

  return found;
}

static bool wing_obj_find_max_child_z(wing_obj_t *obj, int16_t *z_index)
{
  wing_obj_t *child;
  int16_t best;
  bool found;

  if (obj == NULL || z_index == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (!found || child->z_index > best)
        {
          best = child->z_index;
          found = true;
        }
    }

  if (found)
    {
      *z_index = best;
    }

  return found;
}

static bool wing_obj_find_prev_child_z(wing_obj_t *obj, int16_t current,
                                       int16_t *z_index)
{
  wing_obj_t *child;
  int16_t best;
  bool found;

  if (obj == NULL || z_index == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index < current &&
          (!found || child->z_index > best))
        {
          best = child->z_index;
          found = true;
        }
    }

  if (found)
    {
      *z_index = best;
    }

  return found;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_obj_init(wing_obj_t *obj, const wing_rect_t *bounds)
{
  if (obj == NULL)
    {
      return;
    }

  obj->gui = NULL;
  obj->parent = NULL;
  obj->first_child = NULL;
  obj->last_child = NULL;
  obj->prev_sibling = NULL;
  obj->next_sibling = NULL;

  if (bounds != NULL)
    {
      obj->bounds = *bounds;
    }
  else
    {
      obj->bounds.x = 0;
      obj->bounds.y = 0;
      obj->bounds.w = 0;
      obj->bounds.h = 0;
    }

  obj->flags = WING_OBJ_FLAG_VISIBLE | WING_OBJ_FLAG_ENABLED |
               WING_OBJ_FLAG_DIRTY;
  obj->state = WING_OBJ_STATE_DEFAULT;
  obj->z_index = 0;
  obj->opacity = 255;
  wing_space_transform_init(&obj->space_transform);
  obj->layout = WING_LAYOUT_FIXED;
  obj->padding = 0;
  obj->spacing = 0;
  obj->draw = NULL;
  obj->event = NULL;
  obj->screen_bounds = NULL;
  obj->contains_point = NULL;
  obj->user_data = NULL;
}

int wing_obj_add_child(wing_obj_t *parent, wing_obj_t *child)
{
  if (parent == NULL || child == NULL || child == parent ||
      child->parent != NULL)
    {
      return -EINVAL;
    }

  child->parent = parent;
  child->prev_sibling = parent->last_child;
  child->next_sibling = NULL;

  if (parent->last_child != NULL)
    {
      parent->last_child->next_sibling = child;
    }
  else
    {
      parent->first_child = child;
    }

  parent->last_child = child;
  wing_obj_bind_gui(child, parent->gui);
  wing_obj_invalidate(child);

  return 0;
}

int wing_obj_remove_child(wing_obj_t *child)
{
  wing_obj_t *parent;
  wing_gui_t *gui;

  if (child == NULL || child->parent == NULL)
    {
      return -EINVAL;
    }

  parent = child->parent;
  gui = child->gui;
  wing_obj_invalidate(child);
  wing_obj_detach_from_parent(child);
  wing_obj_clear_gui_refs(gui, child);
  wing_obj_bind_gui(child, NULL);
  wing_obj_invalidate(parent);

  return 0;
}

int wing_obj_destroy_tree(wing_obj_t *obj)
{
  wing_gui_t *gui;
  wing_obj_t *child;
  wing_obj_t *next;
  int ret;

  if (obj == NULL)
    {
      return -EINVAL;
    }

  gui = obj->gui;
  wing_obj_invalidate(obj);
  wing_obj_detach_from_parent(obj);
  wing_obj_clear_gui_refs(gui, obj);

  ret = wing_obj_send_delete_tree(obj);
  if (ret < 0)
    {
      return ret;
    }

  wing_obj_bind_gui(obj, NULL);

  child = obj->first_child;
  while (child != NULL)
    {
      next = child->next_sibling;
      child->parent = NULL;
      child->prev_sibling = NULL;
      child->next_sibling = NULL;
      child = next;
    }

  obj->first_child = NULL;
  obj->last_child = NULL;
  return 0;
}

void wing_obj_set_draw_cb(wing_obj_t *obj, wing_obj_draw_fn_t draw)
{
  if (obj != NULL)
    {
      obj->draw = draw;
      wing_obj_invalidate(obj);
    }
}

void wing_obj_set_event_cb(wing_obj_t *obj, wing_obj_event_fn_t event)
{
  if (obj != NULL)
    {
      obj->event = event;
    }
}

void wing_obj_set_geometry_cb(wing_obj_t *obj, wing_obj_draw_fn_t draw,
                              wing_obj_screen_bounds_fn_t bounds,
                              wing_obj_contains_point_fn_t contains)
{
  if (obj != NULL)
    {
      wing_rect_t old_screen_bounds;
      bool has_old_screen_bounds;

      has_old_screen_bounds = false;
      if (obj->gui != NULL)
        {
          has_old_screen_bounds =
            wing_obj_get_screen_bounds(obj, &old_screen_bounds);
        }

      obj->draw = draw;
      obj->screen_bounds = bounds;
      obj->contains_point = contains;
      obj->flags |= WING_OBJ_FLAG_DIRTY;

      if (obj->gui != NULL)
        {
          wing_rect_t new_screen_bounds;

          if (has_old_screen_bounds)
            {
              wing_gui_invalidate_rect(obj->gui, &old_screen_bounds);
            }

          if (wing_obj_get_screen_bounds(obj, &new_screen_bounds))
            {
              wing_gui_invalidate_rect(obj->gui, &new_screen_bounds);
            }
        }
    }
}

void wing_obj_set_screen_bounds_cb(wing_obj_t *obj,
                                   wing_obj_screen_bounds_fn_t bounds)
{
  if (obj != NULL)
    {
      wing_rect_t old_screen_bounds;
      bool has_old_screen_bounds;

      has_old_screen_bounds = false;
      if (obj->gui != NULL)
        {
          has_old_screen_bounds =
            wing_obj_get_screen_bounds(obj, &old_screen_bounds);
        }

      obj->screen_bounds = bounds;
      obj->flags |= WING_OBJ_FLAG_DIRTY;

      if (obj->gui != NULL)
        {
          wing_rect_t new_screen_bounds;

          if (has_old_screen_bounds)
            {
              wing_gui_invalidate_rect(obj->gui, &old_screen_bounds);
            }

          if (wing_obj_get_screen_bounds(obj, &new_screen_bounds))
            {
              wing_gui_invalidate_rect(obj->gui, &new_screen_bounds);
            }
        }
    }
}

void wing_obj_set_contains_point_cb(wing_obj_t *obj,
                                    wing_obj_contains_point_fn_t contains)
{
  if (obj != NULL)
    {
      obj->contains_point = contains;
    }
}

void wing_obj_set_user_data(wing_obj_t *obj, void *user_data)
{
  if (obj != NULL)
    {
      wing_rect_t old_screen_bounds;
      bool has_old_screen_bounds;

      has_old_screen_bounds = false;
      if (obj->gui != NULL)
        {
          has_old_screen_bounds =
            wing_obj_get_screen_bounds(obj, &old_screen_bounds);
        }

      obj->user_data = user_data;
      obj->flags |= WING_OBJ_FLAG_DIRTY;

      if (obj->gui != NULL)
        {
          wing_rect_t new_screen_bounds;

          if (has_old_screen_bounds)
            {
              wing_gui_invalidate_rect(obj->gui, &old_screen_bounds);
            }

          if (wing_obj_get_screen_bounds(obj, &new_screen_bounds))
            {
              wing_gui_invalidate_rect(obj->gui, &new_screen_bounds);
            }
        }
    }
}

void *wing_obj_get_user_data(const wing_obj_t *obj)
{
  return obj != NULL ? obj->user_data : NULL;
}

const wing_rect_t *wing_obj_get_bounds(const wing_obj_t *obj)
{
  return obj != NULL ? &obj->bounds : NULL;
}

void wing_obj_set_z_index(wing_obj_t *obj, int16_t z_index)
{
  wing_obj_t *invalidate_root;

  if (obj == NULL || obj->z_index == z_index)
    {
      return;
    }

  invalidate_root = obj->parent != NULL ? obj->parent : obj;

  if (obj->gui != NULL)
    {
      wing_obj_invalidate_subtree_screen_bounds(invalidate_root);
    }

  obj->z_index = z_index;

  if (obj->gui != NULL)
    {
      wing_obj_invalidate_subtree_screen_bounds(invalidate_root);
    }
  else
    {
      obj->flags |= WING_OBJ_FLAG_DIRTY;
    }
}

int16_t wing_obj_get_z_index(const wing_obj_t *obj)
{
  return obj == NULL ? 0 : obj->z_index;
}

void wing_obj_set_opacity(wing_obj_t *obj, uint8_t opacity)
{
  if (obj == NULL || obj->opacity == opacity)
    {
      return;
    }

  wing_obj_invalidate_subtree_screen_bounds(obj);
  obj->opacity = opacity;
  wing_obj_invalidate_subtree_screen_bounds(obj);
}

uint8_t wing_obj_get_opacity(const wing_obj_t *obj)
{
  return obj == NULL ? 255 : obj->opacity;
}

uint8_t wing_obj_get_effective_opacity(const wing_obj_t *obj)
{
  uint16_t alpha;

  if (obj == NULL)
    {
      return 255;
    }

  if (obj->parent == NULL)
    {
      return obj->opacity;
    }

  alpha = (uint16_t)wing_obj_get_effective_opacity(obj->parent) *
          obj->opacity + 127;
  return (uint8_t)(alpha / 255);
}

int wing_obj_project_quad(const wing_obj_t *obj, wing_projected_quad_t *quad)
{
  wing_space_transform_t world_transform;

  if (obj == NULL || quad == NULL)
    {
      return -EINVAL;
    }

  if (wing_obj_get_world_space_transform(obj, &world_transform) < 0)
    {
      return -EINVAL;
    }

  return wing_project_rect_projected_quad(wing_gui_get_camera(obj->gui),
                                          &world_transform, &obj->bounds,
                                          quad);
}

int wing_obj_get_projected_depth(const wing_obj_t *obj,
                                 int32_t *average_depth)
{
  wing_projected_quad_t projected;

  if (average_depth == NULL)
    {
      return -EINVAL;
    }

  if (wing_obj_project_quad(obj, &projected) < 0)
    {
      return -EINVAL;
    }

  return wing_projected_quad_average_depth(&projected, average_depth);
}

static int32_t wing_obj_get_sort_depth(const wing_obj_t *obj)
{
  int32_t depth;

  if (wing_obj_get_projected_depth(obj, &depth) < 0)
    {
      return 0;
    }

  return depth;
}

static bool wing_obj_find_max_child_depth(wing_obj_t *obj, int16_t z_index,
                                          int32_t *depth)
{
  wing_obj_t *child;
  int32_t best;
  int32_t child_depth;
  bool found;

  if (obj == NULL || depth == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index != z_index)
        {
          continue;
        }

      child_depth = wing_obj_get_sort_depth(child);
      if (!found || child_depth > best)
        {
          best = child_depth;
          found = true;
        }
    }

  if (found)
    {
      *depth = best;
    }

  return found;
}

static bool wing_obj_find_next_lower_child_depth(wing_obj_t *obj,
                                                 int16_t z_index,
                                                 int32_t current,
                                                 int32_t *depth)
{
  wing_obj_t *child;
  int32_t best;
  int32_t child_depth;
  bool found;

  if (obj == NULL || depth == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index != z_index)
        {
          continue;
        }

      child_depth = wing_obj_get_sort_depth(child);
      if (child_depth < current && (!found || child_depth > best))
        {
          best = child_depth;
          found = true;
        }
    }

  if (found)
    {
      *depth = best;
    }

  return found;
}

static bool wing_obj_find_min_child_depth(wing_obj_t *obj, int16_t z_index,
                                          int32_t *depth)
{
  wing_obj_t *child;
  int32_t best;
  int32_t child_depth;
  bool found;

  if (obj == NULL || depth == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index != z_index)
        {
          continue;
        }

      child_depth = wing_obj_get_sort_depth(child);
      if (!found || child_depth < best)
        {
          best = child_depth;
          found = true;
        }
    }

  if (found)
    {
      *depth = best;
    }

  return found;
}

static bool wing_obj_find_next_higher_child_depth(wing_obj_t *obj,
                                                  int16_t z_index,
                                                  int32_t current,
                                                  int32_t *depth)
{
  wing_obj_t *child;
  int32_t best;
  int32_t child_depth;
  bool found;

  if (obj == NULL || depth == NULL)
    {
      return false;
    }

  found = false;
  best = 0;

  for (child = obj->first_child; child != NULL; child = child->next_sibling)
    {
      if (child->z_index != z_index)
        {
          continue;
        }

      child_depth = wing_obj_get_sort_depth(child);
      if (child_depth > current && (!found || child_depth < best))
        {
          best = child_depth;
          found = true;
        }
    }

  if (found)
    {
      *depth = best;
    }

  return found;
}

int wing_obj_compare_space_order(const wing_obj_t *a, const wing_obj_t *b)
{
  int32_t a_depth;
  int32_t b_depth;

  if (a == b)
    {
      return 0;
    }

  if (a == NULL)
    {
      return -1;
    }

  if (b == NULL)
    {
      return 1;
    }

  if (a->z_index != b->z_index)
    {
      return a->z_index < b->z_index ? -1 : 1;
    }

  a_depth = wing_obj_get_sort_depth(a);
  b_depth = wing_obj_get_sort_depth(b);
  if (a_depth != b_depth)
    {
      return a_depth > b_depth ? -1 : 1;
    }

  return 0;
}

int wing_obj_project_quad2d(const wing_obj_t *obj, wing_quad2d_t *quad)
{
  wing_projected_quad_t projected;
  uint8_t i;

  if (quad == NULL)
    {
      return -EINVAL;
    }

  if (wing_obj_project_quad(obj, &projected) < 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < 4; i++)
    {
      quad->points[i] = projected.vertices[i].screen;
    }

  return 0;
}

bool wing_obj_get_screen_bounds(const wing_obj_t *obj, wing_rect_t *bounds)
{
  wing_space_transform_t world_transform;

  if (obj == NULL || bounds == NULL)
    {
      return false;
    }

  if (obj->screen_bounds != NULL &&
      obj->screen_bounds(obj, bounds))
    {
      return true;
    }

  if (wing_obj_get_world_space_transform(obj, &world_transform) < 0)
    {
      return false;
    }

  return wing_obj_project_bounds(&obj->bounds, &world_transform,
                                 wing_gui_get_camera(obj->gui), bounds);
}

bool wing_obj_contains_point(const wing_obj_t *obj, wing_point_t point)
{
  wing_rect_t screen_bounds;
  wing_space_transform_t world_transform;
  wing_quad2d_t quad;

  if (obj == NULL ||
      !wing_obj_get_screen_bounds(obj, &screen_bounds) ||
      !wing_obj_rect_contains_point(&screen_bounds, point))
    {
      return false;
    }

  if (wing_obj_get_world_space_transform(obj, &world_transform) < 0)
    {
      return false;
    }

  if (obj->contains_point != NULL)
    {
      return obj->contains_point(obj, point);
    }

  if (wing_space_transform_is_identity(&world_transform))
    {
      return true;
    }

  if (wing_obj_project_quad_from_bounds(&obj->bounds, &world_transform,
                                        wing_gui_get_camera(obj->gui),
                                        &quad) < 0)
    {
      return false;
    }

  return wing_quad2d_contains_point(&quad, point);
}

int wing_obj_set_bounds(wing_obj_t *obj, const wing_rect_t *bounds)
{
  wing_bounds_event_t payload;
  wing_rect_t old_bounds;
  wing_rect_t old_screen_bounds;
  wing_rect_t new_screen_bounds;
  bool has_old_screen_bounds;

  if (obj == NULL || bounds == NULL)
    {
      return -EINVAL;
    }

  old_bounds = obj->bounds;
  if (wing_obj_rect_equal(&old_bounds, bounds))
    {
      return 0;
    }

  has_old_screen_bounds = false;
  if (obj->gui != NULL)
    {
      has_old_screen_bounds =
        wing_obj_get_screen_bounds(obj, &old_screen_bounds);
    }

  obj->bounds = *bounds;

  if (obj->gui != NULL)
    {
      if (has_old_screen_bounds)
        {
          wing_gui_invalidate_rect(obj->gui, &old_screen_bounds);
        }

      if (wing_obj_get_screen_bounds(obj, &new_screen_bounds))
        {
          wing_gui_invalidate_rect(obj->gui, &new_screen_bounds);
        }
    }
  else
    {
      obj->flags |= WING_OBJ_FLAG_DIRTY;
    }

  payload.old_bounds = old_bounds;
  payload.bounds = obj->bounds;
  (void)wing_obj_send_event(obj, WING_EVENT_BOUNDS_CHANGED, NULL,
                            &payload);
  return 0;
}

void wing_obj_set_space_transform(wing_obj_t *obj,
                                  const wing_space_transform_t *transform)
{
  wing_space_transform_event_t payload;
  wing_space_transform_t old_transform;

  if (obj == NULL || transform == NULL)
    {
      return;
    }

  old_transform = obj->space_transform;
  if (wing_space_transform_equal(&old_transform, transform))
    {
      return;
    }

  if (obj->gui != NULL)
    {
      wing_obj_invalidate_subtree_screen_bounds(obj);
    }

  obj->space_transform = *transform;

  if (obj->gui != NULL)
    {
      wing_obj_invalidate_subtree_screen_bounds(obj);
    }
  else
    {
      obj->flags |= WING_OBJ_FLAG_DIRTY;
    }

  payload.old_transform = old_transform;
  payload.transform = obj->space_transform;
  (void)wing_obj_send_event(obj, WING_EVENT_SPACE_TRANSFORM_CHANGED, NULL,
                            &payload);
}

const wing_space_transform_t *wing_obj_get_space_transform(
  const wing_obj_t *obj)
{
  return obj == NULL ? NULL : &obj->space_transform;
}

int wing_obj_get_world_space_transform(const wing_obj_t *obj,
                                       wing_space_transform_t *transform)
{
  wing_space_transform_t parent_transform;

  if (obj == NULL || transform == NULL)
    {
      return -EINVAL;
    }

  if (obj->parent == NULL)
    {
      *transform = obj->space_transform;
      return 0;
    }

  if (wing_obj_get_world_space_transform(obj->parent,
                                         &parent_transform) < 0)
    {
      return -EINVAL;
    }

  return wing_space_transform_compose(&parent_transform, &obj->space_transform,
                                      transform);
}

bool wing_obj_space_transform_is_identity(const wing_obj_t *obj)
{
  if (obj == NULL)
    {
      return false;
    }

  return wing_space_transform_is_identity(&obj->space_transform);
}

bool wing_obj_is_default_2d(const wing_obj_t *obj)
{
  if (obj == NULL)
    {
      return false;
    }

  return wing_space_transform_is_default_2d(&obj->space_transform);
}

void wing_obj_reset_space_transform(wing_obj_t *obj)
{
  wing_space_transform_t transform;

  if (obj == NULL)
    {
      return;
    }

  wing_space_transform_init(&transform);
  wing_obj_set_space_transform(obj, &transform);
}

static void wing_obj_emit_state_changed(wing_obj_t *obj,
                                        uint16_t old_state,
                                        uint16_t state)
{
  wing_state_event_t payload;

  if (obj == NULL || old_state == state)
    {
      return;
    }

  payload.old_state = old_state;
  payload.state = state;
  payload.changed = (uint16_t)(old_state ^ state);
  (void)wing_obj_send_event(obj, WING_EVENT_STATE_CHANGED, NULL,
                            &payload);
}

void wing_obj_set_state(wing_obj_t *obj, uint16_t state)
{
  if (obj != NULL && obj->state != state)
    {
      uint16_t old_state;

      old_state = obj->state;
      obj->state = state;
      wing_obj_invalidate(obj);
      wing_obj_emit_state_changed(obj, old_state, state);
    }
}

static void wing_obj_set_state_bit(wing_obj_t *obj, uint16_t state,
                                   bool enabled)
{
  uint16_t next;

  if (obj == NULL)
    {
      return;
    }

  next = obj->state;
  if (enabled)
    {
      next |= state;
    }
  else
    {
      next &= (uint16_t)~state;
    }

  wing_obj_set_state(obj, next);
}

uint16_t wing_obj_get_state(const wing_obj_t *obj)
{
  return obj != NULL ? obj->state : WING_OBJ_STATE_DEFAULT;
}

void wing_obj_set_flags(wing_obj_t *obj, uint16_t flags)
{
  uint16_t old_state;
  uint16_t state;

  if (obj != NULL)
    {
      old_state = obj->state;
      state = obj->state;
      if ((flags & WING_OBJ_FLAG_ENABLED) != 0)
        {
          state &= (uint16_t)~WING_OBJ_STATE_DISABLED;
        }
      else
        {
          state |= WING_OBJ_STATE_DISABLED;
        }

      if (obj->flags != flags || obj->state != state)
        {
          obj->flags = flags;
          obj->state = state;
          wing_obj_invalidate(obj);
          wing_obj_emit_state_changed(obj, old_state, state);
        }
    }
}

uint16_t wing_obj_get_flags(const wing_obj_t *obj)
{
  return obj != NULL ? obj->flags : 0;
}

void wing_obj_set_enabled(wing_obj_t *obj, bool enabled)
{
  uint16_t flags;

  if (obj == NULL)
    {
      return;
    }

  flags = wing_obj_get_flags(obj);
  if (enabled)
    {
      flags |= WING_OBJ_FLAG_ENABLED;
    }
  else
    {
      flags &= (uint16_t)~WING_OBJ_FLAG_ENABLED;
    }

  wing_obj_set_flags(obj, flags);
}

bool wing_obj_is_enabled(const wing_obj_t *obj)
{
  return obj != NULL && (obj->flags & WING_OBJ_FLAG_ENABLED) != 0 &&
         (obj->state & WING_OBJ_STATE_DISABLED) == 0;
}

void wing_obj_set_clip_children(wing_obj_t *obj, bool clip_children)
{
  uint16_t flags;

  if (obj == NULL)
    {
      return;
    }

  flags = wing_obj_get_flags(obj);
  if (clip_children)
    {
      flags |= WING_OBJ_FLAG_CLIP_CHILDREN;
    }
  else
    {
      flags &= (uint16_t)~WING_OBJ_FLAG_CLIP_CHILDREN;
    }

  wing_obj_set_flags(obj, flags);
}

bool wing_obj_get_clip_children(const wing_obj_t *obj)
{
  return obj != NULL &&
         (obj->flags & WING_OBJ_FLAG_CLIP_CHILDREN) != 0;
}

void wing_obj_set_selected(wing_obj_t *obj, bool selected)
{
  wing_obj_set_state_bit(obj, WING_OBJ_STATE_SELECTED, selected);
}

bool wing_obj_is_selected(const wing_obj_t *obj)
{
  return obj != NULL &&
         (obj->state & WING_OBJ_STATE_SELECTED) != 0;
}

void wing_obj_set_active(wing_obj_t *obj, bool active)
{
  wing_obj_set_state_bit(obj, WING_OBJ_STATE_ACTIVE, active);
}

bool wing_obj_is_active(const wing_obj_t *obj)
{
  return obj != NULL &&
         (obj->state & WING_OBJ_STATE_ACTIVE) != 0;
}

void wing_obj_set_visible(wing_obj_t *obj, bool visible)
{
  uint16_t flags;

  if (obj == NULL)
    {
      return;
    }

  flags = wing_obj_get_flags(obj);
  if (visible)
    {
      flags |= WING_OBJ_FLAG_VISIBLE;
    }
  else
    {
      flags &= (uint16_t)~WING_OBJ_FLAG_VISIBLE;
      wing_obj_clear_gui_refs(obj->gui, obj);
    }

  wing_obj_set_flags(obj, flags);
}

bool wing_obj_is_visible(const wing_obj_t *obj)
{
  return obj != NULL && (obj->flags & WING_OBJ_FLAG_VISIBLE) != 0;
}

void wing_obj_invalidate(wing_obj_t *obj)
{
  wing_rect_t screen_bounds;

  if (obj == NULL)
    {
      return;
    }

  obj->flags |= WING_OBJ_FLAG_DIRTY;

  if (obj->gui != NULL)
    {
      if (wing_obj_get_screen_bounds(obj, &screen_bounds))
        {
          wing_gui_invalidate_rect(obj->gui, &screen_bounds);
        }
    }
}

int wing_obj_send_event(wing_obj_t *obj, enum wing_event_code_e code,
                        wing_context_t *ctx, void *data)
{
  wing_event_t event;

  if (obj == NULL)
    {
      return -EINVAL;
    }

  if (obj->event == NULL)
    {
      return 0;
    }

  event.code = code;
  event.gui = obj->gui;
  event.target = obj;
  event.context = ctx;
  event.data = data;
  event.stopped = false;

  return obj->event(obj, &event);
}

int wing_obj_bubble_event(wing_obj_t *target, enum wing_event_code_e code,
                          wing_context_t *ctx, void *data)
{
  wing_event_t event;
  wing_obj_t *current;
  int ret;

  if (target == NULL)
    {
      return -EINVAL;
    }

  event.code = code;
  event.gui = target->gui;
  event.target = target;
  event.context = ctx;
  event.data = data;
  event.stopped = false;

  for (current = target; current != NULL && !event.stopped;
       current = current->parent)
    {
      if (current->event == NULL)
        {
          continue;
        }

      ret = current->event(current, &event);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

int wing_obj_draw_tree(wing_obj_t *root, wing_context_t *ctx,
                       const wing_rect_t *dirty)
{
  wing_obj_t *child;
  wing_rect_t screen_bounds;
  int32_t depth;
  int16_t z_index;
  bool clip_children;
  int ret;

  if (root == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  if ((root->flags & WING_OBJ_FLAG_VISIBLE) == 0)
    {
      return 0;
    }

  if (dirty != NULL &&
      (!wing_obj_get_screen_bounds(root, &screen_bounds) ||
       !wing_obj_rect_intersect(&screen_bounds, dirty)))
    {
      return 0;
    }

  ret = wing_obj_send_event(root, WING_EVENT_DRAW, ctx, NULL);
  if (ret < 0)
    {
      return ret;
    }

  if (root->draw != NULL)
    {
      ret = root->draw(root, ctx);
      if (ret < 0)
        {
          return ret;
        }
    }

  root->flags &= ~WING_OBJ_FLAG_DIRTY;

  if (!wing_obj_find_min_child_z(root, &z_index))
    {
      return 0;
    }

  clip_children = wing_obj_get_clip_children(root);
  if (clip_children)
    {
      if (!wing_obj_get_screen_bounds(root, &screen_bounds))
        {
          return 0;
        }

      ret = wing_gui_set_clip(ctx, &screen_bounds);
      if (ret < 0)
        {
          return ret;
        }
    }

  do
    {
      if (!wing_obj_find_max_child_depth(root, z_index, &depth))
        {
          continue;
        }

      do
        {
          for (child = root->first_child; child != NULL;
               child = child->next_sibling)
            {
              if (child->z_index != z_index ||
                  wing_obj_get_sort_depth(child) != depth)
                {
                  continue;
                }

              ret = wing_obj_draw_tree(child, ctx, dirty);
              if (ret < 0)
                {
                  return ret;
                }
            }
        }
      while (wing_obj_find_next_lower_child_depth(root, z_index, depth,
                                                  &depth));
    }
  while (wing_obj_find_next_child_z(root, z_index, &z_index));

  if (clip_children)
    {
      ret = wing_gui_reset_clip(ctx);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

wing_obj_t *wing_obj_hit_test(wing_obj_t *root, wing_point_t point)
{
  wing_obj_t *child;
  wing_obj_t *hit;
  int32_t depth;
  int16_t z_index;

  if (root == NULL || (root->flags & WING_OBJ_FLAG_VISIBLE) == 0 ||
      (root->flags & WING_OBJ_FLAG_ENABLED) == 0 ||
      !wing_obj_contains_point(root, point))
    {
      return NULL;
    }

  if (!wing_obj_find_max_child_z(root, &z_index))
    {
      return root;
    }

  do
    {
      if (!wing_obj_find_min_child_depth(root, z_index, &depth))
        {
          continue;
        }

      do
        {
          for (child = root->last_child; child != NULL;
               child = child->prev_sibling)
            {
              if (child->z_index != z_index ||
                  wing_obj_get_sort_depth(child) != depth)
                {
                  continue;
                }

              hit = wing_obj_hit_test(child, point);
              if (hit != NULL)
                {
                  return hit;
                }
            }
        }
      while (wing_obj_find_next_higher_child_depth(root, z_index, depth,
                                                   &depth));
    }
  while (wing_obj_find_prev_child_z(root, z_index, &z_index));

  return root;
}

int wing_gui_set_root(wing_gui_t *gui, wing_obj_t *root)
{
  if (gui == NULL)
    {
      return -EINVAL;
    }

  if (gui->root != NULL)
    {
      wing_obj_bind_gui(gui->root, NULL);
    }

  gui->root = root;
  wing_obj_bind_gui(root, gui);
  wing_gui_invalidate(gui);

  return 0;
}
