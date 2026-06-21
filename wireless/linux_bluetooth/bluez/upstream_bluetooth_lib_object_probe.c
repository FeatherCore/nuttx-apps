/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_bluetooth_lib_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "upstream/lib/bluetooth/bluetooth.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_bluetooth_lib_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: lib/bluetooth/bluetooth.c role=%s linked=1 "
         "source=third/bluez/lib/bluetooth/bluetooth.c api=bdaddr\n",
         role);
}
