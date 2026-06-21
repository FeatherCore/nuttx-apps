/****************************************************************************
 * apps/graphics/wing/src/core/wing_focus.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <wing/core/wing_focus.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool wing_gui_obj_is_focusable(const wing_obj_t *obj)
{
  uint16_t required;

  required = WING_OBJ_FLAG_VISIBLE | WING_OBJ_FLAG_ENABLED |
             WING_OBJ_FLAG_FOCUSABLE;

  return obj != NULL && (obj->flags & required) == required;
}

static wing_obj_t *wing_gui_first_focusable(wing_obj_t *root)
{
  wing_obj_t *child;
  wing_obj_t *found;

  if (root == NULL)
    {
      return NULL;
    }

  if (wing_gui_obj_is_focusable(root))
    {
      return root;
    }

  for (child = root->first_child; child != NULL; child = child->next_sibling)
    {
      found = wing_gui_first_focusable(child);
      if (found != NULL)
        {
          return found;
        }
    }

  return NULL;
}

static wing_obj_t *wing_gui_last_focusable(wing_obj_t *root)
{
  wing_obj_t *child;
  wing_obj_t *found;

  if (root == NULL)
    {
      return NULL;
    }

  for (child = root->last_child; child != NULL; child = child->prev_sibling)
    {
      found = wing_gui_last_focusable(child);
      if (found != NULL)
        {
          return found;
        }
    }

  return wing_gui_obj_is_focusable(root) ? root : NULL;
}

static wing_obj_t *wing_gui_next_focusable(wing_obj_t *root,
                                           const wing_obj_t *current,
                                           bool *seen_current)
{
  wing_obj_t *child;
  wing_obj_t *found;

  if (root == NULL || seen_current == NULL)
    {
      return NULL;
    }

  if (wing_gui_obj_is_focusable(root))
    {
      if (*seen_current)
        {
          return root;
        }

      if (root == current)
        {
          *seen_current = true;
        }
    }

  for (child = root->first_child; child != NULL; child = child->next_sibling)
    {
      found = wing_gui_next_focusable(child, current, seen_current);
      if (found != NULL)
        {
          return found;
        }
    }

  return NULL;
}

static wing_obj_t *wing_gui_prev_focusable(wing_obj_t *root,
                                           const wing_obj_t *current,
                                           wing_obj_t **previous)
{
  wing_obj_t *child;
  wing_obj_t *found;

  if (root == NULL || previous == NULL)
    {
      return NULL;
    }

  if (wing_gui_obj_is_focusable(root))
    {
      if (root == current)
        {
          return *previous;
        }

      *previous = root;
    }

  for (child = root->first_child; child != NULL; child = child->next_sibling)
    {
      found = wing_gui_prev_focusable(child, current, previous);
      if (found != NULL)
        {
          return found;
        }
    }

  return NULL;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int wing_gui_set_focus(wing_gui_t *gui, wing_obj_t *obj)
{
  wing_obj_t *old_focus;
  uint16_t state;
  int ret;

  if (gui == NULL || (obj != NULL && obj->gui != gui))
    {
      return -EINVAL;
    }

  if (gui->focused_obj == obj)
    {
      return 0;
    }

  old_focus = gui->focused_obj;
  gui->focused_obj = obj;

  if (old_focus != NULL)
    {
      state = wing_obj_get_state(old_focus);
      wing_obj_set_state(old_focus,
                         (uint16_t)(state & ~WING_OBJ_STATE_FOCUSED));

      ret = wing_gui_post_event(gui, old_focus, WING_EVENT_FOCUS_LOST,
                                NULL, NULL);
      if (ret < 0)
        {
          return ret;
        }
    }

  if (obj != NULL)
    {
      state = wing_obj_get_state(obj);
      wing_obj_set_state(obj, (uint16_t)(state | WING_OBJ_STATE_FOCUSED));

      ret = wing_gui_post_event(gui, obj, WING_EVENT_FOCUS_GAINED,
                                NULL, NULL);
      if (ret < 0)
        {
          return ret;
        }
    }

  return 0;
}

int wing_gui_focus_next(wing_gui_t *gui, bool reverse)
{
  wing_obj_t *next;
  wing_obj_t *previous;
  bool seen_current;
  int ret;

  if (gui == NULL || gui->root == NULL)
    {
      return -EINVAL;
    }

  if (gui->focused_obj == NULL)
    {
      next = reverse ? wing_gui_last_focusable(gui->root) :
                       wing_gui_first_focusable(gui->root);
    }
  else if (reverse)
    {
      previous = NULL;
      next = wing_gui_prev_focusable(gui->root, gui->focused_obj, &previous);
      if (next == NULL)
        {
          next = wing_gui_last_focusable(gui->root);
        }
    }
  else
    {
      seen_current = false;
      next = wing_gui_next_focusable(gui->root, gui->focused_obj,
                                     &seen_current);
      if (next == NULL)
        {
          next = wing_gui_first_focusable(gui->root);
        }
    }

  ret = wing_gui_set_focus(gui, next);
  if (ret < 0)
    {
      return ret;
    }

  return next != NULL ? 1 : 0;
}
