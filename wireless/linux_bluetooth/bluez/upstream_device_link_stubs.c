/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_device_link_stubs.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Temporary link boundary for src/device.c object-porting.  Each symbol here
 * marks an upstream owner that still needs its own linked object gate.
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/uio.h>

#include "bluetooth/uuid.h"

char btd_opts[256];
void *device_resolved_drivers;

#define STUB(name) __attribute__((weak)) void *name(void) { return 0; }

struct bluez_upstream_bdaddr_compat
{
  unsigned char b[6];
};

int bachk(const char *str)
{
  int i;

  if (str == 0)
    {
      return -1;
    }

  for (i = 0; i < 17; i++)
    {
      const char c = str[i];
      const int is_hex = (c >= '0' && c <= '9') ||
                         (c >= 'a' && c <= 'f') ||
                         (c >= 'A' && c <= 'F');

      if ((i + 1) % 3 == 0)
        {
          if (i != 17 && c != ':')
            {
              return -1;
            }
        }
      else if (!is_hex)
        {
          return -1;
        }
    }

  return str[17] == '\0' ? 0 : -1;
}

int str2ba(const char *str, struct bluez_upstream_bdaddr_compat *ba)
{
  int i;

  if (bachk(str) < 0 || ba == 0)
    {
      return -1;
    }

  for (i = 5; i >= 0; i--, str += 3)
    {
      unsigned int value = 0;
      sscanf(str, "%02x", &value);
      ba->b[i] = value & 0xff;
    }

  return 0;
}

int sdp_get_browse_groups(const void *rec, void *seqp)
{
  (void)rec;
  (void)seqp;
  return 0;
}

unsigned short cpu_to_le16(unsigned short value)
{
  return value;
}

unsigned int le32_to_cpu(unsigned int value)
{
  return value;
}

unsigned long long cpu_to_le64(unsigned long long value)
{
  return value;
}

unsigned long long le64_to_cpu(unsigned long long value)
{
  return value;
}

unsigned short le16_to_cpu(unsigned short value)
{
  return value;
}

void bswap_128(const void *src, void *dst)
{
  const unsigned char *s = src;
  unsigned char *d = dst;
  int i;

  for (i = 0; i < 16; i++)
    {
      d[i] = s[15 - i];
    }
}

int bt_uuid128_create(bt_uuid_t *btuuid, uint128_t value)
{
  if (btuuid == 0)
    {
      return -1;
    }

  memset(btuuid, 0, sizeof(*btuuid));
  btuuid->type = BT_UUID128;
  btuuid->value.u128 = value;
  return 0;
}

int bt_uuid16_create(bt_uuid_t *btuuid, uint16_t value)
{
  if (btuuid == 0)
    {
      return -1;
    }

  memset(btuuid, 0, sizeof(*btuuid));
  btuuid->type = BT_UUID16;
  btuuid->value.u16 = value;
  return 0;
}

int bt_uuid32_create(bt_uuid_t *btuuid, uint32_t value)
{
  if (btuuid == 0)
    {
      return -1;
    }

  memset(btuuid, 0, sizeof(*btuuid));
  btuuid->type = BT_UUID32;
  btuuid->value.u32 = value;
  return 0;
}

int bt_uuid_cmp(const bt_uuid_t *uuid1, const bt_uuid_t *uuid2)
{
  if (uuid1 == uuid2)
    {
      return 0;
    }

  if (uuid1 == 0)
    {
      return -1;
    }

  if (uuid2 == 0)
    {
      return 1;
    }

  return memcmp(uuid1, uuid2, sizeof(*uuid1));
}

unsigned int bt_uuid_len(const bt_uuid_t *uuid)
{
  if (uuid == 0)
    {
      return 0;
    }

  if (uuid->type == BT_UUID16)
    {
      return 2;
    }

  if (uuid->type == BT_UUID32)
    {
      return 4;
    }

  if (uuid->type == BT_UUID128)
    {
      return 16;
    }

  return 0;
}

int util_iov_pull_u8(struct iovec *iov, uint8_t *value)
{
  uint8_t *base;

  if (iov == 0 || value == 0 || iov->iov_base == 0 || iov->iov_len < 1)
    {
      return 0;
    }

  base = iov->iov_base;
  *value = base[0];
  iov->iov_base = base + 1;
  iov->iov_len--;
  return 1;
}

int util_iov_pull_le16(struct iovec *iov, uint16_t *value)
{
  uint8_t *base;

  if (iov == 0 || value == 0 || iov->iov_base == 0 || iov->iov_len < 2)
    {
      return 0;
    }

  base = iov->iov_base;
  *value = (uint16_t)base[0] | ((uint16_t)base[1] << 8);
  iov->iov_base = base + 2;
  iov->iov_len -= 2;
  return 1;
}

int util_iov_pull_le32(struct iovec *iov, uint32_t *value)
{
  uint8_t *base;

  if (iov == 0 || value == 0 || iov->iov_base == 0 || iov->iov_len < 4)
    {
      return 0;
    }

  base = iov->iov_base;
  *value = (uint32_t)base[0] | ((uint32_t)base[1] << 8) |
           ((uint32_t)base[2] << 16) | ((uint32_t)base[3] << 24);
  iov->iov_base = base + 4;
  iov->iov_len -= 4;
  return 1;
}

void *util_iov_pull_mem(struct iovec *iov, size_t len)
{
  uint8_t *base;

  if (iov == 0 || iov->iov_base == 0 || iov->iov_len < len)
    {
      return 0;
    }

  base = iov->iov_base;
  iov->iov_base = base + len;
  iov->iov_len -= len;
  return base;
}

STUB(DBG_IDX)
STUB(MAX)
STUB(a2dp_avdtp_get)
STUB(a2dp_sep_lock)
STUB(a2dp_sep_unlock)
STUB(adapter_accept_list_add)
STUB(adapter_accept_list_remove)
STUB(adapter_auto_connect_add)
STUB(adapter_auto_connect_remove)
STUB(adapter_cancel_bonding)
STUB(adapter_connect_list_add)
STUB(adapter_connect_list_remove)
STUB(adapter_create_bonding)
STUB(agent_cancel)
STUB(agent_get)
STUB(agent_get_io_capability)
STUB(agent_ref)
STUB(agent_unref)
STUB(avdtp_unref)
STUB(bluez_upstream_object_avdtp_abort)
STUB(bluez_upstream_object_avdtp_add_state_cb)
STUB(bluez_upstream_object_avdtp_close)
STUB(bluez_upstream_object_avdtp_error_category)
STUB(bluez_upstream_object_avdtp_error_init)
STUB(bluez_upstream_object_avdtp_error_posix_errno)
STUB(bluez_upstream_object_avdtp_get_seid)
STUB(bluez_upstream_object_avdtp_remove_state_cb)
STUB(bluez_upstream_object_avdtp_set_configuration)
STUB(bluez_upstream_object_avdtp_start)
STUB(bluez_upstream_object_avdtp_stream_get_transport)
STUB(bluez_upstream_object_avdtp_suspend)
STUB(bt_att_attach_fd)
STUB(bt_att_get_channels)
STUB(bt_att_ref)
STUB(bt_att_register_disconnect)
STUB(bt_att_set_close_on_unref)
STUB(bt_att_set_debug)
STUB(bt_att_set_enc_key_size)
STUB(bt_att_set_local_key)
STUB(bt_att_set_remote_key)
STUB(bt_att_set_security)
STUB(bt_att_unref)
STUB(bt_att_unregister_disconnect)
STUB(bt_bap_req_bcode)
STUB(bt_bap_state_register)
STUB(bt_bap_state_unregister)
STUB(bt_bap_stream_cancel)
STUB(bt_bap_stream_disable)
STUB(bt_bap_stream_enable)
STUB(bt_bap_stream_get_dir)
STUB(bt_bap_stream_get_location)
STUB(bt_bap_stream_get_metadata)
STUB(bt_bap_stream_get_qos)
STUB(bt_bap_stream_get_state)
STUB(bt_bap_stream_io_get_links)
STUB(bt_bap_stream_io_link)
STUB(bt_bap_stream_io_unlink)
STUB(bt_bap_stream_metadata)
STUB(bt_cancel_discovery)
STUB(bt_gatt_client_cancel_all)
STUB(bt_gatt_client_is_ready)
STUB(bt_gatt_client_new)
STUB(bt_gatt_client_ready_register)
STUB(bt_gatt_client_ready_unregister)
STUB(bt_gatt_client_set_debug)
STUB(bt_gatt_client_set_service_changed)
STUB(bt_gatt_client_unref)
STUB(bt_gatt_server_new)
STUB(bt_gatt_server_set_debug)
STUB(bt_gatt_server_set_permissions)
STUB(bt_gatt_server_unref)
STUB(bt_modalias)
STUB(bt_name2string)
STUB(bt_search)
STUB(bt_search_service)
STUB(bt_string2uuid)
STUB(bt_uuid2string)
STUB(bt_uuid_to_string)
STUB(bt_free)
STUB(btd_adapter_cancel_service_auth)
STUB(btd_adapter_confirm_reply)
STUB(btd_adapter_disconnect_device)
STUB(btd_adapter_get_address)
STUB(btd_adapter_get_address_type)
STUB(btd_adapter_get_bredr)
STUB(btd_adapter_get_database)
STUB(btd_adapter_get_powered)
STUB(btd_adapter_get_storage_dir)
STUB(btd_adapter_is_uuid_allowed)
STUB(btd_adapter_passkey_reply)
STUB(btd_adapter_pin_cb_iter_new)
STUB(btd_adapter_pincode_reply)
STUB(btd_adapter_remove_device)
STUB(btd_adapter_ssp_enabled)
STUB(btd_adv_monitor_content_filter)
STUB(btd_adv_monitor_manager_create)
STUB(btd_adv_monitor_notify_monitors)
STUB(btd_adv_monitor_offload_enabled)
STUB(btd_adv_monitor_power_down)
STUB(btd_adv_manager_new)
STUB(btd_battery_provider_manager_create)
STUB(btd_bearer_connected)
STUB(btd_bearer_paired)
STUB(btd_debug)
STUB(btd_device_get_product)
STUB(btd_device_get_vendor)
STUB(btd_device_get_version)
STUB(btd_error)
STUB(btd_error_already_exists)
STUB(btd_error_bredr_errno)
STUB(btd_error_failed)
STUB(btd_error_in_progress)
STUB(btd_error_in_progress_str)
STUB(btd_error_invalid_args)
STUB(btd_error_invalid_args_str)
STUB(btd_error_le_errno)
STUB(btd_error_not_ready_str)
STUB(btd_error_profile_unavailable)
STUB(btd_gatt_client_connected)
STUB(btd_gatt_client_disconnected)
STUB(btd_gatt_client_eatt_connect)
STUB(btd_gatt_client_new)
STUB(btd_gatt_client_ready)
STUB(btd_gatt_client_destroy)
STUB(btd_gatt_client_service_added)
STUB(btd_gatt_client_service_removed)
STUB(btd_gatt_database_att_disconnected)
STUB(btd_gatt_database_new)
STUB(btd_gatt_database_get_db)
STUB(btd_gatt_database_restore_svc_chng_ccc)
STUB(btd_gatt_database_server_connected)
STUB(btd_kernel_experimental_enabled)
STUB(btd_le_connect_before_pairing)
STUB(btd_bearer_new)
STUB(btd_bearer_destroy)
STUB(btd_bearer_bonded)
STUB(btd_set_add_device)
STUB(btd_set_get_path)
STUB(btd_profile_find_remote_uuid)
STUB(btd_profile_foreach)
STUB(btd_profile_sort_list)
STUB(btd_assertion_message_expr)
STUB(btd_service_connect)
STUB(btd_service_disconnect)
STUB(btd_service_get_profile)
STUB(btd_service_get_state)
STUB(btd_service_is_allowed)
STUB(btd_service_set_allowed)
STUB(btd_service_unref)
STUB(btd_settings_gatt_db_load)
STUB(btd_settings_gatt_db_store)
STUB(btd_warn)
STUB(create_file)
STUB(create_filename)
STUB(device_get_name)
STUB(device_get_path)
STUB(device_get_address)
STUB(device_get_privacy)
STUB(device_is_temporary)
STUB(btd_device_is_connected)
STUB(btd_device_get_bdaddr_type)
STUB(btd_device_bearer_is_connected)
STUB(find_record_in_list)
STUB(g_attrib_attach_client)
STUB(g_attrib_cancel_all)
STUB(g_attrib_get_att)
STUB(g_attrib_new)
STUB(g_attrib_unref)
STUB(g_ascii_strup)
STUB(g_dbus_send_reply)
STUB(g_file_set_contents)
STUB(g_hash_table_foreach)
STUB(g_hash_table_size)
STUB(g_io_channel_unix_new)
STUB(g_key_file_get_keys)
STUB(g_key_file_remove_group)
STUB(g_key_file_remove_key)
STUB(g_key_file_set_boolean)
STUB(g_key_file_set_integer)
STUB(g_key_file_set_string)
STUB(g_key_file_set_string_list)
STUB(g_key_file_to_data)
STUB(g_slist_concat)
STUB(g_slist_copy)
STUB(g_slist_delete_link)
STUB(g_slist_insert_sorted)
STUB(g_slist_length)
STUB(g_strdup_printf)
STUB(g_strfreev)
STUB(g_strstrip)
STUB(gatt_db_attribute_get_service_handles)
STUB(gatt_db_attribute_get_service_data)
STUB(gatt_db_attribute_get_service_uuid)
STUB(gatt_db_clear)
STUB(gatt_db_foreach_service)
STUB(gatt_db_isempty)
STUB(gatt_db_new)
STUB(gatt_db_register)
STUB(gatt_db_unregister)
STUB(gatt_db_unref)
STUB(gatt_db_service_get_active)
STUB(gatt_db_service_set_active)
STUB(gatt_db_service_set_claimed)
STUB(gatt_parse_record)
STUB(info)
STUB(media_endpoint_get_btd_adapter)
STUB(media_endpoint_get_codec)
STUB(media_endpoint_get_delay_reporting)
STUB(media_endpoint_get_sep)
STUB(media_endpoint_get_uuid)
STUB(media_endpoint_is_broadcast)
STUB(media_transport_get_dev)
STUB(media_transport_set_a2dp_volume)
STUB(record_from_string)
STUB(strisutf8)
STUB(strstrip)
STUB(strtoutf8)
STUB(sdp_copy_record)
STUB(sdp_data_get)
STUB(sdp_gen_record_pdu)
STUB(sdp_list_find)
STUB(sdp_uuid128_to_uuid)
STUB(sdp_uuid2strn)
STUB(service_accept)
STUB(service_create)
STUB(service_probe)
STUB(sink_add_state_cb)
STUB(sink_remove_state_cb)
STUB(source_add_state_cb)
STUB(source_remove_state_cb)
STUB(util_iov_dup)
STUB(util_iov_free)
STUB(util_iov_memcmp)
STUB(util_iov_new)
STUB(util_get_dt)
STUB(queue_pop_head)
STUB(queue_push_head)
STUB(mgmt_send)
STUB(mgmt_send_timeout)
STUB(mgmt_reply)
STUB(mgmt_reply_timeout)
STUB(get_le16)
STUB(get_le32)
STUB(htobs)
STUB(htobl)
STUB(btohs)
STUB(btohl)
STUB(cpu_to_le32)
STUB(bacpy)
STUB(bacmp)
STUB(ntoh128)
STUB(htob128)
STUB(sdp_uuid16_to_uuid128)
STUB(sdp_uuid32_to_uuid128)
STUB(bt_string_to_uuid)
STUB(bt_uuid_to_uuid128)
STUB(g_try_malloc)
STUB(g_hash_table_contains)
STUB(g_list_next)
STUB(g_queue_delete_link)
STUB(bt_crypto_new)
STUB(bt_crypto_random_bytes)
STUB(bt_crypto_unref)
STUB(btd_exit)
STUB(btd_info)
STUB(btd_adv_manager_refresh)
STUB(btd_adv_monitor_device_remove)
STUB(btd_error_busy)
STUB(btd_error_not_ready)
STUB(btd_error_does_not_exist)
STUB(device_addr_type_cmp)
STUB(device_set_bredr_support)
STUB(device_set_le_support)
STUB(device_set_rssi)
STUB(device_set_tx_power)
STUB(device_set_temporary)
STUB(device_create)
STUB(device_remove)
STUB(device_connect_le)
STUB(device_remove_svc_complete_callback)
STUB(device_is_connectable)
STUB(device_is_bonding)
STUB(device_is_retrying)
STUB(device_bonding_failed)
STUB(device_bonding_complete)
STUB(device_bonding_attempt_retry)
STUB(device_bonding_restart_timer)
STUB(device_request_disconnect)
STUB(device_remove_connection)
STUB(device_cancel_authentication)
STUB(btd_device_set_pending_flags)
STUB(btd_device_get_pending_flags)
STUB(btd_device_get_supported_flags)
STUB(btd_device_get_current_flags)
STUB(btd_device_flags_changed)
STUB(btd_device_set_temporary)

#undef STUB
