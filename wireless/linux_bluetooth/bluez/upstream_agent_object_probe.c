/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_agent_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

#include "dbus/dbus.h"

__attribute__((weak)) DBusConnection *btd_get_dbus_connection(void)
{
  return NULL;
}

#define device_get_adapter bluez_upstream_object_device_get_adapter

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "upstream/src/shared/queue.h"
#define BLUEZ_UPSTREAM_OBJECT_SHIM_QUEUE_H
#include "upstream/src/shared/queue.c"
#include "upstream/src/agent.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_agent_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/agent.c role=%s linked=1 "
         "source=third/bluez/src/agent.c "
         "owner=bluetoothd api=agent\n",
         role);
}
