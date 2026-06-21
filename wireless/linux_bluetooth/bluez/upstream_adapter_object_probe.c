/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_adapter_object_probe.c
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

#define adapter_init bluez_upstream_object_adapter_init
#define adapter_cleanup bluez_upstream_object_adapter_cleanup
#define adapter_shutdown bluez_upstream_object_adapter_shutdown
#define btd_adapter_get_default bluez_upstream_object_btd_adapter_get_default
#define btd_adapter_is_default bluez_upstream_object_btd_adapter_is_default
#define btd_adapter_get_index bluez_upstream_object_btd_adapter_get_index
#define btd_adapter_has_cable_pairing_devices \
  bluez_upstream_object_btd_adapter_has_cable_pairing_devices
#define btd_adapter_foreach bluez_upstream_object_btd_adapter_foreach
#define btd_add_disconnect_cb bluez_upstream_object_btd_add_disconnect_cb
#define btd_remove_disconnect_cb bluez_upstream_object_btd_remove_disconnect_cb
#define btd_add_conn_fail_cb bluez_upstream_object_btd_add_conn_fail_cb
#define btd_remove_conn_fail_cb bluez_upstream_object_btd_remove_conn_fail_cb
#define adapter_find bluez_upstream_object_adapter_find
#define adapter_find_by_id bluez_upstream_object_adapter_find_by_id
#define btd_adapter_get_pairable bluez_upstream_object_btd_adapter_get_pairable
#define btd_adapter_get_powered bluez_upstream_object_btd_adapter_get_powered
#define btd_adapter_get_connectable bluez_upstream_object_btd_adapter_get_connectable
#define btd_adapter_get_discoverable \
  bluez_upstream_object_btd_adapter_get_discoverable
#define btd_adapter_get_bredr bluez_upstream_object_btd_adapter_get_bredr
#define btd_adapter_get_database bluez_upstream_object_btd_adapter_get_database
#define btd_adapter_get_class bluez_upstream_object_btd_adapter_get_class
#define btd_adapter_get_name bluez_upstream_object_btd_adapter_get_name
#define btd_adapter_remove_device bluez_upstream_object_btd_adapter_remove_device
#define btd_adapter_get_device bluez_upstream_object_btd_adapter_get_device
#define btd_adapter_get_services bluez_upstream_object_btd_adapter_get_services
#define btd_adapter_find_device bluez_upstream_object_btd_adapter_find_device
#define btd_adapter_find_device_by_path \
  bluez_upstream_object_btd_adapter_find_device_by_path
#define btd_adapter_find_device_by_fd \
  bluez_upstream_object_btd_adapter_find_device_by_fd
#define btd_adapter_device_found bluez_upstream_object_btd_adapter_device_found
#define adapter_get_path bluez_upstream_object_adapter_get_path
#define btd_adapter_get_address bluez_upstream_object_btd_adapter_get_address
#define btd_adapter_get_address_type \
  bluez_upstream_object_btd_adapter_get_address_type
#define btd_adapter_get_storage_dir \
  bluez_upstream_object_btd_adapter_get_storage_dir
#define adapter_service_add bluez_upstream_object_adapter_service_add
#define adapter_service_remove bluez_upstream_object_adapter_service_remove
#define adapter_get_agent bluez_upstream_object_adapter_get_agent
#define btd_adapter_uuid_is_allowed \
  bluez_upstream_object_btd_adapter_uuid_is_allowed
#define btd_adapter_ref bluez_upstream_object_btd_adapter_ref
#define btd_adapter_unref bluez_upstream_object_btd_adapter_unref
#define btd_adapter_set_class bluez_upstream_object_btd_adapter_set_class
#define btd_adapter_set_name bluez_upstream_object_btd_adapter_set_name
#define device_resolved_drivers bluez_upstream_object_device_resolved_drivers
#define adapter_add_profile bluez_upstream_object_adapter_add_profile
#define adapter_remove_profile bluez_upstream_object_adapter_remove_profile
#define btd_register_adapter_driver \
  bluez_upstream_object_btd_register_adapter_driver
#define btd_unregister_adapter_driver \
  bluez_upstream_object_btd_unregister_adapter_driver
#define btd_request_authorization \
  bluez_upstream_object_btd_request_authorization
#define btd_request_authorization_cable_configured \
  bluez_upstream_object_btd_request_authorization_cable_configured
#define btd_cancel_authorization bluez_upstream_object_btd_cancel_authorization
#define btd_adapter_restore_powered \
  bluez_upstream_object_btd_adapter_restore_powered
#define btd_adapter_set_blocked bluez_upstream_object_btd_adapter_set_blocked
#define btd_adapter_register_pin_cb \
  bluez_upstream_object_btd_adapter_register_pin_cb
#define btd_adapter_unregister_pin_cb \
  bluez_upstream_object_btd_adapter_unregister_pin_cb
#define btd_adapter_pin_cb_iter_new \
  bluez_upstream_object_btd_adapter_pin_cb_iter_new
#define btd_adapter_pin_cb_iter_free \
  bluez_upstream_object_btd_adapter_pin_cb_iter_free
#define btd_adapter_pin_cb_iter_end \
  bluez_upstream_object_btd_adapter_pin_cb_iter_end
#define btd_adapter_pin_cb_iter_next \
  bluez_upstream_object_btd_adapter_pin_cb_iter_next
#define btd_adapter_pin_cb_iter_get \
  bluez_upstream_object_btd_adapter_pin_cb_iter_get
#define btd_adapter_register_msd_cb \
  bluez_upstream_object_btd_adapter_register_msd_cb
#define btd_adapter_unregister_msd_cb \
  bluez_upstream_object_btd_adapter_unregister_msd_cb
#define btd_adapter_set_fast_connectable \
  bluez_upstream_object_btd_adapter_set_fast_connectable
#define btd_adapter_ssp_enabled bluez_upstream_object_btd_adapter_ssp_enabled
#define btd_adapter_check_oob_handler \
  bluez_upstream_object_btd_adapter_check_oob_handler
#define btd_adapter_read_local_oob_data \
  bluez_upstream_object_btd_adapter_read_local_oob_data

#define device_update_last_seen bluez_upstream_object_device_update_last_seen
#define device_attach_att bluez_upstream_object_device_attach_att
#define device_discover_services bluez_upstream_object_device_discover_services
#define device_set_paired bluez_upstream_object_device_set_paired
#define device_set_bonded bluez_upstream_object_device_set_bonded
#define device_set_ltk bluez_upstream_object_device_set_ltk
#define btd_device_get_uuids bluez_upstream_object_btd_device_get_uuids
#define device_probe_profiles bluez_upstream_object_device_probe_profiles
#define device_get_adapter bluez_upstream_object_device_get_adapter
#define device_address_cmp bluez_upstream_object_device_address_cmp
#define device_create_from_storage \
  bluez_upstream_object_device_create_from_storage
#define device_set_privacy bluez_upstream_object_device_set_privacy
#define device_add_connection bluez_upstream_object_device_add_connection
#define device_store_cached_name bluez_upstream_object_device_store_cached_name
#define device_set_legacy bluez_upstream_object_device_set_legacy
#define device_name_resolve_fail bluez_upstream_object_device_name_resolve_fail
#define device_set_rssi_with_delta \
  bluez_upstream_object_device_set_rssi_with_delta
#define device_set_appearance bluez_upstream_object_device_set_appearance
#define device_name_known bluez_upstream_object_device_name_known
#define btd_device_device_set_name \
  bluez_upstream_object_btd_device_device_set_name
#define device_set_class bluez_upstream_object_device_set_class
#define btd_device_set_pnpid bluez_upstream_object_btd_device_set_pnpid
#define device_add_eir_uuids bluez_upstream_object_device_add_eir_uuids
#define device_set_manufacturer_data \
  bluez_upstream_object_device_set_manufacturer_data
#define device_set_service_data bluez_upstream_object_device_set_service_data
#define device_set_data bluez_upstream_object_device_set_data
#define device_set_flags bluez_upstream_object_device_set_flags
#define device_is_name_resolve_allowed \
  bluez_upstream_object_device_is_name_resolve_allowed
#define btd_device_connect_services \
  bluez_upstream_object_btd_device_connect_services
#define device_confirm_passkey bluez_upstream_object_device_confirm_passkey
#define device_request_passkey bluez_upstream_object_device_request_passkey
#define device_notify_passkey bluez_upstream_object_device_notify_passkey
#define device_bonding_iter bluez_upstream_object_device_bonding_iter
#define device_notify_pincode bluez_upstream_object_device_notify_pincode
#define device_request_pincode bluez_upstream_object_device_request_pincode
#define device_set_csrk bluez_upstream_object_device_set_csrk
#define device_update_addr bluez_upstream_object_device_update_addr
#define device_merge_duplicate bluez_upstream_object_device_merge_duplicate
#define device_block bluez_upstream_object_device_block
#define device_unblock bluez_upstream_object_device_unblock
#define device_set_unpaired bluez_upstream_object_device_set_unpaired

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "gdbus/gdbus.h"
#include "upstream/src/adapter.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_adapter_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/adapter.c role=%s linked=1 "
         "source=third/bluez/src/adapter.c "
         "owner=bluetoothd api=btd_adapter\n",
         role);
}

unsigned int bluez_upstream_adapter_dependency_bound(void)
{
  unsigned int symbols = 0;

  symbols += sizeof(&bluez_upstream_object_adapter_get_path) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_adapter_service_add) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_adapter_service_remove) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_btd_adapter_get_device) > 0 ? 1 : 0;

  return symbols == 4 ? 1 : 0;
}
