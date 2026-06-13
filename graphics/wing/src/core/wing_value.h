/****************************************************************************
 * apps/graphics/wing/src/core/wing_value.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_SRC_CORE_WING_VALUE_H
#define __APPS_GRAPHICS_WING_SRC_CORE_WING_VALUE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include <wing/wing.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void wing_value_normalize_range(uint16_t *min, uint16_t *max);
uint16_t wing_value_clamp(uint16_t min, uint16_t max, uint16_t value);
void wing_value_init_storage(uint16_t *stored_min, uint16_t *stored_max,
                             uint16_t *stored_value, uint16_t min,
                             uint16_t max, uint16_t value);
void wing_value_model_init(wing_value_model_t *model, uint16_t min,
                           uint16_t max, uint16_t value, uint16_t step);
void wing_value_update_range(uint16_t *stored_min, uint16_t *stored_max,
                             uint16_t *stored_value, uint16_t min,
                             uint16_t max);
void wing_value_model_set_range(wing_value_model_t *model, uint16_t min,
                                uint16_t max);
bool wing_value_update(uint16_t min, uint16_t max, uint16_t *stored_value,
                       uint16_t value, wing_value_event_t *payload);
bool wing_value_model_set_value(wing_value_model_t *model, uint16_t value,
                                wing_value_event_t *payload);
bool wing_value_update_bool(bool *stored_value, bool value,
                            wing_value_event_t *payload);
void wing_value_model_set_step(wing_value_model_t *model, uint16_t step);
uint16_t wing_value_step(uint16_t min, uint16_t max, uint16_t value,
                         uint16_t step, bool increase);
uint16_t wing_value_model_step_value(const wing_value_model_t *model,
                                     bool increase);
uint16_t wing_value_to_offset(uint16_t min, uint16_t max, uint16_t value,
                              uint16_t width);
uint16_t wing_value_model_to_offset(const wing_value_model_t *model,
                                    uint16_t width);
uint16_t wing_value_from_offset(uint16_t min, uint16_t max, uint16_t offset,
                                uint16_t width);
uint16_t wing_value_model_from_offset(const wing_value_model_t *model,
                                      uint16_t offset, uint16_t width);

#endif /* __APPS_GRAPHICS_WING_SRC_CORE_WING_VALUE_H */
