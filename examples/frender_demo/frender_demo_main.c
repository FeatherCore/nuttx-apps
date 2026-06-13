/****************************************************************************
 * apps/examples/frender_demo/frender_demo_main.c
 *
 * Minimal demo: builds a command list and dispatches through the
 * framebuffer backend. All rendering is done by nuttx/graphics (fb_draw_*).
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <frender/frender.h>

static void print_caps(const char *label, const fr_backend_caps_t *caps)
{
  printf("frender_demo: %s draw=0x%08x present=0x%08x format=0x%08x\n",
         label, (unsigned int)caps->draw_caps,
         (unsigned int)caps->present_caps,
         (unsigned int)caps->format_caps);
}

static int run_stage(const char *stage, fr_backend_instance_t *backend,
                     const fr_command_list_t *list)
{
  int ret = fr_execute(backend, list);
  if (ret < 0)
    printf("frender_demo: stage=%s failed: %d\n", stage, ret);
  else
    printf("frender_demo: stage=%s commands=%u ok\n",
           stage, (unsigned int)fr_command_list_count(list));
  return ret;
}

int main(int argc, FAR char *argv[])
{
  fr_command_t commands[16];
  fr_command_list_t list;
  fr_backend_instance_t fb;
  fr_backend_caps_t caps;
  fr_surface_t image;
  fr_quad_t quad;
  fr_triangle_t tri;
  fr_rect_t r;
  int ret;

  /* Framebuffer backend — delegates all drawing to nuttx/graphics */

  {
    struct { const char *fb_path; } cfg = { "/dev/fb0" };
    ret = fr_backend_open(&fb, "framebuffer", &cfg);
    printf("frender_demo: fb backend open: %d\n", ret);
    if (ret < 0)
      {
        printf("frender_demo: headless mode (no framebuffer)\n");
        return EXIT_SUCCESS;
      }
  }

  /* Image source for blit test */

  static uint32_t img_pixels[16] = {
    0xffffffff, 0xff0000ff, 0xff0000ff, 0xffffffff,
    0xff0000ff, 0xffffffff, 0xffffffff, 0xff0000ff,
    0xff0000ff, 0xffffffff, 0xffffffff, 0xff0000ff,
    0xffffffff, 0xff0000ff, 0xff0000ff, 0xffffffff,
  };
  fr_surface_init(&image, img_pixels, 4, 4, 4, FR_FORMAT_RGBA8888);

  /* Capabilities */

  caps = fr_backend_caps_nuttx_graphics();
  print_caps("nutsx-graphics", &caps);

  /* Build and execute command stages */

  fr_command_list_init(&list, commands, 16);

  fr_cmd_clear(&list, fr_color_rgba(19, 25, 31, 255));
  run_stage("clear", &fb, &list);

  r.x = 40; r.y = 40; r.w = 120; r.h = 80;
  fr_cmd_fill_rect(&list, &r, fr_color_rgba(200, 60, 60, 255));
  run_stage("fill_rect", &fb, &list);

  r.x = 180; r.y = 40; r.w = 100; r.h = 80;
  fr_cmd_stroke_rect(&list, &r, 3, fr_color_rgba(60, 200, 60, 255));
  run_stage("stroke_rect", &fb, &list);

  quad.points[0].x = 40;  quad.points[0].y = 160;
  quad.points[1].x = 160; quad.points[1].y = 140;
  quad.points[2].x = 180; quad.points[2].y = 220;
  quad.points[3].x = 30;  quad.points[3].y = 230;
  fr_cmd_fill_quad(&list, &quad, fr_color_rgba(60, 60, 200, 160));
  run_stage("fill_quad", &fb, &list);

  tri.points[0].x = 220; tri.points[0].y = 160;
  tri.points[1].x = 300; tri.points[1].y = 140;
  tri.points[2].x = 260; tri.points[2].y = 220;
  fr_cmd_fill_triangle(&list, &tri, fr_color_rgba(200, 200, 60, 255));
  run_stage("fill_triangle", &fb, &list);

  quad.points[0].x = 10;  quad.points[0].y = 160;
  quad.points[1].x = 15;  quad.points[1].y = 140;
  quad.points[2].x = 20;  quad.points[2].y = 220;
  quad.points[3].x = 5;   quad.points[3].y = 230;
  fr_cmd_stroke_quad(&list, &quad, 2, fr_color_rgba(200, 100, 50, 255));
  run_stage("stroke_quad", &fb, &list);

  {
    fr_rect_t sr = {0, 0, 4, 4};
    fr_rect_t dr = {250, 40, 50, 50};
    fr_cmd_blit_alpha(&list, &image, &sr, &dr, 200);
    run_stage("blit", &fb, &list);
  }

  fr_cmd_push_clip(&list, &(fr_rect_t){60, 60, 80, 80});
  fr_cmd_fill_rect(&list, &(fr_rect_t){0, 0, 320, 240},
                   fr_color_rgba(255, 255, 255, 128));
  fr_cmd_pop_clip(&list);
  run_stage("clip_fill", &fb, &list);

  printf("frender_demo: done\n");

  fr_backend_close(&fb);
  return EXIT_SUCCESS;
}
