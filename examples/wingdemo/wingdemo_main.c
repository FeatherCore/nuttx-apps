/****************************************************************************
 * apps/examples/wingdemo/wingdemo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nuttx/video/fb.h>
#ifdef CONFIG_BOARDCTL_POWEROFF
#  include <sys/boardctl.h>
#endif
#include <lvgl.h>
#include "lvgl/demos/widgets/lv_demo_widgets.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_WINGDEMO_FBDEV
#  define CONFIG_EXAMPLES_WINGDEMO_FBDEV "/dev/fb0"
#endif

#ifndef CONFIG_EXAMPLES_WINGDEMO_FRAMES
#  define CONFIG_EXAMPLES_WINGDEMO_FRAMES 0
#endif

#ifndef CONFIG_EXAMPLES_WINGDEMO_FRAME_DELAY_US
#  define CONFIG_EXAMPLES_WINGDEMO_FRAME_DELAY_US 33000
#endif

#define WINGDEMO_DRAW_ROWS 32

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct wingdemo_fb_s
{
  int fd;
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
  FAR uint8_t *fbmem;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static FAR struct wingdemo_fb_s *g_flush_fb;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static lv_color_format_t wingdemo_fb_color_format(uint8_t fmt)
{
  switch (fmt)
    {
      case FB_FMT_RGB32:
      case FB_FMT_RGBT32:
        return LV_COLOR_FORMAT_XRGB8888;

      case FB_FMT_RGBA32:
        return LV_COLOR_FORMAT_ARGB8888;

      case FB_FMT_RGB24:
        return LV_COLOR_FORMAT_RGB888;

      case FB_FMT_RGB16_565:
        return LV_COLOR_FORMAT_RGB565;

      default:
        return LV_COLOR_FORMAT_UNKNOWN;
    }
}

static bool wingdemo_supported_fb_format(uint8_t fmt)
{
  return wingdemo_fb_color_format(fmt) != LV_COLOR_FORMAT_UNKNOWN;
}

static bool wingdemo_is_default_fb(FAR const char *path)
{
  return strcmp(path, CONFIG_EXAMPLES_WINGDEMO_FBDEV) == 0 ||
         strcmp(path, "/dev/fb0") == 0;
}

static int wingdemo_lazy_register_fb(FAR const char *path)
{
  int ret;

  if (!wingdemo_is_default_fb(path))
    {
      return -ENOENT;
    }

  ret = fb_register(0, 0);
  if (ret < 0)
    {
      fprintf(stderr, "wingdemo: failed to register framebuffer: %d\n",
              -ret);
    }

  return ret;
}

static int wingdemo_open_fb(FAR struct wingdemo_fb_s *fb,
                            FAR const char *path)
{
  FAR void *mapped;
  int ret;

  memset(fb, 0, sizeof(*fb));
  fb->fd = -1;

  fb->fd = open(path, O_RDWR);
  if (fb->fd < 0)
    {
      ret = -errno;
      if (ret == -ENOENT)
        {
          ret = wingdemo_lazy_register_fb(path);
          if (ret >= 0)
            {
              fb->fd = open(path, O_RDWR);
            }
        }

      if (fb->fd < 0)
        {
          if (ret >= 0)
            {
              ret = -errno;
            }

          fprintf(stderr, "wingdemo: failed to open %s: %d\n", path, -ret);
          return ret;
        }
    }

  ret = ioctl(fb->fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&fb->vinfo));
  if (ret < 0)
    {
      fprintf(stderr, "wingdemo: FBIOGET_VIDEOINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  ret = ioctl(fb->fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&fb->pinfo));
  if (ret < 0)
    {
      fprintf(stderr, "wingdemo: FBIOGET_PLANEINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  if (fb->vinfo.nplanes < 1 ||
      !wingdemo_supported_fb_format(fb->vinfo.fmt))
    {
      fprintf(stderr, "wingdemo: unsupported framebuffer format: %u\n",
              fb->vinfo.fmt);
      ret = -ENOSYS;
      goto errout_close;
    }

  mapped = mmap(NULL, fb->pinfo.fblen, PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FILE, fb->fd, 0);
  if (mapped == MAP_FAILED)
    {
      fprintf(stderr, "wingdemo: mmap failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  fb->fbmem = (FAR uint8_t *)mapped;

  printf("wingdemo: framebuffer %ux%u fmt=%u bpp=%u stride=%u\n",
         fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.fmt, fb->pinfo.bpp,
         fb->pinfo.stride);
  return 0;

errout_close:
  close(fb->fd);
  fb->fd = -1;
  return ret;
}

static void wingdemo_close_fb(FAR struct wingdemo_fb_s *fb)
{
  if (fb->fbmem != NULL)
    {
      munmap(fb->fbmem, fb->pinfo.fblen);
      fb->fbmem = NULL;
    }

  if (fb->fd >= 0)
    {
      close(fb->fd);
      fb->fd = -1;
    }
}

static void wingdemo_flush_cb(FAR lv_display_t *disp,
                              FAR const lv_area_t *area,
                              FAR uint8_t *px_map)
{
  FAR struct wingdemo_fb_s *fb = g_flush_fb;
  uint8_t px_size;
  uint32_t width;
  uint32_t rows;
  uint32_t row;

  if (fb == NULL || area == NULL || px_map == NULL)
    {
      lv_display_flush_ready(disp);
      return;
    }

  px_size = lv_color_format_get_size(lv_display_get_color_format(disp));
  if (px_size == 0)
    {
      lv_display_flush_ready(disp);
      return;
    }

  width = (uint32_t)(area->x2 - area->x1 + 1);
  rows = (uint32_t)(area->y2 - area->y1 + 1);

  for (row = 0; row < rows; row++)
    {
      FAR const uint8_t *src = px_map + row * width * px_size;
      FAR uint8_t *dst = fb->fbmem +
        ((uint32_t)area->y1 + row) * fb->pinfo.stride +
        (uint32_t)area->x1 * px_size;

      memcpy(dst, src, width * px_size);
    }

#ifdef CONFIG_FB_UPDATE
  {
    struct fb_area_s update =
      {
        (uint32_t)area->x1,
        (uint32_t)area->y1,
        width,
        rows
      };

    ioctl(fb->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)&update));
  }
#endif

  lv_display_flush_ready(disp);
}

static bool wingdemo_parse_u32(FAR const char *text, FAR uint32_t *value)
{
  FAR char *endptr;
  unsigned long parsed;

  errno = 0;
  parsed = strtoul(text, &endptr, 10);
  if (errno != 0 || endptr == text || *endptr != '\0' ||
      parsed > UINT32_MAX)
    {
      return false;
    }

  *value = (uint32_t)parsed;
  return true;
}

static uint32_t wingdemo_parse_frames(int argc, FAR char *argv[])
{
  int i;
  uint32_t frames;

  for (i = 1; i < argc; i++)
    {
      if (strncmp(argv[i], "/dev/", 5) == 0)
        {
          continue;
        }

      if (strcmp(argv[i], "--frames") == 0 || strcmp(argv[i], "-f") == 0)
        {
          if (i + 1 >= argc ||
              !wingdemo_parse_u32(argv[i + 1], &frames))
            {
              fprintf(stderr,
                      "wingdemo: expected a non-negative frame count after "
                      "%s\n",
                      argv[i]);
              return CONFIG_EXAMPLES_WINGDEMO_FRAMES;
            }

          return frames;
        }

      if (strncmp(argv[i], "--frames=", 9) == 0)
        {
          if (!wingdemo_parse_u32(argv[i] + 9, &frames))
            {
              fprintf(stderr,
                      "wingdemo: invalid frame count: %s\n", argv[i] + 9);
              return CONFIG_EXAMPLES_WINGDEMO_FRAMES;
            }

          return frames;
        }

      if (wingdemo_parse_u32(argv[i], &frames))
        {
          return frames;
        }
    }

  return CONFIG_EXAMPLES_WINGDEMO_FRAMES;
}

static FAR const char *wingdemo_parse_fbdev(int argc, FAR char *argv[])
{
  int i;

  for (i = 1; i < argc; i++)
    {
      if (strncmp(argv[i], "/dev/", 5) == 0)
        {
          return argv[i];
        }
    }

  return CONFIG_EXAMPLES_WINGDEMO_FBDEV;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *fbdev = wingdemo_parse_fbdev(argc, argv);
  uint32_t frames = wingdemo_parse_frames(argc, argv);
  struct wingdemo_fb_s fb;
  FAR void *drawbuf = NULL;
  lv_display_t *display;
  lv_color_format_t color_format;
  uint32_t px_size;
  uint32_t drawbuf_size;
  uint32_t frame = 0;
  int ret;

  ret = wingdemo_open_fb(&fb, fbdev);
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  color_format = wingdemo_fb_color_format(fb.vinfo.fmt);
  px_size = lv_color_format_get_size(color_format);
  drawbuf_size = fb.vinfo.xres * px_size * WINGDEMO_DRAW_ROWS;

  drawbuf = malloc(drawbuf_size);
  if (drawbuf == NULL)
    {
      fprintf(stderr, "wingdemo: failed to allocate draw buffer\n");
      wingdemo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  lv_init();
  display = lv_display_create(fb.vinfo.xres, fb.vinfo.yres);
  if (display == NULL)
    {
      fprintf(stderr, "wingdemo: failed to create LVGL display facade\n");
      free(drawbuf);
      wingdemo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  g_flush_fb = &fb;
  lv_display_set_default(display);
  lv_display_set_color_format(display, color_format);
  lv_display_set_buffers(display, drawbuf, NULL, drawbuf_size,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, wingdemo_flush_cb);
  lv_demo_widgets();

  printf("wingdemo: upstream LVGL widget demo through Wing; frames=%lu\n",
         (unsigned long)frames);

  while (frames == 0 || frame < frames)
    {
      lv_refr_now(display);
      lv_timer_handler();
      lv_tick_inc(CONFIG_EXAMPLES_WINGDEMO_FRAME_DELAY_US / 1000);
      frame++;
      usleep(CONFIG_EXAMPLES_WINGDEMO_FRAME_DELAY_US);
    }

  if (frames > 0)
    {
      printf("wingdemo: rendered %lu frames; stopping simulator\n",
             (unsigned long)frame);
#ifdef CONFIG_BOARDCTL_POWEROFF
      boardctl(BOARDIOC_POWEROFF, 0);
#endif
    }

  lv_deinit();
  g_flush_fb = NULL;
  free(drawbuf);
  wingdemo_close_fb(&fb);
  return EXIT_SUCCESS;
}
