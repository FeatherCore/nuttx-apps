/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_input.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_INPUT_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_INPUT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

void wing_gui_set_input_reader(wing_gui_t *gui,
                               wing_input_read_fn_t input_read, void *arg);
void wing_input_adapter_init(wing_input_adapter_t *adapter);
int wing_input_adapter_take_pending(wing_input_adapter_t *adapter,
                                    wing_input_event_t *input);
int wing_input_adapter_store_pending(wing_input_adapter_t *adapter,
                                     const wing_input_event_t *input);
bool wing_input_adapter_merge_pointer_move(wing_input_adapter_t *adapter,
                                           wing_input_event_t *current,
                                           const wing_input_event_t *next);
uint16_t wing_input_adapter_get_coalesced_moves(
  const wing_input_adapter_t *adapter);
int wing_gui_poll_input(wing_gui_t *gui, uint8_t max_events);
int wing_gui_enqueue_input(wing_gui_t *gui, const wing_input_event_t *input);
int wing_gui_request_close(wing_gui_t *gui);
int wing_gui_dispatch_input(wing_gui_t *gui, const wing_input_event_t *input);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_INPUT_H */
