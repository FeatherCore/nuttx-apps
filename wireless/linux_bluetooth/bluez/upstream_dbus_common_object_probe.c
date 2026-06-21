/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_dbus_common_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_UPSTREAM_OBJECT_NO_GDBUS_DICT_SHIMS

#define dict_append_entry bluez_upstream_object_dict_append_entry
#define dict_append_array bluez_upstream_object_dict_append_array
#define set_dbus_connection bluez_upstream_object_set_dbus_connection
#define btd_get_dbus_connection bluez_upstream_object_btd_get_dbus_connection
#define class_to_icon bluez_upstream_object_class_to_icon
#define gap_appearance_to_icon bluez_upstream_object_gap_appearance_to_icon

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "upstream/src/dbus-common.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_dbus_common_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/dbus-common.c role=%s linked=1 "
         "source=third/bluez/src/dbus-common.c "
         "owner=bluetoothd api=dbus-common\n",
         role);
}
