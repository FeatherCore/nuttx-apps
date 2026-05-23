/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __FEATHER_CMSIS_COMPILER_H__
#define __FEATHER_CMSIS_COMPILER_H__

#include <stdint.h>

#if defined(__arm__) || defined(__thumb__)
#  include <arm_acle.h>
#endif

#ifndef __IS_COMPILER_ARM_COMPILER_5__
#  define __IS_COMPILER_ARM_COMPILER_5__ 0
#endif

#ifndef __IS_COMPILER_IAR__
#  define __IS_COMPILER_IAR__ 0
#endif

#ifndef __IS_COMPILER_GCC__
#  define __IS_COMPILER_GCC__ 1
#endif

#ifndef __ASM
#  define __ASM __asm
#endif

#ifndef __INLINE
#  define __INLINE inline
#endif

#ifndef __STATIC_INLINE
#  define __STATIC_INLINE static inline
#endif

#ifndef __STATIC_FORCEINLINE
#  define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline
#endif

#ifndef __NO_RETURN
#  define __NO_RETURN __attribute__((__noreturn__))
#endif

#ifndef __USED
#  define __USED __attribute__((used))
#endif

#ifndef __WEAK
#  define __WEAK __attribute__((weak))
#endif

#ifndef __PACKED
#  define __PACKED __attribute__((packed, aligned(1)))
#endif

#ifndef __PACKED_STRUCT
#  define __PACKED_STRUCT struct __attribute__((packed, aligned(1)))
#endif

#ifndef __PACKED_UNION
#  define __PACKED_UNION union __attribute__((packed, aligned(1)))
#endif

#ifndef __ALIGNED
#  define __ALIGNED(x) __attribute__((aligned(x)))
#endif

#ifndef __RESTRICT
#  define __RESTRICT __restrict
#endif

#ifndef __COMPILER_BARRIER
#  define __COMPILER_BARRIER() __ASM volatile("" ::: "memory")
#endif

#ifndef __UNALIGNED_UINT32
struct __attribute__((packed)) __feather_unaligned_uint32_s
{
  uint32_t v;
};
#  define __UNALIGNED_UINT32(x) \
    (((struct __feather_unaligned_uint32_s *)(x))->v)
#endif

#ifndef __UNALIGNED_UINT16_WRITE
__PACKED_STRUCT __feather_unaligned_uint16_write_s
{
  uint16_t v;
};
#  define __UNALIGNED_UINT16_WRITE(addr, val) \
    (void)((((struct __feather_unaligned_uint16_write_s *)(void *)(addr))->v) = (val))
#endif

#ifndef __UNALIGNED_UINT16_READ
__PACKED_STRUCT __feather_unaligned_uint16_read_s
{
  uint16_t v;
};
#  define __UNALIGNED_UINT16_READ(addr) \
    (((const struct __feather_unaligned_uint16_read_s *)(const void *)(addr))->v)
#endif

#ifndef __UNALIGNED_UINT32_WRITE
__PACKED_STRUCT __feather_unaligned_uint32_write_s
{
  uint32_t v;
};
#  define __UNALIGNED_UINT32_WRITE(addr, val) \
    (void)((((struct __feather_unaligned_uint32_write_s *)(void *)(addr))->v) = (val))
#endif

#ifndef __UNALIGNED_UINT32_READ
__PACKED_STRUCT __feather_unaligned_uint32_read_s
{
  uint32_t v;
};
#  define __UNALIGNED_UINT32_READ(addr) \
    (((const struct __feather_unaligned_uint32_read_s *)(const void *)(addr))->v)
#endif

__STATIC_FORCEINLINE uint32_t __REV(uint32_t value)
{
  return __builtin_bswap32(value);
}

__STATIC_FORCEINLINE uint32_t __REV16(uint32_t value)
{
  return ((value & 0x00ff00ffu) << 8) | ((value & 0xff00ff00u) >> 8);
}

#ifndef __clz
#  define __clz(value) __builtin_clz(value)
#endif

#if defined(__arm__) || defined(__thumb__)
#  ifndef __QADD
#    define __QADD(op1, op2) __qadd((op1), (op2))
#  endif

#  ifndef __QSUB
#    define __QSUB(op1, op2) __qsub((op1), (op2))
#  endif

#  ifndef __SMUAD
#    define __SMUAD(op1, op2) __smuad((op1), (op2))
#  endif

#  ifndef __SMUADX
#    define __SMUADX(op1, op2) __smuadx((op1), (op2))
#  endif

#  ifndef __SMLALD
#    define __SMLALD(op1, op2, acc) __smlald((op1), (op2), (acc))
#  endif

#  ifndef __SMLALDX
#    define __SMLALDX(op1, op2, acc) __smlaldx((op1), (op2), (acc))
#  endif
#endif

#endif /* __FEATHER_CMSIS_COMPILER_H__ */
