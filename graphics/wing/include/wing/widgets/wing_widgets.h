/****************************************************************************
 * apps/graphics/wing/include/wing/widgets/wing_widgets.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_WIDGETS_WING_WIDGETS_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_WIDGETS_WING_WIDGETS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

void wing_box_style_init(wing_box_style_t *style);
void wing_box_init(wing_box_t *box, const wing_rect_t *bounds,
                   const wing_box_style_t *style);
wing_obj_t *wing_box_obj(wing_box_t *box);
const wing_obj_t *wing_box_const_obj(const wing_box_t *box);
void wing_box_set_style(wing_box_t *box, const wing_box_style_t *style);
void wing_box_set_state_style(wing_box_t *box, uint16_t state,
                              const wing_box_style_t *style);
const wing_box_style_t *wing_box_get_style(const wing_box_t *box);
const wing_box_style_t *wing_box_get_active_style(const wing_box_t *box);

void wing_button_init(wing_button_t *button, const wing_rect_t *bounds,
                      const wing_box_style_t *style);
wing_obj_t *wing_button_obj(wing_button_t *button);
wing_box_t *wing_button_box(wing_button_t *button);
void wing_button_set_style(wing_button_t *button,
                           const wing_box_style_t *style);
void wing_button_set_state_style(wing_button_t *button, uint16_t state,
                                 const wing_box_style_t *style);
void wing_button_set_event_cb(wing_button_t *button,
                              wing_button_event_fn_t event, void *arg);

void wing_label_init(wing_label_t *label, const wing_rect_t *bounds,
                     const char *text, wing_color_t color, uint8_t scale);
wing_obj_t *wing_label_obj(wing_label_t *label);
void wing_label_set_text(wing_label_t *label, const char *text);
void wing_label_set_color(wing_label_t *label, wing_color_t color);
void wing_label_set_scale(wing_label_t *label, uint8_t scale);
void wing_label_set_font(wing_label_t *label, const wing_font_t *font);
const wing_font_t *wing_label_get_font(const wing_label_t *label);
void wing_label_set_align(wing_label_t *label, enum wing_text_align_e align);
enum wing_text_align_e wing_label_get_align(const wing_label_t *label);
void wing_label_set_text_mode(wing_label_t *label,
                              enum wing_label_text_mode_e mode);
enum wing_label_text_mode_e wing_label_get_text_mode(
  const wing_label_t *label);
int wing_label_get_text_size(const wing_label_t *label, uint16_t *width,
                             uint16_t *height);
int wing_label_get_layout_size(const wing_label_t *label, uint16_t *width,
                               uint16_t *height);

void wing_image_init(wing_image_t *image, const wing_rect_t *bounds,
                     const wing_color_t *pixels, uint16_t width,
                     uint16_t height, uint16_t stride, uint8_t scale);
void wing_image_init_resource(wing_image_t *image, const wing_rect_t *bounds,
                              const wing_image_resource_t *resource,
                              uint8_t scale);
wing_obj_t *wing_image_obj(wing_image_t *image);
void wing_image_set_source(wing_image_t *image,
                           const wing_color_t *pixels, uint16_t width,
                           uint16_t height, uint16_t stride);
void wing_image_set_resource(wing_image_t *image,
                             const wing_image_resource_t *resource);
void wing_image_set_scale(wing_image_t *image, uint8_t scale);

void wing_panel_init(wing_panel_t *panel, const wing_rect_t *bounds,
                     const wing_box_style_t *style);
wing_obj_t *wing_panel_obj(wing_panel_t *panel);
wing_box_t *wing_panel_box(wing_panel_t *panel);
void wing_panel_set_style(wing_panel_t *panel,
                          const wing_box_style_t *style);
void wing_panel_set_state_style(wing_panel_t *panel, uint16_t state,
                                const wing_box_style_t *style);
void wing_panel_set_layout(wing_panel_t *panel,
                           enum wing_layout_type_e layout,
                           uint8_t padding, uint8_t spacing);

void wing_scroll_view_init(wing_scroll_view_t *view,
                           const wing_rect_t *bounds,
                           const wing_box_style_t *style);
wing_obj_t *wing_scroll_view_obj(wing_scroll_view_t *view);
wing_box_t *wing_scroll_view_box(wing_scroll_view_t *view);
void wing_scroll_view_set_style(wing_scroll_view_t *view,
                                const wing_box_style_t *style);
void wing_scroll_view_set_state_style(wing_scroll_view_t *view,
                                      uint16_t state,
                                      const wing_box_style_t *style);
void wing_scroll_view_set_offset(wing_scroll_view_t *view,
                                 int16_t offset_x, int16_t offset_y);
void wing_scroll_view_scroll_by(wing_scroll_view_t *view,
                                int16_t delta_x, int16_t delta_y);
void wing_scroll_view_get_offset(const wing_scroll_view_t *view,
                                 int16_t *offset_x, int16_t *offset_y);
void wing_scroll_view_get_max_offset(const wing_scroll_view_t *view,
                                     int16_t *max_offset_x,
                                     int16_t *max_offset_y);
void wing_scroll_view_set_content_size(wing_scroll_view_t *view,
                                       uint16_t width, uint16_t height);
void wing_scroll_view_get_content_size(const wing_scroll_view_t *view,
                                       uint16_t *width, uint16_t *height);
void wing_scroll_view_set_step(wing_scroll_view_t *view,
                               uint16_t step_x, uint16_t step_y);
void wing_scroll_view_get_step(const wing_scroll_view_t *view,
                               uint16_t *step_x, uint16_t *step_y);
void wing_scroll_view_set_event_cb(wing_scroll_view_t *view,
                                   wing_scroll_view_event_fn_t event,
                                   void *arg);

void wing_card_init(wing_card_t *card, const wing_rect_t *bounds,
                       const wing_box_style_t *front_style,
                       const wing_box_style_t *back_style,
                       const wing_box_style_t *edge_style);
wing_obj_t *wing_card_obj(wing_card_t *card);
void wing_card_set_front_style(wing_card_t *card,
                                  const wing_box_style_t *style);
void wing_card_set_back_style(wing_card_t *card,
                                 const wing_box_style_t *style);
void wing_card_set_edge_style(wing_card_t *card,
                                 const wing_box_style_t *style);

void wing_progress_init(wing_progress_t *progress,
                        const wing_rect_t *bounds,
                        const wing_box_style_t *frame_style,
                        const wing_box_style_t *fill_style,
                        uint16_t min, uint16_t max, uint16_t value);
wing_obj_t *wing_progress_obj(wing_progress_t *progress);
void wing_progress_set_value(wing_progress_t *progress, uint16_t value);
uint16_t wing_progress_get_value(const wing_progress_t *progress);
void wing_progress_get_range(const wing_progress_t *progress,
                             uint16_t *min, uint16_t *max);
void wing_progress_set_range(wing_progress_t *progress,
                             uint16_t min, uint16_t max);
void wing_progress_set_step(wing_progress_t *progress, uint16_t step);
uint16_t wing_progress_get_step(const wing_progress_t *progress);
void wing_progress_set_frame_style(wing_progress_t *progress,
                                   const wing_box_style_t *style);
void wing_progress_set_fill_style(wing_progress_t *progress,
                                  const wing_box_style_t *style);
void wing_progress_set_padding(wing_progress_t *progress, uint8_t padding);
void wing_progress_set_event_cb(wing_progress_t *progress,
                                wing_progress_event_fn_t event, void *arg);

void wing_slider_init(wing_slider_t *slider,
                      const wing_rect_t *bounds,
                      const wing_box_style_t *track_style,
                      const wing_box_style_t *fill_style,
                      const wing_box_style_t *knob_style,
                      uint16_t min, uint16_t max, uint16_t value);
wing_obj_t *wing_slider_obj(wing_slider_t *slider);
void wing_slider_set_value(wing_slider_t *slider, uint16_t value);
uint16_t wing_slider_get_value(const wing_slider_t *slider);
void wing_slider_get_range(const wing_slider_t *slider,
                           uint16_t *min, uint16_t *max);
void wing_slider_set_range(wing_slider_t *slider,
                           uint16_t min, uint16_t max);
void wing_slider_set_step(wing_slider_t *slider, uint16_t step);
uint16_t wing_slider_get_step(const wing_slider_t *slider);
void wing_slider_set_track_style(wing_slider_t *slider,
                                 const wing_box_style_t *style);
void wing_slider_set_fill_style(wing_slider_t *slider,
                                const wing_box_style_t *style);
void wing_slider_set_knob_style(wing_slider_t *slider,
                                const wing_box_style_t *style);
void wing_slider_set_state_style(wing_slider_t *slider, uint16_t state,
                                 const wing_box_style_t *style);
void wing_slider_set_padding(wing_slider_t *slider, uint8_t padding);
void wing_slider_set_knob_size(wing_slider_t *slider, uint8_t knob_size);
void wing_slider_set_track_height(wing_slider_t *slider,
                                  uint8_t track_height);
void wing_slider_set_event_cb(wing_slider_t *slider,
                              wing_slider_event_fn_t event, void *arg);

void wing_scrollbar_init(wing_scrollbar_t *scrollbar,
                         const wing_rect_t *bounds,
                         const wing_box_style_t *track_style,
                         const wing_box_style_t *thumb_style,
                         uint16_t min, uint16_t max, uint16_t value,
                         uint16_t page_size);
wing_obj_t *wing_scrollbar_obj(wing_scrollbar_t *scrollbar);
void wing_scrollbar_set_value(wing_scrollbar_t *scrollbar, uint16_t value);
uint16_t wing_scrollbar_get_value(const wing_scrollbar_t *scrollbar);
void wing_scrollbar_get_range(const wing_scrollbar_t *scrollbar,
                              uint16_t *min, uint16_t *max);
void wing_scrollbar_set_range(wing_scrollbar_t *scrollbar,
                              uint16_t min, uint16_t max);
void wing_scrollbar_set_page_size(wing_scrollbar_t *scrollbar,
                                  uint16_t page_size);
uint16_t wing_scrollbar_get_page_size(const wing_scrollbar_t *scrollbar);
void wing_scrollbar_set_step(wing_scrollbar_t *scrollbar, uint16_t step);
uint16_t wing_scrollbar_get_step(const wing_scrollbar_t *scrollbar);
void wing_scrollbar_set_track_style(wing_scrollbar_t *scrollbar,
                                    const wing_box_style_t *style);
void wing_scrollbar_set_thumb_style(wing_scrollbar_t *scrollbar,
                                    const wing_box_style_t *style);
void wing_scrollbar_set_state_style(wing_scrollbar_t *scrollbar,
                                    uint16_t state,
                                    const wing_box_style_t *style);
void wing_scrollbar_set_padding(wing_scrollbar_t *scrollbar,
                                uint8_t padding);
void wing_scrollbar_set_min_thumb_length(wing_scrollbar_t *scrollbar,
                                         uint8_t min_thumb_length);
void wing_scrollbar_set_axis(wing_scrollbar_t *scrollbar,
                             enum wing_axis_e axis);
void wing_scrollbar_set_event_cb(wing_scrollbar_t *scrollbar,
                                 wing_scrollbar_event_fn_t event,
                                 void *arg);

void wing_switch_init(wing_switch_t *sw, const wing_rect_t *bounds,
                      const wing_box_style_t *off_style,
                      const wing_box_style_t *on_style,
                      const wing_box_style_t *knob_style, bool checked);
wing_obj_t *wing_switch_obj(wing_switch_t *sw);
void wing_switch_set_checked(wing_switch_t *sw, bool checked);
bool wing_switch_get_checked(const wing_switch_t *sw);
void wing_switch_set_off_style(wing_switch_t *sw,
                               const wing_box_style_t *style);
void wing_switch_set_on_style(wing_switch_t *sw,
                              const wing_box_style_t *style);
void wing_switch_set_knob_style(wing_switch_t *sw,
                                const wing_box_style_t *style);
void wing_switch_set_padding(wing_switch_t *sw, uint8_t padding);
void wing_switch_set_knob_size(wing_switch_t *sw, uint8_t knob_size);
void wing_switch_set_event_cb(wing_switch_t *sw,
                              wing_switch_event_fn_t event, void *arg);

void wing_checkbox_init(wing_checkbox_t *checkbox,
                        const wing_rect_t *bounds,
                        const wing_box_style_t *box_style,
                        const wing_box_style_t *checked_style,
                        const wing_box_style_t *mark_style,
                        bool checked);
wing_obj_t *wing_checkbox_obj(wing_checkbox_t *checkbox);
void wing_checkbox_set_checked(wing_checkbox_t *checkbox, bool checked);
bool wing_checkbox_get_checked(const wing_checkbox_t *checkbox);
void wing_checkbox_set_box_style(wing_checkbox_t *checkbox,
                                 const wing_box_style_t *style);
void wing_checkbox_set_checked_style(wing_checkbox_t *checkbox,
                                     const wing_box_style_t *style);
void wing_checkbox_set_mark_style(wing_checkbox_t *checkbox,
                                  const wing_box_style_t *style);
void wing_checkbox_set_padding(wing_checkbox_t *checkbox, uint8_t padding);
void wing_checkbox_set_event_cb(wing_checkbox_t *checkbox,
                                wing_checkbox_event_fn_t event, void *arg);

void wing_text_input_init(wing_text_input_t *input,
                          const wing_rect_t *bounds,
                          const wing_box_style_t *style,
                          const wing_box_style_t *cursor_style,
                          char *buffer, uint16_t capacity);
wing_obj_t *wing_text_input_obj(wing_text_input_t *input);
const char *wing_text_input_get_text(const wing_text_input_t *input);
uint16_t wing_text_input_get_cursor(const wing_text_input_t *input);
bool wing_text_input_has_selection(const wing_text_input_t *input);
void wing_text_input_get_selection(const wing_text_input_t *input,
                                   uint16_t *start, uint16_t *end);
void wing_text_input_set_text(wing_text_input_t *input, const char *text);
void wing_text_input_set_selection(wing_text_input_t *input,
                                   uint16_t start, uint16_t end);
void wing_text_input_select_all(wing_text_input_t *input);
void wing_text_input_set_selection_style(wing_text_input_t *input,
                                         const wing_box_style_t *style);
void wing_text_input_set_padding(wing_text_input_t *input, uint8_t padding);
void wing_text_input_set_event_cb(wing_text_input_t *input,
                                  wing_text_input_event_fn_t event,
                                  void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_WIDGETS_WING_WIDGETS_H */
