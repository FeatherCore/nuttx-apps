/****************************************************************************
 * apps/graphics/wing/src/core/wing_layout.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>

#include <wing/core/wing_layout.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_obj_set_layout(wing_obj_t *obj, enum wing_layout_type_e layout,
                         uint8_t padding, uint8_t spacing)
{
  if (obj != NULL)
    {
      obj->layout = layout;
      obj->padding = padding;
      obj->spacing = spacing;
      wing_obj_invalidate(obj);
    }
}

int wing_obj_layout_tree(wing_obj_t *root)
{
  wing_obj_t *child;
  wing_rect_t child_bounds;
  int32_t cursor_x;
  int32_t cursor_y;
  int ret;

  if (root == NULL)
    {
      return -EINVAL;
    }

  cursor_x = (int32_t)root->bounds.x + root->padding;
  cursor_y = (int32_t)root->bounds.y + root->padding;

  if (root->layout != WING_LAYOUT_FIXED)
    {
      for (child = root->first_child; child != NULL;
           child = child->next_sibling)
        {
          if ((child->flags & WING_OBJ_FLAG_VISIBLE) == 0)
            {
              continue;
            }

          child_bounds = child->bounds;
          if (root->layout == WING_LAYOUT_CENTER)
            {
              child_bounds.x =
                (int16_t)(root->bounds.x +
                          ((int32_t)root->bounds.w - child_bounds.w) / 2);
              child_bounds.y =
                (int16_t)(root->bounds.y +
                          ((int32_t)root->bounds.h - child_bounds.h) / 2);
            }
          else if (root->layout == WING_LAYOUT_FILL)
            {
              child_bounds.x = (int16_t)cursor_x;
              child_bounds.y = (int16_t)cursor_y;
              child_bounds.w =
                root->bounds.w > (uint16_t)(root->padding * 2) ?
                (uint16_t)(root->bounds.w - root->padding * 2) : 0;
              child_bounds.h =
                root->bounds.h > (uint16_t)(root->padding * 2) ?
                (uint16_t)(root->bounds.h - root->padding * 2) : 0;
            }
          else
            {
              child_bounds.x = (int16_t)cursor_x;
              child_bounds.y = (int16_t)cursor_y;
            }

          if (root->layout == WING_LAYOUT_STACK_VERTICAL)
            {
              cursor_y += child_bounds.h + root->spacing;
            }
          else if (root->layout == WING_LAYOUT_STACK_HORIZONTAL)
            {
              cursor_x += child_bounds.w + root->spacing;
            }

          if (child_bounds.x != child->bounds.x ||
              child_bounds.y != child->bounds.y ||
              child_bounds.w != child->bounds.w ||
              child_bounds.h != child->bounds.h)
            {
              ret = wing_obj_set_bounds(child, &child_bounds);
              if (ret < 0)
                {
                  return ret;
                }
            }
        }
    }

  for (child = root->first_child; child != NULL; child = child->next_sibling)
    {
      ret = wing_obj_layout_tree(child);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}
