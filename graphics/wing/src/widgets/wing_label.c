/****************************************************************************
 * apps/graphics/wing/src/widgets/wing_label.c
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
#include <wing/core/wing_font.h>
#include <wing/core/wing_widget_style.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static const wing_font_t *wing_label_active_font(const wing_label_t *label)
{
  return label != NULL && label->font != NULL ?
         label->font : wing_font_builtin_5x7();
}

static int wing_label_draw_glyph(wing_context_t *ctx,
                                 const wing_obj_t *obj,
                                 const wing_rect_t *bounds,
                                 int32_t glyph_x, int32_t glyph_y,
                                 uint8_t scale,
                                 const wing_bitmap_glyph_t *glyph,
                                 wing_color_t color)
{
  wing_rect_t pixel;
  uint8_t row;
  uint8_t col;
  int32_t x;
  int32_t y;

  if (glyph == NULL || glyph->rows == NULL || glyph->width == 0 ||
      glyph->height == 0)
    {
      return 0;
    }

  for (row = 0; row < glyph->height; row++)
    {
      for (col = 0; col < glyph->width; col++)
        {
          if ((glyph->rows[row] & (1 << (glyph->width - 1 - col))) == 0)
            {
              continue;
            }

          x = glyph_x + (int32_t)col * scale;
          y = glyph_y + (int32_t)row * scale;
          if (x < bounds->x || y < bounds->y ||
              x + scale > (int32_t)bounds->x + bounds->w ||
              y + scale > (int32_t)bounds->y + bounds->h)
            {
              continue;
            }

          pixel.x = (int16_t)x;
          pixel.y = (int16_t)y;
          pixel.w = scale;
          pixel.h = scale;

          if (wing_widget_fill_rect_for_obj(ctx, obj, &pixel, color) < 0)
            {
              return -EINVAL;
            }
        }
    }

  return 0;
}

static int wing_label_measure_line(const wing_font_t *font,
                                   const char *text, uint8_t scale,
                                   uint16_t *width)
{
  wing_bitmap_glyph_t glyph;
  const char *cursor;
  uint32_t codepoint;
  uint32_t measured_width;
  int ret;

  if (text == NULL || scale == 0 || width == NULL)
    {
      return -EINVAL;
    }

  cursor = text;
  measured_width = 0;
  while (*cursor != '\0')
    {
      ret = wing_text_next_codepoint(&cursor, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0 || codepoint == '\n')
        {
          break;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      measured_width += glyph.advance;
    }

  *width = (uint16_t)(measured_width * scale);
  return 0;
}

static int wing_label_measure_span(const wing_font_t *font,
                                   const char *start,
                                   const char *end,
                                   uint8_t scale,
                                   uint16_t *width)
{
  wing_bitmap_glyph_t glyph;
  const char *cursor;
  uint32_t codepoint;
  uint32_t measured_width;
  int ret;

  if (font == NULL || start == NULL || end == NULL || scale == 0 ||
      width == NULL)
    {
      return -EINVAL;
    }

  cursor = start;
  measured_width = 0;
  while (cursor < end && *cursor != '\0')
    {
      ret = wing_text_next_codepoint(&cursor, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0 || codepoint == '\n')
        {
          break;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      measured_width += glyph.advance;
    }

  *width = (uint16_t)(measured_width * scale);
  return 0;
}

static const char *wing_label_find_line_end(const char *text)
{
  const char *cursor;

  if (text == NULL)
    {
      return NULL;
    }

  cursor = text;
  while (*cursor != '\0' && *cursor != '\n')
    {
      cursor++;
    }

  return cursor;
}

static int wing_label_fit_span(const wing_font_t *font,
                               const char *start,
                               const char *end,
                               uint8_t scale,
                               uint16_t max_width,
                               const char **fit_end,
                               uint16_t *fit_width)
{
  wing_bitmap_glyph_t glyph;
  const char *cursor;
  const char *last_break;
  const char *next;
  uint32_t codepoint;
  uint32_t break_width;
  uint32_t width;
  uint32_t glyph_width;
  int ret;

  if (font == NULL || start == NULL || end == NULL || fit_end == NULL ||
      fit_width == NULL || scale == 0)
    {
      return -EINVAL;
    }

  cursor = start;
  last_break = NULL;
  break_width = 0;
  width = 0;
  while (cursor < end && *cursor != '\0')
    {
      next = cursor;
      ret = wing_text_next_codepoint(&next, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0 || codepoint == '\n')
        {
          break;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      glyph_width = (uint32_t)glyph.advance * scale;
      if (max_width > 0 && width > 0 && width + glyph_width > max_width)
        {
          if (last_break != NULL)
            {
              cursor = last_break;
              width = break_width;
            }

          break;
        }

      width += glyph_width;
      cursor = next;
      if (codepoint == ' ' || codepoint == '\t')
        {
          last_break = cursor;
          break_width = width - glyph_width;
        }

      if (max_width > 0 && width >= max_width)
        {
          break;
        }
    }

  if (cursor == start && start < end)
    {
      next = cursor;
      ret = wing_text_next_codepoint(&next, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret > 0 && wing_font_get_glyph(font, codepoint, &glyph))
        {
          width = (uint32_t)glyph.advance * scale;
          cursor = next;
        }
    }

  *fit_end = cursor;
  *fit_width = (uint16_t)width;
  return 0;
}

static int wing_label_measure_layout(const wing_label_t *label,
                                     const wing_rect_t *bounds,
                                     uint16_t *width,
                                     uint16_t *height)
{
  const wing_font_t *font;
  const char *cursor;
  const char *line_end;
  const char *span_end;
  uint16_t ellipsis_width;
  uint16_t line_width;
  uint16_t max_width;
  uint16_t measured_height;
  uint16_t measured_width;
  int ret;

  if (label == NULL || width == NULL || height == NULL ||
      label->scale == 0)
    {
      return -EINVAL;
    }

  font = wing_label_active_font(label);
  if (bounds == NULL || label->text_mode == WING_LABEL_TEXT_MODE_CLIP)
    {
      return wing_font_measure_text(font, label->text, label->scale,
                                    width, height);
    }

  max_width = bounds->w;
  cursor = label->text;
  measured_width = 0;
  measured_height = 0;

  if (label->text_mode == WING_LABEL_TEXT_MODE_ELLIPSIS)
    {
      line_end = wing_label_find_line_end(cursor);
      ret = wing_label_measure_span(font, cursor, line_end, label->scale,
                                    &line_width);
      if (ret < 0)
        {
          return ret;
        }

      ret = wing_label_measure_span(font, "...", "..." + 3,
                                    label->scale, &ellipsis_width);
      if (ret < 0)
        {
          return ret;
        }

      if (line_width > max_width)
        {
          line_width = max_width > ellipsis_width ? max_width :
                       ellipsis_width;
        }

      *width = line_width;
      *height = (uint16_t)(font->line_height * label->scale);
      return 0;
    }

  while (*cursor != '\0')
    {
      line_end = wing_label_find_line_end(cursor);
      if (cursor == line_end)
        {
          measured_height += font->line_height * label->scale;
        }
      else
        {
          while (cursor < line_end)
            {
              ret = wing_label_fit_span(font, cursor, line_end,
                                        label->scale, max_width,
                                        &span_end, &line_width);
              if (ret < 0)
                {
                  return ret;
                }

              if (line_width > measured_width)
                {
                  measured_width = line_width;
                }

              measured_height += font->line_height * label->scale;
              if (span_end <= cursor)
                {
                  break;
                }

              cursor = span_end;
            }
        }

      if (*line_end == '\n')
        {
          cursor = line_end + 1;
        }
      else
        {
          cursor = line_end;
        }
    }

  if (measured_height == 0)
    {
      measured_height = (uint16_t)(font->line_height * label->scale);
    }

  *width = measured_width;
  *height = measured_height;
  return 0;
}

static int wing_label_draw_span(wing_context_t *ctx,
                                const wing_obj_t *obj,
                                const wing_rect_t *bounds,
                                const wing_font_t *font,
                                const char *start,
                                const char *end,
                                int32_t x,
                                int32_t y,
                                uint8_t scale,
                                wing_color_t color)
{
  wing_bitmap_glyph_t glyph;
  const char *cursor;
  uint32_t codepoint;
  int ret;

  if (ctx == NULL || bounds == NULL || font == NULL || start == NULL ||
      end == NULL || scale == 0)
    {
      return -EINVAL;
    }

  cursor = start;
  while (cursor < end && *cursor != '\0')
    {
      ret = wing_text_next_codepoint(&cursor, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0 || codepoint == '\n')
        {
          break;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      ret = wing_label_draw_glyph(ctx, obj, bounds, x, y, scale, &glyph,
                                  color);
      if (ret < 0)
        {
          return ret;
        }

      x += (int32_t)glyph.advance * scale;
    }

  return 0;
}

static int32_t wing_label_aligned_x(const wing_rect_t *bounds,
                                    enum wing_text_align_e align,
                                    uint16_t line_width)
{
  int32_t x;

  x = bounds->x;
  if (align == WING_TEXT_ALIGN_CENTER && line_width < bounds->w)
    {
      x += ((int32_t)bounds->w - line_width) / 2;
    }
  else if (align == WING_TEXT_ALIGN_RIGHT && line_width < bounds->w)
    {
      x += (int32_t)bounds->w - line_width;
    }

  return x;
}

static int wing_label_draw_ellipsis(wing_obj_t *obj,
                                    wing_context_t *ctx,
                                    const wing_font_t *font,
                                    wing_label_t *label,
                                    wing_color_t color,
                                    int32_t y)
{
  const wing_rect_t *bounds;
  const char *line_end;
  const char *prefix_end;
  uint16_t ellipsis_width;
  uint16_t line_width;
  uint16_t max_prefix_width;
  uint16_t prefix_width;
  int32_t x;
  int ret;

  bounds = wing_obj_get_bounds(obj);
  line_end = wing_label_find_line_end(label->text);
  ret = wing_label_measure_span(font, label->text, line_end, label->scale,
                                &line_width);
  if (ret < 0)
    {
      return ret;
    }

  if (line_width <= bounds->w)
    {
      x = wing_label_aligned_x(bounds, label->align, line_width);
      return wing_label_draw_span(ctx, obj, bounds, font, label->text,
                                  line_end, x, y, label->scale, color);
    }

  ret = wing_label_measure_span(font, "...", "..." + 3, label->scale,
                                &ellipsis_width);
  if (ret < 0)
    {
      return ret;
    }

  max_prefix_width = bounds->w > ellipsis_width ?
                     bounds->w - ellipsis_width : 0;
  ret = wing_label_fit_span(font, label->text, line_end, label->scale,
                            max_prefix_width, &prefix_end, &prefix_width);
  if (ret < 0)
    {
      return ret;
    }

  line_width = prefix_width + ellipsis_width;
  x = wing_label_aligned_x(bounds, label->align, line_width);
  ret = wing_label_draw_span(ctx, obj, bounds, font, label->text,
                             prefix_end, x, y, label->scale, color);
  if (ret < 0)
    {
      return ret;
    }

  return wing_label_draw_span(ctx, obj, bounds, font, "...", "..." + 3,
                              x + prefix_width, y, label->scale, color);
}

static int wing_label_draw(wing_obj_t *obj, wing_context_t *ctx)
{
  const wing_rect_t *bounds;
  wing_label_t *label;
  wing_bitmap_glyph_t glyph;
  wing_color_t color;
  const wing_font_t *font;
  const char *text;
  uint32_t codepoint;
  uint16_t line_w;
  uint16_t text_w;
  uint16_t text_h;
  int32_t line_x;
  int32_t x;
  int32_t y;
  int32_t line_advance;
  int ret;

  if (obj == NULL || ctx == NULL)
    {
      return -EINVAL;
    }

  label = (wing_label_t *)obj;
  bounds = wing_obj_get_bounds(obj);
  if (bounds == NULL || label->text == NULL || label->scale == 0)
    {
      return 0;
    }

  font = wing_label_active_font(label);
  ret = wing_label_measure_layout(label, bounds, &text_w, &text_h);
  if (ret < 0)
    {
      return ret;
    }

  line_advance = (int32_t)font->line_height * label->scale;
  x = bounds->x;
  if (label->align == WING_TEXT_ALIGN_CENTER && text_w < bounds->w)
    {
      x += ((int32_t)bounds->w - text_w) / 2;
    }
  else if (label->align == WING_TEXT_ALIGN_RIGHT && text_w < bounds->w)
    {
      x += (int32_t)bounds->w - text_w;
    }

  y = bounds->y;
  if (text_h < bounds->h)
    {
      y += ((int32_t)bounds->h - text_h) / 2;
    }

  text = label->text;
  color = wing_widget_style_color_for_obj(obj, NULL, label->color);
  if (label->text_mode == WING_LABEL_TEXT_MODE_ELLIPSIS)
    {
      return wing_label_draw_ellipsis(obj, ctx, font, label, color, y);
    }

  if (label->text_mode == WING_LABEL_TEXT_MODE_WRAP)
    {
      const char *cursor;
      const char *line_end;
      const char *span_end;

      cursor = text;
      while (*cursor != '\0' && y < (int32_t)bounds->y + bounds->h)
        {
          line_end = wing_label_find_line_end(cursor);
          if (cursor == line_end)
            {
              y += line_advance;
            }
          else
            {
              while (cursor < line_end &&
                     y < (int32_t)bounds->y + bounds->h)
                {
                  ret = wing_label_fit_span(font, cursor, line_end,
                                            label->scale, bounds->w,
                                            &span_end, &line_w);
                  if (ret < 0)
                    {
                      return ret;
                    }

                  line_x = wing_label_aligned_x(bounds, label->align,
                                                line_w);
                  ret = wing_label_draw_span(ctx, obj, bounds, font, cursor,
                                             span_end, line_x, y,
                                             label->scale, color);
                  if (ret < 0)
                    {
                      return ret;
                    }

                  y += line_advance;
                  if (span_end <= cursor)
                    {
                      break;
                    }

                  cursor = span_end;
                }
            }

          if (*line_end == '\n')
            {
              cursor = line_end + 1;
            }
          else
            {
              cursor = line_end;
            }
        }

      return 0;
    }

  ret = wing_label_measure_line(font, text, label->scale, &line_w);
  if (ret < 0)
    {
      return ret;
    }

  line_x = bounds->x;
  if (label->align == WING_TEXT_ALIGN_CENTER && line_w < bounds->w)
    {
      line_x += ((int32_t)bounds->w - line_w) / 2;
    }
  else if (label->align == WING_TEXT_ALIGN_RIGHT && line_w < bounds->w)
    {
      line_x += (int32_t)bounds->w - line_w;
    }

  x = line_x;
  while (*text != '\0')
    {
      ret = wing_text_next_codepoint(&text, &codepoint);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0)
        {
          break;
        }

      if (codepoint == '\n')
        {
          y += line_advance;
          if (y >= (int32_t)bounds->y + bounds->h)
            {
              break;
            }

          ret = wing_label_measure_line(font, text, label->scale, &line_w);
          if (ret < 0)
            {
              return ret;
            }

          line_x = bounds->x;
          if (label->align == WING_TEXT_ALIGN_CENTER && line_w < bounds->w)
            {
              line_x += ((int32_t)bounds->w - line_w) / 2;
            }
          else if (label->align == WING_TEXT_ALIGN_RIGHT &&
                   line_w < bounds->w)
            {
              line_x += (int32_t)bounds->w - line_w;
            }

          x = line_x;
          continue;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      ret = wing_label_draw_glyph(ctx, obj, bounds, x, y, label->scale,
                                  &glyph, color);
      if (ret < 0)
        {
          return ret;
        }

      x += (int32_t)glyph.advance * label->scale;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_label_init(wing_label_t *label, const wing_rect_t *bounds,
                     const char *text, wing_color_t color, uint8_t scale)
{
  if (label == NULL)
    {
      return;
    }

  wing_obj_init(&label->obj, bounds);
  label->text = text == NULL ? "" : text;
  label->font = wing_font_builtin_5x7();
  label->color = color;
  label->align = WING_TEXT_ALIGN_LEFT;
  label->text_mode = WING_LABEL_TEXT_MODE_CLIP;
  label->scale = scale == 0 ? 1 : scale;
  wing_obj_set_draw_cb(&label->obj, wing_label_draw);
}

wing_obj_t *wing_label_obj(wing_label_t *label)
{
  return label != NULL ? &label->obj : NULL;
}

void wing_label_set_text(wing_label_t *label, const char *text)
{
  if (label == NULL)
    {
      return;
    }

  label->text = text == NULL ? "" : text;
  wing_obj_invalidate(&label->obj);
}

void wing_label_set_color(wing_label_t *label, wing_color_t color)
{
  if (label == NULL)
    {
      return;
    }

  label->color = color;
  wing_obj_invalidate(&label->obj);
}

void wing_label_set_scale(wing_label_t *label, uint8_t scale)
{
  if (label == NULL || scale == 0)
    {
      return;
    }

  label->scale = scale;
  wing_obj_invalidate(&label->obj);
}

void wing_label_set_font(wing_label_t *label, const wing_font_t *font)
{
  if (label == NULL)
    {
      return;
    }

  label->font = font == NULL ? wing_font_builtin_5x7() : font;
  wing_obj_invalidate(&label->obj);
}

const wing_font_t *wing_label_get_font(const wing_label_t *label)
{
  return wing_label_active_font(label);
}

void wing_label_set_align(wing_label_t *label, enum wing_text_align_e align)
{
  if (label == NULL)
    {
      return;
    }

  label->align = align;
  wing_obj_invalidate(&label->obj);
}

enum wing_text_align_e wing_label_get_align(const wing_label_t *label)
{
  return label == NULL ? WING_TEXT_ALIGN_LEFT : label->align;
}

void wing_label_set_text_mode(wing_label_t *label,
                              enum wing_label_text_mode_e mode)
{
  if (label == NULL)
    {
      return;
    }

  label->text_mode = mode;
  wing_obj_invalidate(&label->obj);
}

enum wing_label_text_mode_e wing_label_get_text_mode(
  const wing_label_t *label)
{
  return label == NULL ? WING_LABEL_TEXT_MODE_CLIP : label->text_mode;
}

int wing_label_get_text_size(const wing_label_t *label, uint16_t *width,
                             uint16_t *height)
{
  if (label == NULL)
    {
      return -EINVAL;
    }

  return wing_font_measure_text(wing_label_active_font(label), label->text,
                                label->scale, width, height);
}

int wing_label_get_layout_size(const wing_label_t *label, uint16_t *width,
                               uint16_t *height)
{
  if (label == NULL)
    {
      return -EINVAL;
    }

  return wing_label_measure_layout(label, wing_obj_get_bounds(&label->obj),
                                   width, height);
}
