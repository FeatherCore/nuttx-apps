#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_UTIL_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef new0
#  define new0(type, n) ((type *)calloc((n), sizeof(type)))
#endif

static inline void *util_memdup(const void *src, size_t size)
{
  void *dst;

  if (src == NULL || size == 0)
    {
      return NULL;
    }

  dst = malloc(size);
  if (dst == NULL)
    {
      return NULL;
    }

  memcpy(dst, src, size);
  return dst;
}

static inline void *util_malloc(size_t size)
{
  return malloc(size);
}

static inline uint8_t util_get_uid(uint64_t *pool, uint8_t max)
{
  uint8_t uid;

  if (pool == NULL)
    {
      return 0;
    }

  for (uid = 1; uid <= max && uid < 64; uid++)
    {
      uint64_t bit = 1ull << uid;

      if ((*pool & bit) == 0)
        {
          *pool |= bit;
          return uid;
        }
    }

  return 0;
}

static inline void util_clear_uid(uint64_t *pool, uint8_t uid)
{
  if (pool != NULL && uid < 64)
    {
      *pool &= ~(1ull << uid);
    }
}

typedef void (*util_debug_func_t)(const char *str, void *user_data);

static inline void util_debug_va(util_debug_func_t function, void *user_data,
                                 const char *format, va_list ap)
{
  char buffer[256];

  if (function == NULL || format == NULL)
    {
      return;
    }

  vsnprintf(buffer, sizeof(buffer), format, ap);
  function(buffer, user_data);
}
#endif
