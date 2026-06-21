/****************************************************************************
 * apps/graphics/wing/include/wing/core/wing_animation.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#ifndef __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_ANIMATION_H
#define __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_ANIMATION_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct wing_gui_s;

typedef struct wing_gui_s wing_gui_t;
typedef void (*wing_anim_apply_cb_t)(wing_gui_t *gui, int32_t value,
                                     void *arg);
typedef void (*wing_anim_done_cb_t)(wing_gui_t *gui, void *arg);

enum wing_anim_path_e
{
  WING_ANIM_PATH_LINEAR = 0,
  WING_ANIM_PATH_EASE_IN,
  WING_ANIM_PATH_EASE_OUT,
  WING_ANIM_PATH_EASE_IN_OUT
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int wing_gui_anim_start(wing_gui_t *gui, int32_t start_value,
                        int32_t end_value, uint32_t duration_ms,
                        wing_anim_apply_cb_t apply,
                        wing_anim_done_cb_t done, void *arg,
                        uint8_t *anim_id);
int wing_gui_anim_start_path(wing_gui_t *gui, int32_t start_value,
                             int32_t end_value, uint32_t duration_ms,
                             enum wing_anim_path_e path,
                             wing_anim_apply_cb_t apply,
                             wing_anim_done_cb_t done, void *arg,
                             uint8_t *anim_id);
int wing_gui_anim_stop(wing_gui_t *gui, uint8_t anim_id);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_GRAPHICS_WING_INCLUDE_WING_CORE_WING_ANIMATION_H */
