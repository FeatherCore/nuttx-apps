/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_storage_object_probe.c
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
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "upstream/src/storage.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_storage_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/storage.c role=%s linked=1 "
         "source=third/bluez/src/storage.c "
         "owner=bluetoothd api=storage\n",
         role);
}
