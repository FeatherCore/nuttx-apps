/****************************************************************************
 * apps/examples/fgfx_demo/fgfx_demo_main.c
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

#include <fgfx/fgfx.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_FGFX_DEMO_FBDEV
#  define CONFIG_EXAMPLES_FGFX_DEMO_FBDEV "/dev/fb0"
#endif

#ifndef CONFIG_EXAMPLES_FGFX_DEMO_FRAMES
#  define CONFIG_EXAMPLES_FGFX_DEMO_FRAMES 0
#endif

#ifndef CONFIG_EXAMPLES_FGFX_DEMO_FRAME_DELAY_US
#  define CONFIG_EXAMPLES_FGFX_DEMO_FRAME_DELAY_US 33000
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fgfxdemo_fb_s
{
  int fd;
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
  FAR uint8_t *fbmem;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static fgfx_color_t fgfxdemo_rgba(uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t a)
{
  fgfx_color_t color =
    {
      r, g, b, a
    };

  return color;
}

static bool fgfxdemo_supported_fb_format(uint8_t fmt)
{
  return fmt == FB_FMT_RGB32 ||
         fmt == FB_FMT_RGB24 ||
         fmt == FB_FMT_RGB16_565 ||
         fmt == FB_FMT_RGBA32 ||
         fmt == FB_FMT_RGBT32;
}

static int fgfxdemo_open_fb(FAR struct fgfxdemo_fb_s *fb,
                            FAR const char *path)
{
  FAR void *mapped;
  int ret;

  memset(fb, 0, sizeof(*fb));
  fb->fd = open(path, O_RDWR);
  if (fb->fd < 0)
    {
      fprintf(stderr, "fgfxdemo: failed to open %s: %d\n", path, errno);
      return -errno;
    }

  ret = ioctl(fb->fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&fb->vinfo));
  if (ret < 0)
    {
      fprintf(stderr, "fgfxdemo: FBIOGET_VIDEOINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  ret = ioctl(fb->fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&fb->pinfo));
  if (ret < 0)
    {
      fprintf(stderr, "fgfxdemo: FBIOGET_PLANEINFO failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  if (fb->vinfo.nplanes < 1 || !fgfxdemo_supported_fb_format(fb->vinfo.fmt))
    {
      fprintf(stderr, "fgfxdemo: unsupported framebuffer format: %u\n",
              fb->vinfo.fmt);
      ret = -ENOSYS;
      goto errout_close;
    }

  mapped = mmap(NULL, fb->pinfo.fblen, PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FILE, fb->fd, 0);
  if (mapped == MAP_FAILED)
    {
      fprintf(stderr, "fgfxdemo: mmap failed: %d\n", errno);
      ret = -errno;
      goto errout_close;
    }

  fb->fbmem = mapped;

  printf("fgfxdemo: framebuffer %ux%u fmt=%u bpp=%u stride=%u\n",
         fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.fmt, fb->pinfo.bpp,
         fb->pinfo.stride);
  return 0;

errout_close:
  close(fb->fd);
  fb->fd = -1;
  return ret;
}

static void fgfxdemo_close_fb(FAR struct fgfxdemo_fb_s *fb)
{
  if (fb->fbmem != NULL)
    {
      munmap(fb->fbmem, fb->pinfo.fblen);
    }

  if (fb->fd >= 0)
    {
      close(fb->fd);
    }
}

static void fgfxdemo_copy_rgba_to_fb(FAR const uint8_t *src,
                                     FAR const struct fgfxdemo_fb_s *fb)
{
  uint32_t width = fb->vinfo.xres;
  uint32_t height = fb->vinfo.yres;
  uint32_t x;
  uint32_t y;

  for (y = 0; y < height; y++)
    {
      FAR const uint8_t *srow = src + y * width * 4u;
      FAR uint8_t *drow = fb->fbmem + y * fb->pinfo.stride;

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
                ((FAR uint32_t *)drow)[x] = ARGBTO32(a, r, g, b);
                break;

              case FB_FMT_RGB24:
                drow[x * 3u + 0] = b;
                drow[x * 3u + 1] = g;
                drow[x * 3u + 2] = r;
                break;

              case FB_FMT_RGB16_565:
                ((FAR uint16_t *)drow)[x] = RGBTO16(r, g, b);
                break;

              default:
                break;
            }
        }
    }
}

static int fgfxdemo_render_frame(FAR uint8_t *pixels, FAR float *depth,
                                 uint32_t width, uint32_t height,
                                 uint32_t frame)
{
  fgfx_surface_t target;
  fgfx_display_list_t list;
  fgfx_context_t ctx;
  fgfx_vertex_t vertices[3];
  fgfx_point_t path[5];
  int32_t w = (int32_t)width;
  int32_t h = (int32_t)height;
  int32_t band_w = w / 4;
  int32_t band_h = h / 6;
  int32_t band_x = (int32_t)((frame * 7u) % (uint32_t)(w + band_w)) -
                   band_w;
  int32_t band_y = h / 3;
  size_t i;
  int ret;

  memset(pixels, 0, width * height * 4u);
  for (i = 0; i < (size_t)width * height; i++)
    {
      depth[i] = 1.0f;
    }

  memset(&target, 0, sizeof(target));
  target.width = width;
  target.height = height;
  target.stride = width * 4u;
  target.format = FGFX_FORMAT_RGBA8888;
  target.pixels = pixels;
  target.depth = depth;
  target.depth_stride = width;

  ret = fgfx_display_list_init(&list, 16);
  if (ret != FGFX_OK)
    {
      return ret;
    }

  ret = fgfx_dl_gradient_rect(&list, (fgfx_rect_t){0, 0, w, h},
                              fgfxdemo_rgba(22, 28, 36, 255),
                              fgfxdemo_rgba(8, 92, 118, 255), true,
                              FGFX_BLEND_REPLACE);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  ret = fgfx_dl_fill_rect(&list,
                          (fgfx_rect_t){band_x, band_y, band_w, band_h},
                          fgfxdemo_rgba(245, 180, 45, 210),
                          FGFX_BLEND_SRC_OVER);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  ret = fgfx_dl_stroke_rect(&list,
                            (fgfx_rect_t){w / 12, h / 10,
                                          w - w / 6, h - h / 5},
                            4, fgfxdemo_rgba(230, 244, 248, 230),
                            FGFX_BLEND_SRC_OVER);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  path[0] = (fgfx_point_t){w / 2, h / 5};
  path[1] = (fgfx_point_t){w * 7 / 10, h / 2};
  path[2] = (fgfx_point_t){w * 3 / 5, h * 4 / 5};
  path[3] = (fgfx_point_t){w * 2 / 5, h * 4 / 5};
  path[4] = (fgfx_point_t){w * 3 / 10, h / 2};
  ret = fgfx_dl_fill_path(&list, path, 5,
                          fgfxdemo_rgba(34, 190, 126, 180),
                          FGFX_BLEND_SRC_OVER);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  vertices[0] = (fgfx_vertex_t){-0.72f, -0.52f, 0.25f, 1.0f,
                                0.0f, 0.0f,
                                fgfxdemo_rgba(255, 70, 80, 255)};
  vertices[1] = (fgfx_vertex_t){0.72f, -0.48f, 0.25f, 1.0f,
                                1.0f, 0.0f,
                                fgfxdemo_rgba(78, 210, 255, 255)};
  vertices[2] = (fgfx_vertex_t){0.0f, 0.72f, 0.18f, 1.0f,
                                0.5f, 1.0f,
                                fgfxdemo_rgba(255, 238, 95, 255)};
  ret = fgfx_dl_draw_vertices(&list, FGFX_PRIMITIVE_TRIANGLES,
                              vertices, 3, NULL, 0, NULL, NULL,
                              FGFX_CULL_NONE, FGFX_DEPTH_LESS, true,
                              FGFX_BLEND_SRC_OVER);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  ret = fgfx_dl_line(&list, 0, h - h / 8, w, h / 8,
                     fgfxdemo_rgba(255, 255, 255, 190),
                     FGFX_BLEND_SRC_OVER);
  if (ret != FGFX_OK)
    {
      goto out;
    }

  fgfx_context_init(&ctx);
  ret = fgfx_context_render(&ctx, &list, &target);

out:
  fgfx_display_list_deinit(&list);
  return ret;
}

static int fgfxdemo_check_frame_size(uint32_t width, uint32_t height)
{
  size_t pixels;

  if (width == 0 || height == 0 ||
      width > SIZE_MAX / height)
    {
      return -EOVERFLOW;
    }

  pixels = (size_t)width * height;
  if (pixels > SIZE_MAX / 4u || pixels > SIZE_MAX / sizeof(float))
    {
      return -EOVERFLOW;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *fbdev = CONFIG_EXAMPLES_FGFX_DEMO_FBDEV;
  struct fgfxdemo_fb_s fb;
  FAR uint8_t *pixels;
  FAR float *depth;
  uint32_t frame = 0;
  int ret;

  if (argc > 1 && strncmp(argv[1], "/dev/", 5) == 0)
    {
      fbdev = argv[1];
    }

  ret = fgfxdemo_open_fb(&fb, fbdev);
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  ret = fgfxdemo_check_frame_size(fb.vinfo.xres, fb.vinfo.yres);
  if (ret < 0)
    {
      fprintf(stderr, "fgfxdemo: invalid framebuffer dimensions\n");
      fgfxdemo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  pixels = malloc((size_t)fb.vinfo.xres * fb.vinfo.yres * 4u);
  depth = malloc((size_t)fb.vinfo.xres * fb.vinfo.yres * sizeof(float));
  if (pixels == NULL || depth == NULL)
    {
      fprintf(stderr, "fgfxdemo: out of memory\n");
      free(depth);
      free(pixels);
      fgfxdemo_close_fb(&fb);
      return EXIT_FAILURE;
    }

  printf("fgfxdemo: rendering FGFX demo; press Ctrl-C to stop simulator\n");

  while (CONFIG_EXAMPLES_FGFX_DEMO_FRAMES == 0 ||
         frame < CONFIG_EXAMPLES_FGFX_DEMO_FRAMES)
    {
      ret = fgfxdemo_render_frame(pixels, depth, fb.vinfo.xres,
                                  fb.vinfo.yres, frame);
      if (ret != FGFX_OK)
        {
          fprintf(stderr, "fgfxdemo: render failed: %d\n", ret);
          break;
        }

      fgfxdemo_copy_rgba_to_fb(pixels, &fb);

#ifdef CONFIG_FB_UPDATE
      {
        struct fb_area_s area =
          {
            0, 0, fb.vinfo.xres, fb.vinfo.yres
          };

        ioctl(fb.fd, FBIO_UPDATE, (unsigned long)((uintptr_t)&area));
      }
#endif

      frame++;
      usleep(CONFIG_EXAMPLES_FGFX_DEMO_FRAME_DELAY_US);
    }

  free(depth);
  free(pixels);
  fgfxdemo_close_fb(&fb);
  return ret == FGFX_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
