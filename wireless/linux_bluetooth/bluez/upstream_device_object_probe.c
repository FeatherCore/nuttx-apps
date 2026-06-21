/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_device_object_probe.c
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

#define device_create bluez_upstream_object_device_create
#define device_create_from_storage bluez_upstream_object_device_create_from_storage
#define btd_device_get_storage_path bluez_upstream_object_btd_device_get_storage_path
#define btd_device_device_set_name bluez_upstream_object_btd_device_device_set_name
#define device_store_cached_name bluez_upstream_object_device_store_cached_name
#define device_get_name bluez_upstream_object_device_get_name
#define device_name_known bluez_upstream_object_device_name_known
#define device_is_name_resolve_allowed bluez_upstream_object_device_is_name_resolve_allowed
#define device_name_resolve_fail bluez_upstream_object_device_name_resolve_fail
#define device_set_class bluez_upstream_object_device_set_class
#define device_address_is_private bluez_upstream_object_device_address_is_private
#define device_set_privacy bluez_upstream_object_device_set_privacy
#define device_get_privacy bluez_upstream_object_device_get_privacy
#define device_update_addr bluez_upstream_object_device_update_addr
#define device_set_bredr_support bluez_upstream_object_device_set_bredr_support
#define device_set_le_support bluez_upstream_object_device_set_le_support
#define device_update_last_seen bluez_upstream_object_device_update_last_seen
#define device_merge_duplicate bluez_upstream_object_device_merge_duplicate
#define btd_device_get_class bluez_upstream_object_btd_device_get_class
#define btd_device_get_vendor bluez_upstream_object_btd_device_get_vendor
#define btd_device_get_vendor_src bluez_upstream_object_btd_device_get_vendor_src
#define btd_device_get_product bluez_upstream_object_btd_device_get_product
#define btd_device_get_version bluez_upstream_object_btd_device_get_version
#define btd_device_get_icon bluez_upstream_object_btd_device_get_icon
#define device_remove_bonding bluez_upstream_object_device_remove_bonding
#define device_remove bluez_upstream_object_device_remove
#define device_address_cmp bluez_upstream_object_device_address_cmp
#define device_bdaddr_cmp bluez_upstream_object_device_bdaddr_cmp
#define device_addr_type_cmp bluez_upstream_object_device_addr_type_cmp
#define btd_device_get_uuids bluez_upstream_object_btd_device_get_uuids
#define btd_device_has_uuid bluez_upstream_object_btd_device_has_uuid
#define device_probe_profiles bluez_upstream_object_device_probe_profiles
#define btd_device_set_record bluez_upstream_object_btd_device_set_record
#define btd_device_get_record bluez_upstream_object_btd_device_get_record
#define btd_device_get_primary bluez_upstream_object_btd_device_get_primary
#define btd_device_get_primaries bluez_upstream_object_btd_device_get_primaries
#define btd_device_get_gatt_db bluez_upstream_object_btd_device_get_gatt_db
#define btd_device_set_gatt_db bluez_upstream_object_btd_device_set_gatt_db
#define btd_device_get_gatt_client bluez_upstream_object_btd_device_get_gatt_client
#define btd_device_get_gatt_server bluez_upstream_object_btd_device_get_gatt_server
#define btd_device_is_initiator bluez_upstream_object_btd_device_is_initiator
#define btd_device_get_attrib bluez_upstream_object_btd_device_get_attrib
#define btd_device_gatt_set_service_changed bluez_upstream_object_btd_device_gatt_set_service_changed
#define device_attach_att bluez_upstream_object_device_attach_att
#define btd_device_add_uuid bluez_upstream_object_btd_device_add_uuid
#define device_add_eir_uuids bluez_upstream_object_device_add_eir_uuids
#define device_set_manufacturer_data bluez_upstream_object_device_set_manufacturer_data
#define device_set_service_data bluez_upstream_object_device_set_service_data
#define device_set_data bluez_upstream_object_device_set_data
#define device_probe_profile bluez_upstream_object_device_probe_profile
#define device_remove_profile bluez_upstream_object_device_remove_profile
#define device_get_adapter bluez_upstream_object_device_get_adapter
#define device_get_address bluez_upstream_object_device_get_address
#define device_get_le_address_type bluez_upstream_object_device_get_le_address_type
#define device_get_path bluez_upstream_object_device_get_path
#define device_is_temporary bluez_upstream_object_device_is_temporary
#define device_is_connectable bluez_upstream_object_device_is_connectable
#define device_is_paired bluez_upstream_object_device_is_paired
#define device_is_bonded bluez_upstream_object_device_is_bonded
#define btd_device_is_trusted bluez_upstream_object_btd_device_is_trusted
#define device_is_cable_pairing bluez_upstream_object_device_is_cable_pairing
#define device_set_paired bluez_upstream_object_device_set_paired
#define device_set_unpaired bluez_upstream_object_device_set_unpaired
#define btd_device_set_temporary bluez_upstream_object_btd_device_set_temporary
#define btd_device_set_trusted bluez_upstream_object_btd_device_set_trusted
#define btd_device_set_connectable bluez_upstream_object_btd_device_set_connectable
#define device_set_bonded bluez_upstream_object_device_set_bonded
#define device_set_legacy bluez_upstream_object_device_set_legacy
#define device_set_cable_pairing bluez_upstream_object_device_set_cable_pairing
#define device_set_rssi_with_delta bluez_upstream_object_device_set_rssi_with_delta
#define device_set_rssi bluez_upstream_object_device_set_rssi
#define device_set_tx_power bluez_upstream_object_device_set_tx_power
#define device_set_flags bluez_upstream_object_device_set_flags
#define btd_device_is_connected bluez_upstream_object_btd_device_is_connected
#define btd_device_bearer_is_connected bluez_upstream_object_btd_device_bearer_is_connected
#define btd_device_bdaddr_type_connected bluez_upstream_object_btd_device_bdaddr_type_connected
#define btd_device_get_bdaddr_type bluez_upstream_object_btd_device_get_bdaddr_type
#define device_is_retrying bluez_upstream_object_device_is_retrying
#define device_bonding_complete bluez_upstream_object_device_bonding_complete
#define device_is_bonding bluez_upstream_object_device_is_bonding
#define device_bonding_attempt_failed bluez_upstream_object_device_bonding_attempt_failed
#define device_bonding_failed bluez_upstream_object_device_bonding_failed
#define device_cancel_bonding bluez_upstream_object_device_cancel_bonding
#define device_bonding_iter bluez_upstream_object_device_bonding_iter
#define device_bonding_attempt_retry bluez_upstream_object_device_bonding_attempt_retry
#define device_bonding_last_duration bluez_upstream_object_device_bonding_last_duration
#define device_bonding_restart_timer bluez_upstream_object_device_bonding_restart_timer
#define device_request_pincode bluez_upstream_object_device_request_pincode
#define device_request_passkey bluez_upstream_object_device_request_passkey
#define device_confirm_passkey bluez_upstream_object_device_confirm_passkey
#define device_notify_passkey bluez_upstream_object_device_notify_passkey
#define device_notify_pincode bluez_upstream_object_device_notify_pincode
#define device_cancel_authentication bluez_upstream_object_device_cancel_authentication
#define device_is_authenticating bluez_upstream_object_device_is_authenticating
#define device_cancel_browse bluez_upstream_object_device_cancel_browse
#define device_add_connection bluez_upstream_object_device_add_connection
#define device_remove_connection bluez_upstream_object_device_remove_connection
#define device_request_disconnect bluez_upstream_object_device_request_disconnect
#define device_is_disconnecting bluez_upstream_object_device_is_disconnecting
#define device_is_connecting bluez_upstream_object_device_is_connecting
#define device_set_ltk bluez_upstream_object_device_set_ltk
#define btd_device_get_ltk bluez_upstream_object_btd_device_get_ltk
#define device_set_csrk bluez_upstream_object_device_set_csrk
#define btd_device_add_set bluez_upstream_object_btd_device_add_set
#define device_store_svc_chng_ccc bluez_upstream_object_device_store_svc_chng_ccc
#define device_load_svc_chng_ccc bluez_upstream_object_device_load_svc_chng_ccc
#define device_set_wake_support bluez_upstream_object_device_set_wake_support
#define device_set_wake_override bluez_upstream_object_device_set_wake_override
#define device_set_past_support bluez_upstream_object_device_set_past_support
#define device_set_refresh_discovery bluez_upstream_object_device_set_refresh_discovery
#define device_add_disconnect_watch bluez_upstream_object_device_add_disconnect_watch
#define device_remove_disconnect_watch bluez_upstream_object_device_remove_disconnect_watch
#define device_disconnect_watches_callback bluez_upstream_object_device_disconnect_watches_callback
#define device_get_appearance bluez_upstream_object_device_get_appearance
#define device_set_appearance bluez_upstream_object_device_set_appearance
#define btd_device_ref bluez_upstream_object_btd_device_ref
#define btd_device_unref bluez_upstream_object_btd_device_unref
#define device_block bluez_upstream_object_device_block
#define device_unblock bluez_upstream_object_device_unblock
#define btd_device_set_pnpid bluez_upstream_object_btd_device_set_pnpid
#define device_connect_le bluez_upstream_object_device_connect_le
#define device_connect_profiles bluez_upstream_object_device_connect_profiles
#define device_wait_for_svc_complete bluez_upstream_object_device_wait_for_svc_complete
#define device_remove_svc_complete_callback bluez_upstream_object_device_remove_svc_complete_callback
#define btd_device_get_service bluez_upstream_object_btd_device_get_service
#define device_discover_services bluez_upstream_object_device_discover_services
#define btd_device_connect_services bluez_upstream_object_btd_device_connect_services
#define btd_device_flags_enabled bluez_upstream_object_btd_device_flags_enabled
#define btd_device_get_current_flags bluez_upstream_object_btd_device_get_current_flags
#define btd_device_get_supported_flags bluez_upstream_object_btd_device_get_supported_flags
#define btd_device_get_pending_flags bluez_upstream_object_btd_device_get_pending_flags
#define btd_device_set_pending_flags bluez_upstream_object_btd_device_set_pending_flags
#define btd_device_flags_changed bluez_upstream_object_btd_device_flags_changed
#define btd_device_all_services_allowed bluez_upstream_object_btd_device_all_services_allowed
#define btd_device_update_allowed_services bluez_upstream_object_btd_device_update_allowed_services
#define btd_device_init bluez_upstream_object_btd_device_init
#define btd_device_cleanup bluez_upstream_object_btd_device_cleanup
#define btd_device_set_volume bluez_upstream_object_btd_device_set_volume
#define btd_device_get_volume bluez_upstream_object_btd_device_get_volume
#define btd_device_foreach_ad bluez_upstream_object_btd_device_foreach_ad
#define btd_device_set_conn_param bluez_upstream_object_btd_device_set_conn_param
#define btd_device_foreach_service_data bluez_upstream_object_btd_device_foreach_service_data
#define btd_device_foreach_service bluez_upstream_object_btd_device_foreach_service
#define device_remove_pending_services bluez_upstream_object_device_remove_pending_services

#define adapter_get_path bluez_upstream_object_adapter_get_path
#define service_remove bluez_upstream_object_service_remove
#define class_to_icon bluez_upstream_object_class_to_icon
#define gap_appearance_to_icon bluez_upstream_object_gap_appearance_to_icon

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "upstream/src/device.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_device_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/device.c role=%s linked=1 "
         "source=third/bluez/src/device.c "
         "owner=bluetoothd api=btd_device\n",
         role);
}

unsigned int bluez_upstream_device_dependency_bound(void)
{
  unsigned int symbols = 0;

  symbols += sizeof(&bluez_upstream_object_device_get_path) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_device_get_adapter) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_btd_device_get_uuids) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_device_probe_profiles) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_device_add_connection) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_device_update_last_seen) > 0 ? 1 : 0;

  return symbols == 6 ? 1 : 0;
}
