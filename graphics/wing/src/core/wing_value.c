/****************************************************************************
 * apps/graphics/wing/src/core/wing_value.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "wing_value.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void wing_value_normalize_range(uint16_t *min, uint16_t *max)
{
  if (min == 0 || max == 0 || *max > *min)
    {
      return;
    }

  if (*min == UINT16_MAX)
    {
      *min = UINT16_MAX - 1;
      *max = UINT16_MAX;
    }
  else
    {
      *max = (uint16_t)(*min + 1);
    }
}

uint16_t wing_value_clamp(uint16_t min, uint16_t max, uint16_t value)
{
  wing_value_normalize_range(&min, &max);

  if (value < min)
    {
      return min;
    }

  if (value > max)
    {
      return max;
    }

  return value;
}

void wing_value_init_storage(uint16_t *stored_min, uint16_t *stored_max,
                             uint16_t *stored_value, uint16_t min,
                             uint16_t max, uint16_t value)
{
  if (stored_min == 0 || stored_max == 0 || stored_value == 0)
    {
      return;
    }

  wing_value_normalize_range(&min, &max);
  *stored_min = min;
  *stored_max = max;
  *stored_value = wing_value_clamp(min, max, value);
}

void wing_value_model_init(wing_value_model_t *model, uint16_t min,
                           uint16_t max, uint16_t value, uint16_t step)
{
  if (model == 0)
    {
      return;
    }

  wing_value_init_storage(&model->min, &model->max, &model->value,
                          min, max, value);
  wing_value_model_set_step(model, step);
}

void wing_value_update_range(uint16_t *stored_min, uint16_t *stored_max,
                             uint16_t *stored_value, uint16_t min,
                             uint16_t max)
{
  if (stored_min == 0 || stored_max == 0 || stored_value == 0)
    {
      return;
    }

  wing_value_normalize_range(&min, &max);
  *stored_min = min;
  *stored_max = max;
  *stored_value = wing_value_clamp(min, max, *stored_value);
}

void wing_value_model_set_range(wing_value_model_t *model, uint16_t min,
                                uint16_t max)
{
  if (model == 0)
    {
      return;
    }

  wing_value_update_range(&model->min, &model->max, &model->value,
                          min, max);
}

bool wing_value_update(uint16_t min, uint16_t max, uint16_t *stored_value,
                       uint16_t value, wing_value_event_t *payload)
{
  uint16_t next;
  uint16_t old;

  if (stored_value == 0)
    {
      return false;
    }

  next = wing_value_clamp(min, max, value);
  if (next == *stored_value)
    {
      return false;
    }

  old = *stored_value;
  *stored_value = next;
  if (payload != 0)
    {
      payload->old_value = old;
      payload->value = next;
      payload->min = min;
      payload->max = max;
    }

  return true;
}

bool wing_value_model_set_value(wing_value_model_t *model, uint16_t value,
                                wing_value_event_t *payload)
{
  if (model == 0)
    {
      return false;
    }

  return wing_value_update(model->min, model->max, &model->value,
                           value, payload);
}

bool wing_value_update_bool(bool *stored_value, bool value,
                            wing_value_event_t *payload)
{
  bool old;

  if (stored_value == 0 || *stored_value == value)
    {
      return false;
    }

  old = *stored_value;
  *stored_value = value;
  if (payload != 0)
    {
      payload->old_value = old ? 1 : 0;
      payload->value = value ? 1 : 0;
      payload->min = 0;
      payload->max = 1;
    }

  return true;
}

void wing_value_model_set_step(wing_value_model_t *model, uint16_t step)
{
  if (model == 0)
    {
      return;
    }

  model->step = step == 0 ? 1 : step;
}

uint16_t wing_value_step(uint16_t min, uint16_t max, uint16_t value,
                         uint16_t step, bool increase)
{
  uint32_t next;

  wing_value_normalize_range(&min, &max);
  value = wing_value_clamp(min, max, value);
  if (step == 0)
    {
      step = 1;
    }

  if (increase)
    {
      next = (uint32_t)value + step;
      return next > max ? max : (uint16_t)next;
    }

  if (value <= min || (uint32_t)(value - min) <= step)
    {
      return min;
    }

  return (uint16_t)(value - step);
}

uint16_t wing_value_model_step_value(const wing_value_model_t *model,
                                     bool increase)
{
  if (model == 0)
    {
      return 0;
    }

  return wing_value_step(model->min, model->max, model->value,
                         model->step, increase);
}

uint16_t wing_value_to_offset(uint16_t min, uint16_t max, uint16_t value,
                              uint16_t width)
{
  uint32_t denom;
  uint32_t normalized;

  wing_value_normalize_range(&min, &max);
  value = wing_value_clamp(min, max, value);

  denom = (uint32_t)(max - min);
  if (denom == 0)
    {
      denom = 1;
    }

  normalized = (uint32_t)(value - min);
  return (uint16_t)((uint32_t)width * normalized / denom);
}

uint16_t wing_value_model_to_offset(const wing_value_model_t *model,
                                    uint16_t width)
{
  if (model == 0)
    {
      return 0;
    }

  return wing_value_to_offset(model->min, model->max, model->value, width);
}

uint16_t wing_value_from_offset(uint16_t min, uint16_t max, uint16_t offset,
                                uint16_t width)
{
  uint32_t range;

  wing_value_normalize_range(&min, &max);
  if (width == 0)
    {
      return min;
    }

  if (offset > width)
    {
      offset = width;
    }

  range = (uint32_t)(max - min);
  return (uint16_t)(min + range * offset / width);
}

uint16_t wing_value_model_from_offset(const wing_value_model_t *model,
                                      uint16_t offset, uint16_t width)
{
  if (model == 0)
    {
      return 0;
    }

  return wing_value_from_offset(model->min, model->max, offset, width);
}
