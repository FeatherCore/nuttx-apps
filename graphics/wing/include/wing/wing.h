/****************************************************************************
 * apps/graphics/wing/include/wing/wing.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_WING_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_WING_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <nuttx/config.h>

#include <frender/frender.h>

#include <wing/core/wing_animation.h>
#include <wing/core/wing_timer.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum wing_pixel_format_e
{
  WING_PIXEL_FORMAT_RGBA8888 = 0
};

enum wing_obj_layer_e
{
  WING_OBJ_LAYER_BACKGROUND = -1000,
  WING_OBJ_LAYER_CONTENT = 0,
  WING_OBJ_LAYER_DECORATION = 10,
  WING_OBJ_LAYER_CONTROL = 20,
  WING_OBJ_LAYER_OVERLAY = 100,
  WING_OBJ_LAYER_MODAL = 200,
  WING_OBJ_LAYER_CURSOR = 300
};

struct wing_color_s
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct wing_rect_s
{
  int16_t x;
  int16_t y;
  uint16_t w;
  uint16_t h;
};

struct wing_surface_s
{
  fr_surface_t render_surface;
};

struct wing_image_resource_s
{
  const struct wing_color_s *pixels;
  uint16_t width;
  uint16_t height;
  uint16_t stride;
  enum wing_pixel_format_e format;
};

struct wing_font_s;
struct wing_bitmap_glyph_s;

typedef bool (*wing_font_get_glyph_fn_t)(
  const struct wing_font_s *font, uint32_t codepoint,
  struct wing_bitmap_glyph_s *glyph);

struct wing_bitmap_glyph_s
{
  const uint8_t *rows;
  uint8_t width;
  uint8_t height;
  uint8_t advance;
};

struct wing_font_s
{
  const char *name;
  uint8_t line_height;
  uint8_t baseline;
  uint8_t default_advance;
  wing_font_get_glyph_fn_t get_glyph;
  const void *user_data;
};

struct wing_point_s
{
  int16_t x;
  int16_t y;
};

struct wing_vec3_s
{
  int16_t x;
  int16_t y;
  int16_t z;
};

struct wing_viewport_s
{
  int16_t x;
  int16_t y;
  uint16_t w;
  uint16_t h;
};

struct wing_camera_s
{
  struct wing_vec3_s position;
  struct wing_vec3_s target;
  struct wing_viewport_s viewport;
  int16_t near_z;
  int16_t far_z;
  uint16_t focal_length;
};

struct wing_space_transform_s
{
  struct wing_vec3_s translation;
  int16_t rotation_x;
  int16_t rotation_y;
  int16_t rotation_z;
  uint16_t scale_q10;
};

struct wing_quad2d_s
{
  struct wing_point_s points[4];
};

struct wing_triangle2d_s
{
  struct wing_point_s points[3];
};

struct wing_projected_vertex_s
{
  struct wing_point_s screen;
  int32_t depth;
};

struct wing_projected_quad_s
{
  struct wing_projected_vertex_s vertices[4];
};

struct wing_projected_triangle_s
{
  struct wing_projected_vertex_s vertices[3];
};

#ifndef CONFIG_GRAPHICS_WING_CLIP_STACK_MAX
#  define CONFIG_GRAPHICS_WING_CLIP_STACK_MAX 8
#endif

struct wing_context_s
{
  struct wing_surface_s *surface;
  fr_command_list_t *commands;
  struct wing_rect_s clip;
  struct wing_rect_s clip_stack[CONFIG_GRAPHICS_WING_CLIP_STACK_MAX];
  uint8_t clip_depth;
  bool active;
};

struct wing_value_event_s
{
  uint16_t old_value;
  uint16_t value;
  uint16_t min;
  uint16_t max;
};

struct wing_state_event_s
{
  uint16_t old_state;
  uint16_t state;
  uint16_t changed;
};

struct wing_bounds_event_s
{
  struct wing_rect_s old_bounds;
  struct wing_rect_s bounds;
};

struct wing_scroll_event_s
{
  int16_t old_offset_x;
  int16_t old_offset_y;
  int16_t offset_x;
  int16_t offset_y;
  int16_t max_offset_x;
  int16_t max_offset_y;
};

struct wing_space_transform_event_s
{
  struct wing_space_transform_s old_transform;
  struct wing_space_transform_s transform;
};

struct wing_camera_event_s
{
  struct wing_camera_s old_camera;
  struct wing_camera_s camera;
};

struct wing_value_model_s
{
  uint16_t min;
  uint16_t max;
  uint16_t value;
  uint16_t step;
};

struct wing_gui_s;
struct wing_obj_s;
struct wing_event_s;
struct wing_box_style_s;
struct wing_widget_state_style_s;
struct wing_theme_s;
struct wing_bitmap_glyph_s;
struct wing_font_s;
struct wing_box_s;
struct wing_button_s;
struct wing_label_s;
struct wing_image_s;
struct wing_image_resource_s;
struct wing_panel_s;
struct wing_card_s;
struct wing_scroll_view_s;
struct wing_progress_s;
struct wing_slider_s;
struct wing_scrollbar_s;
struct wing_switch_s;
struct wing_checkbox_s;
struct wing_text_edit_s;
struct wing_text_input_s;
struct wing_input_event_s;
struct wing_input_adapter_s;
struct wing_queued_event_s;
struct wing_timer_s;
struct wing_anim_s;
struct wing_gui_frame_s;

typedef struct wing_color_s wing_color_t;
typedef struct wing_rect_s wing_rect_t;
typedef struct wing_point_s wing_point_t;
typedef struct wing_vec3_s wing_vec3_t;
typedef struct wing_viewport_s wing_viewport_t;
typedef struct wing_camera_s wing_camera_t;
typedef struct wing_space_transform_s wing_space_transform_t;
typedef struct wing_quad2d_s wing_quad2d_t;
typedef struct wing_triangle2d_s wing_triangle2d_t;
typedef struct wing_projected_vertex_s wing_projected_vertex_t;
typedef struct wing_projected_quad_s wing_projected_quad_t;
typedef struct wing_projected_triangle_s wing_projected_triangle_t;
typedef struct wing_surface_s wing_surface_t;
typedef struct wing_image_resource_s wing_image_resource_t;
typedef struct wing_bitmap_glyph_s wing_bitmap_glyph_t;
typedef struct wing_font_s wing_font_t;
typedef struct wing_context_s wing_context_t;
typedef struct wing_value_event_s wing_value_event_t;
typedef struct wing_state_event_s wing_state_event_t;
typedef struct wing_bounds_event_s wing_bounds_event_t;
typedef struct wing_scroll_event_s wing_scroll_event_t;
typedef struct wing_space_transform_event_s wing_space_transform_event_t;
typedef struct wing_camera_event_s wing_camera_event_t;
typedef struct wing_value_model_s wing_value_model_t;
typedef struct wing_obj_s wing_obj_t;
typedef struct wing_event_s wing_event_t;
typedef struct wing_box_style_s wing_box_style_t;
typedef struct wing_widget_state_style_s wing_widget_state_style_t;
typedef struct wing_theme_s wing_theme_t;
typedef struct wing_box_s wing_box_t;
typedef struct wing_button_s wing_button_t;
typedef struct wing_label_s wing_label_t;
typedef struct wing_image_s wing_image_t;
typedef struct wing_panel_s wing_panel_t;
typedef struct wing_card_s wing_card_t;
typedef struct wing_scroll_view_s wing_scroll_view_t;
typedef struct wing_progress_s wing_progress_t;
typedef struct wing_slider_s wing_slider_t;
typedef struct wing_scrollbar_s wing_scrollbar_t;
typedef struct wing_switch_s wing_switch_t;
typedef struct wing_checkbox_s wing_checkbox_t;
typedef struct wing_text_edit_s wing_text_edit_t;
typedef struct wing_text_input_s wing_text_input_t;
typedef struct wing_input_event_s wing_input_event_t;
typedef struct wing_input_adapter_s wing_input_adapter_t;
typedef struct wing_queued_event_s wing_queued_event_t;
typedef struct wing_timer_s wing_timer_t;
typedef struct wing_anim_s wing_anim_t;
typedef struct wing_gui_frame_s wing_gui_frame_t;

typedef int (*wing_gui_render_fn_t)(struct wing_context_s *ctx, void *arg);
typedef int (*wing_obj_draw_fn_t)(wing_obj_t *obj, wing_context_t *ctx);
typedef int (*wing_obj_event_fn_t)(wing_obj_t *obj, wing_event_t *event);
typedef bool (*wing_obj_screen_bounds_fn_t)(const wing_obj_t *obj,
                                            wing_rect_t *bounds);
typedef bool (*wing_obj_contains_point_fn_t)(const wing_obj_t *obj,
                                             wing_point_t point);
typedef int (*wing_button_event_fn_t)(wing_button_t *button,
                                      wing_event_t *event, void *arg);
typedef int (*wing_progress_event_fn_t)(wing_progress_t *progress,
                                        wing_event_t *event, void *arg);
typedef int (*wing_slider_event_fn_t)(wing_slider_t *slider,
                                      wing_event_t *event, void *arg);
typedef int (*wing_scroll_view_event_fn_t)(wing_scroll_view_t *view,
                                           wing_event_t *event, void *arg);
typedef int (*wing_scrollbar_event_fn_t)(wing_scrollbar_t *scrollbar,
                                         wing_event_t *event, void *arg);
typedef int (*wing_switch_event_fn_t)(wing_switch_t *sw,
                                      wing_event_t *event, void *arg);
typedef int (*wing_checkbox_event_fn_t)(wing_checkbox_t *checkbox,
                                        wing_event_t *event, void *arg);
typedef int (*wing_text_input_event_fn_t)(wing_text_input_t *input,
                                          wing_event_t *event, void *arg);
typedef int (*wing_input_read_fn_t)(wing_gui_t *gui,
                                    wing_input_event_t *input, void *arg);

enum wing_obj_flag_e
{
  WING_OBJ_FLAG_VISIBLE = 1 << 0,
  WING_OBJ_FLAG_ENABLED = 1 << 1,
  WING_OBJ_FLAG_DIRTY   = 1 << 2,
  WING_OBJ_FLAG_FOCUSABLE = 1 << 3,
  WING_OBJ_FLAG_CLIP_CHILDREN = 1 << 4
};

enum wing_obj_state_e
{
  WING_OBJ_STATE_DEFAULT  = 0,
  WING_OBJ_STATE_PRESSED  = 1 << 0,
  WING_OBJ_STATE_FOCUSED  = 1 << 1,
  WING_OBJ_STATE_DISABLED = 1 << 2,
  WING_OBJ_STATE_HOVERED  = 1 << 3,
  WING_OBJ_STATE_CHECKED  = 1 << 4,
  WING_OBJ_STATE_SELECTED = 1 << 5,
  WING_OBJ_STATE_ACTIVE   = 1 << 6
};

enum wing_event_code_e
{
  WING_EVENT_CREATE = 0,
  WING_EVENT_DELETE,
  WING_EVENT_DRAW,
  WING_EVENT_POINTER_DOWN,
  WING_EVENT_POINTER_MOVE,
  WING_EVENT_POINTER_UP,
  WING_EVENT_POINTER_ENTER,
  WING_EVENT_POINTER_LEAVE,
  WING_EVENT_CLICK,
  WING_EVENT_FOCUS_GAINED,
  WING_EVENT_FOCUS_LOST,
  WING_EVENT_KEY_DOWN,
  WING_EVENT_KEY_UP,
  WING_EVENT_ENCODER_ROTATE,
  WING_EVENT_VALUE_CHANGED,
  WING_EVENT_SCROLL_CHANGED,
  WING_EVENT_STATE_CHANGED,
  WING_EVENT_BOUNDS_CHANGED,
  WING_EVENT_SPACE_TRANSFORM_CHANGED,
  WING_EVENT_CAMERA_CHANGED,
  WING_EVENT_CLOSE_REQUEST,
  WING_EVENT_POINTER_CAPTURED,
  WING_EVENT_POINTER_RELEASED,
  WING_EVENT_POINTER_CANCELLED
};

enum wing_input_type_e
{
  WING_INPUT_POINTER_DOWN = 0,
  WING_INPUT_POINTER_MOVE,
  WING_INPUT_POINTER_UP,
  WING_INPUT_KEY_DOWN,
  WING_INPUT_KEY_UP,
  WING_INPUT_ENCODER_ROTATE,
  WING_INPUT_CLOSE_REQUEST
};

enum wing_input_source_e
{
  WING_INPUT_SOURCE_UNKNOWN = 0,
  WING_INPUT_SOURCE_MOUSE,
  WING_INPUT_SOURCE_TOUCH,
  WING_INPUT_SOURCE_KEYBOARD,
  WING_INPUT_SOURCE_ENCODER,
  WING_INPUT_SOURCE_SYSTEM
};

enum wing_key_code_e
{
  WING_KEY_UNKNOWN = 0,
  WING_KEY_BACKSPACE = 8,
  WING_KEY_TAB = 9,
  WING_KEY_ENTER = 13,
  WING_KEY_DELETE = 127,
  WING_KEY_SPACE = 32,
  WING_KEY_LEFT = 1000,
  WING_KEY_RIGHT,
  WING_KEY_UP,
  WING_KEY_DOWN
};

enum wing_layout_type_e
{
  WING_LAYOUT_FIXED = 0,
  WING_LAYOUT_STACK_VERTICAL,
  WING_LAYOUT_STACK_HORIZONTAL,
  WING_LAYOUT_CENTER,
  WING_LAYOUT_FILL
};

enum wing_axis_e
{
  WING_AXIS_HORIZONTAL = 0,
  WING_AXIS_VERTICAL
};

enum wing_text_align_e
{
  WING_TEXT_ALIGN_LEFT = 0,
  WING_TEXT_ALIGN_CENTER,
  WING_TEXT_ALIGN_RIGHT
};

enum wing_label_text_mode_e
{
  WING_LABEL_TEXT_MODE_CLIP = 0,
  WING_LABEL_TEXT_MODE_WRAP,
  WING_LABEL_TEXT_MODE_ELLIPSIS
};

#ifndef CONFIG_GRAPHICS_WING_INPUT_QUEUE_SIZE
#  define CONFIG_GRAPHICS_WING_INPUT_QUEUE_SIZE 64
#endif

#ifndef CONFIG_GRAPHICS_WING_EVENT_QUEUE_SIZE
#  define CONFIG_GRAPHICS_WING_EVENT_QUEUE_SIZE 128
#endif

#ifndef CONFIG_GRAPHICS_WING_TIMER_MAX
#  define CONFIG_GRAPHICS_WING_TIMER_MAX 8
#endif

#ifndef CONFIG_GRAPHICS_WING_ANIM_MAX
#  define CONFIG_GRAPHICS_WING_ANIM_MAX 8
#endif

#ifndef CONFIG_GRAPHICS_WING_DIRTY_RECT_MAX
#  define CONFIG_GRAPHICS_WING_DIRTY_RECT_MAX 8
#endif

#define WING_GUI_INPUT_QUEUE_SIZE CONFIG_GRAPHICS_WING_INPUT_QUEUE_SIZE
#define WING_GUI_EVENT_QUEUE_SIZE CONFIG_GRAPHICS_WING_EVENT_QUEUE_SIZE
#define WING_GUI_TIMER_MAX CONFIG_GRAPHICS_WING_TIMER_MAX
#define WING_GUI_ANIM_MAX CONFIG_GRAPHICS_WING_ANIM_MAX
#define WING_GUI_DIRTY_RECT_MAX CONFIG_GRAPHICS_WING_DIRTY_RECT_MAX
#define WING_GUI_CLIP_STACK_MAX CONFIG_GRAPHICS_WING_CLIP_STACK_MAX

struct wing_input_event_s
{
  enum wing_input_type_e type;
  enum wing_input_source_e source;
  wing_point_t point;
  uint16_t key;
  int16_t encoder_delta;
  uint8_t button;
};

struct wing_event_s
{
  enum wing_event_code_e code;
  wing_gui_t *gui;
  wing_obj_t *target;
  wing_context_t *context;
  void *data;
  bool stopped;
};

struct wing_queued_event_s
{
  wing_obj_t *target;
  enum wing_event_code_e code;
  wing_context_t *context;
  void *data;
  wing_input_event_t input;
  bool has_input;
};

struct wing_timer_s
{
  wing_timer_cb_t callback;
  void *arg;
  uint32_t period_ms;
  uint32_t elapsed_ms;
  bool repeat;
  bool active;
};

struct wing_anim_s
{
  wing_anim_apply_cb_t apply;
  wing_anim_done_cb_t done;
  void *arg;
  int32_t start_value;
  int32_t end_value;
  uint32_t duration_ms;
  uint32_t elapsed_ms;
  enum wing_anim_path_e path;
  bool active;
};

struct wing_gui_frame_s
{
  uint32_t tick_ms;
  int step_result;
  uint8_t input_polled;
  wing_rect_t dirty_before;
  wing_rect_t dirty_after_tick;
  wing_rect_t dirty_after_step;
  wing_rect_t present_rect;
  wing_rect_t present_rects[WING_GUI_DIRTY_RECT_MAX];
  uint8_t dirty_count_before;
  uint8_t dirty_count_after_tick;
  uint8_t dirty_count_after_step;
  uint8_t present_rect_count;
  uint8_t planned_redraw_count;
  uint8_t redraw_count;
  uint16_t dirty_merge_count_before;
  uint16_t dirty_merge_count_after_tick;
  uint16_t dirty_merge_count_after_step;
  bool has_dirty_before;
  bool has_dirty_after_tick;
  bool has_dirty_after_step;
  bool has_present_rect;
  bool command_capacity_fallback;
  bool redraw_cost_fallback;
};

struct wing_obj_s
{
  wing_gui_t *gui;
  wing_obj_t *parent;
  wing_obj_t *first_child;
  wing_obj_t *last_child;
  wing_obj_t *prev_sibling;
  wing_obj_t *next_sibling;
  wing_rect_t bounds;
  uint16_t flags;
  uint16_t state;
  int16_t z_index;
  uint8_t opacity;
  wing_space_transform_t space_transform;
  enum wing_layout_type_e layout;
  uint8_t padding;
  uint8_t spacing;
  wing_obj_draw_fn_t draw;
  wing_obj_event_fn_t event;
  wing_obj_screen_bounds_fn_t screen_bounds;
  wing_obj_contains_point_fn_t contains_point;
  void *user_data;
};

struct wing_box_style_s
{
  wing_color_t fill;
  wing_color_t stroke;
  uint16_t stroke_width;
  uint8_t opacity;
  bool clear;
  bool has_fill;
  bool has_stroke;
};

struct wing_widget_state_style_s
{
  wing_box_style_t pressed;
  wing_box_style_t focused;
  wing_box_style_t disabled;
  wing_box_style_t hovered;
  wing_box_style_t checked;
  wing_box_style_t selected;
  wing_box_style_t active;
  uint16_t mask;
};

struct wing_theme_s
{
  wing_box_style_t root;
  wing_box_style_t header;
  wing_box_style_t panel;
  wing_box_style_t button;
  wing_box_style_t button_focused;
  wing_box_style_t button_pressed;
  wing_box_style_t line_primary;
  wing_box_style_t line_secondary;
  wing_box_style_t progress_frame;
  wing_box_style_t progress_fill;
  wing_box_style_t slider_track;
  wing_box_style_t slider_fill;
  wing_box_style_t slider_knob;
  wing_box_style_t slider_focused;
  wing_box_style_t scrollbar_track;
  wing_box_style_t scrollbar_thumb;
  wing_box_style_t scrollbar_focused;
  wing_box_style_t switch_off;
  wing_box_style_t switch_on;
  wing_box_style_t switch_knob;
  wing_box_style_t checkbox_box;
  wing_box_style_t checkbox_checked;
  wing_box_style_t checkbox_mark;
  wing_color_t text;
};

struct wing_box_s
{
  wing_obj_t obj;
  wing_box_style_t style;
  wing_widget_state_style_t state_style;
};

struct wing_button_s
{
  wing_box_t box;
  wing_button_event_fn_t event;
  void *event_arg;
};

struct wing_label_s
{
  wing_obj_t obj;
  const char *text;
  const wing_font_t *font;
  wing_color_t color;
  enum wing_text_align_e align;
  enum wing_label_text_mode_e text_mode;
  uint8_t scale;
};

struct wing_image_s
{
  wing_obj_t obj;
  wing_image_resource_t inline_resource;
  const wing_image_resource_t *resource;
  uint8_t scale;
};

struct wing_panel_s
{
  wing_box_t box;
};

struct wing_scroll_view_s
{
  wing_box_t box;
  wing_scroll_view_event_fn_t event;
  void *event_arg;
  uint16_t content_width;
  uint16_t content_height;
  uint16_t step_x;
  uint16_t step_y;
  int16_t offset_x;
  int16_t offset_y;
};

struct wing_card_s
{
  wing_obj_t obj;
  wing_box_style_t front_style;
  wing_box_style_t back_style;
  wing_box_style_t edge_style;
};

struct wing_progress_s
{
  wing_obj_t obj;
  wing_box_style_t frame_style;
  wing_box_style_t fill_style;
  wing_progress_event_fn_t event;
  void *event_arg;
  wing_value_model_t value;
  uint8_t padding;
};

struct wing_slider_s
{
  wing_obj_t obj;
  wing_box_style_t track_style;
  wing_box_style_t fill_style;
  wing_box_style_t knob_style;
  wing_widget_state_style_t state_style;
  wing_slider_event_fn_t event;
  void *event_arg;
  wing_value_model_t value;
  uint8_t padding;
  uint8_t knob_size;
  uint8_t track_height;
};

struct wing_scrollbar_s
{
  wing_obj_t obj;
  wing_box_style_t track_style;
  wing_box_style_t thumb_style;
  wing_widget_state_style_t state_style;
  wing_scrollbar_event_fn_t event;
  void *event_arg;
  wing_value_model_t value;
  uint16_t page_size;
  uint8_t padding;
  uint8_t min_thumb_length;
  enum wing_axis_e axis;
};

struct wing_switch_s
{
  wing_obj_t obj;
  wing_box_style_t off_style;
  wing_box_style_t on_style;
  wing_box_style_t knob_style;
  wing_switch_event_fn_t event;
  void *event_arg;
  bool checked;
  uint8_t padding;
  uint8_t knob_size;
};

struct wing_checkbox_s
{
  wing_obj_t obj;
  wing_box_style_t box_style;
  wing_box_style_t checked_style;
  wing_box_style_t mark_style;
  wing_checkbox_event_fn_t event;
  void *event_arg;
  bool checked;
  uint8_t padding;
};

struct wing_text_edit_s
{
  char *buffer;
  uint16_t capacity;
  uint16_t length;
  uint16_t cursor_index;
  uint16_t selection_start;
  uint16_t selection_end;
};

struct wing_text_input_s
{
  wing_box_t box;
  wing_box_t selection;
  wing_label_t label;
  wing_box_t cursor;
  wing_text_edit_t edit;
  uint8_t padding;
  wing_text_input_event_fn_t event;
  void *event_arg;
};

struct wing_input_adapter_s
{
  wing_input_event_t pending_input;
  uint16_t coalesced_moves;
  bool has_pending_input;
};

struct wing_gui_s
{
  struct wing_surface_s *surface;
  fr_command_list_t *commands;
  struct wing_context_s context;
  wing_obj_t *root;
  wing_obj_t *pressed_obj;
  wing_obj_t *captured_obj;
  wing_obj_t *focused_obj;
  wing_obj_t *hovered_obj;
  wing_input_event_t input_queue[WING_GUI_INPUT_QUEUE_SIZE];
  wing_queued_event_t event_queue[WING_GUI_EVENT_QUEUE_SIZE];
  wing_timer_t timers[WING_GUI_TIMER_MAX];
  wing_anim_t animations[WING_GUI_ANIM_MAX];
  wing_camera_t camera;
  wing_gui_render_fn_t render;
  void *render_arg;
  const wing_theme_t *theme;
  wing_input_read_fn_t input_read;
  void *input_read_arg;
  wing_rect_t dirty_rect;
  wing_rect_t dirty_rects[WING_GUI_DIRTY_RECT_MAX];
  wing_rect_t last_redraw_rects[WING_GUI_DIRTY_RECT_MAX];
  uint32_t tick_ms;
  uint32_t last_frame_ms;
  uint32_t frame_interval_ms;
  uint8_t input_head;
  uint8_t input_tail;
  uint8_t input_count;
  uint8_t event_head;
  uint8_t event_tail;
  uint8_t event_count;
  uint8_t dirty_rect_count;
  uint8_t last_planned_redraw_count;
  uint8_t last_redraw_count;
  uint16_t dirty_merge_count;
  bool has_dirty_rect;
  bool last_command_capacity_fallback;
  bool last_redraw_cost_fallback;
  bool dirty;
  bool running;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#include <wing/core/wing_render_node.h>

#ifdef __cplusplus
extern "C"
{
#endif

wing_color_t wing_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t wing_color_pack_rgba8888(wing_color_t color);
void wing_theme_init_default(wing_theme_t *theme);
void wing_theme_init_high_contrast(wing_theme_t *theme);

#include <wing/core/wing_capture.h>
#include <wing/core/wing_focus.h>
#include <wing/core/wing_runtime.h>
#include <wing/core/wing_render.h>
#include <wing/core/wing_input.h>
#include <wing/core/wing_event.h>
#include <wing/core/wing_object.h>
#include <wing/core/wing_space.h>
#include <wing/core/wing_font.h>
#include <wing/core/wing_text_edit.h>
#include <wing/core/wing_theme.h>
#include <wing/core/wing_tick.h>
#include <wing/widgets/wing_widgets.h>

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_WING_H */
