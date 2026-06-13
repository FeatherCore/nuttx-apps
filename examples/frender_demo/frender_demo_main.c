/****************************************************************************
 * apps/examples/frender_demo/frender_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <frender/frender.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH
#  define CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH 320
#endif

#ifndef CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT
#  define CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT 240
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t g_frender_demo_image_pixels[16] =
{
  0xf0b35aff, 0xe65d4fff, 0x4d6f8cff, 0x1d2530ff,
  0xe65d4fff, 0xf2eedcff, 0x5ab086ff, 0x4d6f8cff,
  0x4d6f8cff, 0x5ab086ff, 0xf2eedcff, 0xe0ad54ff,
  0x1d2530ff, 0x4d6f8cff, 0xe0ad54ff, 0xf2eedcff
};

static void frender_demo_print_flag(const char *name, bool *first)
{
  if (!*first)
    {
      printf(" ");
    }

  printf("%s", name);
  *first = false;
}

static void frender_demo_print_draw_caps(uint32_t caps)
{
  bool first = true;

  printf("draw=[");
  if ((caps & FR_DRAW_CAP_COMMANDS) != 0)
    {
      frender_demo_print_flag("commands", &first);
    }

  if ((caps & FR_DRAW_CAP_CLEAR) != 0)
    {
      frender_demo_print_flag("clear", &first);
    }

  if ((caps & FR_DRAW_CAP_FILL_RECT) != 0)
    {
      frender_demo_print_flag("fill_rect", &first);
    }

  if ((caps & FR_DRAW_CAP_STROKE_RECT) != 0)
    {
      frender_demo_print_flag("stroke_rect", &first);
    }

  if ((caps & FR_DRAW_CAP_FILL_QUAD) != 0)
    {
      frender_demo_print_flag("fill_quad", &first);
    }

  if ((caps & FR_DRAW_CAP_FILL_TRIANGLE) != 0)
    {
      frender_demo_print_flag("fill_triangle", &first);
    }

  if ((caps & FR_DRAW_CAP_STROKE_QUAD) != 0)
    {
      frender_demo_print_flag("stroke_quad", &first);
    }

  if ((caps & FR_DRAW_CAP_CLIP) != 0)
    {
      frender_demo_print_flag("clip", &first);
    }

  if ((caps & FR_DRAW_CAP_BLIT) != 0)
    {
      frender_demo_print_flag("blit", &first);
    }

  if ((caps & FR_DRAW_CAP_BLIT_QUAD) != 0)
    {
      frender_demo_print_flag("blit_quad", &first);
    }

  if ((caps & FR_DRAW_CAP_TEXT) != 0)
    {
      frender_demo_print_flag("text", &first);
    }

  if (first)
    {
      printf("none");
    }

  printf("]");
}

static void frender_demo_print_present_caps(uint32_t caps)
{
  bool first = true;

  printf("present=[");
  if ((caps & FR_PRESENT_CAP_FRAMEBUFFER) != 0)
    {
      frender_demo_print_flag("framebuffer", &first);
    }

  if ((caps & FR_PRESENT_CAP_LCD) != 0)
    {
      frender_demo_print_flag("lcd", &first);
    }

  if ((caps & FR_PRESENT_CAP_NX) != 0)
    {
      frender_demo_print_flag("nx", &first);
    }

  if ((caps & FR_PRESENT_CAP_UPDATE_RECT) != 0)
    {
      frender_demo_print_flag("update_rect", &first);
    }

  if ((caps & FR_PRESENT_CAP_VSYNC) != 0)
    {
      frender_demo_print_flag("vsync", &first);
    }

  if (first)
    {
      printf("none");
    }

  printf("]");
}

static void frender_demo_print_memory_caps(uint32_t caps)
{
  bool first = true;

  printf("memory=[");
  if ((caps & FR_MEMORY_CAP_SURFACE) != 0)
    {
      frender_demo_print_flag("surface", &first);
    }

  if ((caps & FR_MEMORY_CAP_MMAP) != 0)
    {
      frender_demo_print_flag("mmap", &first);
    }

  if ((caps & FR_MEMORY_CAP_DIRECT_PTR) != 0)
    {
      frender_demo_print_flag("direct_ptr", &first);
    }

  if ((caps & FR_MEMORY_CAP_PIXEL_WRITE) != 0)
    {
      frender_demo_print_flag("pixel_write", &first);
    }

  if (first)
    {
      printf("none");
    }

  printf("]");
}

static void frender_demo_print_sync_caps(uint32_t caps)
{
  bool first = true;

  printf("sync=[");
  if ((caps & FR_SYNC_CAP_UPDATE_RECT) != 0)
    {
      frender_demo_print_flag("update_rect", &first);
    }

  if ((caps & FR_SYNC_CAP_VSYNC_WAIT) != 0)
    {
      frender_demo_print_flag("vsync_wait", &first);
    }

  if ((caps & FR_SYNC_CAP_FENCE) != 0)
    {
      frender_demo_print_flag("fence", &first);
    }

  if (first)
    {
      printf("none");
    }

  printf("]");
}

static void frender_demo_print_format_caps(uint32_t caps)
{
  bool first = true;

  printf("format=[");
  if ((caps & FR_FORMAT_CAP_RGBA8888) != 0)
    {
      frender_demo_print_flag("rgba8888", &first);
    }

  if ((caps & FR_FORMAT_CAP_RGB565) != 0)
    {
      frender_demo_print_flag("rgb565", &first);
    }

  if ((caps & FR_FORMAT_CAP_RGB24) != 0)
    {
      frender_demo_print_flag("rgb24", &first);
    }

  if ((caps & FR_FORMAT_CAP_RGB32) != 0)
    {
      frender_demo_print_flag("rgb32", &first);
    }

  if ((caps & FR_FORMAT_CAP_RGBT32) != 0)
    {
      frender_demo_print_flag("rgbt32", &first);
    }

  if ((caps & FR_FORMAT_CAP_RGBA32) != 0)
    {
      frender_demo_print_flag("rgba32", &first);
    }

  if (first)
    {
      printf("none");
    }

  printf("]");
}

static void frender_demo_print_caps(const char *label,
                                    const fr_backend_caps_t *caps)
{
  printf("frender_demo: %s caps name=%s kind=%d legacy=0x%08x\n",
         label, caps->name, (int)caps->kind, (unsigned int)caps->caps);
  printf("frender_demo: %s draw=0x%08x present=0x%08x memory=0x%08x sync=0x%08x format=0x%08x preferred=%d max=%ux%u commands=%u\n",
         label, (unsigned int)caps->draw_caps,
         (unsigned int)caps->present_caps,
         (unsigned int)caps->memory_caps,
         (unsigned int)caps->sync_caps,
         (unsigned int)caps->format_caps,
         (int)caps->preferred_format,
         (unsigned int)caps->max_width,
         (unsigned int)caps->max_height,
         (unsigned int)caps->max_commands);
  printf("frender_demo: %s ", label);
  frender_demo_print_draw_caps(caps->draw_caps);
  printf(" ");
  frender_demo_print_present_caps(caps->present_caps);
  printf(" ");
  frender_demo_print_memory_caps(caps->memory_caps);
  printf(" ");
  frender_demo_print_sync_caps(caps->sync_caps);
  printf(" ");
  frender_demo_print_format_caps(caps->format_caps);
  printf("\n");
}

static void frender_demo_print_registry(const char *stage)
{
  const fr_backend_caps_t *caps;
  uint8_t count;
  uint8_t i;

  count = fr_backend_registry_count();
  printf("frender_demo: backend registry %s count=%u\n",
         stage, (unsigned int)count);

  for (i = 0; i < count; i++)
    {
      caps = fr_backend_registry_get(i);
      if (caps != NULL)
        {
          frender_demo_print_caps("registered", caps);
        }
    }
}

static int frender_demo_render_stage(const char *stage,
                                     fr_surface_t *surface,
                                     const fr_command_list_t *list)
{
  uint32_t checksum;
  int ret;

  ret = fr_execute_software(surface, list);
  if (ret < 0)
    {
      printf("frender_demo: stage=%s failed: %d\n", stage, ret);
      return ret;
    }

  checksum = fr_surface_checksum_rgba8888(surface);
  printf("frender_demo: stage=%s commands=%u checksum=0x%08x\n",
         stage, (unsigned int)fr_command_list_count(list),
         (unsigned int)checksum);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  fr_command_t commands[16];
  fr_command_list_t list;
  fr_backend_caps_t fb_caps;
  fr_backend_caps_t sw_caps;
  fr_backend_caps_t nuttx_caps;
  fr_fb_presenter_t presenter;
  fr_surface_t image;
  fr_surface_t surface;
  fr_quad_t quad;
  fr_triangle_t triangle;
  fr_rect_t full_rect;
  fr_rect_t rect;
  fr_rect_t src_rect;
  uint32_t *pixels;
  uint32_t checksum;
  size_t pixel_count;
  int ret;

  pixel_count = (size_t)CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH *
                CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT;

  pixels = (uint32_t *)malloc(pixel_count * sizeof(uint32_t));
  if (pixels == NULL)
    {
      printf("frender_demo: failed to allocate %u pixels\n",
             (unsigned int)pixel_count);
      return EXIT_FAILURE;
    }

  ret = fr_surface_init(&surface, pixels,
                        CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH,
                        CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT,
                        CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH,
                        FR_FORMAT_RGBA8888);
  if (ret < 0)
    {
      printf("frender_demo: surface init failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  ret = fr_surface_init(&image, g_frender_demo_image_pixels, 4, 4, 4,
                        FR_FORMAT_RGBA8888);
  if (ret < 0)
    {
      printf("frender_demo: image surface init failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  sw_caps = fr_backend_caps_software();
  nuttx_caps = fr_backend_caps_nuttx_graphics();
  frender_demo_print_caps("software", &sw_caps);
  frender_demo_print_caps("nuttx", &nuttx_caps);

  fr_backend_registry_reset();
  ret = fr_backend_register_builtin();
  if (ret < 0)
    {
      printf("frender_demo: builtin backend register failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  frender_demo_print_registry("after builtin register");

  printf("frender_demo: command delivery path: app -> frender command list -> software backend -> memory surface\n");
  printf("frender_demo: nuttx graphics is capability/present adapter target, not a command-list submit target yet\n");

  fr_command_list_init(&list, commands, 16);

  ret = fr_cmd_clear(&list, fr_color_rgba(19, 25, 31, 255));
  if (ret == 0)
    {
      ret = frender_demo_render_stage("clear", &surface, &list);
    }

  if (ret == 0)
    {
      rect.x = 18;
      rect.y = 24;
      rect.w = CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH - 36;
      rect.h = 70;
      ret = fr_cmd_fill_rect(&list, &rect,
                             fr_color_rgba(234, 238, 220, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("fill_rect", &surface, &list);
    }

  if (ret == 0)
    {
      ret = fr_cmd_stroke_rect(&list, &rect, 2,
                               fr_color_rgba(91, 134, 118, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("stroke_rect", &surface, &list);
    }

  if (ret == 0)
    {
      rect.x = 36;
      rect.y = 44;
      rect.w = 96;
      rect.h = 28;
      ret = fr_cmd_fill_rect(&list, &rect,
                             fr_color_rgba(217, 102, 71, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("second_fill_rect", &surface, &list);
    }

  if (ret == 0)
    {
      rect.x = 148;
      rect.y = 44;
      rect.w = 122;
      rect.h = 14;
      ret = fr_cmd_fill_rect(&list, &rect,
                             fr_color_rgba(73, 94, 111, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("third_fill_rect", &surface, &list);
    }

  if (ret == 0)
    {
      quad.points[0].x = 210;
      quad.points[0].y = 104;
      quad.points[1].x = 286;
      quad.points[1].y = 118;
      quad.points[2].x = 266;
      quad.points[2].y = 170;
      quad.points[3].x = 192;
      quad.points[3].y = 152;
      ret = fr_cmd_fill_quad(&list, &quad,
                             fr_color_rgba(219, 176, 91, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("fill_quad", &surface, &list);
    }

  if (ret == 0)
    {
      triangle.points[0].x = 160;
      triangle.points[0].y = 100;
      triangle.points[1].x = 196;
      triangle.points[1].y = 174;
      triangle.points[2].x = 124;
      triangle.points[2].y = 174;
      ret = fr_cmd_fill_triangle(&list, &triangle,
                                 fr_color_rgba(90, 176, 134, 220));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("fill_triangle", &surface, &list);
    }

  if (ret == 0)
    {
      ret = fr_cmd_stroke_quad(&list, &quad, 3,
                               fr_color_rgba(73, 94, 111, 255));
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("stroke_quad", &surface, &list);
    }

  if (ret == 0)
    {
      src_rect.x = 0;
      src_rect.y = 0;
      src_rect.w = 4;
      src_rect.h = 4;
      rect.x = 56;
      rect.y = 102;
      rect.w = 80;
      rect.h = 56;
      ret = fr_cmd_blit(&list, &image, &src_rect, &rect);
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("blit", &surface, &list);
    }

  if (ret == 0)
    {
      rect.x = 24;
      rect.y = 120;
      rect.w = CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH - 48;
      rect.h = 30;
      ret = fr_cmd_push_clip(&list, &rect);
    }

  if (ret == 0)
    {
      rect.x = 24;
      rect.y = 120;
      rect.w = CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH - 48;
      rect.h = 30;
      ret = fr_cmd_stroke_rect(&list, &rect, 1,
                               fr_color_rgba(91, 134, 118, 255));
    }

  if (ret == 0)
    {
      rect.x = 26;
      rect.y = 122;
      rect.w = (CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH - 52) * 3 / 5;
      rect.h = 26;
      ret = fr_cmd_fill_rect(&list, &rect,
                             fr_color_rgba(90, 176, 134, 255));
    }

  if (ret == 0)
    {
      ret = fr_cmd_pop_clip(&list);
    }

  if (ret == 0)
    {
      ret = frender_demo_render_stage("clip_push_fill_pop", &surface, &list);
    }

  if (ret < 0)
    {
      printf("frender_demo: demo failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  checksum = fr_surface_checksum_rgba8888(&surface);
  printf("frender_demo: final rendered %dx%d commands=%u checksum=0x%08x\n",
         CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH,
         CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT,
         (unsigned int)fr_command_list_count(&list),
         (unsigned int)checksum);

  ret = fr_fb_presenter_open(&presenter, "/dev/fb0");
  if (ret == 0)
    {
      full_rect.x = 0;
      full_rect.y = 0;
      full_rect.w = CONFIG_EXAMPLES_FRENDER_DEMO_WIDTH;
      full_rect.h = CONFIG_EXAMPLES_FRENDER_DEMO_HEIGHT;

      printf("frender_demo: framebuffer %ux%u fmt=%u bpp=%u stride=%u\n",
             (unsigned int)presenter.xres, (unsigned int)presenter.yres,
             (unsigned int)presenter.fmt, (unsigned int)presenter.bpp,
             (unsigned int)presenter.stride);

      ret = fr_backend_caps_from_fb_presenter(&presenter, &fb_caps);
      if (ret == 0)
        {
          frender_demo_print_caps("framebuffer", &fb_caps);
          ret = fr_backend_register_fb_presenter(&presenter);
          if (ret == 0)
            {
              frender_demo_print_registry("after framebuffer probe");
            }
        }

      ret = fr_fb_presenter_present(&presenter, &surface);
      if (ret == 0)
        {
          ret = fr_fb_presenter_update_rect(&presenter, &full_rect);
        }

      if (ret < 0)
        {
          printf("frender_demo: framebuffer present failed: %d\n", ret);
        }
      else
        {
          printf("frender_demo: framebuffer present ok\n");
          printf("frender_demo: close the framebuffer window to exit demo\n");
          while (!fr_fb_presenter_window_closed(&presenter))
            {
              usleep(50 * 1000);
            }

          printf("frender_demo: framebuffer window closed\n");
        }

      fr_fb_presenter_close(&presenter);
    }
  else
    {
      printf("frender_demo: framebuffer present skipped: %d\n", ret);
    }

  free(pixels);
  return EXIT_SUCCESS;
}
