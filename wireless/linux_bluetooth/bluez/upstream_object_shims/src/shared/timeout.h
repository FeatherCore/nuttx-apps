#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_TIMEOUT_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_TIMEOUT_H

#include "glib.h"

typedef gboolean (*timeout_func_t)(gpointer user_data);

static inline guint timeout_add_seconds(guint seconds, timeout_func_t func,
                                        gpointer user_data,
                                        GDestroyNotify destroy)
{
  (void)seconds;
  (void)func;
  (void)user_data;
  (void)destroy;
  return 1;
}

static inline gboolean timeout_remove(guint id)
{
  (void)id;
  return TRUE;
}
#endif
