/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __FEATHER_ARM_2D_CFG_H__
#define __FEATHER_ARM_2D_CFG_H__

/* STM32N6570-DK uses Arm-2D as a synchronous CPU/Helium backend for LVGL. */
#ifndef __ARM_2D_HAS_ASYNC__
#  define __ARM_2D_HAS_ASYNC__ 0
#endif

/* Keep the LVGL path on the fastest Arm-2D transform kernels by default. */
#ifndef __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__
#  define __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__ 0
#endif

#ifndef __ARM_2D_CFG_UNSAFE_IGNORE_CALIB_IN_TRANSFORM__
#  define __ARM_2D_CFG_UNSAFE_IGNORE_CALIB_IN_TRANSFORM__
#endif

#include "../arm-2d/Library/Include/template/arm_2d_cfg.h"

#endif /* __FEATHER_ARM_2D_CFG_H__ */
