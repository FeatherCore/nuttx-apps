/****************************************************************************
 * apps/examples/wing_gui_demo/wing_gui_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <frender/frender.h>
#include <wing/wing.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_WING_GUI_DEMO_WIDTH
#  define CONFIG_EXAMPLES_WING_GUI_DEMO_WIDTH 320
#endif

#ifndef CONFIG_EXAMPLES_WING_GUI_DEMO_HEIGHT
#  define CONFIG_EXAMPLES_WING_GUI_DEMO_HEIGHT 240
#endif

#define WING_GUI_DEMO_COMMANDS 2048
#define WING_GUI_DEMO_FRAME_MS 33
#define WING_GUI_DEMO_HEADLESS_FRAMES 6

#define WING_GUI_DEMO_MARGIN_X 20
#define WING_GUI_DEMO_HEADER_HEIGHT 34

#define WING_GUI_DEMO_CHECKBOX_X 20
#define WING_GUI_DEMO_CHECKBOX_Y 8
#define WING_GUI_DEMO_CHECKBOX_SIZE 18
#define WING_GUI_DEMO_CHECKBOX_PADDING 4
#define WING_GUI_DEMO_CHECKBOX_INPUT_X 29
#define WING_GUI_DEMO_CHECKBOX_INPUT_Y 17

#define WING_GUI_DEMO_SWITCH_RIGHT_MARGIN 20
#define WING_GUI_DEMO_SWITCH_Y 6
#define WING_GUI_DEMO_SWITCH_WIDTH 44
#define WING_GUI_DEMO_SWITCH_HEIGHT 22
#define WING_GUI_DEMO_SWITCH_PADDING 2
#define WING_GUI_DEMO_SWITCH_INPUT_RIGHT_OFFSET 42
#define WING_GUI_DEMO_SWITCH_INPUT_Y 17

#define WING_GUI_DEMO_TEXT_INPUT_X 96
#define WING_GUI_DEMO_TEXT_INPUT_Y 36
#define WING_GUI_DEMO_TEXT_INPUT_W 78
#define WING_GUI_DEMO_TEXT_INPUT_H 18
#define WING_GUI_DEMO_TEXT_INPUT_PADDING 3
#define WING_GUI_DEMO_TEXT_INPUT_BUFFER 16
#define WING_GUI_DEMO_TEXT_INPUT_INPUT_X 104
#define WING_GUI_DEMO_TEXT_INPUT_INPUT_Y 44

#define WING_GUI_DEMO_CARD_X 20
#define WING_GUI_DEMO_CARD_Y 54
#define WING_GUI_DEMO_CARD_H 74
#define WING_GUI_DEMO_CARD_PAD_X 16
#define WING_GUI_DEMO_CARD_PAD_Y 16

#define WING_GUI_DEMO_BUTTON_W 72
#define WING_GUI_DEMO_BUTTON_H 28
#define WING_GUI_DEMO_BUTTON_INPUT_X 50
#define WING_GUI_DEMO_BUTTON_INPUT_Y 86

#define WING_GUI_DEMO_LABEL_W 30
#define WING_GUI_DEMO_LABEL_H 10
#define WING_GUI_DEMO_LABEL_SCALE 1
#define WING_GUI_DEMO_LABEL_SPACE_ROTATION_Y 10
#define WING_GUI_DEMO_LABEL_SPACE_Z 6

#define WING_GUI_DEMO_LINE_PRIMARY_W 64
#define WING_GUI_DEMO_LINE_PRIMARY_H 14
#define WING_GUI_DEMO_LINE_SECONDARY_X 124
#define WING_GUI_DEMO_LINE_SECONDARY_Y 94
#define WING_GUI_DEMO_LINE_SECONDARY_W 156
#define WING_GUI_DEMO_LINE_SECONDARY_H 12

#define WING_GUI_DEMO_FILL_PANEL_X 244
#define WING_GUI_DEMO_FILL_PANEL_Y 132
#define WING_GUI_DEMO_FILL_PANEL_W 56
#define WING_GUI_DEMO_FILL_PANEL_H 24
#define WING_GUI_DEMO_FILL_PANEL_PAD_X 4
#define WING_GUI_DEMO_FILL_BADGE_SIZE 4
#define WING_GUI_DEMO_FILL_PANEL_OPACITY 192
#define WING_GUI_DEMO_FILL_PANEL_ROTATION_Y -18
#define WING_GUI_DEMO_FILL_PANEL_Z 8

#define WING_GUI_DEMO_SCROLL_VIEW_X 20
#define WING_GUI_DEMO_SCROLL_VIEW_Y 132
#define WING_GUI_DEMO_SCROLL_VIEW_W 64
#define WING_GUI_DEMO_SCROLL_VIEW_H 24
#define WING_GUI_DEMO_SCROLL_CONTENT_X 24
#define WING_GUI_DEMO_SCROLL_CONTENT_Y 137
#define WING_GUI_DEMO_SCROLL_CONTENT_W 92
#define WING_GUI_DEMO_SCROLL_CONTENT_H 9
#define WING_GUI_DEMO_SCROLL_ACCENT_X 24
#define WING_GUI_DEMO_SCROLL_ACCENT_Y 149
#define WING_GUI_DEMO_SCROLL_ACCENT_W 76
#define WING_GUI_DEMO_SCROLL_ACCENT_H 5
#define WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_W 116
#define WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_H 24
#define WING_GUI_DEMO_SCROLL_OFFSET_X 22
#define WING_GUI_DEMO_SCROLL_OFFSET_Y 0
#define WING_GUI_DEMO_SCROLL_STEP_X 8
#define WING_GUI_DEMO_SCROLL_STEP_Y 4
#define WING_GUI_DEMO_SCROLL_INPUT_X 82
#define WING_GUI_DEMO_SCROLL_INPUT_Y 134

#define WING_GUI_DEMO_CLIP_PANEL_X 176
#define WING_GUI_DEMO_CLIP_PANEL_Y 132
#define WING_GUI_DEMO_CLIP_PANEL_W 40
#define WING_GUI_DEMO_CLIP_PANEL_H 20
#define WING_GUI_DEMO_CLIP_CHILD_X 196
#define WING_GUI_DEMO_CLIP_CHILD_Y 136
#define WING_GUI_DEMO_CLIP_CHILD_W 34
#define WING_GUI_DEMO_CLIP_CHILD_H 12

#define WING_GUI_DEMO_SPACE_CARD_X 232
#define WING_GUI_DEMO_SPACE_CARD_Y 76
#define WING_GUI_DEMO_SPACE_CARD_W 54
#define WING_GUI_DEMO_SPACE_CARD_H 34
#define WING_GUI_DEMO_SPACE_CARD_INPUT_X 276
#define WING_GUI_DEMO_SPACE_CARD_INPUT_Y 90
#define WING_GUI_DEMO_SPACE_CARD_ROTATION_START -32
#define WING_GUI_DEMO_SPACE_CARD_ROTATION_END 34
#define WING_GUI_DEMO_SPACE_CARD_Z WING_OBJ_LAYER_DECORATION
#define WING_GUI_DEMO_SPACE_CARD_OPACITY 192
#define WING_GUI_DEMO_SPACE_CARD_EDGE_OPACITY 224

#define WING_GUI_DEMO_DEPTH_CARD_X 150
#define WING_GUI_DEMO_DEPTH_CARD_Y 90
#define WING_GUI_DEMO_DEPTH_CARD_W 58
#define WING_GUI_DEMO_DEPTH_CARD_H 30
#define WING_GUI_DEMO_DEPTH_CARD_INPUT_X 178
#define WING_GUI_DEMO_DEPTH_CARD_INPUT_Y 104
#define WING_GUI_DEMO_DEPTH_CARD_Z_INDEX (WING_OBJ_LAYER_CONTENT + 4)
#define WING_GUI_DEMO_DEPTH_BACK_Z 36
#define WING_GUI_DEMO_DEPTH_FRONT_Z -24

#define WING_GUI_DEMO_CONTROL_Z_INDEX WING_OBJ_LAYER_CONTROL
#define WING_GUI_DEMO_SCROLLBAR_Z_INDEX WING_GUI_DEMO_CONTROL_Z_INDEX
#define WING_GUI_DEMO_SLIDER_Z_INDEX (WING_GUI_DEMO_CONTROL_Z_INDEX + 1)
#define WING_GUI_DEMO_PROGRESS_Z_INDEX (WING_GUI_DEMO_CONTROL_Z_INDEX + 2)

#define WING_GUI_DEMO_PROGRESS_Y 148
#define WING_GUI_DEMO_PROGRESS_H 20
#define WING_GUI_DEMO_PROGRESS_PADDING 2
#define WING_GUI_DEMO_PROGRESS_MIN 0
#define WING_GUI_DEMO_PROGRESS_MAX 100
#define WING_GUI_DEMO_PROGRESS_INITIAL 70
#define WING_GUI_DEMO_PROGRESS_TARGET 90
#define WING_GUI_DEMO_PROGRESS_STEP 6
#define WING_GUI_DEMO_PROGRESS_INPUT_START_X 32
#define WING_GUI_DEMO_PROGRESS_INPUT_DRAG_X 340
#define WING_GUI_DEMO_PROGRESS_INPUT_Y 158

#define WING_GUI_DEMO_SLIDER_Y 184
#define WING_GUI_DEMO_SLIDER_H 26
#define WING_GUI_DEMO_SLIDER_PADDING 4
#define WING_GUI_DEMO_SLIDER_KNOB_SIZE 14
#define WING_GUI_DEMO_SLIDER_TRACK_HEIGHT 6
#define WING_GUI_DEMO_SLIDER_STEP 7
#define WING_GUI_DEMO_SLIDER_INITIAL 25
#define WING_GUI_DEMO_SLIDER_INPUT_X 44
#define WING_GUI_DEMO_SLIDER_INPUT_Y 196

#define WING_GUI_DEMO_SCROLLBAR_Y 220
#define WING_GUI_DEMO_SCROLLBAR_H 12
#define WING_GUI_DEMO_SCROLLBAR_PADDING 2
#define WING_GUI_DEMO_SCROLLBAR_MIN_THUMB 8
#define WING_GUI_DEMO_SCROLLBAR_STEP 9
#define WING_GUI_DEMO_SCROLLBAR_INITIAL 20
#define WING_GUI_DEMO_SCROLLBAR_PAGE_SIZE 25
#define WING_GUI_DEMO_SCROLLBAR_INPUT_X 40
#define WING_GUI_DEMO_SCROLLBAR_INPUT_Y 226

#define WING_GUI_DEMO_POINTER_OUTSIDE_X 340

#define WING_GUI_DEMO_TOAST_X 86
#define WING_GUI_DEMO_TOAST_Y 132
#define WING_GUI_DEMO_TOAST_W 148
#define WING_GUI_DEMO_TOAST_H 24
#define WING_GUI_DEMO_TOAST_LABEL_X 96
#define WING_GUI_DEMO_TOAST_LABEL_Y 139
#define WING_GUI_DEMO_TOAST_LABEL_W 64
#define WING_GUI_DEMO_TOAST_LABEL_H 10

#define WING_GUI_DEMO_IMAGE_X 286
#define WING_GUI_DEMO_IMAGE_Y 150
#define WING_GUI_DEMO_IMAGE_W 20
#define WING_GUI_DEMO_IMAGE_H 20
#define WING_GUI_DEMO_IMAGE_SRC_W 4
#define WING_GUI_DEMO_IMAGE_SRC_H 4
#define WING_GUI_DEMO_IMAGE_SCALE 5
#define WING_GUI_DEMO_IMAGE_OPACITY 196
#define WING_GUI_DEMO_IMAGE_ROTATION_Y -12
#define WING_GUI_DEMO_IMAGE_Z 10

#define WING_GUI_DEMO_TRIANGLE_X 314
#define WING_GUI_DEMO_TRIANGLE_Y 96
#define WING_GUI_DEMO_TRIANGLE_W 42
#define WING_GUI_DEMO_TRIANGLE_H 36
#define WING_GUI_DEMO_TRIANGLE_OPACITY 210
#define WING_GUI_DEMO_TRIANGLE_ROTATION_Y -18
#define WING_GUI_DEMO_TRIANGLE_Z 14

#define WING_GUI_DEMO_THEME_TIMER_FRAMES 2
#define WING_GUI_DEMO_PROGRESS_TIMER_FRAMES 1
#define WING_GUI_DEMO_REPEAT_TIMER_FRAMES 2
#define WING_GUI_DEMO_REPEAT_STOP_TICKS 3
#define WING_GUI_DEMO_ANIMATION_FRAMES 4
#define WING_GUI_DEMO_ENABLE_LOOP_ANIMATION 1
#define WING_GUI_DEMO_TRACE_EACH_FRAME 0
#define WING_GUI_DEMO_PULSE_LOG_INTERVAL 8
#define WING_GUI_DEMO_DISABLE_TIMER_FRAMES 3
#define WING_GUI_DEMO_TOAST_CREATE_FRAMES 4
#define WING_GUI_DEMO_TOAST_HIDE_FRAMES 5
#define WING_GUI_DEMO_TOAST_DESTROY_FRAMES 6

#define WING_GUI_DEMO_BUTTON_CLICK_FILL wing_color_rgba(94, 177, 137, 255)
#define WING_GUI_DEMO_BUTTON_HOVER_FILL wing_color_rgba(76, 140, 224, 255)
#define WING_GUI_DEMO_BUTTON_DISABLED_FILL wing_color_rgba(92, 96, 102, 255)
#define WING_GUI_DEMO_BUTTON_DISABLED_STROKE wing_color_rgba(54, 58, 64, 255)
#define WING_GUI_DEMO_SELECTED_FILL wing_color_rgba(255, 207, 105, 255)
#define WING_GUI_DEMO_ACTIVE_FILL wing_color_rgba(72, 178, 166, 255)
#define WING_GUI_DEMO_REPEAT_FIRST_FILL wing_color_rgba(241, 201, 91, 255)
#define WING_GUI_DEMO_REPEAT_SECOND_FILL wing_color_rgba(83, 147, 227, 255)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct wing_gui_demo_progress_timer_s
{
  wing_progress_t *progress;
  uint16_t target_value;
};

struct wing_gui_demo_width_anim_s
{
  wing_box_t *box;
};

struct wing_gui_demo_loop_width_anim_s
{
  wing_box_t *box;
  uint16_t min_width;
  uint16_t max_width;
  uint16_t segments_completed;
  bool forward;
};

struct wing_gui_demo_space_anim_s
{
  wing_card_t *card;
};

struct wing_gui_demo_theme_timer_s
{
  const wing_theme_t *theme;
  wing_box_t *root;
  wing_box_t *header;
  wing_panel_t *card;
  wing_button_t *button;
  wing_label_t *label;
  wing_box_t *line_primary;
  wing_box_t *line_secondary;
  wing_box_t *fill_panel;
  wing_box_t *fill_badge;
  wing_card_t *space_card;
  wing_progress_t *progress;
  wing_slider_t *slider;
  wing_scrollbar_t *scrollbar;
  wing_switch_t *power_switch;
  wing_checkbox_t *checkbox;
};

struct wing_gui_demo_disable_timer_s
{
  wing_button_t *button;
};

struct wing_gui_demo_repeat_timer_s
{
  wing_box_t *line;
  wing_box_style_t first_style;
  wing_box_style_t second_style;
  uint8_t timer_id;
  uint8_t tick_count;
};

struct wing_gui_demo_toast_s
{
  wing_box_t toast;
  wing_label_t label;
  wing_obj_t *root;
  bool active;
};

struct wing_gui_demo_input_provider_s
{
  fr_fb_presenter_t *presenter;
  wing_input_adapter_t adapter;
  wing_input_event_t events[2];
  uint32_t raw_mouse_seq;
  uint8_t count;
  uint8_t index;
  bool raw_mouse_pressed;
};

struct wing_gui_demo_triangle_s
{
  wing_gui_t *gui;
  int16_t rotation_y;
  int16_t z;
  uint8_t opacity;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int wing_gui_demo_state_probe_event(wing_obj_t *obj,
                                           wing_event_t *event);
static int wing_gui_demo_triangle_project(
  const struct wing_gui_demo_triangle_s *demo, const wing_rect_t *bounds,
  wing_projected_triangle_t *projected);
#if WING_GUI_DEMO_ENABLE_LOOP_ANIMATION
static void wing_gui_demo_loop_width_anim_done(wing_gui_t *gui, void *arg);
#endif
static bool wing_gui_demo_triangle_screen_bounds(const wing_obj_t *obj,
                                                 wing_rect_t *bounds);
static bool wing_gui_demo_triangle_contains_point(const wing_obj_t *obj,
                                                  wing_point_t point);

static const wing_color_t g_wing_gui_demo_image_pixels[] =
{
  {  38,  70, 110, 255 }, {  84, 162, 255, 255 },
  {  84, 162, 255, 255 }, {  38,  70, 110, 255 },
  {  84, 162, 255, 255 }, { 255, 207, 105, 255 },
  { 255, 207, 105, 255 }, {  84, 162, 255, 255 },
  {  84, 162, 255, 255 }, { 255, 207, 105, 255 },
  { 241, 107, 107, 255 }, {  84, 162, 255, 255 },
  {  38,  70, 110, 255 }, {  84, 162, 255, 255 },
  {  84, 162, 255, 255 }, {  38,  70, 110, 255 }
};

static const wing_image_resource_t g_wing_gui_demo_image_resource =
{
  g_wing_gui_demo_image_pixels,
  WING_GUI_DEMO_IMAGE_SRC_W,
  WING_GUI_DEMO_IMAGE_SRC_H,
  WING_GUI_DEMO_IMAGE_SRC_W,
  WING_PIXEL_FORMAT_RGBA8888
};

static int wing_gui_demo_triangle_project(
  const struct wing_gui_demo_triangle_s *demo, const wing_rect_t *bounds,
  wing_projected_triangle_t *projected)
{
  const wing_camera_t *camera;
  wing_space_transform_t transform;
  wing_vec3_t local[3];
  int32_t depth;
  int32_t viewport_cx;
  int32_t viewport_cy;

  if (demo == NULL || bounds == NULL || projected == NULL ||
      demo->gui == NULL)
    {
      return -EINVAL;
    }

  camera = wing_gui_get_camera(demo->gui);
  if (camera == NULL)
    {
      return -EINVAL;
    }

  viewport_cx = (int32_t)camera->viewport.x + camera->viewport.w / 2;
  viewport_cy = (int32_t)camera->viewport.y + camera->viewport.h / 2;

  wing_space_transform_init(&transform);
  transform.translation.z = demo->z;
  depth = (int32_t)transform.translation.z - camera->position.z;
  if (depth < camera->near_z)
    {
      depth = camera->near_z;
    }

  if (depth > camera->far_z)
    {
      depth = camera->far_z;
    }

  transform.translation.x =
    (int16_t)(((int32_t)bounds->x + bounds->w / 2 - viewport_cx) *
              depth / camera->focal_length);
  transform.translation.y =
    (int16_t)(((int32_t)bounds->y + bounds->h / 2 - viewport_cy) *
              depth / camera->focal_length);
  transform.rotation_y = demo->rotation_y;

  local[0].x = 0;
  local[0].y = (int16_t)(-((int16_t)bounds->h / 2));
  local[0].z = 0;
  local[1].x = (int16_t)(-((int16_t)bounds->w / 2));
  local[1].y = (int16_t)(bounds->h / 2);
  local[1].z = 0;
  local[2].x = (int16_t)(bounds->w / 2);
  local[2].y = (int16_t)(bounds->h / 2);
  local[2].z = 0;

  return wing_project_projected_triangle(camera, &transform, local,
                                         projected);
}

static int wing_gui_demo_triangle_draw(wing_obj_t *obj,
                                       wing_context_t *ctx)
{
  struct wing_gui_demo_triangle_s *demo;
  const wing_rect_t *bounds;
  wing_projected_triangle_t projected;
  wing_render_node_t node;
  uint8_t i;

  bounds = wing_obj_get_bounds(obj);
  demo = (struct wing_gui_demo_triangle_s *)wing_obj_get_user_data(obj);
  if (wing_gui_demo_triangle_project(demo, bounds, &projected) < 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < 3; i++)
    {
      node.geometry.triangle.points[i] = projected.vertices[i].screen;
    }

  node.type = WING_RENDER_NODE_FILL_TRIANGLE;
  node.material =
    wing_render_material_color(wing_color_rgba(90, 176, 134,
                                               demo->opacity));
  node.thickness = 0;
  node.source = NULL;
  node.src_rect = (wing_rect_t){ 0, 0, 0, 0 };

  return wing_gui_submit_render_node(ctx, &node);
}

static bool wing_gui_demo_triangle_screen_bounds(const wing_obj_t *obj,
                                                 wing_rect_t *bounds)
{
  struct wing_gui_demo_triangle_s *demo;
  wing_projected_triangle_t projected;

  if (obj == NULL || bounds == NULL)
    {
      return false;
    }

  demo = (struct wing_gui_demo_triangle_s *)wing_obj_get_user_data(obj);
  if (wing_gui_demo_triangle_project(demo, wing_obj_get_bounds(obj),
                                     &projected) < 0)
    {
      return false;
    }

  return wing_projected_triangle_get_bounds(&projected, bounds) == 0;
}

static bool wing_gui_demo_triangle_contains_point(const wing_obj_t *obj,
                                                  wing_point_t point)
{
  struct wing_gui_demo_triangle_s *demo;
  wing_projected_triangle_t projected;

  if (obj == NULL)
    {
      return false;
    }

  demo = (struct wing_gui_demo_triangle_s *)wing_obj_get_user_data(obj);
  if (wing_gui_demo_triangle_project(demo, wing_obj_get_bounds(obj),
                                     &projected) < 0)
    {
      return false;
    }

  return wing_projected_triangle_contains_point(&projected, point);
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int wing_gui_demo_present(fr_fb_presenter_t *presenter,
                                 const wing_surface_t *surface,
                                 const wing_gui_frame_t *frame)
{
  fr_rect_t present_rect;
  const wing_rect_t *dirty;
  uint8_t i;
  int ret;

  if (frame != NULL && frame->has_present_rect &&
      frame->present_rect_count > 0)
    {
      for (i = 0; i < frame->present_rect_count; i++)
        {
          dirty = &frame->present_rects[i];
          present_rect.x = dirty->x;
          present_rect.y = dirty->y;
          present_rect.w = dirty->w;
          present_rect.h = dirty->h;

          ret = fr_fb_presenter_present_rect(presenter,
                                             &surface->render_surface,
                                             &present_rect);
          if (ret < 0)
            {
              return ret;
            }

          ret = fr_fb_presenter_update_rect(presenter, &present_rect);
          if (ret < 0)
            {
              return ret;
            }
        }

      return 0;
    }

  present_rect.x = 0;
  present_rect.y = 0;
  present_rect.w = surface->render_surface.width;
  present_rect.h = surface->render_surface.height;

  ret = fr_fb_presenter_present_rect(presenter, &surface->render_surface,
                                     &present_rect);
  if (ret < 0)
    {
      return ret;
    }

  return fr_fb_presenter_update_rect(presenter, &present_rect);
}

static const char *wing_gui_demo_input_type_name(enum wing_input_type_e type)
{
  switch (type)
    {
      case WING_INPUT_POINTER_DOWN:
        return "pointer_down";
      case WING_INPUT_POINTER_MOVE:
        return "pointer_move";
      case WING_INPUT_POINTER_UP:
        return "pointer_up";
      case WING_INPUT_KEY_DOWN:
        return "key_down";
      case WING_INPUT_KEY_UP:
        return "key_up";
      case WING_INPUT_ENCODER_ROTATE:
        return "encoder_rotate";
      default:
        return "unknown";
    }
}

static void wing_gui_demo_print_stage(const char *stage)
{
  printf("wing_gui_demo: === %s ===\n", stage);
}

static const char *wing_gui_demo_input_source_name(
  enum wing_input_source_e source)
{
  switch (source)
    {
      case WING_INPUT_SOURCE_MOUSE:
        return "mouse";
      case WING_INPUT_SOURCE_TOUCH:
        return "touch";
      case WING_INPUT_SOURCE_KEYBOARD:
        return "keyboard";
      case WING_INPUT_SOURCE_ENCODER:
        return "encoder";
      case WING_INPUT_SOURCE_SYSTEM:
        return "system";
      case WING_INPUT_SOURCE_UNKNOWN:
      default:
        return "unknown";
    }
}

static const char *wing_gui_demo_fr_input_type_name(enum fr_input_type_e type)
{
  switch (type)
    {
      case FR_INPUT_POINTER_DOWN:
        return "pointer_down";
      case FR_INPUT_POINTER_MOVE:
        return "pointer_move";
      case FR_INPUT_POINTER_UP:
        return "pointer_up";
      case FR_INPUT_KEY_DOWN:
        return "key_down";
      case FR_INPUT_KEY_UP:
        return "key_up";
      case FR_INPUT_ENCODER_ROTATE:
        return "encoder_rotate";
      default:
        return "unknown";
    }
}

static void wing_gui_demo_print_raw_fr_input(
  struct wing_gui_demo_input_provider_s *provider,
  const fr_input_event_t *input)
{
  if (provider == NULL || input == NULL)
    {
      return;
    }

  if (input->type == FR_INPUT_POINTER_DOWN)
    {
      provider->raw_mouse_pressed = true;
    }
  else if (input->type == FR_INPUT_POINTER_UP)
    {
      provider->raw_mouse_pressed = false;
    }

  provider->raw_mouse_seq++;

  printf("wing_gui_demo: x11 raw input seq=%lu type=%s pressed=%s point=%d,%d button=%u key=%u encoder=%d\n",
         (unsigned long)provider->raw_mouse_seq,
         wing_gui_demo_fr_input_type_name(input->type),
         provider->raw_mouse_pressed ? "yes" : "no",
         input->x, input->y, (unsigned int)input->button,
         (unsigned int)input->key, (int)input->encoder_delta);
}

static bool wing_gui_demo_convert_fr_input(const fr_input_event_t *fr_input,
                                           wing_input_event_t *input)
{
  if (fr_input == NULL || input == NULL)
    {
      return false;
    }

  input->point.x = fr_input->x;
  input->point.y = fr_input->y;
  input->key = fr_input->key;
  input->encoder_delta = fr_input->encoder_delta;
  input->button = fr_input->button;

  switch (fr_input->type)
    {
      case FR_INPUT_POINTER_DOWN:
        input->type = WING_INPUT_POINTER_DOWN;
        input->source = WING_INPUT_SOURCE_MOUSE;
        return true;
      case FR_INPUT_POINTER_MOVE:
        input->type = WING_INPUT_POINTER_MOVE;
        input->source = WING_INPUT_SOURCE_MOUSE;
        return true;
      case FR_INPUT_POINTER_UP:
        input->type = WING_INPUT_POINTER_UP;
        input->source = WING_INPUT_SOURCE_MOUSE;
        return true;
      case FR_INPUT_KEY_DOWN:
        input->type = WING_INPUT_KEY_DOWN;
        input->source = WING_INPUT_SOURCE_KEYBOARD;
        return true;
      case FR_INPUT_KEY_UP:
        input->type = WING_INPUT_KEY_UP;
        input->source = WING_INPUT_SOURCE_KEYBOARD;
        return true;
      case FR_INPUT_ENCODER_ROTATE:
        input->type = WING_INPUT_ENCODER_ROTATE;
        input->source = WING_INPUT_SOURCE_ENCODER;
        return true;
      default:
        return false;
    }
}

static void wing_gui_demo_print_dirty_rect(const char *label, bool has_dirty,
                                           const wing_rect_t *dirty)
{
  if (has_dirty && dirty != NULL)
    {
      printf("wing_gui_demo: dirty %s x=%d y=%d w=%u h=%u\n",
             label == NULL ? "unnamed" : label,
             (int)dirty->x, (int)dirty->y,
             (unsigned int)dirty->w, (unsigned int)dirty->h);
    }
  else
    {
      printf("wing_gui_demo: dirty %s none\n",
             label == NULL ? "unnamed" : label);
    }
}

static void wing_gui_demo_print_dirty_count(const char *label,
                                            uint8_t count)
{
  printf("wing_gui_demo: dirty list %s count=%u\n",
         label == NULL ? "unnamed" : label, (unsigned int)count);
}

static void wing_gui_demo_print_dirty_merge_count(const char *label,
                                                  uint16_t count)
{
  printf("wing_gui_demo: dirty merge %s count=%u\n",
         label == NULL ? "unnamed" : label, (unsigned int)count);
}

static void wing_gui_demo_print_dirty(const char *label,
                                      const wing_gui_t *gui)
{
  wing_rect_t dirty;

  wing_gui_demo_print_dirty_rect(label,
                                 wing_gui_get_dirty_rect(gui, &dirty),
                                 &dirty);
  wing_gui_demo_print_dirty_count(label, wing_gui_get_dirty_rect_count(gui));
  wing_gui_demo_print_dirty_merge_count(
    label, wing_gui_get_dirty_merge_count(gui));
}

static void wing_gui_demo_enqueue_input(wing_gui_t *gui,
                                        const wing_input_event_t *input,
                                        const char *label)
{
  int ret;

  ret = wing_gui_enqueue_input(gui, input);
  if (ret < 0)
    {
      printf("wing_gui_demo: input enqueue failed label=%s ret=%d\n",
             label == NULL ? "unnamed" : label, ret);
      return;
    }

  printf("wing_gui_demo: input queued label=%s type=%s source=%s point=%d,%d key=%u\n",
         label == NULL ? "unnamed" : label,
         input == NULL ? "null" : wing_gui_demo_input_type_name(input->type),
         input == NULL ? "null" :
         wing_gui_demo_input_source_name(input->source),
         input == NULL ? 0 : input->point.x,
         input == NULL ? 0 : input->point.y,
         input == NULL ? 0 : (unsigned int)input->key);
}

static int wing_gui_demo_input_provider(wing_gui_t *gui,
                                        wing_input_event_t *input,
                                        void *arg)
{
  struct wing_gui_demo_input_provider_s *provider;
  fr_input_event_t fr_input;
  fr_input_event_t next_input;
  wing_input_event_t next;
  uint16_t coalesced;
  int ret;

  (void)gui;

  provider = (struct wing_gui_demo_input_provider_s *)arg;
  if (provider == NULL || input == NULL)
    {
      return 0;
    }

  ret = wing_input_adapter_take_pending(&provider->adapter, input);
  if (ret < 0)
    {
      return ret;
    }

  if (ret > 0)
    {
      printf("wing_gui_demo: x11 input provider emitted pending type=%s source=%s point=%d,%d key=%u button=%u encoder=%d\n",
             wing_gui_demo_input_type_name(input->type),
             wing_gui_demo_input_source_name(input->source),
             input->point.x, input->point.y, (unsigned int)input->key,
             (unsigned int)input->button,
             (int)input->encoder_delta);
      return 1;
    }

  if (provider->presenter != NULL)
    {
      ret = fr_fb_presenter_poll_input(provider->presenter, &fr_input);
      if (ret < 0)
        {
          return ret;
        }

      if (ret > 0)
        {
          wing_gui_demo_print_raw_fr_input(provider, &fr_input);

          provider->adapter.coalesced_moves = 0;
          if (fr_input.type == FR_INPUT_POINTER_MOVE)
            {
              while (true)
                {
                  ret = fr_fb_presenter_poll_input(provider->presenter,
                                                   &next_input);
                  if (ret < 0)
                    {
                      return ret;
                    }

                  if (ret == 0)
                    {
                      break;
                    }

                  wing_gui_demo_print_raw_fr_input(provider, &next_input);

                  if (!wing_gui_demo_convert_fr_input(&next_input, &next))
                    {
                      continue;
                    }

                  if (!wing_gui_demo_convert_fr_input(&fr_input, input))
                    {
                      return 0;
                    }

                  if (!wing_input_adapter_merge_pointer_move(
                        &provider->adapter, input, &next))
                    {
                      break;
                    }

                  fr_input.x = input->point.x;
                  fr_input.y = input->point.y;
                  fr_input.key = input->key;
                  fr_input.encoder_delta = input->encoder_delta;
                  fr_input.button = input->button;
                }
            }

          if (!wing_gui_demo_convert_fr_input(&fr_input, input))
            {
              return 0;
            }

          coalesced =
            wing_input_adapter_get_coalesced_moves(&provider->adapter);
          printf("wing_gui_demo: x11 input provider emitted type=%s source=%s point=%d,%d key=%u button=%u encoder=%d coalesced_moves=%u\n",
                 wing_gui_demo_input_type_name(input->type),
                 wing_gui_demo_input_source_name(input->source),
                 input->point.x, input->point.y, (unsigned int)input->key,
                 (unsigned int)input->button,
                 (int)input->encoder_delta, (unsigned int)coalesced);
          return 1;
        }
    }

  if (provider->index >= provider->count)
    {
      return 0;
    }

  *input = provider->events[provider->index++];
  printf("wing_gui_demo: input provider emitted index=%u type=%s source=%s point=%d,%d key=%u\n",
         (unsigned int)provider->index,
         wing_gui_demo_input_type_name(input->type),
         wing_gui_demo_input_source_name(input->source),
         input->point.x, input->point.y, (unsigned int)input->key);

  return 1;
}

static int wing_gui_demo_button_event(wing_button_t *button,
                                      wing_event_t *event, void *arg)
{
  const wing_box_style_t *current;
  wing_box_style_t next;
  wing_box_t *box;

  (void)arg;

  if (button == NULL || event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      current = wing_box_get_active_style(wing_button_box(button));
      if (current != NULL)
        {
          printf("wing_gui_demo: wing_button focus gained by WING runtime and state style active fill=%u,%u,%u\n",
                 current->fill.r, current->fill.g, current->fill.b);
        }
      else
        {
          printf("wing_gui_demo: wing_button focus gained by WING runtime\n");
        }

      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_button received key down=%u source=%s from focused input path\n",
                 (unsigned int)input->key,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_POINTER_ENTER)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      current = wing_box_get_active_style(wing_button_box(button));
      if (current != NULL)
        {
          printf("wing_gui_demo: wing_button pointer enter source=%s set hovered state style fill=%u,%u,%u\n",
                 input == NULL ? "unknown" :
                 wing_gui_demo_input_source_name(input->source),
                 current->fill.r, current->fill.g, current->fill.b);
        }
      else
        {
          printf("wing_gui_demo: wing_button pointer enter source=%s set hovered state\n",
                 input == NULL ? "unknown" :
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_POINTER_LEAVE)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      printf("wing_gui_demo: wing_button pointer leave source=%s cleared hovered state\n",
             input == NULL ? "unknown" :
             wing_gui_demo_input_source_name(input->source));
      return 0;
    }

  if (event->code != WING_EVENT_CLICK)
    {
      return 0;
    }

  box = wing_button_box(button);
  current = wing_box_get_style(box);
  if (current == NULL)
    {
      return -EINVAL;
    }

  next = *current;
  next.fill = WING_GUI_DEMO_BUTTON_CLICK_FILL;
  next.has_fill = true;
  wing_button_set_style(button, &next);
  wing_button_set_state_style(button, WING_OBJ_STATE_FOCUSED, &next);

  printf("wing_gui_demo: wing_button click event handled by WING widget and updated focused style\n");
  return 0;
}

static int wing_gui_demo_card_event(wing_obj_t *obj, wing_event_t *event)
{
  (void)obj;

  if (event == NULL || event->code != WING_EVENT_CLICK)
    {
      return 0;
    }

  printf("wing_gui_demo: card received bubbled click and stopped propagation\n");
  wing_event_stop_propagation(event);
  return 0;
}

static int wing_gui_demo_space_card_event(wing_obj_t *obj,
                                          wing_event_t *event)
{
  if (obj == NULL || event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_SPACE_TRANSFORM_CHANGED &&
      event->data != NULL)
    {
      const wing_space_transform_event_t *transform;

      transform = (const wing_space_transform_event_t *)event->data;
      printf("wing_gui_demo: space transform changed object=space-card old_ry=%d new_ry=%d old_z=%d new_z=%d\n",
             transform->old_transform.rotation_y,
             transform->transform.rotation_y,
             transform->old_transform.translation.z,
             transform->transform.translation.z);
      return 0;
    }

  if (event->code != WING_EVENT_CLICK)
    {
      return 0;
    }

  printf("wing_gui_demo: space card received click through projected quad hit-test\n");
  wing_event_stop_propagation(event);
  return 0;
}

static int wing_gui_demo_depth_back_card_event(wing_obj_t *obj,
                                               wing_event_t *event)
{
  (void)obj;

  if (event == NULL || event->code != WING_EVENT_CLICK)
    {
      return 0;
    }

  printf("wing_gui_demo: depth back card received click, depth ordering failed\n");
  wing_event_stop_propagation(event);
  return 0;
}

static int wing_gui_demo_depth_front_card_event(wing_obj_t *obj,
                                                wing_event_t *event)
{
  (void)obj;

  if (event == NULL || event->code != WING_EVENT_CLICK)
    {
      return 0;
    }

  printf("wing_gui_demo: depth front card received click through same z-index projected depth order\n");
  wing_event_stop_propagation(event);
  return 0;
}

static int wing_gui_demo_property_probe_event(wing_obj_t *obj,
                                             wing_event_t *event)
{
  const char *name;

  if (obj == NULL || event == NULL)
    {
      return 0;
    }

  name = (const char *)wing_obj_get_user_data(obj);
  if (name == NULL)
    {
      name = "property-probe";
    }

  if (event->code == WING_EVENT_BOUNDS_CHANGED &&
      event->data != NULL)
    {
      const wing_bounds_event_t *bounds;

      bounds = (const wing_bounds_event_t *)event->data;
      printf("wing_gui_demo: bounds changed object=%s old=%d,%d,%u,%u new=%d,%d,%u,%u\n",
             name,
             bounds->old_bounds.x,
             bounds->old_bounds.y,
             (unsigned int)bounds->old_bounds.w,
             (unsigned int)bounds->old_bounds.h,
             bounds->bounds.x,
             bounds->bounds.y,
             (unsigned int)bounds->bounds.w,
             (unsigned int)bounds->bounds.h);
      return 0;
    }

  return wing_gui_demo_state_probe_event(obj, event);
}

static int wing_gui_demo_root_event(wing_obj_t *obj, wing_event_t *event)
{
  (void)obj;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_CLICK)
    {
      printf("wing_gui_demo: root received click after bubbling\n");
      return 0;
    }

  if (event->code == WING_EVENT_CAMERA_CHANGED &&
      event->data != NULL)
    {
      const wing_camera_event_t *camera;
      uint8_t *camera_event_count;

      camera = (const wing_camera_event_t *)event->data;
      camera_event_count = (uint8_t *)wing_obj_get_user_data(obj);
      if (camera_event_count != NULL)
        {
          (*camera_event_count)++;
        }

      printf("wing_gui_demo: root observed runtime camera changed old_focal=%u new_focal=%u old_viewport=%ux%u new_viewport=%ux%u\n",
             (unsigned int)camera->old_camera.focal_length,
             (unsigned int)camera->camera.focal_length,
             (unsigned int)camera->old_camera.viewport.w,
             (unsigned int)camera->old_camera.viewport.h,
             (unsigned int)camera->camera.viewport.w,
             (unsigned int)camera->camera.viewport.h);
      return 0;
    }

  if (event->code == WING_EVENT_CLOSE_REQUEST)
    {
      printf("wing_gui_demo: root received close request through WING input/event queue\n");
      return 0;
    }

  return 0;
}

static int wing_gui_demo_state_probe_event(wing_obj_t *obj,
                                           wing_event_t *event)
{
  const char *name;

  if (obj == NULL || event == NULL ||
      event->code != WING_EVENT_STATE_CHANGED)
    {
      return 0;
    }

  name = (const char *)wing_obj_get_user_data(obj);
  if (name == NULL)
    {
      name = "state-probe";
    }

  if (event->data != NULL)
    {
      const wing_state_event_t *state;

      state = (const wing_state_event_t *)event->data;
      printf("wing_gui_demo: state changed object=%s old=0x%04x new=0x%04x changed=0x%04x selected=%s active=%s\n",
             name,
             (unsigned int)state->old_state,
             (unsigned int)state->state,
             (unsigned int)state->changed,
             wing_obj_is_selected(obj) ? "yes" : "no",
             wing_obj_is_active(obj) ? "yes" : "no");
    }

  return 0;
}

static int wing_gui_demo_toast_event(wing_obj_t *obj, wing_event_t *event)
{
  const char *name;

  if (obj == NULL || event == NULL)
    {
      return 0;
    }

  name = (const char *)wing_obj_get_user_data(obj);
  if (name == NULL)
    {
      name = "unnamed";
    }

  if (event->code == WING_EVENT_CREATE)
    {
      printf("wing_gui_demo: lifecycle create object=%s\n", name);
      return 0;
    }

  if (event->code == WING_EVENT_DELETE)
    {
      printf("wing_gui_demo: lifecycle delete object=%s\n", name);
      return 0;
    }

  return 0;
}

static int wing_gui_demo_slider_event(wing_slider_t *slider,
                                      wing_event_t *event, void *arg)
{
  (void)slider;
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      const wing_value_event_t *value;

      value = (const wing_value_event_t *)event->data;
      if (value != NULL)
        {
          printf("wing_gui_demo: wing_slider value changed from %u to %u by value input\n",
                 (unsigned int)value->old_value,
                 (unsigned int)value->value);
        }

      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_slider handled key step key=%u source=%s\n",
                 (unsigned int)input->key,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_slider handled encoder delta=%d source=%s\n",
                 input->encoder_delta,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      printf("wing_gui_demo: wing_slider focus gained and focused style is visible while focused\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CAPTURED)
    {
      printf("wing_gui_demo: wing_slider pointer captured by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_RELEASED)
    {
      printf("wing_gui_demo: wing_slider pointer released by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      printf("wing_gui_demo: wing_slider pointer capture cancelled by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_UP)
    {
      printf("wing_gui_demo: wing_slider pointer interaction completed\n");
    }

  return 0;
}

static int wing_gui_demo_progress_event(wing_progress_t *progress,
                                        wing_event_t *event, void *arg)
{
  (void)progress;
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      const wing_value_event_t *value;

      value = (const wing_value_event_t *)event->data;
      if (value != NULL)
        {
          printf("wing_gui_demo: wing_progress value changed from %u to %u by direct progress input\n",
                 (unsigned int)value->old_value,
                 (unsigned int)value->value);
        }

      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_progress handled key step key=%u source=%s\n",
                 (unsigned int)input->key,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_progress handled encoder delta=%d source=%s\n",
                 input->encoder_delta,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      printf("wing_gui_demo: wing_progress focus gained as an interactive value widget\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CAPTURED)
    {
      printf("wing_gui_demo: wing_progress pointer captured by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_RELEASED)
    {
      printf("wing_gui_demo: wing_progress pointer released by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      printf("wing_gui_demo: wing_progress pointer capture cancelled by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_UP)
    {
      printf("wing_gui_demo: wing_progress pointer drag interaction completed\n");
    }

  return 0;
}

static int wing_gui_demo_scrollbar_event(wing_scrollbar_t *scrollbar,
                                         wing_event_t *event, void *arg)
{
  (void)scrollbar;
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      const wing_value_event_t *value;

      value = (const wing_value_event_t *)event->data;
      if (value != NULL)
        {
          printf("wing_gui_demo: wing_scrollbar value changed from %u to %u by value input\n",
                 (unsigned int)value->old_value,
                 (unsigned int)value->value);
        }

      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_scrollbar handled key step key=%u source=%s\n",
                 (unsigned int)input->key,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      if (input != NULL)
        {
          printf("wing_gui_demo: wing_scrollbar handled encoder delta=%d source=%s\n",
                 input->encoder_delta,
                 wing_gui_demo_input_source_name(input->source));
        }

      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      printf("wing_gui_demo: wing_scrollbar focus gained and focused style is visible while focused\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CAPTURED)
    {
      printf("wing_gui_demo: wing_scrollbar pointer captured by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_RELEASED)
    {
      printf("wing_gui_demo: wing_scrollbar pointer released by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_CANCELLED)
    {
      printf("wing_gui_demo: wing_scrollbar pointer capture cancelled by WING runtime\n");
      return 0;
    }

  if (event->code == WING_EVENT_POINTER_UP)
    {
      printf("wing_gui_demo: wing_scrollbar pointer interaction completed\n");
    }

  return 0;
}

static int wing_gui_demo_scroll_view_event(wing_scroll_view_t *view,
                                           wing_event_t *event, void *arg)
{
  int16_t offset_x;
  int16_t offset_y;

  (void)arg;

  if (view == NULL || event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      printf("wing_gui_demo: wing_scroll_view focus gained as a normal focusable viewport object\n");
      return 0;
    }

  if (event->code == WING_EVENT_SCROLL_CHANGED)
    {
      const wing_scroll_event_t *scroll;

      scroll = (const wing_scroll_event_t *)event->data;
      if (scroll != NULL)
        {
          printf("wing_gui_demo: wing_scroll_view scroll changed old=%d,%d new=%d,%d max=%d,%d\n",
                 scroll->old_offset_x, scroll->old_offset_y,
                 scroll->offset_x, scroll->offset_y,
                 scroll->max_offset_x, scroll->max_offset_y);
        }

      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      wing_scroll_view_get_offset(view, &offset_x, &offset_y);
      printf("wing_gui_demo: wing_scroll_view handled key=%u source=%s offset=%d,%d\n",
             input == NULL ? 0 : (unsigned int)input->key,
             input == NULL ? "unknown" :
             wing_gui_demo_input_source_name(input->source),
             offset_x, offset_y);
      return 0;
    }

  if (event->code == WING_EVENT_ENCODER_ROTATE)
    {
      const wing_input_event_t *input;

      input = (const wing_input_event_t *)event->data;
      wing_scroll_view_get_offset(view, &offset_x, &offset_y);
      printf("wing_gui_demo: wing_scroll_view handled encoder delta=%d source=%s offset=%d,%d\n",
             input == NULL ? 0 : input->encoder_delta,
             input == NULL ? "unknown" :
             wing_gui_demo_input_source_name(input->source),
             offset_x, offset_y);
      return 0;
    }

  return 0;
}

static int wing_gui_demo_switch_event(wing_switch_t *sw,
                                      wing_event_t *event, void *arg)
{
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      const wing_value_event_t *value;

      value = (const wing_value_event_t *)event->data;
      if (value != NULL)
        {
          printf("wing_gui_demo: wing_switch value changed from %u to %u by toggle input object_checked=%s\n",
                 (unsigned int)value->old_value,
                 (unsigned int)value->value,
                 (sw != NULL &&
                  (wing_obj_get_state(wing_switch_obj(sw)) &
                   WING_OBJ_STATE_CHECKED) != 0) ? "yes" : "no");
        }

      return 0;
    }

  return 0;
}

static int wing_gui_demo_checkbox_event(wing_checkbox_t *checkbox,
                                        wing_event_t *event, void *arg)
{
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      const wing_value_event_t *value;

      value = (const wing_value_event_t *)event->data;
      if (value != NULL)
        {
          printf("wing_gui_demo: wing_checkbox value changed from %u to %u by toggle input object_checked=%s\n",
                 (unsigned int)value->old_value,
                 (unsigned int)value->value,
                 (checkbox != NULL &&
                  (wing_obj_get_state(wing_checkbox_obj(checkbox)) &
                   WING_OBJ_STATE_CHECKED) != 0) ? "yes" : "no");
        }

      return 0;
    }

  return 0;
}

static int wing_gui_demo_text_input_event(wing_text_input_t *input,
                                          wing_event_t *event, void *arg)
{
  (void)arg;

  if (event == NULL)
    {
      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_GAINED)
    {
      printf("wing_gui_demo: text input focus gained and cursor is visible\n");
      return 0;
    }

  if (event->code == WING_EVENT_FOCUS_LOST)
    {
      printf("wing_gui_demo: text input focus lost and cursor is hidden\n");
      return 0;
    }

  if (event->code == WING_EVENT_VALUE_CHANGED)
    {
      printf("wing_gui_demo: text input value changed text=%s cursor=%u\n",
             wing_text_input_get_text(input),
             (unsigned int)wing_text_input_get_cursor(input));
      return 0;
    }

  if (event->code == WING_EVENT_KEY_DOWN)
    {
      const wing_input_event_t *key;

      key = (const wing_input_event_t *)event->data;
      printf("wing_gui_demo: text input key down key=%u source=%s text=%s cursor=%u\n",
             key == NULL ? 0 : (unsigned int)key->key,
             key == NULL ? "unknown" :
             wing_gui_demo_input_source_name(key->source),
             wing_text_input_get_text(input),
             (unsigned int)wing_text_input_get_cursor(input));
      return 0;
    }

  return 0;
}

static void wing_gui_demo_progress_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_progress_timer_s *timer;

  (void)gui;

  timer = (struct wing_gui_demo_progress_timer_s *)arg;
  if (timer == NULL || timer->progress == NULL)
    {
      return;
    }

  wing_progress_set_value(timer->progress, timer->target_value);

  printf("wing_gui_demo: progress timer updated wing_progress value=%u\n",
         (unsigned int)timer->target_value);
}

static void wing_gui_demo_width_anim_apply(wing_gui_t *gui, int32_t value,
                                           void *arg)
{
  const wing_rect_t *current;
  struct wing_gui_demo_width_anim_s *anim;
  wing_rect_t next;

  (void)gui;

  anim = (struct wing_gui_demo_width_anim_s *)arg;
  if (anim == NULL || anim->box == NULL || value < 0)
    {
      return;
    }

  current = wing_obj_get_bounds(wing_box_obj(anim->box));
  if (current == NULL)
    {
      return;
    }

  next = *current;
  next.w = (uint16_t)value;
  (void)wing_obj_set_bounds(wing_box_obj(anim->box), &next);
}

static void wing_gui_demo_width_anim_done(wing_gui_t *gui, void *arg)
{
  (void)gui;
  (void)arg;

  printf("wing_gui_demo: width animation completed by WING runtime\n");
}

#if WING_GUI_DEMO_ENABLE_LOOP_ANIMATION
static void wing_gui_demo_loop_width_anim_apply(wing_gui_t *gui,
                                                int32_t value,
                                                void *arg)
{
  const wing_rect_t *current;
  struct wing_gui_demo_loop_width_anim_s *anim;
  wing_rect_t next;

  (void)gui;

  anim = (struct wing_gui_demo_loop_width_anim_s *)arg;
  if (anim == NULL || anim->box == NULL || value < 0)
    {
      return;
    }

  current = wing_obj_get_bounds(wing_box_obj(anim->box));
  if (current == NULL)
    {
      return;
    }

  next = *current;
  next.w = (uint16_t)value;
  (void)wing_obj_set_bounds(wing_box_obj(anim->box), &next);
}

static int wing_gui_demo_loop_width_anim_start(
  wing_gui_t *gui, struct wing_gui_demo_loop_width_anim_s *anim)
{
  int32_t start_value;
  int32_t end_value;

  if (gui == NULL || anim == NULL || anim->box == NULL)
    {
      return -EINVAL;
    }

  start_value = anim->forward ? anim->min_width : anim->max_width;
  end_value = anim->forward ? anim->max_width : anim->min_width;

  return wing_gui_anim_start_path(gui, start_value, end_value,
                                  WING_GUI_DEMO_FRAME_MS *
                                  WING_GUI_DEMO_ANIMATION_FRAMES,
                                  WING_ANIM_PATH_EASE_IN_OUT,
                                  wing_gui_demo_loop_width_anim_apply,
                                  wing_gui_demo_loop_width_anim_done,
                                  anim, NULL);
}

static void wing_gui_demo_loop_width_anim_done(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_loop_width_anim_s *anim;
  int ret;

  anim = (struct wing_gui_demo_loop_width_anim_s *)arg;
  if (gui == NULL || anim == NULL || anim->box == NULL)
    {
      return;
    }

  anim->segments_completed++;
  if (anim->segments_completed <= 4 ||
      (anim->segments_completed % WING_GUI_DEMO_PULSE_LOG_INTERVAL) == 0)
    {
      printf("wing_gui_demo: continuous pulse animation segment=%u completed direction=%s\n",
             (unsigned int)anim->segments_completed,
             anim->forward ? "forward" : "backward");
    }

#if !WING_GUI_DEMO_ENABLE_LOOP_ANIMATION
  printf("wing_gui_demo: continuous pulse animation disabled; not restarting\n");
  return;
#endif

  anim->forward = !anim->forward;
  ret = wing_gui_demo_loop_width_anim_start(gui, anim);
  if (ret < 0)
    {
      printf("wing_gui_demo: loop width animation restart failed: %d\n", ret);
    }
}
#endif

static void wing_gui_demo_space_anim_apply(wing_gui_t *gui, int32_t value,
                                        void *arg)
{
  struct wing_gui_demo_space_anim_s *anim;
  wing_space_transform_t transform;

  (void)gui;

  anim = (struct wing_gui_demo_space_anim_s *)arg;
  if (anim == NULL || anim->card == NULL)
    {
      return;
    }

  wing_space_transform_init(&transform);
  transform.rotation_y = (int16_t)value;
  transform.rotation_x = 8;
  transform.translation.z = 16;
  wing_obj_set_space_transform(wing_card_obj(anim->card), &transform);
}

static void wing_gui_demo_space_anim_done(wing_gui_t *gui, void *arg)
{
  (void)gui;
  (void)arg;

  printf("wing_gui_demo: space card animation completed through wing_obj_set_space_transform\n");
}

static void wing_gui_demo_apply_theme(wing_gui_t *gui,
                                      struct wing_gui_demo_theme_timer_s *timer)
{
  const wing_theme_t *theme;

  if (gui == NULL || timer == NULL || timer->theme == NULL)
    {
      return;
    }

  theme = timer->theme;
  wing_gui_set_theme(gui, theme);

  if (timer->root != NULL)
    {
      timer->root->style = theme->root;
      wing_obj_invalidate(wing_box_obj(timer->root));
    }

  if (timer->header != NULL)
    {
      timer->header->style = theme->header;
      wing_obj_invalidate(wing_box_obj(timer->header));
    }

  if (timer->card != NULL)
    {
      timer->card->box.style = theme->panel;
      wing_obj_invalidate(wing_panel_obj(timer->card));
    }

  if (timer->button != NULL)
    {
      timer->button->box.style = theme->button;
      wing_button_set_state_style(timer->button, WING_OBJ_STATE_FOCUSED,
                                  &theme->button_focused);
      wing_button_set_state_style(timer->button, WING_OBJ_STATE_PRESSED,
                                  &theme->button_pressed);
      wing_obj_invalidate(wing_button_obj(timer->button));
    }

  if (timer->label != NULL)
    {
      timer->label->color = theme->text;
      wing_obj_invalidate(wing_label_obj(timer->label));
    }

  if (timer->line_primary != NULL)
    {
      timer->line_primary->style = theme->line_primary;
      wing_obj_invalidate(wing_box_obj(timer->line_primary));
    }

  if (timer->line_secondary != NULL)
    {
      timer->line_secondary->style = theme->line_secondary;
      wing_obj_invalidate(wing_box_obj(timer->line_secondary));
    }

  if (timer->fill_panel != NULL)
    {
      timer->fill_panel->style = theme->panel;
      wing_obj_invalidate(wing_box_obj(timer->fill_panel));
    }

  if (timer->fill_badge != NULL)
    {
      timer->fill_badge->style = theme->line_primary;
      wing_obj_invalidate(wing_box_obj(timer->fill_badge));
    }

  if (timer->space_card != NULL)
    {
      wing_card_set_front_style(timer->space_card, &theme->button_focused);
      wing_card_set_back_style(timer->space_card, &theme->button_pressed);
      wing_card_set_edge_style(timer->space_card, &theme->line_primary);
    }

  if (timer->progress != NULL)
    {
      timer->progress->frame_style = theme->progress_frame;
      timer->progress->fill_style = theme->progress_fill;
      wing_obj_invalidate(wing_progress_obj(timer->progress));
    }

  if (timer->slider != NULL)
    {
      timer->slider->track_style = theme->slider_track;
      timer->slider->fill_style = theme->slider_fill;
      timer->slider->knob_style = theme->slider_knob;
      wing_slider_set_state_style(timer->slider, WING_OBJ_STATE_FOCUSED,
                                  &theme->slider_focused);
      wing_obj_invalidate(wing_slider_obj(timer->slider));
    }

  if (timer->scrollbar != NULL)
    {
      timer->scrollbar->track_style = theme->scrollbar_track;
      timer->scrollbar->thumb_style = theme->scrollbar_thumb;
      wing_scrollbar_set_state_style(timer->scrollbar,
                                     WING_OBJ_STATE_FOCUSED,
                                     &theme->scrollbar_focused);
      wing_obj_invalidate(wing_scrollbar_obj(timer->scrollbar));
    }

  if (timer->power_switch != NULL)
    {
      timer->power_switch->off_style = theme->switch_off;
      timer->power_switch->on_style = theme->switch_on;
      timer->power_switch->knob_style = theme->switch_knob;
      wing_obj_invalidate(wing_switch_obj(timer->power_switch));
    }

  if (timer->checkbox != NULL)
    {
      timer->checkbox->box_style = theme->checkbox_box;
      timer->checkbox->checked_style = theme->checkbox_checked;
      timer->checkbox->mark_style = theme->checkbox_mark;
      wing_obj_invalidate(wing_checkbox_obj(timer->checkbox));
    }
}

static void wing_gui_demo_theme_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_theme_timer_s *timer;

  timer = (struct wing_gui_demo_theme_timer_s *)arg;
  wing_gui_demo_apply_theme(gui, timer);

  printf("wing_gui_demo: theme timer switched active theme to high_contrast and invalidated widget tree\n");
}

static void wing_gui_demo_disable_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_disable_timer_s *timer;
  const wing_box_style_t *current;
  wing_obj_t *obj;

  (void)gui;

  timer = (struct wing_gui_demo_disable_timer_s *)arg;
  if (timer == NULL || timer->button == NULL)
    {
      return;
    }

  obj = wing_button_obj(timer->button);
  wing_obj_set_enabled(obj, false);

  current = wing_box_get_active_style(wing_button_box(timer->button));
  if (current != NULL)
    {
      printf("wing_gui_demo: disabled timer cleared button enabled flag and active disabled style fill=%u,%u,%u\n",
             current->fill.r, current->fill.g, current->fill.b);
    }
  else
    {
      printf("wing_gui_demo: disabled timer cleared button enabled flag\n");
    }
}

static void wing_gui_demo_repeat_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_repeat_timer_s *timer;
  const wing_box_style_t *style;
  int ret;

  timer = (struct wing_gui_demo_repeat_timer_s *)arg;
  if (timer == NULL || timer->line == NULL)
    {
      return;
    }

  timer->tick_count++;
  style = (timer->tick_count & 1) != 0 ?
          &timer->first_style : &timer->second_style;
  timer->line->style = *style;
  wing_obj_invalidate(wing_box_obj(timer->line));

  printf("wing_gui_demo: repeat timer tick=%u updated line_secondary style\n",
         (unsigned int)timer->tick_count);

  if (timer->tick_count < WING_GUI_DEMO_REPEAT_STOP_TICKS)
    {
      return;
    }

  ret = wing_gui_timer_stop(gui, timer->timer_id);
  if (ret < 0)
    {
      printf("wing_gui_demo: repeat timer stop failed ret=%d\n", ret);
      return;
    }

  printf("wing_gui_demo: repeat timer stopped itself after tick=%u\n",
         (unsigned int)timer->tick_count);
}

static void wing_gui_demo_toast_create_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_toast_s *toast;
  int ret;

  (void)gui;

  toast = (struct wing_gui_demo_toast_s *)arg;
  if (toast == NULL || toast->root == NULL || toast->active)
    {
      return;
    }

  ret = wing_obj_add_child(toast->root, wing_box_obj(&toast->toast));
  if (ret < 0)
    {
      printf("wing_gui_demo: lifecycle toast create failed ret=%d\n", ret);
      return;
    }

  toast->active = true;
  printf("wing_gui_demo: lifecycle timer attached toast subtree to root\n");
}

static void wing_gui_demo_toast_destroy_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_toast_s *toast;
  int ret;

  (void)gui;

  toast = (struct wing_gui_demo_toast_s *)arg;
  if (toast == NULL || !toast->active)
    {
      return;
    }

  ret = wing_obj_destroy_tree(wing_box_obj(&toast->toast));
  if (ret < 0)
    {
      printf("wing_gui_demo: lifecycle toast destroy failed ret=%d\n", ret);
      return;
    }

  toast->active = false;
  printf("wing_gui_demo: lifecycle timer destroyed toast subtree\n");
}

static void wing_gui_demo_toast_hide_timer(wing_gui_t *gui, void *arg)
{
  struct wing_gui_demo_toast_s *toast;

  (void)gui;

  toast = (struct wing_gui_demo_toast_s *)arg;
  if (toast == NULL || !toast->active)
    {
      return;
    }

  wing_obj_set_visible(wing_box_obj(&toast->toast), false);
  printf("wing_gui_demo: visibility timer hid toast subtree visible=%u\n",
         wing_obj_is_visible(wing_box_obj(&toast->toast)) ? 1 : 0);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  fr_backend_caps_t fb_caps;
  fr_command_t commands[WING_GUI_DEMO_COMMANDS];
  fr_command_list_t list;
  fr_fb_presenter_t presenter;
  wing_box_style_t button_style;
  wing_box_style_t button_focused_style;
  wing_box_style_t button_pressed_style;
  wing_box_style_t button_disabled_style;
  wing_box_style_t button_hovered_style;
  wing_box_style_t card_style;
  wing_box_style_t checkbox_box_style;
  wing_box_style_t checkbox_checked_style;
  wing_box_style_t checkbox_mark_style;
  wing_box_style_t clip_child_style;
  wing_box_style_t clip_panel_style;
  wing_box_style_t header_style;
  wing_box_style_t fill_badge_style;
  wing_box_style_t fill_panel_active_style;
  wing_box_style_t fill_panel_style;
  wing_space_transform_t fill_panel_space;
  wing_space_transform_t image_space;
  wing_space_transform_t label_space;
  wing_box_style_t depth_back_front_style;
  wing_box_style_t depth_front_front_style;
  wing_box_style_t depth_card_back_style;
  wing_box_style_t depth_card_edge_style;
  wing_box_style_t space_card_back_style;
  wing_box_style_t space_card_edge_style;
  wing_box_style_t space_card_front_style;
  wing_box_style_t line_primary_style;
  wing_box_style_t line_primary_selected_style;
  wing_box_style_t line_secondary_style;
  wing_box_style_t progress_fill_style;
  wing_box_style_t progress_frame_style;
  wing_box_style_t root_style;
  wing_box_style_t scrollbar_focused_style;
  wing_box_style_t scrollbar_thumb_style;
  wing_box_style_t scrollbar_track_style;
  wing_box_style_t scroll_accent_style;
  wing_box_style_t scroll_content_style;
  wing_box_style_t scroll_view_style;
  wing_box_style_t slider_fill_style;
  wing_box_style_t slider_focused_style;
  wing_box_style_t slider_knob_style;
  wing_box_style_t slider_track_style;
  wing_box_style_t switch_knob_style;
  wing_box_style_t switch_off_style;
  wing_box_style_t switch_on_style;
  wing_box_style_t text_cursor_style;
  wing_box_style_t text_selection_style;
  wing_box_style_t text_input_style;
  wing_box_t fill_badge;
  wing_box_t fill_panel;
  wing_box_t header;
  wing_box_t clip_child;
  wing_box_t clip_panel;
  wing_box_t line_primary;
  wing_box_t line_secondary;
  wing_box_t root;
  wing_box_t scroll_accent;
  wing_box_t scroll_content;
  wing_card_t depth_back_card;
  wing_card_t depth_front_card;
  wing_card_t space_card;
  wing_button_t button;
  wing_checkbox_t checkbox;
  wing_image_t demo_image;
  wing_label_t button_label;
  wing_label_t ellipsis_label;
  wing_label_t wrap_label;
  wing_panel_t card;
  wing_progress_t progress;
  wing_scrollbar_t scrollbar;
  wing_scroll_view_t scroll_view;
  wing_slider_t slider;
  wing_switch_t power_switch;
  wing_text_input_t text_input;
  wing_obj_t triangle_obj;
  wing_gui_t gui;
  wing_gui_frame_t frame;
  wing_theme_t theme;
  wing_theme_t contrast_theme;
  struct wing_gui_demo_input_provider_s input_provider;
  struct wing_gui_demo_disable_timer_s disable_timer;
  struct wing_gui_demo_progress_timer_s progress_timer;
  struct wing_gui_demo_repeat_timer_s repeat_timer;
  struct wing_gui_demo_theme_timer_s theme_timer;
  struct wing_gui_demo_toast_s toast;
  struct wing_gui_demo_triangle_s triangle_demo;
  struct wing_gui_demo_space_anim_s space_anim;
  struct wing_gui_demo_width_anim_s line_anim;
  struct wing_gui_demo_loop_width_anim_s loop_line_anim;
  wing_camera_t demo_camera;
  const wing_camera_t *gui_camera;
  uint8_t camera_event_count;
  uint8_t camera_events_before_noop;
  wing_quad2d_t card_quad;
  wing_projected_quad_t card_projected_quad;
  wing_projected_triangle_t triangle_projected;
  wing_rect_t card_screen_bounds;
  wing_rect_t clip_child_bounds;
  wing_rect_t clip_panel_bounds;
  wing_rect_t scroll_content_bounds;
  wing_rect_t scroll_view_bounds;
  wing_rect_t triangle_screen_bounds;
  wing_space_transform_t card_world_transform;
  wing_rect_t rect;
  wing_rect_t root_screen_bounds;
  wing_point_t space_card_center_screen;
  wing_point_t depth_card_hit_point;
  wing_point_t space_card_hit_point;
  wing_vec3_t space_card_center;
  const wing_font_t *button_label_font;
  const char *utf8_sample;
  const char *utf8_cursor;
  const char *multiline_sample;
  char text_input_buffer[WING_GUI_DEMO_TEXT_INPUT_BUFFER];
  uint32_t utf8_codepoint;
  int32_t depth_back_average_depth;
  int32_t depth_front_average_depth;
  int32_t space_card_average_depth;
  int32_t space_card_depth;
  int depth_front_back_order;
  int space_card_order;
  wing_surface_t surface;
  wing_space_transform_t space_transform;
  uint32_t *pixels;
  uint32_t checksum;
  size_t pixel_count;
  wing_input_event_t click;
  bool fill_layout_reported;
  bool have_presenter;
  bool idle_reported;
  uint8_t headless_frames;
  int ret;
  int presenter_open_ret;
  uint16_t demo_height;
  uint16_t demo_width;
  uint16_t range_max;
  uint16_t range_min;
  uint16_t button_label_text_h;
  uint16_t button_label_text_w;
  uint16_t text_selection_end;
  uint16_t text_selection_start;
  uint16_t utf8_text_h;
  uint16_t utf8_text_w;
  uint16_t multiline_text_h;
  uint16_t multiline_text_w;
  uint16_t ellipsis_layout_h;
  uint16_t ellipsis_layout_w;
  uint16_t wrap_layout_h;
  uint16_t wrap_layout_w;
  int16_t scroll_offset_x;
  int16_t scroll_offset_y;
  int16_t scroll_max_offset_x;
  int16_t scroll_max_offset_y;
  int16_t depth_card_x;
  int16_t depth_card_y;
  int16_t space_card_x;
  int16_t space_card_y;
  uint8_t utf8_codepoints;

  have_presenter = false;
  presenter_open_ret = fr_fb_presenter_open(&presenter, "/dev/fb0");
  demo_width = CONFIG_EXAMPLES_WING_GUI_DEMO_WIDTH;
  demo_height = CONFIG_EXAMPLES_WING_GUI_DEMO_HEIGHT;
  if (presenter_open_ret == 0)
    {
      have_presenter = true;
      demo_width = presenter.xres;
      demo_height = presenter.yres;
    }

  /* Keep projected space cards near the runtime camera center where the
   * first-stage projection seed stays stable. Draggable value controls use a
   * higher local z layer so decorative space cards cannot cover them.
   */

  space_card_x = (int16_t)(demo_width / 2 -
                           WING_GUI_DEMO_SPACE_CARD_W / 2);
  space_card_y = (int16_t)(demo_height / 2 -
                           WING_GUI_DEMO_SPACE_CARD_H / 2);
  depth_card_x = (int16_t)(space_card_x -
                           WING_GUI_DEMO_DEPTH_CARD_W - 16);
  depth_card_y = (int16_t)(space_card_y + 3);
  if (depth_card_x < WING_GUI_DEMO_MARGIN_X)
    {
      depth_card_x = WING_GUI_DEMO_MARGIN_X;
    }

  pixel_count = (size_t)demo_width * demo_height;
  camera_event_count = 0;

  pixels = (uint32_t *)malloc(pixel_count * sizeof(uint32_t));
  if (pixels == NULL)
    {
      printf("wing_gui_demo: failed to allocate %u pixels\n",
             (unsigned int)pixel_count);
      return EXIT_FAILURE;
    }

  ret = wing_surface_init(&surface, pixels,
                          demo_width,
                          demo_height,
                          demo_width,
                          WING_PIXEL_FORMAT_RGBA8888);
  if (ret < 0)
    {
      printf("wing_gui_demo: surface init failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  fr_command_list_init(&list, commands, WING_GUI_DEMO_COMMANDS);

  ret = wing_gui_create(&gui, &surface, &list, NULL, NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: gui create failed: %d\n", ret);
      free(pixels);
      return EXIT_FAILURE;
    }

  wing_gui_set_frame_interval(&gui, WING_GUI_DEMO_FRAME_MS);
  wing_theme_init_default(&theme);
  wing_theme_init_high_contrast(&contrast_theme);
  wing_gui_set_theme(&gui, &theme);
  wing_gui_demo_print_stage("setup runtime, surface and command list");
  printf("wing_gui_demo: surface=%ux%u command_capacity=%u frame_interval=%ums\n",
         (unsigned int)demo_width,
         (unsigned int)demo_height,
         (unsigned int)WING_GUI_DEMO_COMMANDS,
         (unsigned int)WING_GUI_DEMO_FRAME_MS);
  printf("wing_gui_demo: default theme initialized for root/header/panel/widgets/state styles\n");

  rect.x = 0;
  rect.y = 0;
  rect.w = demo_width;
  rect.h = demo_height;
  root_style = theme.root;
  wing_box_init(&root, &rect, &root_style);
  wing_obj_set_user_data(wing_box_obj(&root), &camera_event_count);
  wing_obj_set_event_cb(wing_box_obj(&root), wing_gui_demo_root_event);

  rect.x = 0;
  rect.y = 0;
  rect.w = demo_width;
  rect.h = WING_GUI_DEMO_HEADER_HEIGHT;
  header_style = theme.header;
  wing_box_init(&header, &rect, &header_style);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_box_obj(&header));

  rect.x = WING_GUI_DEMO_CHECKBOX_X;
  rect.y = WING_GUI_DEMO_CHECKBOX_Y;
  rect.w = WING_GUI_DEMO_CHECKBOX_SIZE;
  rect.h = WING_GUI_DEMO_CHECKBOX_SIZE;
  checkbox_box_style = theme.checkbox_box;
  checkbox_checked_style = theme.checkbox_checked;
  checkbox_mark_style = theme.checkbox_mark;
  wing_checkbox_init(&checkbox, &rect, &checkbox_box_style,
                     &checkbox_checked_style, &checkbox_mark_style, false);
  wing_checkbox_set_padding(&checkbox, WING_GUI_DEMO_CHECKBOX_PADDING);
  wing_checkbox_set_event_cb(&checkbox, wing_gui_demo_checkbox_event, NULL);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_checkbox_obj(&checkbox));

  rect.x = (int16_t)(demo_width -
                     WING_GUI_DEMO_SWITCH_WIDTH -
                     WING_GUI_DEMO_SWITCH_RIGHT_MARGIN);
  rect.y = WING_GUI_DEMO_SWITCH_Y;
  rect.w = WING_GUI_DEMO_SWITCH_WIDTH;
  rect.h = WING_GUI_DEMO_SWITCH_HEIGHT;
  switch_off_style = theme.switch_off;
  switch_on_style = theme.switch_on;
  switch_knob_style = theme.switch_knob;
  wing_switch_init(&power_switch, &rect, &switch_off_style,
                   &switch_on_style, &switch_knob_style, false);
  wing_switch_set_padding(&power_switch, WING_GUI_DEMO_SWITCH_PADDING);
  wing_switch_set_event_cb(&power_switch, wing_gui_demo_switch_event, NULL);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_switch_obj(&power_switch));

  text_input_buffer[0] = 'G';
  text_input_buffer[1] = 'O';
  text_input_buffer[2] = '\0';
  rect.x = WING_GUI_DEMO_TEXT_INPUT_X;
  rect.y = WING_GUI_DEMO_TEXT_INPUT_Y;
  rect.w = WING_GUI_DEMO_TEXT_INPUT_W;
  rect.h = WING_GUI_DEMO_TEXT_INPUT_H;
  text_input_style = theme.panel;
  text_input_style.stroke = theme.line_primary.fill;
  text_input_style.has_stroke = true;
  wing_box_style_init(&text_cursor_style);
  text_cursor_style.fill = theme.text;
  text_cursor_style.has_fill = true;
  wing_box_style_init(&text_selection_style);
  text_selection_style.fill = WING_GUI_DEMO_SELECTED_FILL;
  text_selection_style.fill.a = 112;
  text_selection_style.has_fill = true;
  wing_text_input_init(&text_input, &rect, &text_input_style,
                       &text_cursor_style, text_input_buffer,
                       WING_GUI_DEMO_TEXT_INPUT_BUFFER);
  wing_text_input_set_padding(&text_input, WING_GUI_DEMO_TEXT_INPUT_PADDING);
  wing_text_input_set_selection_style(&text_input, &text_selection_style);
  wing_text_input_set_event_cb(&text_input, wing_gui_demo_text_input_event,
                               NULL);
  wing_text_input_select_all(&text_input);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_text_input_obj(&text_input));

  rect.x = WING_GUI_DEMO_CARD_X;
  rect.y = WING_GUI_DEMO_CARD_Y;
  rect.w = (uint16_t)(demo_width -
                      WING_GUI_DEMO_MARGIN_X * 2);
  rect.h = WING_GUI_DEMO_CARD_H;
  card_style = theme.panel;
  wing_panel_init(&card, &rect, &card_style);
  wing_panel_set_layout(&card, WING_LAYOUT_STACK_HORIZONTAL,
                        WING_GUI_DEMO_CARD_PAD_X,
                        WING_GUI_DEMO_CARD_PAD_Y);
  wing_obj_set_event_cb(wing_panel_obj(&card), wing_gui_demo_card_event);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_panel_obj(&card));

  rect.x = 0;
  rect.y = 0;
  rect.w = WING_GUI_DEMO_BUTTON_W;
  rect.h = WING_GUI_DEMO_BUTTON_H;
  button_style = theme.button;
  wing_button_init(&button, &rect, &button_style);
  button_focused_style = theme.button_focused;
  wing_button_set_state_style(&button, WING_OBJ_STATE_FOCUSED,
                              &button_focused_style);
  button_pressed_style = theme.button_pressed;
  wing_button_set_state_style(&button, WING_OBJ_STATE_PRESSED,
                              &button_pressed_style);
  button_hovered_style = theme.button_focused;
  button_hovered_style.fill = WING_GUI_DEMO_BUTTON_HOVER_FILL;
  wing_button_set_state_style(&button, WING_OBJ_STATE_HOVERED,
                              &button_hovered_style);
  button_disabled_style = theme.button;
  button_disabled_style.fill = WING_GUI_DEMO_BUTTON_DISABLED_FILL;
  button_disabled_style.stroke = WING_GUI_DEMO_BUTTON_DISABLED_STROKE;
  wing_button_set_state_style(&button, WING_OBJ_STATE_DISABLED,
                              &button_disabled_style);
  wing_button_set_event_cb(&button, wing_gui_demo_button_event, NULL);
  wing_obj_set_layout(wing_button_obj(&button), WING_LAYOUT_CENTER, 0, 0);
  (void)wing_obj_add_child(wing_panel_obj(&card), wing_button_obj(&button));

  rect.x = 0;
  rect.y = 0;
  rect.w = WING_GUI_DEMO_LABEL_W;
  rect.h = WING_GUI_DEMO_LABEL_H;
  wing_label_init(&button_label, &rect, "GO", theme.text,
                  WING_GUI_DEMO_LABEL_SCALE);
  wing_label_set_font(&button_label, wing_font_builtin_5x7());
  wing_label_set_align(&button_label, WING_TEXT_ALIGN_CENTER);
  wing_obj_set_enabled(wing_label_obj(&button_label), false);
  (void)wing_obj_add_child(wing_button_obj(&button),
                           wing_label_obj(&button_label));

  rect.x = 154;
  rect.y = 5;
  rect.w = 42;
  rect.h = 14;
  wing_label_init(&ellipsis_label, &rect, "LONG LABEL", theme.text,
                  WING_GUI_DEMO_LABEL_SCALE);
  wing_label_set_text_mode(&ellipsis_label, WING_LABEL_TEXT_MODE_ELLIPSIS);
  wing_obj_set_enabled(wing_label_obj(&ellipsis_label), false);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_label_obj(&ellipsis_label));

  rect.x = 204;
  rect.y = 5;
  rect.w = 44;
  rect.h = 28;
  wing_label_init(&wrap_label, &rect, "DONE DONE", theme.text,
                  WING_GUI_DEMO_LABEL_SCALE);
  wing_label_set_text_mode(&wrap_label, WING_LABEL_TEXT_MODE_WRAP);
  wing_space_transform_init(&label_space);
  label_space.rotation_y = WING_GUI_DEMO_LABEL_SPACE_ROTATION_Y;
  label_space.translation.z = WING_GUI_DEMO_LABEL_SPACE_Z;
  wing_obj_set_space_transform(wing_label_obj(&wrap_label), &label_space);
  wing_obj_set_enabled(wing_label_obj(&wrap_label), false);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_label_obj(&wrap_label));

  rect.x = 0;
  rect.y = 0;
  rect.w = WING_GUI_DEMO_LINE_PRIMARY_W;
  rect.h = WING_GUI_DEMO_LINE_PRIMARY_H;
  line_primary_style = theme.line_primary;
  wing_box_init(&line_primary, &rect, &line_primary_style);
  wing_obj_set_user_data(wing_box_obj(&line_primary), "line-primary");
  wing_obj_set_event_cb(wing_box_obj(&line_primary),
                        wing_gui_demo_property_probe_event);
  line_primary_selected_style = theme.line_primary;
  line_primary_selected_style.fill = WING_GUI_DEMO_SELECTED_FILL;
  wing_box_set_state_style(&line_primary, WING_OBJ_STATE_SELECTED,
                           &line_primary_selected_style);
  wing_obj_set_selected(wing_box_obj(&line_primary), true);
  (void)wing_obj_add_child(wing_panel_obj(&card),
                           wing_box_obj(&line_primary));

  rect.x = WING_GUI_DEMO_LINE_SECONDARY_X;
  rect.y = WING_GUI_DEMO_LINE_SECONDARY_Y;
  rect.w = WING_GUI_DEMO_LINE_SECONDARY_W;
  rect.h = WING_GUI_DEMO_LINE_SECONDARY_H;
  line_secondary_style = theme.line_secondary;
  wing_box_init(&line_secondary, &rect, &line_secondary_style);
  (void)wing_obj_add_child(wing_panel_obj(&card),
                           wing_box_obj(&line_secondary));

  rect.x = WING_GUI_DEMO_FILL_PANEL_X;
  rect.y = WING_GUI_DEMO_FILL_PANEL_Y;
  rect.w = WING_GUI_DEMO_FILL_PANEL_W;
  rect.h = WING_GUI_DEMO_FILL_PANEL_H;
  fill_panel_style = theme.panel;
  wing_box_init(&fill_panel, &rect, &fill_panel_style);
  wing_obj_set_user_data(wing_box_obj(&fill_panel), "fill-panel");
  wing_obj_set_event_cb(wing_box_obj(&fill_panel),
                        wing_gui_demo_state_probe_event);
  fill_panel_active_style = theme.panel;
  fill_panel_active_style.fill = WING_GUI_DEMO_ACTIVE_FILL;
  wing_box_set_state_style(&fill_panel, WING_OBJ_STATE_ACTIVE,
                           &fill_panel_active_style);
  wing_obj_set_active(wing_box_obj(&fill_panel), true);
  wing_obj_set_opacity(wing_box_obj(&fill_panel),
                       WING_GUI_DEMO_FILL_PANEL_OPACITY);
  wing_space_transform_init(&fill_panel_space);
  fill_panel_space.rotation_y = WING_GUI_DEMO_FILL_PANEL_ROTATION_Y;
  fill_panel_space.translation.z = WING_GUI_DEMO_FILL_PANEL_Z;
  wing_obj_set_space_transform(wing_box_obj(&fill_panel),
                               &fill_panel_space);
  wing_obj_set_layout(wing_box_obj(&fill_panel), WING_LAYOUT_FILL,
                      WING_GUI_DEMO_FILL_PANEL_PAD_X, 0);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_box_obj(&fill_panel));

  rect.x = 0;
  rect.y = 0;
  rect.w = WING_GUI_DEMO_FILL_BADGE_SIZE;
  rect.h = WING_GUI_DEMO_FILL_BADGE_SIZE;
  fill_badge_style = theme.line_primary;
  wing_box_init(&fill_badge, &rect, &fill_badge_style);
  (void)wing_obj_add_child(wing_box_obj(&fill_panel),
                           wing_box_obj(&fill_badge));

  rect.x = WING_GUI_DEMO_CLIP_PANEL_X;
  rect.y = WING_GUI_DEMO_CLIP_PANEL_Y;
  rect.w = WING_GUI_DEMO_CLIP_PANEL_W;
  rect.h = WING_GUI_DEMO_CLIP_PANEL_H;
  clip_panel_style = theme.panel;
  clip_panel_style.stroke = theme.line_primary.fill;
  clip_panel_style.has_stroke = true;
  wing_box_init(&clip_panel, &rect, &clip_panel_style);
  wing_obj_set_clip_children(wing_box_obj(&clip_panel), true);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_box_obj(&clip_panel));

  rect.x = WING_GUI_DEMO_CLIP_CHILD_X;
  rect.y = WING_GUI_DEMO_CLIP_CHILD_Y;
  rect.w = WING_GUI_DEMO_CLIP_CHILD_W;
  rect.h = WING_GUI_DEMO_CLIP_CHILD_H;
  clip_child_style = theme.button_pressed;
  wing_box_init(&clip_child, &rect, &clip_child_style);
  (void)wing_obj_add_child(wing_box_obj(&clip_panel),
                           wing_box_obj(&clip_child));

  rect.x = WING_GUI_DEMO_SCROLL_VIEW_X;
  rect.y = WING_GUI_DEMO_SCROLL_VIEW_Y;
  rect.w = WING_GUI_DEMO_SCROLL_VIEW_W;
  rect.h = WING_GUI_DEMO_SCROLL_VIEW_H;
  scroll_view_style = theme.panel;
  scroll_view_style.stroke = theme.line_primary.fill;
  scroll_view_style.has_stroke = true;
  wing_scroll_view_init(&scroll_view, &rect, &scroll_view_style);
  wing_scroll_view_set_content_size(&scroll_view,
                                    WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_W,
                                    WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_H);
  wing_scroll_view_set_step(&scroll_view, WING_GUI_DEMO_SCROLL_STEP_X,
                            WING_GUI_DEMO_SCROLL_STEP_Y);
  wing_scroll_view_set_event_cb(&scroll_view,
                                wing_gui_demo_scroll_view_event, NULL);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_scroll_view_obj(&scroll_view));

  rect.x = WING_GUI_DEMO_SCROLL_CONTENT_X;
  rect.y = WING_GUI_DEMO_SCROLL_CONTENT_Y;
  rect.w = WING_GUI_DEMO_SCROLL_CONTENT_W;
  rect.h = WING_GUI_DEMO_SCROLL_CONTENT_H;
  scroll_content_style = theme.line_secondary;
  wing_box_init(&scroll_content, &rect, &scroll_content_style);
  (void)wing_obj_add_child(wing_scroll_view_obj(&scroll_view),
                           wing_box_obj(&scroll_content));

  rect.x = WING_GUI_DEMO_SCROLL_ACCENT_X;
  rect.y = WING_GUI_DEMO_SCROLL_ACCENT_Y;
  rect.w = WING_GUI_DEMO_SCROLL_ACCENT_W;
  rect.h = WING_GUI_DEMO_SCROLL_ACCENT_H;
  scroll_accent_style = theme.line_primary;
  wing_box_init(&scroll_accent, &rect, &scroll_accent_style);
  (void)wing_obj_add_child(wing_scroll_view_obj(&scroll_view),
                           wing_box_obj(&scroll_accent));
  wing_scroll_view_set_offset(&scroll_view,
                              WING_GUI_DEMO_SCROLL_OFFSET_X,
                              WING_GUI_DEMO_SCROLL_OFFSET_Y);
  wing_scroll_view_scroll_by(&scroll_view, 1000, 0);
  wing_scroll_view_get_max_offset(&scroll_view, &scroll_max_offset_x,
                                  &scroll_max_offset_y);
  wing_scroll_view_get_offset(&scroll_view, &scroll_offset_x,
                              &scroll_offset_y);
  printf("wing_gui_demo: scroll view scroll_by clamps offset to %d,%d max=%d,%d\n",
         scroll_offset_x, scroll_offset_y,
         scroll_max_offset_x, scroll_max_offset_y);
  wing_scroll_view_set_offset(&scroll_view,
                              WING_GUI_DEMO_SCROLL_OFFSET_X,
                              WING_GUI_DEMO_SCROLL_OFFSET_Y);

  rect.x = WING_GUI_DEMO_IMAGE_X;
  rect.y = WING_GUI_DEMO_IMAGE_Y;
  rect.w = WING_GUI_DEMO_IMAGE_W;
  rect.h = WING_GUI_DEMO_IMAGE_H;
  wing_image_init_resource(&demo_image, &rect,
                           &g_wing_gui_demo_image_resource,
                           WING_GUI_DEMO_IMAGE_SCALE);
  wing_obj_set_opacity(wing_image_obj(&demo_image),
                       WING_GUI_DEMO_IMAGE_OPACITY);
  wing_space_transform_init(&image_space);
  image_space.rotation_y = WING_GUI_DEMO_IMAGE_ROTATION_Y;
  image_space.translation.z = WING_GUI_DEMO_IMAGE_Z;
  wing_obj_set_space_transform(wing_image_obj(&demo_image), &image_space);
  wing_obj_set_enabled(wing_image_obj(&demo_image), false);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_image_obj(&demo_image));

  rect.x = space_card_x;
  rect.y = space_card_y;
  rect.w = WING_GUI_DEMO_SPACE_CARD_W;
  rect.h = WING_GUI_DEMO_SPACE_CARD_H;
  space_card_front_style = theme.button_focused;
  space_card_back_style = theme.button_pressed;
  space_card_edge_style = theme.line_primary;
  space_card_front_style.opacity = WING_GUI_DEMO_SPACE_CARD_OPACITY;
  space_card_back_style.opacity = WING_GUI_DEMO_SPACE_CARD_OPACITY;
  space_card_edge_style.opacity = WING_GUI_DEMO_SPACE_CARD_EDGE_OPACITY;
  wing_card_init(&space_card, &rect, &space_card_front_style,
                 &space_card_back_style, &space_card_edge_style);
  wing_obj_set_event_cb(wing_card_obj(&space_card),
                        wing_gui_demo_space_card_event);
  wing_space_transform_init(&space_transform);
  space_transform.rotation_y = WING_GUI_DEMO_SPACE_CARD_ROTATION_START;
  space_transform.rotation_x = 8;
  space_transform.translation.z = 16;
  wing_obj_set_space_transform(wing_card_obj(&space_card),
                               &space_transform);
  wing_obj_set_z_index(wing_card_obj(&space_card),
                       WING_GUI_DEMO_SPACE_CARD_Z);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_card_obj(&space_card));

  rect.x = depth_card_x;
  rect.y = depth_card_y;
  rect.w = WING_GUI_DEMO_DEPTH_CARD_W;
  rect.h = WING_GUI_DEMO_DEPTH_CARD_H;
  depth_front_front_style = theme.button_focused;
  depth_front_front_style.fill = wing_color_rgba(255, 154, 86, 255);
  depth_front_front_style.opacity = 208;
  depth_card_back_style = theme.button_pressed;
  depth_card_back_style.opacity = 208;
  depth_card_edge_style = theme.line_primary;
  depth_card_edge_style.opacity = 240;
  wing_card_init(&depth_front_card, &rect, &depth_front_front_style,
                 &depth_card_back_style, &depth_card_edge_style);
  wing_obj_set_event_cb(wing_card_obj(&depth_front_card),
                        wing_gui_demo_depth_front_card_event);
  wing_space_transform_init(&space_transform);
  space_transform.rotation_y = 12;
  space_transform.translation.z = WING_GUI_DEMO_DEPTH_FRONT_Z;
  wing_obj_set_space_transform(wing_card_obj(&depth_front_card),
                               &space_transform);
  wing_obj_set_z_index(wing_card_obj(&depth_front_card),
                       WING_GUI_DEMO_DEPTH_CARD_Z_INDEX);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_card_obj(&depth_front_card));

  rect.x = (int16_t)(depth_card_x + 4);
  rect.y = (int16_t)(depth_card_y + 2);
  rect.w = WING_GUI_DEMO_DEPTH_CARD_W;
  rect.h = WING_GUI_DEMO_DEPTH_CARD_H;
  depth_back_front_style = theme.line_secondary;
  depth_back_front_style.fill = wing_color_rgba(85, 132, 230, 255);
  depth_back_front_style.opacity = 208;
  wing_card_init(&depth_back_card, &rect, &depth_back_front_style,
                 &depth_card_back_style, &depth_card_edge_style);
  wing_obj_set_event_cb(wing_card_obj(&depth_back_card),
                        wing_gui_demo_depth_back_card_event);
  wing_space_transform_init(&space_transform);
  space_transform.rotation_y = -12;
  space_transform.translation.z = WING_GUI_DEMO_DEPTH_BACK_Z;
  wing_obj_set_space_transform(wing_card_obj(&depth_back_card),
                               &space_transform);
  wing_obj_set_z_index(wing_card_obj(&depth_back_card),
                       WING_GUI_DEMO_DEPTH_CARD_Z_INDEX);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_card_obj(&depth_back_card));

  rect.x = WING_GUI_DEMO_MARGIN_X;
  rect.y = WING_GUI_DEMO_PROGRESS_Y;
  rect.w = (uint16_t)(demo_width -
                      WING_GUI_DEMO_MARGIN_X * 2);
  rect.h = WING_GUI_DEMO_PROGRESS_H;
  progress_frame_style = theme.progress_frame;
  progress_fill_style = theme.progress_fill;
  wing_progress_init(&progress, &rect, &progress_frame_style,
                     &progress_fill_style, WING_GUI_DEMO_PROGRESS_MIN,
                     WING_GUI_DEMO_PROGRESS_MAX,
                     WING_GUI_DEMO_PROGRESS_INITIAL);
  wing_progress_set_padding(&progress, WING_GUI_DEMO_PROGRESS_PADDING);
  wing_progress_set_step(&progress, WING_GUI_DEMO_PROGRESS_STEP);
  wing_progress_set_event_cb(&progress, wing_gui_demo_progress_event, NULL);
  wing_obj_set_z_index(wing_progress_obj(&progress),
                       WING_GUI_DEMO_PROGRESS_Z_INDEX);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_progress_obj(&progress));

  rect.x = WING_GUI_DEMO_MARGIN_X;
  rect.y = WING_GUI_DEMO_SLIDER_Y;
  rect.w = (uint16_t)(demo_width -
                      WING_GUI_DEMO_MARGIN_X * 2);
  rect.h = WING_GUI_DEMO_SLIDER_H;
  slider_track_style = theme.slider_track;
  slider_fill_style = theme.slider_fill;
  slider_knob_style = theme.slider_knob;
  slider_focused_style = theme.slider_focused;
  wing_slider_init(&slider, &rect, &slider_track_style, &slider_fill_style,
                   &slider_knob_style, WING_GUI_DEMO_PROGRESS_MIN,
                   WING_GUI_DEMO_PROGRESS_MAX,
                   WING_GUI_DEMO_SLIDER_INITIAL);
  wing_slider_set_state_style(&slider, WING_OBJ_STATE_FOCUSED,
                              &slider_focused_style);
  wing_slider_set_padding(&slider, WING_GUI_DEMO_SLIDER_PADDING);
  wing_slider_set_knob_size(&slider, WING_GUI_DEMO_SLIDER_KNOB_SIZE);
  wing_slider_set_track_height(&slider,
                               WING_GUI_DEMO_SLIDER_TRACK_HEIGHT);
  wing_slider_set_step(&slider, WING_GUI_DEMO_SLIDER_STEP);
  wing_slider_set_event_cb(&slider, wing_gui_demo_slider_event, NULL);
  wing_obj_set_z_index(wing_slider_obj(&slider),
                       WING_GUI_DEMO_SLIDER_Z_INDEX);
  (void)wing_obj_add_child(wing_box_obj(&root), wing_slider_obj(&slider));

  rect.x = WING_GUI_DEMO_MARGIN_X;
  rect.y = WING_GUI_DEMO_SCROLLBAR_Y;
  rect.w = (uint16_t)(demo_width -
                      WING_GUI_DEMO_MARGIN_X * 2);
  rect.h = WING_GUI_DEMO_SCROLLBAR_H;
  scrollbar_track_style = theme.scrollbar_track;
  scrollbar_thumb_style = theme.scrollbar_thumb;
  scrollbar_focused_style = theme.scrollbar_focused;
  wing_scrollbar_init(&scrollbar, &rect, &scrollbar_track_style,
                      &scrollbar_thumb_style, WING_GUI_DEMO_PROGRESS_MIN,
                      WING_GUI_DEMO_PROGRESS_MAX,
                      WING_GUI_DEMO_SCROLLBAR_INITIAL,
                      WING_GUI_DEMO_SCROLLBAR_PAGE_SIZE);
  wing_scrollbar_set_state_style(&scrollbar, WING_OBJ_STATE_FOCUSED,
                                 &scrollbar_focused_style);
  wing_scrollbar_set_padding(&scrollbar,
                             WING_GUI_DEMO_SCROLLBAR_PADDING);
  wing_scrollbar_set_min_thumb_length(&scrollbar,
                                      WING_GUI_DEMO_SCROLLBAR_MIN_THUMB);
  wing_scrollbar_set_step(&scrollbar, WING_GUI_DEMO_SCROLLBAR_STEP);
  wing_scrollbar_set_event_cb(&scrollbar, wing_gui_demo_scrollbar_event,
                              NULL);
  wing_obj_set_z_index(wing_scrollbar_obj(&scrollbar),
                       WING_GUI_DEMO_SCROLLBAR_Z_INDEX);
  (void)wing_obj_add_child(wing_box_obj(&root),
                           wing_scrollbar_obj(&scrollbar));

  rect.x = WING_GUI_DEMO_TOAST_X;
  rect.y = WING_GUI_DEMO_TOAST_Y;
  rect.w = WING_GUI_DEMO_TOAST_W;
  rect.h = WING_GUI_DEMO_TOAST_H;
  wing_box_init(&toast.toast, &rect, &theme.button_focused);
  wing_obj_set_event_cb(wing_box_obj(&toast.toast),
                        wing_gui_demo_toast_event);
  wing_obj_set_user_data(wing_box_obj(&toast.toast), "toast-box");

  rect.x = WING_GUI_DEMO_TOAST_LABEL_X;
  rect.y = WING_GUI_DEMO_TOAST_LABEL_Y;
  rect.w = WING_GUI_DEMO_TOAST_LABEL_W;
  rect.h = WING_GUI_DEMO_TOAST_LABEL_H;
  wing_label_init(&toast.label, &rect, "DONE \xe2\x9c\x93", theme.text,
                  WING_GUI_DEMO_LABEL_SCALE);
  wing_label_set_align(&toast.label, WING_TEXT_ALIGN_CENTER);
  wing_obj_set_enabled(wing_label_obj(&toast.label), false);
  wing_obj_set_event_cb(wing_label_obj(&toast.label),
                        wing_gui_demo_toast_event);
  wing_obj_set_user_data(wing_label_obj(&toast.label), "toast-label");
  (void)wing_obj_add_child(wing_box_obj(&toast.toast),
                           wing_label_obj(&toast.label));
  toast.root = wing_box_obj(&root);
  toast.active = false;

  ret = wing_gui_set_root(&gui, wing_box_obj(&root));
  if (ret < 0)
    {
      printf("wing_gui_demo: set root failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }

  rect.x = WING_GUI_DEMO_TRIANGLE_X;
  rect.y = WING_GUI_DEMO_TRIANGLE_Y;
  rect.w = WING_GUI_DEMO_TRIANGLE_W;
  rect.h = WING_GUI_DEMO_TRIANGLE_H;
  triangle_demo.gui = &gui;
  triangle_demo.rotation_y = WING_GUI_DEMO_TRIANGLE_ROTATION_Y;
  triangle_demo.z = WING_GUI_DEMO_TRIANGLE_Z;
  triangle_demo.opacity = WING_GUI_DEMO_TRIANGLE_OPACITY;
  wing_obj_init(&triangle_obj, &rect);
  wing_obj_set_user_data(&triangle_obj, &triangle_demo);
  wing_obj_set_geometry_cb(&triangle_obj, wing_gui_demo_triangle_draw,
                           wing_gui_demo_triangle_screen_bounds,
                           wing_gui_demo_triangle_contains_point);
  ret = wing_obj_add_child(wing_box_obj(&root), &triangle_obj);
  if (ret < 0)
    {
      printf("wing_gui_demo: triangle primitive object attach failed: %d\n",
             ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }

  gui_camera = wing_gui_get_camera(&gui);
  if (gui_camera != NULL)
    {
      demo_camera = *gui_camera;
      demo_camera.focal_length =
        (uint16_t)(demo_camera.focal_length + 24);
      wing_gui_set_camera(&gui, &demo_camera);
      demo_camera.focal_length =
        (uint16_t)(demo_camera.focal_length - 24);
      wing_gui_set_camera(&gui, &demo_camera);
      printf("wing_gui_demo: runtime camera update invalidated default object space and restored focal=%u\n",
             (unsigned int)demo_camera.focal_length);
      printf("wing_gui_demo: core camera equality API reports restored=%s\n",
             wing_camera_equal(wing_gui_get_camera(&gui),
                               &demo_camera) ? "yes" : "no");
      camera_events_before_noop = camera_event_count;
      wing_gui_set_camera(&gui, &demo_camera);
      printf("wing_gui_demo: no-op camera update skipped event count=%u unchanged=%s\n",
             (unsigned int)camera_event_count,
             camera_event_count == camera_events_before_noop ?
             "yes" : "no");
    }

  space_card_hit_point.x = (int16_t)(space_card_x +
                                     WING_GUI_DEMO_SPACE_CARD_W / 2);
  space_card_hit_point.y = (int16_t)(space_card_y +
                                     WING_GUI_DEMO_SPACE_CARD_H / 2);
  if (wing_obj_project_quad(wing_card_obj(&space_card),
                            &card_projected_quad) == 0)
    {
      space_card_hit_point.x =
        (int16_t)(((int32_t)card_projected_quad.vertices[0].screen.x +
                   card_projected_quad.vertices[1].screen.x +
                   card_projected_quad.vertices[2].screen.x +
                   card_projected_quad.vertices[3].screen.x) / 4);
      space_card_hit_point.y =
        (int16_t)(((int32_t)card_projected_quad.vertices[0].screen.y +
                   card_projected_quad.vertices[1].screen.y +
                   card_projected_quad.vertices[2].screen.y +
                   card_projected_quad.vertices[3].screen.y) / 4);
    }

  depth_card_hit_point.x = (int16_t)(depth_card_x +
                                     WING_GUI_DEMO_DEPTH_CARD_W / 2);
  depth_card_hit_point.y = (int16_t)(depth_card_y +
                                     WING_GUI_DEMO_DEPTH_CARD_H / 2);
  if (wing_obj_project_quad(wing_card_obj(&depth_front_card),
                            &card_projected_quad) == 0)
    {
      depth_card_hit_point.x =
        (int16_t)(((int32_t)card_projected_quad.vertices[0].screen.x +
                   card_projected_quad.vertices[1].screen.x +
                   card_projected_quad.vertices[2].screen.x +
                   card_projected_quad.vertices[3].screen.x) / 4);
      depth_card_hit_point.y =
        (int16_t)(((int32_t)card_projected_quad.vertices[0].screen.y +
                   card_projected_quad.vertices[1].screen.y +
                   card_projected_quad.vertices[2].screen.y +
                   card_projected_quad.vertices[3].screen.y) / 4);
    }
  printf("wing_gui_demo: dynamic space demo layout uses explicit control z-layers progress=%d slider=%d scrollbar=%d above space cards space_card=%d,%d depth_card=%d,%d hit=%d,%d depth_hit=%d,%d\n",
         WING_GUI_DEMO_PROGRESS_Z_INDEX,
         WING_GUI_DEMO_SLIDER_Z_INDEX,
         WING_GUI_DEMO_SCROLLBAR_Z_INDEX,
         space_card_x, space_card_y, depth_card_x, depth_card_y,
         space_card_hit_point.x, space_card_hit_point.y,
         depth_card_hit_point.x, depth_card_hit_point.y);

  wing_gui_demo_print_stage("synthetic input script");
  click.point.x = WING_GUI_DEMO_BUTTON_INPUT_X;
  click.point.y = WING_GUI_DEMO_BUTTON_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.encoder_delta = 0;
  click.button = 0;
  click.source = WING_INPUT_SOURCE_MOUSE;
  click.type = WING_INPUT_POINTER_MOVE;
  wing_gui_demo_enqueue_input(&gui, &click, "button pointer hover enter");
  click.point.x = WING_GUI_DEMO_POINTER_OUTSIDE_X;
  wing_gui_demo_enqueue_input(&gui, &click, "button pointer hover leave");
  click.point.x = WING_GUI_DEMO_BUTTON_INPUT_X;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "button pointer down");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "button pointer up");

  click.point.x = WING_GUI_DEMO_CHECKBOX_INPUT_X;
  click.point.y = WING_GUI_DEMO_CHECKBOX_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.button = 0;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "checkbox pointer down");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "checkbox pointer up");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_SPACE;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "checkbox keyboard space");

  click.point.x = (int16_t)(demo_width -
                            WING_GUI_DEMO_SWITCH_INPUT_RIGHT_OFFSET);
  click.point.y = WING_GUI_DEMO_SWITCH_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.button = 0;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "switch pointer down");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "switch pointer up");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_SPACE;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "switch keyboard space");

  click.key = WING_KEY_UNKNOWN;
  click.button = 1;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.point.x = WING_GUI_DEMO_TEXT_INPUT_INPUT_X;
  click.point.y = WING_GUI_DEMO_TEXT_INPUT_INPUT_Y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "text input pointer down focus");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "text input pointer up focus");
  click.button = 0;
  click.point.x = 0;
  click.point.y = 0;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  click.key = 'A';
  wing_gui_demo_enqueue_input(&gui, &click, "text input key A");
  click.key = 'B';
  wing_gui_demo_enqueue_input(&gui, &click, "text input key B");
  click.key = WING_KEY_LEFT;
  wing_gui_demo_enqueue_input(&gui, &click, "text input cursor left");
  click.key = WING_KEY_BACKSPACE;
  wing_gui_demo_enqueue_input(&gui, &click, "text input backspace");
  click.key = 'C';
  wing_gui_demo_enqueue_input(&gui, &click, "text input key C");
  click.key = WING_KEY_ENTER;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "text input enter control key ignored");

  click.key = WING_KEY_UNKNOWN;
  click.button = 0;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.point.x = WING_GUI_DEMO_SCROLL_INPUT_X;
  click.point.y = WING_GUI_DEMO_SCROLL_INPUT_Y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "scroll view pointer down focus");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "scroll view pointer up focus");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_RIGHT;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "scroll view keyboard right");
  click.key = WING_KEY_UNKNOWN;
  click.encoder_delta = 2;
  click.source = WING_INPUT_SOURCE_ENCODER;
  click.type = WING_INPUT_ENCODER_ROTATE;
  wing_gui_demo_enqueue_input(&gui, &click, "scroll view encoder rotate +2");
  click.encoder_delta = 0;

  click.key = WING_KEY_UNKNOWN;
  click.button = 0;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.point.x = space_card_hit_point.x;
  click.point.y = space_card_hit_point.y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "space card projected pointer down");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "space card projected pointer up");
  click.point.x = depth_card_hit_point.x;
  click.point.y = depth_card_hit_point.y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "depth cards same z-index pointer down");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "depth cards same z-index pointer up");

  click.point.x = 0;
  click.point.y = 0;
  click.button = 0;
  click.key = WING_KEY_TAB;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "focus traversal tab");
  click.key = WING_KEY_ENTER;
  wing_gui_demo_enqueue_input(&gui, &click, "focused button enter");

  click.key = WING_KEY_UNKNOWN;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.point.x = WING_GUI_DEMO_PROGRESS_INPUT_START_X;
  click.point.y = WING_GUI_DEMO_PROGRESS_INPUT_Y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "progress pointer down");
  click.point.x = WING_GUI_DEMO_PROGRESS_INPUT_DRAG_X;
  click.type = WING_INPUT_POINTER_MOVE;
  wing_gui_demo_enqueue_input(&gui, &click, "progress pointer drag");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click, "progress pointer up");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_RIGHT;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "progress keyboard right");
  click.key = WING_KEY_UNKNOWN;
  click.encoder_delta = 2;
  click.source = WING_INPUT_SOURCE_ENCODER;
  click.type = WING_INPUT_ENCODER_ROTATE;
  wing_gui_demo_enqueue_input(&gui, &click, "progress encoder rotate +2");
  click.encoder_delta = 0;

  click.key = WING_KEY_UNKNOWN;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.point.x = WING_GUI_DEMO_SLIDER_INPUT_X;
  click.point.y = WING_GUI_DEMO_SLIDER_INPUT_Y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "slider pointer down");
  click.point.x = WING_GUI_DEMO_POINTER_OUTSIDE_X;
  click.type = WING_INPUT_POINTER_MOVE;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "slider pointer drag outside capture");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "slider pointer up outside capture");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_RIGHT;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "slider keyboard right");
  click.key = WING_KEY_LEFT;
  wing_gui_demo_enqueue_input(&gui, &click, "slider keyboard left");
  click.key = WING_KEY_UNKNOWN;
  click.encoder_delta = 2;
  click.source = WING_INPUT_SOURCE_ENCODER;
  click.type = WING_INPUT_ENCODER_ROTATE;
  wing_gui_demo_enqueue_input(&gui, &click, "slider encoder rotate +2");
  click.encoder_delta = 0;

  click.point.x = WING_GUI_DEMO_SCROLLBAR_INPUT_X;
  click.point.y = WING_GUI_DEMO_SCROLLBAR_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "scrollbar pointer down");
  click.point.x = WING_GUI_DEMO_POINTER_OUTSIDE_X;
  click.type = WING_INPUT_POINTER_MOVE;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "scrollbar pointer drag outside capture");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "scrollbar pointer up outside capture");
  click.point.x = 0;
  click.point.y = 0;
  click.key = WING_KEY_RIGHT;
  click.source = WING_INPUT_SOURCE_KEYBOARD;
  click.type = WING_INPUT_KEY_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click, "scrollbar keyboard right");
  click.key = WING_KEY_LEFT;
  wing_gui_demo_enqueue_input(&gui, &click, "scrollbar keyboard left");
  click.key = WING_KEY_UNKNOWN;
  click.encoder_delta = -2;
  click.source = WING_INPUT_SOURCE_ENCODER;
  click.type = WING_INPUT_ENCODER_ROTATE;
  wing_gui_demo_enqueue_input(&gui, &click, "scrollbar encoder rotate -2");
  click.encoder_delta = 0;

  click.point.x = WING_GUI_DEMO_SLIDER_INPUT_X;
  click.point.y = WING_GUI_DEMO_SLIDER_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.source = WING_INPUT_SOURCE_TOUCH;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "slider pointer down before capture cancel");
  click.point.x = WING_GUI_DEMO_SCROLLBAR_INPUT_X;
  click.point.y = WING_GUI_DEMO_SCROLLBAR_INPUT_Y;
  click.type = WING_INPUT_POINTER_DOWN;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "scrollbar pointer down cancels slider capture");
  click.type = WING_INPUT_POINTER_UP;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "scrollbar pointer up after capture cancel");
  click.point.x = WING_GUI_DEMO_BUTTON_INPUT_X;
  click.point.y = WING_GUI_DEMO_BUTTON_INPUT_Y;
  click.key = WING_KEY_UNKNOWN;
  click.source = WING_INPUT_SOURCE_MOUSE;
  click.type = WING_INPUT_POINTER_MOVE;
  wing_gui_demo_enqueue_input(&gui, &click,
                              "button pointer hover final settle");

  input_provider.presenter = have_presenter ? &presenter : NULL;
  wing_input_adapter_init(&input_provider.adapter);
  input_provider.raw_mouse_seq = 0;
  input_provider.raw_mouse_pressed = false;
  input_provider.count = 1;
  input_provider.index = 0;
  input_provider.events[0].point.x = 0;
  input_provider.events[0].point.y = 0;
  input_provider.events[0].button = 0;
  input_provider.events[0].encoder_delta = 0;
  input_provider.events[0].key = WING_KEY_RIGHT;
  input_provider.events[0].source = WING_INPUT_SOURCE_KEYBOARD;
  input_provider.events[0].type = WING_INPUT_KEY_DOWN;
  wing_gui_set_input_reader(&gui, wing_gui_demo_input_provider,
                            &input_provider);

  theme_timer.theme = &contrast_theme;
  theme_timer.root = &root;
  theme_timer.header = &header;
  theme_timer.card = &card;
  theme_timer.button = &button;
  theme_timer.label = &button_label;
  theme_timer.line_primary = &line_primary;
  theme_timer.line_secondary = &line_secondary;
  theme_timer.fill_panel = &fill_panel;
  theme_timer.fill_badge = &fill_badge;
  theme_timer.space_card = &space_card;
  theme_timer.progress = &progress;
  theme_timer.slider = &slider;
  theme_timer.scrollbar = &scrollbar;
  theme_timer.power_switch = &power_switch;
  theme_timer.checkbox = &checkbox;
  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_THEME_TIMER_FRAMES,
                             false,
                             wing_gui_demo_theme_timer, &theme_timer,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: theme timer start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: theme switch timer scheduled period=%ums target=high_contrast\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 2));

  progress_timer.progress = &progress;
  progress_timer.target_value = WING_GUI_DEMO_PROGRESS_TARGET;
  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_PROGRESS_TIMER_FRAMES,
                             false,
                             wing_gui_demo_progress_timer, &progress_timer,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: timer start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: timer scheduled period=%ums repeat=0 target_progress=%u\n",
         (unsigned int)WING_GUI_DEMO_FRAME_MS,
         (unsigned int)progress_timer.target_value);

  repeat_timer.line = &line_secondary;
  repeat_timer.first_style = line_secondary_style;
  repeat_timer.first_style.fill = WING_GUI_DEMO_REPEAT_FIRST_FILL;
  repeat_timer.first_style.has_fill = true;
  repeat_timer.second_style = line_secondary_style;
  repeat_timer.second_style.fill = WING_GUI_DEMO_REPEAT_SECOND_FILL;
  repeat_timer.second_style.has_fill = true;
  repeat_timer.timer_id = 0;
  repeat_timer.tick_count = 0;
  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_REPEAT_TIMER_FRAMES,
                             true,
                             wing_gui_demo_repeat_timer, &repeat_timer,
                             &repeat_timer.timer_id);
  if (ret < 0)
    {
      printf("wing_gui_demo: repeat timer start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: repeat timer scheduled period=%ums repeat=1 target=line_secondary stop_after=3\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 2));

  line_anim.box = &line_primary;
  ret = wing_gui_anim_start_path(&gui,
                                 WING_GUI_DEMO_LINE_PRIMARY_W,
                                 WING_GUI_DEMO_LINE_PRIMARY_W + 54,
                                 WING_GUI_DEMO_FRAME_MS *
                                 WING_GUI_DEMO_ANIMATION_FRAMES,
                                 WING_ANIM_PATH_EASE_OUT,
                                 wing_gui_demo_width_anim_apply,
                                 wing_gui_demo_width_anim_done, &line_anim,
                                 NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: animation start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: animation scheduled property=line_width from=%u to=%u duration=%ums path=ease_out\n",
         (unsigned int)WING_GUI_DEMO_LINE_PRIMARY_W,
         (unsigned int)(WING_GUI_DEMO_LINE_PRIMARY_W + 54),
         (unsigned int)(WING_GUI_DEMO_FRAME_MS *
                        WING_GUI_DEMO_ANIMATION_FRAMES));

  loop_line_anim.box = &line_secondary;
  loop_line_anim.min_width = WING_GUI_DEMO_LINE_SECONDARY_W;
  loop_line_anim.max_width = WING_GUI_DEMO_LINE_SECONDARY_W + 36;
  loop_line_anim.segments_completed = 0;
  loop_line_anim.forward = true;
#if WING_GUI_DEMO_ENABLE_LOOP_ANIMATION
  ret = wing_gui_demo_loop_width_anim_start(&gui, &loop_line_anim);
  if (ret < 0)
    {
      printf("wing_gui_demo: loop animation start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: continuous pulse animation scheduled property=line_secondary_width min=%u max=%u repeat=forever path=ease_in_out\n",
         (unsigned int)loop_line_anim.min_width,
         (unsigned int)loop_line_anim.max_width);
#else
  (void)loop_line_anim;
  printf("wing_gui_demo: continuous pulse animation disabled for input debugging\n");
#endif

  space_anim.card = &space_card;
  ret = wing_gui_anim_start_path(&gui,
                                 WING_GUI_DEMO_SPACE_CARD_ROTATION_START,
                                 WING_GUI_DEMO_SPACE_CARD_ROTATION_END,
                                 WING_GUI_DEMO_FRAME_MS *
                                 WING_GUI_DEMO_ANIMATION_FRAMES,
                                 WING_ANIM_PATH_EASE_IN_OUT,
                                 wing_gui_demo_space_anim_apply,
                                 wing_gui_demo_space_anim_done,
                                 &space_anim, NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: space animation start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: space card animation scheduled rotation_y=%d..%d duration=%ums path=ease_in_out\n",
         WING_GUI_DEMO_SPACE_CARD_ROTATION_START,
         WING_GUI_DEMO_SPACE_CARD_ROTATION_END,
         (unsigned int)(WING_GUI_DEMO_FRAME_MS *
                        WING_GUI_DEMO_ANIMATION_FRAMES));

  disable_timer.button = &button;
  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_DISABLE_TIMER_FRAMES,
                             false,
                             wing_gui_demo_disable_timer, &disable_timer,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: disabled timer start failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: disabled timer scheduled period=%ums target=button\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 3));

  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_TOAST_CREATE_FRAMES,
                             false,
                             wing_gui_demo_toast_create_timer, &toast,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: lifecycle create timer start failed: %d\n",
             ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: lifecycle create timer scheduled period=%ums target=toast-subtree\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 4));

  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_TOAST_HIDE_FRAMES,
                             false,
                             wing_gui_demo_toast_hide_timer, &toast,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: visibility hide timer start failed: %d\n",
             ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: visibility hide timer scheduled period=%ums target=toast-subtree\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 5));

  ret = wing_gui_timer_start(&gui,
                             WING_GUI_DEMO_FRAME_MS *
                             WING_GUI_DEMO_TOAST_DESTROY_FRAMES,
                             false,
                             wing_gui_demo_toast_destroy_timer, &toast,
                             NULL);
  if (ret < 0)
    {
      printf("wing_gui_demo: lifecycle destroy timer start failed: %d\n",
             ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }
  printf("wing_gui_demo: lifecycle destroy timer scheduled period=%ums target=toast-subtree\n",
         (unsigned int)(WING_GUI_DEMO_FRAME_MS * 6));

  fr_backend_registry_reset();
  ret = fr_backend_register_builtin();
  if (ret < 0)
    {
      printf("wing_gui_demo: backend register failed: %d\n", ret);
      wing_gui_destroy(&gui);
      free(pixels);
      return EXIT_FAILURE;
    }

  if (have_presenter)
    {
      input_provider.presenter = &presenter;
      ret = fr_backend_caps_from_fb_presenter(&presenter, &fb_caps);
      if (ret == 0)
        {
          (void)fr_backend_register_fb_presenter(&presenter);
        }

      printf("wing_gui_demo: framebuffer %ux%u fmt=%u bpp=%u stride=%u\n",
             (unsigned int)presenter.xres, (unsigned int)presenter.yres,
             (unsigned int)presenter.fmt, (unsigned int)presenter.bpp,
             (unsigned int)presenter.stride);
    }
  else
    {
      printf("wing_gui_demo: framebuffer present skipped: %d\n",
             presenter_open_ret);
    }

  printf("wing_gui_demo: input provider registered events=%u source=%s\n",
         (unsigned int)input_provider.count,
         have_presenter ? "x11-window+demo-script" :
                          "demo-script-no-presenter");

  wing_gui_demo_print_stage("runtime capability summary");
  printf("wing_gui_demo: app task entered WING GUI loop\n");
  printf("wing_gui_demo: frame interval=%ums\n",
         (unsigned int)WING_GUI_DEMO_FRAME_MS);
  gui_camera = wing_gui_get_camera(&gui);
  if (gui_camera != NULL)
    {
      printf("wing_gui_demo: runtime camera viewport x=%d y=%d w=%u h=%u focal=%u\n",
             gui_camera->viewport.x, gui_camera->viewport.y,
             (unsigned int)gui_camera->viewport.w,
             (unsigned int)gui_camera->viewport.h,
             (unsigned int)gui_camera->focal_length);
    }

  if (!have_presenter)
    {
      printf("wing_gui_demo: headless validation runs %u frames before exit\n",
             (unsigned int)WING_GUI_DEMO_HEADLESS_FRAMES);
    }

  printf("wing_gui_demo: panel widget card hosts button and line objects with horizontal stack layout\n");
  printf("wing_gui_demo: button uses center layout to place GO label child\n");
  button_label_font = wing_label_get_font(&button_label);
  if (wing_label_get_text_size(&button_label, &button_label_text_w,
                               &button_label_text_h) == 0)
    {
      printf("wing_gui_demo: label uses font resource name=%s text_size=%ux%u align=%d\n",
             button_label_font != NULL && button_label_font->name != NULL ?
             button_label_font->name : "unknown",
             (unsigned int)button_label_text_w,
             (unsigned int)button_label_text_h,
             (int)wing_label_get_align(&button_label));
    }

  utf8_sample = "GO \xe2\x9c\x93";
  utf8_cursor = utf8_sample;
  utf8_codepoints = 0;
  while (wing_text_next_codepoint(&utf8_cursor, &utf8_codepoint) > 0)
    {
      utf8_codepoints++;
    }

  if (wing_font_measure_text(button_label_font, utf8_sample,
                             WING_GUI_DEMO_LABEL_SCALE,
                             &utf8_text_w, &utf8_text_h) == 0)
    {
      printf("wing_gui_demo: UTF-8 text decode sample codepoints=%u measured=%ux%u builtin font falls back missing glyphs\n",
             (unsigned int)utf8_codepoints,
             (unsigned int)utf8_text_w,
             (unsigned int)utf8_text_h);
    }

  multiline_sample = "GO\nDONE";
  if (wing_font_measure_text(button_label_font, multiline_sample,
                             WING_GUI_DEMO_LABEL_SCALE,
                             &multiline_text_w, &multiline_text_h) == 0)
    {
      printf("wing_gui_demo: multiline label measurement sample lines=2 measured=%ux%u explicit newline uses font line_height\n",
             (unsigned int)multiline_text_w,
             (unsigned int)multiline_text_h);
    }

  if (wing_label_get_layout_size(&ellipsis_label, &ellipsis_layout_w,
                                 &ellipsis_layout_h) == 0)
    {
      printf("wing_gui_demo: ellipsis label layout mode=%d bounds=42x14 layout=%ux%u text=LONG_LABEL\n",
             (int)wing_label_get_text_mode(&ellipsis_label),
             (unsigned int)ellipsis_layout_w,
             (unsigned int)ellipsis_layout_h);
    }

  if (wing_label_get_layout_size(&wrap_label, &wrap_layout_w,
                                 &wrap_layout_h) == 0)
    {
      printf("wing_gui_demo: wrap label layout mode=%d bounds=44x28 layout=%ux%u text=DONE_DONE word-break=space\n",
             (int)wing_label_get_text_mode(&wrap_label),
             (unsigned int)wrap_layout_w,
             (unsigned int)wrap_layout_h);
    }

  printf("wing_gui_demo: fill layout stretches badge child inside fixed demo panel\n");
  wing_scroll_view_get_offset(&scroll_view, &scroll_offset_x,
                              &scroll_offset_y);
  scroll_view_bounds = *wing_obj_get_bounds(
    wing_scroll_view_obj(&scroll_view));
  scroll_content_bounds = *wing_obj_get_bounds(
    wing_box_obj(&scroll_content));
  printf("wing_gui_demo: scroll view clips content with offset=%d,%d viewport=%d,%d,%u,%u first_child=%d,%d,%u,%u\n",
         scroll_offset_x, scroll_offset_y,
         scroll_view_bounds.x, scroll_view_bounds.y,
         (unsigned int)scroll_view_bounds.w,
         (unsigned int)scroll_view_bounds.h,
         scroll_content_bounds.x, scroll_content_bounds.y,
         (unsigned int)scroll_content_bounds.w,
         (unsigned int)scroll_content_bounds.h);
  printf("wing_gui_demo: scroll view content=%ux%u step=%u,%u max_offset=%u,%u\n",
         (unsigned int)WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_W,
         (unsigned int)WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_H,
         (unsigned int)WING_GUI_DEMO_SCROLL_STEP_X,
         (unsigned int)WING_GUI_DEMO_SCROLL_STEP_Y,
         (unsigned int)(WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_W -
                        WING_GUI_DEMO_SCROLL_VIEW_W),
         (unsigned int)(WING_GUI_DEMO_SCROLL_CONTENT_TOTAL_H -
                        WING_GUI_DEMO_SCROLL_VIEW_H));
  clip_panel_bounds = *wing_obj_get_bounds(wing_box_obj(&clip_panel));
  clip_child_bounds = *wing_obj_get_bounds(wing_box_obj(&clip_child));
  printf("wing_gui_demo: clip children panel enabled=%s panel=%d,%d,%u,%u child=%d,%d,%u,%u child_overflows=yes\n",
         wing_obj_get_clip_children(wing_box_obj(&clip_panel)) ?
         "yes" : "no",
         clip_panel_bounds.x, clip_panel_bounds.y,
         (unsigned int)clip_panel_bounds.w,
         (unsigned int)clip_panel_bounds.h,
         clip_child_bounds.x, clip_child_bounds.y,
         (unsigned int)clip_child_bounds.w,
         (unsigned int)clip_child_bounds.h);
  printf("wing_gui_demo: core space treats 2D widgets as identity space transform objects by default\n");
  printf("wing_gui_demo: 2D widget bounds project through core space wing_project_rect_quad as default planes\n");
  printf("wing_gui_demo: local points transform through core space before camera projection\n");
  printf("wing_gui_demo: projected quads produce conservative screen dirty bounds through core space\n");
  printf("wing_gui_demo: object z-index orders sibling draw/hit in default space, space card layer=%d\n",
         wing_obj_get_z_index(wing_card_obj(&space_card)));
  space_card_order =
    wing_obj_compare_space_order(wing_box_obj(&line_primary),
                                 wing_card_obj(&space_card));
  printf("wing_gui_demo: object space order line_primary_vs_space_card=%d negative means first draws behind\n",
         space_card_order);
  printf("wing_gui_demo: same z-index siblings are sorted by projected depth, smaller depth is frontmost for hit-test\n");
  if (wing_obj_get_projected_depth(wing_card_obj(&depth_front_card),
                                   &depth_front_average_depth) == 0 &&
      wing_obj_get_projected_depth(wing_card_obj(&depth_back_card),
                                   &depth_back_average_depth) == 0)
    {
      depth_front_back_order =
        wing_obj_compare_space_order(wing_card_obj(&depth_front_card),
                                     wing_card_obj(&depth_back_card));
      printf("wing_gui_demo: same z-index depth cards front_depth=%ld back_depth=%ld order=%d positive means front draws after back\n",
             (long)depth_front_average_depth,
             (long)depth_back_average_depth,
             depth_front_back_order);
    }
  wing_obj_reset_space_transform(wing_box_obj(&root));
  printf("wing_gui_demo: root object space transform identity=%s after wing_obj_reset_space_transform\n",
         wing_obj_space_transform_is_identity(wing_box_obj(&root)) ?
         "yes" : "no");
  printf("wing_gui_demo: core space transform identity API reports root=%s\n",
         wing_space_transform_is_identity(
           wing_obj_get_space_transform(wing_box_obj(&root))) ? "yes" : "no");
  printf("wing_gui_demo: root object default 2D plane API reports root=%s in default object space\n",
         wing_obj_is_default_2d(wing_box_obj(&root)) ? "yes" : "no");
  if (wing_obj_get_screen_bounds(wing_box_obj(&root), &root_screen_bounds))
    {
      printf("wing_gui_demo: root screen bounds after identity transform x=%d y=%d w=%u h=%u\n",
             root_screen_bounds.x, root_screen_bounds.y,
             (unsigned int)root_screen_bounds.w,
             (unsigned int)root_screen_bounds.h);
    }

  if (wing_obj_get_screen_bounds(wing_card_obj(&space_card),
                                 &card_screen_bounds))
    {
      printf("wing_gui_demo: space card screen dirty bounds x=%d y=%d w=%u h=%u\n",
             card_screen_bounds.x, card_screen_bounds.y,
             (unsigned int)card_screen_bounds.w,
             (unsigned int)card_screen_bounds.h);
    }

  if (wing_obj_project_quad2d(wing_card_obj(&space_card), &card_quad) == 0)
    {
      printf("wing_gui_demo: space card projected quad p0=%d,%d p1=%d,%d p2=%d,%d p3=%d,%d\n",
             card_quad.points[0].x, card_quad.points[0].y,
             card_quad.points[1].x, card_quad.points[1].y,
             card_quad.points[2].x, card_quad.points[2].y,
             card_quad.points[3].x, card_quad.points[3].y);
    }

  if (wing_obj_get_world_space_transform(wing_card_obj(&space_card),
                                         &card_world_transform) == 0)
    {
      printf("wing_gui_demo: space card world transform rotation_y=%d scale_q10=%u\n",
             card_world_transform.rotation_y,
             (unsigned int)card_world_transform.scale_q10);

      space_card_center.x = 0;
      space_card_center.y = 0;
      space_card_center.z = 0;
      if (gui_camera != NULL &&
          wing_project_point_with_depth(gui_camera, &card_world_transform,
                                        &space_card_center,
                                        &space_card_center_screen,
                                        &space_card_depth) == 0)
        {
          printf("wing_gui_demo: space card center projects to %d,%d depth=%ld through core space\n",
                 space_card_center_screen.x, space_card_center_screen.y,
                 (long)space_card_depth);
        }

      if (wing_obj_project_quad(wing_card_obj(&space_card),
                                &card_projected_quad) == 0)
        {
          printf("wing_gui_demo: object projected quad depths d0=%ld d1=%ld d2=%ld d3=%ld through core object space\n",
                 (long)card_projected_quad.vertices[0].depth,
                 (long)card_projected_quad.vertices[1].depth,
                 (long)card_projected_quad.vertices[2].depth,
                 (long)card_projected_quad.vertices[3].depth);
        }

      if (wing_obj_get_projected_depth(wing_card_obj(&space_card),
                                       &space_card_average_depth) == 0)
        {
          printf("wing_gui_demo: object projected average depth=%ld for future space sorting\n",
                 (long)space_card_average_depth);
        }
    }

  printf("wing_gui_demo: space card projected hit-test point=%d,%d contains=%s\n",
         space_card_hit_point.x, space_card_hit_point.y,
         wing_obj_contains_point(wing_card_obj(&space_card),
                                 space_card_hit_point) ? "yes" : "no");
  printf("wing_gui_demo: projected quad hit-test uses core space quad contains API\n");
  printf("wing_gui_demo: space card uses object space transform -> WING camera/project -> FRender quad command fallback\n");
  printf("wing_gui_demo: space card pointer input uses projected quad hit-test before event dispatch\n");
  printf("wing_gui_demo: space card also participates in z-index ordered sibling draw/hit traversal\n");
  printf("wing_gui_demo: style opacity applies alpha before FRender software source-over blend, space card opacity=%u edge=%u\n",
         (unsigned int)space_card_front_style.opacity,
         (unsigned int)space_card_edge_style.opacity);
  printf("wing_gui_demo: object opacity inherits through object tree, fill_panel opacity=%u badge_effective=%u\n",
         (unsigned int)wing_obj_get_opacity(wing_box_obj(&fill_panel)),
         (unsigned int)wing_obj_get_effective_opacity(
           wing_box_obj(&fill_badge)));
  printf("wing_gui_demo: ordinary box fill_panel uses default space transform rotation_y=%d z=%d and shared widget style projects fill and stroke through FRender quad commands when non-identity\n",
         WING_GUI_DEMO_FILL_PANEL_ROTATION_Y,
         WING_GUI_DEMO_FILL_PANEL_Z);
  printf("wing_gui_demo: label widget text=GO consumes wing_font_t glyph resources and wing_obj_set_enabled(false) to skip input\n");
  printf("wing_gui_demo: label layout supports explicit newline, space-aware wrap and ellipsis text modes\n");
  printf("wing_gui_demo: wrap label uses default object space rotation_y=%d z=%d and bitmap glyph pixels render through shared projected quad fill helper\n",
         WING_GUI_DEMO_LABEL_SPACE_ROTATION_Y,
         WING_GUI_DEMO_LABEL_SPACE_Z);
  printf("wing_gui_demo: toast label includes escaped UTF-8 text and uses builtin font fallback glyph for unsupported codepoints\n");
  wing_text_input_get_selection(&text_input, &text_selection_start,
                                &text_selection_end);
  printf("wing_gui_demo: text input widget edits a fixed single-line buffer through wing_text_edit_t with visible selection highlight=%s selection=%u..%u, printable keys, arrows, backspace, cursor box and ignored non-text control keys\n",
         wing_text_input_has_selection(&text_input) ? "yes" : "no",
         (unsigned int)text_selection_start,
         (unsigned int)text_selection_end);
  printf("wing_gui_demo: image widget consumes wing_image_resource_t and renders static RGBA resource through FRender image quad command when object space is non-identity, rotation_y=%d z=%d image_effective_opacity=%u\n",
         WING_GUI_DEMO_IMAGE_ROTATION_Y,
         WING_GUI_DEMO_IMAGE_Z,
         (unsigned int)wing_obj_get_effective_opacity(
           wing_image_obj(&demo_image)));
  if (wing_gui_demo_triangle_project(&triangle_demo,
                                     wing_obj_get_bounds(&triangle_obj),
                                     &triangle_projected) == 0 &&
      wing_obj_get_screen_bounds(&triangle_obj, &triangle_screen_bounds))
    {
      printf("wing_gui_demo: custom object screen bounds callback reports projected triangle dirty bounds x=%d y=%d w=%u h=%u depth0=%ld depth1=%ld depth2=%ld through core space\n",
             triangle_screen_bounds.x, triangle_screen_bounds.y,
             (unsigned int)triangle_screen_bounds.w,
             (unsigned int)triangle_screen_bounds.h,
             (long)triangle_projected.vertices[0].depth,
             (long)triangle_projected.vertices[1].depth,
             (long)triangle_projected.vertices[2].depth);
      printf("wing_gui_demo: custom object contains callback reports triangle vertex_hit=%s corner_hit=%s through projected triangle geometry\n",
             wing_projected_triangle_contains_point(
               &triangle_projected,
               triangle_projected.vertices[0].screen) ? "yes" : "no",
             wing_obj_contains_point(&triangle_obj,
                                     (wing_point_t){ triangle_screen_bounds.x,
                                                    triangle_screen_bounds.y }) ?
             "yes" : "no");
      printf("wing_gui_demo: custom geometry is a normal WING object using unified geometry callbacks, not a separate 3D view\n");
      printf("wing_gui_demo: custom geometry screen-bounds/user-data updates dirty both old and new projected bounds\n");
    }
  printf("wing_gui_demo: triangle primitive object uses runtime camera + core space projection -> WING render node/material seed -> FRender fill_triangle command seed bounds=%d,%d,%u,%u rotation_y=%d z=%d opacity=%u\n",
         WING_GUI_DEMO_TRIANGLE_X,
         WING_GUI_DEMO_TRIANGLE_Y,
         (unsigned int)WING_GUI_DEMO_TRIANGLE_W,
         (unsigned int)WING_GUI_DEMO_TRIANGLE_H,
         WING_GUI_DEMO_TRIANGLE_ROTATION_Y,
         WING_GUI_DEMO_TRIANGLE_Z,
         (unsigned int)WING_GUI_DEMO_TRIANGLE_OPACITY);
  printf("wing_gui_demo: progress widget value advances by WING timer and direct pointer/key/encoder input\n");
  printf("wing_gui_demo: repeat timer updates line style and stops itself through wing_gui_timer_stop\n");
  printf("wing_gui_demo: animation path ease_out drives line width through WING animation runtime\n");
  printf("wing_gui_demo: loop animation uses done callback to restart a ping-pong width animation and then stops\n");
  printf("wing_gui_demo: WING_EVENT_BOUNDS_CHANGED reports animated object bounds changes\n");
  printf("wing_gui_demo: WING_EVENT_SPACE_TRANSFORM_CHANGED reports object space transform changes\n");
  printf("wing_gui_demo: WING_EVENT_CAMERA_CHANGED reports runtime camera changes and triggers default space redraw\n");
  printf("wing_gui_demo: wing_camera_equal keeps no-op camera updates inside core space semantics and skips redundant camera events\n");
  printf("wing_gui_demo: space transform changed events use core space equality to skip no-op updates\n");
  printf("wing_gui_demo: slider widget consumes pointer/key input and emits value changed events\n");
  printf("wing_gui_demo: scrollbar widget reuses WING value model and consumes pointer/key input\n");
  printf("wing_gui_demo: progress/slider/scrollbar use explicit z-layers progress=%d slider=%d scrollbar=%d so draggable controls stay above decorative space cards and upper controls remain visually stable when state rings redraw\n",
         WING_GUI_DEMO_PROGRESS_Z_INDEX,
         WING_GUI_DEMO_SLIDER_Z_INDEX,
         WING_GUI_DEMO_SCROLLBAR_Z_INDEX);
  printf("wing_gui_demo: pointer capture keeps slider/scrollbar drag active after pointer leaves widget bounds\n");
  printf("wing_gui_demo: widget pointer lifecycle helper clears pressed state on pointer up/cancel\n");
  printf("wing_gui_demo: pointer hover enter/leave drives hovered state style before click/focus handling\n");
  printf("wing_gui_demo: slider and scrollbar expose focused state visual feedback\n");
  printf("wing_gui_demo: value widget state style is staged as background fill before content and overlay stroke after content\n");
  printf("wing_gui_demo: value widget focused rings are stroke-only slider_fill=%s slider_stroke=%s slider_stroke_width=%u scrollbar_fill=%s scrollbar_stroke=%s scrollbar_stroke_width=%u\n",
         slider_focused_style.has_fill ? "yes" : "no",
         slider_focused_style.has_stroke ? "yes" : "no",
         (unsigned int)slider_focused_style.stroke_width,
         scrollbar_focused_style.has_fill ? "yes" : "no",
         scrollbar_focused_style.has_stroke ? "yes" : "no",
         (unsigned int)scrollbar_focused_style.stroke_width);
  printf("wing_gui_demo: demo owns widget geometry tuning for padding, knob, track and thumb sizes\n");
  printf("wing_gui_demo: slider step=%u scrollbar step=%u are widget properties\n",
         (unsigned int)wing_slider_get_step(&slider),
         (unsigned int)wing_scrollbar_get_step(&scrollbar));
  printf("wing_gui_demo: progress/slider/scrollbar share wing_value_model_t for range/value/step/input handling\n");
  printf("wing_gui_demo: reusable widget behaviors live under WING src/behaviors instead of core runtime\n");
  printf("wing_gui_demo: progress/slider/scrollbar share WING value input behavior for key and encoder step handling\n");
  printf("wing_gui_demo: progress/slider/scrollbar share WING pointer drag behavior for pressed/capture/update/release lifecycle\n");
  printf("wing_gui_demo: widget base dispatches value updates, invalidation and value changed events for numeric and boolean widgets\n");
  printf("wing_gui_demo: widget base owns state style storage and state-driven style selection\n");
  printf("wing_gui_demo: WING_EVENT_STATE_CHANGED reports object state bit transitions\n");
  printf("wing_gui_demo: WING_EVENT_SCROLL_CHANGED reports scroll view offset changes\n");
  printf("wing_gui_demo: WING_EVENT_CLOSE_REQUEST reports app/window close requests before runtime stops\n");
  printf("wing_gui_demo: selected state line_primary selected=%s active state fill_panel active=%s\n",
         wing_obj_is_selected(wing_box_obj(&line_primary)) ? "yes" : "no",
         wing_obj_is_active(wing_box_obj(&fill_panel)) ? "yes" : "no");
  printf("wing_gui_demo: wing_obj_set_enabled drives enabled flag, disabled state and disabled style selection\n");
  printf("wing_gui_demo: wing_obj_set_visible drives visible flag, hit/draw skip and dirty redraw\n");
  printf("wing_gui_demo: object lifecycle create/delete events support dynamic UI subtree attach and destroy\n");
  printf("wing_gui_demo: dirty system tracks union rect, dirty rect list count, merge count and redraw chunks\n");
  printf("wing_gui_demo: render command capacity fallback can collapse dirty-list redraw to union dirty redraw\n");
  printf("wing_gui_demo: render redraw cost policy can prefer union dirty redraw when it is cheaper than many chunks\n");
  printf("wing_gui_demo: framebuffer present consumes WING frame present rect list so dirty redraw can limit scanout updates\n");
  printf("wing_gui_demo: object clip-children uses nested WING/FRender clip stack for overflowing child content\n");
  printf("wing_gui_demo: scroll view widget reuses object clip-children and moves content children by scroll offset\n");
  printf("wing_gui_demo: scroll view consumes focused key/encoder input to update viewport offset\n");
  printf("wing_gui_demo: scroll view exposes scroll_by and max offset helpers so apps do not inspect internals\n");
  printf("wing_gui_demo: switch widget reuses WING value event payload for boolean state\n");
  printf("wing_gui_demo: checkbox widget reuses WING value event payload for checked state\n");
  printf("wing_gui_demo: switch and checkbox synchronize boolean values into WING_OBJ_STATE_CHECKED\n");
  wing_progress_get_range(&progress, &range_min, &range_max);
  printf("wing_gui_demo: progress getter value=%u range=%u..%u step=%u\n",
         (unsigned int)wing_progress_get_value(&progress),
         (unsigned int)range_min, (unsigned int)range_max,
         (unsigned int)wing_progress_get_step(&progress));
  wing_slider_get_range(&slider, &range_min, &range_max);
  printf("wing_gui_demo: slider getter value=%u range=%u..%u\n",
         (unsigned int)wing_slider_get_value(&slider),
         (unsigned int)range_min, (unsigned int)range_max);
  wing_scrollbar_get_range(&scrollbar, &range_min, &range_max);
  printf("wing_gui_demo: scrollbar getter value=%u range=%u..%u page=%u\n",
         (unsigned int)wing_scrollbar_get_value(&scrollbar),
         (unsigned int)range_min, (unsigned int)range_max,
         (unsigned int)wing_scrollbar_get_page_size(&scrollbar));
  printf("wing_gui_demo: path app loop -> wing_gui_handle -> tick/timer/animation -> layout fixed/stack/center/fill -> input queue -> focus/key/text/state/close-request -> event queue -> bubbled object event -> object tree -> wing_panel/wing_scroll_view/wing_button/wing_label/wing_text_input/wing_progress/wing_slider/wing_scrollbar/wing_switch/wing_checkbox/wing_card/custom-geometry-triangle -> wing_box state style -> FRender commands -> software backend");
  if (have_presenter)
    {
      printf(" -> framebuffer present\n");
      printf("wing_gui_demo: close the framebuffer window to exit demo\n");
    }
  else
    {
      printf("\n");
    }

  ret = 0;
  fill_layout_reported = false;
  headless_frames = 0;
  idle_reported = false;
  while (wing_gui_is_running(&gui))
    {
      if (have_presenter && fr_fb_presenter_window_closed(&presenter))
        {
          printf("wing_gui_demo: framebuffer window closed\n");
          ret = wing_gui_request_close(&gui);
          if (ret < 0)
            {
              printf("wing_gui_demo: close request enqueue failed: %d\n",
                     ret);
              break;
            }
        }

      ret = wing_gui_handle(&gui, WING_GUI_DEMO_FRAME_MS, &frame);
      if (ret < 0)
        {
          printf("wing_gui_demo: handler failed: %d\n", ret);
          break;
        }

      if (!wing_gui_is_running(&gui))
        {
          break;
        }

      if (!fill_layout_reported)
        {
          const wing_rect_t *fill_bounds;

          fill_bounds = wing_obj_get_bounds(wing_box_obj(&fill_badge));
          if (fill_bounds != NULL)
            {
              printf("wing_gui_demo: fill layout child bounds x=%d y=%d w=%u h=%u\n",
                     fill_bounds->x, fill_bounds->y,
                     (unsigned int)fill_bounds->w,
                     (unsigned int)fill_bounds->h);
            }

          fill_layout_reported = true;
        }

      if (ret == 0 && have_presenter)
        {
          if (!idle_reported)
            {
              printf("wing_gui_demo: idle frame no redraw; waiting for framebuffer window close\n");
              idle_reported = true;
            }

          usleep(WING_GUI_DEMO_FRAME_MS * 1000);
          continue;
        }

      idle_reported = false;

      if (WING_GUI_DEMO_TRACE_EACH_FRAME || frame.input_polled > 0)
        {
          wing_gui_demo_print_dirty_rect("before handler",
                                         frame.has_dirty_before,
                                         &frame.dirty_before);
      wing_gui_demo_print_dirty_count("before handler",
                                      frame.dirty_count_before);
      wing_gui_demo_print_dirty_merge_count(
        "before handler", frame.dirty_merge_count_before);
      printf("wing_gui_demo: handler input polled=%u\n",
             (unsigned int)frame.input_polled);
      wing_gui_demo_print_dirty_rect("after tick before step",
                                     frame.has_dirty_after_tick,
                                     &frame.dirty_after_tick);
      wing_gui_demo_print_dirty_count("after tick before step",
                                      frame.dirty_count_after_tick);
      wing_gui_demo_print_dirty_merge_count(
        "after tick before step", frame.dirty_merge_count_after_tick);
      wing_gui_demo_print_dirty_rect("after handler",
                                     frame.has_dirty_after_step,
                                     &frame.dirty_after_step);
      wing_gui_demo_print_dirty_count("after handler",
                                      frame.dirty_count_after_step);
      wing_gui_demo_print_dirty_merge_count(
        "after handler", frame.dirty_merge_count_after_step);
      printf("wing_gui_demo: redraw chunks this frame count=%u\n",
             (unsigned int)frame.redraw_count);
      printf("wing_gui_demo: render command capacity fallback=%s planned_chunks=%u actual_chunks=%u\n",
             frame.command_capacity_fallback ? "yes" : "no",
             (unsigned int)frame.planned_redraw_count,
             (unsigned int)frame.redraw_count);
      printf("wing_gui_demo: render redraw cost fallback=%s planned_chunks=%u actual_chunks=%u\n",
             frame.redraw_cost_fallback ? "yes" : "no",
             (unsigned int)frame.planned_redraw_count,
             (unsigned int)frame.redraw_count);
      wing_gui_demo_print_dirty_rect("present",
                                     frame.has_present_rect,
                                     &frame.present_rect);
      printf("wing_gui_demo: present rect list count=%u\n",
             (unsigned int)frame.present_rect_count);
          wing_gui_demo_print_dirty("runtime after handler", &gui);
        }

      if (ret > 0)
        {
          ret = wing_gui_flush_frender_software(&gui);
          if (ret < 0)
            {
              printf("wing_gui_demo: execute frame failed: %d\n", ret);
              break;
            }

          checksum = wing_surface_checksum_rgba8888(&surface);
          if (WING_GUI_DEMO_TRACE_EACH_FRAME || frame.input_polled > 0)
            {
              printf("wing_gui_demo: frame tick=%u commands=%u checksum=0x%08x\n",
                     (unsigned int)frame.tick_ms,
                     (unsigned int)wing_gui_render_command_count(&gui),
                     (unsigned int)checksum);
            }

          if (have_presenter)
            {
              ret = wing_gui_demo_present(&presenter, &surface, &frame);
              if (ret < 0)
                {
                  printf("wing_gui_demo: present failed: %d\n", ret);
                  break;
                }
            }
        }

      if (have_presenter)
        {
          usleep(WING_GUI_DEMO_FRAME_MS * 1000);
        }

      if (!have_presenter)
        {
          headless_frames++;
          if (headless_frames >= WING_GUI_DEMO_HEADLESS_FRAMES)
            {
              break;
            }

          continue;
        }

      if (fr_fb_presenter_window_closed(&presenter))
        {
          printf("wing_gui_demo: framebuffer window closed\n");
          ret = wing_gui_request_close(&gui);
          if (ret < 0)
            {
              printf("wing_gui_demo: close request enqueue failed: %d\n",
                     ret);
              break;
            }
        }

      usleep(WING_GUI_DEMO_FRAME_MS * 1000);
    }

  if (have_presenter)
    {
      fr_fb_presenter_close(&presenter);
    }

  printf("wing_gui_demo: app task exit\n");
  wing_gui_destroy(&gui);
  free(pixels);

  return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
