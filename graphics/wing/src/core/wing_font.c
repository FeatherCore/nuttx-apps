/****************************************************************************
 * apps/graphics/wing/src/core/wing_font.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <stddef.h>

#include <wing/core/wing_font.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_font_5x7_blank[7] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t g_font_5x7_question[7] =
{
  0x1f, 0x01, 0x02, 0x04, 0x04, 0x00, 0x04
};

static const uint8_t g_font_5x7_a[7] =
{
  0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11
};

static const uint8_t g_font_5x7_d[7] =
{
  0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e
};

static const uint8_t g_font_5x7_e[7] =
{
  0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f
};

static const uint8_t g_font_5x7_g[7] =
{
  0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f
};

static const uint8_t g_font_5x7_l[7] =
{
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f
};

static const uint8_t g_font_5x7_n[7] =
{
  0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11
};

static const uint8_t g_font_5x7_o[7] =
{
  0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e
};

static const uint8_t g_font_5x7_r[7] =
{
  0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11
};

static const uint8_t g_font_5x7_t[7] =
{
  0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04
};

static const uint8_t g_font_5x7_w[7] =
{
  0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a
};

static const uint8_t g_font_5x7_y[7] =
{
  0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04
};

static const uint8_t g_font_5x7_1[7] =
{
  0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e
};

static const uint8_t g_font_5x7_dot[7] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c
};

static bool wing_font_builtin_5x7_get_glyph(const wing_font_t *font,
                                            uint32_t codepoint,
                                            wing_bitmap_glyph_t *glyph);

static const wing_font_t g_wing_font_5x7 =
{
  "wing-builtin-5x7",
  7,
  6,
  6,
  wing_font_builtin_5x7_get_glyph,
  NULL
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void wing_font_set_glyph(wing_bitmap_glyph_t *glyph,
                                const uint8_t *rows, uint8_t width,
                                uint8_t height, uint8_t advance)
{
  glyph->rows = rows;
  glyph->width = width;
  glyph->height = height;
  glyph->advance = advance;
}

static bool wing_font_builtin_5x7_get_glyph(const wing_font_t *font,
                                            uint32_t codepoint,
                                            wing_bitmap_glyph_t *glyph)
{
  const uint8_t *rows;
  uint8_t advance;

  (void)font;

  if (glyph == NULL)
    {
      return false;
    }

  advance = 6;
  switch (toupper((int)codepoint))
    {
      case ' ':
        rows = g_font_5x7_blank;
        advance = 4;
        break;

      case 'A':
        rows = g_font_5x7_a;
        break;

      case 'D':
        rows = g_font_5x7_d;
        break;

      case 'E':
        rows = g_font_5x7_e;
        break;

      case 'G':
        rows = g_font_5x7_g;
        break;

      case 'L':
        rows = g_font_5x7_l;
        break;

      case 'N':
        rows = g_font_5x7_n;
        break;

      case 'O':
      case '0':
        rows = g_font_5x7_o;
        break;

      case 'R':
        rows = g_font_5x7_r;
        break;

      case 'T':
        rows = g_font_5x7_t;
        break;

      case 'W':
        rows = g_font_5x7_w;
        break;

      case 'Y':
        rows = g_font_5x7_y;
        break;

      case '1':
        rows = g_font_5x7_1;
        break;

      case '.':
        rows = g_font_5x7_dot;
        advance = 3;
        break;

      default:
        rows = g_font_5x7_question;
        break;
    }

  wing_font_set_glyph(glyph, rows, 5, 7, advance);
  return true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

const wing_font_t *wing_font_builtin_5x7(void)
{
  return &g_wing_font_5x7;
}

int wing_text_next_codepoint(const char **text, uint32_t *codepoint)
{
  const unsigned char *src;
  uint32_t cp;

  if (text == NULL || *text == NULL || codepoint == NULL)
    {
      return -EINVAL;
    }

  src = (const unsigned char *)*text;
  if (src[0] == '\0')
    {
      return 0;
    }

  if (src[0] < 0x80)
    {
      *codepoint = src[0];
      *text = (const char *)&src[1];
      return 1;
    }

  if ((src[0] & 0xe0) == 0xc0 &&
      (src[1] & 0xc0) == 0x80)
    {
      cp = ((uint32_t)(src[0] & 0x1f) << 6) |
           (uint32_t)(src[1] & 0x3f);
      if (cp >= 0x80)
        {
          *codepoint = cp;
          *text = (const char *)&src[2];
          return 1;
        }
    }

  if ((src[0] & 0xf0) == 0xe0 &&
      (src[1] & 0xc0) == 0x80 &&
      (src[2] & 0xc0) == 0x80)
    {
      cp = ((uint32_t)(src[0] & 0x0f) << 12) |
           ((uint32_t)(src[1] & 0x3f) << 6) |
           (uint32_t)(src[2] & 0x3f);
      if (cp >= 0x800 && (cp < 0xd800 || cp > 0xdfff))
        {
          *codepoint = cp;
          *text = (const char *)&src[3];
          return 1;
        }
    }

  if ((src[0] & 0xf8) == 0xf0 &&
      (src[1] & 0xc0) == 0x80 &&
      (src[2] & 0xc0) == 0x80 &&
      (src[3] & 0xc0) == 0x80)
    {
      cp = ((uint32_t)(src[0] & 0x07) << 18) |
           ((uint32_t)(src[1] & 0x3f) << 12) |
           ((uint32_t)(src[2] & 0x3f) << 6) |
           (uint32_t)(src[3] & 0x3f);
      if (cp >= 0x10000 && cp <= 0x10ffff)
        {
          *codepoint = cp;
          *text = (const char *)&src[4];
          return 1;
        }
    }

  *codepoint = '?';
  *text = (const char *)&src[1];
  return 1;
}

bool wing_font_get_glyph(const wing_font_t *font, uint32_t codepoint,
                         wing_bitmap_glyph_t *glyph)
{
  if (font == NULL)
    {
      font = wing_font_builtin_5x7();
    }

  if (font->get_glyph == NULL)
    {
      return false;
    }

  return font->get_glyph(font, codepoint, glyph);
}

int wing_font_measure_text(const wing_font_t *font, const char *text,
                           uint8_t scale, uint16_t *width,
                           uint16_t *height)
{
  wing_bitmap_glyph_t glyph;
  uint32_t codepoint;
  uint32_t line_width;
  uint32_t measured_width;
  uint32_t measured_height;
  int ret;

  if (text == NULL || scale == 0 || width == NULL || height == NULL)
    {
      return -EINVAL;
    }

  if (font == NULL)
    {
      font = wing_font_builtin_5x7();
    }

  line_width = 0;
  measured_width = 0;
  measured_height = font->line_height;
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
          if (line_width > measured_width)
            {
              measured_width = line_width;
            }

          line_width = 0;
          measured_height += font->line_height;
          continue;
        }

      if (!wing_font_get_glyph(font, codepoint, &glyph))
        {
          return -EINVAL;
        }

      line_width += glyph.advance;
      if (glyph.height > measured_height)
        {
          measured_height = glyph.height;
        }
    }

  if (line_width > measured_width)
    {
      measured_width = line_width;
    }

  *width = (uint16_t)(measured_width * scale);
  *height = (uint16_t)(measured_height * scale);
  return 0;
}
