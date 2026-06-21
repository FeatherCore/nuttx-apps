/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_profile_object_probe.c
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

#define btd_profile_foreach bluez_upstream_object_btd_profile_foreach
#define btd_profile_register bluez_upstream_object_btd_profile_register
#define btd_profile_unregister bluez_upstream_object_btd_profile_unregister
#define btd_profile_add_custom_prop bluez_upstream_object_btd_profile_add_custom_prop
#define btd_profile_remove_custom_prop bluez_upstream_object_btd_profile_remove_custom_prop
#define btd_profile_init bluez_upstream_object_btd_profile_init
#define btd_profile_cleanup bluez_upstream_object_btd_profile_cleanup
#define btd_profile_find_remote_uuid bluez_upstream_object_btd_profile_find_remote_uuid
#define btd_profile_sort_list bluez_upstream_object_btd_profile_sort_list
#define dict_append_entry bluez_upstream_object_dict_append_entry
#define dict_append_array bluez_upstream_object_dict_append_array

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "upstream/src/profile.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_profile_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/profile.c role=%s linked=1 "
         "source=third/bluez/src/profile.c "
         "owner=bluetoothd api=btd_profile\n",
         role);
}
