/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_mgmt_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <sys/socket.h>

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "upstream/src/shared/mgmt.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_mgmt_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/shared/mgmt.c role=%s linked=1 "
         "source=third/bluez/src/shared/mgmt.c "
         "owner=bluetoothd api=mgmt\n",
         role);
}
