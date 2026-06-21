#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SDPD_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SDPD_H

#include <stdint.h>
#include "bluetooth/sdp.h"

static inline uint32_t add_record_to_server(const void *src,
                                            sdp_record_t *record)
{
  (void)src;
  (void)record;
  return 1;
}

static inline int remove_record_from_server(uint32_t handle)
{
  (void)handle;
  return 0;
}

#endif
