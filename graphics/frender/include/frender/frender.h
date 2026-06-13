/****************************************************************************
 * apps/graphics/frender/include/frender/frender.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_FRENDER_INCLUDE_FRENDER_FRENDER_H
#define __APPS_GRAPHICS_FRENDER_INCLUDE_FRENDER_FRENDER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum fr_format_e
{
  FR_FORMAT_RGBA8888 = 0
};

enum fr_input_type_e
{
  FR_INPUT_NONE = 0,
  FR_INPUT_POINTER_DOWN,
  FR_INPUT_POINTER_MOVE,
  FR_INPUT_POINTER_UP,
  FR_INPUT_KEY_DOWN,
  FR_INPUT_KEY_UP,
  FR_INPUT_ENCODER_ROTATE
};

enum fr_key_code_e
{
  FR_KEY_UNKNOWN = 0,
  FR_KEY_BACKSPACE = 8,
  FR_KEY_TAB = 9,
  FR_KEY_ENTER = 13,
  FR_KEY_DELETE = 127,
  FR_KEY_SPACE = 32,
  FR_KEY_LEFT = 1000,
  FR_KEY_RIGHT,
  FR_KEY_UP,
  FR_KEY_DOWN
};

enum fr_command_kind_e
{
  FR_CMD_CLEAR = 0,
  FR_CMD_FILL_RECT,
  FR_CMD_STROKE_RECT,
  FR_CMD_FILL_QUAD,
  FR_CMD_FILL_TRIANGLE,
  FR_CMD_STROKE_QUAD,
  FR_CMD_BLIT,
  FR_CMD_BLIT_QUAD,
  FR_CMD_PUSH_CLIP,
  FR_CMD_POP_CLIP
};

enum fr_backend_cap_e
{
  FR_CAP_NONE                 = 0,
  FR_CAP_SOFTWARE             = 1 << 0,
  FR_CAP_COMMAND_LIST         = 1 << 1,
  FR_CAP_CLEAR                = 1 << 2,
  FR_CAP_FILL_RECT            = 1 << 3,
  FR_CAP_STROKE_RECT          = 1 << 4,
  FR_CAP_CLIP                 = 1 << 5,
  FR_CAP_RGBA8888_TARGET      = 1 << 6,
  FR_CAP_FB_PRESENT           = 1 << 7,
  FR_CAP_NX_PRESENT           = 1 << 8,
  FR_CAP_HARDWARE_ACCELERATOR = 1 << 9,
  FR_CAP_FILL_QUAD            = 1 << 10,
  FR_CAP_BLIT                 = 1 << 11,
  FR_CAP_STROKE_QUAD          = 1 << 12,
  FR_CAP_BLIT_QUAD            = 1 << 13,
  FR_CAP_FILL_TRIANGLE        = 1 << 14
};

enum fr_backend_kind_e
{
  FR_BACKEND_KIND_SOFTWARE = 0,
  FR_BACKEND_KIND_PRESENT,
  FR_BACKEND_KIND_ACCELERATOR,
  FR_BACKEND_KIND_COMPOSITOR
};

enum fr_draw_cap_e
{
  FR_DRAW_CAP_NONE        = 0,
  FR_DRAW_CAP_COMMANDS    = 1 << 0,
  FR_DRAW_CAP_CLEAR       = 1 << 1,
  FR_DRAW_CAP_FILL_RECT   = 1 << 2,
  FR_DRAW_CAP_STROKE_RECT = 1 << 3,
  FR_DRAW_CAP_CLIP        = 1 << 4,
  FR_DRAW_CAP_BLIT        = 1 << 5,
  FR_DRAW_CAP_TEXT        = 1 << 6,
  FR_DRAW_CAP_FILL_QUAD   = 1 << 7,
  FR_DRAW_CAP_STROKE_QUAD = 1 << 8,
  FR_DRAW_CAP_BLIT_QUAD   = 1 << 9,
  FR_DRAW_CAP_FILL_TRIANGLE = 1 << 10
};

enum fr_present_cap_e
{
  FR_PRESENT_CAP_NONE        = 0,
  FR_PRESENT_CAP_FRAMEBUFFER = 1 << 0,
  FR_PRESENT_CAP_LCD         = 1 << 1,
  FR_PRESENT_CAP_NX          = 1 << 2,
  FR_PRESENT_CAP_UPDATE_RECT = 1 << 3,
  FR_PRESENT_CAP_VSYNC       = 1 << 4
};

enum fr_memory_cap_e
{
  FR_MEMORY_CAP_NONE        = 0,
  FR_MEMORY_CAP_SURFACE     = 1 << 0,
  FR_MEMORY_CAP_MMAP        = 1 << 1,
  FR_MEMORY_CAP_DIRECT_PTR  = 1 << 2,
  FR_MEMORY_CAP_PIXEL_WRITE = 1 << 3
};

enum fr_blend_cap_e
{
  FR_BLEND_CAP_NONE         = 0,
  FR_BLEND_CAP_GLOBAL_ALPHA = 1 << 0,
  FR_BLEND_CAP_PIXEL_ALPHA  = 1 << 1,
  FR_BLEND_CAP_COLOR_KEY    = 1 << 2
};

enum fr_transform_cap_e
{
  FR_TRANSFORM_CAP_NONE   = 0,
  FR_TRANSFORM_CAP_SCALE  = 1 << 0,
  FR_TRANSFORM_CAP_ROTATE = 1 << 1,
  FR_TRANSFORM_CAP_FLIP   = 1 << 2,
  FR_TRANSFORM_CAP_3D     = 1 << 3
};

enum fr_sync_cap_e
{
  FR_SYNC_CAP_NONE        = 0,
  FR_SYNC_CAP_UPDATE_RECT = 1 << 0,
  FR_SYNC_CAP_VSYNC_WAIT  = 1 << 1,
  FR_SYNC_CAP_FENCE       = 1 << 2
};

enum fr_format_cap_e
{
  FR_FORMAT_CAP_NONE     = 0,
  FR_FORMAT_CAP_RGBA8888 = 1 << 0,
  FR_FORMAT_CAP_RGB565   = 1 << 1,
  FR_FORMAT_CAP_RGB24    = 1 << 2,
  FR_FORMAT_CAP_RGB32    = 1 << 3,
  FR_FORMAT_CAP_RGBT32   = 1 << 4,
  FR_FORMAT_CAP_RGBA32   = 1 << 5
};

typedef struct fr_color_s
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} fr_color_t;

typedef struct fr_rect_s
{
  int16_t x;
  int16_t y;
  uint16_t w;
  uint16_t h;
} fr_rect_t;

typedef struct fr_point_s
{
  int16_t x;
  int16_t y;
} fr_point_t;

typedef struct fr_quad_s
{
  fr_point_t points[4];
} fr_quad_t;

typedef struct fr_triangle_s
{
  fr_point_t points[3];
} fr_triangle_t;

typedef struct fr_surface_s
{
  void *pixels;
  uint16_t width;
  uint16_t height;
  uint16_t stride;
  enum fr_format_e format;
} fr_surface_t;

typedef struct fr_backend_caps_s
{
  const char *name;
  enum fr_backend_kind_e kind;
  uint32_t caps;
  uint32_t draw_caps;
  uint32_t present_caps;
  uint32_t memory_caps;
  uint32_t blend_caps;
  uint32_t transform_caps;
  uint32_t sync_caps;
  uint32_t format_caps;
  enum fr_format_e preferred_format;
  uint16_t max_width;
  uint16_t max_height;
  uint16_t max_commands;
} fr_backend_caps_t;

typedef struct fr_fb_presenter_s
{
  int fd;
  void *fbmem;
  size_t fblen;
  uint16_t xres;
  uint16_t yres;
  uint16_t stride;
  uint8_t fmt;
  uint8_t bpp;
  bool mapped;
  bool open;
} fr_fb_presenter_t;

typedef struct fr_input_event_s
{
  enum fr_input_type_e type;
  int16_t x;
  int16_t y;
  uint16_t key;
  int16_t encoder_delta;
  uint8_t button;
} fr_input_event_t;

typedef struct fr_command_s
{
  enum fr_command_kind_e kind;
  fr_rect_t rect;
  fr_quad_t quad;
  fr_triangle_t triangle;
  fr_surface_t source;
  fr_rect_t src_rect;
  fr_color_t color;
  uint16_t thickness;
  uint8_t global_alpha;
} fr_command_t;

typedef struct fr_command_list_s
{
  fr_command_t *commands;
  uint16_t capacity;
  uint16_t count;
  bool overflowed;
} fr_command_list_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

fr_color_t fr_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t fr_color_pack_rgba8888(fr_color_t color);

int fr_surface_init(fr_surface_t *surface, void *pixels,
                    uint16_t width, uint16_t height, uint16_t stride,
                    enum fr_format_e format);

void fr_command_list_init(fr_command_list_t *list, fr_command_t *commands,
                          uint16_t capacity);
void fr_command_list_reset(fr_command_list_t *list);
bool fr_command_list_overflowed(const fr_command_list_t *list);
uint16_t fr_command_list_count(const fr_command_list_t *list);

int fr_cmd_clear(fr_command_list_t *list, fr_color_t color);
int fr_cmd_fill_rect(fr_command_list_t *list, const fr_rect_t *rect,
                     fr_color_t color);
int fr_cmd_stroke_rect(fr_command_list_t *list, const fr_rect_t *rect,
                       uint16_t thickness, fr_color_t color);
int fr_cmd_fill_quad(fr_command_list_t *list, const fr_quad_t *quad,
                     fr_color_t color);
int fr_cmd_fill_triangle(fr_command_list_t *list,
                         const fr_triangle_t *triangle,
                         fr_color_t color);
int fr_cmd_stroke_quad(fr_command_list_t *list, const fr_quad_t *quad,
                       uint16_t thickness, fr_color_t color);
int fr_cmd_blit(fr_command_list_t *list, const fr_surface_t *source,
                const fr_rect_t *src_rect, const fr_rect_t *dst_rect);
int fr_cmd_blit_alpha(fr_command_list_t *list, const fr_surface_t *source,
                      const fr_rect_t *src_rect, const fr_rect_t *dst_rect,
                      uint8_t global_alpha);
int fr_cmd_blit_quad_alpha(fr_command_list_t *list,
                           const fr_surface_t *source,
                           const fr_rect_t *src_rect,
                           const fr_quad_t *dst_quad,
                           uint8_t global_alpha);
int fr_cmd_push_clip(fr_command_list_t *list, const fr_rect_t *rect);
int fr_cmd_pop_clip(fr_command_list_t *list);

fr_backend_caps_t fr_backend_caps_none(void);
fr_backend_caps_t fr_backend_caps_software(void);
fr_backend_caps_t fr_backend_caps_nuttx_graphics(void);
fr_backend_caps_t fr_backend_caps_nuttx_framebuffer(void);
fr_backend_caps_t fr_backend_caps_nuttx_lcd(void);
fr_backend_caps_t fr_backend_caps_nuttx_nx(void);
fr_backend_caps_t fr_backend_caps_nuttx_dma2d(void);
fr_backend_caps_t fr_backend_caps_nuttx_gpu2d(void);
int fr_backend_caps_from_fb_presenter(const fr_fb_presenter_t *presenter,
                                      fr_backend_caps_t *caps);
bool fr_backend_supports(const fr_backend_caps_t *caps,
                         enum fr_command_kind_e kind);
bool fr_backend_supports_draw(const fr_backend_caps_t *caps,
                              uint32_t draw_caps);
bool fr_backend_supports_present(const fr_backend_caps_t *caps,
                                 uint32_t present_caps);
bool fr_backend_supports_memory(const fr_backend_caps_t *caps,
                                uint32_t memory_caps);
bool fr_backend_supports_blend(const fr_backend_caps_t *caps,
                               uint32_t blend_caps);
bool fr_backend_supports_transform(const fr_backend_caps_t *caps,
                                   uint32_t transform_caps);
bool fr_backend_supports_sync(const fr_backend_caps_t *caps,
                              uint32_t sync_caps);
bool fr_backend_supports_format(const fr_backend_caps_t *caps,
                                uint32_t format_caps);

void fr_backend_registry_reset(void);
int fr_backend_register(const fr_backend_caps_t *caps);
int fr_backend_register_builtin(void);
int fr_backend_register_nuttx_graphics(void);
int fr_backend_register_fb_presenter(const fr_fb_presenter_t *presenter);
uint8_t fr_backend_registry_count(void);
const fr_backend_caps_t *fr_backend_registry_get(uint8_t index);
const fr_backend_caps_t *fr_backend_registry_find(const char *name);

/* Opaque backend instance — defined in internal fr_backend.h.
 * Callers create one via fr_backend_ops_*() + open(),
 * then pass it to fr_execute(). */

struct fr_backend_ops_s;

typedef struct fr_backend_instance_s
{
  const struct fr_backend_ops_s *ops;
  void *priv;
} fr_backend_instance_t;

/* fr_execute: dispatch a command list through a backend instance.
 * The backend determines where pixels land (framebuffer, NX window, etc.).
 * All drawing is delegated to nuttx/graphics. */

int fr_execute(fr_backend_instance_t *backend,
               const fr_command_list_t *list);

/* Backend lifecycle — open/close/execute through a backend instance */

int  fr_backend_open(fr_backend_instance_t *backend, const char *name,
                     const void *config);
void fr_backend_close(fr_backend_instance_t *backend);
int  fr_backend_get_bounds(fr_backend_instance_t *backend,
                           fr_rect_t *bounds);

/* Backend ops accessors — return the vtable for each built-in backend. */

const struct fr_backend_ops_s *fr_backend_ops_framebuffer(void);
const struct fr_backend_ops_s *fr_backend_ops_nx(void);

int fr_fb_presenter_open(fr_fb_presenter_t *presenter, const char *path);
int fr_fb_presenter_present(fr_fb_presenter_t *presenter,
                            const fr_surface_t *surface);
int fr_fb_presenter_present_rect(fr_fb_presenter_t *presenter,
                                 const fr_surface_t *surface,
                                 const fr_rect_t *rect);
int fr_fb_presenter_update_rect(fr_fb_presenter_t *presenter,
                                const fr_rect_t *rect);
int fr_fb_presenter_poll_input(fr_fb_presenter_t *presenter,
                               fr_input_event_t *input);
bool fr_fb_presenter_window_closed(fr_fb_presenter_t *presenter);
void fr_fb_presenter_close(fr_fb_presenter_t *presenter);

uint32_t fr_surface_checksum_rgba8888(const fr_surface_t *surface);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_FRENDER_INCLUDE_FRENDER_FRENDER_H */
