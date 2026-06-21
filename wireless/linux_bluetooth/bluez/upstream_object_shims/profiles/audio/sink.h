#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SINK_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SINK_H

#include "glib.h"

struct avdtp;
struct btd_service;

static inline gboolean sink_setup_stream(struct btd_service *service,
                                         struct avdtp *session)
{
  (void)service;
  (void)session;
  return TRUE;
}

#endif
