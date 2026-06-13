/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_panel.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_panel_init(wing_panel_t *panel, const wing_rect_t *bounds,
                     const wing_box_style_t *style)
{
  if (panel == NULL)
    {
      return;
    }

  wing_box_init(&panel->box, bounds, style);
}

wing_obj_t *wing_panel_obj(wing_panel_t *panel)
{
  return panel != NULL ? wing_box_obj(&panel->box) : NULL;
}

wing_box_t *wing_panel_box(wing_panel_t *panel)
{
  return panel != NULL ? &panel->box : NULL;
}

void wing_panel_set_style(wing_panel_t *panel,
                          const wing_box_style_t *style)
{
  if (panel == NULL)
    {
      return;
    }

  wing_box_set_style(&panel->box, style);
}

void wing_panel_set_state_style(wing_panel_t *panel, uint16_t state,
                                const wing_box_style_t *style)
{
  if (panel == NULL)
    {
      return;
    }

  wing_box_set_state_style(&panel->box, state, style);
}

void wing_panel_set_layout(wing_panel_t *panel,
                           enum wing_layout_type_e layout,
                           uint8_t padding, uint8_t spacing)
{
  if (panel == NULL)
    {
      return;
    }

  wing_obj_set_layout(wing_panel_obj(panel), layout, padding, spacing);
}
