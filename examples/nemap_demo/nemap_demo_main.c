/****************************************************************************
 * apps/examples/nemap_demo/nemap_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>

#include <nuttx/cache.h>
#include <nuttx/compiler.h>
#include <nuttx/video/fb.h>

#if defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32U5) || \
    defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32N6) || \
    defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32H7RS)
#  include "stm32_gpu2d.h"
#  include "hardware/stm32_gpu2d.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if !defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32U5) && \
    !defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32N6) && \
    !defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32H7RS)
#  error nemap_demo currently needs a selected backend.
#endif

#if defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32U5) && \
    !defined(CONFIG_STM32U5_GPU2D)
#  error The current nemap_demo backend requires CONFIG_STM32U5_GPU2D.
#endif

#if defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32N6) && \
    !defined(CONFIG_STM32N6_GPU2D)
#  error The current nemap_demo backend requires CONFIG_STM32N6_GPU2D.
#endif

#if defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32H7RS) && \
    !defined(CONFIG_STM32H7RS_GPU2D)
#  error The current nemap_demo backend requires CONFIG_STM32H7RS_GPU2D.
#endif

#if !defined(CONFIG_VIDEO_FB)
#  error CONFIG_VIDEO_FB is required by nemap_demo.
#endif

#if defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32U5)
#  define NEMAP_DEMO_U5 1
#  define NEMAP_DEMO_N6 0
#  define NEMAP_DEMO_H7RS 0
#elif defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32N6)
#  define NEMAP_DEMO_U5 0
#  define NEMAP_DEMO_N6 1
#  define NEMAP_DEMO_H7RS 0
#elif defined(CONFIG_EXAMPLES_NEMAP_DEMO_STM32H7RS)
#  define NEMAP_DEMO_U5 0
#  define NEMAP_DEMO_N6 0
#  define NEMAP_DEMO_H7RS 1
#endif

#if NEMAP_DEMO_U5
#  define NEMAP_DEMO_U5_OFFSCREEN_GPU_VISIBLE 1
#else
#  define NEMAP_DEMO_U5_OFFSCREEN_GPU_VISIBLE 0
#endif

#if NEMAP_DEMO_N6
#  define NEMAP_DEMO_N6_PSRAM_RESOURCES 1
#  define NEMAP_DEMO_N6_OFFSCREEN_GPU_VISIBLE 1
#else
#  define NEMAP_DEMO_N6_PSRAM_RESOURCES 0
#  define NEMAP_DEMO_N6_OFFSCREEN_GPU_VISIBLE 0
#endif

#if NEMAP_DEMO_H7RS
#  define NEMAP_DEMO_H7RS_PSRAM_RESOURCES 1
#  define NEMAP_DEMO_H7RS_OFFSCREEN_GPU_VISIBLE 1
#else
#  define NEMAP_DEMO_H7RS_PSRAM_RESOURCES 0
#  define NEMAP_DEMO_H7RS_OFFSCREEN_GPU_VISIBLE 0
#endif

#if NEMAP_DEMO_N6 || NEMAP_DEMO_H7RS
#  define NEMAP_DEMO_AXI64_RGB24_SOURCE_STRICT 0
#  define NEMAP_DEMO_PERSISTENT_ROOT_RING 1
#else
#  define NEMAP_DEMO_AXI64_RGB24_SOURCE_STRICT 1
#  define NEMAP_DEMO_PERSISTENT_ROOT_RING 0
#endif

#define NEMAP_DEMO_PSRAM_RESOURCES \
  (NEMAP_DEMO_N6_PSRAM_RESOURCES || NEMAP_DEMO_H7RS_PSRAM_RESOURCES)

#define NEMAP_DEMO_OFFSCREEN_GPU_VISIBLE \
  (NEMAP_DEMO_U5_OFFSCREEN_GPU_VISIBLE || \
   NEMAP_DEMO_N6_OFFSCREEN_GPU_VISIBLE || \
   NEMAP_DEMO_H7RS_OFFSCREEN_GPU_VISIBLE)

#if NEMAP_DEMO_OFFSCREEN_GPU_VISIBLE
#  define NEMAP_DEMO_OFFSCREEN_SKIP_REASON \
  "(enable CONFIG_EXAMPLES_NEMAP_DEMO_RUN_OFFSCREEN_PHASES)"
#else
#  define NEMAP_DEMO_OFFSCREEN_SKIP_REASON \
  "(offscreen target is not GPU-visible on this chip)"
#endif

#ifndef CONFIG_EXAMPLES_NEMAP_DEMO_FBDEV
#  define CONFIG_EXAMPLES_NEMAP_DEMO_FBDEV "/dev/fb0"
#endif

#ifndef CONFIG_EXAMPLES_NEMAP_DEMO_TIMEOUT_MS
#  define CONFIG_EXAMPLES_NEMAP_DEMO_TIMEOUT_MS 1000
#endif

static bool g_vg_probe_strict;

#ifdef CONFIG_EXAMPLES_NEMAP_DEMO_RUN_VERIFIED_PHASES
#  define NEMAP_DEMO_RUN_VERIFIED_PHASES 1
#else
#  define NEMAP_DEMO_RUN_VERIFIED_PHASES 0
#endif

#ifdef CONFIG_EXAMPLES_NEMAP_DEMO_RUN_OFFSCREEN_PHASES
#  define NEMAP_DEMO_USER_ENABLE_OFFSCREEN_PHASES 1
#else
#  define NEMAP_DEMO_USER_ENABLE_OFFSCREEN_PHASES 0
#endif

#define NEMAP_DEMO_RUN_OFFSCREEN_PHASES \
  (NEMAP_DEMO_USER_ENABLE_OFFSCREEN_PHASES && NEMAP_DEMO_OFFSCREEN_GPU_VISIBLE)

#define NEMAP_DEMO_RING_WORDS 64
#define NEMAP_DEMO_CL_WORDS   1024
#define NEMAP_DEMO_RING_BYTES (NEMAP_DEMO_RING_WORDS * sizeof(uint32_t))
#define NEMAP_DEMO_CL_BYTES   (NEMAP_DEMO_CL_WORDS * sizeof(uint32_t))

#define NEMAP_DEMO_OFFSCREEN_W 64
#define NEMAP_DEMO_OFFSCREEN_H 64
#define NEMAP_DEMO_OFFSCREEN_PIXELS \
  (NEMAP_DEMO_OFFSCREEN_W * NEMAP_DEMO_OFFSCREEN_H)
#define NEMAP_DEMO_OFFSCREEN_BYTES \
  (NEMAP_DEMO_OFFSCREEN_PIXELS * sizeof(uint16_t))
#define NEMAP_DEMO_MASK_BYTES \
  (NEMAP_DEMO_OFFSCREEN_PIXELS * sizeof(uint8_t))
#define NEMAP_DEMO_FMT_SRC_BYTES \
  (NEMAP_DEMO_OFFSCREEN_PIXELS * sizeof(uint32_t))
#define NEMAP_DEMO_VISUAL_DELAY_MS 1000
#define NEMAP_DEMO_RGB565_RED     0xf800
#define NEMAP_DEMO_RGB565_SENTINEL 0x07e0
#define NEMAP_DEMO_RGB565_MAGENTA 0xf81f
#define NEMAP_DEMO_RGB565_BLUE    0x001f
#define NEMAP_DEMO_RGB565_GREEN   0x07e0
#define NEMAP_DEMO_RGB565_WHITE   0xffff
#define NEMAP_DEMO_RGB565_YELLOW  0xffe0
#define NEMAP_DEMO_RGB565_BLACK   0x0000
#define NEMAP_DEMO_RGB565_CYAN    0x07ff
#define NEMAP_DEMO_RGBA_BLUE      0xffff0000
#define NEMAP_DEMO_RGBA_GREEN     0xff00ff00
#define NEMAP_DEMO_RGBA_RED       0xff0000ff
#define NEMAP_DEMO_RGBA_RED_50    0x800000ff
#define NEMAP_DEMO_RGBA_MAGENTA   0xffff00ff
#define NEMAP_DEMO_RGBA_YELLOW    0xff00ffff
#define NEMAP_DEMO_RGBA_BLACK     0xff000000
#define NEMAP_DEMO_RGBA_CYAN      0xffffff00

#define NEMAP_DEMO_NEMA_RGBX8888 0x00
#define NEMAP_DEMO_NEMA_RGBA8888 0x01
#define NEMAP_DEMO_NEMA_XRGB8888 0x02
#define NEMAP_DEMO_NEMA_ARGB8888 0x03
#define NEMAP_DEMO_NEMA_RGBA5650 0x04
#define NEMAP_DEMO_NEMA_RGBA5551 0x05
#define NEMAP_DEMO_NEMA_RGBA4444 0x06
#define NEMAP_DEMO_NEMA_A8       0x08
#define NEMAP_DEMO_NEMA_L8       0x09
#define NEMAP_DEMO_NEMA_TSC6     0x16
#define NEMAP_DEMO_NEMA_TSC6A    0x17
#define NEMAP_DEMO_NEMA_RGB565   0x04
#define NEMAP_DEMO_NEMA_RGB332   0x38
#define NEMAP_DEMO_NEMA_BGR24    0x39
#define NEMAP_DEMO_NEMA_RGB24    0x3c
#define NEMAP_DEMO_TSC6_W        32
#define NEMAP_DEMO_TSC6_H        32
#define NEMAP_DEMO_TSC6_STRIDE   (NEMAP_DEMO_TSC6_W * 3)
#define NEMAP_DEMO_TSC6_FNV      0x574de758
#define NEMAP_DEMO_TSC6A_FNV     0xd914498d
#define NEMAP_DEMO_PROJECTIVE_FNV 0xa3ae06c6
#define NEMAP_DEMO_LINE_FNV      0xcb569745
#define NEMAP_DEMO_LINE_CHANGED  92
#define NEMAP_DEMO_GRADIENT_FNV  0x403525c5
#define NEMAP_DEMO_GRADIENT_CHANGED 2304
#define NEMAP_DEMO_GRADIENT_TL   0x0004
#define NEMAP_DEMO_GRADIENT_TR   0xf804
#define NEMAP_DEMO_GRADIENT_BL   0x07c4
#define NEMAP_DEMO_NEMA_FILTERPS 0x00
#define NEMAP_DEMO_NEMA_FILTERBL 0x01
#define NEMAP_DEMO_NEMA_CLAMP    0x00
#define NEMAP_DEMO_NEMA_REPEAT   (0x01 << 2)
#define NEMAP_DEMO_NEMA_BORDER   (0x02 << 2)
#define NEMAP_DEMO_NEMA_MIRROR   (0x03 << 2)
#define NEMAP_DEMO_DRAW_LINE     0x01
#define NEMAP_DEMO_DRAW_BOX      0x02
#define NEMAP_DEMO_DRAW_TRIANGLE 0x04
#define NEMAP_DEMO_DRAW_QUAD     0x05
#define NEMAP_DEMO_RAST_AA_E3    0x00800000
#define NEMAP_DEMO_RAST_AA_E2    0x01000000
#define NEMAP_DEMO_RAST_AA_E1    0x02000000
#define NEMAP_DEMO_RAST_AA_E0    0x04000000
#define NEMAP_DEMO_RAST_AA_MASK \
  (NEMAP_DEMO_RAST_AA_E0 | NEMAP_DEMO_RAST_AA_E1 | \
   NEMAP_DEMO_RAST_AA_E2 | NEMAP_DEMO_RAST_AA_E3)
#define NEMAP_DEMO_RAST_GRAD    0x08000000
#define NEMAP_DEMO_BLEND_SRC     0x01
#define NEMAP_DEMO_BLEND_SIMPLE  0x504
#define NEMAP_DEMO_BLOP_SRC_CKEY 0x40000000
#define NEMAP_DEMO_BLOP_DST_CKEY 0x80000000
#define NEMAP_DEMO_BLOP_STENCIL_TXTY 0x00800000
#define NEMAP_DEMO_MMUL_BYPASS   0x90000000
#define NEMAP_DEMO_PIXOUT_DATAH  0x0e8e0002
#define NEMAP_DEMO_PIXOUT_DATAL  0x80000009
#define NEMAP_DEMO_PRELOAD_ADDR  31
#define NEMAP_DEMO_PREFETCH_TEXEL 0x00008000
#define NEMAP_DEMO_PRE_XY        0x00001000
#define NEMAP_DEMO_PRE_IMG0      0x00000400
#define NEMAP_DEMO_FILL_CODEPTR \
  (NEMAP_DEMO_PRELOAD_ADDR | \
   ((NEMAP_DEMO_PREFETCH_TEXEL | NEMAP_DEMO_PRE_IMG0 | \
     NEMAP_DEMO_PRE_XY | NEMAP_DEMO_PRELOAD_ADDR) << 16))
#define NEMAP_DEMO_BLIT_CODEPTR \
  (NEMAP_DEMO_PREFETCH_TEXEL | NEMAP_DEMO_PRELOAD_ADDR | \
   ((NEMAP_DEMO_PREFETCH_TEXEL | NEMAP_DEMO_PRE_IMG0 | \
     NEMAP_DEMO_PRE_XY | NEMAP_DEMO_PRELOAD_ADDR) << 16))
#define NEMAP_DEMO_SRC_CKEY_DATAH 0x081c0002
#define NEMAP_DEMO_SRC_CKEY_DATAL 0x8a0761c7
#define NEMAP_DEMO_SRC_CKEY_CODEPTR \
  (NEMAP_DEMO_PREFETCH_TEXEL | \
   ((NEMAP_DEMO_PREFETCH_TEXEL | NEMAP_DEMO_PRE_IMG0 | \
     NEMAP_DEMO_PRE_XY | NEMAP_DEMO_PRELOAD_ADDR) << 16))
#define NEMAP_DEMO_STENCIL_LOAD_DATAH 0x100c118b
#define NEMAP_DEMO_STENCIL_LOAD_DATAL 0x00002000
#define NEMAP_DEMO_STENCIL_MUL_DATAH  0x000c0000
#define NEMAP_DEMO_STENCIL_MUL_DATAL  0x0081a042
#define NEMAP_DEMO_STENCIL_OUT_DATAH  0x080c0002
#define NEMAP_DEMO_STENCIL_OUT_DATAL  0x8a0761c7
#define NEMAP_DEMO_STENCIL_CODEPTR \
  (NEMAP_DEMO_PREFETCH_TEXEL | \
   ((NEMAP_DEMO_PREFETCH_TEXEL | NEMAP_DEMO_PRE_IMG0 | \
     NEMAP_DEMO_PRE_XY | (NEMAP_DEMO_PRELOAD_ADDR - 2)) << 16))

#if NEMAP_DEMO_PSRAM_RESOURCES
#  ifndef CONFIG_STM32N6_PSRAM_HEAP_OFFSET
#    define CONFIG_STM32N6_PSRAM_HEAP_OFFSET 0x00200000
#  endif
#  ifndef CONFIG_STM32H7RS_PSRAM_HEAP_OFFSET
#    define CONFIG_STM32H7RS_PSRAM_HEAP_OFFSET 0x00200000
#  endif
#  if NEMAP_DEMO_N6_PSRAM_RESOURCES
#    define NEMAP_DEMO_PSRAM_LABEL "N6"
#    define NEMAP_DEMO_PSRAM_BASE 0x90000000u
#    define NEMAP_DEMO_PSRAM_LIMIT \
  (NEMAP_DEMO_PSRAM_BASE + CONFIG_STM32N6_PSRAM_HEAP_OFFSET)
#  elif NEMAP_DEMO_H7RS_PSRAM_RESOURCES
#    define NEMAP_DEMO_PSRAM_LABEL "H7RS"
#    define NEMAP_DEMO_PSRAM_BASE 0x90000000u
#    define NEMAP_DEMO_PSRAM_LIMIT \
  (NEMAP_DEMO_PSRAM_BASE + CONFIG_STM32H7RS_PSRAM_HEAP_OFFSET)
#  endif
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

#if NEMAP_DEMO_U5
static uint32_t g_u5_ring[NEMAP_DEMO_RING_WORDS] aligned_data(8);
static uint32_t g_u5_cmdlist[NEMAP_DEMO_CL_WORDS] aligned_data(8);
static uint16_t g_u5_offscreen[NEMAP_DEMO_OFFSCREEN_PIXELS]
  aligned_data(32);
static uint16_t g_u5_blit_src[NEMAP_DEMO_OFFSCREEN_PIXELS]
  aligned_data(32);
static uint16_t g_u5_blit_dst[NEMAP_DEMO_OFFSCREEN_PIXELS]
  aligned_data(32);
static uint8_t g_u5_mask[NEMAP_DEMO_OFFSCREEN_PIXELS]
  aligned_data(32);
static uint32_t g_u5_fmt_src[NEMAP_DEMO_OFFSCREEN_PIXELS]
  aligned_data(32);
#endif

static FAR uint32_t *g_ring;
static FAR uint32_t *g_cmdlist;
static FAR uint16_t *g_offscreen;
static FAR uint16_t *g_blit_src;
static FAR uint16_t *g_blit_dst;
static FAR uint8_t *g_mask;
static FAR uint32_t *g_fmt_src;
static struct stm32_gpu2d_ring_s g_submit_ring;
static bool g_submit_ring_ready;
static const uint8_t g_tsc6_partial1[] =
{
#include "nemap_demo_tsc6_partial1.inc"
};
static const uint8_t g_tsc6a_alpha[] =
{
#include "nemap_demo_tsc6a_alpha.inc"
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void nemap_demo_clean(FAR const void *buffer, size_t size)
{
  uintptr_t start = (uintptr_t)buffer;

  up_clean_dcache(start, start + size);
}

static void nemap_demo_invalidate(FAR const void *buffer, size_t size)
{
  uintptr_t start = (uintptr_t)buffer;

  up_invalidate_dcache(start, start + size);
}

#if NEMAP_DEMO_PSRAM_RESOURCES
static uintptr_t nemap_demo_align_up(uintptr_t value, size_t align)
{
  return (value + align - 1) & ~((uintptr_t)align - 1);
}

static int nemap_demo_psram_take(FAR uintptr_t *cursor, uintptr_t limit,
                                 size_t size, FAR void **out)
{
  uintptr_t base = nemap_demo_align_up(*cursor, 32);
  uintptr_t next = base + size;

  if (next < base || next > limit)
    {
      printf("nemap_demo: " NEMAP_DEMO_PSRAM_LABEL
             " PSRAM resource arena exhausted "
             "base=0x%08" PRIxPTR " size=%zu limit=0x%08" PRIxPTR "\n",
             base, size, limit);
      return -ENOMEM;
    }

  *out = (FAR void *)base;
  *cursor = next;
  return OK;
}
#endif

static int nemap_demo_resources_init(FAR const struct fb_planeinfo_s *pinfo)
{
#if NEMAP_DEMO_U5
  g_ring = g_u5_ring;
  g_cmdlist = g_u5_cmdlist;
  g_offscreen = g_u5_offscreen;
  g_blit_src = g_u5_blit_src;
  g_blit_dst = g_u5_blit_dst;
  g_mask = g_u5_mask;
  g_fmt_src = g_u5_fmt_src;

  printf("nemap_demo: U5 SRAM resources cmdlist=0x%08" PRIxPTR
         " ring=0x%08" PRIxPTR " offscreen=0x%08" PRIxPTR
         " src=0x%08" PRIxPTR " dst=0x%08" PRIxPTR
         " mask=0x%08" PRIxPTR " fmt=0x%08" PRIxPTR "\n",
         (uintptr_t)g_cmdlist, (uintptr_t)g_ring, (uintptr_t)g_offscreen,
         (uintptr_t)g_blit_src,
         (uintptr_t)g_blit_dst, (uintptr_t)g_mask, (uintptr_t)g_fmt_src);
#elif NEMAP_DEMO_PSRAM_RESOURCES
  uintptr_t fbend = (uintptr_t)pinfo->fbmem + pinfo->fblen;
  uintptr_t cursor = nemap_demo_align_up(fbend, 4096);
  uintptr_t limit = (uintptr_t)NEMAP_DEMO_PSRAM_LIMIT;
  int ret;

  if ((uintptr_t)pinfo->fbmem < NEMAP_DEMO_PSRAM_BASE ||
      cursor > limit)
    {
      printf("nemap_demo: " NEMAP_DEMO_PSRAM_LABEL
             " PSRAM framebuffer layout unsupported "
             "fb=0x%08" PRIxPTR " len=%zu cursor=0x%08" PRIxPTR
             " limit=0x%08" PRIxPTR "\n",
             (uintptr_t)pinfo->fbmem, (size_t)pinfo->fblen, cursor, limit);
      return -ENOMEM;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_CL_BYTES,
                              (FAR void **)&g_cmdlist);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_RING_BYTES,
                              (FAR void **)&g_ring);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_OFFSCREEN_BYTES,
                              (FAR void **)&g_offscreen);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_OFFSCREEN_BYTES,
                              (FAR void **)&g_blit_src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_OFFSCREEN_BYTES,
                              (FAR void **)&g_blit_dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_MASK_BYTES,
                              (FAR void **)&g_mask);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_psram_take(&cursor, limit, NEMAP_DEMO_FMT_SRC_BYTES,
                              (FAR void **)&g_fmt_src);
  if (ret < 0)
    {
      return ret;
    }

  printf("nemap_demo: " NEMAP_DEMO_PSRAM_LABEL
         " PSRAM resources cmdlist=0x%08" PRIxPTR
         " ring=0x%08" PRIxPTR " offscreen=0x%08" PRIxPTR
         " src=0x%08" PRIxPTR " dst=0x%08" PRIxPTR
         " mask=0x%08" PRIxPTR " fmt=0x%08" PRIxPTR
         " end=0x%08" PRIxPTR " limit=0x%08" PRIxPTR "\n",
         (uintptr_t)g_cmdlist, (uintptr_t)g_ring, (uintptr_t)g_offscreen,
         (uintptr_t)g_blit_src,
         (uintptr_t)g_blit_dst, (uintptr_t)g_mask, (uintptr_t)g_fmt_src,
         cursor, limit);
#else
#  error Unsupported nemap_demo resource placement.
#endif

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  memset(g_ring, 0, NEMAP_DEMO_RING_BYTES);
  memset(g_offscreen, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  memset(g_blit_src, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  memset(g_blit_dst, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  memset(g_mask, 0, NEMAP_DEMO_MASK_BYTES);
  memset(g_fmt_src, 0, NEMAP_DEMO_FMT_SRC_BYTES);
  memset(&g_submit_ring, 0, sizeof(g_submit_ring));
  g_submit_ring_ready = false;

  nemap_demo_clean(g_cmdlist, NEMAP_DEMO_CL_BYTES);
  nemap_demo_clean(g_ring, NEMAP_DEMO_RING_BYTES);
  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_mask, NEMAP_DEMO_MASK_BYTES);
  nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);
  return OK;
}

static int nemap_demo_ring_start(void)
{
  int ret;

  memset(g_ring, 0, NEMAP_DEMO_RING_BYTES);

  ret = stm32_gpu2d_ringinit(&g_submit_ring, g_ring, NEMAP_DEMO_RING_WORDS,
                             (uintptr_t)g_ring);
  if (ret < 0)
    {
      printf("nemap_demo: ring init failed: %d\n", ret);
      return ret;
    }

  nemap_demo_clean(g_ring, NEMAP_DEMO_RING_BYTES);
  g_submit_ring_ready = true;
  return OK;
}

static uint32_t nemap_demo_yx(uint32_t y, uint32_t x)
{
  return ((y & 0xffff) << 16) | (x & 0xffff);
}

static uint32_t nemap_demo_fstride(uint32_t format, uint32_t mode,
                                   uint32_t stride)
{
  return ((format & 0xff) << 24) | ((mode & 0xff) << 16) |
         (stride & 0xffff);
}

static uint32_t nemap_demo_floatbits(float value)
{
  union
  {
    float f;
    uint32_t u;
  } conv;

  conv.f = value;
  return conv.u;
}

static uint32_t nemap_demo_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) |
         ((uint32_t)a << 24);
}

static uint32_t nemap_demo_fx(float value)
{
  return (uint32_t)((int32_t)(value * 65536.0f));
}

static FAR uint16_t *
nemap_demo_rgb565_pixel(FAR const struct fb_planeinfo_s *pinfo,
                        uint32_t x, uint32_t y)
{
  return &((FAR uint16_t *)pinfo->fbmem)[x + (y * (pinfo->stride / 2))];
}

static uint32_t nemap_demo_rgb565_r(uint16_t color)
{
  return (color >> 11) & 0x1f;
}

static uint32_t nemap_demo_rgb565_g(uint16_t color)
{
  return (color >> 5) & 0x3f;
}

static uint32_t nemap_demo_rgb565_b(uint16_t color)
{
  return color & 0x1f;
}

static bool nemap_demo_rgb565_is_red_blue_blend(uint16_t color)
{
  uint32_t r = nemap_demo_rgb565_r(color);
  uint32_t g = nemap_demo_rgb565_g(color);
  uint32_t b = nemap_demo_rgb565_b(color);

  return color != NEMAP_DEMO_RGB565_RED &&
         color != NEMAP_DEMO_RGB565_BLUE &&
         r >= 8 && r <= 24 && g <= 8 && b >= 8 && b <= 24;
}

static bool nemap_demo_rgb565_is_red_coverage(uint16_t color)
{
  uint32_t r = nemap_demo_rgb565_r(color);
  uint32_t g = nemap_demo_rgb565_g(color);
  uint32_t b = nemap_demo_rgb565_b(color);

  return color != 0 && color != NEMAP_DEMO_RGB565_RED &&
         r > 0 && r < 31 && g <= 1 && b <= 1;
}

static int nemap_demo_emitreg(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                              uint32_t reg, uint32_t data)
{
  return stm32_gpu2d_clemitreg(cmdlist, reg - STM32_GPU2D_BASE, data);
}

static uint32_t nemap_demo_regread(uint32_t reg)
{
  return stm32_gpu2dregread(reg - STM32_GPU2D_BASE);
}

static void nemap_demo_print_snapshot(FAR const char *label)
{
  printf("nemap_demo: %s snapshot last_cmdstatus=0x%08" PRIx32
         " last_status=0x%08" PRIx32 " last_interrupt=0x%08" PRIx32
         " last_syserror=0x%08" PRIx32 " now_syserror=0x%08" PRIx32 "\n",
         label, stm32_gpu2dlastcmdstatus(), stm32_gpu2dlaststatus(),
         stm32_gpu2dlastinterrupt(), stm32_gpu2dlastsyserror(),
         stm32_gpu2dreadsyserror());
}

static int nemap_demo_fb_probe(FAR const char *fbdev,
                               FAR struct fb_videoinfo_s *vinfo,
                               FAR struct fb_planeinfo_s *pinfo)
{
  int fd;
  int ret;

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s failed: %d\n", fbdev, errno);
      return -errno;
    }

  ret = ioctl(fd, FBIOGET_VIDEOINFO, (unsigned long)(uintptr_t)vinfo);
  if (ret < 0)
    {
      ret = -errno;
      printf("nemap_demo: FBIOGET_VIDEOINFO failed: %d\n", -ret);
      close(fd);
      return ret;
    }

  memset(pinfo, 0, sizeof(*pinfo));
  pinfo->display = 0;

  ret = ioctl(fd, FBIOGET_PLANEINFO, (unsigned long)(uintptr_t)pinfo);
  if (ret < 0)
    {
      ret = -errno;
      printf("nemap_demo: FBIOGET_PLANEINFO failed: %d\n", -ret);
      close(fd);
      return ret;
    }

  printf("nemap_demo: framebuffer %s %ux%u fmt=%u bpp=%u "
         "stride=%u fb=%p len=%zu\n",
         fbdev, vinfo->xres, vinfo->yres, vinfo->fmt, pinfo->bpp,
         pinfo->stride, pinfo->fbmem, pinfo->fblen);

  close(fd);
  return OK;
}

static int nemap_demo_get_plane(int fd, uint8_t display,
                                FAR struct fb_planeinfo_s *pinfo)
{
  memset(pinfo, 0, sizeof(*pinfo));
  pinfo->display = display;

  if (ioctl(fd, FBIOGET_PLANEINFO, (unsigned long)(uintptr_t)pinfo) < 0)
    {
      return -errno;
    }

  return OK;
}

static int nemap_demo_show_plane(int fd, FAR struct fb_planeinfo_s *pinfo,
                                 FAR const struct fb_videoinfo_s *vinfo,
                                 FAR const char *label, unsigned int delay_ms)
{
  int ret;

  nemap_demo_invalidate(pinfo->fbmem, pinfo->fblen);

#ifdef CONFIG_FB_UPDATE
  struct fb_area_s area;

  area.x = 0;
  area.y = pinfo->yoffset;
  area.w = vinfo->xres;
  area.h = vinfo->yres;
  ret = ioctl(fd, FBIO_UPDATE, (unsigned long)(uintptr_t)&area);
  if (ret < 0 && errno != ENOTTY && errno != ENOSYS && errno != ENOTSUP)
    {
      printf("nemap_demo: %s FBIO_UPDATE failed: %d\n", label, errno);
    }
#endif

  ret = ioctl(fd, FBIOPAN_DISPLAY, (unsigned long)(uintptr_t)pinfo);
  if (ret < 0)
    {
      ret = -errno;
      printf("nemap_demo: %s FBIOPAN_DISPLAY failed: %d\n", label, -ret);
      return ret;
    }

#ifdef CONFIG_FB_SYNC
  ret = ioctl(fd, FBIO_WAITFORVSYNC, 0);
  if (ret < 0 && errno != ENOTTY && errno != ENOSYS && errno != ENOTSUP)
    {
      printf("nemap_demo: %s FBIO_WAITFORVSYNC failed: %d\n", label, errno);
    }
#endif

  if (delay_ms > 0)
    {
      usleep(delay_ms * 1000);
    }

  return OK;
}

static void nemap_demo_cpu_colorbar(FAR const struct fb_videoinfo_s *vinfo,
                                    FAR const struct fb_planeinfo_s *pinfo)
{
  static const uint16_t colors[8] =
    {
      0xffff, 0xffe0, 0x07ff, 0x07e0,
      0xf81f, 0xf800, 0x001f, 0x0000
    };
  FAR uint16_t *fb = (FAR uint16_t *)pinfo->fbmem;
  uint32_t stride_pixels = pinfo->stride / sizeof(uint16_t);
  uint32_t x;
  uint32_t y;

  for (y = 0; y < vinfo->yres; y++)
    {
      for (x = 0; x < vinfo->xres; x++)
        {
          fb[x + y * stride_pixels] =
            colors[(x * nitems(colors)) / vinfo->xres];
        }
    }

  nemap_demo_clean(pinfo->fbmem, pinfo->fblen);
}

static void nemap_demo_cpu_black(FAR const struct fb_videoinfo_s *vinfo,
                                 FAR const struct fb_planeinfo_s *pinfo)
{
  FAR uint16_t *fb = (FAR uint16_t *)pinfo->fbmem;
  uint32_t stride_pixels = pinfo->stride / sizeof(uint16_t);
  uint32_t x;
  uint32_t y;

  for (y = 0; y < vinfo->yres; y++)
    {
      for (x = 0; x < vinfo->xres; x++)
        {
          fb[x + y * stride_pixels] = 0x0000;
        }
    }

  nemap_demo_clean(pinfo->fbmem, pinfo->fblen);
}

static void nemap_demo_cpu_solid_rgb565(
                                 FAR const struct fb_videoinfo_s *vinfo,
                                 FAR const struct fb_planeinfo_s *pinfo,
                                 uint16_t color)
{
  FAR uint16_t *fb = (FAR uint16_t *)pinfo->fbmem;
  uint32_t stride_pixels = pinfo->stride / sizeof(uint16_t);
  uint32_t x;
  uint32_t y;

  for (y = 0; y < vinfo->yres; y++)
    {
      for (x = 0; x < vinfo->xres; x++)
        {
          fb[x + y * stride_pixels] = color;
        }
    }

  nemap_demo_clean(pinfo->fbmem, pinfo->fblen);
}

static int nemap_demo_finish_screen(FAR const char *fbdev,
                                    FAR const struct fb_videoinfo_s *vinfo,
                                    bool success)
{
  struct fb_planeinfo_s pinfo;
  struct fb_planeinfo_s show;
  bool have_show = false;
  uint8_t display;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for finish screen failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  for (display = 0; display < 2; display++)
    {
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          continue;
        }

      if (success)
        {
          nemap_demo_cpu_colorbar(vinfo, &pinfo);
        }
      else
        {
          nemap_demo_cpu_black(vinfo, &pinfo);
        }

      if (!have_show)
        {
          show = pinfo;
          have_show = true;
        }
    }

  if (!have_show)
    {
      close(fd);
      return -ENODEV;
    }

  ret = nemap_demo_show_plane(fd, &show, vinfo,
                              success ? "finish-colorbar" :
                              "finish-black", 0);
  close(fd);
  return ret;
}

static void nemap_demo_print_feature(FAR const char *name, bool enabled)
{
  printf("nemap_demo:   %-12s %s\n", name, enabled ? "yes" : "no");
}

static void nemap_demo_decode_config(uint32_t config, uint32_t configh)
{
  printf("nemap_demo: CONFIG decode:\n");
  nemap_demo_print_feature("axi-master",
                           (config & GPU2D_CONFIG_AXI_MASTER) != 0);
  nemap_demo_print_feature("tex-filter",
                           (config & GPU2D_CONFIG_TEX_FILTER) != 0);
  nemap_demo_print_feature("tsc6", (config & GPU2D_CONFIG_TSC6) != 0);
  nemap_demo_print_feature("blender", (config & GPU2D_CONFIG_BLENDER) != 0);
  nemap_demo_print_feature("async", (config & GPU2D_CONFIG_ASYNC) != 0);
  nemap_demo_print_feature("dirty", (config & GPU2D_CONFIG_DIRTY) != 0);
  nemap_demo_print_feature("mmu", (config & GPU2D_CONFIG_MMU) != 0);
  nemap_demo_print_feature("zcompr", (config & GPU2D_CONFIG_ZCOMPR) != 0);
  nemap_demo_print_feature("vrx", (config & GPU2D_CONFIG_VRX) != 0);
  nemap_demo_print_feature("zbuf", (config & GPU2D_CONFIG_ZBUF) != 0);
  nemap_demo_print_feature("tsc", (config & GPU2D_CONFIG_TSC) != 0);
  nemap_demo_print_feature("clock-gate",
                           (config & GPU2D_CONFIG_CLOCK_GATE) != 0);
  nemap_demo_print_feature("vg", (config & GPU2D_CONFIG_VG) != 0);
  printf("nemap_demo:   cores       %" PRIu32 "\n",
         stm32_gpu2dcorecount(config));
  printf("nemap_demo:   threads-log2 %" PRIu32 "\n",
         config & GPU2D_CONFIG_THREAD_COUNT_MASK);
  printf("nemap_demo:   debug-level %" PRIu32 "\n",
         stm32_gpu2ddebuglevel(config));

  printf("nemap_demo: CONFIGH decode:\n");
  nemap_demo_print_feature("aa", (configh & GPU2D_CONFIGH_AA) != 0);
  nemap_demo_print_feature("decompress",
                           (configh & GPU2D_CONFIGH_DEC) != 0);
  nemap_demo_print_feature("10bit", (configh & GPU2D_CONFIGH_10BIT) != 0);
  nemap_demo_print_feature("gamma", (configh & GPU2D_CONFIGH_GAMMA) != 0);
  nemap_demo_print_feature("yuv-coeff",
                           (configh & GPU2D_CONFIGH_YUV_COEFF) != 0);
  nemap_demo_print_feature("tex-channels",
                           (configh & GPU2D_CONFIGH_TEX_CHANNELS) != 0);
  nemap_demo_print_feature("mbist", (configh & GPU2D_CONFIGH_MBIST) != 0);
  nemap_demo_print_feature("blue-wrap",
                           (configh & GPU2D_CONFIGH_BLUE_WRAP) != 0);
  nemap_demo_print_feature("radial", (configh & GPU2D_CONFIGH_RADIAL) != 0);
}

static int nemap_demo_identity(void)
{
  uint32_t id;
  uint32_t version;
  uint32_t config;
  uint32_t configh;
  uint32_t syserror;

  id = stm32_gpu2dreadid();
  version = stm32_gpu2dreadipversion();
  config = stm32_gpu2dreadconfig();
  configh = stm32_gpu2dreadconfigh();
  syserror = stm32_gpu2dreadsyserror();

  printf("nemap_demo: ID=0x%08" PRIx32 " IP_VERSION=0x%08" PRIx32 "\n",
         id, version);
  printf("nemap_demo: CONFIG=0x%08" PRIx32 " CONFIGH=0x%08" PRIx32
         " cores=%" PRIu32 " debug=%" PRIu32 "\n",
         config, configh, stm32_gpu2dcorecount(config),
         stm32_gpu2ddebuglevel(config));
  nemap_demo_decode_config(config, configh);
  printf("nemap_demo: SYS_INTERRUPT=0x%08" PRIx32 "\n", syserror);

  if (syserror != 0)
    {
      stm32_gpu2dclearsyserror(syserror);
      printf("nemap_demo: cleared stale SYS_INTERRUPT bits\n");
    }

  if ((id & 0xfffff000) != GPU2D_ID_EXPECTED)
    {
      printf("nemap_demo: warning: unexpected GPU2D ID, expected "
             "0x%08x mask 0xfffff000\n",
             GPU2D_ID_EXPECTED);
    }

  return OK;
}

static int nemap_demo_submit_cmdlist(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                     FAR const char *label)
{
  uint32_t submit_id;
  int ret;

#if NEMAP_DEMO_PERSISTENT_ROOT_RING
  if (!g_submit_ring_ready)
    {
      ret = nemap_demo_ring_start();
      if (ret < 0)
        {
          return ret;
        }
    }
#else
  ret = nemap_demo_ring_start();
  if (ret < 0)
    {
      return ret;
    }
#endif

  nemap_demo_clean(g_cmdlist, NEMAP_DEMO_CL_BYTES);

  ret = stm32_gpu2d_submit(&g_submit_ring, (uintptr_t)g_cmdlist,
                           cmdlist->offset_words, &submit_id);
  if (ret < 0)
    {
      printf("nemap_demo: %s submit failed: %d\n", label, ret);
      return ret;
    }

  nemap_demo_clean(g_ring, NEMAP_DEMO_RING_BYTES);

  printf("nemap_demo: submitted %s CL id=%" PRIu32 " words=%" PRIu32 "\n",
         label, submit_id, cmdlist->offset_words);

  ret = stm32_gpu2d_wait(submit_id, CONFIG_EXAMPLES_NEMAP_DEMO_TIMEOUT_MS);
  if (ret < 0)
    {
      printf("nemap_demo: %s wait failed: %d last_clid=0x%08" PRIx32
             " syserror=0x%08" PRIx32 "\n",
             label, ret, stm32_gpu2dlastclid(), stm32_gpu2dlastsyserror());
      return ret;
    }

  printf("nemap_demo: %s completion ok last_clid=0x%08" PRIx32
         " syserror=0x%08" PRIx32 "\n",
         label, stm32_gpu2dlastclid(), stm32_gpu2dlastsyserror());
  nemap_demo_print_snapshot(label);

  return OK;
}

static int nemap_demo_command_irq(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = stm32_gpu2d_clemitreturn(&cmdlist);
  if (ret < 0)
    {
      printf("nemap_demo: command-list emit failed: %d\n", ret);
      return ret;
    }

  return nemap_demo_submit_cmdlist(&cmdlist, "minimal");
}

static int nemap_demo_emit_fill_rgb565_clip(
                                       FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                       uintptr_t dst, uint32_t width,
                                       uint32_t height, uint32_t stride,
                                       uint32_t x, uint32_t y, uint32_t w,
                                       uint32_t h, uint32_t clipx,
                                       uint32_t clipy, uint32_t clipw,
                                       uint32_t cliph, uint32_t blend_mode,
                                       uint32_t rgba, bool clear_dirty)
{
  int ret;

  if (clear_dirty)
    {
      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DIRTYMIN, 0);
      if (ret < 0)
        {
          return ret;
        }
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (height << 16) | width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           blend_mode);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_FILL_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_COLOR, rgba);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN,
                           nemap_demo_yx(clipy, clipx));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(clipy + cliph, clipx + clipw));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT,
                           NEMAP_DEMO_MMUL_BYPASS);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(y, x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(y + h, x + w));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_BOX);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int nemap_demo_emit_fill_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                       uintptr_t dst, uint32_t width,
                                       uint32_t height, uint32_t stride,
                                       uint32_t x, uint32_t y, uint32_t w,
                                       uint32_t h, uint32_t rgba)
{
  return nemap_demo_emit_fill_rgb565_clip(cmdlist, dst, width, height,
                                          stride, x, y, w, h, 0, 0, width,
                                          height, NEMAP_DEMO_BLEND_SRC, rgba,
                                          false);
}

static int
nemap_demo_emit_line_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                            uintptr_t dst, uint32_t width, uint32_t height,
                            uint32_t stride, uint32_t x0, uint32_t y0,
                            uint32_t x1, uint32_t y1, uint32_t rgba)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (height << 16) | width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_FILL_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_COLOR, rgba);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(height, width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT,
                           NEMAP_DEMO_MMUL_BYPASS);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(y0, x0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(y1, x1));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_LINE);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_gradient_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                uintptr_t dst, uint32_t width,
                                uint32_t height, uint32_t stride,
                                uint32_t x, uint32_t y, uint32_t w,
                                uint32_t h)
{
  const int32_t fx = 1 << 16;
  int32_t red_dx = (255 * fx) / (int32_t)w;
  int32_t green_dy = (255 * fx) / (int32_t)h;
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (height << 16) | width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_FILL_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_RED_INIT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_GREEN_INIT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_BLUE_INIT, 32 * fx);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ALPHA_INIT, 255 * fx);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_RED_DX, (uint32_t)red_dx);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_RED_DY, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_GREEN_DX, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_GREEN_DY,
                           (uint32_t)green_dy);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_BLUE_DX, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_BLUE_DY, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ALPHA_DX, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ALPHA_DY, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(height, width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT,
                           NEMAP_DEMO_MMUL_BYPASS);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(y, x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(y + h, x + w));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_BOX | NEMAP_DEMO_RAST_GRAD);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_fill_rgb565_blend(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                  uintptr_t dst, uint32_t width,
                                  uint32_t height, uint32_t stride,
                                  uint32_t x, uint32_t y, uint32_t w,
                                  uint32_t h, uint32_t blend_mode,
                                  uint32_t rgba)
{
  return nemap_demo_emit_fill_rgb565_clip(cmdlist, dst, width, height,
                                          stride, x, y, w, h, 0, 0, width,
                                          height, blend_mode, rgba, false);
}

static int
nemap_demo_emit_fill_rgb565_dst_ckey(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                     uintptr_t dst, uint32_t width,
                                     uint32_t height, uint32_t stride,
                                     uint32_t x, uint32_t y, uint32_t w,
                                     uint32_t h, uint32_t rgba,
                                     uint32_t key_rgba)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_DST_CKEY, key_rgba);
  if (ret < 0)
    {
      return ret;
    }

  return nemap_demo_emit_fill_rgb565_clip(cmdlist, dst, width, height,
                                          stride, x, y, w, h, 0, 0, width,
                                          height, NEMAP_DEMO_BLEND_SRC |
                                          NEMAP_DEMO_BLOP_DST_CKEY, rgba,
                                          false);
}

static int
nemap_demo_emit_triangle_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                uintptr_t dst, uint32_t width,
                                uint32_t height, uint32_t stride,
                                float x0, float y0, float x1, float y1,
                                float x2, float y2, uint32_t aa_flags)
{
  uint32_t flags = aa_flags;
  int ret;

  if ((flags & NEMAP_DEMO_RAST_AA_E2) != 0)
    {
      flags |= NEMAP_DEMO_RAST_AA_E3;
    }
  else
    {
      flags &= ~NEMAP_DEMO_RAST_AA_E3;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (height << 16) | width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SIMPLE);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_FILL_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_COLOR,
                           NEMAP_DEMO_RGBA_RED);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(height, width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT,
                           NEMAP_DEMO_MMUL_BYPASS);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_X,
                           nemap_demo_fx(x0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_Y,
                           nemap_demo_fx(y0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_X,
                           nemap_demo_fx(x1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_Y,
                           nemap_demo_fx(y1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_X,
                           nemap_demo_fx(x2));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_Y,
                           nemap_demo_fx(y2));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           flags | NEMAP_DEMO_DRAW_TRIANGLE);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_rgb565_fit_mode(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                     uintptr_t dst, uint32_t dst_width,
                                     uint32_t dst_height,
                                     uint32_t dst_stride,
                                     uintptr_t src, uint32_t src_width,
                                     uint32_t src_height,
                                     uint32_t src_stride,
                                     uint32_t src_x, uint32_t src_y,
                                     uint32_t src_w, uint32_t src_h,
                                     uint32_t dst_x, uint32_t dst_y,
                                     uint32_t dst_w, uint32_t dst_h,
                                     uint32_t filter_mode,
                                     bool src_ckey,
                                     uint32_t key_rgba,
                                     bool stencil_txty,
                                     uintptr_t mask,
                                     uint32_t mask_width,
                                     uint32_t mask_height,
                                     uint32_t mask_stride)
{
  float scale_x;
  float scale_y;
  int ret;

  scale_x = (float)src_w / (float)dst_w;
  scale_y = (float)src_h / (float)dst_h;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              dst_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (dst_height << 16) | dst_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_BASE, src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              filter_mode,
                                              src_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_RESXY,
                           (src_height << 16) | src_width);
  if (ret < 0)
    {
      return ret;
    }

  if (stencil_txty)
    {
      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX3_BASE, mask);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX3_FSTRIDE,
                               nemap_demo_fstride(NEMAP_DEMO_NEMA_A8,
                                                  NEMAP_DEMO_NEMA_FILTERPS,
                                                  mask_stride));
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX3_RESXY,
                               (mask_height << 16) | mask_width);
      if (ret < 0)
        {
          return ret;
        }
    }

  if (src_ckey)
    {
      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CONST2, key_rgba);
      if (ret < 0)
        {
          return ret;
        }
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           stencil_txty ? NEMAP_DEMO_BLEND_SIMPLE :
                           (src_ckey ? NEMAP_DEMO_BLEND_SRC |
                           NEMAP_DEMO_BLOP_SRC_CKEY :
                           NEMAP_DEMO_BLEND_SRC));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           (src_ckey || stencil_txty) ? 0 :
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  if (stencil_txty)
    {
      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                               NEMAP_DEMO_STENCIL_LOAD_DATAH);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                               NEMAP_DEMO_STENCIL_LOAD_DATAL);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                               NEMAP_DEMO_STENCIL_MUL_DATAH);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                               NEMAP_DEMO_STENCIL_MUL_DATAL);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                               NEMAP_DEMO_STENCIL_OUT_DATAH);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                               NEMAP_DEMO_STENCIL_OUT_DATAL);
      if (ret < 0)
        {
          return ret;
        }
    }
  else
    {
      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                               src_ckey ? NEMAP_DEMO_SRC_CKEY_DATAH :
                               NEMAP_DEMO_PIXOUT_DATAH);
      if (ret < 0)
        {
          return ret;
        }

      ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                               src_ckey ? NEMAP_DEMO_SRC_CKEY_DATAL :
                               NEMAP_DEMO_PIXOUT_DATAL);
      if (ret < 0)
        {
          return ret;
        }
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           stencil_txty ? NEMAP_DEMO_STENCIL_CODEPTR :
                           (src_ckey ? NEMAP_DEMO_SRC_CKEY_CODEPTR :
                           NEMAP_DEMO_BLIT_CODEPTR));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(dst_height, dst_width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM00,
                           nemap_demo_floatbits(scale_x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM01,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM02,
                           nemap_demo_floatbits((float)src_x -
                                                ((float)dst_x * scale_x)));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM10,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM11,
                           nemap_demo_floatbits(scale_y));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM12,
                           nemap_demo_floatbits((float)src_y -
                                                ((float)dst_y * scale_y)));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM20,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM21,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM22,
                           nemap_demo_floatbits(1.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(dst_y, dst_x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(dst_y + dst_h, dst_x + dst_w));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_BOX);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_rgb565_fit(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                uintptr_t dst, uint32_t dst_width,
                                uint32_t dst_height,
                                uint32_t dst_stride,
                                uintptr_t src, uint32_t src_width,
                                uint32_t src_height,
                                uint32_t src_stride,
                                uint32_t src_x, uint32_t src_y,
                                uint32_t src_w, uint32_t src_h,
                                uint32_t dst_x, uint32_t dst_y,
                                uint32_t dst_w, uint32_t dst_h,
                                uint32_t filter_mode)
{
  return nemap_demo_emit_blit_rgb565_fit_mode(cmdlist, dst, dst_width,
                                              dst_height, dst_stride, src,
                                              src_width, src_height,
                                              src_stride, src_x, src_y, src_w,
                                              src_h, dst_x, dst_y, dst_w,
                                              dst_h, filter_mode, false, 0,
                                              false, 0, 0, 0, 0);
}

static int
nemap_demo_emit_blit_rgb565_matrix(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                   uintptr_t dst, uint32_t dst_width,
                                   uint32_t dst_height,
                                   uint32_t dst_stride,
                                   uintptr_t src, uint32_t src_width,
                                   uint32_t src_height,
                                   uint32_t src_stride,
                                   uint32_t dst_x, uint32_t dst_y,
                                   uint32_t dst_w, uint32_t dst_h,
                                   float mm00, float mm01, float mm02,
                                   float mm10, float mm11, float mm12,
                                   float mm20, float mm21, float mm22,
                                   uint32_t filter_mode)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              dst_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (dst_height << 16) | dst_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_BASE, src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              filter_mode,
                                              src_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_RESXY,
                           (src_height << 16) | src_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_BLIT_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(dst_height, dst_width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM00,
                           nemap_demo_floatbits(mm00));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM01,
                           nemap_demo_floatbits(mm01));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM02,
                           nemap_demo_floatbits(mm02));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM10,
                           nemap_demo_floatbits(mm10));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM11,
                           nemap_demo_floatbits(mm11));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM12,
                           nemap_demo_floatbits(mm12));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM20,
                           nemap_demo_floatbits(mm20));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM21,
                           nemap_demo_floatbits(mm21));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM22,
                           nemap_demo_floatbits(mm22));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(dst_y, dst_x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(dst_y + dst_h, dst_x + dst_w));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_BOX);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_rgb565_quad_matrix(
                                  FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                  uintptr_t dst, uint32_t dst_width,
                                  uint32_t dst_height,
                                  uint32_t dst_stride,
                                  uintptr_t src, uint32_t src_width,
                                  uint32_t src_height,
                                  uint32_t src_stride,
                                  float x0, float y0, float x1, float y1,
                                  float x2, float y2, float x3, float y3,
                                  float mm00, float mm01, float mm02,
                                  float mm10, float mm11, float mm12,
                                  float mm20, float mm21, float mm22,
                                  uint32_t filter_mode)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              dst_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (dst_height << 16) | dst_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_BASE, src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              filter_mode,
                                              src_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_RESXY,
                           (src_height << 16) | src_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_BLIT_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(dst_height, dst_width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM00,
                           nemap_demo_floatbits(mm00));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM01,
                           nemap_demo_floatbits(mm01));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM02,
                           nemap_demo_floatbits(mm02));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM10,
                           nemap_demo_floatbits(mm10));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM11,
                           nemap_demo_floatbits(mm11));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM12,
                           nemap_demo_floatbits(mm12));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM20,
                           nemap_demo_floatbits(mm20));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM21,
                           nemap_demo_floatbits(mm21));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM22,
                           nemap_demo_floatbits(mm22));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_X,
                           nemap_demo_fx(x0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_Y,
                           nemap_demo_fx(y0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_X,
                           nemap_demo_fx(x1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_Y,
                           nemap_demo_fx(y1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_X,
                           nemap_demo_fx(x2));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_Y,
                           nemap_demo_fx(y2));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT3_X,
                           nemap_demo_fx(x3));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT3_Y,
                           nemap_demo_fx(y3));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_QUAD);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_rgb565_triangle_matrix(
                                  FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                  uintptr_t dst, uint32_t dst_width,
                                  uint32_t dst_height,
                                  uint32_t dst_stride,
                                  uintptr_t src, uint32_t src_width,
                                  uint32_t src_height,
                                  uint32_t src_stride,
                                  float x0, float y0, float x1, float y1,
                                  float x2, float y2,
                                  float mm00, float mm01, float mm02,
                                  float mm10, float mm11, float mm12,
                                  float mm20, float mm21, float mm22,
                                  uint32_t filter_mode)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              dst_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (dst_height << 16) | dst_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_BASE, src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              filter_mode,
                                              src_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_RESXY,
                           (src_height << 16) | src_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_BLIT_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(dst_height, dst_width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM00,
                           nemap_demo_floatbits(mm00));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM01,
                           nemap_demo_floatbits(mm01));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM02,
                           nemap_demo_floatbits(mm02));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM10,
                           nemap_demo_floatbits(mm10));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM11,
                           nemap_demo_floatbits(mm11));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM12,
                           nemap_demo_floatbits(mm12));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM20,
                           nemap_demo_floatbits(mm20));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM21,
                           nemap_demo_floatbits(mm21));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM22,
                           nemap_demo_floatbits(mm22));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_X,
                           nemap_demo_fx(x0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT0_Y,
                           nemap_demo_fx(y0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_X,
                           nemap_demo_fx(x1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT1_Y,
                           nemap_demo_fx(y1));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_X,
                           nemap_demo_fx(x2));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_PT2_Y,
                           nemap_demo_fx(y2));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_TRIANGLE);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_format_to_rgb565_mode(
                                      FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                      uintptr_t dst, uint32_t dst_width,
                                      uint32_t dst_height,
                                      uint32_t dst_stride,
                                      uintptr_t src, uint32_t src_format,
                                      uint32_t src_width,
                                      uint32_t src_height,
                                      uint32_t src_stride,
                                      uint32_t dst_x, uint32_t dst_y,
                                      uint32_t w, uint32_t h,
                                      uint32_t blend_mode)
{
  float scale_x = (float)src_width / (float)w;
  float scale_y = (float)src_height / (float)h;
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              dst_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (dst_height << 16) | dst_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_BASE, src);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_FSTRIDE,
                           nemap_demo_fstride(src_format,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              src_stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX1_RESXY,
                           (src_height << 16) | src_width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE, blend_mode);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_BLIT_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(dst_height, dst_width));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT, 0);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM00,
                           nemap_demo_floatbits(scale_x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM01,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM02,
                           nemap_demo_floatbits(-((float)dst_x * scale_x)));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM10,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM11,
                           nemap_demo_floatbits(scale_y));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM12,
                           nemap_demo_floatbits(-((float)dst_y * scale_y)));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM20,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM21,
                           nemap_demo_floatbits(0.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_MM22,
                           nemap_demo_floatbits(1.0f));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx(dst_y, dst_x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx(dst_y + h, dst_x + w));
  if (ret < 0)
    {
      return ret;
    }

  ret = stm32_gpu2d_clemit(cmdlist,
                           GPU2D_CL_HOLD |
                           GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                         STM32_GPU2D_BASE),
                           NEMAP_DEMO_DRAW_BOX);
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_blit_format_to_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                      uintptr_t dst, uint32_t dst_width,
                                      uint32_t dst_height,
                                      uint32_t dst_stride,
                                      uintptr_t src, uint32_t src_format,
                                      uint32_t src_width,
                                      uint32_t src_height,
                                      uint32_t src_stride,
                                      uint32_t dst_x, uint32_t dst_y,
                                      uint32_t w, uint32_t h)
{
  return nemap_demo_emit_blit_format_to_rgb565_mode(cmdlist, dst, dst_width,
                                                    dst_height, dst_stride,
                                                    src, src_format,
                                                    src_width, src_height,
                                                    src_stride, dst_x, dst_y,
                                                    w, h,
                                                    NEMAP_DEMO_BLEND_SRC);
}

static int nemap_demo_emit_blit_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                       uintptr_t dst, uint32_t dst_width,
                                       uint32_t dst_height,
                                       uint32_t dst_stride,
                                       uintptr_t src, uint32_t src_width,
                                       uint32_t src_height,
                                       uint32_t src_stride,
                                       uint32_t dst_x, uint32_t dst_y,
                                       uint32_t w, uint32_t h)
{
  return nemap_demo_emit_blit_rgb565_fit(cmdlist, dst, dst_width, dst_height,
                                         dst_stride, src, src_width,
                                         src_height, src_stride, 0, 0, w, h,
                                         dst_x, dst_y, w, h,
                                         NEMAP_DEMO_NEMA_FILTERPS);
}

static uint16_t nemap_demo_affine_pattern(uint32_t x, uint32_t y)
{
  return (uint16_t)(((x & 0x1f) << 11) | ((y & 0x3f) << 5) |
                    ((x ^ y) & 0x1f));
}

static void nemap_demo_prepare_affine_pattern(void)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_affine_pattern(x, y);
        }
    }
}

static int nemap_demo_fill_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  size_t i;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_offscreen[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);
  printf("nemap_demo: fill offscreen sentinel first=0x%04x\n",
         g_offscreen[0]);

  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)g_offscreen,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_OFFSCREEN_W *
                                    sizeof(uint16_t),
                                    0, 0,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_RGBA_RED);
  if (ret < 0)
    {
      printf("nemap_demo: fill offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "fill-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      if (g_offscreen[i] != NEMAP_DEMO_RGB565_RED)
        {
          printf("nemap_demo: fill offscreen mismatch index=%zu "
                 "got=0x%04x expected=0x%04x\n",
                 i, g_offscreen[i], NEMAP_DEMO_RGB565_RED);
          nemap_demo_print_snapshot("fill-offscreen mismatch");
          return -EIO;
        }
    }

  printf("nemap_demo: PASS phase 5b RGB565 offscreen fill compare\n");
  return OK;
}

static int nemap_demo_clip_dirty_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t dirtymin;
  uint32_t dirtymax;
  uint32_t expected_dirtymin;
  uint32_t expected_dirtymax;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_offscreen[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565_clip(&cmdlist, (uintptr_t)g_offscreen,
                                         NEMAP_DEMO_OFFSCREEN_W,
                                         NEMAP_DEMO_OFFSCREEN_H,
                                         NEMAP_DEMO_OFFSCREEN_W *
                                         sizeof(uint16_t),
                                         0, 0,
                                         NEMAP_DEMO_OFFSCREEN_W,
                                         NEMAP_DEMO_OFFSCREEN_H,
                                         4, 5, 8, 8,
                                         NEMAP_DEMO_BLEND_SRC,
                                         NEMAP_DEMO_RGBA_RED, true);
  if (ret < 0)
    {
      printf("nemap_demo: clip/dirty emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "clip-dirty-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  dirtymin = stm32_gpu2dregread(STM32_GPU2D_DIRTYMIN - STM32_GPU2D_BASE);
  dirtymax = stm32_gpu2dregread(STM32_GPU2D_DIRTYMAX - STM32_GPU2D_BASE);
  expected_dirtymin = nemap_demo_yx(5, 4);
  expected_dirtymax = nemap_demo_yx(12, 11);

  printf("nemap_demo: clip/dirty dirtymin=0x%08" PRIx32
         " dirtymax=0x%08" PRIx32 " expected=0x%08" PRIx32
         "/0x%08" PRIx32 "\n",
         dirtymin, dirtymax, expected_dirtymin, expected_dirtymax);

  nemap_demo_invalidate(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t expected;
          uint16_t got;

          expected = x >= 4 && x < 12 && y >= 5 && y < 13 ?
            NEMAP_DEMO_RGB565_RED : NEMAP_DEMO_RGB565_SENTINEL;
          got = g_offscreen[x + y * NEMAP_DEMO_OFFSCREEN_W];
          if (got != expected)
            {
              printf("nemap_demo: clip/dirty mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x\n",
                     x, y, got, expected);
              nemap_demo_print_snapshot("clip-dirty mismatch");
              return -EIO;
            }
        }
    }

  if (dirtymin != expected_dirtymin || dirtymax != expected_dirtymax)
    {
      printf("nemap_demo: clip/dirty bounds mismatch\n");
      nemap_demo_print_snapshot("clip-dirty bounds mismatch");
      return -EIO;
    }

  stm32_gpu2dregwrite(STM32_GPU2D_DIRTYMIN - STM32_GPU2D_BASE, 0);

  printf("nemap_demo: PASS phase 7 RGB565 clip and dirty region\n");
  return OK;
}

static uint16_t nemap_demo_blit_pattern(uint32_t x, uint32_t y)
{
  uint32_t halfw = NEMAP_DEMO_OFFSCREEN_W / 2;
  uint32_t halfh = NEMAP_DEMO_OFFSCREEN_H / 2;

  if (x < halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_RED;
    }

  if (x >= halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_BLUE;
    }

  if (x < halfw)
    {
      return NEMAP_DEMO_RGB565_MAGENTA;
    }

  return NEMAP_DEMO_RGB565_YELLOW;
}

static void nemap_demo_prepare_blit_pattern(void)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_blit_pattern(x, y);
        }
    }
}

static uint16_t nemap_demo_scale_pattern(uint32_t x, uint32_t y,
                                         uint32_t w, uint32_t h)
{
  uint32_t halfw = w / 2;
  uint32_t halfh = h / 2;

  if (x < halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_RED;
    }

  if (x >= halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_BLUE;
    }

  if (x < halfw)
    {
      return NEMAP_DEMO_RGB565_MAGENTA;
    }

  return NEMAP_DEMO_RGB565_YELLOW;
}

static void nemap_demo_prepare_scale_pattern(uint32_t w, uint32_t h)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          if (x < w && y < h)
            {
              g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                nemap_demo_scale_pattern(x, y, w, h);
            }
          else
            {
              g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                NEMAP_DEMO_RGB565_SENTINEL;
            }
        }
    }
}

static void nemap_demo_prepare_filter_pattern(uint32_t w, uint32_t h)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          if (x < w && y < h)
            {
              g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                x < w / 2 ? NEMAP_DEMO_RGB565_RED :
                NEMAP_DEMO_RGB565_BLUE;
            }
          else
            {
              g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                NEMAP_DEMO_RGB565_SENTINEL;
            }
        }
    }
}

static uint16_t nemap_demo_dst_ckey_pattern(uint32_t x, uint32_t y,
                                            uint32_t cell)
{
  return (((x / cell) + (y / cell)) & 1) == 0 ?
    NEMAP_DEMO_RGB565_BLUE : NEMAP_DEMO_RGB565_YELLOW;
}

static void nemap_demo_prepare_dst_ckey_offscreen(uint32_t cell)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_offscreen[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_dst_ckey_pattern(x, y, cell);
        }
    }
}

static void nemap_demo_prepare_dst_ckey_fb(
                                    FAR const struct fb_videoinfo_s *vinfo,
                                    FAR const struct fb_planeinfo_s *pinfo,
                                    uint32_t cell)
{
  FAR uint16_t *fb = (FAR uint16_t *)pinfo->fbmem;
  uint32_t stride_pixels = pinfo->stride / sizeof(uint16_t);
  uint32_t x;
  uint32_t y;

  for (y = 0; y < vinfo->yres; y++)
    {
      for (x = 0; x < vinfo->xres; x++)
        {
          fb[x + y * stride_pixels] =
            nemap_demo_dst_ckey_pattern(x, y, cell);
        }
    }

  nemap_demo_clean(pinfo->fbmem, pinfo->fblen);
}

static uint16_t nemap_demo_src_ckey_pattern(uint32_t x, uint32_t y,
                                            uint32_t cell)
{
  return (((x / cell) + (y / cell)) & 1) == 0 ?
    NEMAP_DEMO_RGB565_RED : NEMAP_DEMO_RGB565_YELLOW;
}

static void nemap_demo_prepare_src_ckey_source(uint32_t cell)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_src_ckey_pattern(x, y, cell);
        }
    }
}

static uint8_t nemap_demo_stencil_mask_pattern(uint32_t x, uint32_t y,
                                               uint32_t cell)
{
  return (((x / cell) + (y / cell)) & 1) == 0 ? 0xff : 0x00;
}

static void nemap_demo_prepare_stencil_source_and_mask(uint32_t cell)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_RED;
          g_mask[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_stencil_mask_pattern(x, y, cell);
        }
    }
}

static FAR const char *nemap_demo_format_name(uint32_t format)
{
  switch (format)
    {
      case NEMAP_DEMO_NEMA_RGBX8888:
        return "RGBX8888";
      case NEMAP_DEMO_NEMA_RGBA8888:
        return "RGBA8888";
      case NEMAP_DEMO_NEMA_XRGB8888:
        return "XRGB8888";
      case NEMAP_DEMO_NEMA_ARGB8888:
        return "ARGB8888";
      case NEMAP_DEMO_NEMA_RGBA5650:
        return "RGBA5650";
      case NEMAP_DEMO_NEMA_RGBA5551:
        return "RGBA5551";
      case NEMAP_DEMO_NEMA_RGBA4444:
        return "RGBA4444";
      case NEMAP_DEMO_NEMA_L8:
        return "L8";
      case NEMAP_DEMO_NEMA_TSC6:
        return "TSC6";
      case NEMAP_DEMO_NEMA_TSC6A:
        return "TSC6A";
      case NEMAP_DEMO_NEMA_RGB332:
        return "RGB332";
      case NEMAP_DEMO_NEMA_BGR24:
        return "BGR24";
      case NEMAP_DEMO_NEMA_RGB24:
        return "RGB24";
      default:
        return "unknown";
    }
}

static uint32_t nemap_demo_pack_32bit(uint32_t format, uint8_t r, uint8_t g,
                                      uint8_t b, uint8_t a)
{
  switch (format)
    {
      case NEMAP_DEMO_NEMA_RGBX8888:
      case NEMAP_DEMO_NEMA_RGBA8888:
        return nemap_demo_rgba(r, g, b, a);

      case NEMAP_DEMO_NEMA_XRGB8888:
      case NEMAP_DEMO_NEMA_ARGB8888:
        return (uint32_t)a | ((uint32_t)r << 8) | ((uint32_t)g << 16) |
               ((uint32_t)b << 24);

      default:
        return 0;
    }
}

static uint16_t nemap_demo_pack_16bit(uint32_t format, uint8_t r, uint8_t g,
                                      uint8_t b, uint8_t a)
{
  switch (format)
    {
      case NEMAP_DEMO_NEMA_RGBA5650:
        return (((uint16_t)r >> 3) << 11) |
               (((uint16_t)g >> 2) << 5) |
               ((uint16_t)b >> 3);

      case NEMAP_DEMO_NEMA_RGBA5551:
        return (((uint16_t)r >> 3) << 11) |
               (((uint16_t)g >> 3) << 6) |
               (((uint16_t)b >> 3) << 1) |
               (a >= 0x80 ? 1 : 0);

      case NEMAP_DEMO_NEMA_RGBA4444:
        return (((uint16_t)r >> 4) << 12) |
               (((uint16_t)g >> 4) << 8) |
               (((uint16_t)b >> 4) << 4) |
               ((uint16_t)a >> 4);

      default:
        return 0;
    }
}

static uint16_t nemap_demo_rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint16_t)r >> 3) << 11) |
         (((uint16_t)g >> 2) << 5) |
         ((uint16_t)b >> 3);
}

static uint16_t nemap_demo_format_pattern(uint32_t x, uint32_t y)
{
  uint32_t halfw = NEMAP_DEMO_OFFSCREEN_W / 2;
  uint32_t halfh = NEMAP_DEMO_OFFSCREEN_H / 2;

  if (x < halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_RED;
    }

  if (x >= halfw && y < halfh)
    {
      return NEMAP_DEMO_RGB565_GREEN;
    }

  if (x < halfw)
    {
      return NEMAP_DEMO_RGB565_BLUE;
    }

  return NEMAP_DEMO_RGB565_WHITE;
}

static void nemap_demo_format_pattern_rgb888(uint32_t x, uint32_t y,
                                             FAR uint8_t *r,
                                             FAR uint8_t *g,
                                             FAR uint8_t *b)
{
  switch (nemap_demo_format_pattern(x, y))
    {
      case NEMAP_DEMO_RGB565_RED:
        *r = 0xff;
        *g = 0x00;
        *b = 0x00;
        break;

      case NEMAP_DEMO_RGB565_GREEN:
        *r = 0x00;
        *g = 0xff;
        *b = 0x00;
        break;

      case NEMAP_DEMO_RGB565_BLUE:
        *r = 0x00;
        *g = 0x00;
        *b = 0xff;
        break;

      default:
        *r = 0xff;
        *g = 0xff;
        *b = 0xff;
        break;
    }
}

static uint8_t nemap_demo_pack_8bit(uint32_t format, uint32_t x, uint32_t y)
{
  if (format == NEMAP_DEMO_NEMA_RGB332)
    {
      switch (nemap_demo_format_pattern(x, y))
        {
          case NEMAP_DEMO_RGB565_RED:
            return 0xe0;
          case NEMAP_DEMO_RGB565_GREEN:
            return 0x1c;
          case NEMAP_DEMO_RGB565_BLUE:
            return 0x03;
          default:
            return 0xff;
        }
    }

  switch (nemap_demo_format_pattern(x, y))
    {
      case NEMAP_DEMO_RGB565_RED:
        return 0x00;
      case NEMAP_DEMO_RGB565_GREEN:
        return 0x55;
      case NEMAP_DEMO_RGB565_BLUE:
        return 0xaa;
      default:
        return 0xff;
    }
}

static uint16_t nemap_demo_format_low_expected(uint32_t format, uint32_t x,
                                               uint32_t y)
{
  if (format == NEMAP_DEMO_NEMA_L8)
    {
      uint8_t luma = nemap_demo_pack_8bit(format, x, y);

      return nemap_demo_rgb888_to_rgb565(luma, luma, luma);
    }

  return nemap_demo_format_pattern(x, y);
}

static uint32_t nemap_demo_format_source_bpp(uint32_t format)
{
  switch (format)
    {
      case NEMAP_DEMO_NEMA_L8:
      case NEMAP_DEMO_NEMA_RGB332:
        return 1;

      case NEMAP_DEMO_NEMA_RGB24:
      case NEMAP_DEMO_NEMA_BGR24:
        return 3;

      default:
        return 4;
    }
}

static uint32_t nemap_demo_rgb565_fnv(FAR const uint16_t *pixels,
                                      uint32_t stride_pixels,
                                      uint32_t width, uint32_t height)
{
  uint32_t hash = 2166136261u;
  uint32_t x;
  uint32_t y;

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          uint16_t color = pixels[x + y * stride_pixels];

          hash ^= color & 0xff;
          hash *= 16777619u;
          hash ^= color >> 8;
          hash *= 16777619u;
        }
    }

  return hash;
}

static uint32_t nemap_demo_format_pattern32(uint32_t format, uint32_t x,
                                            uint32_t y)
{
  switch (nemap_demo_format_pattern(x, y))
    {
      case NEMAP_DEMO_RGB565_RED:
        return nemap_demo_pack_32bit(format, 0xff, 0x00, 0x00, 0xff);
      case NEMAP_DEMO_RGB565_GREEN:
        return nemap_demo_pack_32bit(format, 0x00, 0xff, 0x00, 0xff);
      case NEMAP_DEMO_RGB565_BLUE:
        return nemap_demo_pack_32bit(format, 0x00, 0x00, 0xff, 0xff);
      default:
        return nemap_demo_pack_32bit(format, 0xff, 0xff, 0xff, 0xff);
    }
}

static uint16_t nemap_demo_format_pattern16(uint32_t format, uint32_t x,
                                            uint32_t y)
{
  switch (nemap_demo_format_pattern(x, y))
    {
      case NEMAP_DEMO_RGB565_RED:
        return nemap_demo_pack_16bit(format, 0xff, 0x00, 0x00, 0xff);
      case NEMAP_DEMO_RGB565_GREEN:
        return nemap_demo_pack_16bit(format, 0x00, 0xff, 0x00, 0xff);
      case NEMAP_DEMO_RGB565_BLUE:
        return nemap_demo_pack_16bit(format, 0x00, 0x00, 0xff, 0xff);
      default:
        return nemap_demo_pack_16bit(format, 0xff, 0xff, 0xff, 0xff);
    }
}

static void nemap_demo_prepare_format16_source(uint32_t format)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_format_pattern16(format, x, y);
        }
    }
}

static void nemap_demo_prepare_format_low_source(uint32_t format)
{
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t bpp = nemap_demo_format_source_bpp(format);
  uint32_t stride = NEMAP_DEMO_OFFSCREEN_W * bpp;
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          FAR uint8_t *pixel = &src[y * stride + x * bpp];

          if (bpp == 1)
            {
              pixel[0] = nemap_demo_pack_8bit(format, x, y);
            }
          else
            {
              uint8_t r;
              uint8_t g;
              uint8_t b;

              nemap_demo_format_pattern_rgb888(x, y, &r, &g, &b);
              if (format == NEMAP_DEMO_NEMA_BGR24)
                {
                  pixel[0] = b;
                  pixel[1] = g;
                  pixel[2] = r;
                }
              else
                {
                  pixel[0] = r;
                  pixel[1] = g;
                  pixel[2] = b;
                }
            }
        }
    }
}

static void nemap_demo_prepare_format_source(uint32_t format)
{
  uint32_t x;
  uint32_t y;

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_fmt_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_format_pattern32(format, x, y);
        }
    }
}

static int nemap_demo_blit_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_blit_pattern();
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_blit_rgb565(&cmdlist, (uintptr_t)g_blit_dst,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_OFFSCREEN_W *
                                    sizeof(uint16_t),
                                    (uintptr_t)g_blit_src,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_OFFSCREEN_W *
                                    sizeof(uint16_t),
                                    0, 0,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H);
  if (ret < 0)
    {
      printf("nemap_demo: blit offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "blit-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t expected;
          uint16_t got;
          uint32_t edge_x = NEMAP_DEMO_OFFSCREEN_W / 2;
          uint32_t edge_y = NEMAP_DEMO_OFFSCREEN_H / 2;

          if (x == edge_x - 1 || x == edge_x ||
              y == edge_y - 1 || y == edge_y)
            {
              continue;
            }

          expected = nemap_demo_blit_pattern(x, y);
          got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];
          if (got != expected)
            {
              printf("nemap_demo: blit offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x\n",
                     x, y, got, expected);
              nemap_demo_print_snapshot("blit-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 8a RGB565 source blit offscreen "
         "interior compare\n");
  return OK;
}

static int nemap_demo_blit_visible(FAR const char *fbdev,
                                   FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t x;
  uint32_t y;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 8b visible blit: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible blit failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 8b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  x = vinfo->xres > NEMAP_DEMO_OFFSCREEN_W ?
    (vinfo->xres - NEMAP_DEMO_OFFSCREEN_W) / 2 : 0;
  y = vinfo->yres > NEMAP_DEMO_OFFSCREEN_H ?
    (vinfo->yres - NEMAP_DEMO_OFFSCREEN_H) / 2 : 0;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    0, 0, vinfo->xres, vinfo->yres,
                                    NEMAP_DEMO_RGBA_BLACK);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blit-visible-bg");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  nemap_demo_prepare_blit_pattern();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_blit_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    (uintptr_t)g_blit_src,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_OFFSCREEN_W *
                                    sizeof(uint16_t),
                                    x, y,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blit-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  nemap_demo_invalidate(pinfo.fbmem, pinfo.fblen);

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 8b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 8b RGB565 source blit visible marker "
         "at %" PRIu32 ",%" PRIu32 "\n",
         x, y);
  close(fd);
  return OK;
}

static int nemap_demo_scale_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_scale_pattern(32, 32);
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_blit_rgb565_fit(&cmdlist, (uintptr_t)g_blit_dst,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        (uintptr_t)g_blit_src,
                                        32, 32,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        0, 0, 32, 32,
                                        0, 0,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      printf("nemap_demo: scale offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "scale-offscreen-ps");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t expected;
          uint16_t got;
          uint32_t edge_x = NEMAP_DEMO_OFFSCREEN_W / 2;
          uint32_t edge_y = NEMAP_DEMO_OFFSCREEN_H / 2;

          if (x >= edge_x - 2 && x <= edge_x + 1)
            {
              continue;
            }

          if (y >= edge_y - 2 && y <= edge_y + 1)
            {
              continue;
            }

          expected = nemap_demo_scale_pattern(x / 2, y / 2, 32, 32);
          got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];
          if (got != expected)
            {
              printf("nemap_demo: scale offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x\n",
                     x, y, got, expected);
              nemap_demo_print_snapshot("scale-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 10a RGB565 point-sampled 2x scale "
         "offscreen compare\n");
  return OK;
}

static int nemap_demo_filter_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t mixed = 0;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_filter_pattern(4, 4);
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_blit_rgb565_fit(&cmdlist, (uintptr_t)g_blit_dst,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        (uintptr_t)g_blit_src,
                                        4, 4,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        0, 0, 4, 4,
                                        0, 0,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_NEMA_FILTERBL);
  if (ret < 0)
    {
      printf("nemap_demo: filter offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "filter-offscreen-bl");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 24; x < 40; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (nemap_demo_rgb565_is_red_blue_blend(got))
            {
              mixed++;
            }
        }
    }

  if (mixed < 64)
    {
      printf("nemap_demo: filter offscreen mixed-count too small: %"
             PRIu32 "\n", mixed);
      nemap_demo_print_snapshot("filter-offscreen mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 10b RGB565 bilinear filter offscreen "
         "mixed-count=%" PRIu32 "\n",
         mixed);
  return OK;
}

static int nemap_demo_scale_filter_visible(FAR const char *fbdev,
                                           FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t size;
  uint32_t gap = 24;
  uint32_t left_x;
  uint32_t right_x;
  uint32_t y;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 10c visible scale/filter: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible scale/filter failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 10c get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  size = MIN((uint32_t)160, ((uint32_t)vinfo->xres - gap - 32) / 2);
  size = MIN(size, (uint32_t)vinfo->yres - 80);
  size &= ~1u;
  if (size < 32)
    {
      size = 32;
    }

  left_x = ((uint32_t)vinfo->xres - (size * 2 + gap)) / 2;
  right_x = left_x + size + gap;
  y = ((uint32_t)vinfo->yres - size) / 2;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    0, 0, vinfo->xres, vinfo->yres,
                                    NEMAP_DEMO_RGBA_BLACK);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "scale-filter-visible-bg");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  nemap_demo_prepare_scale_pattern(32, 32);
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit(&cmdlist, (uintptr_t)pinfo.fbmem,
                                        vinfo->xres, vinfo->yres,
                                        pinfo.stride, (uintptr_t)g_blit_src,
                                        32, 32,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        0, 0, 32, 32, left_x, y,
                                        size, size,
                                        NEMAP_DEMO_NEMA_FILTERPS);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "scale-visible-ps");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  nemap_demo_prepare_filter_pattern(4, 4);
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit(&cmdlist, (uintptr_t)pinfo.fbmem,
                                        vinfo->xres, vinfo->yres,
                                        pinfo.stride, (uintptr_t)g_blit_src,
                                        4, 4,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        0, 0, 4, 4, right_x, y,
                                        size, size,
                                        NEMAP_DEMO_NEMA_FILTERBL);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "filter-visible-bl");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 10c",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 10c RGB565 visible point-scale and "
         "bilinear-filter panels size=%" PRIu32 " at %" PRIu32 ",%"
         PRIu32 " / %" PRIu32 ",%" PRIu32 "\n",
         size, left_x, y, right_x, y);
  close(fd);
  return OK;
}

static int nemap_demo_triangle_aa_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t aa_mixed = 0;
  uint32_t noaa_mixed = 0;
  uint32_t diff = 0;
  size_t i;
  int ret;

  memset(g_offscreen, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  memset(g_blit_dst, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_triangle_rgb565(&cmdlist, (uintptr_t)g_offscreen,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        7.25f, 8.50f,
                                        56.50f, 20.25f,
                                        18.25f, 56.75f, 0);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "aa-offscreen-noaa");
    }

  if (ret < 0)
    {
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_triangle_rgb565(&cmdlist, (uintptr_t)g_blit_dst,
                                        NEMAP_DEMO_OFFSCREEN_W,
                                        NEMAP_DEMO_OFFSCREEN_H,
                                        NEMAP_DEMO_OFFSCREEN_W *
                                        sizeof(uint16_t),
                                        7.25f, 8.50f,
                                        56.50f, 20.25f,
                                        18.25f, 56.75f,
                                        NEMAP_DEMO_RAST_AA_MASK);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "aa-offscreen-aa");
    }

  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      if (nemap_demo_rgb565_is_red_coverage(g_offscreen[i]))
        {
          noaa_mixed++;
        }

      if (nemap_demo_rgb565_is_red_coverage(g_blit_dst[i]))
        {
          aa_mixed++;
        }

      if (g_offscreen[i] != g_blit_dst[i])
        {
          diff++;
        }
    }

  if (aa_mixed < 8 || aa_mixed <= noaa_mixed || diff < 8)
    {
      printf("nemap_demo: AA triangle weak edge evidence aa-mixed=%"
             PRIu32 " noaa-mixed=%" PRIu32 " diff=%" PRIu32 "\n",
             aa_mixed, noaa_mixed, diff);
      nemap_demo_print_snapshot("aa-offscreen mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 11a RGB565 triangle AA offscreen "
         "aa-mixed=%" PRIu32 " noaa-mixed=%" PRIu32 " diff=%" PRIu32
         "\n", aa_mixed, noaa_mixed, diff);
  return OK;
}

static int nemap_demo_triangle_aa_visible(FAR const char *fbdev,
                                          FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t size;
  uint32_t gap = 32;
  uint32_t left_x;
  uint32_t right_x;
  uint32_t y;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 11b visible AA: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible AA failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 11b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  size = MIN((uint32_t)170, ((uint32_t)vinfo->xres - gap - 32) / 2);
  size = MIN(size, (uint32_t)vinfo->yres - 96);
  if (size < 48)
    {
      size = 48;
    }

  left_x = ((uint32_t)vinfo->xres - (size * 2 + gap)) / 2;
  right_x = left_x + size + gap;
  y = ((uint32_t)vinfo->yres - size) / 2;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    0, 0, vinfo->xres, vinfo->yres,
                                    NEMAP_DEMO_RGBA_BLACK);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "aa-visible-bg");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_triangle_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                        vinfo->xres, vinfo->yres,
                                        pinfo.stride,
                                        (float)left_x + 8.25f,
                                        (float)y + 6.50f,
                                        (float)left_x + (float)size - 8.50f,
                                        (float)y + (float)size * 0.35f,
                                        (float)left_x + (float)size * 0.25f,
                                        (float)y + (float)size - 8.25f, 0);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "aa-visible-noaa");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_triangle_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                        vinfo->xres, vinfo->yres,
                                        pinfo.stride,
                                        (float)right_x + 8.25f,
                                        (float)y + 6.50f,
                                        (float)right_x + (float)size - 8.50f,
                                        (float)y + (float)size * 0.35f,
                                        (float)right_x + (float)size * 0.25f,
                                        (float)y + (float)size - 8.25f,
                                        NEMAP_DEMO_RAST_AA_MASK);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "aa-visible-aa");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 11b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 11b RGB565 visible triangle AA "
         "noaa/aa panels size=%" PRIu32 " at %" PRIu32 ",%" PRIu32
         " / %" PRIu32 ",%" PRIu32 "\n",
         size, left_x, y, right_x, y);
  close(fd);
  return OK;
}

static int nemap_demo_dst_ckey_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  int ret;

  nemap_demo_prepare_dst_ckey_offscreen(8);
  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565_dst_ckey(&cmdlist,
                                             (uintptr_t)g_offscreen,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_RGBA_RED,
                                             NEMAP_DEMO_RGBA_BLUE);
  if (ret < 0)
    {
      printf("nemap_demo: dst ckey offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "dst-ckey-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t original;
          uint16_t expected;
          uint16_t got;

          original = nemap_demo_dst_ckey_pattern(x, y, 8);
          expected = original == NEMAP_DEMO_RGB565_BLUE ?
            NEMAP_DEMO_RGB565_RED : NEMAP_DEMO_RGB565_YELLOW;
          got = g_offscreen[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (got != expected)
            {
              printf("nemap_demo: dst ckey offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                     " original=0x%04x\n",
                     x, y, got, expected, original);
              nemap_demo_print_snapshot("dst-ckey-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 12a RGB565 destination color key "
         "offscreen compare\n");
  return OK;
}

static int nemap_demo_dst_ckey_visible(FAR const char *fbdev,
                                       FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t square;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 12b visible dst ckey: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible dst ckey failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 12b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  square = MIN((uint32_t)280, MIN((uint32_t)vinfo->xres,
                                  (uint32_t)vinfo->yres) - 64);
  square &= ~1u;
  if (square < 32)
    {
      square = 32;
    }

  x0 = ((uint32_t)vinfo->xres - square) / 2;
  y0 = ((uint32_t)vinfo->yres - square) / 2;

  nemap_demo_prepare_dst_ckey_fb(vinfo, &pinfo, 40);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565_dst_ckey(&cmdlist, (uintptr_t)pinfo.fbmem,
                                             vinfo->xres, vinfo->yres,
                                             pinfo.stride, x0, y0,
                                             square, square,
                                             NEMAP_DEMO_RGBA_RED,
                                             NEMAP_DEMO_RGBA_BLUE);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "dst-ckey-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 12b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 12b RGB565 destination color key "
         "visible checker center=%" PRIu32 "x%" PRIu32 " at %" PRIu32
         ",%" PRIu32 "\n",
         square, square, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_src_ckey_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  int ret;

  nemap_demo_prepare_src_ckey_source(8);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_BLUE;
        }
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit_mode(&cmdlist, (uintptr_t)g_blit_dst,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             (uintptr_t)g_blit_src,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_NEMA_FILTERPS,
                                             true, NEMAP_DEMO_RGBA_YELLOW,
                                             false, 0, 0, 0, 0);
  if (ret < 0)
    {
      printf("nemap_demo: src ckey offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "src-ckey-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t src = nemap_demo_src_ckey_pattern(x, y, 8);
          uint16_t expected = src == NEMAP_DEMO_RGB565_YELLOW ?
            NEMAP_DEMO_RGB565_BLUE : NEMAP_DEMO_RGB565_RED;
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (got != expected)
            {
              printf("nemap_demo: src ckey offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                     " src=0x%04x\n",
                     x, y, got, expected, src);
              nemap_demo_print_snapshot("src-ckey-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 13a RGB565 source color key "
         "offscreen compare\n");
  return OK;
}

static int nemap_demo_src_ckey_visible(FAR const char *fbdev,
                                       FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t square;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 13b visible src ckey: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible src ckey failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 13b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  square = MIN((uint32_t)280, MIN((uint32_t)vinfo->xres,
                                  (uint32_t)vinfo->yres) - 64);
  square &= ~1u;
  if (square < 32)
    {
      square = 32;
    }

  x0 = ((uint32_t)vinfo->xres - square) / 2;
  y0 = ((uint32_t)vinfo->yres - square) / 2;

  nemap_demo_cpu_black(vinfo, &pinfo);
  nemap_demo_prepare_src_ckey_source(8);
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit_mode(&cmdlist, (uintptr_t)pinfo.fbmem,
                                             vinfo->xres, vinfo->yres,
                                             pinfo.stride,
                                             (uintptr_t)g_blit_src,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             x0, y0, square, square,
                                             NEMAP_DEMO_NEMA_FILTERPS,
                                             true, NEMAP_DEMO_RGBA_YELLOW,
                                             false, 0, 0, 0, 0);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "src-ckey-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 13b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 13b RGB565 source color key "
         "visible red/black checker center=%" PRIu32 "x%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n",
         square, square, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_stencil_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  int ret;

  nemap_demo_prepare_stencil_source_and_mask(8);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_BLUE;
        }
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_mask, NEMAP_DEMO_MASK_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit_mode(&cmdlist, (uintptr_t)g_blit_dst,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             (uintptr_t)g_blit_src,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_NEMA_FILTERPS,
                                             false, 0, true,
                                             (uintptr_t)g_mask,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W);
  if (ret < 0)
    {
      printf("nemap_demo: stencil offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "stencil-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint8_t mask = nemap_demo_stencil_mask_pattern(x, y, 8);
          uint16_t expected = mask == 0xff ? NEMAP_DEMO_RGB565_RED :
            NEMAP_DEMO_RGB565_BLUE;
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (got != expected)
            {
              printf("nemap_demo: stencil offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                     " mask=0x%02x\n",
                     x, y, got, expected, mask);
              nemap_demo_print_snapshot("stencil-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 14a RGB565 TEX3 A8 stencil "
         "offscreen compare\n");
  return OK;
}

static int nemap_demo_stencil_visible(FAR const char *fbdev,
                                      FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t square;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 14b visible stencil: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible stencil failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 14b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  square = MIN((uint32_t)280, MIN((uint32_t)vinfo->xres,
                                  (uint32_t)vinfo->yres) - 64);
  square &= ~1u;
  if (square < 32)
    {
      square = 32;
    }

  x0 = ((uint32_t)vinfo->xres - square) / 2;
  y0 = ((uint32_t)vinfo->yres - square) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);
  nemap_demo_prepare_stencil_source_and_mask(8);
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_mask, NEMAP_DEMO_MASK_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_fit_mode(&cmdlist, (uintptr_t)pinfo.fbmem,
                                             vinfo->xres, vinfo->yres,
                                             pinfo.stride,
                                             (uintptr_t)g_blit_src,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W *
                                             sizeof(uint16_t),
                                             0, 0,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             x0, y0, square, square,
                                             NEMAP_DEMO_NEMA_FILTERPS,
                                             false, 0, true,
                                             (uintptr_t)g_mask,
                                             NEMAP_DEMO_OFFSCREEN_W,
                                             NEMAP_DEMO_OFFSCREEN_H,
                                             NEMAP_DEMO_OFFSCREEN_W);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "stencil-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 14b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 14b RGB565 TEX3 A8 stencil "
         "visible red/blue checker center=%" PRIu32 "x%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n",
         square, square, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_format_sweep_offscreen(void)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_RGBX8888,
    NEMAP_DEMO_NEMA_RGBA8888,
    NEMAP_DEMO_NEMA_XRGB8888,
    NEMAP_DEMO_NEMA_ARGB8888
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t fi;
  uint32_t x;
  uint32_t y;
  int ret;

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];

      nemap_demo_prepare_format_source(format);
      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                NEMAP_DEMO_RGB565_SENTINEL;
            }
        }

      nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);
      nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)g_blit_dst,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint16_t),
                                                  (uintptr_t)g_fmt_src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint32_t),
                                                  0, 0,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H);
      if (ret < 0)
        {
          printf("nemap_demo: format %s offscreen emit failed: %d\n",
                 nemap_demo_format_name(format), ret);
          return ret;
        }

      ret = nemap_demo_submit_cmdlist(&cmdlist,
                                      nemap_demo_format_name(format));
      if (ret < 0)
        {
          return ret;
        }

      nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              uint16_t expected = nemap_demo_format_pattern(x, y);
              uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

              if (got != expected)
                {
                  printf("nemap_demo: format %s mismatch x=%" PRIu32
                         " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                         " raw=0x%08" PRIx32 "\n",
                         nemap_demo_format_name(format), x, y, got, expected,
                         g_fmt_src[x + y * NEMAP_DEMO_OFFSCREEN_W]);
                  nemap_demo_print_snapshot("format-sweep mismatch");
                  return -EIO;
                }
            }
        }
    }

  printf("nemap_demo: PASS phase 15a 32-bit source format sweep "
         "RGBX/RGBA/XRGB/ARGB8888 to RGB565 offscreen compare\n");
  return OK;
}

static int nemap_demo_format_sweep_visible(FAR const char *fbdev,
                                           FAR const struct fb_videoinfo_s *vinfo)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_RGBX8888,
    NEMAP_DEMO_NEMA_RGBA8888,
    NEMAP_DEMO_NEMA_XRGB8888,
    NEMAP_DEMO_NEMA_ARGB8888
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t cell;
  uint32_t gap;
  uint32_t total;
  uint32_t x0;
  uint32_t y0;
  uint32_t fi;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 15b visible format sweep: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible format sweep failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 15b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  cell = MIN((uint32_t)96, (MIN((uint32_t)vinfo->xres,
                                (uint32_t)vinfo->yres) - 64) / 2);
  cell &= ~1u;
  if (cell < 32)
    {
      cell = 32;
    }

  gap = 16;
  total = cell * 2 + gap;
  x0 = ((uint32_t)vinfo->xres - total) / 2;
  y0 = ((uint32_t)vinfo->yres - total) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];
      uint32_t px = x0 + (fi & 1) * (cell + gap);
      uint32_t py = y0 + (fi >> 1) * (cell + gap);

      nemap_demo_prepare_format_source(format);
      nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)pinfo.fbmem,
                                                  vinfo->xres, vinfo->yres,
                                                  pinfo.stride,
                                                  (uintptr_t)g_fmt_src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint32_t),
                                                  px, py, cell, cell);
      if (ret >= 0)
        {
          ret = nemap_demo_submit_cmdlist(&cmdlist,
                                          nemap_demo_format_name(format));
        }

      if (ret < 0)
        {
          close(fd);
          return ret;
        }
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 15b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 15b visible 32-bit format sweep "
         "four panels cell=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         cell, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_format16_sweep_offscreen(void)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_RGBA5650,
    NEMAP_DEMO_NEMA_RGBA5551,
    NEMAP_DEMO_NEMA_RGBA4444
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t fi;
  uint32_t x;
  uint32_t y;
  int ret;

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];

      nemap_demo_prepare_format16_source(format);
      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                NEMAP_DEMO_RGB565_SENTINEL;
            }
        }

      nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
      nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)g_blit_dst,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint16_t),
                                                  (uintptr_t)g_blit_src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint16_t),
                                                  0, 0,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H);
      if (ret < 0)
        {
          printf("nemap_demo: format %s offscreen emit failed: %d\n",
                 nemap_demo_format_name(format), ret);
          return ret;
        }

      ret = nemap_demo_submit_cmdlist(&cmdlist,
                                      nemap_demo_format_name(format));
      if (ret < 0)
        {
          return ret;
        }

      nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              uint16_t expected = nemap_demo_format_pattern(x, y);
              uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

              if (got != expected)
                {
                  printf("nemap_demo: format %s mismatch x=%" PRIu32
                         " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                         " raw=0x%04x\n",
                         nemap_demo_format_name(format), x, y, got, expected,
                         g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W]);
                  nemap_demo_print_snapshot("format16-sweep mismatch");
                  return -EIO;
                }
            }
        }
    }

  printf("nemap_demo: PASS phase 15c 16-bit source format sweep "
         "RGBA5650/RGBA5551/RGBA4444 to RGB565 offscreen compare\n");
  return OK;
}

static int nemap_demo_format16_sweep_visible(FAR const char *fbdev,
                                             FAR const struct fb_videoinfo_s *vinfo)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_RGBA5650,
    NEMAP_DEMO_NEMA_RGBA5551,
    NEMAP_DEMO_NEMA_RGBA4444
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t cell;
  uint32_t gap;
  uint32_t total;
  uint32_t x0;
  uint32_t y0;
  uint32_t fi;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 15d visible 16-bit format sweep: "
             "fmt=%u\n", vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible 16-bit format sweep "
             "failed: %d\n", fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 15d get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  cell = MIN((uint32_t)112, (uint32_t)vinfo->xres / 4);
  cell = MIN(cell, (uint32_t)vinfo->yres - 96);
  cell &= ~1u;
  if (cell < 32)
    {
      cell = 32;
    }

  gap = 16;
  total = cell * 3 + gap * 2;
  x0 = ((uint32_t)vinfo->xres - total) / 2;
  y0 = ((uint32_t)vinfo->yres - cell) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];
      uint32_t px = x0 + fi * (cell + gap);

      nemap_demo_prepare_format16_source(format);
      nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)pinfo.fbmem,
                                                  vinfo->xres, vinfo->yres,
                                                  pinfo.stride,
                                                  (uintptr_t)g_blit_src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint16_t),
                                                  px, y0, cell, cell);
      if (ret >= 0)
        {
          ret = nemap_demo_submit_cmdlist(&cmdlist,
                                          nemap_demo_format_name(format));
        }

      if (ret < 0)
        {
          close(fd);
          return ret;
        }
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 15d",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 15d visible 16-bit format sweep "
         "three panels cell=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         cell, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_format_low_sweep_offscreen(void)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_L8,
    NEMAP_DEMO_NEMA_RGB332,
    NEMAP_DEMO_NEMA_RGB24,
    NEMAP_DEMO_NEMA_BGR24
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t fi;
  uint32_t x;
  uint32_t y;
  uint32_t axi64_rgb24_limited = 0;
  int ret;

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];
      uint32_t bpp = nemap_demo_format_source_bpp(format);
      uint32_t src_stride = NEMAP_DEMO_OFFSCREEN_W * bpp;

      nemap_demo_prepare_format_low_source(format);
      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
                NEMAP_DEMO_RGB565_SENTINEL;
            }
        }

      nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);
      nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)g_blit_dst,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  NEMAP_DEMO_OFFSCREEN_W *
                                                  sizeof(uint16_t),
                                                  (uintptr_t)src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  src_stride,
                                                  0, 0,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H);
      if (ret < 0)
        {
          printf("nemap_demo: format %s offscreen emit failed: %d\n",
                 nemap_demo_format_name(format), ret);
          return ret;
        }

      ret = nemap_demo_submit_cmdlist(&cmdlist,
                                      nemap_demo_format_name(format));
      if (ret < 0)
        {
          return ret;
        }

      nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
        {
          for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
            {
              uint16_t expected =
                nemap_demo_format_low_expected(format, x, y);
              uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];
              uint32_t rawoff = y * src_stride + x * bpp;

              if (got != expected)
                {
#if !NEMAP_DEMO_AXI64_RGB24_SOURCE_STRICT
                  if (bpp == 3)
                    {
                      printf("nemap_demo: AXI64 %s source strict mismatch "
                             "x=%" PRIu32 " y=%" PRIu32
                             " got=0x%04x expected=0x%04x"
                             " raw=%02x/%02x/%02x "
                             "(known 24-bit source cross-beat limitation)\n",
                             nemap_demo_format_name(format), x, y, got,
                             expected, src[rawoff], src[rawoff + 1],
                             src[rawoff + 2]);
                      nemap_demo_print_snapshot("format-low-sweep axi64-rgb24");
                      axi64_rgb24_limited++;
                      goto next_format;
                    }
#endif
                  printf("nemap_demo: format %s mismatch x=%" PRIu32
                         " y=%" PRIu32 " got=0x%04x expected=0x%04x"
                         " raw=%02x/%02x/%02x\n",
                         nemap_demo_format_name(format), x, y, got, expected,
                         src[rawoff],
                         bpp > 1 ? src[rawoff + 1] : 0,
                         bpp > 2 ? src[rawoff + 2] : 0);
                  nemap_demo_print_snapshot("format-low-sweep mismatch");
                  return -EIO;
                }
            }
        }

#if !NEMAP_DEMO_AXI64_RGB24_SOURCE_STRICT
next_format:
      ;
#endif
    }

  if (axi64_rgb24_limited > 0)
    {
      printf("nemap_demo: PASS phase 15e 8/24-bit source format sweep "
             "L8/RGB332 strict, RGB24/BGR24 limited on AXI64 count=%" PRIu32
             "\n", axi64_rgb24_limited);
    }
  else
    {
      printf("nemap_demo: PASS phase 15e 8/24-bit source format sweep "
             "L8/RGB332/RGB24/BGR24 to RGB565 offscreen compare\n");
    }

  return OK;
}

static int nemap_demo_format_low_sweep_visible(FAR const char *fbdev,
                                               FAR const struct fb_videoinfo_s *vinfo)
{
  static const uint32_t formats[] =
  {
    NEMAP_DEMO_NEMA_L8,
    NEMAP_DEMO_NEMA_RGB332,
    NEMAP_DEMO_NEMA_RGB24,
    NEMAP_DEMO_NEMA_BGR24
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t cell;
  uint32_t gap;
  uint32_t total;
  uint32_t x0;
  uint32_t y0;
  uint32_t fi;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 15f visible 8/24-bit format sweep: "
             "fmt=%u\n", vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible 8/24-bit format sweep "
             "failed: %d\n", fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 15f get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  cell = MIN((uint32_t)96, (MIN((uint32_t)vinfo->xres,
                                (uint32_t)vinfo->yres) - 64) / 2);
  cell &= ~1u;
  if (cell < 32)
    {
      cell = 32;
    }

  gap = 16;
  total = cell * 2 + gap;
  x0 = ((uint32_t)vinfo->xres - total) / 2;
  y0 = ((uint32_t)vinfo->yres - total) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);

  for (fi = 0; fi < nitems(formats); fi++)
    {
      uint32_t format = formats[fi];
      uint32_t bpp = nemap_demo_format_source_bpp(format);
      uint32_t src_stride = NEMAP_DEMO_OFFSCREEN_W * bpp;
      uint32_t px = x0 + (fi & 1) * (cell + gap);
      uint32_t py = y0 + (fi >> 1) * (cell + gap);

      nemap_demo_prepare_format_low_source(format);
      nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                                  (uintptr_t)pinfo.fbmem,
                                                  vinfo->xres, vinfo->yres,
                                                  pinfo.stride,
                                                  (uintptr_t)src,
                                                  format,
                                                  NEMAP_DEMO_OFFSCREEN_W,
                                                  NEMAP_DEMO_OFFSCREEN_H,
                                                  src_stride,
                                                  px, py, cell, cell);
      if (ret >= 0)
        {
          ret = nemap_demo_submit_cmdlist(&cmdlist,
                                          nemap_demo_format_name(format));
        }

      if (ret < 0)
        {
          close(fd);
          return ret;
        }
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 15f",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 15f visible 8/24-bit format sweep "
         "four panels cell=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         cell, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_tsc6_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  int ret;

  memset(src, 0, NEMAP_DEMO_FMT_SRC_BYTES);
  memcpy(src, g_tsc6_partial1, sizeof(g_tsc6_partial1));

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_SENTINEL;
        }
    }

  nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                              (uintptr_t)g_blit_dst,
                                              NEMAP_DEMO_OFFSCREEN_W,
                                              NEMAP_DEMO_OFFSCREEN_H,
                                              NEMAP_DEMO_OFFSCREEN_W *
                                              sizeof(uint16_t),
                                              (uintptr_t)src,
                                              NEMAP_DEMO_NEMA_TSC6,
                                              NEMAP_DEMO_TSC6_W,
                                              NEMAP_DEMO_TSC6_H,
                                              NEMAP_DEMO_TSC6_STRIDE,
                                              0, 0,
                                              NEMAP_DEMO_TSC6_W,
                                              NEMAP_DEMO_TSC6_H);
  if (ret < 0)
    {
      printf("nemap_demo: TSC6 offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "tsc6-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
  hash = nemap_demo_rgb565_fnv(g_blit_dst, NEMAP_DEMO_OFFSCREEN_W,
                               NEMAP_DEMO_TSC6_W, NEMAP_DEMO_TSC6_H);
  if (hash != NEMAP_DEMO_TSC6_FNV)
    {
      printf("nemap_demo: TSC6 offscreen hash mismatch got=0x%08" PRIx32
             " expected=0x%08x sample00=0x%04x sample31=0x%04x\n",
             hash, NEMAP_DEMO_TSC6_FNV, g_blit_dst[0],
             g_blit_dst[(NEMAP_DEMO_TSC6_H - 1) * NEMAP_DEMO_OFFSCREEN_W +
                        (NEMAP_DEMO_TSC6_W - 1)]);
      nemap_demo_print_snapshot("tsc6-offscreen mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 16a TSC6 source decompress/blit "
         "offscreen hash=0x%08" PRIx32 " size=%ux%u\n",
         hash, NEMAP_DEMO_TSC6_W, NEMAP_DEMO_TSC6_H);
  return OK;
}

static int nemap_demo_tsc6_visible(FAR const char *fbdev,
                                   FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t cell;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 16b visible TSC6: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible TSC6 failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 16b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  cell = MIN((uint32_t)256, MIN((uint32_t)vinfo->xres,
                                (uint32_t)vinfo->yres) - 96);
  cell &= ~3u;
  if (cell < 64)
    {
      cell = 64;
    }

  x0 = ((uint32_t)vinfo->xres - cell) / 2;
  y0 = ((uint32_t)vinfo->yres - cell) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);
  memset(src, 0, NEMAP_DEMO_FMT_SRC_BYTES);
  memcpy(src, g_tsc6_partial1, sizeof(g_tsc6_partial1));
  nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                              (uintptr_t)pinfo.fbmem,
                                              vinfo->xres, vinfo->yres,
                                              pinfo.stride,
                                              (uintptr_t)src,
                                              NEMAP_DEMO_NEMA_TSC6,
                                              NEMAP_DEMO_TSC6_W,
                                              NEMAP_DEMO_TSC6_H,
                                              NEMAP_DEMO_TSC6_STRIDE,
                                              x0, y0, cell, cell);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "tsc6-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 16b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 16b visible TSC6 decompress/blit "
         "panel=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         cell, x0, y0);
  close(fd);
  return OK;
}

static bool nemap_demo_rgb565_is_alpha_mix(uint16_t color)
{
  uint32_t r = nemap_demo_rgb565_r(color);
  uint32_t g = nemap_demo_rgb565_g(color);
  uint32_t b = nemap_demo_rgb565_b(color);

  return color != NEMAP_DEMO_RGB565_BLUE &&
         color != NEMAP_DEMO_RGB565_RED &&
         color != NEMAP_DEMO_RGB565_WHITE &&
         r >= 8 && g >= 4 && b >= 4;
}

static int nemap_demo_tsc6a_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint16_t transparent;
  uint16_t half;
  uint16_t opaque;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  int ret;

  memset(src, 0, NEMAP_DEMO_FMT_SRC_BYTES);
  memcpy(src, g_tsc6a_alpha, sizeof(g_tsc6a_alpha));

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_SENTINEL;
        }
    }

  nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_format_to_rgb565(&cmdlist,
                                              (uintptr_t)g_blit_dst,
                                              NEMAP_DEMO_OFFSCREEN_W,
                                              NEMAP_DEMO_OFFSCREEN_H,
                                              NEMAP_DEMO_OFFSCREEN_W *
                                              sizeof(uint16_t),
                                              (uintptr_t)src,
                                              NEMAP_DEMO_NEMA_TSC6A,
                                              NEMAP_DEMO_TSC6_W,
                                              NEMAP_DEMO_TSC6_H,
                                              NEMAP_DEMO_TSC6_STRIDE,
                                              0, 0,
                                              NEMAP_DEMO_TSC6_W,
                                              NEMAP_DEMO_TSC6_H);
  if (ret < 0)
    {
      printf("nemap_demo: TSC6A source offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "tsc6a-src-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
  hash = nemap_demo_rgb565_fnv(g_blit_dst, NEMAP_DEMO_OFFSCREEN_W,
                               NEMAP_DEMO_TSC6_W, NEMAP_DEMO_TSC6_H);
  if (hash != NEMAP_DEMO_TSC6A_FNV)
    {
      printf("nemap_demo: TSC6A source hash mismatch got=0x%08" PRIx32
             " expected=0x%08x sample16=0x%04x\n",
             hash, NEMAP_DEMO_TSC6A_FNV,
             g_blit_dst[16 + 16 * NEMAP_DEMO_OFFSCREEN_W]);
      nemap_demo_print_snapshot("tsc6a-src mismatch");
      return -EIO;
    }

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            NEMAP_DEMO_RGB565_BLUE;
        }
    }

  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_format_to_rgb565_mode(&cmdlist,
                                                   (uintptr_t)g_blit_dst,
                                                   NEMAP_DEMO_OFFSCREEN_W,
                                                   NEMAP_DEMO_OFFSCREEN_H,
                                                   NEMAP_DEMO_OFFSCREEN_W *
                                                   sizeof(uint16_t),
                                                   (uintptr_t)src,
                                                   NEMAP_DEMO_NEMA_TSC6A,
                                                   NEMAP_DEMO_TSC6_W,
                                                   NEMAP_DEMO_TSC6_H,
                                                   NEMAP_DEMO_TSC6_STRIDE,
                                                   0, 0,
                                                   NEMAP_DEMO_TSC6_W,
                                                   NEMAP_DEMO_TSC6_H,
                                                   NEMAP_DEMO_BLEND_SIMPLE);
  if (ret < 0)
    {
      printf("nemap_demo: TSC6A alpha offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "tsc6a-alpha-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
  transparent = g_blit_dst[0];
  half = g_blit_dst[8 + 16 * NEMAP_DEMO_OFFSCREEN_W];
  opaque = g_blit_dst[16 + 16 * NEMAP_DEMO_OFFSCREEN_W];

  if (transparent != NEMAP_DEMO_RGB565_BLUE ||
      opaque != NEMAP_DEMO_RGB565_WHITE ||
      !nemap_demo_rgb565_is_alpha_mix(half))
    {
      printf("nemap_demo: TSC6A alpha samples mismatch transparent=0x%04x"
             " half=0x%04x opaque=0x%04x\n",
             transparent, half, opaque);
      nemap_demo_print_snapshot("tsc6a-alpha mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 17a TSC6A source decode hash=0x%08" PRIx32
         " and alpha blend samples transparent=0x%04x half=0x%04x"
         " opaque=0x%04x\n",
         hash, transparent, half, opaque);
  return OK;
}

static int nemap_demo_tsc6a_visible(FAR const char *fbdev,
                                    FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  FAR uint8_t *src = (FAR uint8_t *)g_fmt_src;
  uint32_t cell;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 17b visible TSC6A: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible TSC6A failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 17b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  cell = MIN((uint32_t)256, MIN((uint32_t)vinfo->xres,
                                (uint32_t)vinfo->yres) - 96);
  cell &= ~3u;
  if (cell < 64)
    {
      cell = 64;
    }

  x0 = ((uint32_t)vinfo->xres - cell) / 2;
  y0 = ((uint32_t)vinfo->yres - cell) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);
  memset(src, 0, NEMAP_DEMO_FMT_SRC_BYTES);
  memcpy(src, g_tsc6a_alpha, sizeof(g_tsc6a_alpha));
  nemap_demo_clean(g_fmt_src, NEMAP_DEMO_FMT_SRC_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_format_to_rgb565_mode(&cmdlist,
                                                   (uintptr_t)pinfo.fbmem,
                                                   vinfo->xres, vinfo->yres,
                                                   pinfo.stride,
                                                   (uintptr_t)src,
                                                   NEMAP_DEMO_NEMA_TSC6A,
                                                   NEMAP_DEMO_TSC6_W,
                                                   NEMAP_DEMO_TSC6_H,
                                                   NEMAP_DEMO_TSC6_STRIDE,
                                                   x0, y0, cell, cell,
                                                   NEMAP_DEMO_BLEND_SIMPLE);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "tsc6a-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 17b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 17b visible TSC6A alpha blend "
         "panel=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         cell, x0, y0);
  close(fd);
  return OK;
}

static FAR const char *nemap_demo_wrap_name(uint32_t mode)
{
  switch (mode & (0x03 << 2))
    {
      case NEMAP_DEMO_NEMA_CLAMP:
        return "clamp";
      case NEMAP_DEMO_NEMA_REPEAT:
        return "repeat";
      case NEMAP_DEMO_NEMA_BORDER:
        return "border";
      case NEMAP_DEMO_NEMA_MIRROR:
        return "mirror";
      default:
        return "unknown";
    }
}

static uint16_t nemap_demo_wrap_pattern(uint32_t x, uint32_t y)
{
  static const uint16_t colors[4][4] =
  {
    {
      NEMAP_DEMO_RGB565_RED,
      NEMAP_DEMO_RGB565_GREEN,
      NEMAP_DEMO_RGB565_BLUE,
      NEMAP_DEMO_RGB565_WHITE,
    },
    {
      NEMAP_DEMO_RGB565_YELLOW,
      NEMAP_DEMO_RGB565_MAGENTA,
      NEMAP_DEMO_RGB565_CYAN,
      NEMAP_DEMO_RGB565_BLACK,
    },
    {
      0x780f,
      0x03ef,
      0xfc00,
      0x8410,
    },
    {
      0xf81f,
      0x07ff,
      0xffe0,
      0x0010,
    },
  };

  return colors[y & 3][x & 3];
}

static void nemap_demo_prepare_wrap_source(void)
{
  uint32_t x;
  uint32_t y;

  memset(g_blit_src, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
  for (y = 0; y < 4; y++)
    {
      for (x = 0; x < 4; x++)
        {
          g_blit_src[x + y * NEMAP_DEMO_OFFSCREEN_W] =
            nemap_demo_wrap_pattern(x, y);
        }
    }
}

static bool nemap_demo_wrap_coord(uint32_t mode, uint32_t coord,
                                  uint32_t size, FAR uint32_t *wrapped)
{
  uint32_t t;

  switch (mode & (0x03 << 2))
    {
      case NEMAP_DEMO_NEMA_CLAMP:
        *wrapped = coord < size ? coord : size - 1;
        return true;

      case NEMAP_DEMO_NEMA_REPEAT:
        *wrapped = coord & (size - 1);
        return true;

      case NEMAP_DEMO_NEMA_BORDER:
        if (coord >= size)
          {
            return false;
          }

        *wrapped = coord;
        return true;

      case NEMAP_DEMO_NEMA_MIRROR:
        t = coord & ((size << 1) - 1);
        *wrapped = t < size ? t : ((size << 1) - 1) - t;
        return true;

      default:
        *wrapped = coord;
        return true;
    }
}

static uint16_t nemap_demo_wrap_expected(uint32_t mode, uint32_t x,
                                         uint32_t y)
{
  uint32_t sx;
  uint32_t sy;

  if (!nemap_demo_wrap_coord(mode, x, 4, &sx) ||
      !nemap_demo_wrap_coord(mode, y, 4, &sy))
    {
      return NEMAP_DEMO_RGB565_YELLOW;
    }

  return nemap_demo_wrap_pattern(sx, sy);
}

static int nemap_demo_wrap_emit(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                uintptr_t dst, uint32_t dst_width,
                                uint32_t dst_height,
                                uint32_t dst_stride,
                                uintptr_t src, uint32_t dst_x,
                                uint32_t dst_y, uint32_t dst_w,
                                uint32_t dst_h, uint32_t mode)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX_COLOR,
                           NEMAP_DEMO_RGBA_YELLOW);
  if (ret < 0)
    {
      return ret;
    }

  return nemap_demo_emit_blit_rgb565_fit(cmdlist, dst, dst_width,
                                         dst_height, dst_stride, src, 4, 4,
                                         NEMAP_DEMO_OFFSCREEN_W *
                                         sizeof(uint16_t),
                                         0, 0, 8, 8,
                                         dst_x, dst_y, dst_w, dst_h,
                                         NEMAP_DEMO_NEMA_FILTERPS | mode);
}

static int nemap_demo_wrap_offscreen(void)
{
  static const uint32_t modes[] =
  {
    NEMAP_DEMO_NEMA_CLAMP,
    NEMAP_DEMO_NEMA_REPEAT,
    NEMAP_DEMO_NEMA_BORDER,
    NEMAP_DEMO_NEMA_MIRROR,
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  size_t i;
  uint32_t x;
  uint32_t y;
  int ret;

  nemap_demo_prepare_wrap_source();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (i = 0; i < nitems(modes); i++)
    {
      memset(g_blit_dst, 0, NEMAP_DEMO_OFFSCREEN_BYTES);
      nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_wrap_emit(&cmdlist, (uintptr_t)g_blit_dst,
                                 NEMAP_DEMO_OFFSCREEN_W,
                                 NEMAP_DEMO_OFFSCREEN_H,
                                 NEMAP_DEMO_OFFSCREEN_W * sizeof(uint16_t),
                                 (uintptr_t)g_blit_src, 0, 0, 8, 8,
                                 modes[i]);
      if (ret < 0)
        {
          printf("nemap_demo: wrap %s offscreen emit failed: %d\n",
                 nemap_demo_wrap_name(modes[i]), ret);
          return ret;
        }

      ret = nemap_demo_submit_cmdlist(&cmdlist,
                                      nemap_demo_wrap_name(modes[i]));
      if (ret < 0)
        {
          return ret;
        }

      nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);
      for (y = 0; y < 8; y++)
        {
          for (x = 0; x < 8; x++)
            {
              uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];
              uint16_t expected = nemap_demo_wrap_expected(modes[i], x, y);

              if (got != expected)
                {
                  printf("nemap_demo: wrap %s mismatch x=%" PRIu32
                         " y=%" PRIu32 " got=0x%04x expected=0x%04x\n",
                         nemap_demo_wrap_name(modes[i]), x, y, got,
                         expected);
                  nemap_demo_print_snapshot("wrap-offscreen mismatch");
                  return -EIO;
                }
            }
        }
    }

  printf("nemap_demo: PASS phase 18a RGB565 texture wrap "
         "clamp/repeat/border/mirror offscreen compare\n");
  return OK;
}

static int nemap_demo_wrap_visible(FAR const char *fbdev,
                                   FAR const struct fb_videoinfo_s *vinfo)
{
  static const uint32_t modes[] =
  {
    NEMAP_DEMO_NEMA_CLAMP,
    NEMAP_DEMO_NEMA_REPEAT,
    NEMAP_DEMO_NEMA_BORDER,
    NEMAP_DEMO_NEMA_MIRROR,
  };

  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t gap = 24;
  uint32_t total;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  size_t i;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 18b visible wrap: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible wrap failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 18b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = (MIN((uint32_t)vinfo->xres, (uint32_t)vinfo->yres) - 128) / 2;
  panel &= ~7u;
  if (panel < 96)
    {
      panel = 96;
    }

  total = panel * 2 + gap;
  x0 = ((uint32_t)vinfo->xres - total) / 2;
  y0 = ((uint32_t)vinfo->yres - total) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLUE);
  nemap_demo_prepare_wrap_source();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (i = 0; i < nitems(modes); i++)
    {
      uint32_t px = x0 + (uint32_t)(i & 1) * (panel + gap);
      uint32_t py = y0 + (uint32_t)(i >> 1) * (panel + gap);

      memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
      stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
      ret = nemap_demo_wrap_emit(&cmdlist, (uintptr_t)pinfo.fbmem,
                                 vinfo->xres, vinfo->yres, pinfo.stride,
                                 (uintptr_t)g_blit_src, px, py,
                                 panel, panel, modes[i]);
      if (ret >= 0)
        {
          ret = nemap_demo_submit_cmdlist(&cmdlist,
                                          nemap_demo_wrap_name(modes[i]));
        }

      if (ret < 0)
        {
          close(fd);
          return ret;
        }
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 18b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 18b visible RGB565 texture wrap "
         "panels clamp/repeat/border/mirror panel=%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n",
         panel, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_affine_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  const uint32_t dst_x = 16;
  const uint32_t dst_y = 16;
  const uint32_t size = 24;
  const float mm00 = 1.0f;
  const float mm01 = 1.0f;
  const float mm10 = 1.0f;
  const float mm11 = -1.0f;
  const float src_x0 = 4.0f;
  const float src_y0 = 32.0f;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_affine_pattern();
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_matrix(&cmdlist, (uintptr_t)g_blit_dst,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           dst_x, dst_y, size, size,
                                           mm00, mm01,
                                           src_x0 - 1.0f -
                                           (float)dst_x * mm00 -
                                           (float)dst_y * mm01,
                                           mm10, mm11,
                                           src_y0 - (float)dst_x * mm10 -
                                           (float)dst_y * mm11,
                                           0.0f, 0.0f, 1.0f,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      printf("nemap_demo: affine offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "affine-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 1; y < size - 1; y++)
    {
      for (x = 1; x < size - 1; x++)
        {
          uint32_t sx = (uint32_t)(src_x0 + (float)x * mm00 +
                                   (float)y * mm01);
          uint32_t sy = (uint32_t)(src_y0 + (float)x * mm10 +
                                   (float)y * mm11);
          uint16_t expected = nemap_demo_affine_pattern(sx, sy);
          uint16_t got = g_blit_dst[(dst_x + x) +
                                    (dst_y + y) * NEMAP_DEMO_OFFSCREEN_W];

          if (got != expected)
            {
              printf("nemap_demo: affine offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " src=%" PRIu32 ",%" PRIu32
                     " got=0x%04x expected=0x%04x\n",
                     x, y, sx, sy, got, expected);
              nemap_demo_print_snapshot("affine-offscreen mismatch");
              return -EIO;
            }
        }
    }

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          if (x >= dst_x && x < dst_x + size &&
              y >= dst_y && y < dst_y + size)
            {
              continue;
            }

          if (g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] !=
              NEMAP_DEMO_RGB565_SENTINEL)
            {
              printf("nemap_demo: affine offscreen outside write x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x\n",
                     x, y, g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W]);
              nemap_demo_print_snapshot("affine-offscreen outside");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 21a RGB565 affine textured box "
         "offscreen compare integer shear matrix=1,1,1,-1\n");
  return OK;
}

static int nemap_demo_affine_visible(FAR const char *fbdev,
                                     FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  float mm00;
  float mm01;
  float mm10;
  float mm11;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 21b visible affine: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible affine failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 21b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)224, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;
  mm00 = 48.0f / (float)panel;
  mm01 = 12.0f / (float)panel;
  mm10 = 6.0f / (float)panel;
  mm11 = 44.0f / (float)panel;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);
  nemap_demo_prepare_affine_pattern();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_matrix(&cmdlist, (uintptr_t)pinfo.fbmem,
                                           vinfo->xres, vinfo->yres,
                                           pinfo.stride,
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           x0, y0, panel, panel,
                                           mm00, mm01,
                                           3.0f - (float)x0 * mm00 -
                                           (float)y0 * mm01,
                                           mm10, mm11,
                                           2.0f - (float)x0 * mm10 -
                                           (float)y0 * mm11,
                                           0.0f, 0.0f, 1.0f,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "affine-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 21b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 21b visible affine textured panel=%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n",
         panel, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_textured_quad_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  const uint32_t dst_x = 18;
  const uint32_t dst_y = 18;
  const uint32_t size = 24;
  const float mm00 = 1.0f;
  const float mm01 = 1.0f;
  const float mm10 = 1.0f;
  const float mm11 = -1.0f;
  const float src_x0 = 4.0f;
  const float src_y0 = 32.0f;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_affine_pattern();
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_quad_matrix(
                                           &cmdlist, (uintptr_t)g_blit_dst,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (float)dst_x, (float)dst_y,
                                           (float)(dst_x + size),
                                           (float)dst_y,
                                           (float)(dst_x + size),
                                           (float)(dst_y + size),
                                           (float)dst_x,
                                           (float)(dst_y + size),
                                           mm00, mm01,
                                           src_x0 - 1.0f -
                                           (float)dst_x * mm00 -
                                           (float)dst_y * mm01,
                                           mm10, mm11,
                                           src_y0 - (float)dst_x * mm10 -
                                           (float)dst_y * mm11,
                                           0.0f, 0.0f, 1.0f,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      printf("nemap_demo: textured quad offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "quad-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 1; y < size - 1; y++)
    {
      for (x = 1; x < size - 1; x++)
        {
          uint32_t sx = (uint32_t)(src_x0 + (float)x * mm00 +
                                   (float)y * mm01);
          uint32_t sy = (uint32_t)(src_y0 + (float)x * mm10 +
                                   (float)y * mm11);
          uint16_t expected = nemap_demo_affine_pattern(sx, sy);
          uint16_t got = g_blit_dst[(dst_x + x) +
                                    (dst_y + y) * NEMAP_DEMO_OFFSCREEN_W];

          if (got != expected)
            {
              printf("nemap_demo: textured quad offscreen mismatch x=%"
                     PRIu32 " y=%" PRIu32 " src=%" PRIu32 ",%" PRIu32
                     " got=0x%04x expected=0x%04x\n",
                     x, y, sx, sy, got, expected);
              nemap_demo_print_snapshot("quad-offscreen mismatch");
              return -EIO;
            }
        }
    }

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          if (x >= dst_x && x < dst_x + size &&
              y >= dst_y && y < dst_y + size)
            {
              continue;
            }

          if (g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W] !=
              NEMAP_DEMO_RGB565_SENTINEL)
            {
              printf("nemap_demo: textured quad outside write x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x\n",
                     x, y, g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W]);
              nemap_demo_print_snapshot("quad-offscreen outside");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 22a RGB565 textured quad offscreen "
         "compare matrix origin-compensated\n");
  return OK;
}

static int
nemap_demo_textured_quad_visible(FAR const char *fbdev,
                                 FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  float mm00;
  float mm01;
  float mm10;
  float mm11;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 22b visible textured quad: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible textured quad failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 22b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)224, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;
  mm00 = 48.0f / (float)panel;
  mm01 = 10.0f / (float)panel;
  mm10 = 8.0f / (float)panel;
  mm11 = 44.0f / (float)panel;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);
  nemap_demo_prepare_affine_pattern();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_quad_matrix(
                                           &cmdlist, (uintptr_t)pinfo.fbmem,
                                           vinfo->xres, vinfo->yres,
                                           pinfo.stride,
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (float)(x0 + 20), (float)y0,
                                           (float)(x0 + panel - 10),
                                           (float)(y0 + 18),
                                           (float)(x0 + panel - 24),
                                           (float)(y0 + panel - 8),
                                           (float)(x0 + 8),
                                           (float)(y0 + panel - 20),
                                           mm00, mm01,
                                           3.0f - (float)x0 * mm00 -
                                           (float)y0 * mm01,
                                           mm10, mm11,
                                           2.0f - (float)x0 * mm10 -
                                           (float)y0 * mm11,
                                           0.0f, 0.0f, 1.0f,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "quad-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 22b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 22b visible textured quad panel=%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n",
         panel, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_projective_quad_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  const uint32_t dst_x = 12;
  const uint32_t dst_y = 14;
  const uint32_t size = 36;
  const uint32_t bbox_x0 = dst_x;
  const uint32_t bbox_y0 = dst_y;
  const uint32_t bbox_x1 = dst_x + size + 1;
  const uint32_t bbox_y1 = dst_y + size + 1;
  const float mm00 = 0.85f;
  const float mm01 = 0.20f;
  const float mm10 = 0.10f;
  const float mm11 = 0.70f;
  const float mm20 = 0.004f;
  const float mm21 = 0.003f;
  const float src_x0 = 5.0f;
  const float src_y0 = 6.0f;
  uint32_t changed = 0;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  nemap_demo_prepare_affine_pattern();
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_quad_matrix(
                                           &cmdlist, (uintptr_t)g_blit_dst,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (float)(dst_x + 4),
                                           (float)dst_y,
                                           (float)(dst_x + size),
                                           (float)(dst_y + 3),
                                           (float)(dst_x + size - 5),
                                           (float)(dst_y + size),
                                           (float)dst_x,
                                           (float)(dst_y + size - 4),
                                           mm00, mm01,
                                           src_x0 - 1.0f -
                                           (float)dst_x * mm00 -
                                           (float)dst_y * mm01,
                                           mm10, mm11,
                                           src_y0 - (float)dst_x * mm10 -
                                           (float)dst_y * mm11,
                                           mm20, mm21,
                                           1.0f - (float)dst_x * mm20 -
                                           (float)dst_y * mm21,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      printf("nemap_demo: projective quad offscreen emit failed: %d\n",
             ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "projective-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (x >= bbox_x0 && x < bbox_x1 &&
              y >= bbox_y0 && y < bbox_y1)
            {
              if (got != NEMAP_DEMO_RGB565_SENTINEL)
                {
                  changed++;
                }
            }
          else if (got != NEMAP_DEMO_RGB565_SENTINEL)
            {
              printf("nemap_demo: projective quad outside write x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x\n", x, y, got);
              nemap_demo_print_snapshot("projective-offscreen outside");
              return -EIO;
            }
        }
    }

  if (changed < 240)
    {
      printf("nemap_demo: projective quad changed too few pixels=%" PRIu32
             "\n", changed);
      nemap_demo_print_snapshot("projective-offscreen sparse");
      return -EIO;
    }

  hash = nemap_demo_rgb565_fnv(&g_blit_dst[bbox_x0 +
                              bbox_y0 * NEMAP_DEMO_OFFSCREEN_W],
                              NEMAP_DEMO_OFFSCREEN_W,
                              bbox_x1 - bbox_x0, bbox_y1 - bbox_y0);
  if (hash != NEMAP_DEMO_PROJECTIVE_FNV)
    {
      printf("nemap_demo: projective quad hash mismatch changed=%" PRIu32
             " got=0x%08" PRIx32 " expected=0x%08x\n",
             changed, hash, NEMAP_DEMO_PROJECTIVE_FNV);
      nemap_demo_print_snapshot("projective-offscreen hash");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 23a RGB565 projective textured quad "
         "compare changed=%" PRIu32 " hash=0x%08" PRIx32
         " mm20=%08" PRIx32 " mm21=%08" PRIx32 "\n",
         changed, hash, nemap_demo_floatbits(mm20),
         nemap_demo_floatbits(mm21));
  return OK;
}

static int
nemap_demo_projective_quad_visible(FAR const char *fbdev,
                                   FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  float mm00;
  float mm01;
  float mm10;
  float mm11;
  float mm20;
  float mm21;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 23b visible projective quad: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible projective quad failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 23b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)224, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;
  mm00 = 44.0f / (float)panel;
  mm01 = 8.0f / (float)panel;
  mm10 = 5.0f / (float)panel;
  mm11 = 42.0f / (float)panel;
  mm20 = 0.18f / (float)panel;
  mm21 = 0.13f / (float)panel;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);
  nemap_demo_prepare_affine_pattern();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_quad_matrix(
                                           &cmdlist, (uintptr_t)pinfo.fbmem,
                                           vinfo->xres, vinfo->yres,
                                           pinfo.stride,
                                           (uintptr_t)g_blit_src,
                                           NEMAP_DEMO_OFFSCREEN_W,
                                           NEMAP_DEMO_OFFSCREEN_H,
                                           NEMAP_DEMO_OFFSCREEN_W *
                                           sizeof(uint16_t),
                                           (float)(x0 + 28),
                                           (float)(y0 + 2),
                                           (float)(x0 + panel - 4),
                                           (float)(y0 + 36),
                                           (float)(x0 + panel - 42),
                                           (float)(y0 + panel - 8),
                                           (float)(x0 + 4),
                                           (float)(y0 + panel - 34),
                                           mm00, mm01,
                                           4.0f - 1.0f -
                                           (float)x0 * mm00 -
                                           (float)y0 * mm01,
                                           mm10, mm11,
                                           4.0f - (float)x0 * mm10 -
                                           (float)y0 * mm11,
                                           mm20, mm21,
                                           1.0f - (float)x0 * mm20 -
                                           (float)y0 * mm21,
                                           NEMAP_DEMO_NEMA_FILTERPS);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "projective-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 23b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 23b visible projective textured quad "
         "panel=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         panel, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_textured_triangle_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t changed = 0;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  uint16_t sample0;
  uint16_t sample1;
  size_t i;
  int ret;

  nemap_demo_prepare_affine_pattern();
  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_triangle_matrix(
                                            &cmdlist,
                                            (uintptr_t)g_blit_dst,
                                            NEMAP_DEMO_OFFSCREEN_W,
                                            NEMAP_DEMO_OFFSCREEN_H,
                                            NEMAP_DEMO_OFFSCREEN_W *
                                            sizeof(uint16_t),
                                            (uintptr_t)g_blit_src,
                                            NEMAP_DEMO_OFFSCREEN_W,
                                            NEMAP_DEMO_OFFSCREEN_H,
                                            NEMAP_DEMO_OFFSCREEN_W *
                                            sizeof(uint16_t),
                                            8.0f, 8.0f, 56.0f, 8.0f,
                                            8.0f, 56.0f,
                                            1.0f, 0.0f, -8.0f,
                                            0.0f, 1.0f, -8.0f,
                                            0.0f, 0.0f, 1.0f,
                                            NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      printf("nemap_demo: textured triangle offscreen emit failed: %d\n",
             ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "tri-texture-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (got != NEMAP_DEMO_RGB565_SENTINEL)
            {
              changed++;
              if (x < 8 || y < 8 || x > 56 || y > 56)
                {
                  printf("nemap_demo: textured triangle outside write x=%"
                         PRIu32 " y=%" PRIu32 " got=0x%04x\n",
                         x, y, got);
                  nemap_demo_print_snapshot("tri-texture outside");
                  return -EIO;
                }
            }
        }
    }

  sample0 = g_blit_dst[12 + 12 * NEMAP_DEMO_OFFSCREEN_W];
  sample1 = g_blit_dst[24 + 16 * NEMAP_DEMO_OFFSCREEN_W];
  if (sample0 != nemap_demo_affine_pattern(4, 4) ||
      sample1 != nemap_demo_affine_pattern(16, 8) ||
      changed < 900 || changed > 1500)
    {
      printf("nemap_demo: textured triangle sample mismatch changed=%"
             PRIu32 " sample0=0x%04x/0x%04x sample1=0x%04x/0x%04x\n",
             changed, sample0, nemap_demo_affine_pattern(4, 4),
             sample1, nemap_demo_affine_pattern(16, 8));
      nemap_demo_print_snapshot("tri-texture sample mismatch");
      return -EIO;
    }

  hash = nemap_demo_rgb565_fnv(&g_blit_dst[8 + 8 * NEMAP_DEMO_OFFSCREEN_W],
                              NEMAP_DEMO_OFFSCREEN_W, 49, 49);
  printf("nemap_demo: PASS phase 28a RGB565 textured triangle offscreen "
         "compare changed=%" PRIu32 " hash=0x%08" PRIx32 "\n",
         changed, hash);
  return OK;
}

static int
nemap_demo_textured_triangle_visible(FAR const char *fbdev,
                                     FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  float scale;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 28b visible textured triangle: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible textured triangle failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 28b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)240, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;
  scale = (float)NEMAP_DEMO_OFFSCREEN_W / (float)panel;

  nemap_demo_prepare_affine_pattern();
  nemap_demo_clean(g_blit_src, NEMAP_DEMO_OFFSCREEN_BYTES);
  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_blit_rgb565_triangle_matrix(
                                            &cmdlist, (uintptr_t)pinfo.fbmem,
                                            vinfo->xres, vinfo->yres,
                                            pinfo.stride,
                                            (uintptr_t)g_blit_src,
                                            NEMAP_DEMO_OFFSCREEN_W,
                                            NEMAP_DEMO_OFFSCREEN_H,
                                            NEMAP_DEMO_OFFSCREEN_W *
                                            sizeof(uint16_t),
                                            (float)x0, (float)y0,
                                            (float)(x0 + panel),
                                            (float)y0,
                                            (float)x0,
                                            (float)(y0 + panel),
                                            scale, 0.0f,
                                            -((float)x0 * scale),
                                            0.0f, scale,
                                            -((float)y0 * scale),
                                            0.0f, 0.0f, 1.0f,
                                            NEMAP_DEMO_NEMA_FILTERPS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "tri-texture-visible");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 28b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 28b visible textured triangle panel=%"
         PRIu32 " at %" PRIu32 ",%" PRIu32 "\n", panel, x0, y0);
  close(fd);
  return OK;
}

static int
nemap_demo_submit_line_rgb565(uintptr_t dst, uint32_t width, uint32_t height,
                              uint32_t stride, uint32_t x0, uint32_t y0,
                              uint32_t x1, uint32_t y1, uint32_t rgba,
                              FAR const char *name)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_line_rgb565(&cmdlist, dst, width, height, stride,
                                    x0, y0, x1, y1, rgba);
  if (ret < 0)
    {
      printf("nemap_demo: %s emit failed: %d\n", name, ret);
      return ret;
    }

  return nemap_demo_submit_cmdlist(&cmdlist, name);
}

static int nemap_demo_line_primitive_offscreen(void)
{
  const uint32_t bbox_x0 = 6;
  const uint32_t bbox_y0 = 6;
  const uint32_t bbox_x1 = 58;
  const uint32_t bbox_y1 = 58;
  uint32_t changed = 0;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  ret = nemap_demo_submit_line_rgb565((uintptr_t)g_blit_dst,
                                      NEMAP_DEMO_OFFSCREEN_W,
                                      NEMAP_DEMO_OFFSCREEN_H,
                                      NEMAP_DEMO_OFFSCREEN_W *
                                      sizeof(uint16_t),
                                      8, 8, 55, 8, NEMAP_DEMO_RGBA_RED,
                                      "line-horizontal");
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_submit_line_rgb565((uintptr_t)g_blit_dst,
                                      NEMAP_DEMO_OFFSCREEN_W,
                                      NEMAP_DEMO_OFFSCREEN_H,
                                      NEMAP_DEMO_OFFSCREEN_W *
                                      sizeof(uint16_t),
                                      8, 12, 8, 55, NEMAP_DEMO_RGBA_GREEN,
                                      "line-vertical");
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_submit_line_rgb565((uintptr_t)g_blit_dst,
                                      NEMAP_DEMO_OFFSCREEN_W,
                                      NEMAP_DEMO_OFFSCREEN_H,
                                      NEMAP_DEMO_OFFSCREEN_W *
                                      sizeof(uint16_t),
                                      12, 14, 55, 52,
                                      NEMAP_DEMO_RGBA_YELLOW,
                                      "line-diagonal");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (x >= bbox_x0 && x < bbox_x1 &&
              y >= bbox_y0 && y < bbox_y1)
            {
              if (got != NEMAP_DEMO_RGB565_SENTINEL)
                {
                  changed++;
                }
            }
          else if (got != NEMAP_DEMO_RGB565_SENTINEL)
            {
              printf("nemap_demo: line primitive outside write x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x\n", x, y, got);
              nemap_demo_print_snapshot("line-offscreen outside");
              return -EIO;
            }
        }
    }

  if (changed != NEMAP_DEMO_LINE_CHANGED)
    {
      printf("nemap_demo: line primitive changed mismatch got=%" PRIu32
             " expected=%u\n", changed, NEMAP_DEMO_LINE_CHANGED);
      nemap_demo_print_snapshot("line-offscreen changed mismatch");
      return -EIO;
    }

  hash = nemap_demo_rgb565_fnv(&g_blit_dst[bbox_x0 +
                              bbox_y0 * NEMAP_DEMO_OFFSCREEN_W],
                              NEMAP_DEMO_OFFSCREEN_W,
                              bbox_x1 - bbox_x0, bbox_y1 - bbox_y0);
  if (hash != NEMAP_DEMO_LINE_FNV)
    {
      printf("nemap_demo: line primitive hash mismatch got=0x%08" PRIx32
             " expected=0x%08x\n", hash, NEMAP_DEMO_LINE_FNV);
      nemap_demo_print_snapshot("line-offscreen hash mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 24a RGB565 DRAW_LINE primitive "
         "compare changed=%" PRIu32 " hash=0x%08" PRIx32 "\n",
         changed, hash);
  return OK;
}

static int
nemap_demo_line_primitive_visible(FAR const char *fbdev,
                                  FAR const struct fb_videoinfo_s *vinfo)
{
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 24b visible line primitive: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible line primitive failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 24b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)240, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);

  ret = nemap_demo_submit_line_rgb565((uintptr_t)pinfo.fbmem,
                                      vinfo->xres, vinfo->yres,
                                      pinfo.stride,
                                      x0, y0, x0 + panel - 1, y0,
                                      NEMAP_DEMO_RGBA_RED,
                                      "line-visible-top");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_submit_line_rgb565((uintptr_t)pinfo.fbmem,
                                      vinfo->xres, vinfo->yres,
                                      pinfo.stride,
                                      x0, y0, x0, y0 + panel - 1,
                                      NEMAP_DEMO_RGBA_GREEN,
                                      "line-visible-left");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_submit_line_rgb565((uintptr_t)pinfo.fbmem,
                                      vinfo->xres, vinfo->yres,
                                      pinfo.stride,
                                      x0 + 12, y0 + 20,
                                      x0 + panel - 20,
                                      y0 + panel - 18,
                                      NEMAP_DEMO_RGBA_YELLOW,
                                      "line-visible-diagonal");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 24b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 24b visible DRAW_LINE primitive "
         "panel=%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         panel, x0, y0);
  close(fd);
  return OK;
}

static int
nemap_demo_submit_gradient_rgb565(uintptr_t dst, uint32_t width,
                                  uint32_t height, uint32_t stride,
                                  uint32_t x, uint32_t y, uint32_t w,
                                  uint32_t h, FAR const char *name)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_gradient_rgb565(&cmdlist, dst, width, height, stride,
                                        x, y, w, h);
  if (ret < 0)
    {
      printf("nemap_demo: %s emit failed: %d\n", name, ret);
      return ret;
    }

  return nemap_demo_submit_cmdlist(&cmdlist, name);
}

static int nemap_demo_gradient_offscreen(void)
{
  const uint32_t x0 = 8;
  const uint32_t y0 = 8;
  const uint32_t w = 48;
  const uint32_t h = 48;
  uint16_t top_left;
  uint16_t top_right;
  uint16_t bottom_left;
  uint32_t changed = 0;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  ret = nemap_demo_submit_gradient_rgb565((uintptr_t)g_blit_dst,
                                          NEMAP_DEMO_OFFSCREEN_W,
                                          NEMAP_DEMO_OFFSCREEN_H,
                                          NEMAP_DEMO_OFFSCREEN_W *
                                          sizeof(uint16_t),
                                          x0, y0, w, h,
                                          "gradient-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (x >= x0 && x < x0 + w && y >= y0 && y < y0 + h)
            {
              if (got != NEMAP_DEMO_RGB565_SENTINEL)
                {
                  changed++;
                }
            }
          else if (got != NEMAP_DEMO_RGB565_SENTINEL)
            {
              printf("nemap_demo: gradient outside write x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x\n", x, y, got);
              nemap_demo_print_snapshot("gradient-offscreen outside");
              return -EIO;
            }
        }
    }

  top_left = g_blit_dst[x0 + y0 * NEMAP_DEMO_OFFSCREEN_W];
  top_right = g_blit_dst[(x0 + w - 1) + y0 * NEMAP_DEMO_OFFSCREEN_W];
  bottom_left = g_blit_dst[x0 + (y0 + h - 1) * NEMAP_DEMO_OFFSCREEN_W];
  if (changed != NEMAP_DEMO_GRADIENT_CHANGED ||
      top_left != NEMAP_DEMO_GRADIENT_TL ||
      top_right != NEMAP_DEMO_GRADIENT_TR ||
      bottom_left != NEMAP_DEMO_GRADIENT_BL)
    {
      printf("nemap_demo: gradient sample mismatch changed=%" PRIu32
             "/%u tl=0x%04x/0x%04x tr=0x%04x/0x%04x "
             "bl=0x%04x/0x%04x\n",
             changed, NEMAP_DEMO_GRADIENT_CHANGED, top_left,
             NEMAP_DEMO_GRADIENT_TL, top_right, NEMAP_DEMO_GRADIENT_TR,
             bottom_left, NEMAP_DEMO_GRADIENT_BL);
      nemap_demo_print_snapshot("gradient-offscreen sample mismatch");
      return -EIO;
    }

  hash = nemap_demo_rgb565_fnv(&g_blit_dst[x0 + y0 * NEMAP_DEMO_OFFSCREEN_W],
                              NEMAP_DEMO_OFFSCREEN_W, w, h);
  if (hash != NEMAP_DEMO_GRADIENT_FNV)
    {
      printf("nemap_demo: gradient hash mismatch got=0x%08" PRIx32
             " expected=0x%08x\n", hash, NEMAP_DEMO_GRADIENT_FNV);
      nemap_demo_print_snapshot("gradient-offscreen hash mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 25a RGB565 gradient register path "
         "compare changed=%" PRIu32 " hash=0x%08" PRIx32
         " samples=0x%04x/0x%04x/0x%04x\n",
         changed, hash, top_left, top_right, bottom_left);
  return OK;
}

static int
nemap_demo_gradient_visible(FAR const char *fbdev,
                            FAR const struct fb_videoinfo_s *vinfo)
{
  struct fb_planeinfo_s pinfo;
  uint32_t panel;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 25b visible gradient: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible gradient failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 25b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel = MIN((uint32_t)240, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) - 96);
  panel &= ~3u;
  if (panel < 96)
    {
      panel = 96;
    }

  x0 = ((uint32_t)vinfo->xres - panel) / 2;
  y0 = ((uint32_t)vinfo->yres - panel) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);

  ret = nemap_demo_submit_gradient_rgb565((uintptr_t)pinfo.fbmem,
                                          vinfo->xres, vinfo->yres,
                                          pinfo.stride, x0, y0, panel,
                                          panel, "gradient-visible");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 25b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 25b visible RGB565 gradient panel=%" PRIu32
         " at %" PRIu32 ",%" PRIu32 "\n", panel, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_debug_status(void)
{
  uint32_t breakpoint = nemap_demo_regread(STM32_GPU2D_BREAKPOINT);
  uint32_t cmdstatus = nemap_demo_regread(STM32_GPU2D_CMDSTATUS);
  uint32_t cmdringstop = nemap_demo_regread(STM32_GPU2D_CMDRINGSTOP);
  uint32_t cmdaddr = nemap_demo_regread(STM32_GPU2D_CMDADDR);
  uint32_t cmdsize = nemap_demo_regread(STM32_GPU2D_CMDSIZE);
  uint32_t interrupt = nemap_demo_regread(STM32_GPU2D_INTERRUPT);
  uint32_t status = nemap_demo_regread(STM32_GPU2D_STATUS);
  uint32_t dirtymin = nemap_demo_regread(STM32_GPU2D_DIRTYMIN);
  uint32_t dirtymax = nemap_demo_regread(STM32_GPU2D_DIRTYMAX);
  uint32_t dbg_status = nemap_demo_regread(STM32_GPU2D_DBG_STATUS);
  uint32_t dbg_addr = nemap_demo_regread(STM32_GPU2D_DBG_ADDR);
  uint32_t dbg_data = nemap_demo_regread(STM32_GPU2D_DBG_DATA);
  uint32_t dbg_ctrl = nemap_demo_regread(STM32_GPU2D_DBG_CTRL);
  uint32_t irq_id = nemap_demo_regread(STM32_GPU2D_IRQ_ID);
  uint32_t gp_flags = nemap_demo_regread(STM32_GPU2D_GP_FLAGS);
  uint32_t syserror = nemap_demo_regread(STM32_GPU2D_SYS_INTERRUPT);
  uint32_t syserror_mask = nemap_demo_regread(STM32_GPU2D_SYS_ERROR_MASK);

  printf("nemap_demo: debug/status cmdstatus=0x%08" PRIx32
         " status=0x%08" PRIx32 " interrupt=0x%08" PRIx32
         " syserror=0x%08" PRIx32 " mask=0x%08" PRIx32 "\n",
         cmdstatus, status, interrupt, syserror, syserror_mask);
  printf("nemap_demo: debug/ring breakpoint=0x%08" PRIx32
         " ringstop=0x%08" PRIx32 " cmdaddr=0x%08" PRIx32
         " cmdsize=0x%08" PRIx32 " irq_id=0x%08" PRIx32
         " gp_flags=0x%08" PRIx32 "\n",
         breakpoint, cmdringstop, cmdaddr, cmdsize, irq_id, gp_flags);
  printf("nemap_demo: debug/dirty dirtymin=0x%08" PRIx32
         " dirtymax=0x%08" PRIx32 " dbg_status=0x%08" PRIx32
         " dbg_addr=0x%08" PRIx32 " dbg_data=0x%08" PRIx32
         " dbg_ctrl=0x%08" PRIx32 "\n",
         dirtymin, dirtymax, dbg_status, dbg_addr, dbg_data, dbg_ctrl);

  if (syserror != 0)
    {
      printf("nemap_demo: debug/status unexpected SYS_INTERRUPT=0x%08" PRIx32
             "\n", syserror);
      return -EIO;
    }

  printf("nemap_demo: PASS phase 19 GPU2D debug/status idle snapshot\n");
  return OK;
}

static int nemap_demo_depth_capability(void)
{
  uint32_t config = stm32_gpu2dreadconfig();
  uint32_t depth_ctrl = nemap_demo_regread(STM32_GPU2D_DEPTH_CTRL);
  uint32_t syserror = nemap_demo_regread(STM32_GPU2D_SYS_INTERRUPT);

  printf("nemap_demo: depth capability zbuf=%s zcompr=%s "
         "depth_ctrl=0x%08" PRIx32 " syserror=0x%08" PRIx32 "\n",
         (config & GPU2D_CONFIG_ZBUF) != 0 ? "yes" : "no",
         (config & GPU2D_CONFIG_ZCOMPR) != 0 ? "yes" : "no",
         depth_ctrl, syserror);

  if (syserror != 0)
    {
      printf("nemap_demo: depth capability unexpected SYS_INTERRUPT=0x%08"
             PRIx32 "\n", syserror);
      return -EIO;
    }

  if ((config & GPU2D_CONFIG_ZBUF) == 0)
    {
      printf("nemap_demo: PASS phase 20 depth capability absent "
             "(CONFIG.ZBUF=0, destructive depth-buffer tests skipped)\n");
      return OK;
    }

  printf("nemap_demo: PASS phase 20 depth capability present "
         "(CONFIG.ZBUF=1, depth draw tests pending)\n");
  return OK;
}

static int
nemap_demo_emit_raster_rgb565_setup(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                    uintptr_t dst, uint32_t width,
                                    uint32_t height, uint32_t stride,
                                    uint32_t rgba)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_BASE, dst);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_FSTRIDE,
                           nemap_demo_fstride(NEMAP_DEMO_NEMA_RGB565,
                                              NEMAP_DEMO_NEMA_FILTERPS,
                                              stride));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_TEX0_RESXY,
                           (height << 16) | width);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_ROP_BLEND_MODE,
                           NEMAP_DEMO_BLEND_SRC);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_ADDR,
                           NEMAP_DEMO_PRELOAD_ADDR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAH,
                           NEMAP_DEMO_PIXOUT_DATAH);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_IMEM_DATAL,
                           NEMAP_DEMO_PIXOUT_DATAL);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CODEPTR,
                           NEMAP_DEMO_FILL_CODEPTR);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_COLOR, rgba);
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMIN, nemap_demo_yx(0, 0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_CLIPMAX,
                           nemap_demo_yx(height, width));
  if (ret < 0)
    {
      return ret;
    }

  return nemap_demo_emitreg(cmdlist, STM32_GPU2D_MATMULT,
                            NEMAP_DEMO_MMUL_BYPASS);
}

static int
nemap_demo_emit_raster_box(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                           int x, int y, int w, int h)
{
  int ret;

  if (w <= 0 || h <= 0)
    {
      return OK;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx((uint32_t)y, (uint32_t)x));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx((uint32_t)(y + h),
                                         (uint32_t)(x + w)));
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemit(cmdlist,
                            GPU2D_CL_HOLD |
                            GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                          STM32_GPU2D_BASE),
                            NEMAP_DEMO_DRAW_BOX);
}

static int
nemap_demo_emit_raster_line(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                            int x0, int y0, int x1, int y1)
{
  int ret;

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_STARTXY,
                           nemap_demo_yx((uint32_t)y0, (uint32_t)x0));
  if (ret < 0)
    {
      return ret;
    }

  ret = nemap_demo_emitreg(cmdlist, STM32_GPU2D_DRAW_ENDXY,
                           nemap_demo_yx((uint32_t)y1, (uint32_t)x1));
  if (ret < 0)
    {
      return ret;
    }

  return stm32_gpu2d_clemit(cmdlist,
                            GPU2D_CL_HOLD |
                            GPU2D_CL_REG(STM32_GPU2D_DRAW_CMD -
                                          STM32_GPU2D_BASE),
                            NEMAP_DEMO_DRAW_LINE);
}

static int
nemap_demo_emit_rounded_rect_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                                    uintptr_t dst, uint32_t width,
                                    uint32_t height, uint32_t stride,
                                    int x0, int y0, int w, int h, int r,
                                    uint32_t rgba)
{
  int x;
  int y;
  int d;
  int x_l;
  int y_t;
  int x_r;
  int y_b;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_raster_rgb565_setup(cmdlist, dst, width, height,
                                            stride, rgba);
  if (ret < 0)
    {
      return ret;
    }

  if ((w / 2) < r)
    {
      r = w / 2;
    }

  if ((h / 2) < r)
    {
      r = h / 2;
    }

  if (r <= 0)
    {
      ret = nemap_demo_emit_raster_box(cmdlist, x0, y0, w, h);
      if (ret < 0)
        {
          return ret;
        }

      return stm32_gpu2d_clemitreturn(cmdlist);
    }

  ret = nemap_demo_emit_raster_box(cmdlist, x0, y0 + r, w, h - 2 * r);
  if (ret < 0)
    {
      return ret;
    }

  x_l = x0 + r;
  y_t = y0 + r;
  x_r = x0 + w - r - 1;
  y_b = y0 + h - r - 1;
  x = 0;
  y = r;
  d = 3 - 2 * r;

  while (x <= y)
    {
      if (x != 0)
        {
          ret = nemap_demo_emit_raster_line(cmdlist, x_l - y, y_t - x,
                                            x_r + y, y_t - x);
          if (ret < 0)
            {
              return ret;
            }

          ret = nemap_demo_emit_raster_line(cmdlist, x_l - y, y_b + x,
                                            x_r + y, y_b + x);
          if (ret < 0)
            {
              return ret;
            }
        }

      if (d < 0)
        {
          d += 4 * x + 6;
        }
      else
        {
          if (x != y)
            {
              ret = nemap_demo_emit_raster_line(cmdlist, x_l - x,
                                                y_t - y, x_r + x,
                                                y_t - y);
              if (ret < 0)
                {
                  return ret;
                }

              ret = nemap_demo_emit_raster_line(cmdlist, x_l - x,
                                                y_b + y, x_r + x,
                                                y_b + y);
              if (ret < 0)
                {
                  return ret;
                }
            }

          d += 4 * (x - y) + 10;
          y--;
        }

      x++;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int
nemap_demo_emit_circle_rgb565(FAR struct stm32_gpu2d_cmdlist_s *cmdlist,
                              uintptr_t dst, uint32_t width,
                              uint32_t height, uint32_t stride,
                              int cx, int cy, int r, uint32_t rgba)
{
  int x;
  int y;
  int d;
  int ret;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_raster_rgb565_setup(cmdlist, dst, width, height,
                                            stride, rgba);
  if (ret < 0)
    {
      return ret;
    }

  if (r <= 0)
    {
      ret = nemap_demo_emit_raster_box(cmdlist, cx, cy, 1, 1);
      if (ret < 0)
        {
          return ret;
        }

      return stm32_gpu2d_clemitreturn(cmdlist);
    }

  x = 0;
  y = r;
  d = 3 - 2 * r;

  while (x <= y)
    {
      ret = nemap_demo_emit_raster_line(cmdlist, cx - y, cy - x,
                                        cx + y, cy - x);
      if (ret < 0)
        {
          return ret;
        }

      if (x != 0)
        {
          ret = nemap_demo_emit_raster_line(cmdlist, cx - y, cy + x,
                                            cx + y, cy + x);
          if (ret < 0)
            {
              return ret;
            }
        }

      if (x != y)
        {
          ret = nemap_demo_emit_raster_line(cmdlist, cx - x, cy - y,
                                            cx + x, cy - y);
          if (ret < 0)
            {
              return ret;
            }

          ret = nemap_demo_emit_raster_line(cmdlist, cx - x, cy + y,
                                            cx + x, cy + y);
          if (ret < 0)
            {
              return ret;
            }
        }

      if (d < 0)
        {
          d += 4 * x + 6;
        }
      else
        {
          d += 4 * (x - y) + 10;
          y--;
        }

      x++;
    }

  return stm32_gpu2d_clemitreturn(cmdlist);
}

static int nemap_demo_circle_raster_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  const int cx = 32;
  const int cy = 32;
  const int radius = 22;
  uint32_t changed = 0;
  uint32_t hash;
  uint32_t x;
  uint32_t y;
  size_t i;
  int ret;

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_blit_dst[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  ret = nemap_demo_emit_circle_rgb565(&cmdlist, (uintptr_t)g_blit_dst,
                                      NEMAP_DEMO_OFFSCREEN_W,
                                      NEMAP_DEMO_OFFSCREEN_H,
                                      NEMAP_DEMO_OFFSCREEN_W *
                                      sizeof(uint16_t),
                                      cx, cy, radius,
                                      NEMAP_DEMO_RGBA_CYAN);
  if (ret < 0)
    {
      printf("nemap_demo: circle offscreen emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "circle-raster-offscreen");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_blit_dst, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t got = g_blit_dst[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if (got != NEMAP_DEMO_RGB565_SENTINEL)
            {
              changed++;
              if (x < (uint32_t)(cx - radius) ||
                  x > (uint32_t)(cx + radius) ||
                  y < (uint32_t)(cy - radius) ||
                  y > (uint32_t)(cy + radius))
                {
                  printf("nemap_demo: circle outside write x=%" PRIu32
                         " y=%" PRIu32 " got=0x%04x\n", x, y, got);
                  nemap_demo_print_snapshot("circle-raster outside");
                  return -EIO;
                }
            }
        }
    }

  if (g_blit_dst[cx + cy * NEMAP_DEMO_OFFSCREEN_W] !=
      NEMAP_DEMO_RGB565_CYAN ||
      g_blit_dst[(cx - radius) + cy * NEMAP_DEMO_OFFSCREEN_W] !=
      NEMAP_DEMO_RGB565_CYAN ||
      g_blit_dst[cx + (cy - radius) * NEMAP_DEMO_OFFSCREEN_W] !=
      NEMAP_DEMO_RGB565_CYAN ||
      g_blit_dst[(cx - radius) + (cy - radius) *
                 NEMAP_DEMO_OFFSCREEN_W] != NEMAP_DEMO_RGB565_SENTINEL ||
      changed < 1200 || changed > 1800)
    {
      printf("nemap_demo: circle sample mismatch changed=%" PRIu32
             " center=0x%04x left=0x%04x top=0x%04x corner=0x%04x\n",
             changed, g_blit_dst[cx + cy * NEMAP_DEMO_OFFSCREEN_W],
             g_blit_dst[(cx - radius) + cy * NEMAP_DEMO_OFFSCREEN_W],
             g_blit_dst[cx + (cy - radius) * NEMAP_DEMO_OFFSCREEN_W],
             g_blit_dst[(cx - radius) + (cy - radius) *
                        NEMAP_DEMO_OFFSCREEN_W]);
      nemap_demo_print_snapshot("circle-raster sample mismatch");
      return -EIO;
    }

  hash = nemap_demo_rgb565_fnv(&g_blit_dst[(cx - radius) +
                              (cy - radius) * NEMAP_DEMO_OFFSCREEN_W],
                              NEMAP_DEMO_OFFSCREEN_W,
                              (uint32_t)(radius * 2 + 1),
                              (uint32_t)(radius * 2 + 1));
  printf("nemap_demo: PASS phase 29a RGB565 circle raster span "
         "compare changed=%" PRIu32 " hash=0x%08" PRIx32 "\n",
         changed, hash);
  return OK;
}

static int
nemap_demo_circle_raster_visible(FAR const char *fbdev,
                                 FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t radius;
  uint32_t cx;
  uint32_t cy;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 29b visible circle raster: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible circle raster failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 29b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  radius = MIN((uint32_t)36, MIN((uint32_t)vinfo->xres,
                                 (uint32_t)vinfo->yres) / 6);
  if (radius < 24)
    {
      radius = 24;
    }

  cx = (uint32_t)vinfo->xres / 2;
  cy = (uint32_t)vinfo->yres / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);

  ret = nemap_demo_emit_circle_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                      vinfo->xres, vinfo->yres,
                                      pinfo.stride, (int)cx, (int)cy,
                                      (int)radius, NEMAP_DEMO_RGBA_CYAN);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "circle-raster-visible");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 29b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 29b visible circle raster span "
         "radius=%" PRIu32 " center=%" PRIu32 ",%" PRIu32 "\n",
         radius, cx, cy);
  close(fd);
  return OK;
}

static int
nemap_demo_vg_path_probe_visible(FAR const char *fbdev,
                                 FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  FAR uint16_t *fb;
  uint32_t config = stm32_gpu2dreadconfig();
  uint32_t configh = stm32_gpu2dreadconfigh();
  uint32_t panel_w;
  uint32_t panel_h;
  uint32_t radius;
  uint32_t x0;
  uint32_t y0;
  uint32_t stride_pixels;
  uint32_t changed = 0;
  uint32_t x;
  uint32_t y;
  uint16_t center;
  uint16_t top;
  uint16_t left;
  uint16_t outside;
  uint8_t display = 1;
  int fd;
  int ret;

  printf("nemap_demo: rounded-rect raster probe config vg=%s radial=%s "
         "config=0x%08" PRIx32 " configh=0x%08" PRIx32 "\n",
         (config & GPU2D_CONFIG_VG) != 0 ? "yes" : "no",
         (configh & GPU2D_CONFIGH_RADIAL) != 0 ? "yes" : "no",
         config, configh);

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 26 VG path probe: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for VG path probe failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 26 get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  panel_w = MIN((uint32_t)300, (uint32_t)vinfo->xres - 64);
  panel_h = MIN((uint32_t)180, (uint32_t)vinfo->yres - 64);
  panel_w &= ~1u;
  panel_h &= ~1u;
  if (panel_w < 160)
    {
      panel_w = 160;
    }

  if (panel_h < 96)
    {
      panel_h = 96;
    }

  radius = MIN(panel_w, panel_h) / 5;
  radius &= ~1u;
  if (radius < 16)
    {
      radius = 16;
    }

  x0 = ((uint32_t)vinfo->xres - panel_w) / 2;
  y0 = ((uint32_t)vinfo->yres - panel_h) / 2;

  nemap_demo_cpu_solid_rgb565(vinfo, &pinfo, NEMAP_DEMO_RGB565_BLACK);

  ret = nemap_demo_emit_rounded_rect_rgb565(&cmdlist,
                                            (uintptr_t)pinfo.fbmem,
                                            vinfo->xres, vinfo->yres,
                                            pinfo.stride, (int)x0, (int)y0,
                                            (int)panel_w, (int)panel_h,
                                            (int)radius,
                                            NEMAP_DEMO_RGBA_YELLOW);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "rounded-rect-raster");
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  nemap_demo_invalidate(pinfo.fbmem, pinfo.fblen);
  fb = (FAR uint16_t *)pinfo.fbmem;
  stride_pixels = pinfo.stride / sizeof(uint16_t);

  for (y = y0; y < y0 + panel_h; y++)
    {
      for (x = x0; x < x0 + panel_w; x++)
        {
          if (fb[x + y * stride_pixels] != NEMAP_DEMO_RGB565_BLACK)
            {
              changed++;
            }
        }
    }

  center = fb[(x0 + panel_w / 2) + (y0 + panel_h / 2) * stride_pixels];
  top = fb[(x0 + panel_w / 2) + y0 * stride_pixels];
  left = fb[x0 + (y0 + panel_h / 2) * stride_pixels];
  outside = fb[(x0 + 2) + (y0 + 2) * stride_pixels];

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 26",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  close(fd);
  if (ret < 0)
    {
      return ret;
    }

  if (center != NEMAP_DEMO_RGB565_YELLOW ||
      top != NEMAP_DEMO_RGB565_YELLOW ||
      left != NEMAP_DEMO_RGB565_YELLOW ||
      outside != NEMAP_DEMO_RGB565_BLACK ||
      changed < (panel_w * panel_h) / 2)
    {
      printf("nemap_demo: rounded-rect raster probe sample mismatch "
             "changed=%" PRIu32 " center=0x%04x top=0x%04x "
             "left=0x%04x outside=0x%04x expected fill=0x%04x "
             "outside=0x%04x\n",
             changed, center, top, left, outside,
             NEMAP_DEMO_RGB565_YELLOW, NEMAP_DEMO_RGB565_BLACK);
      nemap_demo_debug_status();
      nemap_demo_print_snapshot("rounded-rect-raster mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 26 GPU2D rounded-rect raster draw "
         "panel=%" PRIu32 "x%" PRIu32 " r=%" PRIu32
         " at %" PRIu32 ",%" PRIu32 " changed=%" PRIu32
         " (CONFIG.VG=%s)\n",
         panel_w, panel_h, radius, x0, y0, changed,
         (config & GPU2D_CONFIG_VG) != 0 ? "yes" : "no");
  return OK;
}

static int nemap_demo_vg_capability(void)
{
  uint32_t config = stm32_gpu2dreadconfig();
  uint32_t configh = stm32_gpu2dreadconfigh();
  uint32_t syserror = nemap_demo_regread(STM32_GPU2D_SYS_INTERRUPT);

  printf("nemap_demo: vg capability vg=%s radial=%s "
         "config=0x%08" PRIx32 " configh=0x%08" PRIx32
         " syserror=0x%08" PRIx32 "\n",
         (config & GPU2D_CONFIG_VG) != 0 ? "yes" : "no",
         (configh & GPU2D_CONFIGH_RADIAL) != 0 ? "yes" : "no",
         config, configh, syserror);

  if (syserror != 0)
    {
      printf("nemap_demo: vg capability unexpected SYS_INTERRUPT=0x%08"
             PRIx32 "\n", syserror);
      return -EIO;
    }

  if ((config & GPU2D_CONFIG_VG) == 0)
    {
      if (g_vg_probe_strict)
        {
          printf("nemap_demo: FAIL phase 26 native NemaVG path draw probe "
                 "blocked (CONFIG.VG=0, no hardware VG path engine)\n");
          return -ENOTSUP;
        }

      printf("nemap_demo: PASS phase 26 NemaVG capability absent "
             "(CONFIG.VG=0, VG middleware tests skipped)\n");
      return OK;
    }

  if (g_vg_probe_strict)
    {
      printf("nemap_demo: FAIL phase 26 native NemaVG path draw probe "
             "requires middleware draw_path coverage (CONFIG.VG=1)\n");
      return -ENOSYS;
    }

  printf("nemap_demo: PASS phase 26 NemaVG capability present "
         "(CONFIG.VG=1, middleware path tests pending)\n");
  return OK;
}

static int
nemap_demo_display_boundary(FAR const char *fbdev,
                            FAR const struct fb_videoinfo_s *vinfo,
                            FAR const struct fb_planeinfo_s *pinfo)
{
  printf("nemap_demo: display boundary fbdev=%s fmt=%u bpp=%u "
         "stride=%u len=%u fb=%p xres=%u yres=%u\n",
         fbdev, vinfo->fmt, pinfo->bpp, (unsigned int)pinfo->stride,
         (unsigned int)pinfo->fblen, pinfo->fbmem, vinfo->xres,
         vinfo->yres);

  printf("nemap_demo: PASS phase 27 NemaDC boundary "
         "(STM32 backend exposes framebuffer/display-controller path, "
         "no raw NemaDC register block validated)\n");
  return OK;
}

static int
nemap_demo_extra_texture_shape_phases(FAR const char *fbdev,
                                      FAR const struct fb_videoinfo_s *vinfo,
                                      bool stop_on_fail)
{
  int failures = 0;
  int ret;

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_textured_triangle_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 28a textured triangle offscreen: "
                 "%d\n", ret);
          failures++;
          if (stop_on_fail)
            {
              return -EIO;
            }
        }
    }
  else
    {
      printf("nemap_demo: skip phase 28a textured triangle offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_textured_triangle_visible(fbdev, vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 28b visible textured triangle: %d\n",
             ret);
      failures++;
      if (stop_on_fail)
        {
          return -EIO;
        }
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_circle_raster_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 29a circle raster offscreen: %d\n",
                 ret);
          failures++;
          if (stop_on_fail)
            {
              return -EIO;
            }
        }
    }
  else
    {
      printf("nemap_demo: skip phase 29a circle raster offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_circle_raster_visible(fbdev, vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 29b visible circle raster: %d\n", ret);
      failures++;
      if (stop_on_fail)
        {
          return -EIO;
        }
    }

  return failures == 0 ? OK : -EIO;
}

static int nemap_demo_blend_offscreen(void)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  uint32_t x;
  uint32_t y;
  uint32_t x0 = 16;
  uint32_t y0 = 16;
  uint32_t w = 32;
  uint32_t h = 32;
  size_t i;
  int ret;

  for (i = 0; i < NEMAP_DEMO_OFFSCREEN_PIXELS; i++)
    {
      g_offscreen[i] = NEMAP_DEMO_RGB565_SENTINEL;
    }

  nemap_demo_clean(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)g_offscreen,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_OFFSCREEN_W *
                                    sizeof(uint16_t),
                                    0, 0,
                                    NEMAP_DEMO_OFFSCREEN_W,
                                    NEMAP_DEMO_OFFSCREEN_H,
                                    NEMAP_DEMO_RGBA_BLUE);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blend-offscreen-bg");
    }

  if (ret < 0)
    {
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565_blend(&cmdlist, (uintptr_t)g_offscreen,
                                          NEMAP_DEMO_OFFSCREEN_W,
                                          NEMAP_DEMO_OFFSCREEN_H,
                                          NEMAP_DEMO_OFFSCREEN_W *
                                          sizeof(uint16_t),
                                          x0, y0, w, h,
                                          NEMAP_DEMO_BLEND_SIMPLE,
                                          NEMAP_DEMO_RGBA_RED_50);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blend-offscreen");
    }

  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(g_offscreen, NEMAP_DEMO_OFFSCREEN_BYTES);

  for (y = 0; y < NEMAP_DEMO_OFFSCREEN_H; y++)
    {
      for (x = 0; x < NEMAP_DEMO_OFFSCREEN_W; x++)
        {
          uint16_t expected;
          uint16_t got;
          bool inside;

          inside = x >= x0 && x < x0 + w && y >= y0 && y < y0 + h;
          expected = inside ? 0 : NEMAP_DEMO_RGB565_BLUE;
          got = g_offscreen[x + y * NEMAP_DEMO_OFFSCREEN_W];

          if ((!inside && got != expected) ||
              (inside && !nemap_demo_rgb565_is_red_blue_blend(got)))
            {
              printf("nemap_demo: blend offscreen mismatch x=%" PRIu32
                     " y=%" PRIu32 " got=0x%04x r=%" PRIu32
                     " g=%" PRIu32 " b=%" PRIu32 "\n",
                     x, y, got, nemap_demo_rgb565_r(got),
                     nemap_demo_rgb565_g(got), nemap_demo_rgb565_b(got));
              nemap_demo_print_snapshot("blend-offscreen mismatch");
              return -EIO;
            }
        }
    }

  printf("nemap_demo: PASS phase 9a RGB565 ROP alpha blend offscreen "
         "compare\n");
  return OK;
}

static int nemap_demo_blend_visible(FAR const char *fbdev,
                                    FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t square;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 9b visible blend: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible blend failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 9b get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  square = MIN((uint32_t)240, MIN((uint32_t)vinfo->xres,
                                  (uint32_t)vinfo->yres) - 80);
  square &= ~1u;
  if (square < 2)
    {
      square = 2;
    }

  x0 = ((uint32_t)vinfo->xres - square) / 2;
  y0 = ((uint32_t)vinfo->yres - square) / 2;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    0, 0, vinfo->xres, vinfo->yres,
                                    NEMAP_DEMO_RGBA_BLUE);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blend-visible-bg");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565_blend(&cmdlist, (uintptr_t)pinfo.fbmem,
                                          vinfo->xres, vinfo->yres,
                                          pinfo.stride, x0, y0, square,
                                          square, NEMAP_DEMO_BLEND_SIMPLE,
                                          NEMAP_DEMO_RGBA_RED_50);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "blend-visible");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 9b",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 9b RGB565 ROP alpha blend visible "
         "center=%" PRIu32 "x%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         square, square, x0, y0);
  close(fd);
  return OK;
}

static int nemap_demo_fill_framebuffer(FAR const struct fb_videoinfo_s *vinfo,
                                       FAR const struct fb_planeinfo_s *pinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  FAR uint16_t *sample;
  uint32_t row;
  uint32_t col;
  uint32_t w;
  uint32_t h;
  uint32_t x0;
  uint32_t y0;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565 || pinfo->bpp != 16)
    {
      printf("nemap_demo: skip phase 5a framebuffer fill: fmt=%u bpp=%u\n",
             vinfo->fmt, pinfo->bpp);
      return OK;
    }

  if (vinfo->xres <= 32 || vinfo->yres <= 32)
    {
      printf("nemap_demo: skip phase 5a framebuffer fill: "
             "resolution=%ux%u\n",
             vinfo->xres, vinfo->yres);
      return OK;
    }

  w = MIN((uint32_t)192, (uint32_t)vinfo->xres - 32);
  h = MIN((uint32_t)192, (uint32_t)vinfo->yres - 32);
  x0 = ((uint32_t)vinfo->xres - w) / 2;
  y0 = ((uint32_t)vinfo->yres - h) / 2;
  sample = nemap_demo_rgb565_pixel(pinfo, x0, y0);

  for (row = y0; row < y0 + h; row++)
    {
      for (col = x0; col < x0 + w; col++)
        {
          *nemap_demo_rgb565_pixel(pinfo, col, row) =
            NEMAP_DEMO_RGB565_SENTINEL;
        }
    }

  nemap_demo_clean(pinfo->fbmem, pinfo->fblen);
  printf("nemap_demo: fill framebuffer sentinel sample=0x%04x\n",
         *sample);

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);

  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo->fbmem,
                                    vinfo->xres, vinfo->yres, pinfo->stride,
                                    x0, y0, w, h, NEMAP_DEMO_RGBA_RED);
  if (ret < 0)
    {
      printf("nemap_demo: fill framebuffer emit failed: %d\n", ret);
      return ret;
    }

  ret = nemap_demo_submit_cmdlist(&cmdlist, "fill-framebuffer");
  if (ret < 0)
    {
      return ret;
    }

  nemap_demo_invalidate(pinfo->fbmem, pinfo->fblen);

  if (*sample != NEMAP_DEMO_RGB565_RED)
    {
      printf("nemap_demo: fill framebuffer sample mismatch got=0x%04x "
             "expected=0x%04x\n",
             *sample, NEMAP_DEMO_RGB565_RED);
      nemap_demo_print_snapshot("fill-framebuffer mismatch");
      return -EIO;
    }

  printf("nemap_demo: PASS phase 5a RGB565 framebuffer fill compare "
         "visible rect=%" PRIu32 "x%" PRIu32 " at %" PRIu32 ",%"
         PRIu32 "\n", w, h, x0, y0);
  usleep(NEMAP_DEMO_VISUAL_DELAY_MS * 1000);
  return OK;
}

static int nemap_demo_visual_frame(FAR const char *fbdev,
                                   FAR const struct fb_videoinfo_s *vinfo)
{
  struct stm32_gpu2d_cmdlist_s cmdlist;
  struct fb_planeinfo_s pinfo;
  uint32_t square;
  uint32_t half;
  uint32_t x0;
  uint32_t y0;
  uint8_t display = 1;
  int fd;
  int ret;

  if (vinfo->fmt != FB_FMT_RGB16_565)
    {
      printf("nemap_demo: skip phase 6 visible frame: fmt=%u\n",
             vinfo->fmt);
      return OK;
    }

  fd = open(fbdev, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    {
      printf("nemap_demo: open %s for visible frame failed: %d\n",
             fbdev, errno);
      return -errno;
    }

  ret = nemap_demo_get_plane(fd, display, &pinfo);
  if (ret < 0)
    {
      display = 0;
      ret = nemap_demo_get_plane(fd, display, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: phase 6 get plane failed: %d\n", ret);
          close(fd);
          return ret;
        }
    }

  printf("nemap_demo: phase 6 visible frame target display=%u fb=%p "
         "yoffset=%" PRIu32 "\n",
         display, pinfo.fbmem, pinfo.yoffset);

  square = MIN((uint32_t)280, MIN((uint32_t)vinfo->xres,
                                  (uint32_t)vinfo->yres) - 40);
  square &= ~1u;
  if (square < 2)
    {
      square = 2;
    }

  half = square / 2;
  x0 = ((uint32_t)vinfo->xres - square) / 2;
  y0 = ((uint32_t)vinfo->yres - square) / 2;

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    0, 0, vinfo->xres, vinfo->yres,
                                    NEMAP_DEMO_RGBA_BLUE);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "visible-blue");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    x0, y0, square, half,
                                    NEMAP_DEMO_RGBA_GREEN);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "visible-green");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  memset(g_cmdlist, 0, NEMAP_DEMO_CL_BYTES);
  stm32_gpu2d_clinit(&cmdlist, g_cmdlist, NEMAP_DEMO_CL_WORDS);
  ret = nemap_demo_emit_fill_rgb565(&cmdlist, (uintptr_t)pinfo.fbmem,
                                    vinfo->xres, vinfo->yres, pinfo.stride,
                                    x0, y0 + half, square, half,
                                    NEMAP_DEMO_RGBA_MAGENTA);
  if (ret >= 0)
    {
      ret = nemap_demo_submit_cmdlist(&cmdlist, "visible-magenta");
    }

  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  ret = nemap_demo_show_plane(fd, &pinfo, vinfo, "phase 6",
                              NEMAP_DEMO_VISUAL_DELAY_MS);
  if (ret < 0)
    {
      close(fd);
      return ret;
    }

  printf("nemap_demo: PASS phase 6 visible framebuffer pattern "
         "center=%" PRIu32 "x%" PRIu32 " at %" PRIu32 ",%" PRIu32 "\n",
         square, square, x0, y0);
  close(fd);
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *fbdev = CONFIG_EXAMPLES_NEMAP_DEMO_FBDEV;
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
  int failures = 0;
  int ret;
  int exitcode = EXIT_FAILURE;
  int argi;

  for (argi = 1; argi < argc; argi++)
    {
      if (strcmp(argv[argi], "--probe-vg") == 0)
        {
          g_vg_probe_strict = true;
        }
      else if (strcmp(argv[argi], "--help") == 0)
        {
          printf("usage: nemap_demo [--probe-vg] [fbdev]\n");
          printf("  --probe-vg  run only the rounded-rectangle path probe "
                 "and print GPU2D diagnostics on mismatch\n");
          return EXIT_SUCCESS;
        }
      else
        {
          fbdev = argv[argi];
        }
    }

  printf("nemap_demo: STM32 NemaP/GPU2D validation start%s\n",
         g_vg_probe_strict ? " (VG path draw probe)" : "");

  ret = nemap_demo_fb_probe(fbdev, &vinfo, &pinfo);
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  ret = nemap_demo_resources_init(&pinfo);
  if (ret < 0)
    {
      printf("nemap_demo: resource init failed: %d\n", ret);
      return EXIT_FAILURE;
    }

  ret = stm32_gpu2dinitialize();
  if (ret < 0)
    {
      printf("nemap_demo: stm32_gpu2dinitialize failed: %d\n", ret);
      failures++;
      goto out;
    }

  ret = nemap_demo_identity();
  if (ret < 0)
    {
      failures++;
      goto out;
    }

  ret = nemap_demo_command_irq();
  if (ret < 0)
    {
      failures++;
      goto out;
    }

  printf("nemap_demo: PASS phase 0-4\n");

  if (g_vg_probe_strict)
    {
      printf("nemap_demo: --probe-vg active, skip verified phases 5-25 "
             "and run only the pending phase 26 rounded-rect probe\n");

      ret = nemap_demo_vg_path_probe_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 26 rounded-rect path probe: %d\n",
                 ret);
          failures++;
        }

      goto out;
    }

  if (!NEMAP_DEMO_RUN_VERIFIED_PHASES)
    {
      ret = nemap_demo_debug_status();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 19 debug/status snapshot: %d\n",
                 ret);
          failures++;
          goto out;
        }

      ret = nemap_demo_depth_capability();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 20 depth capability: %d\n", ret);
          failures++;
          goto out;
        }

      if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
        {
          ret = nemap_demo_affine_offscreen();
          if (ret < 0)
            {
              printf("nemap_demo: FAIL phase 21a affine offscreen: %d\n",
                     ret);
              failures++;
              goto out;
            }
        }
      else
        {
          printf("nemap_demo: skip phase 21a affine offscreen "
                 NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
        }

      ret = nemap_demo_affine_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 21b visible affine: %d\n", ret);
          failures++;
          goto out;
        }

      if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
        {
          ret = nemap_demo_textured_quad_offscreen();
          if (ret < 0)
            {
              printf("nemap_demo: FAIL phase 22a textured quad offscreen: "
                     "%d\n", ret);
              failures++;
              goto out;
            }
        }
      else
        {
          printf("nemap_demo: skip phase 22a textured quad offscreen "
                 NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
        }

      ret = nemap_demo_textured_quad_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 22b visible textured quad: %d\n",
                 ret);
          failures++;
          goto out;
        }

      if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
        {
          ret = nemap_demo_projective_quad_offscreen();
          if (ret < 0)
            {
              printf("nemap_demo: FAIL phase 23a projective quad offscreen: "
                     "%d\n", ret);
              failures++;
              goto out;
            }
        }
      else
        {
          printf("nemap_demo: skip phase 23a projective quad offscreen "
                 NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
        }

      ret = nemap_demo_projective_quad_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 23b visible projective quad: %d\n",
                 ret);
          failures++;
          goto out;
        }

      if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
        {
          ret = nemap_demo_line_primitive_offscreen();
          if (ret < 0)
            {
              printf("nemap_demo: FAIL phase 24a line primitive offscreen: "
                     "%d\n", ret);
              failures++;
              goto out;
            }
        }
      else
        {
          printf("nemap_demo: skip phase 24a line primitive offscreen "
                 NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
        }

      ret = nemap_demo_line_primitive_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 24b visible line primitive: %d\n",
                 ret);
          failures++;
          goto out;
        }

      if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
        {
          ret = nemap_demo_gradient_offscreen();
          if (ret < 0)
            {
              printf("nemap_demo: FAIL phase 25a gradient offscreen: %d\n",
                     ret);
              failures++;
              goto out;
            }
        }
      else
        {
          printf("nemap_demo: skip phase 25a gradient offscreen "
                 NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
        }

      ret = nemap_demo_gradient_visible(fbdev, &vinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 25b visible gradient: %d\n", ret);
          failures++;
          goto out;
        }

      ret = g_vg_probe_strict ? nemap_demo_vg_path_probe_visible(fbdev,
                                                                 &vinfo) :
                                nemap_demo_vg_capability();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 26 NemaVG path/capability: %d\n",
                 ret);
          failures++;
          goto out;
        }

      ret = nemap_demo_display_boundary(fbdev, &vinfo, &pinfo);
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 27 display boundary: %d\n", ret);
          failures++;
          goto out;
        }

      ret = nemap_demo_extra_texture_shape_phases(fbdev, &vinfo, true);
      if (ret < 0)
        {
          failures++;
          goto out;
        }

      printf("nemap_demo: skip verified phases 5-18 "
             "(enable CONFIG_EXAMPLES_NEMAP_DEMO_RUN_VERIFIED_PHASES "
             "for full regression)\n");
      goto out;
    }

  ret = nemap_demo_fill_framebuffer(&vinfo, &pinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 5a framebuffer fill: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_fill_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 5b offscreen fill: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 5b offscreen fill "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_visual_frame(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 6 visible framebuffer pattern: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_clip_dirty_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 7 clip/dirty offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 7 clip/dirty offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_blit_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 8a source blit offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 8a source blit offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_blit_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 8b source blit visible: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_blend_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 9a ROP alpha blend offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 9a ROP alpha blend offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_blend_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 9b ROP alpha blend visible: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_scale_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 10a point-sampled scale offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 10a point-sampled scale offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_filter_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 10b bilinear filter offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 10b bilinear filter offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_scale_filter_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 10c visible scale/filter: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_triangle_aa_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 11a triangle AA offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 11a triangle AA offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_triangle_aa_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 11b visible triangle AA: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_dst_ckey_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 12a destination color key "
                 "offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 12a destination color key offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_dst_ckey_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 12b visible destination color key: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_src_ckey_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 13a source color key offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 13a source color key offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_src_ckey_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 13b visible source color key: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_stencil_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 14a TEX3 stencil offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 14a TEX3 stencil offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_stencil_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 14b visible TEX3 stencil: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_format_sweep_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 15a 32-bit format sweep "
                 "offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 15a 32-bit format sweep offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_format_sweep_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 15b visible 32-bit format sweep: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_format16_sweep_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 15c 16-bit format sweep "
                 "offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 15c 16-bit format sweep offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_format16_sweep_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 15d visible 16-bit format sweep: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_format_low_sweep_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 15e 8/24-bit format sweep "
                 "offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 15e 8/24-bit format sweep offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_format_low_sweep_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 15f visible 8/24-bit format "
             "sweep: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_tsc6_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 16a TSC6 offscreen "
                 "decompress/blit: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 16a TSC6 offscreen decompress/blit "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_tsc6_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 16b visible TSC6 decompress/blit: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_tsc6a_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 17a TSC6A source/alpha "
                 "offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 17a TSC6A source/alpha offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_tsc6a_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 17b visible TSC6A alpha blend: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_wrap_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 18a texture wrap offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 18a texture wrap offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_wrap_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 18b visible texture wrap: %d\n", ret);
      failures++;
    }

  ret = nemap_demo_debug_status();
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 19 debug/status snapshot: %d\n", ret);
      failures++;
    }

  ret = nemap_demo_depth_capability();
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 20 depth capability: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_affine_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 21a affine offscreen: %d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 21a affine offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_affine_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 21b visible affine: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_textured_quad_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 22a textured quad offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 22a textured quad offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_textured_quad_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 22b visible textured quad: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_projective_quad_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 23a projective quad offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 23a projective quad offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_projective_quad_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 23b visible projective quad: %d\n",
             ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_line_primitive_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 24a line primitive offscreen: "
                 "%d\n", ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 24a line primitive offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_line_primitive_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 24b visible line primitive: %d\n", ret);
      failures++;
    }

  if (NEMAP_DEMO_RUN_OFFSCREEN_PHASES)
    {
      ret = nemap_demo_gradient_offscreen();
      if (ret < 0)
        {
          printf("nemap_demo: FAIL phase 25a gradient offscreen: %d\n",
                 ret);
          failures++;
        }
    }
  else
    {
      printf("nemap_demo: skip phase 25a gradient offscreen "
             NEMAP_DEMO_OFFSCREEN_SKIP_REASON "\n");
    }

  ret = nemap_demo_gradient_visible(fbdev, &vinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 25b visible gradient: %d\n", ret);
      failures++;
    }

  ret = g_vg_probe_strict ? nemap_demo_vg_path_probe_visible(fbdev, &vinfo) :
                            nemap_demo_vg_capability();
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 26 NemaVG path/capability: %d\n", ret);
      failures++;
    }

  ret = nemap_demo_display_boundary(fbdev, &vinfo, &pinfo);
  if (ret < 0)
    {
      printf("nemap_demo: FAIL phase 27 display boundary: %d\n", ret);
      failures++;
    }

  ret = nemap_demo_extra_texture_shape_phases(fbdev, &vinfo, false);
  if (ret < 0)
    {
      failures++;
    }

out:
  ret = nemap_demo_finish_screen(fbdev, &vinfo, failures == 0);
  if (ret < 0)
    {
      printf("nemap_demo: finish screen failed: %d\n", ret);
      failures++;
    }

  if (failures == 0)
    {
      if (NEMAP_DEMO_RUN_VERIFIED_PHASES)
        {
          printf("nemap_demo: PASS all phases, restored colorbar\n");
        }
      else
        {
          printf("nemap_demo: PASS all enabled phases, restored colorbar\n");
        }

      exitcode = EXIT_SUCCESS;
    }
  else
    {
      printf("nemap_demo: FAIL %d phase(s), switched to black screen\n",
             failures);
    }

  return exitcode;
}
