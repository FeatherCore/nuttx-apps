/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __FEATHER_ARM_MATH_F16_H__
#define __FEATHER_ARM_MATH_F16_H__

#include "arm_math.h"

#if defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE
typedef union _any16x8_t
{
  float16x8_t f;
  int16x8_t i;
} any16x8_t;
#endif

#endif /* __FEATHER_ARM_MATH_F16_H__ */
