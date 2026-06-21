#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_DBUS_COMMON_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_DBUS_COMMON_H

#include "dbus/dbus.h"

static inline DBusConnection *btd_get_dbus_connection(void)
{
  return NULL;
}

#endif
