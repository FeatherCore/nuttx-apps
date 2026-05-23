/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __FEATHER_ARM2D_GCC_MVE_COMPAT_H__
#define __FEATHER_ARM2D_GCC_MVE_COMPAT_H__

#if defined(__GNUC__) && defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE

#include <stdint.h>
#include <arm_mve.h>

static inline uint8x16_t __feather_vmulq_u8_vec(uint8x16_t a, uint8x16_t b)
{
  return __arm_vmulq(a, b);
}

static inline uint8x16_t __feather_vmulq_u8_scalar(uint8x16_t a, uint32_t b)
{
  return __arm_vmulq(a, (uint8_t)b);
}

static inline uint16x8_t __feather_vmulq_u16_vec(uint16x8_t a, uint16x8_t b)
{
  return __arm_vmulq(a, b);
}

static inline uint16x8_t __feather_vmulq_u16_scalar(uint16x8_t a, uint32_t b)
{
  return __arm_vmulq(a, (uint16_t)b);
}

static inline uint32x4_t __feather_vmulq_u32_vec(uint32x4_t a, uint32x4_t b)
{
  return __arm_vmulq(a, b);
}

static inline uint32x4_t __feather_vmulq_u32_scalar(uint32x4_t a, uint32_t b)
{
  return __arm_vmulq(a, b);
}

static inline int8x16_t __feather_vmulq_s8_vec(int8x16_t a, int8x16_t b)
{
  return __arm_vmulq(a, b);
}

static inline int8x16_t __feather_vmulq_s8_scalar(int8x16_t a, int32_t b)
{
  return __arm_vmulq(a, (int8_t)b);
}

static inline int16x8_t __feather_vmulq_s16_vec(int16x8_t a, int16x8_t b)
{
  return __arm_vmulq(a, b);
}

static inline int16x8_t __feather_vmulq_s16_scalar(int16x8_t a, int32_t b)
{
  return __arm_vmulq(a, (int16_t)b);
}

static inline int32x4_t __feather_vmulq_s32_vec(int32x4_t a, int32x4_t b)
{
  return __arm_vmulq(a, b);
}

static inline int32x4_t __feather_vmulq_s32_scalar(int32x4_t a, int32_t b)
{
  return __arm_vmulq(a, b);
}

#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 2)
static inline float16x8_t __feather_vmulq_f16_vec(float16x8_t a, float16x8_t b)
{
  return __arm_vmulq(a, b);
}

static inline float16x8_t __feather_vmulq_f16_scalar(float16x8_t a, float16_t b)
{
  return __arm_vmulq(a, b);
}

static inline float32x4_t __feather_vmulq_f32_vec(float32x4_t a, float32x4_t b)
{
  return __arm_vmulq(a, b);
}

static inline float32x4_t __feather_vmulq_f32_scalar(float32x4_t a, float b)
{
  return __arm_vmulq(a, b);
}
#endif

#undef vmulq
#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 2)
#  define vmulq(a, b) \
    _Generic((b), \
      uint8x16_t: __feather_vmulq_u8_vec, \
      uint16x8_t: __feather_vmulq_u16_vec, \
      uint32x4_t: __feather_vmulq_u32_vec, \
      int8x16_t: __feather_vmulq_s8_vec, \
      int16x8_t: __feather_vmulq_s16_vec, \
      int32x4_t: __feather_vmulq_s32_vec, \
      float16x8_t: __feather_vmulq_f16_vec, \
      float32x4_t: __feather_vmulq_f32_vec, \
      default: _Generic((a), \
        uint8x16_t: __feather_vmulq_u8_scalar, \
        uint16x8_t: __feather_vmulq_u16_scalar, \
        uint32x4_t: __feather_vmulq_u32_scalar, \
        int8x16_t: __feather_vmulq_s8_scalar, \
        int16x8_t: __feather_vmulq_s16_scalar, \
        int32x4_t: __feather_vmulq_s32_scalar, \
        float16x8_t: __feather_vmulq_f16_scalar, \
        float32x4_t: __feather_vmulq_f32_scalar))((a), (b))
#else
#  define vmulq(a, b) \
    _Generic((b), \
      uint8x16_t: __feather_vmulq_u8_vec, \
      uint16x8_t: __feather_vmulq_u16_vec, \
      uint32x4_t: __feather_vmulq_u32_vec, \
      int8x16_t: __feather_vmulq_s8_vec, \
      int16x8_t: __feather_vmulq_s16_vec, \
      int32x4_t: __feather_vmulq_s32_vec, \
      default: _Generic((a), \
        uint8x16_t: __feather_vmulq_u8_scalar, \
        uint16x8_t: __feather_vmulq_u16_scalar, \
        uint32x4_t: __feather_vmulq_u32_scalar, \
        int8x16_t: __feather_vmulq_s8_scalar, \
        int16x8_t: __feather_vmulq_s16_scalar, \
        int32x4_t: __feather_vmulq_s32_scalar))((a), (b))
#endif

#endif /* GCC + MVE */

#endif /* __FEATHER_ARM2D_GCC_MVE_COMPAT_H__ */
