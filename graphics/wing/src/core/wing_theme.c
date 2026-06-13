/****************************************************************************
 * apps/graphics/wing/src/core/wing_theme.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/core/wing_theme.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static fr_color_t wing_color_to_fr(wing_color_t color)
{
  return fr_color_rgba(color.r, color.g, color.b, color.a);
}

static void wing_theme_make_fill(wing_box_style_t *style, wing_color_t color)
{
  wing_box_style_init(style);
  style->fill = color;
  style->has_fill = true;
}

static void wing_theme_make_clear(wing_box_style_t *style, wing_color_t color)
{
  wing_box_style_init(style);
  style->fill = color;
  style->clear = true;
  style->has_fill = true;
}

static void wing_theme_make_panel(wing_box_style_t *style, wing_color_t fill,
                                  wing_color_t stroke,
                                  uint16_t stroke_width)
{
  wing_box_style_init(style);
  style->fill = fill;
  style->stroke = stroke;
  style->stroke_width = stroke_width;
  style->has_fill = true;
  style->has_stroke = true;
}

static void wing_theme_make_stroke(wing_box_style_t *style,
                                   wing_color_t stroke,
                                   uint16_t stroke_width)
{
  wing_box_style_init(style);
  style->stroke = stroke;
  style->stroke_width = stroke_width;
  style->has_stroke = true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

wing_color_t wing_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  wing_color_t color;

  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;

  return color;
}

uint32_t wing_color_pack_rgba8888(wing_color_t color)
{
  return fr_color_pack_rgba8888(wing_color_to_fr(color));
}

void wing_theme_init_default(wing_theme_t *theme)
{
  wing_color_t background;
  wing_color_t surface;
  wing_color_t surface_alt;
  wing_color_t primary;
  wing_color_t primary_alt;
  wing_color_t accent;
  wing_color_t text;
  wing_color_t border;
  wing_color_t focus;
  wing_color_t success;
  wing_color_t disabled;

  if (theme == NULL)
    {
      return;
    }

  background = wing_color_rgba(238, 241, 246, 255);
  surface = wing_color_rgba(255, 255, 255, 255);
  surface_alt = wing_color_rgba(226, 232, 240, 255);
  primary = wing_color_rgba(31, 96, 196, 255);
  primary_alt = wing_color_rgba(60, 131, 246, 255);
  accent = wing_color_rgba(244, 121, 32, 255);
  text = wing_color_rgba(31, 41, 55, 255);
  border = wing_color_rgba(148, 163, 184, 255);
  focus = wing_color_rgba(20, 184, 166, 255);
  success = wing_color_rgba(22, 163, 74, 255);
  disabled = wing_color_rgba(203, 213, 225, 255);

  wing_theme_make_clear(&theme->root, background);
  wing_theme_make_panel(&theme->header, primary, primary_alt, 2);
  wing_theme_make_panel(&theme->panel, surface, border, 1);
  wing_theme_make_panel(&theme->button, primary, primary_alt, 2);
  wing_theme_make_panel(&theme->button_focused, primary_alt, focus, 2);
  wing_theme_make_panel(&theme->button_pressed, accent, primary, 2);
  wing_theme_make_stroke(&theme->line_primary, primary, 2);
  wing_theme_make_stroke(&theme->line_secondary, border, 1);
  wing_theme_make_panel(&theme->progress_frame, surface_alt, border, 1);
  wing_theme_make_fill(&theme->progress_fill, success);
  wing_theme_make_stroke(&theme->slider_track, surface_alt, 4);
  wing_theme_make_stroke(&theme->slider_fill, primary, 4);
  wing_theme_make_panel(&theme->slider_knob, surface, primary, 2);
  wing_theme_make_stroke(&theme->slider_focused, focus, 2);
  wing_theme_make_fill(&theme->scrollbar_track, surface_alt);
  wing_theme_make_fill(&theme->scrollbar_thumb, primary);
  wing_theme_make_stroke(&theme->scrollbar_focused, focus, 1);
  wing_theme_make_fill(&theme->switch_off, disabled);
  wing_theme_make_fill(&theme->switch_on, success);
  wing_theme_make_panel(&theme->switch_knob, surface, border, 1);
  wing_theme_make_panel(&theme->checkbox_box, surface, border, 1);
  wing_theme_make_fill(&theme->checkbox_checked, primary);
  wing_theme_make_stroke(&theme->checkbox_mark, surface, 2);
  theme->text = text;
}

void wing_theme_init_high_contrast(wing_theme_t *theme)
{
  wing_color_t background;
  wing_color_t surface;
  wing_color_t surface_alt;
  wing_color_t primary;
  wing_color_t primary_alt;
  wing_color_t accent;
  wing_color_t text;
  wing_color_t border;
  wing_color_t focus;
  wing_color_t success;
  wing_color_t disabled;

  if (theme == NULL)
    {
      return;
    }

  background = wing_color_rgba(8, 13, 24, 255);
  surface = wing_color_rgba(15, 23, 42, 255);
  surface_alt = wing_color_rgba(30, 41, 59, 255);
  primary = wing_color_rgba(250, 204, 21, 255);
  primary_alt = wing_color_rgba(234, 179, 8, 255);
  accent = wing_color_rgba(56, 189, 248, 255);
  text = wing_color_rgba(248, 250, 252, 255);
  border = wing_color_rgba(148, 163, 184, 255);
  focus = wing_color_rgba(45, 212, 191, 255);
  success = wing_color_rgba(74, 222, 128, 255);
  disabled = wing_color_rgba(71, 85, 105, 255);

  wing_theme_make_clear(&theme->root, background);
  wing_theme_make_panel(&theme->header, surface, primary, 2);
  wing_theme_make_panel(&theme->panel, surface, border, 1);
  wing_theme_make_panel(&theme->button, primary, primary_alt, 2);
  wing_theme_make_panel(&theme->button_focused, primary_alt, focus, 2);
  wing_theme_make_panel(&theme->button_pressed, accent, primary, 2);
  wing_theme_make_stroke(&theme->line_primary, primary, 2);
  wing_theme_make_stroke(&theme->line_secondary, border, 1);
  wing_theme_make_panel(&theme->progress_frame, surface_alt, border, 1);
  wing_theme_make_fill(&theme->progress_fill, success);
  wing_theme_make_stroke(&theme->slider_track, surface_alt, 4);
  wing_theme_make_stroke(&theme->slider_fill, primary, 4);
  wing_theme_make_panel(&theme->slider_knob, surface, primary, 2);
  wing_theme_make_stroke(&theme->slider_focused, focus, 2);
  wing_theme_make_fill(&theme->scrollbar_track, surface_alt);
  wing_theme_make_fill(&theme->scrollbar_thumb, primary);
  wing_theme_make_stroke(&theme->scrollbar_focused, focus, 1);
  wing_theme_make_fill(&theme->switch_off, disabled);
  wing_theme_make_fill(&theme->switch_on, success);
  wing_theme_make_panel(&theme->switch_knob, text, border, 1);
  wing_theme_make_panel(&theme->checkbox_box, surface, border, 1);
  wing_theme_make_fill(&theme->checkbox_checked, primary);
  wing_theme_make_stroke(&theme->checkbox_mark, background, 2);
  theme->text = text;
}
