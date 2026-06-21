/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_uuid_lib_object_probe.c
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

#include "upstream/lib/bluetooth/uuid.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_uuid_lib_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: lib/bluetooth/uuid.c role=%s linked=1 "
         "source=third/bluez/lib/bluetooth/uuid.c api=bt_uuid\n",
         role);
}
