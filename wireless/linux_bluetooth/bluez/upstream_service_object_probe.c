/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_service_object_probe.c
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

#define btd_service_ref bluez_upstream_object_btd_service_ref
#define btd_service_unref bluez_upstream_object_btd_service_unref
#define service_create bluez_upstream_object_service_create
#define service_probe bluez_upstream_object_service_probe
#define service_remove bluez_upstream_object_service_remove
#define service_accept bluez_upstream_object_service_accept
#define service_set_connecting bluez_upstream_object_service_set_connecting
#define btd_service_connect bluez_upstream_object_btd_service_connect
#define btd_service_disconnect bluez_upstream_object_btd_service_disconnect
#define btd_service_get_device bluez_upstream_object_btd_service_get_device
#define btd_service_get_profile bluez_upstream_object_btd_service_get_profile
#define btd_service_set_user_data bluez_upstream_object_btd_service_set_user_data
#define btd_service_get_user_data bluez_upstream_object_btd_service_get_user_data
#define btd_service_get_state bluez_upstream_object_btd_service_get_state
#define btd_service_get_error bluez_upstream_object_btd_service_get_error
#define btd_service_is_initiator bluez_upstream_object_btd_service_is_initiator
#define btd_service_add_state_cb bluez_upstream_object_btd_service_add_state_cb
#define btd_service_remove_state_cb bluez_upstream_object_btd_service_remove_state_cb
#define btd_service_set_allowed bluez_upstream_object_btd_service_set_allowed
#define btd_service_is_allowed bluez_upstream_object_btd_service_is_allowed
#define btd_service_connecting_complete \
  bluez_upstream_object_btd_service_connecting_complete
#define btd_service_disconnecting_complete \
  bluez_upstream_object_btd_service_disconnecting_complete

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "gdbus/gdbus.h"
#include "upstream/src/service.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_service_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/service.c role=%s linked=1 "
         "source=third/bluez/src/service.c "
         "owner=bluetoothd api=btd_service\n",
         role);
}
