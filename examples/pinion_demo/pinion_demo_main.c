/****************************************************************************
 * apps/examples/pinion_demo/pinion_demo_main.c
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
#include <nuttx/video/rgbcolors.h>

#include <pinion/pinion.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_PINION_DEMO_FBDEV
#  define CONFIG_EXAMPLES_PINION_DEMO_FBDEV "/dev/fb0"
#endif

#ifndef CONFIG_EXAMPLES_PINION_DEMO_FRAMES
#  define CONFIG_EXAMPLES_PINION_DEMO_FRAMES 0
#endif

#ifndef CONFIG_EXAMPLES_PINION_DEMO_FRAME_DELAY_US
#  define CONFIG_EXAMPLES_PINION_DEMO_FRAME_DELAY_US 33000
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct pinion_demo_fb_s
{
  int fd;
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
  uint8_t *fbmem;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool pinion_demo_supported_fb_format(uint8_t fmt)
{
  return fmt == FB_FMT_RGB32 ||
         fmt == FB_FMT_RGB24 ||
         fmt == FB_FMT_RGB16_565 ||
         fmt == FB_FMT_RGBA32 ||
         fmt == FB_FMT_RGBT32;
}

static int pinion_demo_open_fb(struct pinion_demo_fb_s *fb,
                               const char *path)
{
  void *mapped;
  int ret;

  memset(fb, 0, sizeof(*fb));
  fb->fd = open(path, O_RDWR);
  if (fb->fd < 0)
    {
      fprintf(stderr, "piniondemo: failed to open %s: %d\n", path, errno);
      return -errno;
    }

  ret = ioctl(fb->fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&fb->vinfo));
  if (ret < 0)
    {
      fprintf(stderr, "piniondemo: FBIOGET_VIDEOINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  ret = ioctl(fb->fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&fb->pinfo));
  if (ret < 0)
    {
      fprintf(stderr, "piniondemo: FBIOGET_PLANEINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  if (fb->vinfo.nplanes < 1 ||
      !pinion_demo_supported_fb_format(fb->vinfo.fmt))
    {
      fprintf(stderr, "piniondemo: unsupported framebuffer format: %u\n",
              fb->vinfo.fmt);
      ret = -ENOSYS;
      goto errout_close;
    }

  mapped = mmap(NULL, fb->pinfo.fblen, PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FILE, fb->fd, 0);
  if (mapped == MAP_FAILED)
    {
      fprintf(stderr, "piniondemo: mmap failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  fb->fbmem = mapped;

  printf("piniondemo: framebuffer %ux%u fmt=%u bpp=%u stride=%u\n",
         fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.fmt, fb->pinfo.bpp,
         fb->pinfo.stride);
  return 0;

errout_close:
  close(fb->fd);
  fb->fd = -1;
  return ret;
}

static void pinion_demo_close_fb(struct pinion_demo_fb_s *fb)
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

static void pinion_demo_copy_rgba_to_fb(const uint8_t *src,
                                        const struct pinion_demo_fb_s *fb)
{
  uint32_t width = fb->vinfo.xres;
  uint32_t height = fb->vinfo.yres;
  uint32_t x;
  uint32_t y;

  for (y = 0; y < height; y++)
    {
      const uint8_t *srow = src + y * width * 4u;
      uint8_t *drow = fb->fbmem + y * fb->pinfo.stride;

      for (x = 0; x < width; x++)
        {
          uint8_t r = srow[x * 4u + 0];
          uint8_t g = srow[x * 4u + 1];
          uint8_t b = srow[x * 4u + 2];
          uint8_t a = srow[x * 4u + 3];

          switch (fb->vinfo.fmt)
            {
              case FB_FMT_RGB32:
              case FB_FMT_RGBA32:
              case FB_FMT_RGBT32:
                ((uint32_t *)drow)[x] = ARGBTO32(a, r, g, b);
                break;

              case FB_FMT_RGB24:
                drow[x * 3u + 0] = b;
                drow[x * 3u + 1] = g;
                drow[x * 3u + 2] = r;
                break;

              case FB_FMT_RGB16_565:
                ((uint16_t *)drow)[x] = RGBTO16(r, g, b);
                break;

              default:
                break;
            }
        }
    }
}

static void pinion_demo_render(pinion_surface_t *surface, uint32_t frame)
{
  int32_t w = (int32_t)surface->width;
  int32_t h = (int32_t)surface->height;
  int32_t hero_size = w < h ? w / 10 : h / 10;
  int32_t hero_x = (int32_t)((frame * 5u) % (uint32_t)(w + hero_size)) -
                   hero_size;
  int32_t hero_y = h / 2 - hero_size / 2;
  int32_t orb_x = w / 2 + (int32_t)((frame * 3u) % 160u) - 80;

  if (hero_size < 12)
    {
      hero_size = 12;
    }

  pinion_surface_clear(surface, pinion_rgba(10, 14, 22, 255));
  pinion_draw_rect(surface, 0, h - h / 5, w, h / 5,
                   pinion_rgba(24, 66, 62, 255));
  pinion_draw_rect(surface, 0, h - h / 5 - 8, w, 8,
                   pinion_rgba(238, 184, 82, 255));

  pinion_draw_circle(surface, orb_x, h / 3, h / 9,
                     pinion_rgba(67, 181, 212, 255));
  pinion_draw_rect(surface, hero_x, hero_y, hero_size, hero_size,
                   pinion_rgba(236, 92, 76, 255));
  pinion_draw_rect(surface, hero_x + hero_size / 4,
                   hero_y - hero_size / 2,
                   hero_size / 2, hero_size / 2,
                   pinion_rgba(250, 220, 130, 255));
}

static const char *pinion_demo_arg_value(int argc, char *argv[], int *arg)
{
  const char *value;

  value = strchr(argv[*arg], '=');
  if (value != NULL)
    {
      return value + 1;
    }

  if (*arg + 1 >= argc)
    {
      return NULL;
    }

  (*arg)++;
  return argv[*arg];
}

static uint32_t pinion_demo_parse_u32(const char *text, uint32_t fallback)
{
  char *endptr;
  unsigned long value;

  if (text == NULL)
    {
      return fallback;
    }

  value = strtoul(text, &endptr, 10);
  if (endptr == text)
    {
      return fallback;
    }

  return (uint32_t)value;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  struct pinion_demo_fb_s fb;
  pinion_engine_t engine;
  pinion_surface_t surface;
  uint8_t *pixels;
  const char *fbdev = CONFIG_EXAMPLES_PINION_DEMO_FBDEV;
  uint32_t frames = CONFIG_EXAMPLES_PINION_DEMO_FRAMES;
  uint32_t delay_us = CONFIG_EXAMPLES_PINION_DEMO_FRAME_DELAY_US;
  uint32_t frame;
  size_t pixel_size;
  int ret;
  int i;

  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--fb") == 0 ||
          strncmp(argv[i], "--fb=", 5) == 0)
        {
          fbdev = pinion_demo_arg_value(argc, argv, &i);
        }
      else if (strcmp(argv[i], "-f") == 0 ||
               strcmp(argv[i], "--frames") == 0 ||
               strncmp(argv[i], "--frames=", 9) == 0)
        {
          frames = pinion_demo_parse_u32(
            pinion_demo_arg_value(argc, argv, &i), frames);
        }
      else if (strcmp(argv[i], "--delay-us") == 0 ||
               strncmp(argv[i], "--delay-us=", 11) == 0)
        {
          delay_us = pinion_demo_parse_u32(
            pinion_demo_arg_value(argc, argv, &i), delay_us);
        }
    }

  ret = pinion_demo_open_fb(&fb, fbdev);
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  pixel_size = (size_t)fb.vinfo.xres * fb.vinfo.yres * 4u;
  pixels = malloc(pixel_size);
  if (pixels == NULL)
    {
      fprintf(stderr, "piniondemo: failed to allocate frame surface\n");
      pinion_demo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  ret = pinion_engine_init(&engine, 30);
  if (ret < 0)
    {
      fprintf(stderr, "piniondemo: failed to initialize engine: %d\n", -ret);
      free(pixels);
      pinion_demo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  ret = pinion_surface_init(&surface, fb.vinfo.xres, fb.vinfo.yres,
                            fb.vinfo.xres * 4u, pixels);
  if (ret < 0)
    {
      fprintf(stderr, "piniondemo: failed to initialize surface: %d\n",
              -ret);
      free(pixels);
      pinion_demo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  frame = 0;
  while (frames == 0 || frame < frames)
    {
      pinion_engine_begin_frame(&engine);
      pinion_demo_render(&surface, engine.frame);
      pinion_demo_copy_rgba_to_fb(pixels, &fb);

#ifdef CONFIG_FB_UPDATE
      {
        struct fb_area_s area;

        area.x = 0;
        area.y = 0;
        area.w = fb.vinfo.xres;
        area.h = fb.vinfo.yres;
        ioctl(fb.fd, FBIO_UPDATE,
              (unsigned long)((uintptr_t)&area));
      }
#endif

      pinion_engine_end_frame(&engine);
      frame++;

      if (delay_us > 0)
        {
          usleep(delay_us);
        }
    }

  free(pixels);
  pinion_demo_close_fb(&fb);
  return EXIT_SUCCESS;
}
