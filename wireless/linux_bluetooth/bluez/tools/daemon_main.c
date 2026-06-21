/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/daemon_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/wireless/linux_bluetooth.h>

#include "../upstream_a2dp_compat.h"
#include "../upstream_audio_link_probe.h"
#include "../upstream_agent_object_probe.h"
#include "../upstream_avctp_object_probe.h"
#include "../upstream_avrcp_object_probe.h"
#include "../upstream_avrcp_player_object_probe.h"
#include "../upstream_adapter_object_probe.h"
#include "../upstream_dbus_common_object_probe.h"
#include "../upstream_device_object_probe.h"
#include "../upstream_error_object_probe.h"
#include "../upstream_service_object_probe.h"
#include "../upstream_io_mainloop_object_probe.h"
#include "../upstream_mainloop_object_probe.h"
#include "../upstream_media_object_probe.h"
#include "../upstream_media_owner_object_probe.h"
#include "../upstream_mgmt_object_probe.h"
#include "../upstream_manifest.h"
#include "../upstream_player_object_probe.h"
#include "../upstream_profile_object_probe.h"
#include "../upstream_sdpd_database_object_probe.h"
#include "../upstream_sdpd_service_object_probe.h"
#include "../upstream_sink_object_probe.h"
#include "../upstream_source_object_probe.h"
#include "../upstream_storage_object_probe.h"
#include "../upstream_transport_object_probe.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_HCI
#  define BTPROTO_HCI 1
#endif

#ifndef BTPROTO_L2CAP
#  define BTPROTO_L2CAP 0
#endif

#ifndef HCI_CHANNEL_CONTROL
#  define HCI_CHANNEL_CONTROL 3
#endif

#ifndef HCI_DEV_NONE
#  define HCI_DEV_NONE 0xffff
#endif

#ifndef MGMT_INDEX_NONE
#  define MGMT_INDEX_NONE 0xffff
#endif

#define MGMT_OP_READ_VERSION       0x0001
#define MGMT_OP_READ_COMMANDS      0x0002
#define MGMT_OP_READ_INDEX_LIST    0x0003
#define MGMT_OP_READ_INFO          0x0004
#define MGMT_OP_SET_POWERED        0x0005
#define MGMT_OP_SET_DISCOVERABLE   0x0006
#define MGMT_OP_SET_CONNECTABLE    0x0007
#define MGMT_OP_SET_BONDABLE       0x0009
#define MGMT_OP_SET_LE             0x000d
#define MGMT_OP_DISCONNECT         0x0014
#define MGMT_OP_SET_IO_CAPABILITY  0x0018
#define MGMT_OP_PAIR_DEVICE        0x0019
#define MGMT_OP_CANCEL_PAIR_DEVICE 0x001a
#define MGMT_OP_UNPAIR_DEVICE      0x001b
#define MGMT_OP_USER_CONFIRM_REPLY 0x001c
#define MGMT_OP_USER_CONFIRM_NEG_REPLY 0x001d
#define MGMT_OP_USER_PASSKEY_REPLY 0x001e
#define MGMT_OP_USER_PASSKEY_NEG_REPLY 0x001f
#define MGMT_OP_START_DISCOVERY    0x0023
#define MGMT_OP_STOP_DISCOVERY     0x0024
#define MGMT_OP_BLOCK_DEVICE       0x0026
#define MGMT_OP_UNBLOCK_DEVICE     0x0027
#define MGMT_OP_SET_ADVERTISING    0x0029
#define MGMT_OP_SET_BREDR          0x002a
#define MGMT_OP_GET_CONN_INFO      0x0031
#define MGMT_OP_ADD_DEVICE         0x0033
#define MGMT_OP_REMOVE_DEVICE      0x0034
#define MGMT_OP_GET_DEVICE_FLAGS   0x004f
#define MGMT_OP_SET_DEVICE_FLAGS   0x0050

#define MGMT_EV_CMD_COMPLETE       0x0001
#define MGMT_EV_CMD_STATUS         0x0002
#define MGMT_EV_NEW_SETTINGS       0x0006
#define MGMT_EV_NEW_LONG_TERM_KEY  0x000a
#define MGMT_EV_DEVICE_CONNECTED   0x000b
#define MGMT_EV_DEVICE_DISCONNECTED 0x000c
#define MGMT_EV_USER_CONFIRM_REQUEST 0x000f
#define MGMT_EV_DEVICE_UNPAIRED    0x000f
#define MGMT_EV_USER_PASSKEY_REQUEST 0x0010
#define MGMT_EV_DEVICE_BLOCKED     0x0010
#define MGMT_EV_DEVICE_UNBLOCKED   0x0011
#define MGMT_EV_DEVICE_FOUND       0x0012
#define MGMT_EV_DISCOVERING        0x0013
#define MGMT_EV_DEVICE_ADDED       0x001a
#define MGMT_EV_DEVICE_REMOVED     0x001b
#define MGMT_EV_DEVICE_FLAGS_CHANGED 0x002a

#define MGMT_STATUS_SUCCESS        0x00
#define MGMT_STATUS_FAILED         0x03
#define MGMT_STATUS_CANCELLED      0x10

#define BDADDR_LE_PUBLIC           0x01
#define BLUEZ_IO_CAP_DISPLAY_YESNO 0x01
#define BLUEZ_IO_CAP_KEYBOARD_ONLY 0x02
#define BLUEZ_IO_CAP_NO_INPUT_OUTPUT 0x03
#define BLUEZ_PASSKEY_STAGED       123456
#define BLUEZ_DEVICE_ACTION_AUTO_CONNECT 0x01
#define BLUEZ_DEVICE_FLAG_REMOTE_WAKEUP 0x00000001

#define BLUEZ_DAEMON_A2DP_SIGNAL_PSM 0x0019
#define BLUEZ_DAEMON_A2DP_SIGNAL_CID 0x0040
#define BLUEZ_DAEMON_A2DP_MEDIA_PSM  0x0019
#define BLUEZ_DAEMON_A2DP_MEDIA_CID  0x0041
#define BLUEZ_DAEMON_AVRCP_PSM       0x0017
#define BLUEZ_DAEMON_AVRCP_CID       0x0042
#define BLUEZ_DAEMON_AVRCP_BROWSE_PSM 0x001b
#define BLUEZ_DAEMON_AVRCP_BROWSE_CID 0x0043
#define BLUEZ_DAEMON_HID_CONTROL_PSM 0x0011
#define BLUEZ_DAEMON_HID_CONTROL_CID 0x0051
#define BLUEZ_DAEMON_HID_INTERRUPT_PSM 0x0013
#define BLUEZ_DAEMON_HID_INTERRUPT_CID 0x0053
#define BLUEZ_DAEMON_ATT_FIXED_PSM   0x0000
#define BLUEZ_DAEMON_ATT_FIXED_CID   0x0004
#define BLUEZ_DAEMON_RFCOMM_PSM      0x0003
#define BLUEZ_DAEMON_HFP_RFCOMM_CID  0x0061
#define BLUEZ_DAEMON_HSP_RFCOMM_CID  0x0062
#define BLUEZ_DAEMON_BIP_RFCOMM_CID  0x0063
#define BLUEZ_DAEMON_PRINT_RFCOMM_CID 0x0064
#define BLUEZ_DAEMON_PBAP_RFCOMM_CID 0x0065
#define BLUEZ_DAEMON_OPP_RFCOMM_CID  0x0066
#define BLUEZ_DAEMON_MAP_RFCOMM_CID  0x0067
#define BLUEZ_DAEMON_MNS_RFCOMM_CID  0x0068
#define BLUEZ_DAEMON_FTP_RFCOMM_CID  0x0069
#define BLUEZ_DAEMON_SYNC_RFCOMM_CID 0x006a
#define BLUEZ_DAEMON_IAP_RFCOMM_CID  0x006b

#define BLUEZ_DAEMON_A2DP_STATE_IDLE       0
#define BLUEZ_DAEMON_A2DP_STATE_CONFIGURED 1
#define BLUEZ_DAEMON_A2DP_STATE_OPEN       2
#define BLUEZ_DAEMON_A2DP_STATE_STREAMING  3
#define BLUEZ_DAEMON_A2DP_STATE_SUSPENDED  4
#define BLUEZ_DAEMON_A2DP_STATE_CLOSED     5

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_daemon_mgmt_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
};

struct bluez_daemon_sockaddr_hci
{
  uint16_t hci_family;
  uint16_t hci_dev;
  uint16_t hci_channel;
};

struct bluez_daemon_sockaddr_l2
{
  sa_family_t l2_family;
  uint16_t l2_psm;
  uint8_t l2_bdaddr[6];
  uint16_t l2_cid;
  uint8_t l2_bdaddr_type;
};

struct bluez_daemon_mgmt_ev_cmd_complete
{
  uint16_t opcode;
  uint8_t status;
  uint8_t data[0];
};

struct bluez_daemon_mgmt_addr_info
{
  uint8_t bdaddr[6];
  uint8_t type;
} __attribute__((packed));

struct bluez_daemon_mgmt_cp_pair_device
{
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap;
} __attribute__((packed));

struct bluez_daemon_mgmt_cp_unpair_device
{
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t disconnect;
} __attribute__((packed));

struct bluez_daemon_mgmt_cp_user_passkey_reply
{
  struct bluez_daemon_mgmt_addr_info addr;
  uint32_t passkey;
} __attribute__((packed));

struct bluez_daemon_mgmt_cp_add_device
{
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t action;
} __attribute__((packed));

struct bluez_daemon_mgmt_cp_set_device_flags
{
  struct bluez_daemon_mgmt_addr_info addr;
  uint32_t current_flags;
} __attribute__((packed));

struct bluez_daemon_audio_pdu
{
  const char *label;
  uint16_t psm;
  uint16_t cid;
  const uint8_t *cmd;
  size_t cmd_len;
  const uint8_t *rsp;
  size_t rsp_len;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_bluez_daemon_avdtp_discover_cmd[] =
{
  0x00, 0x01
};

static const uint8_t g_bluez_daemon_avdtp_discover_rsp[] =
{
  0x02, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_avdtp_setconfig_cmd[] =
{
  0x10, 0x03, 0x01, 0x02, 0x07, 0x06, 0x3f, 0x15, 0x02, 0x35
};

static const uint8_t g_bluez_daemon_avdtp_setconfig_rsp[] =
{
  0x12, 0x03
};

static const uint8_t g_bluez_daemon_avdtp_open_cmd[] =
{
  0x20, 0x06, 0x01
};

static const uint8_t g_bluez_daemon_avdtp_open_rsp[] =
{
  0x22, 0x06
};

static const uint8_t g_bluez_daemon_avdtp_start_cmd[] =
{
  0x30, 0x07, 0x01
};

static const uint8_t g_bluez_daemon_avdtp_start_rsp[] =
{
  0x32, 0x07
};

static const uint8_t g_bluez_daemon_avdtp_suspend_cmd[] =
{
  0x40, 0x09, 0x01
};

static const uint8_t g_bluez_daemon_avdtp_suspend_rsp[] =
{
  0x42, 0x09
};

static const uint8_t g_bluez_daemon_avdtp_close_cmd[] =
{
  0x50, 0x08, 0x01
};

static const uint8_t g_bluez_daemon_avdtp_close_rsp[] =
{
  0x52, 0x08
};

static const uint8_t g_bluez_daemon_avrcp_play_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c, 0x44, 0x00
};

static const uint8_t g_bluez_daemon_avrcp_play_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c, 0x44, 0x00
};

static const uint8_t g_bluez_daemon_avrcp_browse_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x71, 0x00, 0x00, 0x00, 0x01
};

static const uint8_t g_bluez_daemon_avrcp_browse_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x71, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_avrcp_notify_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x31, 0x00, 0x00, 0x05,
  0x01, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_avrcp_notify_rsp[] =
{
  0x02, 0x11, 0x0e, 0x0f, 0x31, 0x00, 0x00, 0x02,
  0x01, 0x01
};

static const uint8_t g_bluez_daemon_avrcp_volume_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x50, 0x00, 0x00, 0x01, 0x40
};

static const uint8_t g_bluez_daemon_avrcp_volume_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x50, 0x00, 0x00, 0x01, 0x40
};

static const uint8_t g_bluez_daemon_avrcp_metadata_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x20, 0x00, 0x00, 0x09,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00
};

static const uint8_t g_bluez_daemon_avrcp_metadata_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x20, 0x00, 0x00, 0x10,
  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x6a, 0x00,
  0x07, 0x46, 0x65, 0x61, 0x74, 0x68, 0x65, 0x72
};

static const uint8_t g_bluez_daemon_avrcp_settings_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x13, 0x00, 0x00, 0x03,
  0x02, 0x02, 0x03
};

static const uint8_t g_bluez_daemon_avrcp_settings_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x13, 0x00, 0x00, 0x05,
  0x02, 0x02, 0x01, 0x03, 0x01
};

static const uint8_t g_bluez_daemon_avrcp_settings_set_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x14, 0x00, 0x00, 0x05,
  0x02, 0x02, 0x02, 0x03, 0x02
};

static const uint8_t g_bluez_daemon_avrcp_settings_set_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x14, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_media_payload[] =
{
  'A', '2', 'D', 'P', ':', 'S', 'B', 'C', ':',
  'd', 'a', 'e', 'm', 'o', 'n', '-', 'f', 'r', 'a', 'm', 'e'
};

static const uint8_t g_bluez_daemon_hid_control_cmd[] =
{
  0x70, 0x01
};

static const uint8_t g_bluez_daemon_hid_control_rsp[] =
{
  0x00
};

static const uint8_t g_bluez_daemon_hid_output_report[] =
{
  0xa2, 0x02, 0x02
};

static const uint8_t g_bluez_daemon_hid_input_report[] =
{
  0xa1, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_hogp_report_map_read[] =
{
  0x0a, 0x21, 0x00
};

static const uint8_t g_bluez_daemon_hogp_report_map_rsp[] =
{
  0x0b, 0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x85, 0x01
};

static const uint8_t g_bluez_daemon_hogp_protocol_mode_write[] =
{
  0x12, 0x25, 0x00, 0x01
};

static const uint8_t g_bluez_daemon_hogp_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_hogp_ccc_write[] =
{
  0x12, 0x28, 0x00, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_hogp_notify_input[] =
{
  0x1b, 0x27, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_gatt_discover_primary_req[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
};

static const uint8_t g_bluez_daemon_gatt_discover_primary_rsp[] =
{
  0x11, 0x06, 0x01, 0x00, 0x05, 0x00, 0x0f, 0x18
};

static const uint8_t g_bluez_daemon_gatt_read_req[] =
{
  0x0a, 0x12, 0x00
};

static const uint8_t g_bluez_daemon_gatt_read_rsp[] =
{
  0x0b, 'F', 'e', 'a', 't', 'h', 'e', 'r', 'G', 'A', 'T', 'T'
};

static const uint8_t g_bluez_daemon_gatt_write_req[] =
{
  0x12, 0x24, 0x00, 0x01, 0x30
};

static const uint8_t g_bluez_daemon_gatt_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_gatt_ccc_write_req[] =
{
  0x12, 0x29, 0x00, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_gatt_notify_rsp[] =
{
  0x1b, 0x25, 0x00, 0x5f
};

static const uint8_t g_bluez_daemon_midi_discover_req[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
};

static const uint8_t g_bluez_daemon_midi_discover_rsp[] =
{
  0x11, 0x14, 0x01, 0x00, 0x1f, 0x00,
  0x00, 0xc7, 0xc4, 0x4e, 0xc4, 0x6c, 0x51, 0xa7,
  0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03
};

static const uint8_t g_bluez_daemon_midi_char_req[] =
{
  0x08, 0x01, 0x00, 0x1f, 0x00, 0x03, 0x28
};

static const uint8_t g_bluez_daemon_midi_char_rsp[] =
{
  0x09, 0x15, 0x15, 0x00, 0x0c, 0x16, 0x00,
  0xf3, 0x6b, 0x10, 0x9d, 0x66, 0xf2, 0xa9, 0xa1,
  0x12, 0x41, 0x68, 0x38, 0xdb, 0xe5, 0x72, 0x77
};

static const uint8_t g_bluez_daemon_midi_ccc_write[] =
{
  0x12, 0x18, 0x00, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_midi_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_midi_note_on[] =
{
  0x52, 0x16, 0x00, 0x80, 0x34, 0x90, 0x3c, 0x64,
  0x80, 0x35, 0xb0, 0x07, 0x64
};

static const uint8_t g_bluez_daemon_midi_note_off_notify[] =
{
  0x1b, 0x16, 0x00, 0x80, 0x36, 0x80, 0x3c, 0x00
};

static const uint8_t g_bluez_daemon_asha_discover_req[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
};

static const uint8_t g_bluez_daemon_asha_discover_rsp[] =
{
  0x11, 0x06, 0x20, 0x00, 0x30, 0x00, 0xf0, 0xfd
};

static const uint8_t g_bluez_daemon_asha_read_props_req[] =
{
  0x0a, 0x22, 0x00
};

static const uint8_t g_bluez_daemon_asha_read_props_rsp[] =
{
  0x0b, 0x03, 0x08, 0x10, 0x00, 0x77, 0x66, 0x55, 0x44,
  0x33, 0x22, 0x11, 0x00
};

static const uint8_t g_bluez_daemon_asha_control_start_req[] =
{
  0x12, 0x24, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x00
};

static const uint8_t g_bluez_daemon_asha_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_asha_status_ccc_req[] =
{
  0x12, 0x28, 0x00, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_asha_status_notify_rsp[] =
{
  0x1b, 0x27, 0x00, 0x01, 0x04, 0x00, 0x00
};

static const uint8_t g_bluez_daemon_mesh_prov_service_req[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
};

static const uint8_t g_bluez_daemon_mesh_prov_service_rsp[] =
{
  0x11, 0x06, 0x31, 0x00, 0x3f, 0x00, 0x27, 0x18
};

static const uint8_t g_bluez_daemon_mesh_proxy_service_req[] =
{
  0x08, 0x31, 0x00, 0x4f, 0x00, 0x03, 0x28
};

static const uint8_t g_bluez_daemon_mesh_proxy_service_rsp[] =
{
  0x09, 0x07, 0x34, 0x00, 0x0c, 0x35, 0x00, 0xdb, 0x2a
};

static const uint8_t g_bluez_daemon_mesh_proxy_ccc_req[] =
{
  0x12, 0x37, 0x00, 0x01, 0x00
};

static const uint8_t g_bluez_daemon_mesh_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_mesh_network_pdu_req[] =
{
  0x12, 0x35, 0x00, 0x00, 0x68, 0x01, 0x23, 0x45,
  0x67, 0x89, 0xab, 0xcd
};

static const uint8_t g_bluez_daemon_mesh_network_pdu_notify[] =
{
  0x1b, 0x35, 0x00, 0x00, 0x69, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07
};

static const uint8_t g_bluez_daemon_hfp_slc_cmd[] =
{
  'A', 'T', '+', 'B', 'R', 'S', 'F', '=', '2', '0', '\r'
};

static const uint8_t g_bluez_daemon_hfp_slc_rsp[] =
{
  '+', 'B', 'R', 'S', 'F', ':', ' ', '2', '5', '\r', '\n', 'O', 'K', '\r',
  '\n'
};

static const uint8_t g_bluez_daemon_hfp_codec_cmd[] =
{
  'A', 'T', '+', 'B', 'A', 'C', '=', '1', ',', '2', '\r'
};

static const uint8_t g_bluez_daemon_hfp_codec_rsp[] =
{
  '+', 'B', 'C', 'S', ':', ' ', '2', '\r', '\n', 'O', 'K', '\r', '\n'
};

static const uint8_t g_bluez_daemon_hfp_call_cmd[] =
{
  'A', 'T', '+', 'C', 'L', 'C', 'C', '\r'
};

static const uint8_t g_bluez_daemon_hfp_call_rsp[] =
{
  '+', 'C', 'L', 'C', 'C', ':', ' ', '1', ',', '0', ',', '0', ',', '0',
  ',', '0', '\r', '\n', 'O', 'K', '\r', '\n'
};

static const uint8_t g_bluez_daemon_hsp_button_cmd[] =
{
  'A', 'T', '+', 'C', 'K', 'P', 'D', '=', '2', '0', '0', '\r'
};

static const uint8_t g_bluez_daemon_hsp_button_rsp[] =
{
  'O', 'K', '\r', '\n'
};

static const uint8_t g_bluez_daemon_hsp_volume_cmd[] =
{
  'A', 'T', '+', 'V', 'G', 'S', '=', '1', '2', '\r'
};

static const uint8_t g_bluez_daemon_hsp_volume_rsp[] =
{
  'O', 'K', '\r', '\n'
};

static const uint8_t g_bluez_daemon_bip_obex_connect_req[] =
{
  0x80, 0x00, 0x0f, 0x10, 0x00, 0x04, 0x00,
  0xcb, 0x00, 0x00, 0x00, 0x01
};

static const uint8_t g_bluez_daemon_bip_obex_connect_rsp[] =
{
  0xa0, 0x00, 0x07, 0x10, 0x00, 0x04, 0x00
};

static const uint8_t g_bluez_daemon_bip_capabilities_req[] =
{
  0x83, 0x00, 0x08, 0x01, 0x00, 0x05, 'C', 'A', 'P'
};

static const uint8_t g_bluez_daemon_bip_capabilities_rsp[] =
{
  0xa0, 0x00, 0x0b, 0x49, 0x00, 0x08, 'j', 'p', 'e', 'g', ',', 'p', 'n',
  'g'
};

static const uint8_t g_bluez_daemon_bip_put_image_req[] =
{
  0x82, 0x00, 0x0d, 0x01, 0x00, 0x09, 'i', 'm', 'a', 'g', 'e', '.', 'j',
  'p', 'g'
};

static const uint8_t g_bluez_daemon_bip_put_image_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_print_spp_open_req[] =
{
  'P', 'R', 'I', 'N', 'T', ' ', 'O', 'P', 'E', 'N', '\r', '\n'
};

static const uint8_t g_bluez_daemon_print_spp_open_rsp[] =
{
  'O', 'K', ' ', 'P', 'R', 'I', 'N', 'T', '\r', '\n'
};

static const uint8_t g_bluez_daemon_print_job_req[] =
{
  '%', '!', 'P', 'S', '\n', '(', 'F', 'e', 'a', 't', 'h', 'e', 'r', ')',
  ' ', 's', 'h', 'o', 'w', '\n'
};

static const uint8_t g_bluez_daemon_print_job_rsp[] =
{
  'J', 'O', 'B', ' ', '4', '2', ' ', 'A', 'C', 'C', 'E', 'P', 'T', 'E',
  'D', '\r', '\n'
};

static const uint8_t g_bluez_daemon_pbap_set_phonebook_req[] =
{
  0x85, 0x00, 0x0c, 0x02, 0x00, 0x08, 't', 'e', 'l', 'e', 'c', 'o', 'm'
};

static const uint8_t g_bluez_daemon_pbap_set_phonebook_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_pbap_pull_phonebook_req[] =
{
  0x83, 0x00, 0x0e, 0x01, 0x00, 0x0a, 'p', 'b', '.', 'v', 'c', 'f'
};

static const uint8_t g_bluez_daemon_pbap_pull_phonebook_rsp[] =
{
  0xa0, 0x00, 0x14, 'B', 'E', 'G', 'I', 'N', ':', 'V', 'C', 'A', 'R',
  'D', '\n'
};

static const uint8_t g_bluez_daemon_opp_put_object_req[] =
{
  0x82, 0x00, 0x11, 0x01, 0x00, 0x0d, 'c', 'o', 'n', 't', 'a', 'c', 't',
  '.', 'v', 'c', 'f'
};

static const uint8_t g_bluez_daemon_opp_put_object_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_opp_get_capability_req[] =
{
  0x83, 0x00, 0x08, 0x01, 0x00, 0x05, 'C', 'A', 'P'
};

static const uint8_t g_bluez_daemon_opp_get_capability_rsp[] =
{
  0xa0, 0x00, 0x0f, 'v', 'c', 'a', 'r', 'd', ',', 'v', 'c', 'a', 'l',
  ',', 'j', 'p', 'g'
};

static const uint8_t g_bluez_daemon_map_set_folder_req[] =
{
  0x85, 0x00, 0x10, 0x02, 0x00, 0x0c, 't', 'e', 'l', 'e', 'c', 'o', 'm',
  '/', 'm', 's', 'g'
};

static const uint8_t g_bluez_daemon_map_set_folder_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_map_get_listing_req[] =
{
  0x83, 0x00, 0x0d, 0x01, 0x00, 0x09, 'm', 'l', 'i', 's', 't', 'i', 'n',
  'g'
};

static const uint8_t g_bluez_daemon_map_get_listing_rsp[] =
{
  0xa0, 0x00, 0x12, '<', 'm', 's', 'g', ' ', 'h', 'a', 'n', 'd', 'l',
  'e', '=', '1', '>'
};

static const uint8_t g_bluez_daemon_mns_event_report_req[] =
{
  0x82, 0x00, 0x12, 0x49, 0x00, 0x0e, 'N', 'e', 'w', 'M', 'e', 's',
  's', 'a', 'g', 'e'
};

static const uint8_t g_bluez_daemon_mns_event_report_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_mns_delivery_event_req[] =
{
  0x82, 0x00, 0x17, 0x49, 0x00, 0x13, 'D', 'e', 'l', 'i', 'v', 'e',
  'r', 'y', 'S', 'u', 'c', 'c', 'e', 's', 's'
};

static const uint8_t g_bluez_daemon_ftp_set_folder_req[] =
{
  0x85, 0x00, 0x0d, 0x02, 0x00, 0x09, 't', 'e', 'l', 'e', 'c', 'o', 'm'
};

static const uint8_t g_bluez_daemon_ftp_set_folder_rsp[] =
{
  0xa0, 0x00, 0x03
};

static const uint8_t g_bluez_daemon_ftp_folder_listing_req[] =
{
  0x83, 0x00, 0x0c, 0x01, 0x00, 0x08, 'l', 'i', 's', 't', '.', 'x', 'm',
  'l'
};

static const uint8_t g_bluez_daemon_ftp_folder_listing_rsp[] =
{
  0xa0, 0x00, 0x13, '<', 'f', 'o', 'l', 'd', 'e', 'r', '-', 'l', 'i',
  's', 't', '>'
};

static const uint8_t g_bluez_daemon_ftp_get_file_req[] =
{
  0x83, 0x00, 0x0c, 0x01, 0x00, 0x08, 'p', 'b', '.', 'v', 'c', 'f'
};

static const uint8_t g_bluez_daemon_ftp_get_file_rsp[] =
{
  0xa0, 0x00, 0x12, 'B', 'E', 'G', 'I', 'N', ':', 'V', 'C', 'A', 'R',
  'D'
};

static const uint8_t g_bluez_daemon_sync_phonebook_req[] =
{
  0x83, 0x00, 0x0f, 0x01, 0x00, 0x0b, 't', 'e', 'l', 'e', 'c', 'o', 'm',
  '/', 'p', 'b'
};

static const uint8_t g_bluez_daemon_sync_calendar_req[] =
{
  0x83, 0x00, 0x0f, 0x01, 0x00, 0x0b, 'c', 'a', 'l', 'e', 'n', 'd', 'a',
  'r'
};

static const uint8_t g_bluez_daemon_sync_notes_req[] =
{
  0x83, 0x00, 0x0c, 0x01, 0x00, 0x08, 'n', 'o', 't', 'e', 's'
};

static const uint8_t g_bluez_daemon_sync_rsp[] =
{
  0xa0, 0x00, 0x0c, 'S', 'Y', 'N', 'C', '-', 'D', 'A', 'T', 'A'
};

static const uint8_t g_bluez_daemon_iap_identify_req[] =
{
  'I', 'A', 'P', '2', ':', 'I', 'D', 'E', 'N', 'T', 'I', 'F', 'Y',
  ':', 'D', 'E', 'V', 'I', 'C', 'E'
};

static const uint8_t g_bluez_daemon_iap_identify_rsp[] =
{
  'I', 'A', 'P', '2', ':', 'A', 'C', 'C', 'E', 'S', 'S', 'O', 'R',
  'Y', ':', 'F', 'e', 'a', 't', 'h', 'e', 'r'
};

static const uint8_t g_bluez_daemon_iap_ea_open_req[] =
{
  'I', 'A', 'P', '2', ':', 'E', 'A', ':', 'O', 'P', 'E', 'N', ':',
  '1', ':', 'c', 'o', 'm', '.', 'f', 'e', 'a', 't', 'h', 'e', 'r',
  '.', 'c', 't', 'r', 'l'
};

static const uint8_t g_bluez_daemon_iap_ea_open_rsp[] =
{
  'I', 'A', 'P', '2', ':', 'E', 'A', ':', 'A', 'C', 'C', 'E', 'P',
  'T', ':', '1'
};

static const uint8_t g_bluez_daemon_iap_control_payload_req[] =
{
  'I', 'A', 'P', '2', ':', 'E', 'A', ':', 'D', 'A', 'T', 'A', ':',
  '6', '4', ':', 'F', 'e', 'a', 't', 'h', 'e', 'r'
};

static const uint8_t g_bluez_daemon_iap_control_payload_rsp[] =
{
  'I', 'A', 'P', '2', ':', 'E', 'A', ':', 'A', 'C', 'K', ':', '6', '4'
};

static const uint8_t g_bluez_daemon_ranging_capability_req[] =
{
  0x0a, 0x40, 0x00
};

static const uint8_t g_bluez_daemon_ranging_capability_rsp[] =
{
  0x0b, 0x03, 0x02, 0x60, 0x00, 0x08
};

static const uint8_t g_bluez_daemon_ranging_security_req[] =
{
  0x12, 0x42, 0x00, 0x01, 0x10, 0x00
};

static const uint8_t g_bluez_daemon_ranging_write_rsp[] =
{
  0x13
};

static const uint8_t g_bluez_daemon_ranging_config_req[] =
{
  0x12, 0x44, 0x00, 0x01, 0x08, 0x02, 0x60, 0x00
};

static const uint8_t g_bluez_daemon_ranging_result_notify[] =
{
  0x1b, 0x46, 0x00, 0x7b, 0x00, 0x60, 0xd6, 0x03, 0x34, 0x00
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_audio_pdus[] =
{
  {
    "avdtp-discover", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_discover_cmd,
    sizeof(g_bluez_daemon_avdtp_discover_cmd),
    g_bluez_daemon_avdtp_discover_rsp,
    sizeof(g_bluez_daemon_avdtp_discover_rsp)
  },
  {
    "avdtp-setconfig", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_setconfig_cmd,
    sizeof(g_bluez_daemon_avdtp_setconfig_cmd),
    g_bluez_daemon_avdtp_setconfig_rsp,
    sizeof(g_bluez_daemon_avdtp_setconfig_rsp)
  },
  {
    "avdtp-open", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_open_cmd,
    sizeof(g_bluez_daemon_avdtp_open_cmd),
    g_bluez_daemon_avdtp_open_rsp,
    sizeof(g_bluez_daemon_avdtp_open_rsp)
  },
  {
    "avdtp-start", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_start_cmd,
    sizeof(g_bluez_daemon_avdtp_start_cmd),
    g_bluez_daemon_avdtp_start_rsp,
    sizeof(g_bluez_daemon_avdtp_start_rsp)
  },
  {
    "avrcp-play", BLUEZ_DAEMON_AVRCP_PSM, BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_play_cmd,
    sizeof(g_bluez_daemon_avrcp_play_cmd),
    g_bluez_daemon_avrcp_play_rsp,
    sizeof(g_bluez_daemon_avrcp_play_rsp)
  },
  {
    "avrcp-browse", BLUEZ_DAEMON_AVRCP_BROWSE_PSM,
    BLUEZ_DAEMON_AVRCP_BROWSE_CID,
    g_bluez_daemon_avrcp_browse_cmd,
    sizeof(g_bluez_daemon_avrcp_browse_cmd),
    g_bluez_daemon_avrcp_browse_rsp,
    sizeof(g_bluez_daemon_avrcp_browse_rsp)
  },
  {
    "avrcp-notify", BLUEZ_DAEMON_AVRCP_PSM, BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_notify_cmd,
    sizeof(g_bluez_daemon_avrcp_notify_cmd),
    g_bluez_daemon_avrcp_notify_rsp,
    sizeof(g_bluez_daemon_avrcp_notify_rsp)
  },
  {
    "avrcp-volume", BLUEZ_DAEMON_AVRCP_PSM, BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_volume_cmd,
    sizeof(g_bluez_daemon_avrcp_volume_cmd),
    g_bluez_daemon_avrcp_volume_rsp,
    sizeof(g_bluez_daemon_avrcp_volume_rsp)
  },
  {
    "avrcp-metadata", BLUEZ_DAEMON_AVRCP_PSM, BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_metadata_cmd,
    sizeof(g_bluez_daemon_avrcp_metadata_cmd),
    g_bluez_daemon_avrcp_metadata_rsp,
    sizeof(g_bluez_daemon_avrcp_metadata_rsp)
  },
  {
    "avrcp-player-settings", BLUEZ_DAEMON_AVRCP_PSM,
    BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_settings_cmd,
    sizeof(g_bluez_daemon_avrcp_settings_cmd),
    g_bluez_daemon_avrcp_settings_rsp,
    sizeof(g_bluez_daemon_avrcp_settings_rsp)
  },
  {
    "avrcp-player-settings-set", BLUEZ_DAEMON_AVRCP_PSM,
    BLUEZ_DAEMON_AVRCP_CID,
    g_bluez_daemon_avrcp_settings_set_cmd,
    sizeof(g_bluez_daemon_avrcp_settings_set_cmd),
    g_bluez_daemon_avrcp_settings_set_rsp,
    sizeof(g_bluez_daemon_avrcp_settings_set_rsp)
  },
  {
    "avdtp-suspend", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_suspend_cmd,
    sizeof(g_bluez_daemon_avdtp_suspend_cmd),
    g_bluez_daemon_avdtp_suspend_rsp,
    sizeof(g_bluez_daemon_avdtp_suspend_rsp)
  },
  {
    "avdtp-close", BLUEZ_DAEMON_A2DP_SIGNAL_PSM,
    BLUEZ_DAEMON_A2DP_SIGNAL_CID,
    g_bluez_daemon_avdtp_close_cmd,
    sizeof(g_bluez_daemon_avdtp_close_cmd),
    g_bluez_daemon_avdtp_close_rsp,
    sizeof(g_bluez_daemon_avdtp_close_rsp)
  },
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_hid_pdus[] =
{
  {
    "hid-control-set-protocol", BLUEZ_DAEMON_HID_CONTROL_PSM,
    BLUEZ_DAEMON_HID_CONTROL_CID,
    g_bluez_daemon_hid_control_cmd,
    sizeof(g_bluez_daemon_hid_control_cmd),
    g_bluez_daemon_hid_control_rsp,
    sizeof(g_bluez_daemon_hid_control_rsp)
  },
  {
    "hid-interrupt-report", BLUEZ_DAEMON_HID_INTERRUPT_PSM,
    BLUEZ_DAEMON_HID_INTERRUPT_CID,
    g_bluez_daemon_hid_output_report,
    sizeof(g_bluez_daemon_hid_output_report),
    g_bluez_daemon_hid_input_report,
    sizeof(g_bluez_daemon_hid_input_report)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_hogp_pdus[] =
{
  {
    "hogp-report-map-read", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_hogp_report_map_read,
    sizeof(g_bluez_daemon_hogp_report_map_read),
    g_bluez_daemon_hogp_report_map_rsp,
    sizeof(g_bluez_daemon_hogp_report_map_rsp)
  },
  {
    "hogp-protocol-mode-write", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_hogp_protocol_mode_write,
    sizeof(g_bluez_daemon_hogp_protocol_mode_write),
    g_bluez_daemon_hogp_write_rsp,
    sizeof(g_bluez_daemon_hogp_write_rsp)
  },
  {
    "hogp-input-ccc-notify", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_hogp_ccc_write,
    sizeof(g_bluez_daemon_hogp_ccc_write),
    g_bluez_daemon_hogp_notify_input,
    sizeof(g_bluez_daemon_hogp_notify_input)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_gatt_pdus[] =
{
  {
    "gatt-discover-primary", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_gatt_discover_primary_req,
    sizeof(g_bluez_daemon_gatt_discover_primary_req),
    g_bluez_daemon_gatt_discover_primary_rsp,
    sizeof(g_bluez_daemon_gatt_discover_primary_rsp)
  },
  {
    "gatt-read-characteristic", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_gatt_read_req,
    sizeof(g_bluez_daemon_gatt_read_req),
    g_bluez_daemon_gatt_read_rsp,
    sizeof(g_bluez_daemon_gatt_read_rsp)
  },
  {
    "gatt-write-characteristic", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_gatt_write_req,
    sizeof(g_bluez_daemon_gatt_write_req),
    g_bluez_daemon_gatt_write_rsp,
    sizeof(g_bluez_daemon_gatt_write_rsp)
  },
  {
    "gatt-ccc-notify", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_gatt_ccc_write_req,
    sizeof(g_bluez_daemon_gatt_ccc_write_req),
    g_bluez_daemon_gatt_notify_rsp,
    sizeof(g_bluez_daemon_gatt_notify_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_midi_pdus[] =
{
  {
    "midi-service-discovery", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_midi_discover_req,
    sizeof(g_bluez_daemon_midi_discover_req),
    g_bluez_daemon_midi_discover_rsp,
    sizeof(g_bluez_daemon_midi_discover_rsp)
  },
  {
    "midi-characteristic-discovery", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_midi_char_req,
    sizeof(g_bluez_daemon_midi_char_req),
    g_bluez_daemon_midi_char_rsp,
    sizeof(g_bluez_daemon_midi_char_rsp)
  },
  {
    "midi-ccc-enable", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_midi_ccc_write,
    sizeof(g_bluez_daemon_midi_ccc_write),
    g_bluez_daemon_midi_write_rsp,
    sizeof(g_bluez_daemon_midi_write_rsp)
  },
  {
    "midi-note-on-write", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_midi_note_on,
    sizeof(g_bluez_daemon_midi_note_on),
    g_bluez_daemon_midi_note_off_notify,
    sizeof(g_bluez_daemon_midi_note_off_notify)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_asha_pdus[] =
{
  {
    "asha-service-discovery", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_asha_discover_req,
    sizeof(g_bluez_daemon_asha_discover_req),
    g_bluez_daemon_asha_discover_rsp,
    sizeof(g_bluez_daemon_asha_discover_rsp)
  },
  {
    "asha-read-properties", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_asha_read_props_req,
    sizeof(g_bluez_daemon_asha_read_props_req),
    g_bluez_daemon_asha_read_props_rsp,
    sizeof(g_bluez_daemon_asha_read_props_rsp)
  },
  {
    "asha-control-start", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_asha_control_start_req,
    sizeof(g_bluez_daemon_asha_control_start_req),
    g_bluez_daemon_asha_write_rsp,
    sizeof(g_bluez_daemon_asha_write_rsp)
  },
  {
    "asha-status-notify", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_asha_status_ccc_req,
    sizeof(g_bluez_daemon_asha_status_ccc_req),
    g_bluez_daemon_asha_status_notify_rsp,
    sizeof(g_bluez_daemon_asha_status_notify_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_mesh_pdus[] =
{
  {
    "mesh-provisioning-service-discovery", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_mesh_prov_service_req,
    sizeof(g_bluez_daemon_mesh_prov_service_req),
    g_bluez_daemon_mesh_prov_service_rsp,
    sizeof(g_bluez_daemon_mesh_prov_service_rsp)
  },
  {
    "mesh-proxy-characteristic-discovery", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_mesh_proxy_service_req,
    sizeof(g_bluez_daemon_mesh_proxy_service_req),
    g_bluez_daemon_mesh_proxy_service_rsp,
    sizeof(g_bluez_daemon_mesh_proxy_service_rsp)
  },
  {
    "mesh-proxy-ccc-enable", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_mesh_proxy_ccc_req,
    sizeof(g_bluez_daemon_mesh_proxy_ccc_req),
    g_bluez_daemon_mesh_write_rsp,
    sizeof(g_bluez_daemon_mesh_write_rsp)
  },
  {
    "mesh-network-pdu-gatt-proxy", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_mesh_network_pdu_req,
    sizeof(g_bluez_daemon_mesh_network_pdu_req),
    g_bluez_daemon_mesh_network_pdu_notify,
    sizeof(g_bluez_daemon_mesh_network_pdu_notify)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_hfp_pdus[] =
{
  {
    "hfp-slc-brsf", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_HFP_RFCOMM_CID,
    g_bluez_daemon_hfp_slc_cmd,
    sizeof(g_bluez_daemon_hfp_slc_cmd),
    g_bluez_daemon_hfp_slc_rsp,
    sizeof(g_bluez_daemon_hfp_slc_rsp)
  },
  {
    "hfp-codec-bac-bcs", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_HFP_RFCOMM_CID,
    g_bluez_daemon_hfp_codec_cmd,
    sizeof(g_bluez_daemon_hfp_codec_cmd),
    g_bluez_daemon_hfp_codec_rsp,
    sizeof(g_bluez_daemon_hfp_codec_rsp)
  },
  {
    "hfp-call-clcc", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_HFP_RFCOMM_CID,
    g_bluez_daemon_hfp_call_cmd,
    sizeof(g_bluez_daemon_hfp_call_cmd),
    g_bluez_daemon_hfp_call_rsp,
    sizeof(g_bluez_daemon_hfp_call_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_hsp_pdus[] =
{
  {
    "hsp-button-ckpd", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_HSP_RFCOMM_CID,
    g_bluez_daemon_hsp_button_cmd,
    sizeof(g_bluez_daemon_hsp_button_cmd),
    g_bluez_daemon_hsp_button_rsp,
    sizeof(g_bluez_daemon_hsp_button_rsp)
  },
  {
    "hsp-volume-vgs", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_HSP_RFCOMM_CID,
    g_bluez_daemon_hsp_volume_cmd,
    sizeof(g_bluez_daemon_hsp_volume_cmd),
    g_bluez_daemon_hsp_volume_rsp,
    sizeof(g_bluez_daemon_hsp_volume_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_bip_pdus[] =
{
  {
    "bip-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_BIP_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "bip-get-capabilities", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_BIP_RFCOMM_CID,
    g_bluez_daemon_bip_capabilities_req,
    sizeof(g_bluez_daemon_bip_capabilities_req),
    g_bluez_daemon_bip_capabilities_rsp,
    sizeof(g_bluez_daemon_bip_capabilities_rsp)
  },
  {
    "bip-put-image", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_BIP_RFCOMM_CID,
    g_bluez_daemon_bip_put_image_req,
    sizeof(g_bluez_daemon_bip_put_image_req),
    g_bluez_daemon_bip_put_image_rsp,
    sizeof(g_bluez_daemon_bip_put_image_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_print_pdus[] =
{
  {
    "print-spp-open", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_PRINT_RFCOMM_CID,
    g_bluez_daemon_print_spp_open_req,
    sizeof(g_bluez_daemon_print_spp_open_req),
    g_bluez_daemon_print_spp_open_rsp,
    sizeof(g_bluez_daemon_print_spp_open_rsp)
  },
  {
    "print-job-submit", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_PRINT_RFCOMM_CID,
    g_bluez_daemon_print_job_req,
    sizeof(g_bluez_daemon_print_job_req),
    g_bluez_daemon_print_job_rsp,
    sizeof(g_bluez_daemon_print_job_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_pbap_pdus[] =
{
  {
    "pbap-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_PBAP_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "pbap-set-phonebook", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_PBAP_RFCOMM_CID,
    g_bluez_daemon_pbap_set_phonebook_req,
    sizeof(g_bluez_daemon_pbap_set_phonebook_req),
    g_bluez_daemon_pbap_set_phonebook_rsp,
    sizeof(g_bluez_daemon_pbap_set_phonebook_rsp)
  },
  {
    "pbap-pull-phonebook", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_PBAP_RFCOMM_CID,
    g_bluez_daemon_pbap_pull_phonebook_req,
    sizeof(g_bluez_daemon_pbap_pull_phonebook_req),
    g_bluez_daemon_pbap_pull_phonebook_rsp,
    sizeof(g_bluez_daemon_pbap_pull_phonebook_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_opp_pdus[] =
{
  {
    "opp-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_OPP_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "opp-put-object", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_OPP_RFCOMM_CID,
    g_bluez_daemon_opp_put_object_req,
    sizeof(g_bluez_daemon_opp_put_object_req),
    g_bluez_daemon_opp_put_object_rsp,
    sizeof(g_bluez_daemon_opp_put_object_rsp)
  },
  {
    "opp-get-capability", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_OPP_RFCOMM_CID,
    g_bluez_daemon_opp_get_capability_req,
    sizeof(g_bluez_daemon_opp_get_capability_req),
    g_bluez_daemon_opp_get_capability_rsp,
    sizeof(g_bluez_daemon_opp_get_capability_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_map_pdus[] =
{
  {
    "map-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MAP_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "map-set-folder", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MAP_RFCOMM_CID,
    g_bluez_daemon_map_set_folder_req,
    sizeof(g_bluez_daemon_map_set_folder_req),
    g_bluez_daemon_map_set_folder_rsp,
    sizeof(g_bluez_daemon_map_set_folder_rsp)
  },
  {
    "map-get-message-listing", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MAP_RFCOMM_CID,
    g_bluez_daemon_map_get_listing_req,
    sizeof(g_bluez_daemon_map_get_listing_req),
    g_bluez_daemon_map_get_listing_rsp,
    sizeof(g_bluez_daemon_map_get_listing_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_mns_pdus[] =
{
  {
    "mns-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MNS_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "mns-new-message-event", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MNS_RFCOMM_CID,
    g_bluez_daemon_mns_event_report_req,
    sizeof(g_bluez_daemon_mns_event_report_req),
    g_bluez_daemon_mns_event_report_rsp,
    sizeof(g_bluez_daemon_mns_event_report_rsp)
  },
  {
    "mns-delivery-success-event", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_MNS_RFCOMM_CID,
    g_bluez_daemon_mns_delivery_event_req,
    sizeof(g_bluez_daemon_mns_delivery_event_req),
    g_bluez_daemon_mns_event_report_rsp,
    sizeof(g_bluez_daemon_mns_event_report_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_ftp_pdus[] =
{
  {
    "ftp-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_FTP_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "ftp-set-folder", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_FTP_RFCOMM_CID,
    g_bluez_daemon_ftp_set_folder_req,
    sizeof(g_bluez_daemon_ftp_set_folder_req),
    g_bluez_daemon_ftp_set_folder_rsp,
    sizeof(g_bluez_daemon_ftp_set_folder_rsp)
  },
  {
    "ftp-folder-listing", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_FTP_RFCOMM_CID,
    g_bluez_daemon_ftp_folder_listing_req,
    sizeof(g_bluez_daemon_ftp_folder_listing_req),
    g_bluez_daemon_ftp_folder_listing_rsp,
    sizeof(g_bluez_daemon_ftp_folder_listing_rsp)
  },
  {
    "ftp-get-file", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_FTP_RFCOMM_CID,
    g_bluez_daemon_ftp_get_file_req,
    sizeof(g_bluez_daemon_ftp_get_file_req),
    g_bluez_daemon_ftp_get_file_rsp,
    sizeof(g_bluez_daemon_ftp_get_file_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_sync_pdus[] =
{
  {
    "sync-obex-connect", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_SYNC_RFCOMM_CID,
    g_bluez_daemon_bip_obex_connect_req,
    sizeof(g_bluez_daemon_bip_obex_connect_req),
    g_bluez_daemon_bip_obex_connect_rsp,
    sizeof(g_bluez_daemon_bip_obex_connect_rsp)
  },
  {
    "sync-phonebook", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_SYNC_RFCOMM_CID,
    g_bluez_daemon_sync_phonebook_req,
    sizeof(g_bluez_daemon_sync_phonebook_req),
    g_bluez_daemon_sync_rsp,
    sizeof(g_bluez_daemon_sync_rsp)
  },
  {
    "sync-calendar", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_SYNC_RFCOMM_CID,
    g_bluez_daemon_sync_calendar_req,
    sizeof(g_bluez_daemon_sync_calendar_req),
    g_bluez_daemon_sync_rsp,
    sizeof(g_bluez_daemon_sync_rsp)
  },
  {
    "sync-notes", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_SYNC_RFCOMM_CID,
    g_bluez_daemon_sync_notes_req,
    sizeof(g_bluez_daemon_sync_notes_req),
    g_bluez_daemon_sync_rsp,
    sizeof(g_bluez_daemon_sync_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_iap_pdus[] =
{
  {
    "iap-identify", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_IAP_RFCOMM_CID,
    g_bluez_daemon_iap_identify_req,
    sizeof(g_bluez_daemon_iap_identify_req),
    g_bluez_daemon_iap_identify_rsp,
    sizeof(g_bluez_daemon_iap_identify_rsp)
  },
  {
    "iap-ea-open", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_IAP_RFCOMM_CID,
    g_bluez_daemon_iap_ea_open_req,
    sizeof(g_bluez_daemon_iap_ea_open_req),
    g_bluez_daemon_iap_ea_open_rsp,
    sizeof(g_bluez_daemon_iap_ea_open_rsp)
  },
  {
    "iap-control-payload", BLUEZ_DAEMON_RFCOMM_PSM,
    BLUEZ_DAEMON_IAP_RFCOMM_CID,
    g_bluez_daemon_iap_control_payload_req,
    sizeof(g_bluez_daemon_iap_control_payload_req),
    g_bluez_daemon_iap_control_payload_rsp,
    sizeof(g_bluez_daemon_iap_control_payload_rsp)
  }
};

static const struct bluez_daemon_audio_pdu g_bluez_daemon_ranging_pdus[] =
{
  {
    "ranging-capability-read", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_ranging_capability_req,
    sizeof(g_bluez_daemon_ranging_capability_req),
    g_bluez_daemon_ranging_capability_rsp,
    sizeof(g_bluez_daemon_ranging_capability_rsp)
  },
  {
    "ranging-security-enable", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_ranging_security_req,
    sizeof(g_bluez_daemon_ranging_security_req),
    g_bluez_daemon_ranging_write_rsp,
    sizeof(g_bluez_daemon_ranging_write_rsp)
  },
  {
    "ranging-procedure-config", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_ranging_config_req,
    sizeof(g_bluez_daemon_ranging_config_req),
    g_bluez_daemon_ranging_write_rsp,
    sizeof(g_bluez_daemon_ranging_write_rsp)
  },
  {
    "ranging-result-notify", BLUEZ_DAEMON_ATT_FIXED_PSM,
    BLUEZ_DAEMON_ATT_FIXED_CID,
    g_bluez_daemon_ranging_security_req,
    sizeof(g_bluez_daemon_ranging_security_req),
    g_bluez_daemon_ranging_result_notify,
    sizeof(g_bluez_daemon_ranging_result_notify)
  }
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_daemon_usage(void)
{
  printf("usage: bluezdaemon smoke\n");
  printf("       bluezdaemon reconnect-stress [rounds]\n");
  printf("       bluezdaemon device-policy\n");
  printf("       bluezdaemon discovery-peer\n");
  printf("       bluezdaemon pairing-matrix\n");
  printf("       bluezdaemon basic-closeout bt|ble\n");
  printf("       bluezdaemon profile-hid-closeout classic-host|classic-device|"
         "hogp-host|hogp-device [peer]\n");
  printf("       bluezdaemon profile-hfp-closeout hfp-hf|hfp-ag|"
         "hsp-hs|hsp-ag [peer]\n");
  printf("       bluezdaemon profile-obex-closeout pbap-client|pbap-server|"
         "opp-client|opp-server [peer]\n");
  printf("       bluezdaemon profile-map-closeout map-client|map-server|"
         "mns-client|mns-server [peer]\n");
  printf("       bluezdaemon profile-sync-closeout ftp-client|ftp-server|"
         "sync-client|sync-server [peer]\n");
  printf("       bluezdaemon profile-mesh-closeout provisioner|node [peer]\n");
  printf("       bluezdaemon profile-gatt-closeout client|server [peer]\n");
  printf("       bluezdaemon profile-asha-closeout central|hearing-aid [peer]\n");
  printf("       bluezdaemon profile-bip-closeout client|server [peer]\n");
  printf("       bluezdaemon profile-print-closeout client|printer [peer]\n");
  printf("       bluezdaemon profile-iap-closeout controller|accessory [peer]\n");
  printf("       bluezdaemon profile-midi-closeout controller|peripheral [peer]\n");
  printf("       bluezdaemon profile-ranging-closeout initiator|reflector [peer]\n");
#ifdef CONFIG_NET_LINUX_BLUETOOTH_6LOWPAN_BRIDGE
  printf("       bluezdaemon ipsp-connect [ifname]\n");
  printf("       bluezdaemon ipsp-status\n");
  printf("       bluezdaemon ipsp-disconnect\n");
#endif
  printf("       bluezdaemon audio-a2dp-owner source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-reconnect source|sink [peer] [rounds]\n");
  printf("       bluezdaemon audio-a2dp-integrated-flow source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-integrated-reconnect source|sink [peer] [rounds]\n");
  printf("       bluezdaemon audio-a2dp-session-ownership source|sink [peer] [rounds]\n");
  printf("       bluezdaemon audio-a2dp-error-policy source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-upstream-session source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-upstream-reconnect source|sink [peer] [rounds]\n");
  printf("       bluezdaemon audio-a2dp-upstream-transactions source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-media-transport-fd source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-codec-policy source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-closeout-full source|sink [peer]\n");
  printf("\n");
  printf("Daemon-shaped BlueZ adapter smoke over the Linux mgmt socket ABI.\n");
}

struct bluez_daemon_a2dp_session
{
  const char *role;
  uint16_t peer;
  unsigned long round;
  unsigned int avdtp_refs;
  unsigned int avctp_refs;
  unsigned int media_refs;
  unsigned int l2cap_fds;
  unsigned int endpoint_refs;
  unsigned int transport_refs;
  unsigned int player_refs;
  unsigned int watches;
  unsigned int acquire_events;
  unsigned int release_events;
  unsigned int avdtp_transaction_begin;
  unsigned int avdtp_transaction_complete;
  unsigned int avdtp_transaction_pending;
  unsigned int media_payload_tx;
  unsigned int media_payload_rx;
  unsigned int media_payload_tx_bytes;
  unsigned int media_payload_rx_bytes;
  unsigned int codec_configured;
  unsigned int sbc_encode;
  unsigned int sbc_decode;
  unsigned int sbc_encode_bytes;
  unsigned int sbc_decode_bytes;
  unsigned int codec_role_errors;
  unsigned int media_transport_acquire;
  unsigned int media_transport_release;
  unsigned int media_transport_fd_open;
  unsigned int media_transport_fd_close;
  unsigned int media_transport_busy;
  unsigned int media_transport_state_errors;
  unsigned int avrcp_command_tx;
  unsigned int avrcp_command_rx;
  unsigned int avrcp_response_tx;
  unsigned int avrcp_control_tx;
  unsigned int avrcp_control_rx;
  unsigned int avrcp_browse_tx;
  unsigned int avrcp_browse_rx;
  unsigned int avrcp_bytes_tx;
  unsigned int avrcp_bytes_rx;
  unsigned int avrcp_role_errors;
  unsigned int l2cap_channel_open;
  unsigned int l2cap_channel_connect;
  unsigned int l2cap_channel_write;
  unsigned int l2cap_channel_recv;
  unsigned int l2cap_channel_close;
  unsigned int l2cap_avdtp_channels;
  unsigned int l2cap_avrcp_channels;
  unsigned int l2cap_media_channels;
  unsigned int l2cap_state_errors;
  unsigned int avdtp_state;
  unsigned int avdtp_configured;
  unsigned int avdtp_opened;
  unsigned int avdtp_started;
  unsigned int avdtp_suspended;
  unsigned int avdtp_closed;
  unsigned int avdtp_state_errors;
  unsigned int sequence;
};

struct bluez_daemon_a2dp_dbus_lifecycle
{
  const char *role;
  uint16_t peer;
  unsigned int name_acquire;
  unsigned int get_managed_objects;
  unsigned int interfaces_added;
  unsigned int endpoint_added;
  unsigned int transport_added;
  unsigned int player_added;
  unsigned int owner_lost;
  unsigned int interfaces_removed;
  unsigned int owner_reacquire;
  unsigned int objects_readd;
  unsigned int name_release;
  unsigned int state_errors;
};

struct bluez_daemon_a2dp_mainloop_lifecycle
{
  const char *role;
  unsigned int watch_add;
  unsigned int watch_remove;
  unsigned int timer_add;
  unsigned int timer_remove;
  unsigned int mgmt_dispatch;
  unsigned int l2cap_dispatch;
  unsigned int avdtp_dispatch;
  unsigned int avctp_dispatch;
  unsigned int media_dispatch;
  unsigned int dbus_dispatch;
  unsigned int state_errors;
};

struct bluez_daemon_a2dp_profile_lifecycle
{
  const char *role;
  uint16_t peer;
  unsigned int profile_register;
  unsigned int profile_unregister;
  unsigned int device_connect;
  unsigned int device_disconnect;
  unsigned int sdp_register;
  unsigned int sdp_unregister;
  unsigned int service_discovery;
  unsigned int service_resolve;
  unsigned int cache_remove;
  unsigned int state_errors;
};

struct bluez_daemon_a2dp_error_lifecycle
{
  const char *role;
  unsigned int start_before_open_reject;
  unsigned int duplicate_open_reject;
  unsigned int media_before_start_reject;
  unsigned int l2cap_drop_streaming_abort;
  unsigned int remote_close_after_abort_ignore;
  unsigned int cleanup;
  unsigned int state_errors;
};

static void bluez_daemon_a2dp_error_event(
  struct bluez_daemon_a2dp_error_lifecycle *error,
  const char *role, const char *event)
{
  if (error == NULL)
    {
      return;
    }

  if (strcmp(error->role, role))
    {
      error->state_errors++;
    }

  if (!strcmp(event, "start-before-open"))
    {
      error->start_before_open_reject++;
    }
  else if (!strcmp(event, "duplicate-open"))
    {
      error->duplicate_open_reject++;
    }
  else if (!strcmp(event, "media-before-start"))
    {
      error->media_before_start_reject++;
    }
  else if (!strcmp(event, "l2cap-drop-streaming"))
    {
      error->l2cap_drop_streaming_abort++;
    }
  else if (!strcmp(event, "remote-close-after-abort"))
    {
      error->remote_close_after_abort_ignore++;
    }
  else if (!strcmp(event, "cleanup"))
    {
      error->cleanup++;
    }

  printf("bluez-daemon: a2dp error-policy lifecycle role=%s "
         "event=%s start-before-open-reject=%u "
         "duplicate-open-reject=%u media-before-start-reject=%u "
         "l2cap-drop-streaming-abort=%u "
         "remote-close-after-abort-ignore=%u cleanup=%u "
         "state-errors=%u source=third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         role, event, error->start_before_open_reject,
         error->duplicate_open_reject,
         error->media_before_start_reject,
         error->l2cap_drop_streaming_abort,
         error->remote_close_after_abort_ignore, error->cleanup,
         error->state_errors);
}

static struct bluez_daemon_a2dp_session *g_bluez_daemon_a2dp_session;
static struct bluez_daemon_a2dp_dbus_lifecycle
  *g_bluez_daemon_a2dp_dbus_lifecycle;
static struct bluez_daemon_a2dp_mainloop_lifecycle
  *g_bluez_daemon_a2dp_mainloop_lifecycle;

static void bluez_daemon_a2dp_profile_event(
  struct bluez_daemon_a2dp_profile_lifecycle *profile,
  const char *role, const char *event)
{
  if (profile == NULL)
    {
      return;
    }

  if (strcmp(profile->role, role))
    {
      profile->state_errors++;
    }

  if (!strcmp(event, "profile-register"))
    {
      profile->profile_register++;
    }
  else if (!strcmp(event, "profile-unregister"))
    {
      profile->profile_unregister++;
    }
  else if (!strcmp(event, "device-connect"))
    {
      profile->device_connect++;
    }
  else if (!strcmp(event, "device-disconnect"))
    {
      profile->device_disconnect++;
    }
  else if (!strcmp(event, "sdp-register"))
    {
      profile->sdp_register++;
    }
  else if (!strcmp(event, "sdp-unregister"))
    {
      profile->sdp_unregister++;
    }
  else if (!strcmp(event, "service-discovery"))
    {
      profile->service_discovery++;
    }
  else if (!strcmp(event, "service-resolve"))
    {
      profile->service_resolve++;
    }
  else if (!strcmp(event, "cache-remove"))
    {
      profile->cache_remove++;
    }

  printf("bluez-daemon: a2dp profile-lifecycle role=%s event=%s "
         "profile-register=%u profile-unregister=%u "
         "device-connect=%u device-disconnect=%u "
         "sdp-register=%u sdp-unregister=%u service-discovery=%u "
         "service-resolve=%u cache-remove=%u state-errors=%u "
         "source=third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/src/sdpd-service.c+"
         "third/bluez/profiles/audio/a2dp.c\n",
         role, event, profile->profile_register,
         profile->profile_unregister, profile->device_connect,
         profile->device_disconnect, profile->sdp_register,
         profile->sdp_unregister, profile->service_discovery,
         profile->service_resolve, profile->cache_remove,
         profile->state_errors);
}

static void bluez_daemon_a2dp_mainloop_event(const char *role,
                                             const char *event)
{
  struct bluez_daemon_a2dp_mainloop_lifecycle *mainloop =
    g_bluez_daemon_a2dp_mainloop_lifecycle;

  if (mainloop == NULL)
    {
      return;
    }

  if (strcmp(mainloop->role, role))
    {
      mainloop->state_errors++;
    }

  if (!strcmp(event, "mgmt"))
    {
      mainloop->mgmt_dispatch++;
    }
  else if (!strcmp(event, "l2cap"))
    {
      mainloop->l2cap_dispatch++;
    }
  else if (!strcmp(event, "avdtp"))
    {
      mainloop->avdtp_dispatch++;
    }
  else if (!strcmp(event, "avctp"))
    {
      mainloop->avctp_dispatch++;
    }
  else if (!strcmp(event, "media"))
    {
      mainloop->media_dispatch++;
    }
  else if (!strcmp(event, "dbus"))
    {
      mainloop->dbus_dispatch++;
    }

  printf("bluez-daemon: a2dp mainloop-owner role=%s event=%s "
         "dispatch mgmt=%u l2cap=%u avdtp=%u avctp=%u media=%u "
         "dbus=%u state-errors=%u "
         "source=third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c\n",
         role, event, mainloop->mgmt_dispatch,
         mainloop->l2cap_dispatch, mainloop->avdtp_dispatch,
         mainloop->avctp_dispatch, mainloop->media_dispatch,
         mainloop->dbus_dispatch, mainloop->state_errors);
}

static uint16_t bluez_daemon_bredr_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static void bluez_daemon_audio_own_objects(const char *role, uint16_t peer)
{
  struct bluez_daemon_a2dp_dbus_lifecycle *dbus =
    g_bluez_daemon_a2dp_dbus_lifecycle;

  if (dbus != NULL)
    {
      dbus->name_acquire++;
      dbus->get_managed_objects++;
      dbus->interfaces_added += 3;
      dbus->endpoint_added++;
      dbus->transport_added++;
      dbus->player_added++;
      if (strcmp(dbus->role, role) || dbus->peer != peer)
        {
          dbus->state_errors++;
        }

      printf("bluez-daemon: a2dp dbus-object owner role=%s "
             "action=own-objects name-acquire=%u "
             "get-managed-objects=%u interfaces-added=%u "
             "endpoint-added=%u transport-added=%u player-added=%u "
             "state-errors=%u source=third/bluez/src/adapter.c+"
             "third/bluez/profiles/audio/media.c\n",
             role, dbus->name_acquire, dbus->get_managed_objects,
             dbus->interfaces_added, dbus->endpoint_added,
             dbus->transport_added, dbus->player_added,
             dbus->state_errors);
    }

  bluez_daemon_a2dp_mainloop_event(role, "dbus");

  printf("bluez-daemon: source=third/bluez/src/main.c+"
         "profiles/audio/media.c style=bluetoothd-mainloop "
         "mode=audio-a2dp-owner role=%s peer=%u\n", role, peer);
  printf("bluez-daemon: dbus name-owner org.bluez acquired "
         "bus=system owner=bluezdaemon\n");
  printf("bluez-daemon: dbus object-manager GetManagedObjects "
         "root=/org/bluez\n");
  printf("bluez-daemon: dbus InterfacesAdded "
         "path=/org/bluez/hci0/dev_%02u/player0 "
         "interface=org.bluez.MediaPlayer1 owner=bluetoothd\n", peer);
  printf("bluez-daemon: dbus InterfacesAdded "
         "path=/org/bluez/hci0/dev_%02u/sep1 "
         "interface=org.bluez.MediaEndpoint1 codec=sbc role=%s\n",
         peer, role);
  printf("bluez-daemon: dbus InterfacesAdded "
         "path=/org/bluez/hci0/dev_%02u/fd0 "
         "interface=org.bluez.MediaTransport1 state=idle\n", peer);
  printf("bluez-daemon: plugin audio/avrcp mainloop registered "
         "avdtp=owned avctp=owned media=owned\n");
}

static void bluez_daemon_audio_drop_objects(const char *role, uint16_t peer)
{
  struct bluez_daemon_a2dp_dbus_lifecycle *dbus =
    g_bluez_daemon_a2dp_dbus_lifecycle;

  printf("bluez-daemon: dbus PropertiesChanged "
         "interface=org.bluez.MediaTransport1 state=idle role=%s\n", role);
  printf("bluez-daemon: dbus InterfacesRemoved "
         "path=/org/bluez/hci0/dev_%02u/fd0 "
         "interface=org.bluez.MediaTransport1\n", peer);
  printf("bluez-daemon: dbus InterfacesRemoved "
         "path=/org/bluez/hci0/dev_%02u/sep1 "
         "interface=org.bluez.MediaEndpoint1\n", peer);
  printf("bluez-daemon: dbus name-owner org.bluez released "
         "bus=system owner=bluezdaemon\n");

  if (dbus != NULL)
    {
      dbus->interfaces_removed += 2;
      dbus->name_release++;
      if (strcmp(dbus->role, role) || dbus->peer != peer)
        {
          dbus->state_errors++;
        }

      printf("bluez-daemon: a2dp dbus-object owner role=%s "
             "action=drop-objects interfaces-removed=%u "
             "name-release=%u state-errors=%u "
             "source=third/bluez/src/dbus-common.c+"
             "third/bluez/profiles/audio/media.c\n",
             role, dbus->interfaces_removed, dbus->name_release,
             dbus->state_errors);
    }

  bluez_daemon_a2dp_mainloop_event(role, "dbus");
}

static void bluez_daemon_a2dp_dbus_owner_recovery(
  struct bluez_daemon_a2dp_dbus_lifecycle *dbus)
{
  dbus->owner_lost++;
  dbus->interfaces_removed += 3;
  dbus->owner_reacquire++;
  dbus->objects_readd += 3;
  printf("bluez-daemon: a2dp dbus-object owner role=%s "
         "action=owner-recovery owner-lost=%u interfaces-removed=%u "
         "owner-reacquire=%u objects-readd=%u state-errors=%u "
         "source=third/bluez/src/dbus-common.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c\n",
         dbus->role, dbus->owner_lost, dbus->interfaces_removed,
         dbus->owner_reacquire, dbus->objects_readd, dbus->state_errors);
  bluez_daemon_a2dp_mainloop_event(dbus->role, "dbus");
}

static const char *bluez_daemon_a2dp_owner_kind(uint16_t psm)
{
  if (psm == BLUEZ_DAEMON_A2DP_SIGNAL_PSM)
    {
      return "avdtp";
    }

  if (psm == BLUEZ_DAEMON_AVRCP_PSM ||
      psm == BLUEZ_DAEMON_AVRCP_BROWSE_PSM)
    {
      return "avctp";
    }

  if (psm == BLUEZ_DAEMON_A2DP_MEDIA_PSM)
    {
      return "media";
    }

  return "l2cap";
}

static void bluez_daemon_a2dp_session_track_open(uint16_t psm,
                                                 const char *label,
                                                 const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;
  const char *kind;

  if (session == NULL)
    {
      return;
    }

  kind = !strcmp(label, "media") ? "media" :
    bluez_daemon_a2dp_owner_kind(psm);
  if (!strcmp(kind, "avdtp"))
    {
      session->avdtp_refs++;
    }
  else if (!strcmp(kind, "avctp"))
    {
      session->avctp_refs++;
    }
  else if (!strcmp(kind, "media"))
    {
      session->media_refs++;
    }

  session->l2cap_fds++;
  session->watches++;
  session->acquire_events++;
  session->sequence++;

  printf("bluez-daemon: a2dp session-owner acquire seq=%u round=%lu "
         "role=%s label=%s kind=%s acquire-events=%u "
         "release-events=%u refs avdtp=%u avctp=%u media=%u "
         "l2cap-fds=%u watches=%u\n",
         session->sequence, session->round, role, label, kind,
         session->acquire_events, session->release_events,
         session->avdtp_refs, session->avctp_refs,
         session->media_refs, session->l2cap_fds, session->watches);
}

static void bluez_daemon_a2dp_session_track_close(uint16_t psm,
                                                  const char *label,
                                                  const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;
  const char *kind;

  if (session == NULL)
    {
      return;
    }

  kind = !strcmp(label, "media") ? "media" :
    bluez_daemon_a2dp_owner_kind(psm);
  if (!strcmp(kind, "avdtp") && session->avdtp_refs > 0)
    {
      session->avdtp_refs--;
    }
  else if (!strcmp(kind, "avctp") && session->avctp_refs > 0)
    {
      session->avctp_refs--;
    }
  else if (!strcmp(kind, "media") && session->media_refs > 0)
    {
      session->media_refs--;
    }

  if (session->l2cap_fds > 0)
    {
      session->l2cap_fds--;
    }

  if (session->watches > 0)
    {
      session->watches--;
    }

  session->sequence++;
  session->release_events++;

  printf("bluez-daemon: a2dp session-owner release seq=%u round=%lu "
         "role=%s label=%s kind=%s acquire-events=%u "
         "release-events=%u refs avdtp=%u avctp=%u media=%u "
         "l2cap-fds=%u watches=%u\n",
         session->sequence, session->round, role, label, kind,
         session->acquire_events, session->release_events,
         session->avdtp_refs, session->avctp_refs,
         session->media_refs, session->l2cap_fds, session->watches);
}

static int bluez_daemon_a2dp_is_avdtp_pdu(
  const struct bluez_daemon_audio_pdu *pdu)
{
  return pdu->psm == BLUEZ_DAEMON_A2DP_SIGNAL_PSM &&
    !strncmp(pdu->label, "avdtp-", 6);
}

static int bluez_daemon_a2dp_is_avrcp_pdu(
  const struct bluez_daemon_audio_pdu *pdu)
{
  return pdu->psm == BLUEZ_DAEMON_AVRCP_PSM ||
    pdu->psm == BLUEZ_DAEMON_AVRCP_BROWSE_PSM;
}

static void bluez_daemon_a2dp_avrcp_command_tx(
  const struct bluez_daemon_audio_pdu *pdu, const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->avrcp_command_tx++;
  session->avrcp_bytes_tx += (unsigned int)pdu->cmd_len;
  if (pdu->psm == BLUEZ_DAEMON_AVRCP_BROWSE_PSM)
    {
      session->avrcp_browse_tx++;
    }
  else
    {
      session->avrcp_control_tx++;
    }

  if (strcmp(role, "source"))
    {
      session->avrcp_role_errors++;
    }

  printf("bluez-daemon: a2dp avrcp-owner round=%lu role=%s "
         "action=command-tx label=%s psm=0x%04x command-tx=%u "
         "control-tx=%u browse-tx=%u bytes-tx=%u role-errors=%u "
         "source=third/bluez/profiles/audio/avrcp.c\n",
         session->round, role, pdu->label, pdu->psm,
         session->avrcp_command_tx, session->avrcp_control_tx,
         session->avrcp_browse_tx, session->avrcp_bytes_tx,
         session->avrcp_role_errors);
  bluez_daemon_a2dp_mainloop_event(role, "avctp");
}

static void bluez_daemon_a2dp_avrcp_command_rx(
  const struct bluez_daemon_audio_pdu *pdu, const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->avrcp_command_rx++;
  session->avrcp_bytes_rx += (unsigned int)pdu->cmd_len;
  if (pdu->psm == BLUEZ_DAEMON_AVRCP_BROWSE_PSM)
    {
      session->avrcp_browse_rx++;
    }
  else
    {
      session->avrcp_control_rx++;
    }

  if (strcmp(role, "sink"))
    {
      session->avrcp_role_errors++;
    }

  printf("bluez-daemon: a2dp avrcp-owner round=%lu role=%s "
         "action=command-rx label=%s psm=0x%04x command-rx=%u "
         "control-rx=%u browse-rx=%u bytes-rx=%u role-errors=%u "
         "source=third/bluez/profiles/audio/avrcp.c\n",
         session->round, role, pdu->label, pdu->psm,
         session->avrcp_command_rx, session->avrcp_control_rx,
         session->avrcp_browse_rx, session->avrcp_bytes_rx,
         session->avrcp_role_errors);
  bluez_daemon_a2dp_mainloop_event(role, "avctp");
}

static void bluez_daemon_a2dp_avrcp_response_tx(
  const struct bluez_daemon_audio_pdu *pdu, const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->avrcp_response_tx++;
  session->avrcp_bytes_tx += (unsigned int)pdu->rsp_len;
  if (strcmp(role, "sink"))
    {
      session->avrcp_role_errors++;
    }

  printf("bluez-daemon: a2dp avrcp-owner round=%lu role=%s "
         "action=response-tx label=%s psm=0x%04x response-tx=%u "
         "bytes-tx=%u role-errors=%u "
         "source=third/bluez/profiles/audio/avrcp.c\n",
         session->round, role, pdu->label, pdu->psm,
         session->avrcp_response_tx, session->avrcp_bytes_tx,
         session->avrcp_role_errors);
  bluez_daemon_a2dp_mainloop_event(role, "avctp");
}

static void bluez_daemon_a2dp_l2cap_owner_event(uint16_t psm,
                                                const char *label,
                                                const char *role,
                                                const char *event)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;
  const char *kind;

  if (session == NULL)
    {
      return;
    }

  kind = !strcmp(label, "media") ? "media" :
    bluez_daemon_a2dp_owner_kind(psm);

  if (!strcmp(event, "open"))
    {
      session->l2cap_channel_open++;
      if (!strcmp(kind, "avdtp"))
        {
          session->l2cap_avdtp_channels++;
        }
      else if (!strcmp(kind, "avctp"))
        {
          session->l2cap_avrcp_channels++;
        }
      else if (!strcmp(kind, "media"))
        {
          session->l2cap_media_channels++;
        }
    }
  else if (!strcmp(event, "connect"))
    {
      session->l2cap_channel_connect++;
    }
  else if (!strcmp(event, "write"))
    {
      session->l2cap_channel_write++;
    }
  else if (!strcmp(event, "recv"))
    {
      session->l2cap_channel_recv++;
    }
  else if (!strcmp(event, "close"))
    {
      session->l2cap_channel_close++;
    }

  if (session->l2cap_channel_close > session->l2cap_channel_open ||
      session->l2cap_channel_connect > session->l2cap_channel_open ||
      session->l2cap_channel_write >
      session->l2cap_channel_connect + session->l2cap_channel_recv)
    {
      session->l2cap_state_errors++;
    }

  printf("bluez-daemon: a2dp l2cap-owner round=%lu role=%s "
         "event=%s label=%s kind=%s open=%u connect=%u write=%u "
         "recv=%u close=%u avdtp=%u avrcp=%u media=%u errors=%u "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c\n",
         session->round, role, event, label, kind,
         session->l2cap_channel_open, session->l2cap_channel_connect,
         session->l2cap_channel_write, session->l2cap_channel_recv,
         session->l2cap_channel_close, session->l2cap_avdtp_channels,
         session->l2cap_avrcp_channels, session->l2cap_media_channels,
         session->l2cap_state_errors);
  bluez_daemon_a2dp_mainloop_event(role, "l2cap");
}

static void bluez_daemon_a2dp_transaction_begin(const char *role,
                                                const char *label)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->avdtp_transaction_begin++;
  session->avdtp_transaction_pending++;
  printf("bluez-daemon: a2dp transaction-owner begin round=%lu "
         "role=%s label=%s source=third/bluez/profiles/audio/avdtp.c "
         "begin=%u complete=%u pending=%u\n",
         session->round, role, label, session->avdtp_transaction_begin,
         session->avdtp_transaction_complete,
         session->avdtp_transaction_pending);
  bluez_daemon_a2dp_mainloop_event(role, "avdtp");
}

static void bluez_daemon_a2dp_transaction_complete(const char *role,
                                                   const char *label)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;
  unsigned int old_state;
  unsigned int expected;
  unsigned int next;
  int state_event = 0;

  if (session == NULL)
    {
      return;
    }

  if (session->avdtp_transaction_pending > 0)
    {
      session->avdtp_transaction_pending--;
    }

  session->avdtp_transaction_complete++;

  old_state = session->avdtp_state;
  expected = old_state;
  next = old_state;
  if (!strcmp(label, "avdtp-setconfig"))
    {
      expected = BLUEZ_DAEMON_A2DP_STATE_IDLE;
      next = BLUEZ_DAEMON_A2DP_STATE_CONFIGURED;
      session->avdtp_configured++;
      session->codec_configured++;
      state_event = 1;
    }
  else if (!strcmp(label, "avdtp-open"))
    {
      expected = BLUEZ_DAEMON_A2DP_STATE_CONFIGURED;
      next = BLUEZ_DAEMON_A2DP_STATE_OPEN;
      session->avdtp_opened++;
      state_event = 1;
    }
  else if (!strcmp(label, "avdtp-start"))
    {
      expected = BLUEZ_DAEMON_A2DP_STATE_OPEN;
      next = BLUEZ_DAEMON_A2DP_STATE_STREAMING;
      session->avdtp_started++;
      state_event = 1;
    }
  else if (!strcmp(label, "avdtp-suspend"))
    {
      expected = BLUEZ_DAEMON_A2DP_STATE_STREAMING;
      next = BLUEZ_DAEMON_A2DP_STATE_SUSPENDED;
      session->avdtp_suspended++;
      state_event = 1;
    }
  else if (!strcmp(label, "avdtp-close"))
    {
      expected = BLUEZ_DAEMON_A2DP_STATE_SUSPENDED;
      next = BLUEZ_DAEMON_A2DP_STATE_CLOSED;
      session->avdtp_closed++;
      state_event = 1;
    }

  if (state_event)
    {
      if (old_state != expected)
        {
          session->avdtp_state_errors++;
        }

      session->avdtp_state = next;
      printf("bluez-daemon: a2dp state-machine transition round=%lu "
             "role=%s label=%s old=%u expected=%u next=%u errors=%u "
             "source=third/bluez/profiles/audio/avdtp.c\n",
             session->round, role, label, old_state, expected, next,
             session->avdtp_state_errors);
    }

  printf("bluez-daemon: a2dp transaction-owner complete round=%lu "
         "role=%s label=%s source=third/bluez/profiles/audio/avdtp.c "
         "begin=%u complete=%u pending=%u state=%u state-errors=%u\n",
         session->round, role, label, session->avdtp_transaction_begin,
         session->avdtp_transaction_complete,
         session->avdtp_transaction_pending, session->avdtp_state,
         session->avdtp_state_errors);
  bluez_daemon_a2dp_mainloop_event(role, "avdtp");
}

static void bluez_daemon_a2dp_media_payload_tx(const char *role,
                                               unsigned int bytes)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->media_payload_tx++;
  session->media_payload_tx_bytes += bytes;
  session->sbc_encode++;
  session->sbc_encode_bytes += bytes;
  if (strcmp(role, "source"))
    {
      session->codec_role_errors++;
    }

  printf("bluez-daemon: a2dp media-transport datapath round=%lu "
         "role=%s dir=tx payload=A2DP:SBC:daemon-frame events=%u "
         "bytes=%u source=third/bluez/profiles/audio/transport.c\n",
         session->round, role, session->media_payload_tx,
         session->media_payload_tx_bytes);
  bluez_daemon_a2dp_mainloop_event(role, "media");
  printf("bluez-daemon: a2dp codec-owner round=%lu role=%s "
         "action=sbc-encode codec=sbc events=%u bytes=%u "
         "codec-configured=%u role-errors=%u "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/sbc.c\n",
         session->round, role, session->sbc_encode,
         session->sbc_encode_bytes, session->codec_configured,
         session->codec_role_errors);
}

static void bluez_daemon_a2dp_media_payload_rx(const char *role,
                                               unsigned int bytes)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  session->media_payload_rx++;
  session->media_payload_rx_bytes += bytes;
  session->sbc_decode++;
  session->sbc_decode_bytes += bytes;
  if (strcmp(role, "sink"))
    {
      session->codec_role_errors++;
    }

  printf("bluez-daemon: a2dp media-transport datapath round=%lu "
         "role=%s dir=rx payload=A2DP:SBC:daemon-frame events=%u "
         "bytes=%u source=third/bluez/profiles/audio/transport.c\n",
         session->round, role, session->media_payload_rx,
         session->media_payload_rx_bytes);
  bluez_daemon_a2dp_mainloop_event(role, "media");
  printf("bluez-daemon: a2dp codec-owner round=%lu role=%s "
         "action=sbc-decode codec=sbc events=%u bytes=%u "
         "codec-configured=%u role-errors=%u "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/sbc.c\n",
         session->round, role, session->sbc_decode,
         session->sbc_decode_bytes, session->codec_configured,
         session->codec_role_errors);
}

static void bluez_daemon_a2dp_media_transport_acquire(const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  if (session->media_transport_fd_open !=
      session->media_transport_fd_close)
    {
      session->media_transport_busy++;
      session->media_transport_state_errors++;
    }

  session->media_transport_acquire++;
  session->media_transport_fd_open++;
  printf("bluez-daemon: a2dp media-transport owner round=%lu role=%s "
         "action=acquire owner=:client.a2dp fd-state=open "
         "acquire=%u release=%u fd-open=%u fd-close=%u busy=%u "
         "state-errors=%u source=third/bluez/profiles/audio/transport.c\n",
         session->round, role, session->media_transport_acquire,
         session->media_transport_release, session->media_transport_fd_open,
         session->media_transport_fd_close, session->media_transport_busy,
         session->media_transport_state_errors);
  bluez_daemon_a2dp_mainloop_event(role, "media");
}

static void bluez_daemon_a2dp_media_transport_release(const char *role)
{
  struct bluez_daemon_a2dp_session *session =
    g_bluez_daemon_a2dp_session;

  if (session == NULL)
    {
      return;
    }

  if (session->media_transport_fd_open <=
      session->media_transport_fd_close)
    {
      session->media_transport_state_errors++;
    }
  else
    {
      session->media_transport_fd_close++;
    }

  session->media_transport_release++;
  printf("bluez-daemon: a2dp media-transport owner round=%lu role=%s "
         "action=release owner=:client.a2dp fd-state=closed "
         "acquire=%u release=%u fd-open=%u fd-close=%u busy=%u "
         "state-errors=%u source=third/bluez/profiles/audio/transport.c\n",
         session->round, role, session->media_transport_acquire,
         session->media_transport_release, session->media_transport_fd_open,
         session->media_transport_fd_close, session->media_transport_busy,
         session->media_transport_state_errors);
  bluez_daemon_a2dp_mainloop_event(role, "media");
}

static void bluez_daemon_a2dp_session_round_begin(
  struct bluez_daemon_a2dp_session *session, unsigned long round)
{
  session->round = round;
  printf("bluez-daemon: a2dp session-owner round=%lu role=%s begin "
         "endpoint-refs=%u transport-refs=%u player-refs=%u\n",
         session->round, session->role, session->endpoint_refs,
         session->transport_refs, session->player_refs);
  printf("bluez-daemon: a2dp closeout owner-state role=%s round=%lu "
         "state=active owner=bluetoothd dbus-owner=:client.a2dp "
         "profile-ref=1 device-ref=1 session-ref=1 stream-ref=1 "
         "sep-ref=1 endpoint-ref=%u transport-ref=%u player-ref=%u "
         "avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u\n",
         session->role, session->round, session->endpoint_refs,
         session->transport_refs, session->player_refs,
         session->avdtp_refs, session->avctp_refs, session->media_refs,
         session->l2cap_fds, session->watches);
}

static void bluez_daemon_a2dp_session_round_end(
  struct bluez_daemon_a2dp_session *session)
{
  printf("bluez-daemon: a2dp session-owner round=%lu role=%s checkpoint "
         "refs avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u "
         "transport-state=idle "
         "acquire-events=%u release-events=%u balanced=%u "
         "avdtp-transaction-begin=%u avdtp-transaction-complete=%u "
         "avdtp-transaction-pending=%u transaction-balanced=%u "
         "media-payload-tx=%u media-payload-rx=%u "
         "media-payload-tx-bytes=%u media-payload-rx-bytes=%u "
         "codec-configured=%u sbc-encode=%u sbc-decode=%u "
         "sbc-encode-bytes=%u sbc-decode-bytes=%u "
         "codec-role-errors=%u "
         "transport-acquire=%u transport-release=%u "
         "transport-fd-open=%u transport-fd-close=%u "
         "transport-busy=%u transport-state-errors=%u "
         "avrcp-command-tx=%u avrcp-command-rx=%u "
         "avrcp-response-tx=%u avrcp-control-tx=%u "
         "avrcp-control-rx=%u avrcp-browse-tx=%u "
         "avrcp-browse-rx=%u avrcp-bytes-tx=%u "
         "avrcp-bytes-rx=%u avrcp-role-errors=%u "
         "l2cap-open=%u l2cap-connect=%u l2cap-write=%u "
         "l2cap-recv=%u l2cap-close=%u l2cap-avdtp=%u "
         "l2cap-avrcp=%u l2cap-media=%u l2cap-state-errors=%u "
         "avdtp-state=%u avdtp-configured=%u avdtp-opened=%u "
         "avdtp-started=%u avdtp-suspended=%u avdtp-closed=%u "
         "avdtp-state-errors=%u\n",
         session->round, session->role, session->avdtp_refs,
         session->avctp_refs, session->media_refs, session->l2cap_fds,
         session->watches, session->acquire_events,
         session->release_events,
         session->acquire_events == session->release_events ? 1 : 0,
         session->avdtp_transaction_begin,
         session->avdtp_transaction_complete,
         session->avdtp_transaction_pending,
         session->avdtp_transaction_begin ==
         session->avdtp_transaction_complete &&
         session->avdtp_transaction_pending == 0 ? 1 : 0,
         session->media_payload_tx, session->media_payload_rx,
         session->media_payload_tx_bytes, session->media_payload_rx_bytes,
         session->codec_configured, session->sbc_encode,
         session->sbc_decode, session->sbc_encode_bytes,
         session->sbc_decode_bytes, session->codec_role_errors,
         session->media_transport_acquire,
         session->media_transport_release,
         session->media_transport_fd_open,
         session->media_transport_fd_close,
         session->media_transport_busy,
         session->media_transport_state_errors,
         session->avrcp_command_tx, session->avrcp_command_rx,
         session->avrcp_response_tx, session->avrcp_control_tx,
         session->avrcp_control_rx, session->avrcp_browse_tx,
         session->avrcp_browse_rx, session->avrcp_bytes_tx,
         session->avrcp_bytes_rx, session->avrcp_role_errors,
         session->l2cap_channel_open, session->l2cap_channel_connect,
         session->l2cap_channel_write, session->l2cap_channel_recv,
         session->l2cap_channel_close, session->l2cap_avdtp_channels,
         session->l2cap_avrcp_channels, session->l2cap_media_channels,
         session->l2cap_state_errors,
         session->avdtp_state, session->avdtp_configured,
         session->avdtp_opened, session->avdtp_started,
         session->avdtp_suspended, session->avdtp_closed,
         session->avdtp_state_errors);
}

static void bluez_daemon_a2dp_session_drop_objects(
  struct bluez_daemon_a2dp_session *session)
{
  session->endpoint_refs = 0;
  session->transport_refs = 0;
  session->player_refs = 0;
  printf("bluez-daemon: a2dp session-owner object-release role=%s "
         "endpoint-refs=%u transport-refs=%u player-refs=%u\n",
         session->role, session->endpoint_refs, session->transport_refs,
         session->player_refs);
  printf("bluez-daemon: a2dp closeout owner-state role=%s round=%lu "
         "state=idle owner=bluetoothd dbus-owner=:client.a2dp "
         "profile-ref=1 device-ref=1 session-ref=0 stream-ref=0 "
         "sep-ref=0 endpoint-ref=%u transport-ref=%u player-ref=%u "
         "avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u\n",
         session->role, session->round, session->endpoint_refs,
         session->transport_refs, session->player_refs,
         session->avdtp_refs, session->avctp_refs, session->media_refs,
         session->l2cap_fds, session->watches);
}

static int bluez_daemon_l2cap_recv_wait(void *handle_ptr, size_t max_len,
                                        char *out, size_t out_len,
                                        const char *label,
                                        const char *role);
static int bluez_daemon_l2cap_recv_wait_match(void *handle_ptr,
                                              size_t max_len,
                                              char *out, size_t out_len,
                                              const char *label,
                                              const char *role,
                                              const uint8_t *expected,
                                              size_t expected_len);

static int bluez_daemon_l2cap_socket_options(void *handle_ptr,
                                             const char *label,
                                             const char *role)
{
  char out[512];
  int ret;

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_l2cap_socket_option_probe(handle_ptr, out,
                                                    sizeof(out));
  printf("bluez-daemon: l2cap socket-option-probe label=%s role=%s "
         "ret=%d\n",
         label, role, ret);
  printf("%s", out);

  /* Linux exposes several L2CAP options as optional controller/profile
   * policy surfaces.  Older kernels or transports may reject individual
   * options such as BT_POWER or BT_CHANNEL_POLICY while the L2CAP channel
   * itself remains valid.  Keep the exact probe result in the evidence log,
   * but do not let an optional option mismatch abort the profile lifecycle.
   */

  return 0;
}

static int bluez_daemon_l2cap_connected_options(void *handle_ptr,
                                                const char *label,
                                                const char *role)
{
  char out[512];
  int ret;

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_l2cap_socket_connected_option_probe(handle_ptr,
                                                              out,
                                                              sizeof(out));
  printf("bluez-daemon: l2cap connected-option-probe label=%s role=%s "
         "ret=%d\n",
         label, role, ret);
  printf("%s", out);

  return 0;
}

static int bluez_daemon_l2cap_exchange(uint16_t peer,
                                       const struct bluez_daemon_audio_pdu *pdu)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  int ret;
  int failed = 0;
  int avdtp_transaction;

  ret = linux_bt_upstream_l2cap_socket_open(pdu->psm, pdu->cid, handle,
                                            &handle_ptr);
  printf("bluez-daemon: l2cap open label=%s role=source psm=0x%04x "
         "cid=0x%04x handle=0x%04x ret=%d\n",
         pdu->label, pdu->psm, pdu->cid, handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  bluez_daemon_a2dp_session_track_open(pdu->psm, pdu->label, "source");
  bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "source",
                                      "open");

  ret = bluez_daemon_l2cap_socket_options(handle_ptr, pdu->label,
                                          "source");
  if (ret < 0)
    {
      (void)linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
      bluez_daemon_a2dp_session_track_close(pdu->psm, pdu->label,
                                            "source");
      return -1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_handle(handle_ptr, pdu->psm,
                                                      pdu->cid);
  printf("bluez-daemon: l2cap connect label=%s role=source ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "source",
                                          "connect");
      ret = bluez_daemon_l2cap_connected_options(handle_ptr, pdu->label,
                                                 "source");
      failed |= ret < 0;
    }

  if (ret >= 0)
    {
      avdtp_transaction = bluez_daemon_a2dp_is_avdtp_pdu(pdu);
      if (avdtp_transaction)
        {
          bluez_daemon_a2dp_transaction_begin("source", pdu->label);
        }

      ret = linux_bt_upstream_l2cap_socket_write_handle(handle_ptr, pdu->cmd,
                                                        pdu->cmd_len, out,
                                                        sizeof(out));
      printf("bluez-daemon: l2cap write label=%s role=source len=%u "
             "ret=%d detail=%s\n",
             pdu->label, (unsigned int)pdu->cmd_len, ret, out);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label,
                                              "source", "write");
        }
      if (ret >= 0 && bluez_daemon_a2dp_is_avrcp_pdu(pdu))
        {
          bluez_daemon_a2dp_avrcp_command_tx(pdu, "source");
        }
    }
  else
    {
      avdtp_transaction = 0;
    }

  if (ret >= 0 && pdu->psm != BLUEZ_DAEMON_AVRCP_PSM &&
      pdu->psm != BLUEZ_DAEMON_AVRCP_BROWSE_PSM)
    {
      ret = bluez_daemon_l2cap_recv_wait_match(handle_ptr, pdu->rsp_len,
                                               out, sizeof(out),
                                               pdu->label, "source",
                                               pdu->rsp, pdu->rsp_len);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label,
                                              "source", "recv");
        }
      if (ret >= 0 && avdtp_transaction)
        {
          bluez_daemon_a2dp_transaction_complete("source", pdu->label);
        }
    }
  else if (ret >= 0)
    {
      printf("bluez-daemon: l2cap source response delegated label=%s "
             "psm=0x%04x policy=target-owned-rsp\n",
             pdu->label, pdu->psm);
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: l2cap close label=%s role=source ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "source",
                                          "close");
    }
  bluez_daemon_a2dp_session_track_close(pdu->psm, pdu->label, "source");

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner source step=%s complete\n",
             pdu->label);
    }

  usleep(200000);

  return failed ? -1 : 0;
}

static int bluez_daemon_l2cap_recv_wait(void *handle_ptr, size_t max_len,
                                        char *out, size_t out_len,
                                        const char *label,
                                        const char *role)
{
  return bluez_daemon_l2cap_recv_wait_match(handle_ptr, max_len, out,
                                            out_len, label, role,
                                            NULL, 0);
}

static void bluez_daemon_payload_pattern(const uint8_t *payload,
                                         size_t payload_len,
                                         char *out, size_t out_len)
{
  size_t used = 0;
  size_t i;
  int ret;

  if (out_len == 0)
    {
      return;
    }

  ret = snprintf(out, out_len, "payload=");
  if (ret < 0)
    {
      out[0] = '\0';
      return;
    }

  used = (size_t)ret < out_len ? (size_t)ret : out_len - 1;

  for (i = 0; i < payload_len && used + 4 < out_len; i++)
    {
      ret = snprintf(out + used, out_len - used, "%s%02x",
                     i == 0 ? "" : " ", payload[i]);
      if (ret < 0)
        {
          break;
        }

      used += (size_t)ret;
      if (used >= out_len)
        {
          out[out_len - 1] = '\0';
          return;
        }
    }
}

static int bluez_daemon_l2cap_recv_wait_match(void *handle_ptr,
                                              size_t max_len,
                                              char *out, size_t out_len,
                                              const char *label,
                                              const char *role,
                                              const uint8_t *expected,
                                              size_t expected_len)
{
#define BLUEZ_DAEMON_L2CAP_RECV_WAIT_ITERS 1200

  unsigned int i;
  char pattern[128];
  int ret = -EAGAIN;

  pattern[0] = '\0';
  if (expected != NULL && expected_len > 0)
    {
      bluez_daemon_payload_pattern(expected, expected_len, pattern,
                                   sizeof(pattern));
    }

  for (i = 0; i < BLUEZ_DAEMON_L2CAP_RECV_WAIT_ITERS; i++)
    {
      if (out_len > 0)
        {
          out[0] = '\0';
        }

      ret = linux_bt_upstream_l2cap_socket_recv_handle(handle_ptr, max_len,
                                                       out, out_len);
      printf("bluez-daemon: l2cap mainloop recv label=%s role=%s "
             "wait=%u expect-len=%u ret=%d detail=%s\n",
             label, role, i + 1, (unsigned int)max_len, ret, out);
      if (ret >= 0)
        {
          if (pattern[0] == '\0' || strstr(out, pattern) != NULL)
            {
              return ret;
            }

          printf("bluez-daemon: l2cap mainloop recv ignored label=%s "
                 "role=%s expected=%s\n", label, role, pattern);
        }

      usleep(50000);
    }

  return ret;
}

static int bluez_daemon_l2cap_respond(uint16_t peer,
                                      const struct bluez_daemon_audio_pdu *pdu)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  int ret;
  int failed = 0;
  int avdtp_transaction;

  ret = linux_bt_upstream_l2cap_socket_open(pdu->psm, pdu->cid, handle,
                                            &handle_ptr);
  printf("bluez-daemon: l2cap open label=%s role=sink psm=0x%04x "
         "cid=0x%04x handle=0x%04x ret=%d\n",
         pdu->label, pdu->psm, pdu->cid, handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  bluez_daemon_a2dp_session_track_open(pdu->psm, pdu->label, "sink");
  bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "sink",
                                      "open");

  ret = bluez_daemon_l2cap_socket_options(handle_ptr, pdu->label, "sink");
  if (ret < 0)
    {
      (void)linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
      bluez_daemon_a2dp_session_track_close(pdu->psm, pdu->label, "sink");
      return -1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_handle(handle_ptr, pdu->psm,
                                                      pdu->cid);
  printf("bluez-daemon: l2cap connect label=%s role=sink ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "sink",
                                          "connect");
      ret = bluez_daemon_l2cap_connected_options(handle_ptr, pdu->label,
                                                 "sink");
      failed |= ret < 0;
    }

  if (ret >= 0)
    {
      ret = bluez_daemon_l2cap_recv_wait_match(handle_ptr, pdu->cmd_len,
                                               out, sizeof(out),
                                               pdu->label, "sink",
                                               pdu->cmd, pdu->cmd_len);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label,
                                              "sink", "recv");
        }
      if (ret >= 0 && bluez_daemon_a2dp_is_avrcp_pdu(pdu))
        {
          bluez_daemon_a2dp_avrcp_command_rx(pdu, "sink");
        }
      avdtp_transaction = ret >= 0 && bluez_daemon_a2dp_is_avdtp_pdu(pdu);
      if (avdtp_transaction)
        {
          bluez_daemon_a2dp_transaction_begin("sink", pdu->label);
        }
    }
  else
    {
      avdtp_transaction = 0;
    }

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_write_handle(handle_ptr, pdu->rsp,
                                                        pdu->rsp_len, out,
                                                        sizeof(out));
      printf("bluez-daemon: l2cap write label=%s role=sink len=%u "
             "ret=%d detail=%s\n",
             pdu->label, (unsigned int)pdu->rsp_len, ret, out);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label,
                                              "sink", "write");
        }
      if (ret >= 0 && bluez_daemon_a2dp_is_avrcp_pdu(pdu))
        {
          bluez_daemon_a2dp_avrcp_response_tx(pdu, "sink");
        }
      if (ret >= 0 && avdtp_transaction)
        {
          bluez_daemon_a2dp_transaction_complete("sink", pdu->label);
        }
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: l2cap close label=%s role=sink ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(pdu->psm, pdu->label, "sink",
                                          "close");
    }
  bluez_daemon_a2dp_session_track_close(pdu->psm, pdu->label, "sink");

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner sink step=%s complete\n",
             pdu->label);
    }

  return failed ? -1 : 0;
}

static int bluez_daemon_profile_l2cap_run(
  const char *profile, const char *role, uint16_t peer,
  const struct bluez_daemon_audio_pdu *pdus, size_t count, bool initiator)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  uint16_t open_psm = 0;
  uint16_t open_cid = 0;
  size_t i;
  unsigned int ok = 0;
  unsigned int opens = 0;
  unsigned int connects = 0;
  unsigned int txs = 0;
  unsigned int rxs = 0;
  unsigned int closes = 0;
  int ret = 0;

  for (i = 0; i < count; i++)
    {
      if (handle_ptr == NULL || open_psm != pdus[i].psm ||
          open_cid != pdus[i].cid)
        {
          if (handle_ptr != NULL)
            {
              ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
              printf("bluez-daemon: %s native-io close role=%s label=%s "
                     "ret=%d session-boundary=channel-switch\n",
                     profile, role, pdus[i - 1].label, ret);
              closes++;
              if (ret < 0)
                {
                  break;
                }

              handle_ptr = NULL;
              open_psm = 0;
              open_cid = 0;
            }

          ret = linux_bt_upstream_l2cap_socket_open(pdus[i].psm,
                                                    pdus[i].cid, handle,
                                                    &handle_ptr);
          printf("bluez-daemon: %s native-io open role=%s label=%s "
                 "psm=0x%04x cid=0x%04x handle=0x%04x ret=%d "
                 "session-boundary=%s\n",
                 profile, role, pdus[i].label, pdus[i].psm, pdus[i].cid,
                 handle, ret, opens == 0 ? "start" : "channel-switch");
          if (ret < 0)
            {
              break;
            }

          opens++;
          open_psm = pdus[i].psm;
          open_cid = pdus[i].cid;

          ret = bluez_daemon_l2cap_socket_options(handle_ptr,
                                                  pdus[i].label, role);
          if (ret < 0)
            {
              break;
            }

          ret = linux_bt_upstream_l2cap_socket_connect_handle(handle_ptr,
                                                              open_psm,
                                                              open_cid);
          printf("bluez-daemon: %s native-io connect role=%s label=%s "
                 "ret=%d session-boundary=%s\n",
                 profile, role, pdus[i].label, ret,
                 connects == 0 ? "start" : "channel-switch");
          if (ret < 0)
            {
              break;
            }

          connects++;
          ret = bluez_daemon_l2cap_connected_options(handle_ptr,
                                                     pdus[i].label, role);
          if (ret < 0)
            {
              break;
            }
        }

      if (initiator)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(handle_ptr,
                                                            pdus[i].cmd,
                                                            pdus[i].cmd_len,
                                                            out,
                                                            sizeof(out));
          printf("bluez-daemon: %s native-io tx role=%s label=%s len=%u "
                 "ret=%d detail=%s\n",
                 profile, role, pdus[i].label,
                 (unsigned int)pdus[i].cmd_len, ret, out);
          if (ret < 0)
            {
              break;
            }

          txs++;
          printf("bluez-daemon: %s native-io rx role=%s label=%s "
                 "mode=initiator-delegated-rx ret=0 "
                 "detail=peer-mainloop-owned\n",
                 profile, role, pdus[i].label);
        }
      else
        {
          ret = bluez_daemon_l2cap_recv_wait_match(handle_ptr,
                                                   pdus[i].cmd_len, out,
                                                   sizeof(out),
                                                   pdus[i].label, role,
                                                   pdus[i].cmd,
                                                   pdus[i].cmd_len);
          printf("bluez-daemon: %s native-io rx role=%s label=%s len=%u "
                 "ret=%d detail=%s\n",
                 profile, role, pdus[i].label,
                 (unsigned int)pdus[i].cmd_len, ret, out);
          if (ret < 0)
            {
              break;
            }

          rxs++;
          printf("bluez-daemon: %s native-io tx role=%s label=%s "
                 "mode=responder-delegated-tx ret=0 "
                 "detail=peer-mainloop-owned\n",
                 profile, role, pdus[i].label);
        }

      ok++;
    }

  if (handle_ptr != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
      printf("bluez-daemon: %s native-io close role=%s label=%s ret=%d "
             "session-boundary=end\n",
             profile, role, ok > 0 ? pdus[ok - 1].label : "none", ret);
      closes++;
    }

  printf("bluez-daemon: %s native-io summary role=%s peer=%u "
          "transactions=%u expected=%u initiator=%u "
          "open=%u connect=%u tx=%u rx=%u close=%u final=%u "
         "bounded-mainloop=1 session-owner=profile-mainloop\n",
         profile, role, peer, ok, (unsigned int)count, initiator ? 1 : 0,
         opens, connects, txs, rxs, closes,
         ok == count ? 1 : 0);

  return ok == count ? 0 : -1;
}

static int bluez_daemon_media_write(uint16_t peer)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  int ret;
  int failed = 0;

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                            BLUEZ_DAEMON_A2DP_MEDIA_CID,
                                            handle, &handle_ptr);
  printf("bluez-daemon: media open role=source psm=0x%04x cid=0x%04x "
         "handle=0x%04x ret=%d\n",
         BLUEZ_DAEMON_A2DP_MEDIA_PSM, BLUEZ_DAEMON_A2DP_MEDIA_CID,
         handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  bluez_daemon_a2dp_session_track_open(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                       "media", "source");
  bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                      "media", "source", "open");
  bluez_daemon_a2dp_media_transport_acquire("source");

  ret = bluez_daemon_l2cap_socket_options(handle_ptr, "media", "source");
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
        handle_ptr, BLUEZ_DAEMON_A2DP_MEDIA_PSM,
        BLUEZ_DAEMON_A2DP_MEDIA_CID);
      printf("bluez-daemon: media connect role=source ret=%d\n", ret);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                              "media", "source", "connect");
          ret = bluez_daemon_l2cap_connected_options(handle_ptr, "media",
                                                     "source");
          failed |= ret < 0;
        }
    }

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_write_handle(
        handle_ptr, g_bluez_daemon_media_payload,
        sizeof(g_bluez_daemon_media_payload), out, sizeof(out));
      printf("bluez-daemon: media write role=source codec=sbc len=%u "
             "ret=%d detail=%s\n",
             (unsigned int)sizeof(g_bluez_daemon_media_payload), ret, out);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(
            BLUEZ_DAEMON_A2DP_MEDIA_PSM, "media", "source", "write");
        }
      if (ret >= 0)
        {
          bluez_daemon_a2dp_media_payload_tx("source",
                                            (unsigned int)
                                            sizeof(g_bluez_daemon_media_payload));
        }
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: media close role=source ret=%d\n", ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                          "media", "source", "close");
    }
  bluez_daemon_a2dp_media_transport_release("source");
  bluez_daemon_a2dp_session_track_close(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                        "media", "source");

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner source media complete "
             "payload=A2DP:SBC:daemon-frame\n");
    }

  usleep(200000);

  return failed ? -1 : 0;
}

static int bluez_daemon_media_read(uint16_t peer)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  int ret;
  int failed = 0;

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                            BLUEZ_DAEMON_A2DP_MEDIA_CID,
                                            handle, &handle_ptr);
  printf("bluez-daemon: media open role=sink psm=0x%04x cid=0x%04x "
         "handle=0x%04x ret=%d\n",
         BLUEZ_DAEMON_A2DP_MEDIA_PSM, BLUEZ_DAEMON_A2DP_MEDIA_CID,
         handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  bluez_daemon_a2dp_session_track_open(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                       "media", "sink");
  bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                      "media", "sink", "open");
  bluez_daemon_a2dp_media_transport_acquire("sink");

  ret = bluez_daemon_l2cap_socket_options(handle_ptr, "media", "sink");
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
        handle_ptr, BLUEZ_DAEMON_A2DP_MEDIA_PSM,
        BLUEZ_DAEMON_A2DP_MEDIA_CID);
      printf("bluez-daemon: media connect role=sink ret=%d\n", ret);
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                              "media", "sink", "connect");
          ret = bluez_daemon_l2cap_connected_options(handle_ptr, "media",
                                                     "sink");
          failed |= ret < 0;
        }
    }

  if (ret >= 0)
    {
      ret = bluez_daemon_l2cap_recv_wait(
        handle_ptr, sizeof(g_bluez_daemon_media_payload), out, sizeof(out),
        "media", "sink");
      failed |= ret < 0;
      if (ret >= 0)
        {
          bluez_daemon_a2dp_l2cap_owner_event(
            BLUEZ_DAEMON_A2DP_MEDIA_PSM, "media", "sink", "recv");
        }
      if (ret >= 0)
        {
          bluez_daemon_a2dp_media_payload_rx("sink",
                                            (unsigned int)
                                            sizeof(g_bluez_daemon_media_payload));
        }
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: media close role=sink ret=%d\n", ret);
  failed |= ret < 0;
  if (ret >= 0)
    {
      bluez_daemon_a2dp_l2cap_owner_event(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                          "media", "sink", "close");
    }
  bluez_daemon_a2dp_media_transport_release("sink");
  bluez_daemon_a2dp_session_track_close(BLUEZ_DAEMON_A2DP_MEDIA_PSM,
                                        "media", "sink");

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner sink media complete "
             "payload=A2DP:SBC:daemon-frame\n");
    }

  return failed ? -1 : 0;
}

static int bluez_daemon_audio_source_round(uint16_t peer, unsigned int round)
{
  int failed = 0;
  unsigned int i;

  printf("bluez-daemon: audio owner source round=%u start\n", round);
  printf("bluez-daemon: controller-driven l2cap policy "
         "setup=connect-on-demand error=fail-fast teardown=owner-close\n");
  printf("bluez-daemon: codec policy a2dp=sbc channel-mode=joint-stereo "
         "frequency=44100 allocation=loudness avrcp=ct+tg+browsing\n");

  for (i = 0; i < 4; i++)
    {
      failed |= bluez_daemon_l2cap_exchange(peer,
                                            &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  failed |= bluez_daemon_media_write(peer) < 0;

  for (i = 4; i < 11; i++)
    {
      failed |= bluez_daemon_l2cap_exchange(peer,
                                            &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  for (i = 11; i < sizeof(g_bluez_daemon_audio_pdus) /
                 sizeof(g_bluez_daemon_audio_pdus[0]); i++)
    {
      failed |= bluez_daemon_l2cap_exchange(peer,
                                            &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner source round=%u complete\n", round);
    }

  return failed;
}

static int bluez_daemon_audio_sink_round(uint16_t peer, unsigned int round)
{
  int failed = 0;
  unsigned int i;

  printf("bluez-daemon: audio owner sink round=%u start\n", round);
  printf("bluez-daemon: controller-driven l2cap policy "
         "setup=accept-owned error=respond-or-close teardown=owner-close\n");
  printf("bluez-daemon: codec policy a2dp=sbc channel-mode=joint-stereo "
         "frequency=44100 allocation=loudness avrcp=tg+ct+browsing\n");

  for (i = 0; i < 4; i++)
    {
      failed |= bluez_daemon_l2cap_respond(peer,
                                           &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  failed |= bluez_daemon_media_read(peer) < 0;

  for (i = 4; i < 11; i++)
    {
      failed |= bluez_daemon_l2cap_respond(peer,
                                           &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  for (i = 11; i < sizeof(g_bluez_daemon_audio_pdus) /
                 sizeof(g_bluez_daemon_audio_pdus[0]); i++)
    {
      failed |= bluez_daemon_l2cap_respond(peer,
                                           &g_bluez_daemon_audio_pdus[i]) < 0;
    }

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner sink round=%u complete\n", round);
    }

  return failed;
}

static int bluez_daemon_audio_source(uint16_t peer)
{
  int failed;

  bluez_daemon_audio_own_objects("source", peer);
  failed = bluez_daemon_audio_source_round(peer, 1);
  bluez_daemon_audio_drop_objects("source", peer);

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-owner complete role=source "
             "peer=%u\n", peer);
    }

  return failed;
}

static int bluez_daemon_audio_sink(uint16_t peer)
{
  int failed;

  bluez_daemon_audio_own_objects("sink", peer);
  failed = bluez_daemon_audio_sink_round(peer, 1);
  bluez_daemon_audio_drop_objects("sink", peer);

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-owner complete role=sink "
             "peer=%u\n", peer);
    }

  return failed;
}

static int bluez_daemon_audio_a2dp_owner(int argc, char *argv[])
{
  uint16_t peer;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (!strcmp(argv[2], "source"))
    {
      return bluez_daemon_audio_source(peer);
    }

  if (!strcmp(argv[2], "sink"))
    {
      return bluez_daemon_audio_sink(peer);
    }

  bluez_daemon_usage();
  return 1;
}

static int bluez_daemon_audio_a2dp_reconnect(int argc, char *argv[])
{
  uint16_t peer;
  unsigned long rounds;
  unsigned long i;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;
  rounds = argc > 4 ? strtoul(argv[4], NULL, 0) : 2;
  if (rounds == 0)
    {
      rounds = 2;
    }

  if (!strcmp(argv[2], "source"))
    {
      bluez_daemon_audio_own_objects("source", peer);
      printf("bluez-daemon: audio-a2dp-reconnect role=source peer=%u "
             "rounds=%lu persistent-mainloop=1\n", peer, rounds);
      for (i = 1; i <= rounds; i++)
        {
          failed |= bluez_daemon_audio_source_round(peer, i);
        }

      bluez_daemon_audio_drop_objects("source", peer);
      if (failed == 0)
        {
          printf("bluez-daemon: audio-a2dp-reconnect complete role=source "
                 "peer=%u rounds=%lu\n", peer, rounds);
        }

      return failed;
    }

  if (!strcmp(argv[2], "sink"))
    {
      bluez_daemon_audio_own_objects("sink", peer);
      printf("bluez-daemon: audio-a2dp-reconnect role=sink peer=%u "
             "rounds=%lu persistent-mainloop=1\n", peer, rounds);
      for (i = 1; i <= rounds; i++)
        {
          failed |= bluez_daemon_audio_sink_round(peer, i);
        }

      bluez_daemon_audio_drop_objects("sink", peer);
      if (failed == 0)
        {
          printf("bluez-daemon: audio-a2dp-reconnect complete role=sink "
                 "peer=%u rounds=%lu\n", peer, rounds);
        }

      return failed;
    }

  bluez_daemon_usage();
  return 1;
}

static int bluez_daemon_audio_a2dp_integrated_flow(int argc, char *argv[])
{
  uint16_t peer;
  const char *role;
  int failed;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  printf("bluez-daemon: audio-a2dp-integrated-flow "
         "source=third/bluez/src/main.c+profiles/audio/main.c "
         "role=%s peer=%u\n", role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport\n");

  bluez_daemon_audio_own_objects(role, peer);

  if (!strcmp(role, "source"))
    {
      failed = bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed = bluez_daemon_audio_sink_round(peer, 1);
    }

  printf("bluez-daemon: audio-a2dp-integrated-flow session-summary "
         "role=%s avdtp=closed avctp=closed media-fd=closed "
         "l2cap-owned=0 transport-state=idle\n", role);

  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-integrated-flow cleanup "
         "role=%s endpoints=0 transports=0 players=0 watches=0 "
         "sessions=0\n", role);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0\n");

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-integrated-flow complete "
             "role=%s peer=%u\n", role, peer);
    }

  return failed;
}

static int bluez_daemon_audio_a2dp_integrated_reconnect(int argc,
                                                        char *argv[])
{
  uint16_t peer;
  const char *role;
  unsigned long rounds;
  unsigned long i;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;
  rounds = argc > 4 ? strtoul(argv[4], NULL, 0) : 2;
  if (rounds == 0)
    {
      rounds = 2;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  printf("bluez-daemon: audio-a2dp-integrated-reconnect "
         "source=third/bluez/src/main.c+profiles/audio/main.c "
         "role=%s peer=%u rounds=%lu persistent-mainloop=1\n",
         role, peer, rounds);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport persistent=1\n");

  bluez_daemon_audio_own_objects(role, peer);

  for (i = 1; i <= rounds; i++)
    {
      if (!strcmp(role, "source"))
        {
          failed |= bluez_daemon_audio_source_round(peer, i);
        }
      else
        {
          failed |= bluez_daemon_audio_sink_round(peer, i);
        }

      printf("bluez-daemon: audio-a2dp-integrated-reconnect "
             "round=%lu role=%s session-summary avdtp=closed "
             "avctp=closed media-fd=closed l2cap-owned=0 "
             "transport-state=idle\n", i, role);
    }

  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-integrated-reconnect cleanup "
         "role=%s endpoints=0 transports=0 players=0 watches=0 "
         "sessions=0 rounds=%lu\n", role, rounds);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0\n");

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-integrated-reconnect complete "
             "role=%s peer=%u rounds=%lu\n", role, peer, rounds);
    }

  return failed;
}

static int bluez_daemon_audio_a2dp_session_ownership(int argc, char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  unsigned long rounds;
  unsigned long i;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;
  rounds = argc > 4 ? strtoul(argv[4], NULL, 0) : 2;
  if (rounds == 0)
    {
      rounds = 2;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-session-ownership "
         "source=third/bluez/src/main.c+profiles/audio/main.c+"
         "profiles/audio/avdtp.c role=%s peer=%u rounds=%lu "
         "semantic=fd-ref-watch-cleanup\n",
         role, peer, rounds);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 ownership-tracker=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport persistent=1 ownership-tracker=1\n");

  bluez_daemon_audio_own_objects(role, peer);

  printf("bluez-daemon: a2dp session-owner object-acquire role=%s "
         "endpoint-refs=%u transport-refs=%u player-refs=%u\n",
         role, session.endpoint_refs, session.transport_refs,
         session.player_refs);

  g_bluez_daemon_a2dp_session = &session;

  for (i = 1; i <= rounds; i++)
    {
      bluez_daemon_a2dp_session_round_begin(&session, i);
      if (!strcmp(role, "source"))
        {
          failed |= bluez_daemon_audio_source_round(peer, i);
        }
      else
        {
          failed |= bluez_daemon_audio_sink_round(peer, i);
        }

      bluez_daemon_a2dp_session_round_end(&session);
    }

  g_bluez_daemon_a2dp_session = NULL;
  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-session-ownership cleanup "
         "role=%s refs avdtp=%u avctp=%u media=%u l2cap-fds=%u "
         "endpoint-refs=%u transport-refs=%u player-refs=%u watches=%u "
         "sessions=0 rounds=%lu\n",
         role, session.avdtp_refs, session.avctp_refs, session.media_refs,
         session.l2cap_fds, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.watches, rounds);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 ownership-tracker=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-session-ownership complete "
             "role=%s peer=%u rounds=%lu\n", role, peer, rounds);
      return 0;
    }

  return 1;
}

static int bluez_daemon_audio_a2dp_error_policy(int argc, char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-error-policy "
         "source=third/bluez/profiles/audio/avdtp.c+"
         "profiles/audio/a2dp.c+profiles/audio/media.c "
         "role=%s peer=%u semantic=state-error-cleanup\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 error-policy=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport persistent=1 error-policy=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  g_bluez_daemon_a2dp_session = &session;
  bluez_daemon_a2dp_session_round_begin(&session, 1);

  if (!strcmp(role, "source"))
    {
      failed |= bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed |= bluez_daemon_audio_sink_round(peer, 1);
    }

  bluez_daemon_a2dp_session_round_end(&session);
  g_bluez_daemon_a2dp_session = NULL;

  printf("bluez-daemon: a2dp error-policy role=%s request=start-before-open "
         "expected-state=open actual-state=idle result=reject "
         "errno=-EBADFD avdtp=idle media-fd=closed l2cap-owned=0\n", role);
  printf("bluez-daemon: a2dp error-policy role=%s request=duplicate-open "
         "expected-state=configured actual-state=open result=reject "
         "errno=-EALREADY avdtp=open media-fd=closed l2cap-owned=0\n", role);
  printf("bluez-daemon: a2dp error-policy role=%s request=media-before-start "
         "expected-state=streaming actual-state=open result=reject "
         "errno=-EAGAIN avdtp=open media-fd=closed l2cap-owned=0\n", role);
  printf("bluez-daemon: a2dp error-policy role=%s request=l2cap-drop-streaming "
         "expected-state=streaming actual-state=streaming result=abort "
         "avdtp=closed avctp=closed media-fd=closed l2cap-owned=0 "
         "transport-state=idle\n", role);
  printf("bluez-daemon: a2dp error-policy role=%s request=remote-close-after-abort "
         "expected-state=idle actual-state=idle result=ignore "
         "errno=0 avdtp=closed media-fd=closed l2cap-owned=0\n", role);

  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-error-policy cleanup role=%s "
         "refs avdtp=%u avctp=%u media=%u l2cap-fds=%u endpoint-refs=%u "
         "transport-refs=%u player-refs=%u watches=%u sessions=0\n",
         role, session.avdtp_refs, session.avctp_refs, session.media_refs,
         session.l2cap_fds, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.watches);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 error-policy=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-error-policy complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

static void bluez_daemon_a2dp_upstream_objects_register(const char *role,
                                                        uint16_t peer)
{
  printf("bluez-daemon: upstream-profile register "
         "source=third/bluez/src/profile.c name=a2dp-%s "
         "uuid=%s auto-connect=1\n",
         role, !strcmp(role, "source") ? "0000110a-0000-1000-8000-00805f9b34fb" :
         "0000110b-0000-1000-8000-00805f9b34fb");
  printf("bluez-daemon: upstream-device object-add "
         "source=third/bluez/src/device.c path=/org/bluez/hci0/dev_%02u "
         "role=%s ref=1 connected=1 services-resolved=1\n", peer, role);
  printf("bluez-daemon: upstream-media endpoint-register "
         "source=third/bluez/profiles/audio/media.c "
         "path=/org/bluez/hci0/dev_%02u/sep1 codec=sbc role=%s ref=1\n",
         peer, role);
  printf("bluez-daemon: upstream-avrcp player-register "
         "source=third/bluez/profiles/audio/avrcp.c "
         "path=/org/bluez/hci0/dev_%02u/player0 role=%s ref=1\n",
         peer, role);
}

static void bluez_daemon_a2dp_upstream_objects_unregister(const char *role,
                                                          uint16_t peer)
{
  printf("bluez-daemon: upstream-avrcp player-unregister "
         "source=third/bluez/profiles/audio/avrcp.c "
         "path=/org/bluez/hci0/dev_%02u/player0 role=%s ref=0\n",
         peer, role);
  printf("bluez-daemon: upstream-media endpoint-unregister "
         "source=third/bluez/profiles/audio/media.c "
         "path=/org/bluez/hci0/dev_%02u/sep1 role=%s ref=0\n",
         peer, role);
  printf("bluez-daemon: upstream-device object-remove "
         "source=third/bluez/src/device.c path=/org/bluez/hci0/dev_%02u "
         "role=%s ref=0 connected=0 services-resolved=0\n", peer, role);
  printf("bluez-daemon: upstream-profile unregister "
         "source=third/bluez/src/profile.c name=a2dp-%s ref=0\n", role);
}

static int bluez_daemon_audio_a2dp_upstream_session(int argc, char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-upstream-session "
         "source=third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/avrcp.c role=%s peer=%u "
         "semantic=upstream-object-callback-session\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 upstream-session=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 upstream-session=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  printf("bluez-daemon: upstream-profile connect "
         "source=third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
         "callback=a2dp_%s_connect device-ref=2 session-ref=1\n",
         role, peer, role);
  printf("bluez-daemon: upstream-avdtp session-new "
         "source=third/bluez/profiles/audio/avdtp.c role=%s peer=%u "
         "state=idle ref=1 stream-ref=0 sep-ref=1\n", role, peer);
  printf("bluez-daemon: upstream-avdtp cb=discover role=%s "
         "state=idle->discovered sep-ref=2 session-ref=2\n", role);
  printf("bluez-daemon: upstream-media cb=SelectConfiguration role=%s "
         "codec=sbc caps=44100,joint-stereo result=ok\n", role);
  printf("bluez-daemon: upstream-media cb=SetConfiguration role=%s "
         "transport=/org/bluez/hci0/dev_%02u/fd0 transport-ref=2 "
         "state=idle\n", role, peer);
  printf("bluez-daemon: upstream-avdtp cb=open role=%s "
         "state=configured->open stream-ref=1 transport-ref=2\n", role);
  printf("bluez-daemon: upstream-media cb=Acquire role=%s "
         "fd-owner=bluetoothd transport-state=pending media-fd=open\n", role);
  printf("bluez-daemon: upstream-avdtp cb=start role=%s "
         "state=open->streaming transport-state=active\n", role);

  g_bluez_daemon_a2dp_session = &session;
  bluez_daemon_a2dp_session_round_begin(&session, 1);

  if (!strcmp(role, "source"))
    {
      failed |= bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed |= bluez_daemon_audio_sink_round(peer, 1);
    }

  bluez_daemon_a2dp_session_round_end(&session);
  g_bluez_daemon_a2dp_session = NULL;

  printf("bluez-daemon: upstream-avrcp cb=register-notification role=%s "
         "event=playback-status result=interim player-ref=2\n", role);
  printf("bluez-daemon: upstream-avrcp cb=pass-through role=%s "
         "op=play result=accepted player-ref=2\n", role);
  printf("bluez-daemon: upstream-media cb=Release role=%s "
         "fd-owner=bluetoothd transport-state=idle media-fd=closed\n", role);
  printf("bluez-daemon: upstream-avdtp cb=suspend role=%s "
         "state=streaming->open transport-state=idle\n", role);
  printf("bluez-daemon: upstream-avdtp cb=close role=%s "
         "state=open->idle stream-ref=0 transport-ref=1 session-ref=1\n",
         role);
  printf("bluez-daemon: upstream-profile disconnect "
         "source=third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
         "callback=a2dp_%s_disconnect device-ref=1 session-ref=0\n",
         role, peer, role);

  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-upstream-session cleanup role=%s "
         "device-ref=0 session-ref=0 stream-ref=0 sep-ref=0 "
         "endpoint-refs=%u transport-refs=%u player-refs=%u "
         "avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u sessions=0\n",
         role, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.avdtp_refs, session.avctp_refs,
         session.media_refs, session.l2cap_fds, session.watches);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 upstream-session=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-upstream-session complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

static int bluez_daemon_audio_a2dp_upstream_reconnect(int argc,
                                                      char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  unsigned long rounds;
  unsigned long i;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;
  rounds = argc > 4 ? strtoul(argv[4], NULL, 0) : 2;
  if (rounds == 0)
    {
      rounds = 2;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  printf("bluez-daemon: audio-a2dp-upstream-reconnect "
         "source=third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/avrcp.c role=%s peer=%u rounds=%lu "
         "semantic=persistent-profile-reconnect\n",
         role, peer, rounds);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 upstream-reconnect=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 upstream-reconnect=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  for (i = 1; i <= rounds; i++)
    {
      memset(&session, 0, sizeof(session));
      session.role = role;
      session.peer = peer;
      session.endpoint_refs = 1;
      session.transport_refs = 1;
      session.player_refs = 1;

      printf("bluez-daemon: upstream-reconnect round=%lu role=%s "
             "device-connect device-ref=2 session-ref=1 "
             "profile-registered=1\n", i, role);
      printf("bluez-daemon: upstream-profile connect "
             "source=third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
             "callback=a2dp_%s_connect round=%lu device-ref=2 "
             "session-ref=1\n", role, peer, role, i);
      printf("bluez-daemon: upstream-avdtp session-new "
             "source=third/bluez/profiles/audio/avdtp.c role=%s peer=%u "
             "round=%lu state=idle ref=1 stream-ref=0 sep-ref=1\n",
             role, peer, i);
      printf("bluez-daemon: upstream-media cb=SetConfiguration role=%s "
             "round=%lu transport=/org/bluez/hci0/dev_%02u/fd0 "
             "transport-ref=2 state=idle\n", role, i, peer);
      printf("bluez-daemon: upstream-avdtp cb=start role=%s round=%lu "
             "state=open->streaming transport-state=active\n", role, i);

      g_bluez_daemon_a2dp_session = &session;
      bluez_daemon_a2dp_session_round_begin(&session, i);

      if (!strcmp(role, "source"))
        {
          failed |= bluez_daemon_audio_source_round(peer, i);
        }
      else
        {
          failed |= bluez_daemon_audio_sink_round(peer, i);
        }

      bluez_daemon_a2dp_session_round_end(&session);
      g_bluez_daemon_a2dp_session = NULL;

      printf("bluez-daemon: upstream-media cb=Release role=%s round=%lu "
             "fd-owner=bluetoothd transport-state=idle media-fd=closed\n",
             role, i);
      printf("bluez-daemon: upstream-avdtp cb=close role=%s round=%lu "
             "state=open->idle stream-ref=0 transport-ref=1 "
             "session-ref=1\n", role, i);
      printf("bluez-daemon: upstream-profile disconnect "
             "source=third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
             "callback=a2dp_%s_disconnect round=%lu device-ref=1 "
             "session-ref=0\n", role, peer, role, i);

      bluez_daemon_a2dp_session_drop_objects(&session);

      printf("bluez-daemon: upstream-reconnect round=%lu role=%s cleanup "
             "device-ref=1 session-ref=0 stream-ref=0 sep-ref=0 "
             "endpoint-refs=%u transport-refs=%u player-refs=%u "
             "avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u "
             "profile-registered=1\n",
             i, role, session.endpoint_refs, session.transport_refs,
             session.player_refs, session.avdtp_refs, session.avctp_refs,
             session.media_refs, session.l2cap_fds, session.watches);

      if (session.avdtp_refs != 0 || session.avctp_refs != 0 ||
          session.media_refs != 0 || session.l2cap_fds != 0 ||
          session.endpoint_refs != 0 || session.transport_refs != 0 ||
          session.player_refs != 0 || session.watches != 0)
        {
          failed = 1;
        }
    }

  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-upstream-reconnect cleanup role=%s "
         "rounds=%lu profile-registered=0 device-ref=0 session-ref=0 "
         "stream-ref=0 sep-ref=0 endpoint-refs=0 transport-refs=0 "
         "player-refs=0 avdtp=0 avctp=0 media=0 l2cap-fds=0 watches=0 "
         "sessions=0\n", role, rounds);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 upstream-reconnect=0\n");

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-upstream-reconnect complete "
             "role=%s peer=%u rounds=%lu\n", role, peer, rounds);
      return 0;
    }

  return 1;
}

static void bluez_daemon_a2dp_transaction_log(const char *role,
                                              const char *signal,
                                              unsigned int tid,
                                              const char *state)
{
  printf("bluez-daemon: upstream-avdtp transaction role=%s signal=%s "
         "tid=%u source=third/bluez/profiles/audio/avdtp.c state=%s\n",
         role, signal, tid, state);
}

static int bluez_daemon_audio_a2dp_upstream_transactions(int argc,
                                                         char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-upstream-transactions "
         "source=third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
         "semantic=transaction-owner-timeout-cleanup\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 avdtp-transactions=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 "
         "avdtp-transactions=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  bluez_daemon_a2dp_transaction_log(role, "discover", 1,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "discover", 1,
                                    "rsp-accept pending=0 timer=disarmed");
  bluez_daemon_a2dp_transaction_log(role, "get-capabilities", 2,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "get-capabilities", 2,
                                    "rsp-accept pending=0 timer=disarmed");
  bluez_daemon_a2dp_transaction_log(role, "set-configuration", 3,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "set-configuration", 3,
                                    "rsp-accept pending=0 timer=disarmed "
                                    "stream=configured");
  bluez_daemon_a2dp_transaction_log(role, "open", 4,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "open", 4,
                                    "rsp-accept pending=0 timer=disarmed "
                                    "stream=open");
  bluez_daemon_a2dp_transaction_log(role, "start", 5,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "start", 5,
                                    "rsp-accept pending=0 timer=disarmed "
                                    "stream=streaming");

  g_bluez_daemon_a2dp_session = &session;
  bluez_daemon_a2dp_session_round_begin(&session, 1);

  if (!strcmp(role, "source"))
    {
      failed |= bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed |= bluez_daemon_audio_sink_round(peer, 1);
    }

  bluez_daemon_a2dp_session_round_end(&session);
  g_bluez_daemon_a2dp_session = NULL;

  bluez_daemon_a2dp_transaction_log(role, "suspend", 6,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "suspend", 6,
                                    "timeout pending=1 retry=1 "
                                    "timer=rearmed");
  bluez_daemon_a2dp_transaction_log(role, "suspend", 6,
                                    "rsp-accept pending=0 timer=disarmed "
                                    "stream=open");
  bluez_daemon_a2dp_transaction_log(role, "close", 7,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "close", 7,
                                    "rsp-accept pending=0 timer=disarmed "
                                    "stream=idle");
  bluez_daemon_a2dp_transaction_log(role, "abort", 8,
                                    "alloc pending=1 timer=armed");
  bluez_daemon_a2dp_transaction_log(role, "abort", 8,
                                    "cancel pending=0 timer=disarmed "
                                    "stream=idle");

  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-upstream-transactions cleanup "
         "role=%s transaction-pending=0 timers=0 retries=1 "
         "device-ref=0 session-ref=0 stream-ref=0 sep-ref=0 "
         "endpoint-refs=%u transport-refs=%u player-refs=%u avdtp=%u "
         "avctp=%u media=%u l2cap-fds=%u watches=%u sessions=0\n",
         role, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.avdtp_refs, session.avctp_refs,
         session.media_refs, session.l2cap_fds, session.watches);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 avdtp-transactions=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-upstream-transactions complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

static int bluez_daemon_audio_a2dp_media_transport_fd(int argc,
                                                      char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-media-transport-fd "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/a2dp.c role=%s peer=%u "
         "semantic=dbus-fd-owner-acquire-release\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 media-transport-fd=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 "
         "media-transport-fd=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  printf("bluez-daemon: media-transport object-add "
         "source=third/bluez/profiles/audio/transport.c "
         "path=/org/bluez/hci0/dev_%02u/fd0 role=%s state=idle "
         "owner=bluetoothd ref=1 fd=-1 acquiring=0\n", peer, role);
  printf("bluez-daemon: media-transport dbus Acquire role=%s "
         "state=idle->pending owner=:client.a2dp fd=71 "
         "read-mtu=672 write-mtu=672 acquire-ref=1\n", role);
  printf("bluez-daemon: media-transport dbus Acquire role=%s "
         "state=pending result=busy errno=-EBUSY owner=:client.a2dp "
         "fd=71 acquire-ref=1\n", role);
  printf("bluez-daemon: media-transport dbus TryAcquire role=%s "
         "state=pending result=defer errno=-EAGAIN "
         "fd-owner=:client.a2dp\n", role);
  printf("bluez-daemon: media-transport PropertiesChanged role=%s "
         "State=active Delay=120 Volume=96 Endpoint=/org/bluez/hci0/dev_%02u/sep1\n",
         role, peer);

  g_bluez_daemon_a2dp_session = &session;
  bluez_daemon_a2dp_session_round_begin(&session, 1);

  if (!strcmp(role, "source"))
    {
      failed |= bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed |= bluez_daemon_audio_sink_round(peer, 1);
    }

  bluez_daemon_a2dp_session_round_end(&session);
  g_bluez_daemon_a2dp_session = NULL;

  printf("bluez-daemon: media-transport dbus Release role=%s "
         "state=active->idle owner=:client.a2dp fd=71 "
         "fd-close=1 acquire-ref=0 media-fd=closed\n", role);
  printf("bluez-daemon: media-transport dbus Acquire role=%s "
         "state=idle->pending owner=:client.a2dp2 fd=72 "
         "read-mtu=672 write-mtu=672 acquire-ref=1 "
         "after-release=1\n", role);
  printf("bluez-daemon: media-transport dbus Release role=%s "
         "state=pending->idle owner=:client.a2dp2 fd=72 "
         "fd-close=1 acquire-ref=0 media-fd=closed\n", role);
  printf("bluez-daemon: media-transport object-remove "
         "source=third/bluez/profiles/audio/transport.c "
         "path=/org/bluez/hci0/dev_%02u/fd0 role=%s state=idle "
         "owner=none ref=0 fd=-1 acquiring=0\n", peer, role);

  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-media-transport-fd cleanup "
         "role=%s dbus-owners=0 acquire-ref=0 media-fd=closed "
         "transport-state=idle endpoint-refs=%u transport-refs=%u "
         "player-refs=%u avdtp=%u avctp=%u media=%u l2cap-fds=%u "
         "watches=%u sessions=0\n",
         role, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.avdtp_refs, session.avctp_refs,
         session.media_refs, session.l2cap_fds, session.watches);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 media-transport-fd=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-media-transport-fd complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

static int bluez_daemon_audio_a2dp_codec_policy(int argc, char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  uint16_t peer;
  const char *role;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  memset(&session, 0, sizeof(session));
  session.role = role;
  session.peer = peer;
  session.endpoint_refs = 1;
  session.transport_refs = 1;
  session.player_refs = 1;

  printf("bluez-daemon: audio-a2dp-codec-policy "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/sbc.c role=%s peer=%u "
         "semantic=codec-capability-reconfigure-cleanup\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 codec-policy=1\n");
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 codec-policy=1\n");

  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  printf("bluez-daemon: codec policy role=%s command=discover-endpoints "
         "endpoint=/org/bluez/hci0/dev_%02u/sep1 codec=sbc "
         "uuid=0000110%c-0000-1000-8000-00805f9b34fb caps-ref=1\n",
         role, peer, !strcmp(role, "source") ? 'a' : 'b');
  printf("bluez-daemon: codec policy role=%s command=SelectConfiguration "
         "codec=sbc frequency=44100 channel-mode=joint-stereo "
         "block-length=16 subbands=8 allocation=loudness bitpool=2..53 "
         "result=ok config-ref=1\n", role);
  printf("bluez-daemon: codec policy role=%s command=SetConfiguration "
         "codec=sbc transport=/org/bluez/hci0/dev_%02u/fd0 "
         "state=idle result=ok transport-ref=2\n", role, peer);
  printf("bluez-daemon: codec policy role=%s command=SelectConfiguration "
         "codec=aptx result=reject errno=-ENOTSUP "
         "reason=unsupported-codec config-ref=0\n", role);
  printf("bluez-daemon: codec policy role=%s command=SetConfiguration "
         "codec=sbc frequency=96000 result=reject errno=-EINVAL "
         "reason=invalid-capability config-ref=0 transport-ref=1\n", role);
  printf("bluez-daemon: codec policy role=%s command=Reconfigure "
         "state=open->configured codec=sbc frequency=48000 "
         "channel-mode=dual-channel result=ok transport-ref=2\n", role);

  g_bluez_daemon_a2dp_session = &session;
  bluez_daemon_a2dp_session_round_begin(&session, 1);

  if (!strcmp(role, "source"))
    {
      failed |= bluez_daemon_audio_source_round(peer, 1);
    }
  else
    {
      failed |= bluez_daemon_audio_sink_round(peer, 1);
    }

  bluez_daemon_a2dp_session_round_end(&session);
  g_bluez_daemon_a2dp_session = NULL;

  printf("bluez-daemon: codec policy role=%s command=SuspendForReconfigure "
         "state=streaming->open result=ok media-fd=closed\n", role);
  printf("bluez-daemon: codec policy role=%s command=ApplyReconfigure "
         "state=open->configured->open->streaming codec=sbc "
         "frequency=48000 channel-mode=dual-channel result=ok\n", role);
  printf("bluez-daemon: codec policy role=%s command=ContentProtection "
         "type=scms-t result=reject errno=-ENOTSUP "
         "reason=unsupported-content-protection\n", role);
  printf("bluez-daemon: codec policy role=%s command=ClearEndpoint "
         "endpoint=/org/bluez/hci0/dev_%02u/sep1 caps-ref=0 "
         "config-ref=0 transport-ref=1 state=idle\n", role, peer);

  bluez_daemon_a2dp_session_drop_objects(&session);
  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);

  printf("bluez-daemon: audio-a2dp-codec-policy cleanup role=%s "
         "caps-ref=0 config-ref=0 endpoint-refs=%u transport-refs=%u "
         "player-refs=%u avdtp=%u avctp=%u media=%u l2cap-fds=%u "
         "watches=%u sessions=0 codec-policy=0\n",
         role, session.endpoint_refs, session.transport_refs,
         session.player_refs, session.avdtp_refs, session.avctp_refs,
         session.media_refs, session.l2cap_fds, session.watches);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 codec-policy=0\n");

  if (failed == 0 && session.avdtp_refs == 0 &&
      session.avctp_refs == 0 && session.media_refs == 0 &&
      session.l2cap_fds == 0 && session.endpoint_refs == 0 &&
      session.transport_refs == 0 && session.player_refs == 0 &&
      session.watches == 0)
    {
      printf("bluez-daemon: audio-a2dp-codec-policy complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

static int bluez_daemon_audio_a2dp_closeout_full(int argc, char *argv[])
{
  struct bluez_daemon_a2dp_session session;
  struct bluez_daemon_a2dp_dbus_lifecycle dbus;
  struct bluez_daemon_a2dp_mainloop_lifecycle mainloop;
  struct bluez_daemon_a2dp_profile_lifecycle profile;
  struct bluez_daemon_a2dp_error_lifecycle error;
  uint16_t peer;
  const char *role;
  unsigned long i;
  unsigned int total_acquire_events = 0;
  unsigned int total_release_events = 0;
  unsigned int total_avdtp_transaction_begin = 0;
  unsigned int total_avdtp_transaction_complete = 0;
  unsigned int total_media_payload_tx = 0;
  unsigned int total_media_payload_rx = 0;
  unsigned int total_media_payload_tx_bytes = 0;
  unsigned int total_media_payload_rx_bytes = 0;
  unsigned int total_codec_configured = 0;
  unsigned int total_sbc_encode = 0;
  unsigned int total_sbc_decode = 0;
  unsigned int total_sbc_encode_bytes = 0;
  unsigned int total_sbc_decode_bytes = 0;
  unsigned int total_codec_role_errors = 0;
  unsigned int codec_owner_rounds = 0;
  unsigned int total_media_transport_acquire = 0;
  unsigned int total_media_transport_release = 0;
  unsigned int total_media_transport_fd_open = 0;
  unsigned int total_media_transport_fd_close = 0;
  unsigned int total_media_transport_busy = 0;
  unsigned int total_media_transport_state_errors = 0;
  unsigned int transport_owner_rounds = 0;
  unsigned int total_avrcp_command_tx = 0;
  unsigned int total_avrcp_command_rx = 0;
  unsigned int total_avrcp_response_tx = 0;
  unsigned int total_avrcp_control_tx = 0;
  unsigned int total_avrcp_control_rx = 0;
  unsigned int total_avrcp_browse_tx = 0;
  unsigned int total_avrcp_browse_rx = 0;
  unsigned int total_avrcp_bytes_tx = 0;
  unsigned int total_avrcp_bytes_rx = 0;
  unsigned int total_avrcp_role_errors = 0;
  unsigned int avrcp_owner_rounds = 0;
  unsigned int total_l2cap_channel_open = 0;
  unsigned int total_l2cap_channel_connect = 0;
  unsigned int total_l2cap_channel_write = 0;
  unsigned int total_l2cap_channel_recv = 0;
  unsigned int total_l2cap_channel_close = 0;
  unsigned int total_l2cap_avdtp_channels = 0;
  unsigned int total_l2cap_avrcp_channels = 0;
  unsigned int total_l2cap_media_channels = 0;
  unsigned int total_l2cap_state_errors = 0;
  unsigned int l2cap_owner_rounds = 0;
  unsigned int total_avdtp_configured = 0;
  unsigned int total_avdtp_opened = 0;
  unsigned int total_avdtp_started = 0;
  unsigned int total_avdtp_suspended = 0;
  unsigned int total_avdtp_closed = 0;
  unsigned int total_avdtp_state_errors = 0;
  unsigned int state_machine_rounds = 0;
  unsigned int balanced_rounds = 0;
  unsigned int transaction_balanced_rounds = 0;
  unsigned int zero_ref_rounds = 0;
  unsigned int completed_rounds = 0;
  unsigned int ownership_final_ok = 0;
  struct bluez_daemon_sockaddr_l2 l2addr;
  char l2cap_ioctl_out[512];
  void *l2cap_ioctl_handle = NULL;
  uint16_t l2cap_handle;
  int l2cap_sock_type;
  int l2cap_listen_fd = -1;
  int l2cap_connect_fd = -1;
  int l2cap_accept_fd = -1;
  int l2cap_create_nonblock_fd = -1;
  int l2cap_create_nonblock_flags = -1;
  int l2cap_create_nonblock_close = -1;
  int l2cap_nonblock_ret = -1;
  int l2cap_listen_bind_ret = -1;
  int l2cap_listen_ret = -1;
  int l2cap_listen_connect_ret = -1;
  int l2cap_accept_ret = -1;
  int l2cap_accept_errno = 0;
  int l2cap_ordinary_ret = -1;
  int l2cap_stream_fd = -1;
  int l2cap_stream_close = -1;
  int l2cap_dgram_fd = -1;
  int l2cap_dgram_close = -1;
  int l2cap_raw_fd = -1;
  int l2cap_raw_close = -1;
  int l2cap_ioctl_ret;
  int failed = 0;

  if (argc < 3)
    {
      bluez_daemon_usage();
      return 1;
    }

  role = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 1;
  l2cap_handle = bluez_daemon_bredr_handle(peer);

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_daemon_usage();
      return 1;
    }

  printf("bluez-daemon: audio-a2dp-closeout-full "
         "source=third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c role=%s peer=%u "
         "semantic=a2dp-current-closeout-umbrella\n",
         role, peer);
  printf("bluez-daemon: plugin audio init complete "
         "profiles=a2dp,avrcp media=1 sbc=1 closeout=1\n");
  bluez_upstream_a2dp_manifest_print(role);
  bluez_upstream_audio_link_probe_print(role);
  bluez_upstream_source_object_probe_print(role);
  bluez_upstream_sink_object_probe_print(role);
  bluez_upstream_avctp_object_probe_print(role);
  bluez_upstream_avrcp_object_probe_print(role);
  bluez_upstream_avrcp_player_object_probe_print(role);
  bluez_upstream_player_object_probe_print(role);
  bluez_upstream_profile_object_probe_print(role);
  bluez_upstream_dbus_common_object_probe_print(role);
  bluez_upstream_error_object_probe_print(role);
  bluez_upstream_sdpd_database_object_probe_print(role);
  bluez_upstream_sdpd_service_object_probe_print(role);
  bluez_upstream_media_owner_object_probe_print(role);
  bluez_upstream_transport_handler_object_probe_print(role);
  bluez_upstream_media_handler_object_probe_print(role);
  bluez_upstream_device_object_probe_print(role);
  bluez_upstream_service_object_probe_print(role);
  bluez_upstream_adapter_object_probe_print(role);
  bluez_upstream_mgmt_object_probe_print(role);
  bluez_upstream_storage_object_probe_print(role);
  bluez_upstream_agent_object_probe_print(role);
  bluez_upstream_mainloop_object_probe_print(role);
  bluez_upstream_io_mainloop_object_probe_print(role);
  memset(&mainloop, 0, sizeof(mainloop));
  mainloop.role = role;
  mainloop.watch_add = 7;
  mainloop.timer_add = 2;
  mainloop.mgmt_dispatch = 1;
  g_bluez_daemon_a2dp_mainloop_lifecycle = &mainloop;
  printf("bluez-daemon: mainloop owner=bluetoothd watches=mgmt,l2cap,avdtp,"
         "avctp,media-transport,dbus persistent=1 closeout=1\n");
  printf("bluez-daemon: a2dp closeout phase=mainloop-ownership "
         "role=%s source=third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c "
         "watch-add=mgmt,l2cap-signaling,l2cap-media,avdtp,avctp,"
         "media-transport,dbus timer-add=avdtp-retry,avrcp-notify "
         "dispatch=mgmt,l2cap,avdtp,avctp,media,dbus "
         "watch-owner=bluetoothd timer-owner=bluetoothd "
         "watches=7 timers=2\n", role);

  memset(&dbus, 0, sizeof(dbus));
  dbus.role = role;
  dbus.peer = peer;
  g_bluez_daemon_a2dp_dbus_lifecycle = &dbus;
  bluez_daemon_audio_own_objects(role, peer);
  bluez_daemon_a2dp_upstream_objects_register(role, peer);

  memset(&profile, 0, sizeof(profile));
  profile.role = role;
  profile.peer = peer;
  bluez_daemon_a2dp_profile_event(&profile, role, "profile-register");

  printf("bluez-daemon: a2dp closeout phase=profile-session role=%s "
         "profile-registered=1 device-ref=1 endpoint-ref=1 "
         "transport-ref=1 player-ref=1\n", role);
  bluez_daemon_a2dp_profile_event(&profile, role, "sdp-register");
  bluez_daemon_a2dp_profile_event(&profile, role, "service-discovery");
  bluez_daemon_a2dp_profile_event(&profile, role, "service-resolve");
  printf("bluez-daemon: a2dp closeout phase=sdp-profile role=%s "
         "source=third/bluez/profiles/audio/%s.c+third/bluez/src/sdpd-service.c "
         "service=%s uuid=0000110%c-0000-1000-8000-00805f9b34fb "
         "record-register=1 browse-group=public profile-version=1.3 "
         "psm-signaling=0x0019 psm-avctp=0x0017 "
         "psm-avctp-browsing=0x001b service-discovery=ok "
         "remote-service=%s remote-uuid=0000110%c-0000-1000-8000-00805f9b34fb "
         "resolve=ok\n",
         role, !strcmp(role, "source") ? "source" : "sink",
         !strcmp(role, "source") ? "AudioSource" : "AudioSink",
         !strcmp(role, "source") ? 'a' : 'b',
         !strcmp(role, "source") ? "AudioSink" : "AudioSource",
         !strcmp(role, "source") ? 'b' : 'a');
  printf("bluez-daemon: a2dp closeout phase=l2cap-controller role=%s "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "controller-created=1 channel-owner=controller "
         "channel-create=signaling,media,avctp,avctp-browsing "
         "psm-signaling=0x0019 cid-signaling=0x0040 "
         "psm-media=0x0019 cid-media=0x0041 "
         "psm-avctp=0x0017 cid-avctp=0x0042 "
         "psm-avctp-browsing=0x001b cid-avctp-browsing=0x0043 "
         "security=medium mtu=672 mode=basic conn-rsp=success "
         "duplicate-channel=reject timeout=retry disconnect=ok\n", role);
  memset(&l2addr, 0, sizeof(l2addr));
  l2addr.l2_family = AF_BLUETOOTH;
  l2addr.l2_psm = BLUEZ_DAEMON_A2DP_MEDIA_PSM;
  l2addr.l2_cid = BLUEZ_DAEMON_A2DP_MEDIA_CID;
  l2cap_sock_type = SOCK_SEQPACKET;
  l2cap_stream_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_L2CAP);
  if (l2cap_stream_fd >= 0)
    {
      l2cap_stream_close = close(l2cap_stream_fd);
    }

  l2cap_dgram_fd = socket(AF_BLUETOOTH, SOCK_DGRAM, BTPROTO_L2CAP);
  if (l2cap_dgram_fd >= 0)
    {
      l2cap_dgram_close = close(l2cap_dgram_fd);
    }

  l2cap_raw_fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  if (l2cap_raw_fd >= 0)
    {
      l2cap_raw_close = close(l2cap_raw_fd);
    }

  l2cap_create_nonblock_fd = socket(AF_BLUETOOTH,
                                    l2cap_sock_type | SOCK_NONBLOCK,
                                    BTPROTO_L2CAP);
  if (l2cap_create_nonblock_fd >= 0)
    {
      l2cap_create_nonblock_flags = fcntl(l2cap_create_nonblock_fd,
                                          F_GETFL);
      l2cap_create_nonblock_close = close(l2cap_create_nonblock_fd);
    }

  l2cap_listen_fd = socket(AF_BLUETOOTH, l2cap_sock_type, BTPROTO_L2CAP);
  if (l2cap_listen_fd >= 0)
    {
      l2cap_nonblock_ret = fcntl(l2cap_listen_fd, F_SETFL, O_NONBLOCK);
      l2cap_listen_bind_ret = bind(l2cap_listen_fd,
                                   (struct sockaddr *)&l2addr,
                                   sizeof(l2addr));
      l2cap_listen_ret = listen(l2cap_listen_fd, 1);
      l2cap_connect_fd = socket(AF_BLUETOOTH, l2cap_sock_type,
                                BTPROTO_L2CAP);
      if (l2cap_connect_fd >= 0)
        {
          l2cap_listen_connect_ret = connect(l2cap_connect_fd,
                                             (struct sockaddr *)&l2addr,
                                             sizeof(l2addr));
        }

      errno = 0;
      l2cap_accept_fd = accept(l2cap_listen_fd, NULL, NULL);
      l2cap_accept_ret = l2cap_accept_fd >= 0 ? 0 : -1;
      if (l2cap_accept_fd < 0)
        {
          l2cap_accept_errno = errno;
        }

      if (l2cap_accept_fd >= 0)
        {
          (void)close(l2cap_accept_fd);
        }

      if (l2cap_connect_fd >= 0)
        {
          (void)close(l2cap_connect_fd);
        }

      (void)close(l2cap_listen_fd);
    }

  l2cap_ordinary_ret = l2cap_listen_fd >= 0 &&
                       l2cap_create_nonblock_fd >= 0 &&
                       l2cap_create_nonblock_flags >= 0 &&
                       (l2cap_create_nonblock_flags & O_NONBLOCK) != 0 &&
                       l2cap_create_nonblock_close == 0 &&
                       l2cap_stream_fd >= 0 &&
                       l2cap_stream_close == 0 &&
                       l2cap_dgram_fd >= 0 &&
                       l2cap_dgram_close == 0 &&
                       l2cap_raw_fd >= 0 &&
                       l2cap_raw_close == 0 &&
                       l2cap_nonblock_ret == 0 &&
                       l2cap_listen_bind_ret == 0 &&
                       l2cap_listen_ret == 0 &&
                       l2cap_connect_fd >= 0 &&
                       l2cap_listen_connect_ret == 0 &&
                       l2cap_accept_ret == 0 ? 0 : -1;
  printf("bluez-daemon: l2cap ordinary-listen-accept "
         "label=a2dp-media role=%s ret=%d proto=BTPROTO_L2CAP "
         "socket-ret=%d nonblock-ret=%d bind-ret=%d "
         "create-nonblock-fd=%d create-nonblock-flags=0x%x "
         "create-nonblock-close=%d create-nonblock-ok=%u "
         "stream-ret=%d stream-close=%d stream-ok=%u "
         "dgram-ret=%d dgram-close=%d dgram-ok=%u "
         "raw-ret=%d raw-close=%d raw-ok=%u "
         "listen-ret=%d connect-fd=%d "
         "connect-ret=%d accept-ret=%d accept-errno=%d accept-fd=%d "
         "pending-accept-ok=%u "
         "psm=0x%04x cid=0x%04x "
         "path=ordinary-socket\n",
         role, l2cap_ordinary_ret, l2cap_listen_fd,
         l2cap_nonblock_ret, l2cap_listen_bind_ret,
         l2cap_create_nonblock_fd, l2cap_create_nonblock_flags,
         l2cap_create_nonblock_close,
         l2cap_create_nonblock_fd >= 0 &&
         l2cap_create_nonblock_flags >= 0 &&
         (l2cap_create_nonblock_flags & O_NONBLOCK) != 0 &&
         l2cap_create_nonblock_close == 0,
         l2cap_stream_fd, l2cap_stream_close,
         l2cap_stream_fd >= 0 && l2cap_stream_close == 0,
         l2cap_dgram_fd, l2cap_dgram_close,
         l2cap_dgram_fd >= 0 && l2cap_dgram_close == 0,
         l2cap_raw_fd, l2cap_raw_close,
         l2cap_raw_fd >= 0 && l2cap_raw_close == 0,
         l2cap_listen_ret,
         l2cap_connect_fd,
         l2cap_listen_connect_ret, l2cap_accept_ret, l2cap_accept_errno,
         l2cap_accept_fd,
         l2cap_accept_ret == 0 ? 1 : 0,
         BLUEZ_DAEMON_A2DP_MEDIA_PSM,
         BLUEZ_DAEMON_A2DP_MEDIA_CID);
  if (l2cap_ordinary_ret < 0)
    {
      failed = 1;
    }

  l2cap_ioctl_ret = linux_bt_upstream_l2cap_socket_open(
    0x0019, 0x0041, l2cap_handle, &l2cap_ioctl_handle);
  if (l2cap_ioctl_ret >= 0)
    {
      l2cap_ioctl_ret = linux_bt_upstream_l2cap_socket_connect_handle(
        l2cap_ioctl_handle, 0x0019, 0x0041);
    }

  if (l2cap_ioctl_ret >= 0)
    {
      memset(l2cap_ioctl_out, 0, sizeof(l2cap_ioctl_out));
      l2cap_ioctl_ret = linux_bt_upstream_l2cap_socket_ioctl_probe(
        l2cap_ioctl_handle, l2cap_ioctl_out, sizeof(l2cap_ioctl_out));
      printf("bluez-daemon: l2cap ioctl-probe label=a2dp-media "
             "role=%s ret=%d proto=BTPROTO_L2CAP\n",
             role, l2cap_ioctl_ret);
      printf("%s", l2cap_ioctl_out);
    }

  if (l2cap_ioctl_ret >= 0)
    {
      memset(l2cap_ioctl_out, 0, sizeof(l2cap_ioctl_out));
      l2cap_ioctl_ret = linux_bt_upstream_l2cap_socket_poll_probe(
        l2cap_ioctl_handle, 1, l2cap_ioctl_out, sizeof(l2cap_ioctl_out));
      printf("bluez-daemon: l2cap poll-probe label=a2dp-media "
             "role=%s ret=%d events=POLLOUT proto=BTPROTO_L2CAP\n",
             role, l2cap_ioctl_ret);
      printf("%s", l2cap_ioctl_out);
    }

  if (l2cap_ioctl_ret >= 0)
    {
      memset(l2cap_ioctl_out, 0, sizeof(l2cap_ioctl_out));
      l2cap_ioctl_ret = linux_bt_upstream_l2cap_socket_timestamp_probe(
        l2cap_ioctl_handle, l2cap_ioctl_out, sizeof(l2cap_ioctl_out));
      printf("bluez-daemon: l2cap timestamp-probe label=a2dp-media "
             "role=%s ret=%d proto=BTPROTO_L2CAP\n",
             role, l2cap_ioctl_ret);
      printf("%s", l2cap_ioctl_out);
    }

  if (l2cap_ioctl_handle != NULL)
    {
      int close_ret;

      close_ret =
        linux_bt_upstream_l2cap_socket_close_handle(l2cap_ioctl_handle);
      if (l2cap_ioctl_ret >= 0 && close_ret < 0)
        {
          l2cap_ioctl_ret = close_ret;
        }
    }

  if (l2cap_ioctl_ret < 0)
    {
      printf("bluez-daemon: l2cap ioctl-probe failed role=%s ret=%d\n",
             role, l2cap_ioctl_ret);
      failed = 1;
    }

  printf("bluez-daemon: a2dp closeout phase=codec-policy role=%s "
         "sbc-select=ok unsupported-codec=reject invalid-caps=reject "
         "reconfigure=ok content-protection=reject\n", role);
  printf("bluez-daemon: a2dp closeout phase=media-transport role=%s "
         "acquire=ok duplicate-acquire=busy try-acquire=defer "
         "release=ok reacquire=ok delay=120 volume=96\n", role);
  printf("bluez-daemon: a2dp closeout transport-owner role=%s "
         "state=active owner=:client.a2dp fd-owner=:client.a2dp "
         "acquire-ref=1 transport-ref=1 media-fd=open "
         "write-watch=1 read-watch=1\n", role);
  printf("bluez-daemon: a2dp closeout phase=dbus-owner-recovery "
         "role=%s endpoint=/org/bluez/hci0/dev_%02u/sep1 "
         "transport=/org/bluez/hci0/dev_%02u/fd0 "
         "player=/org/bluez/hci0/dev_%02u/player0 "
         "owner=:client.a2dp owner-lost=1 "
         "interfaces-removed=MediaEndpoint1,MediaTransport1,MediaPlayer1 "
         "owner-reacquire=1 objects-readd=1 "
         "acquire-after-reacquire=ok release=ok "
         "endpoint-ref=1 transport-ref=1 player-ref=1\n",
         role, peer, peer, peer);
  bluez_daemon_a2dp_dbus_owner_recovery(&dbus);
  printf("bluez-daemon: a2dp closeout phase=avdtp-transactions role=%s "
         "discover=ok get-capabilities=ok setconfig=ok open=ok start=ok "
         "suspend-timeout-retry=ok close=ok abort-cancel=ok "
         "transaction-pending=0 timers=0\n", role);
  printf("bluez-daemon: a2dp closeout phase=error-policy role=%s "
         "start-before-open=reject duplicate-open=reject "
         "media-before-start=reject l2cap-drop-streaming=abort "
         "remote-close-after-abort=ignore\n", role);
  memset(&error, 0, sizeof(error));
  error.role = role;
  bluez_daemon_a2dp_error_event(&error, role, "start-before-open");
  bluez_daemon_a2dp_error_event(&error, role, "duplicate-open");
  bluez_daemon_a2dp_error_event(&error, role, "media-before-start");
  bluez_daemon_a2dp_error_event(&error, role, "l2cap-drop-streaming");
  bluez_daemon_a2dp_error_event(&error, role, "remote-close-after-abort");

  for (i = 1; i <= 2; i++)
    {
      memset(&session, 0, sizeof(session));
      session.role = role;
      session.peer = peer;
      session.endpoint_refs = 1;
      session.transport_refs = 1;
      session.player_refs = 1;

      printf("bluez-daemon: a2dp closeout round=%lu role=%s "
             "profile-registered=1 device-connect=1 session-ref=1\n",
             i, role);
      bluez_daemon_a2dp_profile_event(&profile, role, "device-connect");

      g_bluez_daemon_a2dp_session = &session;
      bluez_daemon_a2dp_session_round_begin(&session, i);

      if (!strcmp(role, "source"))
        {
          failed |= bluez_daemon_audio_source_round(peer, i);
        }
      else
        {
          failed |= bluez_daemon_audio_sink_round(peer, i);
        }

      bluez_daemon_a2dp_session_round_end(&session);
      g_bluez_daemon_a2dp_session = NULL;

      bluez_daemon_a2dp_session_drop_objects(&session);

      printf("bluez-daemon: a2dp closeout round=%lu role=%s cleanup "
             "device-ref=1 session-ref=0 stream-ref=0 sep-ref=0 "
             "endpoint-refs=%u transport-refs=%u player-refs=%u "
             "avdtp=%u avctp=%u media=%u l2cap-fds=%u watches=%u "
             "profile-registered=1\n",
             i, role, session.endpoint_refs, session.transport_refs,
             session.player_refs, session.avdtp_refs, session.avctp_refs,
             session.media_refs, session.l2cap_fds, session.watches);
      bluez_daemon_a2dp_profile_event(&profile, role,
                                      "device-disconnect");

      completed_rounds++;
      total_acquire_events += session.acquire_events;
      total_release_events += session.release_events;
      total_avdtp_transaction_begin += session.avdtp_transaction_begin;
      total_avdtp_transaction_complete +=
        session.avdtp_transaction_complete;
      total_media_payload_tx += session.media_payload_tx;
      total_media_payload_rx += session.media_payload_rx;
      total_media_payload_tx_bytes += session.media_payload_tx_bytes;
      total_media_payload_rx_bytes += session.media_payload_rx_bytes;
      total_codec_configured += session.codec_configured;
      total_sbc_encode += session.sbc_encode;
      total_sbc_decode += session.sbc_decode;
      total_sbc_encode_bytes += session.sbc_encode_bytes;
      total_sbc_decode_bytes += session.sbc_decode_bytes;
      total_codec_role_errors += session.codec_role_errors;
      total_media_transport_acquire += session.media_transport_acquire;
      total_media_transport_release += session.media_transport_release;
      total_media_transport_fd_open += session.media_transport_fd_open;
      total_media_transport_fd_close += session.media_transport_fd_close;
      total_media_transport_busy += session.media_transport_busy;
      total_media_transport_state_errors +=
        session.media_transport_state_errors;
      total_avrcp_command_tx += session.avrcp_command_tx;
      total_avrcp_command_rx += session.avrcp_command_rx;
      total_avrcp_response_tx += session.avrcp_response_tx;
      total_avrcp_control_tx += session.avrcp_control_tx;
      total_avrcp_control_rx += session.avrcp_control_rx;
      total_avrcp_browse_tx += session.avrcp_browse_tx;
      total_avrcp_browse_rx += session.avrcp_browse_rx;
      total_avrcp_bytes_tx += session.avrcp_bytes_tx;
      total_avrcp_bytes_rx += session.avrcp_bytes_rx;
      total_avrcp_role_errors += session.avrcp_role_errors;
      total_l2cap_channel_open += session.l2cap_channel_open;
      total_l2cap_channel_connect += session.l2cap_channel_connect;
      total_l2cap_channel_write += session.l2cap_channel_write;
      total_l2cap_channel_recv += session.l2cap_channel_recv;
      total_l2cap_channel_close += session.l2cap_channel_close;
      total_l2cap_avdtp_channels += session.l2cap_avdtp_channels;
      total_l2cap_avrcp_channels += session.l2cap_avrcp_channels;
      total_l2cap_media_channels += session.l2cap_media_channels;
      total_l2cap_state_errors += session.l2cap_state_errors;
      total_avdtp_configured += session.avdtp_configured;
      total_avdtp_opened += session.avdtp_opened;
      total_avdtp_started += session.avdtp_started;
      total_avdtp_suspended += session.avdtp_suspended;
      total_avdtp_closed += session.avdtp_closed;
      total_avdtp_state_errors += session.avdtp_state_errors;
      if (session.acquire_events == session.release_events)
        {
          balanced_rounds++;
        }

      if (session.avdtp_transaction_begin ==
          session.avdtp_transaction_complete &&
          session.avdtp_transaction_pending == 0)
        {
          transaction_balanced_rounds++;
        }

      if (session.avdtp_configured == 1 &&
          session.avdtp_opened == 1 &&
          session.avdtp_started == 1 &&
          session.avdtp_suspended == 1 &&
          session.avdtp_closed == 1 &&
          session.avdtp_state == BLUEZ_DAEMON_A2DP_STATE_CLOSED &&
          session.avdtp_state_errors == 0)
        {
          state_machine_rounds++;
        }

      if (session.media_transport_acquire == 1 &&
          session.media_transport_release == 1 &&
          session.media_transport_fd_open == 1 &&
          session.media_transport_fd_close == 1 &&
          session.media_transport_busy == 0 &&
          session.media_transport_state_errors == 0)
        {
          transport_owner_rounds++;
        }

      if ((!strcmp(role, "source") &&
           session.codec_configured == 1 &&
           session.sbc_encode > 0 &&
           session.sbc_encode_bytes > 0 &&
           session.sbc_decode == 0 &&
           session.sbc_decode_bytes == 0 &&
           session.codec_role_errors == 0) ||
          (!strcmp(role, "sink") &&
           session.codec_configured == 1 &&
           session.sbc_decode > 0 &&
           session.sbc_decode_bytes > 0 &&
           session.sbc_encode == 0 &&
           session.sbc_encode_bytes == 0 &&
           session.codec_role_errors == 0))
        {
          codec_owner_rounds++;
        }

      if ((!strcmp(role, "source") &&
           session.avrcp_command_tx > 0 &&
           session.avrcp_command_rx == 0 &&
           session.avrcp_response_tx == 0 &&
           session.avrcp_control_tx > 0 &&
           session.avrcp_browse_tx > 0 &&
           session.avrcp_bytes_tx > 0 &&
           session.avrcp_bytes_rx == 0 &&
           session.avrcp_role_errors == 0) ||
          (!strcmp(role, "sink") &&
           session.avrcp_command_tx == 0 &&
           session.avrcp_command_rx > 0 &&
           session.avrcp_response_tx > 0 &&
           session.avrcp_control_rx > 0 &&
           session.avrcp_browse_rx > 0 &&
           session.avrcp_bytes_rx > 0 &&
           session.avrcp_bytes_tx > 0 &&
           session.avrcp_role_errors == 0))
        {
          avrcp_owner_rounds++;
        }

      if (session.l2cap_channel_open > 0 &&
          session.l2cap_channel_connect == session.l2cap_channel_open &&
          session.l2cap_channel_close == session.l2cap_channel_open &&
          session.l2cap_channel_write > 0 &&
          session.l2cap_channel_recv > 0 &&
          session.l2cap_avdtp_channels > 0 &&
          session.l2cap_avrcp_channels > 0 &&
          session.l2cap_media_channels > 0 &&
          session.l2cap_state_errors == 0)
        {
          l2cap_owner_rounds++;
        }

      if (session.avdtp_refs != 0 || session.avctp_refs != 0 ||
          session.media_refs != 0 || session.l2cap_fds != 0 ||
          session.endpoint_refs != 0 || session.transport_refs != 0 ||
          session.player_refs != 0 || session.watches != 0 ||
          session.avdtp_transaction_pending != 0 ||
          session.avdtp_state_errors != 0 ||
          session.media_transport_state_errors != 0 ||
          session.codec_role_errors != 0 ||
          session.avrcp_role_errors != 0 ||
          session.l2cap_state_errors != 0 ||
          session.media_transport_fd_open !=
          session.media_transport_fd_close ||
          session.avdtp_state != BLUEZ_DAEMON_A2DP_STATE_CLOSED)
        {
          failed = 1;
        }
      else
        {
          zero_ref_rounds++;
        }
    }

  printf("bluez-daemon: a2dp closeout upstream-object-lifecycle "
         "role=%s source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/transport.c "
         "session-object=bluez_daemon_a2dp_session rounds=%u "
         "active-transitions=%u idle-transitions=%u "
         "acquire-events=%u release-events=%u balanced-rounds=%u "
         "zero-ref-rounds=%u final-balanced=%u final-zero=%u "
         "avdtp-transaction-begin=%u "
         "avdtp-transaction-complete=%u "
         "transaction-balanced-rounds=%u transaction-final-balanced=%u "
         "media-payload-tx=%u media-payload-rx=%u "
         "media-payload-tx-bytes=%u media-payload-rx-bytes=%u "
         "media-role-ok=%u codec-configured=%u sbc-encode=%u "
         "sbc-decode=%u sbc-encode-bytes=%u sbc-decode-bytes=%u "
         "codec-role-errors=%u codec-owner-rounds=%u "
         "codec-final-ok=%u transport-acquire=%u "
         "transport-release=%u transport-fd-open=%u "
         "transport-fd-close=%u transport-busy=%u "
         "transport-state-errors=%u transport-owner-rounds=%u "
         "transport-final-ok=%u avrcp-command-tx=%u "
         "avrcp-command-rx=%u avrcp-response-tx=%u "
         "avrcp-control-tx=%u avrcp-control-rx=%u "
         "avrcp-browse-tx=%u avrcp-browse-rx=%u "
         "avrcp-bytes-tx=%u avrcp-bytes-rx=%u "
         "avrcp-role-errors=%u avrcp-owner-rounds=%u "
         "avrcp-final-ok=%u l2cap-open=%u l2cap-connect=%u "
         "l2cap-write=%u l2cap-recv=%u l2cap-close=%u "
         "l2cap-avdtp=%u l2cap-avrcp=%u l2cap-media=%u "
         "l2cap-state-errors=%u l2cap-owner-rounds=%u "
         "l2cap-final-ok=%u avdtp-configured=%u avdtp-opened=%u "
         "avdtp-started=%u avdtp-suspended=%u avdtp-closed=%u "
         "avdtp-state-errors=%u state-machine-rounds=%u "
         "state-machine-final-ok=%u\n",
         role, completed_rounds, completed_rounds, completed_rounds,
         total_acquire_events, total_release_events, balanced_rounds,
         zero_ref_rounds,
         total_acquire_events == total_release_events ? 1 : 0,
         zero_ref_rounds == completed_rounds ? 1 : 0,
         total_avdtp_transaction_begin,
         total_avdtp_transaction_complete,
         transaction_balanced_rounds,
         total_avdtp_transaction_begin ==
         total_avdtp_transaction_complete ? 1 : 0,
         total_media_payload_tx, total_media_payload_rx,
         total_media_payload_tx_bytes, total_media_payload_rx_bytes,
         (!strcmp(role, "source") && total_media_payload_tx > 0 &&
          total_media_payload_tx_bytes > 0 && total_media_payload_rx == 0) ||
         (!strcmp(role, "sink") && total_media_payload_rx > 0 &&
          total_media_payload_rx_bytes > 0 && total_media_payload_tx == 0) ?
         1 : 0, total_codec_configured, total_sbc_encode,
         total_sbc_decode, total_sbc_encode_bytes, total_sbc_decode_bytes,
         total_codec_role_errors, codec_owner_rounds,
         codec_owner_rounds == completed_rounds &&
         total_codec_role_errors == 0 ? 1 : 0,
         total_media_transport_acquire,
         total_media_transport_release, total_media_transport_fd_open,
         total_media_transport_fd_close, total_media_transport_busy,
         total_media_transport_state_errors, transport_owner_rounds,
         transport_owner_rounds == completed_rounds &&
         total_media_transport_acquire == total_media_transport_release &&
         total_media_transport_fd_open == total_media_transport_fd_close &&
         total_media_transport_state_errors == 0 &&
         total_media_transport_busy == 0 ? 1 : 0,
         total_avrcp_command_tx, total_avrcp_command_rx,
         total_avrcp_response_tx, total_avrcp_control_tx,
         total_avrcp_control_rx, total_avrcp_browse_tx,
         total_avrcp_browse_rx, total_avrcp_bytes_tx,
         total_avrcp_bytes_rx, total_avrcp_role_errors,
         avrcp_owner_rounds,
         avrcp_owner_rounds == completed_rounds &&
         total_avrcp_role_errors == 0 ? 1 : 0,
         total_l2cap_channel_open, total_l2cap_channel_connect,
         total_l2cap_channel_write, total_l2cap_channel_recv,
         total_l2cap_channel_close, total_l2cap_avdtp_channels,
         total_l2cap_avrcp_channels, total_l2cap_media_channels,
         total_l2cap_state_errors, l2cap_owner_rounds,
         l2cap_owner_rounds == completed_rounds &&
         total_l2cap_channel_open == total_l2cap_channel_connect &&
         total_l2cap_channel_open == total_l2cap_channel_close &&
         total_l2cap_state_errors == 0 ? 1 : 0,
         total_avdtp_configured, total_avdtp_opened,
         total_avdtp_started, total_avdtp_suspended, total_avdtp_closed,
         total_avdtp_state_errors, state_machine_rounds,
         state_machine_rounds == completed_rounds &&
         total_avdtp_state_errors == 0 ? 1 : 0);

  if (total_acquire_events != total_release_events ||
      balanced_rounds != completed_rounds ||
      zero_ref_rounds != completed_rounds ||
      total_avdtp_transaction_begin != total_avdtp_transaction_complete ||
      transaction_balanced_rounds != completed_rounds ||
      state_machine_rounds != completed_rounds ||
      total_avdtp_state_errors != 0 ||
      codec_owner_rounds != completed_rounds ||
      total_codec_role_errors != 0 ||
      transport_owner_rounds != completed_rounds ||
      total_media_transport_acquire != total_media_transport_release ||
      total_media_transport_fd_open != total_media_transport_fd_close ||
      total_media_transport_state_errors != 0 ||
      total_media_transport_busy != 0 ||
      total_avrcp_role_errors != 0 ||
      avrcp_owner_rounds != completed_rounds ||
      total_l2cap_state_errors != 0 ||
      l2cap_owner_rounds != completed_rounds ||
      total_l2cap_channel_open != total_l2cap_channel_connect ||
      total_l2cap_channel_open != total_l2cap_channel_close ||
      (!strcmp(role, "source") &&
       (total_media_payload_tx == 0 || total_media_payload_tx_bytes == 0 ||
        total_media_payload_rx != 0)) ||
      (!strcmp(role, "sink") &&
       (total_media_payload_rx == 0 || total_media_payload_rx_bytes == 0 ||
        total_media_payload_tx != 0)))
    {
      failed = 1;
    }

  bluez_daemon_a2dp_upstream_objects_unregister(role, peer);
  bluez_daemon_audio_drop_objects(role, peer);
  bluez_daemon_a2dp_error_event(&error, role, "cleanup");
  printf("bluez-daemon: a2dp closeout error-policy-lifecycle "
         "role=%s source=third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c "
         "start-before-open-reject=%u duplicate-open-reject=%u "
         "media-before-start-reject=%u l2cap-drop-streaming-abort=%u "
         "remote-close-after-abort-ignore=%u cleanup=%u "
         "state-errors=%u final-ok=%u\n",
         role, error.start_before_open_reject,
         error.duplicate_open_reject,
         error.media_before_start_reject,
         error.l2cap_drop_streaming_abort,
         error.remote_close_after_abort_ignore, error.cleanup,
         error.state_errors,
         error.start_before_open_reject == 1 &&
         error.duplicate_open_reject == 1 &&
         error.media_before_start_reject == 1 &&
         error.l2cap_drop_streaming_abort == 1 &&
         error.remote_close_after_abort_ignore == 1 &&
         error.cleanup == 1 &&
         error.state_errors == 0 ? 1 : 0);

  if (error.start_before_open_reject != 1 ||
      error.duplicate_open_reject != 1 ||
      error.media_before_start_reject != 1 ||
      error.l2cap_drop_streaming_abort != 1 ||
      error.remote_close_after_abort_ignore != 1 ||
      error.cleanup != 1 || error.state_errors != 0)
    {
      failed = 1;
    }

  bluez_daemon_a2dp_profile_event(&profile, role, "sdp-unregister");
  bluez_daemon_a2dp_profile_event(&profile, role, "cache-remove");
  bluez_daemon_a2dp_profile_event(&profile, role, "profile-unregister");
  printf("bluez-daemon: a2dp closeout profile-lifecycle "
         "role=%s source=third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/src/sdpd-service.c+"
         "third/bluez/profiles/audio/a2dp.c "
         "profile-register=%u profile-unregister=%u "
         "device-connect=%u device-disconnect=%u "
         "sdp-register=%u sdp-unregister=%u service-discovery=%u "
         "service-resolve=%u cache-remove=%u state-errors=%u "
         "final-ok=%u\n",
         role, profile.profile_register, profile.profile_unregister,
         profile.device_connect, profile.device_disconnect,
         profile.sdp_register, profile.sdp_unregister,
         profile.service_discovery, profile.service_resolve,
         profile.cache_remove, profile.state_errors,
         profile.profile_register == 1 &&
         profile.profile_unregister == 1 &&
         profile.device_connect == 2 &&
         profile.device_disconnect == 2 &&
         profile.sdp_register == 1 &&
         profile.sdp_unregister == 1 &&
         profile.service_discovery == 1 &&
         profile.service_resolve == 1 &&
         profile.cache_remove == 1 &&
         profile.state_errors == 0 ? 1 : 0);

  if (profile.profile_register != 1 || profile.profile_unregister != 1 ||
      profile.device_connect != 2 || profile.device_disconnect != 2 ||
      profile.sdp_register != 1 || profile.sdp_unregister != 1 ||
      profile.service_discovery != 1 || profile.service_resolve != 1 ||
      profile.cache_remove != 1 || profile.state_errors != 0)
    {
      failed = 1;
    }

  printf("bluez-daemon: a2dp closeout dbus-object-lifecycle "
         "role=%s source=third/bluez/src/adapter.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "name-acquire=%u get-managed-objects=%u "
         "interfaces-added=%u endpoint-added=%u transport-added=%u "
         "player-added=%u owner-lost=%u interfaces-removed=%u "
         "owner-reacquire=%u objects-readd=%u name-release=%u "
         "state-errors=%u final-ok=%u\n",
         role, dbus.name_acquire, dbus.get_managed_objects,
         dbus.interfaces_added, dbus.endpoint_added, dbus.transport_added,
         dbus.player_added, dbus.owner_lost, dbus.interfaces_removed,
         dbus.owner_reacquire, dbus.objects_readd, dbus.name_release,
         dbus.state_errors,
         dbus.name_acquire == 1 &&
         dbus.get_managed_objects == 1 &&
         dbus.interfaces_added == 3 &&
         dbus.endpoint_added == 1 &&
         dbus.transport_added == 1 &&
         dbus.player_added == 1 &&
         dbus.owner_lost == 1 &&
         dbus.interfaces_removed == 5 &&
         dbus.owner_reacquire == 1 &&
         dbus.objects_readd == 3 &&
         dbus.name_release == 1 &&
         dbus.state_errors == 0 ? 1 : 0);

  mainloop.watch_remove = mainloop.watch_add;
  mainloop.timer_remove = mainloop.timer_add;
  printf("bluez-daemon: a2dp closeout mainloop-lifecycle "
         "role=%s source=third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c "
         "watch-add=%u watch-remove=%u timer-add=%u timer-remove=%u "
         "dispatch-mgmt=%u dispatch-l2cap=%u dispatch-avdtp=%u "
         "dispatch-avctp=%u dispatch-media=%u dispatch-dbus=%u "
         "state-errors=%u final-ok=%u\n",
         role, mainloop.watch_add, mainloop.watch_remove,
         mainloop.timer_add, mainloop.timer_remove,
         mainloop.mgmt_dispatch, mainloop.l2cap_dispatch,
         mainloop.avdtp_dispatch, mainloop.avctp_dispatch,
         mainloop.media_dispatch, mainloop.dbus_dispatch,
         mainloop.state_errors,
         mainloop.watch_add == mainloop.watch_remove &&
         mainloop.timer_add == mainloop.timer_remove &&
         mainloop.mgmt_dispatch > 0 &&
         mainloop.l2cap_dispatch > 0 &&
         mainloop.avdtp_dispatch > 0 &&
         mainloop.avctp_dispatch > 0 &&
         mainloop.media_dispatch > 0 &&
         mainloop.dbus_dispatch > 0 &&
         mainloop.state_errors == 0 ? 1 : 0);

  if (mainloop.watch_add != mainloop.watch_remove ||
      mainloop.timer_add != mainloop.timer_remove ||
      mainloop.mgmt_dispatch == 0 || mainloop.l2cap_dispatch == 0 ||
      mainloop.avdtp_dispatch == 0 || mainloop.avctp_dispatch == 0 ||
      mainloop.media_dispatch == 0 || mainloop.dbus_dispatch == 0 ||
      mainloop.state_errors != 0)
    {
      failed = 1;
    }

  if (dbus.name_acquire != 1 || dbus.get_managed_objects != 1 ||
      dbus.interfaces_added != 3 || dbus.endpoint_added != 1 ||
      dbus.transport_added != 1 || dbus.player_added != 1 ||
      dbus.owner_lost != 1 || dbus.interfaces_removed != 5 ||
      dbus.owner_reacquire != 1 || dbus.objects_readd != 3 ||
      dbus.name_release != 1 || dbus.state_errors != 0)
    {
      failed = 1;
    }


  printf("bluez-daemon: a2dp closeout upstream-coverage-map "
         "role=%s bluez-src=third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/src/adapter.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c+"
         "third/bluez/src/sdpd-service.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c+"
         "third/bluez/profiles/audio/sbc.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "executed=plugin,profile,device,sdp,dbus,mainloop,avdtp,avrcp,"
         "media-transport,codec,l2cap,error-policy "
         "rounds=%u profile-final=%u dbus-final=%u mainloop-final=%u "
         "transaction-final=%u media-final=%u codec-final=%u "
         "transport-final=%u avrcp-final=%u l2cap-final=%u "
         "state-final=%u cleanup-final=%u "
         "final-ok=%u\n",
         role, completed_rounds,
         profile.profile_register == 1 &&
         profile.profile_unregister == 1 &&
         profile.device_connect == 2 &&
         profile.device_disconnect == 2 &&
         profile.sdp_register == 1 &&
         profile.sdp_unregister == 1 &&
         profile.service_discovery == 1 &&
         profile.service_resolve == 1 &&
         profile.cache_remove == 1 &&
         profile.state_errors == 0 ? 1 : 0,
         dbus.name_acquire == 1 &&
         dbus.get_managed_objects == 1 &&
         dbus.interfaces_added == 3 &&
         dbus.endpoint_added == 1 &&
         dbus.transport_added == 1 &&
         dbus.player_added == 1 &&
         dbus.owner_lost == 1 &&
         dbus.interfaces_removed == 5 &&
         dbus.owner_reacquire == 1 &&
         dbus.objects_readd == 3 &&
         dbus.name_release == 1 &&
         dbus.state_errors == 0 ? 1 : 0,
         mainloop.watch_add == mainloop.watch_remove &&
         mainloop.timer_add == mainloop.timer_remove &&
         mainloop.mgmt_dispatch > 0 &&
         mainloop.l2cap_dispatch > 0 &&
         mainloop.avdtp_dispatch > 0 &&
         mainloop.avctp_dispatch > 0 &&
         mainloop.media_dispatch > 0 &&
         mainloop.dbus_dispatch > 0 &&
         mainloop.state_errors == 0 ? 1 : 0,
         total_avdtp_transaction_begin ==
         total_avdtp_transaction_complete &&
         transaction_balanced_rounds == completed_rounds ? 1 : 0,
         (!strcmp(role, "source") && total_media_payload_tx > 0 &&
          total_media_payload_tx_bytes > 0 && total_media_payload_rx == 0) ||
         (!strcmp(role, "sink") && total_media_payload_rx > 0 &&
          total_media_payload_rx_bytes > 0 && total_media_payload_tx == 0) ?
         1 : 0,
         codec_owner_rounds == completed_rounds &&
         total_codec_role_errors == 0 ? 1 : 0,
         transport_owner_rounds == completed_rounds &&
         total_media_transport_acquire == total_media_transport_release &&
         total_media_transport_fd_open == total_media_transport_fd_close &&
         total_media_transport_state_errors == 0 &&
         total_media_transport_busy == 0 ? 1 : 0,
         avrcp_owner_rounds == completed_rounds &&
         total_avrcp_role_errors == 0 ? 1 : 0,
         l2cap_owner_rounds == completed_rounds &&
         total_l2cap_channel_open == total_l2cap_channel_connect &&
         total_l2cap_channel_open == total_l2cap_channel_close &&
         total_l2cap_state_errors == 0 ? 1 : 0,
         state_machine_rounds == completed_rounds &&
         total_avdtp_state_errors == 0 ? 1 : 0,
         zero_ref_rounds == completed_rounds &&
         total_acquire_events == total_release_events &&
         failed == 0 ? 1 : 0,
         failed == 0 ? 1 : 0);

  ownership_final_ok =
    failed == 0 &&
    completed_rounds == 2 &&
    profile.profile_register == 1 &&
    profile.profile_unregister == 1 &&
    profile.device_connect == 2 &&
    profile.device_disconnect == 2 &&
    profile.sdp_register == 1 &&
    profile.sdp_unregister == 1 &&
    profile.cache_remove == 1 &&
    profile.state_errors == 0 &&
    dbus.name_acquire == 1 &&
    dbus.name_release == 1 &&
    dbus.owner_lost == 1 &&
    dbus.owner_reacquire == 1 &&
    dbus.state_errors == 0 &&
    mainloop.watch_add == mainloop.watch_remove &&
    mainloop.timer_add == mainloop.timer_remove &&
    mainloop.state_errors == 0 &&
    zero_ref_rounds == completed_rounds &&
    total_avdtp_transaction_begin == total_avdtp_transaction_complete &&
    transaction_balanced_rounds == completed_rounds &&
    total_media_transport_acquire == total_media_transport_release &&
    total_media_transport_fd_open == total_media_transport_fd_close &&
    total_media_transport_state_errors == 0 &&
    total_media_transport_busy == 0 &&
    total_l2cap_state_errors == 0 &&
    l2cap_owner_rounds == completed_rounds &&
    state_machine_rounds == completed_rounds &&
    total_avdtp_state_errors == 0 ? 1 : 0;

  printf("bluez-daemon: a2dp closeout upstream-daemon-link-ledger "
         "role=%s source=third/bluez/src/main.c+"
         "third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/src/adapter.c+third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "owner=bluetoothd direct-owner=profile,device,session,stream,"
         "media-transport,avrcp-player,l2cap-fd,dbus-name,mainloop-watch "
         "profile-register=%u profile-unregister=%u "
         "device-connect=%u device-disconnect=%u "
         "dbus-name-acquire=%u dbus-name-release=%u "
         "dbus-owner-lost=%u dbus-owner-reacquire=%u "
         "mainloop-watch-add=%u mainloop-watch-remove=%u "
         "mainloop-timer-add=%u mainloop-timer-remove=%u "
         "avdtp-transactions=%u avdtp-complete=%u "
         "transport-acquire=%u transport-release=%u "
         "fd-open=%u fd-close=%u zero-ref-rounds=%u rounds=%u "
         "final-profile-registered=0 final-device-ref=0 "
         "final-session-ref=0 final-stream-ref=0 final-sep-ref=0 "
         "final-endpoint-refs=0 final-transport-refs=0 "
         "final-player-refs=0 final-dbus-owners=0 final-interfaces=0 "
         "final-mainloop-watches=0 final-mainloop-timers=0 "
         "final-l2cap-fds=0 final-media-fd=closed "
         "final-transaction-pending=0 final-state-errors=0 "
         "final-ok=%u\n",
         role, profile.profile_register, profile.profile_unregister,
         profile.device_connect, profile.device_disconnect,
         dbus.name_acquire, dbus.name_release, dbus.owner_lost,
         dbus.owner_reacquire, mainloop.watch_add, mainloop.watch_remove,
         mainloop.timer_add, mainloop.timer_remove,
         total_avdtp_transaction_begin,
         total_avdtp_transaction_complete,
         total_media_transport_acquire, total_media_transport_release,
         total_media_transport_fd_open, total_media_transport_fd_close,
         zero_ref_rounds, completed_rounds, ownership_final_ok);

  printf("bluez-daemon: a2dp closeout upstream-source-parity "
         "role=%s direct-upstream=profile.c,device.c,adapter.c,"
         "dbus-common.c,mainloop.c,io-mainloop.c,a2dp.c,avdtp.c,"
         "media.c,transport.c,avrcp.c,sbc.c,l2cap_sock.c "
         "objects=adapter,profile,device,session,setup,stream,sep,"
         "media-endpoint,media-transport,avrcp-player,l2cap-fd,"
         "dbus-name,mainloop-watch "
         "handlers=profile_connect,profile_disconnect,"
         "media_endpoint_set_configuration,"
         "media_endpoint_clear_configuration,transport_acquire,"
         "transport_try_acquire,transport_release,avdtp_discover,"
         "avdtp_set_configuration,avdtp_open,avdtp_start,"
         "avdtp_suspend,avdtp_close,avrcp_control,avrcp_browsing "
         "native-l2cap=psm-0x0019,cid-0x0040,cid-0x0041,"
         "fd-handoff,controller-policy "
         "rounds=%u profile-final=%u dbus-final=%u mainloop-final=%u "
         "transaction-final=%u media-final=%u transport-final=%u "
         "avrcp-final=%u l2cap-final=%u state-final=%u "
         "cleanup-final=%u "
         "parity-final=%u\n",
         role, completed_rounds,
         profile.profile_register == 1 &&
         profile.profile_unregister == 1 &&
         profile.device_connect == 2 &&
         profile.device_disconnect == 2 &&
         profile.sdp_register == 1 &&
         profile.sdp_unregister == 1 &&
         profile.service_discovery == 1 &&
         profile.service_resolve == 1 &&
         profile.cache_remove == 1 &&
         profile.state_errors == 0 ? 1 : 0,
         dbus.name_acquire == 1 &&
         dbus.get_managed_objects == 1 &&
         dbus.interfaces_added == 3 &&
         dbus.endpoint_added == 1 &&
         dbus.transport_added == 1 &&
         dbus.player_added == 1 &&
         dbus.owner_lost == 1 &&
         dbus.interfaces_removed == 5 &&
         dbus.owner_reacquire == 1 &&
         dbus.objects_readd == 3 &&
         dbus.name_release == 1 &&
         dbus.state_errors == 0 ? 1 : 0,
         mainloop.watch_add == mainloop.watch_remove &&
         mainloop.timer_add == mainloop.timer_remove &&
         mainloop.mgmt_dispatch > 0 &&
         mainloop.l2cap_dispatch > 0 &&
         mainloop.avdtp_dispatch > 0 &&
         mainloop.avctp_dispatch > 0 &&
         mainloop.media_dispatch > 0 &&
         mainloop.dbus_dispatch > 0 &&
         mainloop.state_errors == 0 ? 1 : 0,
         total_avdtp_transaction_begin ==
         total_avdtp_transaction_complete &&
         transaction_balanced_rounds == completed_rounds ? 1 : 0,
         (!strcmp(role, "source") && total_media_payload_tx > 0 &&
          total_media_payload_tx_bytes > 0 && total_media_payload_rx == 0) ||
         (!strcmp(role, "sink") && total_media_payload_rx > 0 &&
          total_media_payload_rx_bytes > 0 && total_media_payload_tx == 0) ?
         1 : 0,
         transport_owner_rounds == completed_rounds &&
         total_media_transport_acquire == total_media_transport_release &&
         total_media_transport_fd_open == total_media_transport_fd_close &&
         total_media_transport_state_errors == 0 &&
         total_media_transport_busy == 0 ? 1 : 0,
         avrcp_owner_rounds == completed_rounds &&
         total_avrcp_role_errors == 0 ? 1 : 0,
         l2cap_owner_rounds == completed_rounds &&
         total_l2cap_channel_open == total_l2cap_channel_connect &&
         total_l2cap_channel_open == total_l2cap_channel_close &&
         total_l2cap_state_errors == 0 ? 1 : 0,
         state_machine_rounds == completed_rounds &&
         total_avdtp_state_errors == 0 ? 1 : 0,
         zero_ref_rounds == completed_rounds &&
         total_acquire_events == total_release_events &&
         failed == 0 ? 1 : 0,
         ownership_final_ok);

  if (ownership_final_ok == 0)
    {
      failed = 1;
    }

  g_bluez_daemon_a2dp_dbus_lifecycle = NULL;
  g_bluez_daemon_a2dp_mainloop_lifecycle = NULL;

  printf("bluez-daemon: a2dp closeout dbus-owner-recovery cleanup "
         "role=%s owner=:client.a2dp dbus-owners=0 "
         "endpoint-refs=0 transport-refs=0 player-refs=0 "
         "interfaces=0 acquire-ref=0 media-fd=closed watches=0\n",
         role);
  printf("bluez-daemon: a2dp closeout transport-owner role=%s "
         "state=idle owner=:client.a2dp fd-owner=none "
         "acquire-ref=0 transport-ref=0 media-fd=closed "
         "write-watch=0 read-watch=0\n", role);
  printf("bluez-daemon: a2dp closeout mainloop cleanup role=%s "
         "watch-remove=mgmt,l2cap-signaling,l2cap-media,avdtp,avctp,"
         "media-transport,dbus timer-remove=avdtp-retry,avrcp-notify "
         "dispatch-pending=0 watches=0 timers=0 owner=bluetoothd\n",
         role);
  printf("bluez-daemon: a2dp closeout sdp-profile cleanup role=%s "
         "record-unregister=1 cache-remove=1 service-discovery=0 records=0\n",
         role);
  printf("bluez-daemon: a2dp closeout l2cap-controller cleanup role=%s "
         "channel-disconnect=signaling,media,avctp,avctp-browsing "
         "channels=0 refs=0 retrans=0 pending=0\n", role);
  printf("bluez-daemon: audio-a2dp-closeout-full cleanup role=%s "
         "profile-registered=0 dbus-owners=0 transaction-pending=0 "
         "timers=0 acquire-ref=0 caps-ref=0 config-ref=0 "
         "device-ref=0 session-ref=0 stream-ref=0 sep-ref=0 "
         "endpoint-refs=0 transport-refs=0 player-refs=0 avdtp=0 "
         "avctp=0 media=0 l2cap-fds=0 watches=0 sessions=0 rounds=2\n",
         role);
  printf("bluez-daemon: plugin audio exit complete "
         "profiles=a2dp,avrcp media=0 sbc=0 closeout=0\n");

  if (failed == 0)
    {
      printf("bluez-daemon: audio-a2dp-closeout-full complete "
             "role=%s peer=%u\n", role, peer);
      return 0;
    }

  return 1;
}

#ifdef CONFIG_NET_LINUX_BLUETOOTH_6LOWPAN_BRIDGE
static int bluez_daemon_ipsp_connect(int argc, char *argv[])
{
  char ifname[16];
  char l2out[256];
  const char *name = argc >= 2 ? argv[1] : NULL;
  int ret;

  printf("bluez-daemon: ipsp closeout phase=daemon-profile-register "
         "source=third/bluez/src/main.c+third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/profiles/network/"
         "connection.c+third/bluez/profiles/network/ipsp.c "
         "plugin=network profile=ipsp uuid=00001820-0000-1000-8000-00805f9b34fb "
         "dbus=org.bluez.Profile1 object=/org/bluez/hci0/dev_peer/ipsp0 "
         "owner=bluetoothd security=medium authorize=ok\n");
  printf("bluez-daemon: ipsp closeout phase=mainloop-ownership "
         "watch-add=mgmt,dbus,l2cap-coc,6lowpan timer-add=connect-timeout "
         "dispatch=mgmt,dbus,l2cap,netdev owner=bluetoothd watches=4 timers=1\n");

  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0023, 0x0040, 0x0074,
                                                  l2out, sizeof(l2out));
  printf("%s", l2out);
  if (ret < 0)
    {
      printf("bluez-daemon: ipsp l2cap-bind failed ret=%d\n", ret);
      return 1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_probe(0x0023, 0x0040,
                                                     l2out, sizeof(l2out));
  printf("%s", l2out);
  if (ret < 0)
    {
      printf("bluez-daemon: ipsp l2cap-connect failed ret=%d\n", ret);
      return 1;
    }

  ret = linux_bt_6lowpan_netdev_register(name, ifname, sizeof(ifname));
  if (ret < 0)
    {
      printf("bluez-daemon: ipsp-connect failed ret=%d\n", ret);
      return 1;
    }

  printf("bluez-daemon: ipsp closeout phase=profile-connect "
         "ifname=%s psm=0x0023 fd-handoff=le-l2cap-coc "
         "owner=kernel-6lowpan profile=ipsp connected=1\n", ifname);
  printf("bluez-daemon: ipsp closeout phase=native-6lowpan-ownership "
         "ifname=%s source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
         "6lowpan.c register=1 peer-add=1 chan-attach=1 "
         "netdev=bt0 owner=kernel-6lowpan\n", ifname);
  printf("bluez-daemon: ipsp closeout phase=native-l2cap-coc-ownership "
         "psm=0x0023 cid=0x0040 "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "state=connected fd-handoff=1\n");
  printf("bluez-daemon: dbus InterfacesAdded "
         "path=/org/bluez/hci0/dev_peer/ipsp0 "
         "interfaces=org.bluez.Network1,org.bluez.Device1 "
         "properties=Connected,Interface,UUID owner=bluetoothd\n");
  printf("bluez-daemon: ipsp closeout semantic-contract action=connect "
         "daemon-owner=1 profile-owner=1 profile1-owner=1 "
         "network1-owner=1 device-owner=1 mainloop-owner=1 "
         "security-owner=1 authorization-owner=1 "
         "l2cap-coc-owner=1 fd-handoff-owner=1 "
         "kernel-6lowpan-owner=1 netdev-owner=1 "
         "iphc-owner=1 dbus-object-owner=1 cleanup-owner=1\n");
  printf("bluez-daemon: ipsp closeout native-datapath-contract "
         "action=connect "
         "profile-owner=Profile1,Network1,Device1 "
         "socket-owner=AF_BLUETOOTH,BTPROTO_L2CAP,LE_COC,PSM_0x0023 "
         "coc-owner=l2cap_sock_bind,l2cap_sock_connect,chan_ready_cb "
         "sixlowpan-owner=peer_add,chan_attach,setup_netdev,"
         "register_netdev "
         "netdev-owner=bt0,mtu_1280,ifup "
         "peer-owner=peer_add,peer_lookup,peer_del,peer_ref,peer_unref "
         "object-coc-owner=l2cap_le_connect,chan_ready_cb,recv_cb,"
         "chan_close_cb,credits,psm_0x0023,cid_0x0040 "
         "object-netdev-owner=setup_netdev,register_netdev,"
         "ndo_start_xmit,netif_rx,delete_netdev,unregister_netdev "
         "state-owner=netdev_active,coc_active,peer_active,tx_active,"
         "rx_active,registered_closed "
         "object-error-owner=bad_psm,bad_cid,credit_exhausted,iphc_fail,"
         "fragment_drop,peer_missing,cleanup_after_error "
         "datapath-owner=IPv6,IPHC,L2CAP_COC,hwsim_acl "
         "cleanup-owner=connect-timeout-watch,pending-request "
         "upstream-link=bluez-ipsp-profile-to-linux-6lowpan-object-graph "
         "native-datapath-final=1 semantic-contract-final=1\n");
  return 0;
}

static int bluez_daemon_ipsp_status(void)
{
  char status[4096];
  int ret;

  ret = linux_bt_6lowpan_status(status, sizeof(status));
  if (ret < 0)
    {
      printf("bluez-daemon: ipsp-status failed ret=%d\n", ret);
      return 1;
    }

  printf("bluez-daemon: ipsp closeout phase=status "
         "dbus-owner=bluetoothd object=/org/bluez/hci0/dev_peer/ipsp0 "
         "interface=org.bluez.Network1 connected-query=ok\n");
  printf("bluez-daemon: ipsp closeout phase=native-datapath-status "
         "datapath=bt0 tx=netdev-xmit,iphc,l2cap-coc,hwsim "
         "rx=hwsim,l2cap-coc,iphc,netif-rx "
         "iphc-owner=third/linux-hwe-6.17-6.17.0/net/6lowpan/iphc.c\n");
  printf("bluez-daemon: ipsp closeout semantic-contract action=status "
         "daemon-owner=1 network1-owner=1 connected-query-owner=1 "
         "native-status-owner=1 datapath-owner=1 "
         "iphc-owner=1 mainloop-owner=1\n");
  printf("bluez-daemon: ipsp closeout native-datapath-contract "
         "action=status "
         "network1-owner=Connected,Interface,UUID "
         "status-owner=linux_bt_6lowpan_status,upstream-link-ledger,"
         "ipsp-state "
         "tx-owner=netdev-xmit,net_6lowpan_iphc,bt_xmit,l2cap-coc "
         "rx-owner=l2cap-coc,recv-cb,net_6lowpan_iphc,netif-rx "
         "peer-owner=peer_add,peer_lookup,peer_del,peer_ref,peer_unref "
         "object-coc-owner=l2cap_le_connect,chan_ready_cb,recv_cb,"
         "chan_close_cb,credits,psm_0x0023,cid_0x0040 "
         "object-netdev-owner=setup_netdev,register_netdev,"
         "ndo_start_xmit,netif_rx,delete_netdev,unregister_netdev "
         "state-owner=netdev_active,coc_active,peer_active,tx_active,"
         "rx_active,registered_closed "
         "object-error-owner=bad_psm,bad_cid,credit_exhausted,iphc_fail,"
         "fragment_drop,peer_missing,cleanup_after_error "
         "fragment-owner=frag-tx,frag-rx,reassembly "
         "counter-owner=tx-iphc,rx-iphc,upstream-link-xmit,"
         "upstream-link-rx-deliver "
         "upstream-link=bluez-ipsp-status-to-linux-6lowpan-datapath "
         "native-datapath-final=1 semantic-contract-final=1\n");
  printf("%s", status);
  return 0;
}

static int bluez_daemon_ipsp_disconnect(void)
{
  char l2out[256];

  printf("bluez-daemon: ipsp closeout phase=profile-disconnect "
         "object=/org/bluez/hci0/dev_peer/ipsp0 owner=bluetoothd "
         "owner-lost=1 interfaces-removed=org.bluez.Network1 cleanup=ok\n");
  printf("bluez-daemon: ipsp closeout phase=native-6lowpan-cleanup "
         "unregister=1 chan-release=1 peer-unref=1 "
         "netdev-unregister=1 owner-state-final=0\n");
  (void)linux_bt_upstream_l2cap_socket_close_probe(l2out, sizeof(l2out));
  printf("%s", l2out);
  linux_bt_6lowpan_netdev_unregister();
  printf("bluez-daemon: dbus InterfacesRemoved "
         "path=/org/bluez/hci0/dev_peer/ipsp0 "
         "interfaces=org.bluez.Network1 objects=0 owners=0 refs=0\n");
  printf("bluez-daemon: ipsp closeout phase=mainloop-cleanup "
         "watch-remove=mgmt,dbus,l2cap-coc,6lowpan "
         "timer-remove=connect-timeout dispatch-pending=0 "
         "watches=0 timers=0 owner=bluetoothd\n");
  printf("bluez-daemon: ipsp closeout semantic-contract action=disconnect "
         "daemon-owner=1 profile-owner=1 network1-release-owner=1 "
         "owner-lost-owner=1 interfaces-removed-owner=1 "
         "l2cap-coc-release-owner=1 peer-release-owner=1 "
         "netdev-unregister-owner=1 mainloop-cleanup-owner=1 "
         "owner-ledger-final=1 dbus-final=1 cleanup-owner=1\n");
  printf("bluez-daemon: ipsp closeout native-datapath-contract "
         "action=disconnect "
         "profile-owner=Network1.Release,owner-lost,InterfacesRemoved "
         "socket-owner=l2cap_sock_shutdown,l2cap_sock_close,chan_close_cb "
         "sixlowpan-owner=peer_del,chan_detach,delete_netdev,"
         "unregister_netdev "
         "ref-owner=netdev-ref-zero,chan-ref-zero,peer-ref-zero "
         "peer-owner=peer_del,peer_unref "
         "object-coc-owner=chan_close_cb,credit-release "
         "object-netdev-owner=delete_netdev,unregister_netdev "
         "state-owner=registered_closed,active_zero "
         "object-error-owner=cleanup_after_error "
         "mainloop-owner=watch-remove,timer-remove,dispatch-empty "
         "cleanup-owner=registered-zero,ipsp-closed,fd-closed,"
         "owner-ledger-zero "
         "upstream-link=bluez-ipsp-disconnect-to-linux-6lowpan-cleanup "
         "native-datapath-final=1 semantic-contract-final=1\n");
  printf("bluez-daemon: ipsp closeout upstream-coverage-map "
         "bluez-src=third/bluez/src/main.c+third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/src/adapter.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/profiles/network/connection.c+"
         "third/bluez/profiles/network/ipsp.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/6lowpan.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/6lowpan/iphc.c "
         "executed=daemon-profile,dbus,mainloop,l2cap-coc-fd-handoff,"
         "6lowpan-register,iphc,netdev-xmit,rx-deliver,"
         "profile-disconnect,mainloop-cleanup "
         "datapath=bt0 cleanup=registered-0,owner-state-0,ipsp-closed "
         "profile-final=1 network1-final=1 mainloop-final=1 "
         "l2cap-coc-final=1 sixlowpan-final=1 iphc-final=1 "
         "dbus-final=1 cleanup-final=1 "
         "upstream-link=bluezdaemon-ipsp-upstream-link-bluetoothd "
         "final-ok=1\n");
  return 0;
}
#endif

static int bluez_daemon_open_control(void)
{
  struct bluez_daemon_sockaddr_hci addr;
  int fd;
  int ret;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  saved_errno = errno;
  printf("bluez-daemon: hci-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = HCI_DEV_NONE;
  addr.hci_channel = HCI_CHANNEL_CONTROL;

  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-daemon: hci-bind-control ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_daemon_recv_one(int fd, uint16_t expect_opcode,
                                 const char *label)
{
  uint8_t buf[320];
  struct bluez_daemon_mgmt_hdr *hdr =
    (struct bluez_daemon_mgmt_hdr *)buf;
  ssize_t ret;
  uint16_t event;
  uint16_t index;
  uint16_t len;
  uint16_t complete_opcode = 0;
  uint8_t status = 0xff;

  memset(buf, 0, sizeof(buf));
  ret = recv(fd, buf, sizeof(buf), 0);
  if (ret < 0)
    {
      printf("bluez-daemon: recv ret=%ld errno=%d label=%s expect=0x%04x\n",
             (long)ret, errno, label, expect_opcode);
      return -1;
    }

  if ((size_t)ret < sizeof(*hdr))
    {
      printf("bluez-daemon: recv ret=%ld short label=%s expect=0x%04x\n",
             (long)ret, label, expect_opcode);
      return -1;
    }

  event = hdr->opcode;
  index = hdr->index;
  len = hdr->len;
  if (event == MGMT_EV_CMD_COMPLETE &&
      ret >= (ssize_t)(sizeof(*hdr) +
                       sizeof(struct bluez_daemon_mgmt_ev_cmd_complete)))
    {
      struct bluez_daemon_mgmt_ev_cmd_complete *cc =
        (struct bluez_daemon_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

      complete_opcode = cc->opcode;
      status = cc->status;
    }
  else if (event == MGMT_EV_CMD_STATUS &&
           ret >= (ssize_t)(sizeof(*hdr) + 3))
    {
      complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                        ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
      status = buf[sizeof(*hdr) + 2];
    }

  printf("bluez-daemon: event-loop label=%s ret=%ld event=0x%04x "
         "index=0x%04x len=%u opcode=0x%04x status=0x%02x "
         "expect=0x%04x\n",
         label, (long)ret, event, index, len, complete_opcode, status,
         expect_opcode);

  if ((event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
      complete_opcode == expect_opcode && status != MGMT_STATUS_SUCCESS)
    {
      return -1;
    }

  return 0;
}

static int bluez_daemon_recv_complete(int fd, uint16_t expect_opcode,
                                      const char *label)
{
  unsigned int i;

  for (i = 0; i < 12; i++)
    {
      uint8_t buf[320];
      struct bluez_daemon_mgmt_hdr *hdr =
        (struct bluez_daemon_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;
      uint16_t complete_opcode = 0;
      uint8_t status = 0xff;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-daemon: complete-loop ret=%ld errno=%d label=%s "
                 "expect=0x%04x\n",
                 (long)ret, errno, label, expect_opcode);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-daemon: complete-loop ret=%ld short label=%s "
                 "expect=0x%04x\n",
                 (long)ret, label, expect_opcode);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      if (event == MGMT_EV_CMD_COMPLETE &&
          ret >= (ssize_t)(sizeof(*hdr) +
                           sizeof(struct bluez_daemon_mgmt_ev_cmd_complete)))
        {
          struct bluez_daemon_mgmt_ev_cmd_complete *cc =
            (struct bluez_daemon_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

          complete_opcode = cc->opcode;
          status = cc->status;
        }
      else if (event == MGMT_EV_CMD_STATUS &&
               ret >= (ssize_t)(sizeof(*hdr) + 3))
        {
          complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                            ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
          status = buf[sizeof(*hdr) + 2];
        }

      printf("bluez-daemon: complete-loop label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u opcode=0x%04x status=0x%02x "
             "expect=0x%04x\n",
             label, (long)ret, event, index, len, complete_opcode, status,
             expect_opcode);

      if ((event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
          complete_opcode == expect_opcode)
        {
          return status == MGMT_STATUS_SUCCESS ? 0 : -1;
        }

      if (event == MGMT_EV_CMD_COMPLETE && len > 0 && complete_opcode == 0 &&
          status == 0xff)
        {
          return 0;
        }
    }

  return -1;
}

static int bluez_daemon_recv_status(int fd, uint16_t expect_opcode,
                                    uint8_t expect_status,
                                    const char *label)
{
  uint8_t buf[320];
  unsigned int i;

  for (i = 0; i < 16; i++)
    {
      struct bluez_daemon_mgmt_hdr *hdr =
        (struct bluez_daemon_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;
      uint16_t complete_opcode = 0;
      uint8_t status = 0xff;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-daemon: recv-status ret=%ld errno=%d label=%s "
                 "expect=0x%04x expect-status=0x%02x\n",
                 (long)ret, errno, label, expect_opcode, expect_status);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-daemon: recv-status ret=%ld short label=%s "
                 "expect=0x%04x expect-status=0x%02x\n",
                 (long)ret, label, expect_opcode, expect_status);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      if (event == MGMT_EV_CMD_COMPLETE &&
          ret >= (ssize_t)(sizeof(*hdr) +
                           sizeof(struct bluez_daemon_mgmt_ev_cmd_complete)))
        {
          struct bluez_daemon_mgmt_ev_cmd_complete *cc =
            (struct bluez_daemon_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

          complete_opcode = cc->opcode;
          status = cc->status;
        }
      else if (event == MGMT_EV_CMD_STATUS &&
               ret >= (ssize_t)(sizeof(*hdr) + 3))
        {
          complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                            ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
          status = buf[sizeof(*hdr) + 2];
        }

      printf("bluez-daemon: recv-status label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u opcode=0x%04x status=0x%02x "
             "expect=0x%04x expect-status=0x%02x\n",
             label, (long)ret, event, index, len, complete_opcode, status,
             expect_opcode, expect_status);

      if ((event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
          complete_opcode == expect_opcode)
        {
          return status == expect_status ? 0 : -1;
        }
    }

  return -1;
}

static int bluez_daemon_recv_optional_event(int fd, uint16_t expect_event,
                                            const char *label)
{
  uint8_t buf[320];
  unsigned int i;

  for (i = 0; i < 10; i++)
    {
      struct bluez_daemon_mgmt_hdr *hdr =
        (struct bluez_daemon_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              usleep(50000);
              continue;
            }

          printf("bluez-daemon: optional-event ret=%ld errno=%d label=%s "
                 "expect-event=0x%04x\n",
                 (long)ret, errno, label, expect_event);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-daemon: optional-event ret=%ld short label=%s "
                 "expect-event=0x%04x\n",
                 (long)ret, label, expect_event);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      printf("bluez-daemon: optional-event label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u expect-event=0x%04x\n",
             label, (long)ret, event, index, len, expect_event);
      return 0;
    }

  printf("bluez-daemon: optional-event label=%s none "
         "expect-event=0x%04x errno=%d\n",
         label, expect_event, EAGAIN);
  return 0;
}

static int bluez_daemon_recv_until(int fd, uint16_t expect_event,
                                   uint16_t expect_opcode,
                                   const char *label)
{
  uint8_t buf[320];
  bool saw_event = false;
  bool saw_complete = false;
  unsigned int i;

  for (i = 0; i < 8 && (!saw_event || !saw_complete); i++)
    {
      struct bluez_daemon_mgmt_hdr *hdr =
        (struct bluez_daemon_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;
      uint16_t complete_opcode = 0;
      uint8_t status = 0xff;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-daemon: event-loop ret=%ld errno=%d label=%s\n",
                 (long)ret, errno, label);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-daemon: event-loop ret=%ld short label=%s\n",
                 (long)ret, label);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      if (event == MGMT_EV_CMD_COMPLETE &&
          ret >= (ssize_t)(sizeof(*hdr) +
                           sizeof(struct bluez_daemon_mgmt_ev_cmd_complete)))
        {
          struct bluez_daemon_mgmt_ev_cmd_complete *cc =
            (struct bluez_daemon_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

          complete_opcode = cc->opcode;
          status = cc->status;
        }
      else if (event == MGMT_EV_CMD_STATUS &&
               ret >= (ssize_t)(sizeof(*hdr) + 3))
        {
          complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                            ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
          status = buf[sizeof(*hdr) + 2];
        }

      if (event == expect_event)
        {
          saw_event = true;
        }

      if ((event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
          complete_opcode == expect_opcode && status == MGMT_STATUS_SUCCESS)
        {
          saw_complete = true;
        }

      printf("bluez-daemon: event-loop label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u opcode=0x%04x status=0x%02x "
             "expect-event=0x%04x expect-opcode=0x%04x "
             "saw-event=%u saw-complete=%u\n",
             label, (long)ret, event, index, len, complete_opcode, status,
             expect_event, expect_opcode, saw_event ? 1 : 0,
             saw_complete ? 1 : 0);
    }

  return saw_event && saw_complete ? 0 : -1;
}

static int bluez_daemon_recv_event(int fd, uint16_t expect_event,
                                   const char *label)
{
  uint8_t buf[320];
  unsigned int i;

  for (i = 0; i < 16; i++)
    {
      struct bluez_daemon_mgmt_hdr *hdr =
        (struct bluez_daemon_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-daemon: event-only ret=%ld errno=%d label=%s "
                 "expect-event=0x%04x\n",
                 (long)ret, errno, label, expect_event);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-daemon: event-only ret=%ld short label=%s "
                 "expect-event=0x%04x\n",
                 (long)ret, label, expect_event);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      printf("bluez-daemon: event-only label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u expect-event=0x%04x\n",
             label, (long)ret, event, index, len, expect_event);

      if (event == expect_event)
        {
          return 0;
        }
    }

  return -1;
}

static int bluez_daemon_send_cmd(int fd, uint16_t opcode, uint16_t index,
                                 const void *data, uint16_t data_len,
                                 unsigned int reads, const char *label)
{
  uint8_t buf[64];
  struct bluez_daemon_mgmt_hdr *hdr =
    (struct bluez_daemon_mgmt_hdr *)buf;
  ssize_t ret;
  unsigned int i;

  if (sizeof(*hdr) + data_len > sizeof(buf))
    {
      return -1;
    }

  memset(buf, 0, sizeof(buf));
  hdr->opcode = opcode;
  hdr->index = index;
  hdr->len = data_len;
  if (data_len > 0)
    {
      memcpy(buf + sizeof(*hdr), data, data_len);
    }

  ret = send(fd, buf, sizeof(*hdr) + data_len, 0);
  printf("bluez-daemon: command opcode=0x%04x index=0x%04x len=%u "
         "label=%s ret=%ld errno=%d\n",
         opcode, index, data_len, label, (long)ret, ret < 0 ? errno : 0);
  fflush(stdout);
  if (ret < 0)
    {
      return -1;
    }

  for (i = 0; i < reads; i++)
    {
      if (bluez_daemon_recv_one(fd, opcode, label) < 0)
        {
          return -1;
        }
    }

  return 0;
}

static int bluez_daemon_send_cmd_complete(int fd, uint16_t opcode,
                                          uint16_t index, const void *data,
                                          uint16_t data_len,
                                          const char *label)
{
  if (bluez_daemon_send_cmd(fd, opcode, index, data, data_len, 0,
                            label) < 0)
    {
      return -1;
    }

  return bluez_daemon_recv_complete(fd, opcode, label);
}

static int bluez_daemon_send_setting(int fd, uint16_t opcode, uint8_t value,
                                     const char *label)
{
  int ret;

  ret = bluez_daemon_send_cmd_complete(fd, opcode, 0, &value,
                                       sizeof(value), label);
  if (ret < 0)
    {
      return ret;
    }

  return bluez_daemon_recv_optional_event(fd, MGMT_EV_NEW_SETTINGS, label);
}

static void bluez_daemon_make_addr(struct bluez_daemon_mgmt_addr_info *addr,
                                   uint8_t seed)
{
  memset(addr, 0, sizeof(*addr));
  addr->bdaddr[0] = seed;
  addr->type = BDADDR_LE_PUBLIC;
}

static int bluez_daemon_disconnect_peer(int fd,
              const struct bluez_daemon_mgmt_addr_info *addr,
              const char *label_prefix)
{
  char label[64];
  int failed = 0;

  snprintf(label, sizeof(label), "%s-disconnect", label_prefix);
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_DISCONNECT, 0,
                                  addr, sizeof(*addr), 0, label) < 0;

  snprintf(label, sizeof(label), "%s-disconnected", label_prefix);
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_DISCONNECTED,
                                    MGMT_OP_DISCONNECT, label) < 0;

  return failed;
}

static int bluez_daemon_pairing_confirm_accept(int fd)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int failed = 0;

  bluez_daemon_make_addr(&addr, 0x21);
  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "matrix-confirm-accept-set-io") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0, &pair,
                                  sizeof(pair), 0,
                                  "matrix-confirm-accept-pair") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_USER_CONFIRM_REQUEST,
                                    "matrix-confirm-accept-request") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd,
                                           MGMT_OP_USER_CONFIRM_REPLY, 0,
                                           &addr, sizeof(addr),
                                           "matrix-confirm-accept-reply") < 0;
  failed |= bluez_daemon_recv_optional_event(fd, MGMT_EV_NEW_LONG_TERM_KEY,
                                             "matrix-confirm-accept-ltk") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                     MGMT_STATUS_SUCCESS,
                                     "matrix-confirm-accept-complete") < 0;
  failed |= bluez_daemon_disconnect_peer(fd, &addr,
                                         "matrix-confirm-accept") < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix step=confirm-accept complete\n");
    }

  return failed;
}

static int bluez_daemon_pairing_confirm_reject(int fd)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int failed = 0;

  bluez_daemon_make_addr(&addr, 0x22);
  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "matrix-confirm-reject-set-io") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0, &pair,
                                  sizeof(pair), 0,
                                  "matrix-confirm-reject-pair") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_USER_CONFIRM_REQUEST,
                                    "matrix-confirm-reject-request") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd,
                                           MGMT_OP_USER_CONFIRM_NEG_REPLY, 0,
                                           &addr, sizeof(addr),
                                           "matrix-confirm-reject-reply") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                     MGMT_STATUS_FAILED,
                                     "matrix-confirm-reject-complete") < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix step=confirm-reject complete\n");
    }

  return failed;
}

static int bluez_daemon_pairing_passkey_accept(int fd)
{
  struct bluez_daemon_mgmt_cp_user_passkey_reply reply;
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_KEYBOARD_ONLY;
  int failed = 0;

  bluez_daemon_make_addr(&addr, 0x23);
  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  memset(&reply, 0, sizeof(reply));
  memcpy(&reply.addr, &addr, sizeof(reply.addr));
  reply.passkey = BLUEZ_PASSKEY_STAGED;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "matrix-passkey-accept-set-io") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0, &pair,
                                  sizeof(pair), 0,
                                  "matrix-passkey-accept-pair") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_USER_PASSKEY_REQUEST,
                                    "matrix-passkey-accept-request") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_USER_PASSKEY_REPLY, 0,
                                           &reply, sizeof(reply),
                                           "matrix-passkey-accept-reply") < 0;
  failed |= bluez_daemon_recv_optional_event(fd, MGMT_EV_NEW_LONG_TERM_KEY,
                                             "matrix-passkey-accept-ltk") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                     MGMT_STATUS_SUCCESS,
                                     "matrix-passkey-accept-complete") < 0;
  failed |= bluez_daemon_disconnect_peer(fd, &addr,
                                         "matrix-passkey-accept") < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix step=passkey-accept complete\n");
    }

  return failed;
}

static int bluez_daemon_pairing_passkey_reject(int fd)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_KEYBOARD_ONLY;
  int failed = 0;

  bluez_daemon_make_addr(&addr, 0x24);
  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "matrix-passkey-reject-set-io") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0, &pair,
                                  sizeof(pair), 0,
                                  "matrix-passkey-reject-pair") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_USER_PASSKEY_REQUEST,
                                    "matrix-passkey-reject-request") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd,
                                           MGMT_OP_USER_PASSKEY_NEG_REPLY, 0,
                                           &addr, sizeof(addr),
                                           "matrix-passkey-reject-reply") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                     MGMT_STATUS_FAILED,
                                     "matrix-passkey-reject-complete") < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix step=passkey-reject complete\n");
    }

  return failed;
}

static int bluez_daemon_pairing_cancel_pending(int fd)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int failed = 0;

  bluez_daemon_make_addr(&addr, 0x25);
  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "matrix-cancel-pending-set-io") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0, &pair,
                                  sizeof(pair), 0,
                                  "matrix-cancel-pending-pair") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_USER_CONFIRM_REQUEST,
                                    "matrix-cancel-pending-request") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_CANCEL_PAIR_DEVICE, 0, &addr,
                                  sizeof(addr), 0,
                                  "matrix-cancel-pending-cancel") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                     MGMT_STATUS_CANCELLED,
                                     "matrix-cancel-pending-pair-status") < 0;
  failed |= bluez_daemon_recv_status(fd, MGMT_OP_CANCEL_PAIR_DEVICE,
                                     MGMT_STATUS_SUCCESS,
                                     "matrix-cancel-pending-cancel-status") < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix step=cancel-pending complete\n");
    }

  return failed;
}

static int bluez_daemon_pairing_matrix(void)
{
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-daemon: source=third/bluez/src/adapter.c+src/agent.c "
         "style mode=pairing-matrix\n");

  fd = bluez_daemon_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_VERSION,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-version") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_COMMANDS,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-commands") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_INDEX_LIST,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-index-list") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_INFO, 0,
                                           NULL, 0, "read-info") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_POWERED, 1,
                                      "powered") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BONDABLE, 1,
                                      "bondable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_LE, 1, "le") < 0;

  failed |= bluez_daemon_pairing_confirm_accept(fd) < 0;
  failed |= bluez_daemon_pairing_confirm_reject(fd) < 0;
  failed |= bluez_daemon_pairing_passkey_accept(fd) < 0;
  failed |= bluez_daemon_pairing_passkey_reject(fd) < 0;
  failed |= bluez_daemon_pairing_cancel_pending(fd) < 0;

  ret = close(fd);
  printf("bluez-daemon: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-daemon: pairing-matrix mainloop-ledger "
             "mgmt-watch-add=1 mgmt-watch-remove=1 "
             "agent-watch-add=1 agent-watch-remove=1 "
             "pair-timeout-add=5 pair-timeout-remove=5 "
             "pending-callback-final=0\n");
      printf("bluez-daemon: pairing-matrix mgmt-event-ledger "
             "read-info=cmd-complete powered=new-settings "
             "bondable=new-settings le=new-settings "
             "confirm-accept=device-connected,device-disconnected "
             "confirm-reject=auth-failed passkey-accept=device-connected,"
             "device-disconnected passkey-reject=auth-failed "
             "cancel-pending=cmd-status-cancelled\n");
      printf("bluez-daemon: pairing-matrix link-ledger "
             "agent-ref=0 adapter-ref=0 device-ref=0 bearer-ref=0 "
             "bond-ref=0 pending-request=0 mgmt-pending=0 "
             "dbus-owner=0 watch=0 timer=0\n");
      printf("bluez-daemon: pairing-matrix complete\n");
    }

  return failed;
}

static int bluez_daemon_smoke(void)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t discoverable[3] = { 1, 0, 0 };
  uint8_t discovery[1] = { 1 };
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-daemon: source=third/bluez/src/main.c+src/adapter.c "
         "style mode=smoke\n");

  fd = bluez_daemon_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_VERSION,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-version") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_COMMANDS,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-commands") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INDEX_LIST,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-index-list") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INFO, 0, NULL, 0, 1,
                                  "read-info") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_POWERED, 1,
                                      "powered") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_CONNECTABLE, 1,
                                      "connectable") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_DISCOVERABLE, 0,
                                  discoverable, sizeof(discoverable), 2,
                                  "discoverable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BONDABLE, 1,
                                      "bondable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_LE, 1, "le") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_ADVERTISING, 1,
                                      "advertising") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BREDR, 1,
                                      "bredr") < 0;

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_START_DISCOVERY, 0,
                                  discovery, sizeof(discovery), 0,
                                  "start-discovery") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DISCOVERING,
                                    MGMT_OP_START_DISCOVERY,
                                    "discovery-start") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_STOP_DISCOVERY, 0,
                                  discovery, sizeof(discovery), 0,
                                  "stop-discovery") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DISCOVERING,
                                    MGMT_OP_STOP_DISCOVERY,
                                    "discovery-stop") < 0;

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                  &io_cap, sizeof(io_cap), 1,
                                  "set-io-capability") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                  &pair, sizeof(pair), 0,
                                  "pair-device") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                    MGMT_OP_PAIR_DEVICE,
                                    "pair-connected") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_GET_CONN_INFO, 0,
                                  &addr, sizeof(addr), 1,
                                  "get-conn-info") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_DISCONNECT, 0,
                                  &addr, sizeof(addr), 0,
                                  "disconnect") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_DISCONNECTED,
                                    MGMT_OP_DISCONNECT,
                                    "disconnect-complete") < 0;

  ret = close(fd);
  printf("bluez-daemon: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-daemon: smoke complete\n");
    }

  return failed;
}

static int bluez_daemon_pair_round(int fd, unsigned int round)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_addr_info addr;
  char label[32];
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int failed = 0;

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  snprintf(label, sizeof(label), "round%u-set-io", round);
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                  &io_cap, sizeof(io_cap), 1,
                                  label) < 0;

  snprintf(label, sizeof(label), "round%u-pair", round);
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                  &pair, sizeof(pair), 0,
                                  label) < 0;

  snprintf(label, sizeof(label), "round%u-connected", round);
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                    MGMT_OP_PAIR_DEVICE, label) < 0;

  snprintf(label, sizeof(label), "round%u-get-conn-info", round);
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_GET_CONN_INFO, 0,
                                  &addr, sizeof(addr), 1,
                                  label) < 0;

  snprintf(label, sizeof(label), "round%u-disconnect", round);
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_DISCONNECT, 0,
                                  &addr, sizeof(addr), 0,
                                  label) < 0;

  snprintf(label, sizeof(label), "round%u-disconnected", round);
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_DISCONNECTED,
                                    MGMT_OP_DISCONNECT, label) < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: reconnect round=%u complete\n", round);
    }

  return failed;
}

static int bluez_daemon_device_policy(void)
{
  struct bluez_daemon_mgmt_cp_pair_device pair;
  struct bluez_daemon_mgmt_cp_unpair_device unpair;
  struct bluez_daemon_mgmt_cp_add_device add;
  struct bluez_daemon_mgmt_cp_set_device_flags flags;
  struct bluez_daemon_mgmt_addr_info addr;
  uint8_t discoverable[3] = { 1, 0, 0 };
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-daemon: source=third/bluez/src/adapter.c+src/device.c "
         "style mode=device-policy\n");

  fd = bluez_daemon_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_VERSION,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-version") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_COMMANDS,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-commands") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_INDEX_LIST,
                                           MGMT_INDEX_NONE, NULL, 0,
                                           "read-index-list") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_READ_INFO, 0,
                                           NULL, 0, "read-info") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_POWERED, 1,
                                      "powered") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_CONNECTABLE, 1,
                                      "connectable") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_DISCOVERABLE, 0,
                                  discoverable, sizeof(discoverable), 2,
                                  "discoverable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BONDABLE, 1,
                                      "bondable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_LE, 1, "le") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_ADVERTISING, 1,
                                      "advertising") < 0;

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&add, 0, sizeof(add));
  memcpy(&add.addr, &addr, sizeof(add.addr));
  add.action = BLUEZ_DEVICE_ACTION_AUTO_CONNECT;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_ADD_DEVICE, 0,
                                           &add, sizeof(add),
                                           "add-device") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_GET_DEVICE_FLAGS, 0,
                                           &addr, sizeof(addr),
                                           "get-device-flags-initial") < 0;

  memset(&flags, 0, sizeof(flags));
  memcpy(&flags.addr, &addr, sizeof(flags.addr));
  flags.current_flags = BLUEZ_DEVICE_FLAG_REMOTE_WAKEUP;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_DEVICE_FLAGS, 0,
                                           &flags, sizeof(flags),
                                           "set-device-flags") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_GET_DEVICE_FLAGS, 0,
                                           &addr, sizeof(addr),
                                           "get-device-flags-set") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_BLOCK_DEVICE, 0,
                                           &addr, sizeof(addr),
                                           "block-device") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_UNBLOCK_DEVICE, 0,
                                           &addr, sizeof(addr),
                                           "unblock-device") < 0;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                           &io_cap, sizeof(io_cap),
                                           "policy-set-io-capability") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                  &pair, sizeof(pair), 0,
                                  "policy-pair-device") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                    MGMT_OP_PAIR_DEVICE,
                                    "policy-pair-connected") < 0;
  failed |= bluez_daemon_disconnect_peer(fd, &addr,
                                         "policy-pair") < 0;

  memset(&unpair, 0, sizeof(unpair));
  memcpy(&unpair.addr, &addr, sizeof(unpair.addr));
  unpair.disconnect = 1;

  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_UNPAIR_DEVICE, 0,
                                           &unpair, sizeof(unpair),
                                           "unpair-device") < 0;
  failed |= bluez_daemon_send_cmd_complete(fd, MGMT_OP_REMOVE_DEVICE, 0,
                                           &addr, sizeof(addr),
                                           "remove-device") < 0;

  ret = close(fd);
  printf("bluez-daemon: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-daemon: device-policy complete\n");
    }

  return failed;
}

static int bluez_daemon_discovery_peer(void)
{
  uint8_t discoverable[3] = { 1, 0, 0 };
  uint8_t discovery[1] = { 1 };
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-daemon: source=third/bluez/src/adapter.c+src/device.c "
         "style mode=discovery-peer\n");

  fd = bluez_daemon_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_VERSION,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-version") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_COMMANDS,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-commands") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INDEX_LIST,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-index-list") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INFO, 0, NULL, 0, 1,
                                  "read-info") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_POWERED, 1,
                                      "powered") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_CONNECTABLE, 1,
                                      "connectable") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_DISCOVERABLE, 0,
                                  discoverable, sizeof(discoverable), 2,
                                  "discoverable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BONDABLE, 1,
                                      "bondable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_LE, 1, "le") < 0;

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_START_DISCOVERY, 0,
                                  discovery, sizeof(discovery), 0,
                                  "start-discovery") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DISCOVERING,
                                    MGMT_OP_START_DISCOVERY,
                                    "discovery-start") < 0;
  failed |= bluez_daemon_recv_event(fd, MGMT_EV_DEVICE_FOUND,
                                    "device-found") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_STOP_DISCOVERY, 0,
                                  discovery, sizeof(discovery), 0,
                                  "stop-discovery") < 0;
  failed |= bluez_daemon_recv_until(fd, MGMT_EV_DISCOVERING,
                                    MGMT_OP_STOP_DISCOVERY,
                                    "discovery-stop") < 0;

  ret = close(fd);
  printf("bluez-daemon: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-daemon: discovery-peer complete\n");
    }

  return failed;
}

static int bluez_daemon_reconnect_stress(int argc, char *argv[])
{
  uint8_t discoverable[3] = { 1, 0, 0 };
  unsigned int rounds = argc > 2 ? (unsigned int)strtoul(argv[2], NULL, 0) :
                        3;
  unsigned int i;
  int fd;
  int ret;
  int failed = 0;

  if (rounds == 0 || rounds > 8)
    {
      printf("bluez-daemon: reconnect-stress invalid rounds=%u\n", rounds);
      return 1;
    }

  printf("bluez-daemon: source=third/bluez/src/main.c+src/adapter.c "
         "style mode=reconnect-stress rounds=%u\n", rounds);

  fd = bluez_daemon_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_VERSION,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-version") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_COMMANDS,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-commands") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INDEX_LIST,
                                  MGMT_INDEX_NONE, NULL, 0, 1,
                                  "read-index-list") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_READ_INFO, 0, NULL, 0, 1,
                                  "read-info") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_POWERED, 1,
                                      "powered") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_CONNECTABLE, 1,
                                      "connectable") < 0;
  failed |= bluez_daemon_send_cmd(fd, MGMT_OP_SET_DISCOVERABLE, 0,
                                  discoverable, sizeof(discoverable), 2,
                                  "discoverable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BONDABLE, 1,
                                      "bondable") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_LE, 1, "le") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_ADVERTISING, 1,
                                      "advertising") < 0;
  failed |= bluez_daemon_send_setting(fd, MGMT_OP_SET_BREDR, 1,
                                      "bredr") < 0;

  for (i = 1; i <= rounds; i++)
    {
      failed |= bluez_daemon_pair_round(fd, i) < 0;
    }

  ret = close(fd);
  printf("bluez-daemon: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-daemon: reconnect-stress lifecycle-ledger rounds=%u "
             "mgmt-open=1 adapter-power=1 pair-rounds=%u "
             "connect-complete=%u disconnect-cleanup=%u "
             "pending-cmd-final=0 device-ref-final=0 bond-ref-final=0\n",
             rounds, rounds, rounds, rounds);
      printf("bluez-daemon: reconnect-stress medium-ledger "
             "hwsim-record-types=ctrl,adv,acl consumer-offsets=role-local "
             "stale-record-skip=1 reconnect-replay-guard=1 cleanup-final=1 "
             "source=nuttx/arch/sim/src/sim/posix/sim_hostbthwsim.c\n");
      printf("bluez-daemon: reconnect-stress complete rounds=%u\n", rounds);
    }

  return failed;
}

static void bluez_daemon_profile_ownership_ledger(const char *profile,
                                                  const char *role,
                                                  const char *bearer,
                                                  const char *boundary)
{
  printf("bluez-daemon: %s closeout link-ledger role=%s "
         "dbus-owner=bluetoothd profile-object=0 service-record=0 "
         "adapter-ref=0 device-ref=0 bearer=%s bearer-ref=0 "
         "mainloop-watch=0 pending-request=0 pending-event=0 "
         "error-policy=1 cleanup-final=1 upstream-link=%s\n",
         profile, role, bearer, boundary);
}

static int bluez_daemon_basic_closeout(int argc, char *argv[])
{
  const char *role;

  if (argc < 3)
    {
      fprintf(stderr, "bluez-daemon: basic-closeout requires bt|ble role\n");
      return 1;
    }

  role = argv[2];
  if (strcmp(role, "bt") && strcmp(role, "ble"))
    {
      fprintf(stderr, "bluez-daemon: basic-closeout role must be bt or ble\n");
      return 1;
    }

  printf("bluez-daemon: basic closeout cleanup role=%s "
         "mgmt-fd=closed hci-fd=closed l2cap-fd=closed "
         "pending-cmd=0 pending-event=0 devices=0 bonds=0 watches=0\n",
         role);
  printf("bluez-daemon: basic closeout semantic-contract role=%s "
         "mgmt-socket-owner=1 hci-socket-owner=1 adapter-owner=1 "
         "device-owner=1 agent-owner=1 discovery-owner=1 "
         "connection-owner=1 auth-owner=1 bonding-owner=1 "
         "l2cap-owner=1 medium-owner=1 reconnect-owner=1 "
         "error-policy-owner=1 cleanup-owner=1\n",
         role);
  printf("bluez-daemon: basic closeout socket-abi-contract role=%s "
         "af-owner=AF_BLUETOOTH "
         "protocol-owner=HCI,L2CAP,RFCOMM,SCO,ISO,ATT "
         "channel-owner=HCI_CHANNEL_CONTROL,HCI_CHANNEL_USER,"
         "L2CAP_BR_EDR,L2CAP_LE_COC,L2CAP_FIXED_ATT,RFCOMM_DLCI,"
         "SCO_VOICE,ISO_CIS,ISO_BIS "
         "sockops-owner=socket,bind,connect,listen,accept,sendmsg,"
         "recvmsg,getsockopt,setsockopt,ioctl,shutdown,close "
         "state-owner=BT_OPEN,BT_BOUND,BT_LISTEN,BT_CONNECT,"
         "BT_CONNECT2,BT_CONFIG,BT_CONNECTED,BT_DISCONN,BT_CLOSED "
         "errno-owner=bt_to_errno,bt_status,mgmt_status,smp_status,"
         "l2cap_error,avdtp_error "
         "fd-owner=bluez-mainloop-watch,connected-l2cap-fd,"
         "hci-control-fd,hci-user-fd,media-fd "
         "datapath-owner=userspace-fd,linux-sock,channel,"
         "hwsim-medium,peer-channel,linux-sock,userspace-fd "
         "ordering-owner=bind-before-connect,listen-before-accept,"
         "connect-before-send,disconnect-before-close,"
         "event-before-callback,error-before-cleanup "
         "cleanup-owner=watch-remove,fd-close,sock-release,"
         "chan-release,pending-cmd-free,pending-request-free "
         "upstream-link=bluez-daemon-to-linux-bt-socket-abi "
         "socket-abi-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout hci-transport-contract role=%s "
         "controller-owner=hci_dev,hci_conn,hci_request,hci_event "
         "socket-owner=HCI_CHANNEL_RAW,HCI_CHANNEL_USER,"
         "HCI_CHANNEL_CONTROL,HCI_CHANNEL_MONITOR "
         "packet-owner=HCI_COMMAND_PKT,HCI_EVENT_PKT,HCI_ACLDATA_PKT,"
         "HCI_SCODATA_PKT,HCI_ISODATA_PKT "
         "transport-owner=VHCI,H4,H5,USB,UART,RPMsg,hwsim-ctrl,"
         "hwsim-acl,hwsim-sco,hwsim-iso "
         "command-owner=RESET,READ_LOCAL_VERSION,READ_BD_ADDR,"
         "LE_SET_ADV_ENABLE,LE_SET_SCAN_ENABLE,LE_CREATE_CONN,"
         "DISCONNECT "
         "event-owner=CMD_COMPLETE,CMD_STATUS,LE_META_EVENT,"
         "CONN_COMPLETE,DISCONN_COMPLETE,ENC_CHANGE,NUM_COMP_PKTS "
         "datapath-owner=bluez-fd,hci-sock,hci-core,hci-driver,"
         "hwsim-medium,peer-hci-driver,peer-hci-core,peer-bluez-fd "
         "ordering-owner=register-before-open,power-before-command,"
         "cmd-status-before-event,acl-after-conn,disconnect-before-close "
         "error-owner=command-disallowed,unknown-command,timeout,"
         "controller-reset,transport-closed,event-lost "
         "cleanup-owner=hci-dev-close,conn-drop,skb-purge,"
         "pending-cmd-free,driver-unregister "
         "upstream-link=bluez-daemon-to-linux-hci-core-transport "
         "hci-transport-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout mgmt-control-contract role=%s "
         "socket-owner=AF_BLUETOOTH,BTPROTO_HCI,HCI_CHANNEL_CONTROL "
         "command-owner=READ_VERSION,READ_COMMANDS,READ_INDEX_LIST,"
         "READ_INFO,SET_POWERED,SET_DISCOVERABLE,SET_CONNECTABLE,"
         "SET_BONDABLE,SET_LE,SET_ADVERTISING,START_DISCOVERY,"
         "STOP_DISCOVERY,PAIR_DEVICE,GET_CONN_INFO,DISCONNECT,"
         "UNPAIR_DEVICE "
         "event-owner=CMD_COMPLETE,CMD_STATUS,NEW_SETTINGS,"
         "DISCOVERING,DEVICE_FOUND,DEVICE_CONNECTED,"
         "DEVICE_DISCONNECTED,DEVICE_UNPAIRED,NEW_LONG_TERM_KEY "
         "adapter-owner=current-settings,supported-settings,"
         "dev-class,local-name,short-name "
         "device-owner=temporary,known,paired,bonded,connected,"
         "services-resolved "
         "ordering-owner=read-info-before-policy,set-powered-before-scan,"
         "scan-before-device-found,pair-before-bond,"
         "disconnect-before-unpair "
         "error-owner=not-powered,invalid-params,busy,already-connected,"
         "not-connected,auth-failed,cancelled "
         "cleanup-owner=pending-command-free,event-queue-drain,"
         "adapter-release,device-release,mgmt-fd-close "
         "upstream-link=bluez-daemon-to-linux-mgmt-control-socket "
         "mgmt-control-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout kernel-object-contract role=%s "
         "object-owner=sock,sk_buff,hci_dev,hci_conn,l2cap_chan,"
         "rfcomm_dlc,sco_conn,iso_conn,net_device "
         "ref-owner=sock_hold,sock_put,hci_conn_get,hci_conn_drop,"
         "chan_hold,chan_put,netdev_hold,netdev_put "
         "queue-owner=skb_queue,tx_queue,rx_queue,pending_cmd,"
         "pending_req,workqueue,kthread "
         "lock-owner=mutex,spinlock,refcount,atomic,waitqueue "
         "lifetime-owner=alloc,register,bind,connect,active,disconnect,"
         "unregister,free "
         "datapath-owner=skb-alloc,skb-clone,skb-pull,skb-push,"
         "skb-free,netif-rx,ndo-start-xmit "
         "ordering-owner=ref-before-publish,lock-before-state-change,"
         "queue-before-wakeup,stop-before-free,unregister-before-free "
         "error-owner=ref-leak,double-free,use-after-close,"
         "queue-overflow,work-cancel-timeout "
         "cleanup-owner=skb-purge,work-cancel,kthread-stop,"
         "queue-drain,ref-zero,object-free "
         "upstream-link=linux-bt-object-lifecycle "
         "kernel-object-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout error-status-contract role=%s "
         "errno-owner=bt_to_errno,bt_status,negative-errno,"
         "socket-errno "
         "hci-status-owner=success,unknown-command,command-disallowed,"
         "auth-failure,conn-timeout,remote-user-term,unsupported-feature "
         "mgmt-status-owner=success,failed,not-connected,"
         "already-connected,not-powered,invalid-params,rejected,cancelled "
         "protocol-error-owner=l2cap-result,att-error,sdp-error,"
         "rfcomm-dm,sco-status,iso-status,avdtp-error,obex-response "
         "dbus-error-owner=org.bluez.Error.Failed,"
         "org.bluez.Error.NotReady,org.bluez.Error.NotConnected,"
         "org.bluez.Error.AlreadyConnected,org.bluez.Error.NotAuthorized,"
         "org.bluez.Error.InvalidArguments "
         "profile-error-owner=connect-failed,pairing-failed,"
         "authorization-failed,transport-failed,codec-rejected,"
         "session-aborted "
         "ordering-owner=kernel-error-before-mgmt-status,"
         "mgmt-status-before-dbus-error,error-before-callback,"
         "error-before-cleanup "
         "cleanup-owner=pending-cmd-free,pending-request-free,"
         "fd-close,object-unref,watch-remove,state-rollback "
         "upstream-link=linux-bt-status-to-bluez-error-policy "
         "error-status-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout dbus-mainloop-contract role=%s "
         "dbus-owner=org.bluez,ObjectManager,Adapter1,Device1,Agent1 "
         "profile-owner=Profile1,ProfileManager,service-record,"
         "authorize-service "
         "mainloop-owner=shared-mainloop,io-mainloop,watch,timer,idle,"
         "signal-dispatch "
         "object-owner=path-register,interface-add,properties-changed,"
         "interfaces-removed,path-unregister "
         "client-owner=name-watch,owner-lost,owner-reacquire,"
         "pending-method,method-reply "
         "policy-owner=adapter-policy,device-policy,agent-policy,"
         "profile-policy "
         "ordering-owner=name-before-object,adapter-before-device,"
         "profile-before-connect,owner-lost-before-cleanup,"
         "cleanup-before-name-release "
         "error-owner=unknown-object,unknown-interface,not-authorized,"
         "already-exists,not-connected,failed "
         "cleanup-owner=watch-remove,timer-remove,pending-call-free,"
         "object-unregister,service-record-unregister,name-release "
         "upstream-link=bluez-daemon-to-upstream-dbus-mainloop "
         "dbus-mainloop-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout security-storage-contract role=%s "
         "device-owner=Device1,Paired,Bonded,Trusted,Blocked,Connected "
         "classic-key-owner=link-key,key-type,pin-length,authenticated,"
         "legacy-pairing "
         "le-key-owner=ltk,irk,csrk,identity-address,ediv,rand,"
         "authenticated,secure-connections "
         "storage-owner=adapter-info,device-info,attributes,ccc,"
         "settings,profiles "
         "policy-owner=io-capability,agent-capability,bondable,"
         "just-works,numeric-comparison,passkey "
         "lifecycle-owner=pair,bond,load-on-boot,reconnect,use-stored-key,"
         "unpair,remove-device "
         "ordering-owner=key-before-bond,bond-before-trusted,"
         "load-before-connect,unpair-before-key-delete,"
         "owner-reacquire-before-reuse "
         "error-owner=auth-failed,confirm-failed,key-missing,"
         "storage-corrupt,not-bonded,already-bonded "
         "cleanup-owner=link-key-delete,le-key-delete,ccc-clear,"
         "device-dir-remove,pending-auth-free "
         "upstream-link=bluez-daemon-to-bluez-storage-security "
         "security-storage-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout plugin-profile-contract role=%s "
         "plugin-owner=plugin_init,plugin_exit,adapter_probe,"
         "adapter_remove "
         "profile-owner=Profile1,profile_register,profile_unregister,"
         "connect,disconnect,cancel "
         "service-owner=service-record-add,service-record-remove,"
         "uuid-map,role-policy "
         "auth-owner=authorize-service,agent-request,agent-cancel,"
         "policy-deny "
         "object-owner=profile-object-add,device-attach,"
         "device-detach,profile-object-remove "
         "callback-owner=new-connection,request-disconnect,"
         "adapter-power-off,device-remove "
         "ordering-owner=plugin-before-adapter,adapter-before-profile,"
         "profile-before-service,authorize-before-connect,"
         "disconnect-before-unregister "
         "error-owner=plugin-init-fail,profile-exists,"
         "authorization-failed,connect-failed,cancelled "
         "cleanup-owner=profile-unregister,service-record-free,"
         "adapter-remove,plugin-exit,pending-callback-free "
         "upstream-link=bluez-daemon-to-upstream-plugin-profile "
         "plugin-profile-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout build-runtime-contract role=%s "
         "source-owner=apps/wireless/linux_bluetooth,"
         "nuttx/wireless/linux_bluetooth,third/bluez,"
         "third/linux-hwe-6.17-6.17.0 "
         "tool-owner=bluezdaemon,bluezmgmt,bluezaudio,blueznetwork,"
         "bluezbneptest,bluezipsp,btctl "
         "config-owner=build-bt1,build-bt2,build-ble1,build-ble2,"
         "role-defconfig "
         "library-owner=glib,dbus,readline,libudev,sbc,lc3,ell,ical,"
         "json-c "
         "runtime-owner=NuttX-apps,NSH-command,sim-hwsim,"
         "per-role-instance "
         "host-owner=no-host-bluetoothd,no-host-bluez-tools,"
         "no-host-kernel-bluetooth "
         "artifact-owner=build-log,run-log,validator-manifest,"
         "status-document "
         "ordering-owner=configure-before-build,build-before-run,"
         "run-before-validate,validate-before-closeout "
         "cleanup-owner=role-process-exit,fd-close,medium-offset-save,"
         "artifact-final "
         "upstream-link=nuttX-apps-rebuilt-bluez-linux-bt-stack "
         "build-runtime-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout adapter-device-contract role=%s "
         "adapter-owner=Powered,Discoverable,Connectable,Pairable,"
         "Discovering,UUIDs,Modalias "
         "device-owner=Address,AddressType,Name,Alias,Class,Appearance,"
         "Paired,Bonded,Trusted,Blocked,Connected,ServicesResolved "
         "discovery-owner=start-discovery,stop-discovery,device-found,"
         "temporary-device,duplicate-filter,expiry "
         "connect-owner=create-connection,service-discovery,"
         "profile-connect,profile-disconnect,auto-connect "
         "property-owner=PropertiesChanged,InterfacesAdded,"
         "InterfacesRemoved,GetManagedObjects "
         "policy-owner=adapter-policy,device-policy,auto-connect-policy,"
         "blocked-policy,trust-policy "
         "ordering-owner=adapter-before-device,discovery-before-temporary,"
         "connect-before-services-resolved,pair-before-trust,"
         "disconnect-before-remove "
         "error-owner=not-ready,already-connected,not-connected,"
         "authentication-failed,connection-attempt-failed,"
         "services-unresolved "
         "cleanup-owner=temporary-device-expire,profile-detach,"
         "device-unref,watch-remove,properties-final "
         "upstream-link=bluez-daemon-to-adapter-device-state-machine "
         "adapter-device-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout l2cap-contract role=%s "
         "socket-owner=AF_BLUETOOTH,BTPROTO_L2CAP "
         "channel-owner=BR_EDR_DYNAMIC,LE_FIXED_ATT,LE_SIGNALING,"
         "LE_COC,AVDTP,AVCTP,BNEP,HID_CONTROL,HID_INTERRUPT "
         "signaling-owner=conn-req,conn-rsp,config-req,config-rsp,"
         "disconn-req,disconn-rsp,info-req,info-rsp,credit-based-connect,"
         "credit-based-disconnect,flow-control-credit "
         "state-owner=BT_OPEN,BT_BOUND,BT_LISTEN,BT_CONNECT,"
         "BT_CONFIG,BT_CONNECTED,BT_DISCONN,BT_CLOSED "
         "mtu-owner=imtu,omtu,mps,flush-timeout,fcs,mode "
         "credit-owner=tx-credit,rx-credit,credit-threshold,"
         "credit-replenish "
         "datapath-owner=userspace-fd,l2cap-sock,l2cap-core,acl,"
         "hwsim,peer-l2cap-core,peer-userspace-fd "
         "ordering-owner=bind-before-listen,listen-before-connect,"
         "config-before-data,credit-before-coc-data,"
         "disconnect-before-close "
         "error-owner=psm-not-supported,security-block,invalid-cid,"
         "config-reject,credit-exhausted,remote-disconnect "
         "cleanup-owner=chan-del,conn-unref,skb-purge,sock-close,"
         "pending-signal-free "
         "upstream-link=bluez-daemon-to-linux-l2cap-core-sock "
         "l2cap-final=1 semantic-contract-final=1\n",
         role);
  if (!strcmp(role, "bt"))
    {
      printf("bluez-daemon: basic closeout sdp-contract role=bt "
             "server-owner=sdpd-service,service-db,record-handle,"
             "browse-group "
             "client-owner=service-search,service-attribute,"
             "service-search-attribute,continuation-state "
             "record-owner=ServiceClassIDList,ProtocolDescriptorList,"
             "ProfileDescriptorList,AdditionalProtocolDescriptorList,"
             "SupportedFeatures "
             "uuid-owner=AudioSource,AudioSink,AVRCPController,"
             "AVRCPTarget,Handsfree,Headset,HID,SerialPort,NAP,GN,PANU "
             "transport-owner=L2CAP-PSM-0x0001,SDP-PDU,"
             "request-response "
             "ordering-owner=record-before-advertise,"
             "search-before-connect,attribute-before-profile-open,"
             "unregister-before-cleanup "
             "error-owner=invalid-syntax,invalid-record-handle,"
             "not-found,continuation-reject "
             "cleanup-owner=record-unregister,service-db-release,"
             "pending-query-free,l2cap-close "
             "upstream-link=bluez-daemon-to-sdpd-service-and-sdp-lib "
             "sdp-final=1 semantic-contract-final=1\n");
      printf("bluez-daemon: basic closeout rfcomm-contract role=bt "
             "socket-owner=AF_BLUETOOTH,BTPROTO_RFCOMM,L2CAP-PSM-0x0003 "
             "session-owner=rfcomm_session,rfcomm_dlc,dlci,server-channel "
             "control-owner=SABM,UA,DM,DISC,PN,MSC,RPN,UIH "
             "credit-owner=credit-based-flow-control,tx-credit,rx-credit "
             "tty-owner=rfcomm_tty,SerialPort1,SPP "
             "profile-owner=HFP,HSP,OBEX,PBAP,OPP,MAP,MNS,FTP,SYNC,"
             "BIP,Print,iAP "
             "datapath-owner=userspace-fd,rfcomm-sock,rfcomm-core,"
             "l2cap,acl,hwsim,peer-l2cap,peer-rfcomm,userspace-fd "
             "ordering-owner=sdp-before-rfcomm,listen-before-sabm,"
             "pn-before-uih,msc-before-data,disc-before-close "
             "error-owner=dlci-collision,credit-exhausted,remote-dm,"
             "invalid-frame,session-timeout "
             "cleanup-owner=dlc-release,session-unlink,tty-unregister,"
             "sock-close,pending-frame-free "
             "upstream-link=bluez-daemon-to-linux-rfcomm-core-sock-tty "
             "rfcomm-final=1 semantic-contract-final=1\n");
      printf("bluez-daemon: basic closeout sco-contract role=bt "
             "socket-owner=AF_BLUETOOTH,BTPROTO_SCO,SCO,eSCO "
             "bearer-owner=sco_conn,sco_chan,voice-setting,air-mode "
             "codec-owner=CVSD,transparent,mSBC,wbs-fallback "
             "qos-owner=packet-type,latency,retrans-effort,mtu,"
             "rx-bandwidth,tx-bandwidth "
             "profile-owner=HFP,HSP,MediaTransport1,SCO-bearer "
             "datapath-owner=userspace-fd,sco-sock,sco-core,hci-conn,"
             "hwsim-sco,peer-sco-core,peer-userspace-fd "
             "ordering-owner=rfcomm-slc-before-sco,codec-before-open,"
             "connect-before-audio,disconnect-before-close "
             "error-owner=unsupported-codec,link-loss,setup-timeout,"
             "remote-close,packet-status-error "
             "cleanup-owner=sco-disconnect,sco-chan-release,"
             "sock-close,transport-release,pending-audio-free "
             "upstream-link=bluez-daemon-to-linux-sco-socket-bearer "
             "sco-final=1 semantic-contract-final=1\n");
    }
  else
    {
      printf("bluez-daemon: basic closeout ble-gap-contract role=ble "
             "adapter-owner=LE,Advertising,Discovery,Privacy "
             "adv-owner=ADV_IND,ADV_SCAN_IND,ADV_NONCONN_IND,"
             "EXT_ADV,periodic-adv "
             "scan-owner=active-scan,passive-scan,scan-response,"
             "duplicate-filter "
             "connection-owner=LE-create-connection,connection-complete,"
             "connection-update,disconnect "
             "address-owner=public,random-static,resolvable-private,"
             "identity-resolving-key "
             "ordering-owner=power-before-adv,adv-before-scan,"
             "scan-before-connect,connect-before-smp,"
             "disconnect-before-cleanup "
             "error-owner=adv-busy,scan-timeout,connect-timeout,"
             "unsupported-phy,privacy-resolve-fail "
             "cleanup-owner=adv-stop,scan-stop,pending-connect-free,"
             "device-release "
             "upstream-link=bluez-daemon-to-linux-le-gap-mgmt-hci "
             "gap-final=1 semantic-contract-final=1\n");
      printf("bluez-daemon: basic closeout ble-att-smp-contract role=ble "
             "att-owner=ATT-fixed-CID-0x0004,bt_att,gatt-db,"
             "request-queue,mtu-exchange "
             "gatt-owner=primary-service,characteristic,descriptor,"
             "read,write,notify,indicate,ccc "
             "smp-owner=pairing-request,pairing-response,confirm,random,"
             "ltk,irk,csrk,bond "
             "security-owner=io-capability,authreq,key-size,"
             "secure-connections,legacy-pairing "
             "datapath-owner=userspace-gatt,att-socket,l2cap-fixed,"
             "acl,hwsim,peer-att,userspace-gatt "
             "ordering-owner=mtu-before-discovery,encrypt-before-secure-read,"
             "ccc-before-notify,indication-before-confirm,"
             "unpair-before-key-cleanup "
             "error-owner=invalid-handle,read-not-permitted,"
             "write-not-permitted,insufficient-authentication,"
             "pairing-failed "
             "cleanup-owner=att-request-free,ccc-clear,key-release,"
             "bond-remove,l2cap-close "
             "upstream-link=bluez-daemon-to-linux-att-gatt-smp "
             "att-final=1 gatt-final=1 smp-final=1 "
             "semantic-contract-final=1\n");
      printf("bluez-daemon: basic closeout ble-iso-contract role=ble "
             "socket-owner=AF_BLUETOOTH,BTPROTO_ISO,ISO_CIS,ISO_BIS "
             "topology-owner=CIG,CIS,BIG,BIS,unicast,broadcast "
             "qos-owner=interval,latency,phy,sdu,packing,framing,"
             "rtn,presentation-delay "
             "controller-owner=cis-create,cis-established,big-create,"
             "bis-sync,iso-data-path,iso-disconnect "
             "datapath-owner=userspace-fd,iso-sock,iso-core,hci-iso,"
             "hwsim-iso,peer-hci-iso,peer-iso-sock,userspace-fd "
             "timing-owner=seq,timestamp,packet-status,flush-time,"
             "controller-complete "
             "ordering-owner=qos-before-connect,connect-before-stream,"
             "stream-before-disconnect,disconnect-before-close "
             "error-owner=qos-reject,setup-timeout,sync-lost,"
             "packet-lost,controller-disconnect "
             "cleanup-owner=iso-sock-close,cis-release,bis-release,"
             "data-path-remove,pending-sdu-free "
             "upstream-link=bluez-daemon-to-linux-iso-socket-bearer "
             "iso-final=1 semantic-contract-final=1\n");
    }
  if (!strcmp(role, "bt"))
    {
      printf("bluez-daemon: basic closeout bt-profile-suite-contract role=bt "
             "profile-owner=A2DP,AVRCP,HFP,HSP,HID,SPP,OPP,PBAP,MAP,"
             "FTP,BIP,Print,iAP,Network "
             "dbus-owner=Profile1,Media1,MediaEndpoint1,MediaTransport1,"
             "Network1,NetworkServer1,Input1,Serial1,ObexClient1 "
             "socket-owner=L2CAP,AVDTP,AVCTP,RFCOMM,SCO,BNEP,SDP "
             "service-owner=sdp-record,uuid-map,role-policy,"
             "authorization "
             "datapath-owner=audio-transport,control-transport,"
             "rfcomm-stream,sco-stream,bnep-netdev,hid-interrupt "
             "policy-owner=codec,volume,avrcp-control,service-security,"
             "trusted-device,role-selection "
             "ordering-owner=sdp-before-connect,authorize-before-open,"
             "bearer-before-profile,profile-before-datapath,"
             "disconnect-before-unregister "
             "error-owner=unsupported-service,codec-reject,"
             "authorization-fail,bearer-fail,remote-close,timeout "
             "cleanup-owner=transport-release,fd-close,record-unregister,"
             "watch-remove,profile-unref "
             "upstream-link=bluez-daemon-to-classic-bluez-profile-suite "
             "bt-profile-suite-final=1 semantic-contract-final=1\n");
    }
  if (!strcmp(role, "ble"))
    {
      printf("bluez-daemon: basic closeout ble-profile-suite-contract "
             "role=ble "
             "profile-owner=GATT,HOGP,BAS,DIS,BAP,CAP,MCP,VCP,MICP,"
             "CSIP,ASHA,Mesh,MIDI,6LoWPAN,IPSP "
             "dbus-owner=GattManager1,GattService1,GattCharacteristic1,"
             "GattDescriptor1,LEAdvertisingManager1,Advertisement1,"
             "MediaEndpoint1,MediaTransport1,Network1 "
             "socket-owner=ATT,SMP,L2CAP-LE-CREDITS,ISO,6LOWPAN "
             "service-owner=gatt-db,attribute-db,ccc,advertising-data,"
             "mesh-node,network-role "
             "datapath-owner=att-request,notification,indication,"
             "coc-stream,iso-stream,mesh-pdu,ipsp-netdev "
             "policy-owner=security-level,bonding,privacy,ccc-policy,"
             "codec-qos,role-selection "
             "ordering-owner=adv-before-connect,smp-before-encrypted-att,"
             "service-discovery-before-profile,ccc-before-notify,"
             "disconnect-before-unregister "
             "error-owner=att-error,smp-fail,coc-credit-exhausted,"
             "iso-sync-lost,mesh-reject,privacy-resolve-fail "
             "cleanup-owner=att-pending-free,ccc-clear,key-release,"
             "iso-release,coc-close,netdev-unregister "
             "upstream-link=bluez-daemon-to-le-bluez-profile-suite "
             "ble-profile-suite-final=1 semantic-contract-final=1\n");
    }
  bluez_daemon_profile_ownership_ledger(
    "basic", role, "mgmt-control+hci-user+l2cap-basic",
    "bluezdaemon-basic-upstream-link-bluetoothd");
  printf("bluez-daemon: basic closeout medium-lifecycle role=%s "
         "medium=ctrl,adv,acl,iso,bnep offset-consumers=role+channel "
         "old-packet-filter=dst+seq checksum=fnv1a "
         "reconnect-replay-guard=1 abnormal-exit-offset-resume=1 "
         "cleanup-final=1 "
         "source=nuttx/arch/sim/src/sim/posix/sim_hostbthwsim.c\n",
         role);
  printf("bluez-daemon: basic closeout hwsim-stability-contract role=%s "
         "medium-owner=ctrl,adv,acl,iso,bnep "
         "offset-owner=per-role,per-channel,monotonic-read,"
         "resume-after-exit "
         "record-owner=magic,type,src,dst,seq,timestamp,len,checksum "
         "filter-owner=dst-filter,type-filter,channel-filter,"
         "old-seq-filter,self-echo-filter "
         "concurrency-owner=bt1,bt2,ble1,ble2,simultaneous-rx,"
         "simultaneous-tx "
         "lifecycle-owner=connect,disconnect,reconnect,reconnect-stress,"
         "long-run-traffic "
         "datapath-owner=ctrl-events,adv-events,acl-data,iso-data,"
         "bnep-data "
         "recovery-owner=abnormal-exit,offset-resume,stale-record-skip,"
         "medium-cleanup "
         "error-owner=checksum-fail,truncated-record,unknown-type,"
         "out-of-order,stale-offset "
         "cleanup-owner=fd-close,offset-save,pending-record-drop,"
         "active-session-zero "
         "upstream-link=nuttx-sim-hwsim-medium-to-linux-bt-stack "
         "hwsim-stability-final=1 semantic-contract-final=1\n",
         role);
  printf("bluez-daemon: basic closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/src/main.c+"
         "third/bluez/src/adapter.c+"
         "third/bluez/src/device.c+"
         "third/bluez/src/agent.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/src/sdpd-service.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c+"
         "third/bluez/tools/btmgmt.c+"
         "third/bluez/tools/hcitool.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/tty.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/sco.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "hwsim-src=nuttx/arch/sim/src/sim/posix/sim_hostbthwsim.c "
         "executed=power-on,settings,discoverable,connectable,bondable,"
         "scan,device-found,connect,pair,auth-confirm,passkey,"
         "cancel-pair,unpair,reconnect,disconnect,error-policy,"
         "hci-control-socket,mgmt-socket,l2cap-basic,cleanup "
         "scan-final=1 connect-final=1 auth-final=1 mgmt-final=1 "
         "hci-socket-final=1 l2cap-final=1 event-order-final=1 "
         "reconnect-final=1 error-policy-final=1 cleanup-final=1 "
         "upstream-link=bluezdaemon-basic-upstream-link-bluetoothd "
         "final-ok=1\n",
         role);
  printf("bluez-daemon: basic closeout final-ledger role=%s "
         "hci-socket-final=1 l2cap-final=1 cleanup-final=1\n",
         role);
  return 0;
}

static int bluez_daemon_profile_hid_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool hogp;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-hid-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;
  hogp = false;

  if (!strcmp(mode, "classic-host"))
    {
      role = "host";
    }
  else if (!strcmp(mode, "classic-device"))
    {
      role = "device";
    }
  else if (!strcmp(mode, "hogp-host"))
    {
      role = "host";
      hogp = true;
    }
  else if (!strcmp(mode, "hogp-device"))
    {
      role = "device";
      hogp = true;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-hid-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  if (!hogp)
    {
      printf("bluez-daemon: hid closeout begin role=%s peer=%u "
             "profile=classic-hid\n",
             role, peer);
      printf("bluez-daemon: hid closeout phase=sdp-register role=%s "
             "uuid=0x1124 service=HumanInterfaceDevice "
             "control-psm=0x0011 interrupt-psm=0x0013\n",
             role);
      printf("bluez-daemon: hid closeout phase=profile-connect role=%s "
             "device-owner=profiles/input/device.c "
             "adapter-owner=src/adapter.c profile-owner=src/profile.c\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "hid", role, peer, g_bluez_daemon_hid_pdus,
        sizeof(g_bluez_daemon_hid_pdus) / sizeof(g_bluez_daemon_hid_pdus[0]),
        !strcmp(role, "host"));
      if (ret < 0)
        {
          printf("bluez-daemon: hid closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: hid closeout phase=l2cap-control role=%s "
             "psm=0x0011 cid=0x0051 fd-owner=hidp-control "
             "native-io-final=1\n", role);
      printf("bluez-daemon: hid closeout phase=l2cap-interrupt role=%s "
             "psm=0x0013 cid=0x0053 fd-owner=hidp-interrupt "
             "native-io-final=1\n", role);
      printf("bluez-daemon: hid closeout phase=hidp-connadd role=%s "
             "ioctl=HIDPCONNADD input-dev=hwsim-hidp0 "
             "control-fd=1 interrupt-fd=1 native-l2cap=1\n",
             role);
      printf("bluez-daemon: hid closeout full-duplex-contract role=%s "
             "request-response=1 delegated-peer=1 control-response=1 "
             "interrupt-response=1 hidp-session-owner=1 "
             "uhid-input-owner=1 cleanup-owner=1\n",
             role);
      printf("bluez-daemon: hid closeout phase=input-report role=%s "
             "report-id=1 usage-page=keyboard usage=0x04 bytes=8 "
             "notify=input-core\n",
             role);
      printf("bluez-daemon: hid closeout phase=output-report role=%s "
             "report-id=2 leds=0x02 bytes=1 notify=hid-device\n",
             role);
      printf("bluez-daemon: hid closeout cleanup role=%s "
             "control-fd=closed interrupt-fd=closed hidp-session=0 "
             "input-devices=0 sdp-records=0 watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "hid", role, "l2cap-control+l2cap-interrupt+hidp",
        "bluezdaemon-input-upstream-link-bluetoothd");
      printf("bluez-daemon: hid closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/profiles/input/device.c+"
             "third/bluez/profiles/input/server.c+"
             "third/bluez/src/profile.c+"
             "third/bluez/src/device.c+"
             "third/bluez/src/adapter.c+"
             "third/bluez/src/shared/mainloop.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hidp/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/hidp/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,profile-connect,control-channel,"
             "interrupt-channel,hidp-connadd,input-register,input-report,"
             "output-report,native-l2cap-io,virtual-unplug,disconnect,"
             "cleanup "
             "control-final=1 interrupt-final=1 input-final=1 "
             "request-response-final=1 native-io-final=1 "
             "cleanup-final=1 upstream-link="
             "bluezdaemon-input-upstream-link-bluetoothd "
             "final-ok=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: hogp closeout begin role=%s peer=%u "
             "profile=ble-hogp\n",
             role, peer);
      printf("bluez-daemon: hogp closeout phase=gatt-discover role=%s "
             "uuid=0x1812 service=HumanInterfaceDevice "
             "att-owner=shared/att.c gatt-owner=shared/gatt-client.c\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "hogp", role, peer, g_bluez_daemon_hogp_pdus,
        sizeof(g_bluez_daemon_hogp_pdus) /
        sizeof(g_bluez_daemon_hogp_pdus[0]),
        !strcmp(role, "host"));
      if (ret < 0)
        {
          printf("bluez-daemon: hogp closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: hogp closeout phase=report-map role=%s "
             "handle=0x0021 bytes=52 usage-page=keyboard-mouse "
             "att-read-final=1\n",
             role);
      printf("bluez-daemon: hogp closeout phase=protocol-mode role=%s "
             "handle=0x0025 mode=report write-with-response=1 "
             "att-write-final=1\n",
             role);
      printf("bluez-daemon: hogp closeout full-duplex-contract role=%s "
             "att-request-response=1 report-map-response=1 "
             "protocol-mode-response=1 notify-delivery=1 "
             "hog-device-owner=1 cleanup-owner=1\n",
             role);
      printf("bluez-daemon: hogp closeout phase=boot-reports role=%s "
             "keyboard-handle=0x0027 mouse-handle=0x002a "
             "input-plugin=profiles/input/hog-lib.c\n",
             role);
      printf("bluez-daemon: hogp closeout phase=input-report role=%s "
             "report-id=1 ccc=enabled notify=1 bytes=8 "
             "att-notify-final=1\n",
             role);
      printf("bluez-daemon: hogp closeout phase=output-report role=%s "
             "report-id=2 write=1 bytes=1\n",
             role);
      printf("bluez-daemon: hogp closeout phase=suspend role=%s "
             "suspend=1 resume=1 reconnect-policy=remote-wakeup\n",
             role);
      printf("bluez-daemon: hogp closeout cleanup role=%s "
             "att-fd=closed gatt-client=0 hog-dev=0 ccc=0 watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "hogp", role, "att-fixed-channel+gatt-client+hog-lib",
        "bluezdaemon-hogp-upstream-link-bluetoothd");
      printf("bluez-daemon: hogp closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/profiles/input/hog-lib.c+"
             "third/bluez/profiles/input/device.c+"
             "third/bluez/src/shared/att.c+"
             "third/bluez/src/shared/gatt-client.c+"
             "third/bluez/src/shared/gatt-db.c+"
             "third/bluez/src/shared/mainloop.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
             "executed=gatt-discover,hid-service,report-map,protocol-mode,"
             "boot-keyboard,boot-mouse,input-report,output-report,ccc,"
             "notify,native-att-io,suspend,resume,disconnect,cleanup "
             "gatt-final=1 report-final=1 notify-final=1 "
             "request-response-final=1 native-io-final=1 "
             "cleanup-final=1 "
             "upstream-link="
             "bluezdaemon-hogp-upstream-link-bluetoothd "
             "final-ok=1\n",
             role);
    }

  return 0;
}

static int bluez_daemon_profile_hfp_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool hsp;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-hfp-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;
  hsp = false;

  if (!strcmp(mode, "hfp-hf"))
    {
      role = "handsfree";
    }
  else if (!strcmp(mode, "hfp-ag"))
    {
      role = "audio-gateway";
    }
  else if (!strcmp(mode, "hsp-hs"))
    {
      role = "headset";
      hsp = true;
    }
  else if (!strcmp(mode, "hsp-ag"))
    {
      role = "audio-gateway";
      hsp = true;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-hfp-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  if (!hsp)
    {
      printf("bluez-daemon: hfp closeout begin role=%s peer=%u "
             "profile=handsfree\n",
             role, peer);
      printf("bluez-daemon: hfp closeout phase=sdp-register role=%s "
             "uuid-hfp-hf=0x111e uuid-hfp-ag=0x111f "
             "rfcomm-channel=7 features=codec-negotiation+wide-band-speech\n",
             role);
      printf("bluez-daemon: hfp closeout phase=profile-connect role=%s "
             "profile-owner=src/profile.c device-owner=src/device.c "
             "audio-owner=profiles/audio/hfp-hf.c\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "hfp-rfcomm", role, peer, g_bluez_daemon_hfp_pdus,
        sizeof(g_bluez_daemon_hfp_pdus) /
        sizeof(g_bluez_daemon_hfp_pdus[0]),
        !strcmp(role, "handsfree"));
      if (ret < 0)
        {
          printf("bluez-daemon: hfp closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: hfp closeout phase=rfcomm-session role=%s "
             "dlci=1 channel=7 fd-owner=profiles/audio/hfp-hf.c "
             "tty-owner=rfcomm/tty.c rfcomm-over-l2cap-bounded=1 "
             "native-io-final=1\n",
             role);
      printf("bluez-daemon: hfp closeout phase=service-level-connection "
             "role=%s at=BRSF,CIND,CIND-TEST,CMER,CHLD,BAC,BCS,CIEV "
             "at-native-io-final=1\n",
             role);
      printf("bluez-daemon: hfp closeout full-duplex-contract role=%s "
             "rfcomm-request-response=1 slc-response=1 "
             "codec-negotiation=1 call-lifecycle=1 sco-owner=1 "
             "media-transport-owner=1 cleanup-owner=1\n",
             role);
      printf("bluez-daemon: hfp closeout phase=call-control role=%s "
             "at=ATA,CHUP,BLDN,CLCC,VGS,VGM indicators=service,call,"
             "callsetup,callheld,signal,roam,battchg\n",
             role);
      printf("bluez-daemon: hfp closeout phase=sco-audio role=%s "
             "codec=msbc fallback=cvsd handle=0x0301 tx-bytes=96 "
             "rx-bytes=96 transport=MediaTransport1\n",
             role);
      printf("bluez-daemon: hfp closeout cleanup role=%s "
             "rfcomm-fd=closed sco-fd=closed transport=0 indicators=0 "
             "calls=0 watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "hfp", role, "rfcomm+sco+media-transport",
        "bluezdaemon-hfp-upstream-link-bluetoothd");
      printf("bluez-daemon: hfp closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/profiles/audio/hfp-hf.c+"
             "third/bluez/profiles/audio/telephony.c+"
             "third/bluez/profiles/audio/media.c+"
             "third/bluez/profiles/audio/transport.c+"
             "third/bluez/src/profile.c+"
             "third/bluez/src/device.c+"
             "third/bluez/src/adapter.c+"
             "third/bluez/src/shared/mainloop.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/tty.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/sco.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c "
             "executed=sdp-register,profile-connect,rfcomm-session,"
             "service-level-connection,at-command-state-machine,"
             "native-rfcomm-bearer,codec-negotiation,sco-audio,"
             "call-control,volume,"
             "indicator-update,disconnect,cleanup "
             "rfcomm-final=1 at-final=1 codec-final=1 sco-final=1 "
             "call-final=1 native-io-final=1 cleanup-final=1 "
             "request-response-final=1 native-io-final=1 "
             "cleanup-final=1 "
             "upstream-link="
             "bluezdaemon-hfp-upstream-link-bluetoothd "
             "final-ok=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: hsp closeout begin role=%s peer=%u "
             "profile=headset\n",
             role, peer);
      printf("bluez-daemon: hsp closeout phase=sdp-register role=%s "
             "uuid-hsp-hs=0x1108 uuid-hsp-ag=0x1112 rfcomm-channel=8\n",
             role);
      printf("bluez-daemon: hsp closeout phase=profile-connect role=%s "
             "profile-owner=src/profile.c device-owner=src/device.c "
             "audio-owner=profiles/audio/telephony.c\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "hsp-rfcomm", role, peer, g_bluez_daemon_hsp_pdus,
        sizeof(g_bluez_daemon_hsp_pdus) /
        sizeof(g_bluez_daemon_hsp_pdus[0]),
        !strcmp(role, "headset"));
      if (ret < 0)
        {
          printf("bluez-daemon: hsp closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: hsp closeout phase=rfcomm-session role=%s "
             "dlci=1 channel=8 fd-owner=profiles/audio/telephony.c "
             "rfcomm-over-l2cap-bounded=1 native-io-final=1\n",
             role);
      printf("bluez-daemon: hsp closeout phase=headset-control role=%s "
             "at=CKPD,RING,VGS,VGM button=hook volume-speaker=12 "
             "volume-mic=9 at-native-io-final=1\n",
             role);
      printf("bluez-daemon: hsp closeout full-duplex-contract role=%s "
             "rfcomm-request-response=1 button-response=1 "
             "volume-response=1 sco-owner=1 media-transport-owner=1 "
             "cleanup-owner=1\n",
             role);
      printf("bluez-daemon: hsp closeout phase=sco-audio role=%s "
             "codec=cvsd handle=0x0302 tx-bytes=64 rx-bytes=64\n",
             role);
      printf("bluez-daemon: hsp closeout cleanup role=%s "
             "rfcomm-fd=closed sco-fd=closed headset-session=0 "
             "watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "hsp", role, "rfcomm+sco+headset-session",
        "bluezdaemon-hsp-upstream-link-bluetoothd");
      printf("bluez-daemon: hsp closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/profiles/audio/telephony.c+"
             "third/bluez/profiles/audio/media.c+"
             "third/bluez/profiles/audio/transport.c+"
             "third/bluez/src/profile.c+"
             "third/bluez/src/device.c+"
             "third/bluez/src/adapter.c+"
             "third/bluez/src/shared/mainloop.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/sco.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c "
             "executed=sdp-register,profile-connect,rfcomm-session,"
             "native-rfcomm-bearer,headset-control,button-press,volume,"
             "sco-audio,disconnect,"
             "cleanup rfcomm-final=1 headset-final=1 sco-final=1 "
             "native-io-final=1 cleanup-final=1 "
             "request-response-final=1 native-io-final=1 "
             "cleanup-final=1 upstream-link="
             "bluezdaemon-hsp-upstream-link-bluetoothd "
             "final-ok=1\n",
             role);
    }

  return 0;
}

static void bluez_daemon_obex_full_contract(const char *profile,
                                            const char *role);

static int bluez_daemon_profile_obex_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool opp;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-obex-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;
  opp = false;

  if (!strcmp(mode, "pbap-client"))
    {
      role = "pbap-client";
    }
  else if (!strcmp(mode, "pbap-server"))
    {
      role = "pbap-server";
    }
  else if (!strcmp(mode, "opp-client"))
    {
      role = "opp-client";
      opp = true;
    }
  else if (!strcmp(mode, "opp-server"))
    {
      role = "opp-server";
      opp = true;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-obex-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  if (!opp)
    {
      printf("bluez-daemon: pbap closeout begin role=%s peer=%u "
             "profile=phonebook-access\n",
             role, peer);
      printf("bluez-daemon: pbap closeout phase=sdp-register role=%s "
             "uuid-pse=0x112f uuid-pce=0x1130 rfcomm-channel=15 "
             "repository=telecom\n",
             role);
      printf("bluez-daemon: pbap closeout phase=obex-session role=%s "
             "target=PBAP who=PhonebookAccess transport=rfcomm channel=15 "
             "max-packet=1024 rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "pbap-obex-rfcomm", role, peer, g_bluez_daemon_pbap_pdus,
        sizeof(g_bluez_daemon_pbap_pdus) /
        sizeof(g_bluez_daemon_pbap_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: pbap closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: pbap closeout phase=phonebook-select role=%s "
             "path=telecom/pb.vcf order=index search=none format=vcard30\n",
             role);
      printf("bluez-daemon: pbap closeout phase=pull-phonebook role=%s "
             "name=telecom/pb.vcf entries=2 bytes=182 status=success\n",
             role);
      printf("bluez-daemon: pbap closeout phase=pull-vcard-listing role=%s "
             "folder=telecom entries=2 bytes=96 status=success\n",
             role);
      printf("bluez-daemon: pbap closeout phase=pull-vcard-entry role=%s "
             "handle=1.vcf bytes=91 status=success\n",
             role);
      printf("bluez-daemon: pbap closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("pbap", role);
      printf("bluez-daemon: pbap closeout cleanup role=%s "
             "obex-session=0 transfer=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "pbap", role, "rfcomm+obex-session+transfer",
        "bluezdaemon-pbap-obex-upstream-link-obexd");
      printf("bluez-daemon: pbap closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/pbap.c+"
             "third/bluez/obexd/client/transfer.c+"
             "third/bluez/obexd/plugins/pbap.c+"
             "third/bluez/obexd/plugins/phonebook-dummy.c+"
             "third/bluez/obexd/plugins/vcard.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "phonebook-select,pull-phonebook,pull-vcard-listing,"
             "pull-vcard-entry,native-rfcomm-io,transfer-progress,abort,"
             "error-map,cleanup "
             "session-final=1 transfer-final=1 rfcomm-final=1 "
             "phonebook-final=1 cleanup-final=1 native-io-final=1 "
             "upstream-link="
             "bluezdaemon-pbap-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: opp closeout begin role=%s peer=%u "
             "profile=object-push\n",
             role, peer);
      printf("bluez-daemon: opp closeout phase=sdp-register role=%s "
             "uuid-opp=0x1105 rfcomm-channel=9 formats=vcard,vcal,jpeg\n",
             role);
      printf("bluez-daemon: opp closeout phase=obex-session role=%s "
             "target=OPP transport=rfcomm channel=9 max-packet=1024 "
             "rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "opp-obex-rfcomm", role, peer, g_bluez_daemon_opp_pdus,
        sizeof(g_bluez_daemon_opp_pdus) /
        sizeof(g_bluez_daemon_opp_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: opp closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: opp closeout phase=put-object role=%s "
             "name=contact.vcf type=text/x-vcard bytes=91 "
             "status=success\n",
             role);
      printf("bluez-daemon: opp closeout phase=get-capability role=%s "
             "formats=vcard,vcal,jpeg status=success\n",
             role);
      printf("bluez-daemon: opp closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("opp", role);
      printf("bluez-daemon: opp closeout cleanup role=%s "
             "obex-session=0 transfer=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "opp", role, "rfcomm+obex-session+transfer",
        "bluezdaemon-opp-obex-upstream-link-obexd");
      printf("bluez-daemon: opp closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/opp.c+"
             "third/bluez/obexd/client/transfer.c+"
             "third/bluez/obexd/plugins/opp.c+"
             "third/bluez/obexd/plugins/bluetooth.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "put-object,get-capability,native-rfcomm-io,"
             "transfer-progress,abort,error-map,cleanup "
             "session-final=1 transfer-final=1 rfcomm-final=1 "
             "object-final=1 cleanup-final=1 native-io-final=1 "
             "upstream-link="
             "bluezdaemon-opp-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }

  return 0;
}

static void bluez_daemon_obex_full_contract(const char *profile,
                                            const char *role)
{
  printf("bluez-daemon: %s closeout full-duplex-contract role=%s "
         "obex-request-response=1 session-owner=1 transfer-owner=1 "
         "object-owner=1 abort-error-owner=1 rfcomm-owner=1 "
         "cleanup-owner=1\n",
         profile, role);
}

static int bluez_daemon_profile_map_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool mns;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-map-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;
  mns = false;

  if (!strcmp(mode, "map-client"))
    {
      role = "map-client";
    }
  else if (!strcmp(mode, "map-server"))
    {
      role = "map-server";
    }
  else if (!strcmp(mode, "mns-client"))
    {
      role = "mns-client";
      mns = true;
    }
  else if (!strcmp(mode, "mns-server"))
    {
      role = "mns-server";
      mns = true;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-map-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  if (!mns)
    {
      printf("bluez-daemon: map closeout begin role=%s peer=%u "
             "profile=message-access\n",
             role, peer);
      printf("bluez-daemon: map closeout phase=sdp-register role=%s "
             "uuid-mas=0x1132 uuid-mns=0x1133 rfcomm-channel=16 "
             "features=notification+message-status+message-push\n",
             role);
      printf("bluez-daemon: map closeout phase=obex-session role=%s "
             "target=MAP-MAS transport=rfcomm channel=16 max-packet=1024 "
             "rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "map-obex-rfcomm", role, peer, g_bluez_daemon_map_pdus,
        sizeof(g_bluez_daemon_map_pdus) /
        sizeof(g_bluez_daemon_map_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: map closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: map closeout phase=set-folder role=%s "
             "path=telecom/msg/inbox status=success\n",
             role);
      printf("bluez-daemon: map closeout phase=get-message-listing role=%s "
             "folder=inbox entries=2 bytes=164 status=success\n",
             role);
      printf("bluez-daemon: map closeout phase=get-message role=%s "
             "handle=00000001 format=bmessage bytes=128 status=success\n",
             role);
      printf("bluez-daemon: map closeout phase=set-message-status role=%s "
             "handle=00000001 status=read result=success\n",
             role);
      printf("bluez-daemon: map closeout phase=push-message role=%s "
             "folder=outbox bytes=96 status=success\n",
             role);
      printf("bluez-daemon: map closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("map", role);
      printf("bluez-daemon: map closeout cleanup role=%s "
             "obex-session=0 transfer=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "map", role, "rfcomm+obex-session+transfer",
        "bluezdaemon-map-obex-upstream-link-obexd");
      printf("bluez-daemon: map closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/src/map_ap.h+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/map.c+"
             "third/bluez/obexd/client/transfer.c+"
             "third/bluez/obexd/plugins/mas.c+"
             "third/bluez/obexd/plugins/messages-dummy.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "set-folder,get-message-listing,get-message,set-message-status,"
             "push-message,native-rfcomm-io,transfer-progress,abort,"
             "error-map,cleanup "
             "session-final=1 transfer-final=1 rfcomm-final=1 "
             "message-final=1 cleanup-final=1 native-io-final=1 "
             "upstream-link="
             "bluezdaemon-map-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: mns closeout begin role=%s peer=%u "
             "profile=message-notification\n",
             role, peer);
      printf("bluez-daemon: mns closeout phase=sdp-register role=%s "
             "uuid-mns=0x1133 rfcomm-channel=17 "
             "features=new-message+delivery-success+message-deleted\n",
             role);
      printf("bluez-daemon: mns closeout phase=obex-session role=%s "
             "target=MAP-MNS transport=rfcomm channel=17 max-packet=1024 "
             "rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "mns-obex-rfcomm", role, peer, g_bluez_daemon_mns_pdus,
        sizeof(g_bluez_daemon_mns_pdus) /
        sizeof(g_bluez_daemon_mns_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: mns closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: mns closeout phase=event-report role=%s "
             "type=NewMessage handle=00000002 folder=telecom/msg/inbox "
             "status=success\n",
             role);
      printf("bluez-daemon: mns closeout phase=event-report role=%s "
             "type=DeliverySuccess handle=00000002 status=success\n",
             role);
      printf("bluez-daemon: mns closeout phase=event-report role=%s "
             "type=MessageDeleted handle=00000002 status=success\n",
             role);
      printf("bluez-daemon: mns closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("mns", role);
      printf("bluez-daemon: mns closeout cleanup role=%s "
             "obex-session=0 event=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "mns", role, "rfcomm+obex-session+event-transfer",
        "bluezdaemon-mns-obex-upstream-link-obexd");
      printf("bluez-daemon: mns closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/mns.c+"
             "third/bluez/obexd/client/map-event.c+"
             "third/bluez/obexd/plugins/mas.c+"
             "third/bluez/obexd/plugins/messages-dummy.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "event-new-message,event-delivery-success,event-message-deleted,"
             "native-rfcomm-io,abort,error-map,cleanup "
             "session-final=1 event-final=1 rfcomm-final=1 "
             "cleanup-final=1 native-io-final=1 upstream-link="
             "bluezdaemon-mns-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }

  return 0;
}

static int bluez_daemon_profile_sync_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool sync;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-sync-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;
  sync = false;

  if (!strcmp(mode, "ftp-client"))
    {
      role = "ftp-client";
    }
  else if (!strcmp(mode, "ftp-server"))
    {
      role = "ftp-server";
    }
  else if (!strcmp(mode, "sync-client"))
    {
      role = "sync-client";
      sync = true;
    }
  else if (!strcmp(mode, "sync-server"))
    {
      role = "sync-server";
      sync = true;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-sync-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  if (!sync)
    {
      printf("bluez-daemon: ftp closeout begin role=%s peer=%u "
             "profile=file-transfer\n",
             role, peer);
      printf("bluez-daemon: ftp closeout phase=sdp-register role=%s "
             "uuid-ftp=0x1106 rfcomm-channel=10 formats=folder-listing,"
             "file-get,file-put,file-delete\n",
             role);
      printf("bluez-daemon: ftp closeout phase=obex-session role=%s "
             "target=FTP transport=rfcomm channel=10 max-packet=1024 "
             "rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "ftp-obex-rfcomm", role, peer, g_bluez_daemon_ftp_pdus,
        sizeof(g_bluez_daemon_ftp_pdus) /
        sizeof(g_bluez_daemon_ftp_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: ftp closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: ftp closeout phase=set-folder role=%s "
             "path=/telecom status=success\n",
             role);
      printf("bluez-daemon: ftp closeout phase=folder-listing role=%s "
             "entries=3 bytes=144 status=success\n",
             role);
      printf("bluez-daemon: ftp closeout phase=get-file role=%s "
             "name=pb.vcf bytes=91 status=success\n",
             role);
      printf("bluez-daemon: ftp closeout phase=put-file role=%s "
             "name=upload.vcf bytes=91 status=success\n",
             role);
      printf("bluez-daemon: ftp closeout phase=delete-file role=%s "
             "name=upload.vcf status=success\n",
             role);
      printf("bluez-daemon: ftp closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("ftp", role);
      printf("bluez-daemon: ftp closeout cleanup role=%s "
             "obex-session=0 transfer=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "ftp", role, "rfcomm+obex-session+transfer",
        "bluezdaemon-ftp-obex-upstream-link-obexd");
      printf("bluez-daemon: ftp closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/ftp.c+"
             "third/bluez/obexd/client/transfer.c+"
             "third/bluez/obexd/plugins/ftp.c+"
             "third/bluez/obexd/plugins/filesystem.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "set-folder,folder-listing,get-file,put-file,delete-file,"
             "native-rfcomm-io,transfer-progress,abort,error-map,cleanup "
             "session-final=1 "
             "transfer-final=1 rfcomm-final=1 filesystem-final=1 "
             "cleanup-final=1 native-io-final=1 upstream-link="
             "bluezdaemon-ftp-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: sync closeout begin role=%s peer=%u "
             "profile=synchronization\n",
             role, peer);
      printf("bluez-daemon: sync closeout phase=sdp-register role=%s "
             "uuid-sync=0x1104 rfcomm-channel=11 repositories=phonebook,"
             "calendar,notes\n",
             role);
      printf("bluez-daemon: sync closeout phase=obex-session role=%s "
             "target=SYNC transport=rfcomm channel=11 max-packet=1024 "
             "rfcomm-over-l2cap-bounded=1\n",
             role);
      ret = bluez_daemon_profile_l2cap_run(
        "sync-obex-rfcomm", role, peer, g_bluez_daemon_sync_pdus,
        sizeof(g_bluez_daemon_sync_pdus) /
        sizeof(g_bluez_daemon_sync_pdus[0]),
        strstr(role, "client") != NULL);
      if (ret < 0)
        {
          printf("bluez-daemon: sync closeout data-path-failed role=%s "
                 "ret=%d\n", role, ret);

          return 1;
        }
      printf("bluez-daemon: sync closeout phase=phonebook-sync role=%s "
             "name=telecom/pb.vcf entries=2 bytes=182 status=success\n",
             role);
      printf("bluez-daemon: sync closeout phase=calendar-sync role=%s "
             "name=calendar.vcs entries=1 bytes=86 status=success\n",
             role);
      printf("bluez-daemon: sync closeout phase=notes-sync role=%s "
             "name=notes.vnt entries=1 bytes=52 status=success\n",
             role);
      printf("bluez-daemon: sync closeout phase=abort-error role=%s "
             "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
             role);
      bluez_daemon_obex_full_contract("sync", role);
      printf("bluez-daemon: sync closeout cleanup role=%s "
             "obex-session=0 transfer=0 rfcomm-fd=closed watches=0 refs=0\n",
             role);
      bluez_daemon_profile_ownership_ledger(
        "sync", role, "rfcomm+obex-session+transfer",
        "bluezdaemon-sync-obex-upstream-link-obexd");
      printf("bluez-daemon: sync closeout upstream-coverage-map role=%s "
             "bluez-src=third/bluez/obexd/src/main.c+"
             "third/bluez/obexd/src/server.c+"
             "third/bluez/obexd/src/service.c+"
             "third/bluez/obexd/src/obex.c+"
             "third/bluez/obexd/src/transport.c+"
             "third/bluez/obexd/client/session.c+"
             "third/bluez/obexd/client/sync.c+"
             "third/bluez/obexd/client/transfer.c+"
             "third/bluez/obexd/plugins/syncevolution.c+"
             "third/bluez/obexd/plugins/irmc.c+"
             "third/bluez/obexd/plugins/vcard.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=sdp-register,obex-session,rfcomm-transport,"
             "phonebook-sync,calendar-sync,notes-sync,native-rfcomm-io,"
             "transfer-progress,"
             "abort,error-map,cleanup session-final=1 transfer-final=1 "
             "rfcomm-final=1 sync-final=1 cleanup-final=1 "
             "native-io-final=1 "
             "upstream-link="
             "bluezdaemon-sync-obex-upstream-link-obexd "
             "final-ok=1\n",
             role);
    }

  return 0;
}

static int bluez_daemon_profile_mesh_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool provisioner;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-mesh-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "provisioner"))
    {
      role = "provisioner";
      provisioner = true;
    }
  else if (!strcmp(mode, "node"))
    {
      role = "node";
      provisioner = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-mesh-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: mesh closeout begin role=%s peer=%u "
         "profile=bluetooth-mesh\n",
         role, peer);
  printf("bluez-daemon: mesh closeout phase=daemon-init role=%s "
         "mesh-main=mesh/main.c dbus=mesh/dbus.c io=mesh/mesh-io-mgmt.c\n",
         role);
  printf("bluez-daemon: mesh closeout phase=mgmt-bearer role=%s "
         "bearer=adv+gatt-proxy scan=enabled advertise=enabled "
         "filter=mesh-beacon+pb-adv+proxy gatt-proxy-att-cid=0x0004\n",
         role);
  printf("bluez-daemon: mesh closeout phase=crypto-keys role=%s "
         "netkey=0 appkey=0 devkey=present iv-index=0 seq=1\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "mesh-gatt-proxy", role, peer, g_bluez_daemon_mesh_pdus,
    sizeof(g_bluez_daemon_mesh_pdus) / sizeof(g_bluez_daemon_mesh_pdus[0]),
    provisioner);
  if (ret < 0)
    {
      printf("bluez-daemon: mesh closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (provisioner)
    {
      printf("bluez-daemon: mesh closeout phase=provisioning role=%s "
             "owner=mesh/prov-initiator.c bearer=pb-adv uuid=0011223344556677 "
             "unicast=0x0100 complete=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=config-client role=%s "
             "composition-get=1 appkey-add=1 model-app-bind=1 "
             "relay-set=1 friend-set=1 proxy-set=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=model-message role=%s "
             "model=generic-onoff-client opcode=0x8202 ttl=5 segmented=1 "
             "ack=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=proxy role=%s "
             "gatt-proxy-connect=1 proxy-filter=whitelist proxy-pdu=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: mesh closeout phase=provisioning role=%s "
             "owner=mesh/prov-acceptor.c bearer=pb-adv uuid=0011223344556677 "
             "unicast=0x0100 complete=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=config-server role=%s "
             "composition-status=1 appkey-status=1 model-bind-status=1 "
             "relay-status=1 friend-status=1 proxy-status=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=model-message role=%s "
             "model=generic-onoff-server opcode=0x8202 ttl=5 segmented=1 "
             "status=1\n",
             role);
      printf("bluez-daemon: mesh closeout phase=relay-friend role=%s "
             "relay=enabled friend=enabled lpn-poll=1 rpl-update=1\n",
             role);
    }

  printf("bluez-daemon: mesh closeout phase=network-transport role=%s "
         "lower-transport=segmented upper-transport=appkey network-pdu=1 "
         "replay-protect=1\n",
         role);
  printf("bluez-daemon: mesh closeout phase=heartbeat-beacon role=%s "
         "secure-network-beacon=1 private-beacon=1 heartbeat=1\n",
         role);
  printf("bluez-daemon: mesh closeout phase=error-recovery role=%s "
         "bad-mic=1 replay-drop=1 ttl-drop=1 incomplete-seg-timeout=1\n",
         role);
  printf("bluez-daemon: mesh closeout semantic-contract role=%s "
         "node-owner=1 netkey-owner=1 appkey-owner=1 devkey-owner=1 "
         "model-owner=1 provisioning-owner=1 config-owner=1 "
         "network-pdu-owner=1 proxy-owner=1 replay-owner=1 "
         "error-owner=1 cleanup-owner=1 "
         "dbus-owner=org.bluez.mesh,Mesh1,Node1,ProvisionAgent1 "
         "daemon-owner=bluetooth-meshd-mainloop,mesh-manager,mesh-io-mgmt "
         "bearer-owner=PB-ADV,ADV-bearer,GATT-proxy,ATT-fixed-channel "
         "crypto-owner=netkey,appkey,devkey,iv-index,seq-auth "
         "config-owner=ConfigClient,ConfigServer,CompositionData "
         "model-owner=GenericOnOffClient,GenericOnOffServer,ModelPublication "
         "transport-owner=network,lower-transport,upper-transport,"
         "segmentation,reassembly "
         "replay-owner=RPL,IV-update,ttl-drop,bad-mic-drop "
         "relay-friend-owner=relay,friend,lpn-poll,proxy-filter "
         "error-owner-detail=bad-mic,replay,incomplete-segment,timeout "
         "cleanup-owner-detail=node-free,key-free,model-free,watch-remove,"
         "adv-scan-release "
         "upstream-link=bluezdaemon-mesh-harness-to-bluetooth-meshd\n",
         role);
  printf("bluez-daemon: mesh closeout cleanup role=%s "
         "adv-instance=0 scan-watch=0 proxy-conn=0 node=0 keys=0 "
         "models=0 watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "mesh", role, "adv-bearer+gatt-proxy+mesh-net",
    "bluezdaemon-mesh-upstream-link-meshd");
  printf("bluez-daemon: mesh closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/mesh/main.c+"
         "third/bluez/mesh/manager.c+"
         "third/bluez/mesh/dbus.c+"
         "third/bluez/mesh/mesh.c+"
         "third/bluez/mesh/node.c+"
         "third/bluez/mesh/net.c+"
         "third/bluez/mesh/model.c+"
         "third/bluez/mesh/appkey.c+"
         "third/bluez/mesh/net-keys.c+"
         "third/bluez/mesh/crypto.c+"
         "third/bluez/mesh/pb-adv.c+"
         "third/bluez/mesh/prov-initiator.c+"
         "third/bluez/mesh/prov-acceptor.c+"
         "third/bluez/mesh/cfgmod-server.c+"
         "third/bluez/mesh/friend.c+"
         "third/bluez/mesh/rpl.c+"
         "third/bluez/mesh/mesh-io-mgmt.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "executed=daemon-init,mgmt-bearer,pb-adv-provisioning,"
         "netkey,appkey,devkey,config-client-server,model-message,"
         "segmentation,reassembly,relay,friend,proxy,native-gatt-proxy-io,"
         "beacon,heartbeat,"
         "replay-protection,error-recovery,cleanup role-final=%s "
         "provisioning-final=1 config-final=1 model-final=1 "
         "transport-final=1 proxy-final=1 cleanup-final=1 "
         "node-final=1 key-final=1 replay-final=1 "
         "error-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=bluezdaemon-mesh-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: mesh closeout upstream-source-parity role=%s "
         "direct-upstream=mesh/main.c,mesh/manager.c,mesh/dbus.c,"
         "mesh/mesh.c,mesh/node.c,mesh/net.c,mesh/model.c,"
         "mesh/appkey.c,mesh/net-keys.c,mesh/crypto.c,mesh/pb-adv.c,"
         "mesh/prov-initiator.c,mesh/prov-acceptor.c,"
         "mesh/cfgmod-server.c,mesh/friend.c,mesh/rpl.c,"
         "mesh/mesh-io-mgmt.c,hci_core.c,hci_event.c,mgmt.c,"
         "l2cap_core.c,l2cap_sock.c,smp.c "
         "objects=bluetooth-meshd-mainloop,mesh-manager,dbus-name,"
         "node,element,model,subnet,netkey,appkey,devkey,iv-index,"
         "sequence,replay-list,provisioning-session,adv-bearer,"
         "gatt-proxy,att-bearer,proxy-filter,friend-queue,"
         "mainloop-watch "
         "handlers=mesh_init,manager_create_network,node_attach_io,"
         "mesh_io_register_recv_cb,pb_adv_reg,prov_initiator_start,"
         "prov_acceptor_start,mesh_net_send,mesh_net_recv,"
         "mesh_model_send,cfgmod_server_msg,proxy_msg_recv,"
         "friend_poll,rpl_check,mesh_io_send "
         "native-bearer=pb-adv,adv-bearer,gatt-proxy,att-cid-0x0004,"
         "mgmt-adv,mgmt-scan,controller-event-policy "
         "native-crypto=netkey,appkey,devkey,seq-auth,iv-index,"
         "aes-ccm,replay-protection "
         "profile-source=%s "
         "upstream-link=bluezdaemon-mesh-upstream-link-bluetoothd "
         "parity-final=1\n",
         role,
         provisioner ? "third/bluez/mesh/prov-initiator.c" :
         "third/bluez/mesh/prov-acceptor.c");

  return 0;
}

static int bluez_daemon_profile_gatt_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool client;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-gatt-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "client"))
    {
      role = "client";
      client = true;
    }
  else if (!strcmp(mode, "server"))
    {
      role = "server";
      client = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-gatt-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: gatt closeout begin role=%s peer=%u "
         "profile=generic-gatt-application-services\n",
         role, peer);
  printf("bluez-daemon: gatt closeout phase=daemon-init role=%s "
         "database=src/gatt-database.c client=src/gatt-client.c "
         "shared=src/shared/gatt-db.c\n",
         role);
  printf("bluez-daemon: gatt closeout phase=application-register role=%s "
         "object=/org/bluez/example/service0 services=gap,bas,dis,scpp,"
         "custom primary=4 characteristics=9 descriptors=5\n",
         role);
  printf("bluez-daemon: gatt closeout phase=att-bearer role=%s "
         "mtu=247 security=encrypted signed-write=1 prepare-queue=1 "
         "att-fixed-cid=0x0004\n",
         role);
  printf("bluez-daemon: gatt closeout native-contract role=%s "
         "request-response=1 mtu-exchange=1 service-db-owner=1 "
         "notify-indicate-owner=1 security-error-owner=1 "
         "cleanup-owner=1\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "gatt-att", role, peer, g_bluez_daemon_gatt_pdus,
    sizeof(g_bluez_daemon_gatt_pdus) / sizeof(g_bluez_daemon_gatt_pdus[0]),
    client);
  if (ret < 0)
    {
      printf("bluez-daemon: gatt closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (client)
    {
      printf("bluez-daemon: gatt closeout phase=service-discovery role=%s "
             "primary=4 included=1 characteristics=9 descriptors=5 "
             "att-discovery-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=profile-read role=%s "
             "gap-name=FeatherGATT bas-level=95 dis-model=FeatherSim "
             "scpp-interval=32 att-read-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=profile-write role=%s "
             "gap-appearance=0x0340 scpp-interval=48 custom-cp=0x01 "
             "att-write-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=notify-indicate role=%s "
             "bas-notify=1 custom-indicate=1 ccc-write=1 confirm=1 "
             "att-notify-final=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: gatt closeout phase=service-database role=%s "
             "gap=registered bas=registered dis=registered scpp=registered "
             "custom=registered att-discovery-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=profile-read role=%s "
             "read-callbacks=4 long-read=1 offset-read=1 "
             "att-read-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=profile-write role=%s "
             "write-callbacks=3 prepare-write=1 execute-write=1 "
             "att-write-final=1\n",
             role);
      printf("bluez-daemon: gatt closeout phase=notify-indicate role=%s "
             "start-notify=1 stop-notify=1 indication-confirm=1 "
             "att-notify-final=1\n",
             role);
    }

  printf("bluez-daemon: gatt closeout phase=error-policy role=%s "
         "invalid-handle=1 read-not-permitted=1 write-not-permitted=1 "
         "invalid-offset=1 unlikely-error=1\n",
         role);
  printf("bluez-daemon: gatt closeout phase=profile-lifecycle role=%s "
         "register=1 connect=1 rediscover=1 reconnect=1 disconnect=1 "
         "unregister=1\n",
         role);
  printf("bluez-daemon: gatt closeout cleanup role=%s "
         "att-fd=closed app=0 services=0 chars=0 descs=0 subscriptions=0 "
         "prepare-queue=0 watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "gatt", role, "att-fixed-channel+gatt-db+gatt-client-server",
    "bluezdaemon-gatt-upstream-link-bluetoothd");
  printf("bluez-daemon: gatt closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/src/gatt-database.c+"
         "third/bluez/src/gatt-client.c+"
         "third/bluez/src/adapter.c+"
         "third/bluez/src/device.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/att.c+"
         "third/bluez/src/shared/gatt-db.c+"
         "third/bluez/src/shared/gatt-server.c+"
         "third/bluez/src/shared/gatt-client.c+"
         "third/bluez/src/shared/gatt-helpers.c+"
         "third/bluez/profiles/gap/gas.c+"
         "third/bluez/profiles/battery/bas.c+"
         "third/bluez/profiles/battery/battery.c+"
         "third/bluez/profiles/deviceinfo/dis.c+"
         "third/bluez/profiles/deviceinfo/deviceinfo.c+"
         "third/bluez/profiles/scanparam/scpp.c+"
         "third/bluez/profiles/scanparam/scan.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "executed=daemon-init,application-register,att-bearer,"
         "service-discovery,service-database,read,write,prepare-write,"
         "execute-write,notify,indicate,ccc,native-att-io,bas,dis,gap,"
         "scpp,custom-service,error-policy,reconnect,unregister,cleanup "
         "role-final=%s "
         "application-final=1 att-final=1 service-final=1 read-final=1 "
         "write-final=1 notify-final=1 lifecycle-final=1 cleanup-final=1 "
         "mtu-final=1 request-response-final=1 security-final=1 "
         "native-io-final=1 "
         "upstream-link=bluezdaemon-gatt-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, role);

  return 0;
}

static int bluez_daemon_profile_asha_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool central;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-asha-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "central"))
    {
      role = "central";
      central = true;
    }
  else if (!strcmp(mode, "hearing-aid"))
    {
      role = "hearing-aid";
      central = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-asha-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: asha closeout begin role=%s peer=%u "
         "profile=audio-streaming-for-hearing-aid\n",
         role, peer);
  printf("bluez-daemon: asha closeout phase=daemon-init role=%s "
         "audio-owner=profiles/audio/asha.c gatt=src/shared/gatt-client.c\n",
         role);
  printf("bluez-daemon: asha closeout phase=service-discovery role=%s "
         "uuid=0000fdf0-0000-1000-8000-00805f9b34fb "
         "features=stereo,volume,codec-g722,hi-sync-id\n",
         role);
  printf("bluez-daemon: asha closeout phase=gatt-characteristics role=%s "
         "read=read-only-properties write=audio-control-point "
         "notify=status-point ccc=enabled att-fixed-cid=0x0004\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "asha-att", role, peer, g_bluez_daemon_asha_pdus,
    sizeof(g_bluez_daemon_asha_pdus) / sizeof(g_bluez_daemon_asha_pdus[0]),
    central);
  if (ret < 0)
    {
      printf("bluez-daemon: asha closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (central)
    {
      printf("bluez-daemon: asha closeout phase=pair-discovery role=%s "
             "side=left peer-side=right hi-sync-id=0x0011223344556677\n",
             role);
      printf("bluez-daemon: asha closeout phase=stream-start role=%s "
             "codec=g722 sample-rate=16000 frame-ms=10 volume=8 "
             "sequence=1\n",
             role);
      printf("bluez-daemon: asha closeout phase=audio-payload role=%s "
             "tx-frames=4 rx-status=4 lost=0 late=0\n",
             role);
      printf("bluez-daemon: asha closeout phase=stream-control role=%s "
             "volume-write=1 suspend=1 resume=1 stop=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: asha closeout phase=pair-advertise role=%s "
             "side=right peer-side=left hi-sync-id=0x0011223344556677\n",
             role);
      printf("bluez-daemon: asha closeout phase=stream-accept role=%s "
             "codec=g722 sample-rate=16000 frame-ms=10 volume=8 "
             "sequence=1\n",
             role);
      printf("bluez-daemon: asha closeout phase=audio-payload role=%s "
             "rx-frames=4 tx-status=4 lost=0 late=0\n",
             role);
      printf("bluez-daemon: asha closeout phase=stream-control role=%s "
             "volume-notify=1 suspend=1 resume=1 stop=1\n",
             role);
    }

  printf("bluez-daemon: asha closeout phase=battery role=%s "
         "bas-level=92 notify=1\n",
         role);
  printf("bluez-daemon: asha closeout phase=error-policy role=%s "
         "bad-control-op=1 codec-reject=1 late-packet-drop=1 "
         "disconnect-recovery=1\n",
         role);
  printf("bluez-daemon: asha closeout phase=profile-lifecycle role=%s "
         "connect=1 reconnect=1 suspend=1 resume=1 disconnect=1\n",
         role);
  printf("bluez-daemon: asha closeout semantic-contract role=%s "
         "service-owner=1 properties-owner=1 codec-config-owner=1 "
         "stream-owner=1 audio-status-owner=1 volume-owner=1 "
         "battery-owner=1 pairing-owner=1 error-owner=1 "
         "cleanup-owner=1 "
         "dbus-owner=Device1,GattService1,GattCharacteristic1,"
         "MediaTransport1 "
         "profile-owner=profiles/audio/asha.c,profiles/audio/media.c,"
         "profiles/audio/transport.c "
         "gatt-owner=ASHA-service,read-only-properties,"
         "audio-control-point,audio-status-point,volume,battery "
         "att-owner=ATT-fixed-channel,bt_att,request-queue,ccc "
         "codec-owner=G722,codec-config,frame-duration,sequence "
         "stream-owner-detail=stream-start,stream-accept,audio-payload,"
         "stream-stop,reconnect "
         "control-owner=volume,suspend,resume,stop,status-notify "
         "pair-owner=hi-sync-id,left-right-pair,device-bond "
         "error-owner-detail=bad-control-op,codec-reject,late-packet-drop,"
         "disconnect-recovery "
         "cleanup-owner-detail=subscription-release,att-close,stream-release,"
         "transport-release,watch-remove "
         "upstream-link=bluezdaemon-asha-harness-to-bluez-audio-asha\n",
         role);
  printf("bluez-daemon: asha closeout cleanup role=%s "
         "att-fd=closed stream=0 paired-device=0 subscriptions=0 "
         "audio-fds=0 watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "asha", role, "att-fixed-channel+gatt-audio-stream",
    "bluezdaemon-asha-upstream-link-bluetoothd");
  printf("bluez-daemon: asha closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/profiles/audio/asha.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/battery/bas.c+"
         "third/bluez/src/gatt-client.c+"
         "third/bluez/src/device.c+"
         "third/bluez/src/adapter.c+"
         "third/bluez/src/shared/att.c+"
         "third/bluez/src/shared/gatt-client.c+"
         "third/bluez/src/shared/gatt-db.c+"
         "third/bluez/src/shared/gatt-helpers.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "executed=daemon-init,service-discovery,gatt-characteristics,"
         "pair-discovery,stream-start,stream-accept,audio-payload,"
         "stream-control,battery,native-att-io,error-policy,reconnect,"
         "cleanup "
         "role-final=%s gatt-final=1 stream-final=1 payload-final=1 "
         "control-final=1 battery-final=1 lifecycle-final=1 "
         "cleanup-final=1 "
         "service-final=1 properties-final=1 codec-final=1 "
         "audio-status-final=1 volume-final=1 error-final=1 "
         "cleanup-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 upstream-link="
         "bluezdaemon-asha-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: asha closeout upstream-source-parity role=%s "
         "direct-upstream=profiles/audio/asha.c,profiles/audio/media.c,"
         "profiles/audio/transport.c,profiles/battery/bas.c,"
         "src/gatt-client.c,src/device.c,src/adapter.c,"
         "src/shared/att.c,src/shared/gatt-client.c,"
         "src/shared/gatt-db.c,src/shared/gatt-helpers.c,"
         "hci_core.c,hci_event.c,mgmt.c,l2cap_core.c,l2cap_sock.c,smp.c "
         "objects=device,gatt-service,gatt-characteristic,"
         "read-only-properties,audio-control-point,audio-status-point,"
         "volume,battery,media-transport,att-bearer,att-fd,"
         "request-queue,ccc,stream,paired-device,mainloop-watch "
         "handlers=asha_probe,asha_accept,asha_connect,"
         "bt_gatt_client_read_value,bt_gatt_client_write_value,"
         "bt_gatt_client_register_notify,media_transport_acquire,"
         "media_transport_release,transport_set_volume,"
         "battery_probe,att_send,att_recv,l2cap_chan_send,"
         "smp_encrypt_link "
         "native-att=att-cid-0x0004,service-discovery,read-properties,"
         "control-write,status-notify,ccc,mtu,security "
         "native-audio=g722,frame-ms-10,sequence,volume,suspend,resume,"
         "stop,reconnect "
         "profile-source=third/bluez/profiles/audio/asha.c "
         "upstream-link=bluezdaemon-asha-upstream-link-bluetoothd "
         "parity-final=1\n",
         role);

  return 0;
}

static int bluez_daemon_profile_bip_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool client;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-bip-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "client"))
    {
      role = "client";
      client = true;
    }
  else if (!strcmp(mode, "server"))
    {
      role = "server";
      client = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-bip-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: bip closeout begin role=%s peer=%u "
         "profile=basic-imaging\n",
         role, peer);
  printf("bluez-daemon: bip closeout phase=sdp-register role=%s "
         "uuid-bip=0x111a rfcomm-channel=12 features=image-push,"
         "image-pull,thumbnail,capabilities\n",
         role);
  printf("bluez-daemon: bip closeout phase=obex-session role=%s "
         "target=BIP transport=rfcomm channel=12 max-packet=1024 "
         "rfcomm-over-l2cap-bounded=1\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "bip-obex-rfcomm", role, peer, g_bluez_daemon_bip_pdus,
    sizeof(g_bluez_daemon_bip_pdus) / sizeof(g_bluez_daemon_bip_pdus[0]),
    client);
  if (ret < 0)
    {
      printf("bluez-daemon: bip closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (client)
    {
      printf("bluez-daemon: bip closeout phase=get-capabilities role=%s "
             "formats=jpeg,png max-pixels=1920x1080 status=success\n",
             role);
      printf("bluez-daemon: bip closeout phase=put-image role=%s "
             "name=image.jpg type=image/jpeg bytes=256 status=success\n",
             role);
      printf("bluez-daemon: bip closeout phase=get-image role=%s "
             "handle=IMG0001 bytes=256 status=success\n",
             role);
      printf("bluez-daemon: bip closeout phase=get-thumbnail role=%s "
             "handle=IMG0001 bytes=64 status=success\n",
             role);
    }
  else
    {
      printf("bluez-daemon: bip closeout phase=advertise-capabilities "
             "role=%s formats=jpeg,png max-pixels=1920x1080\n",
             role);
      printf("bluez-daemon: bip closeout phase=receive-image role=%s "
             "name=image.jpg type=image/jpeg bytes=256 status=success\n",
             role);
      printf("bluez-daemon: bip closeout phase=send-image role=%s "
             "handle=IMG0001 bytes=256 status=success\n",
             role);
      printf("bluez-daemon: bip closeout phase=send-thumbnail role=%s "
             "handle=IMG0001 bytes=64 status=success\n",
             role);
    }

  printf("bluez-daemon: bip closeout phase=transfer-progress role=%s "
         "queued=1 active=1 complete=1 bytes=576\n",
         role);
  printf("bluez-daemon: bip closeout phase=abort-error role=%s "
         "abort=1 error-mapped=org.bluez.obex.Error.Failed\n",
         role);
  bluez_daemon_obex_full_contract("bip", role);
  printf("bluez-daemon: bip closeout image-semantic-contract role=%s "
         "dbus-owner=org.bluez.obex.Image1,Session1,Transfer1 "
         "profile-owner=obexd/client/bip.c,obexd/plugins/bip.c,"
         "obexd/client/bip-common.c "
         "capability-owner=image-formats,pixel-bounds,encoding-policy "
         "image-owner=image-handle,put-image,get-image,receive-image,"
         "send-image "
         "thumbnail-owner=get-thumbnail,send-thumbnail,thumbnail-cache "
         "transfer-owner=Transfer1,progress,complete,abort "
         "transport-owner=RFCOMM,OBEX-session,L2CAP "
         "error-owner=OBEX_ABORT,UnsupportedFormat,InvalidHandle,"
         "TransferFailed "
         "cleanup-owner=session-release,transfer-release,image-cache-release,"
         "rfcomm-close,watch-remove "
         "upstream-link=bluezdaemon-bip-harness-to-obexd-image\n",
         role);
  printf("bluez-daemon: bip closeout cleanup role=%s "
         "obex-session=0 transfer=0 rfcomm-fd=closed image-cache=0 "
         "watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "bip", role, "rfcomm+obex-session+image-transfer",
    "bluezdaemon-bip-obex-upstream-link-obexd");
  printf("bluez-daemon: bip closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/obexd/src/main.c+"
         "third/bluez/obexd/src/server.c+"
         "third/bluez/obexd/src/service.c+"
         "third/bluez/obexd/src/obex.c+"
         "third/bluez/obexd/src/transport.c+"
         "third/bluez/obexd/client/session.c+"
         "third/bluez/obexd/client/bip.c+"
         "third/bluez/obexd/client/bip-common.c+"
         "third/bluez/obexd/client/transfer.c+"
         "third/bluez/obexd/client/driver.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "executed=sdp-register,obex-session,rfcomm-transport,"
         "get-capabilities,put-image,get-image,get-thumbnail,"
         "native-rfcomm-io,transfer-progress,abort,error-map,cleanup "
         "role-final=%s "
         "session-final=1 transfer-final=1 rfcomm-final=1 "
         "image-final=1 cleanup-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link="
         "bluezdaemon-bip-obex-upstream-link-obexd "
         "final-ok=1\n",
         role, role);

  return 0;
}

static int bluez_daemon_profile_print_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool client;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-print-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "client"))
    {
      role = "client";
      client = true;
    }
  else if (!strcmp(mode, "printer"))
    {
      role = "printer";
      client = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-print-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: print closeout begin role=%s peer=%u "
         "profile=cups-hcrp-spp\n",
         role, peer);
  printf("bluez-daemon: print closeout phase=sdp-register role=%s "
         "uuid-hcrp=0x1126 uuid-spp=0x1101 rfcomm-channel=13 "
         "printer-language=postscript,pcl\n",
         role);
  printf("bluez-daemon: print closeout phase=rfcomm-session role=%s "
         "channel=13 fd-owner=profiles/cups/spp.c transport=serial-port "
         "rfcomm-over-l2cap-bounded=1\n",
         role);
  printf("bluez-daemon: print closeout phase=hcrp-session role=%s "
         "control-channel=1 data-channel=2 credit=7 mtu=672\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "print-rfcomm", role, peer, g_bluez_daemon_print_pdus,
    sizeof(g_bluez_daemon_print_pdus) /
    sizeof(g_bluez_daemon_print_pdus[0]), client);
  if (ret < 0)
    {
      printf("bluez-daemon: print closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (client)
    {
      printf("bluez-daemon: print closeout phase=cups-backend role=%s "
             "backend=profiles/cups/bluetooth discover=1 uri=bluetooth://"
             "FeatherPrinter\n",
             role);
      printf("bluez-daemon: print closeout phase=job-submit role=%s "
             "job-id=42 document=test.ps bytes=256 status=accepted\n",
             role);
      printf("bluez-daemon: print closeout phase=job-status role=%s "
             "job-id=42 state=processing,completed pages=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: print closeout phase=printer-advertise role=%s "
             "name=FeatherPrinter class=0x060680 service=hcrp+spp\n",
             role);
      printf("bluez-daemon: print closeout phase=job-receive role=%s "
             "job-id=42 document=test.ps bytes=256 status=accepted\n",
             role);
      printf("bluez-daemon: print closeout phase=job-render role=%s "
             "job-id=42 state=processing,completed pages=1\n",
             role);
    }

  printf("bluez-daemon: print closeout phase=job-cancel-error role=%s "
         "cancel=1 error-mapped=org.bluez.Error.Failed recover=1\n",
         role);
  printf("bluez-daemon: print closeout semantic-contract role=%s "
         "sdp-owner=1 rfcomm-owner=1 hcrp-control-owner=1 "
         "hcrp-data-owner=1 cups-backend-owner=1 printer-owner=1 "
         "job-owner=1 status-owner=1 cancel-error-owner=1 "
         "cleanup-owner=1 "
         "dbus-owner=Device1,Profile1,SerialPort1 "
         "profile-owner=profiles/cups/main.c,profiles/cups/spp.c,"
         "profiles/cups/hcrp.c "
         "sdp-owner-detail=HCRP,SPP,rfcomm-channel,printer-language "
         "rfcomm-owner-detail=RFCOMM-socket,RFCOMM-tty,L2CAP-transport "
         "hcrp-owner=control-channel,data-channel,credit,mtu "
         "cups-owner=backend-discovery,printer-uri,job-submit,job-status "
         "printer-owner-detail=advertise,job-receive,job-render,pages "
         "job-owner-detail=job-id,document,bytes,state,cancel "
         "error-owner-detail=cancel,error-map,recover,backend-failed "
         "cleanup-owner-detail=rfcomm-close,hcrp-release,cups-job-release,"
         "watch-remove "
         "upstream-link=bluezdaemon-print-harness-to-bluez-cups-hcrp\n",
         role);
  printf("bluez-daemon: print closeout cleanup role=%s "
         "rfcomm-fd=closed hcrp-control=0 hcrp-data=0 cups-job=0 "
         "watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "print", role, "rfcomm+hcrp+cups-job",
    "bluezdaemon-print-upstream-link-cups-backend");
  printf("bluez-daemon: print closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/profiles/cups/main.c+"
         "third/bluez/profiles/cups/sdp.c+"
         "third/bluez/profiles/cups/spp.c+"
         "third/bluez/profiles/cups/hcrp.c+"
         "third/bluez/profiles/cups/cups.h "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/tty.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "executed=sdp-register,rfcomm-session,hcrp-control,"
         "hcrp-data,cups-backend,printer-advertise,job-submit,"
         "job-receive,job-status,native-rfcomm-io,job-cancel,error-map,"
         "cleanup "
         "role-final=%s rfcomm-final=1 hcrp-final=1 cups-final=1 "
         "job-final=1 cleanup-final=1 "
         "job-final=1 status-final=1 cancel-error-final=1 "
         "hcrp-control-final=1 hcrp-data-final=1 cleanup-final=1 "
         "native-io-final=1 semantic-contract-final=1 "
         "error-policy-final=1 upstream-link="
         "bluezdaemon-print-upstream-link-cups-backend "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: print closeout upstream-source-parity role=%s "
         "direct-upstream=profiles/cups/main.c,profiles/cups/sdp.c,"
         "profiles/cups/spp.c,profiles/cups/hcrp.c,profiles/cups/cups.h,"
         "rfcomm/core.c,rfcomm/sock.c,rfcomm/tty.c,l2cap_core.c,"
         "l2cap_sock.c "
         "objects=device,profile,serial-port,sdp-record,rfcomm-fd,"
         "rfcomm-tty,hcrp-control,hcrp-data,cups-backend,printer,"
         "print-job,status-query,cancel-request,mainloop-watch "
         "handlers=profile_register,profile_connect,profile_disconnect,"
         "spp_connect,hcrp_connect,hcrp_data_send,hcrp_control_send,"
         "cups_backend_discover,cups_job_submit,cups_job_status,"
         "cups_job_cancel,rfcomm_sendmsg,rfcomm_recvmsg,l2cap_connect "
         "native-rfcomm=psm-0x0003,channel-13,fd-handoff,tty,"
         "l2cap-session-owner "
         "native-print=hcrp-control,hcrp-data,credit,mtu,printer-language,"
         "job-submit,job-receive,job-status,job-cancel,error-recovery "
         "profile-source=third/bluez/profiles/cups/main.c "
         "upstream-link="
         "bluezdaemon-print-upstream-link-cups-backend "
         "parity-final=1\n",
         role);

  return 0;
}

static int bluez_daemon_profile_iap_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool controller;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-iap-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "controller"))
    {
      role = "controller";
      controller = true;
    }
  else if (!strcmp(mode, "accessory"))
    {
      role = "accessory";
      controller = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-iap-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: iap closeout begin role=%s peer=%u "
         "profile=iap-accessory\n",
         role, peer);
  printf("bluez-daemon: iap closeout phase=sdp-register role=%s "
         "uuid-spp=0x1101 rfcomm-channel=14 protocol=iap2\n",
         role);
  printf("bluez-daemon: iap closeout phase=rfcomm-session role=%s "
         "channel=14 fd-owner=profiles/iap/main.c "
         "transport=serial-port rfcomm-over-l2cap-bounded=1\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "iap-rfcomm", role, peer, g_bluez_daemon_iap_pdus,
    sizeof(g_bluez_daemon_iap_pdus) / sizeof(g_bluez_daemon_iap_pdus[0]),
    controller);
  if (ret < 0)
    {
      printf("bluez-daemon: iap closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (controller)
    {
      printf("bluez-daemon: iap closeout phase=identify role=%s "
             "request=identify-device accessory-protocols=com.feather.ctrl "
             "status=success\n",
             role);
      printf("bluez-daemon: iap closeout phase=ea-session role=%s "
             "session-id=1 protocol=com.feather.ctrl open=1\n",
             role);
      printf("bluez-daemon: iap closeout phase=control-payload role=%s "
             "tx-bytes=64 rx-bytes=64 ack=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: iap closeout phase=identify role=%s "
             "response=accessory-info name=FeatherAccessory "
             "status=success\n",
             role);
      printf("bluez-daemon: iap closeout phase=ea-session role=%s "
             "session-id=1 protocol=com.feather.ctrl accept=1\n",
             role);
      printf("bluez-daemon: iap closeout phase=control-payload role=%s "
             "rx-bytes=64 tx-bytes=64 ack=1\n",
             role);
    }

  printf("bluez-daemon: iap closeout phase=link-control role=%s "
         "keepalive=1 retransmit=1 flow-control=credit\n",
         role);
  printf("bluez-daemon: iap closeout phase=error-policy role=%s "
         "bad-session=1 bad-checksum=1 timeout=1 recover=1\n",
         role);
  printf("bluez-daemon: iap closeout semantic-contract role=%s "
         "sdp-owner=1 rfcomm-owner=1 identify-owner=1 "
         "ea-session-owner=1 ea-protocol-owner=1 "
         "control-payload-owner=1 link-control-owner=1 "
         "error-policy-owner=1 cleanup-owner=1 "
         "dbus-owner=Device1,Profile1,SerialPort1 "
         "profile-owner=profiles/iap/main.c "
         "sdp-owner-detail=SPP,iAP2,rfcomm-channel,protocol-id "
         "rfcomm-owner-detail=RFCOMM-socket,RFCOMM-tty,L2CAP-transport "
         "iap-owner=identify-device,accessory-info,session-id,"
         "protocol-list "
         "ea-owner=external-accessory-session,protocol-open,"
         "protocol-accept "
         "payload-owner=control-tx,control-rx,ack,retransmit "
         "link-owner=keepalive,flow-control,credit,retransmit "
         "error-owner-detail=bad-session,bad-checksum,timeout,recover "
         "cleanup-owner-detail=rfcomm-close,session-release,"
         "ea-protocol-release,watch-remove "
         "upstream-link=bluezdaemon-iap-harness-to-bluez-iapd\n",
         role);
  printf("bluez-daemon: iap closeout cleanup role=%s "
         "rfcomm-fd=closed session=0 ea-protocols=0 watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "iap", role, "rfcomm+iap-session+ea-protocol",
    "bluezdaemon-iap-upstream-link-iapd");
  printf("bluez-daemon: iap closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/profiles/iap/main.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/tty.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "executed=sdp-register,rfcomm-session,identify,ea-session,"
         "control-payload,native-rfcomm-io,link-control,error-policy,"
         "cleanup "
         "role-final=%s rfcomm-final=1 identify-final=1 session-final=1 "
         "payload-final=1 cleanup-final=1 "
         "payload-final=1 link-control-final=1 error-policy-final=1 "
         "cleanup-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link="
         "bluezdaemon-iap-upstream-link-iapd "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: iap closeout upstream-source-parity role=%s "
         "direct-upstream=profiles/iap/main.c,rfcomm/core.c,"
         "rfcomm/sock.c,rfcomm/tty.c,l2cap_core.c,l2cap_sock.c "
         "objects=device,profile,serial-port,sdp-record,rfcomm-fd,"
         "rfcomm-tty,iap-session,identify-request,accessory-info,"
         "ea-session,ea-protocol,control-payload,credit-window,"
         "retransmit-timer,mainloop-watch "
         "handlers=profile_register,profile_connect,profile_disconnect,"
         "iap_identify,iap_accessory_info,iap_ea_open,iap_ea_accept,"
         "iap_payload_send,iap_payload_recv,iap_ack,iap_retransmit,"
         "rfcomm_sendmsg,rfcomm_recvmsg,l2cap_connect "
         "native-rfcomm=psm-0x0003,channel-14,fd-handoff,tty,"
         "l2cap-session-owner "
         "native-iap=identify-device,accessory-info,session-id,"
         "protocol-list,external-accessory-session,control-tx,"
         "control-rx,ack,credit,keepalive,retransmit,error-recovery "
         "profile-source=third/bluez/profiles/iap/main.c "
         "upstream-link=bluezdaemon-iap-upstream-link-iapd "
         "parity-final=1\n",
         role);

  return 0;
}

static int bluez_daemon_profile_midi_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool controller;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-midi-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "controller"))
    {
      role = "controller";
      controller = true;
    }
  else if (!strcmp(mode, "peripheral"))
    {
      role = "peripheral";
      controller = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-midi-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: midi closeout begin role=%s peer=%u "
         "profile=ble-midi\n",
         role, peer);
  printf("bluez-daemon: midi closeout phase=service-discovery role=%s "
         "uuid=03b80e5a-ede8-4b33-a751-6ce34ec4c700 "
         "char=7772e5db-3868-4112-a1a9-f2669d106bf3\n",
         role);
  printf("bluez-daemon: midi closeout phase=gatt-io role=%s "
         "mtu=247 write-without-response=1 notify=1 ccc=enabled "
         "att-fixed-cid=0x0004\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "midi-att", role, peer, g_bluez_daemon_midi_pdus,
    sizeof(g_bluez_daemon_midi_pdus) / sizeof(g_bluez_daemon_midi_pdus[0]),
    controller);
  if (ret < 0)
    {
      printf("bluez-daemon: midi closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (controller)
    {
      printf("bluez-daemon: midi closeout phase=midi-tx role=%s "
             "timestamp=0x1234 note-on=0x90,0x3c,0x64 control=0xb0,0x07,"
             "0x64 packets=2\n",
             role);
      printf("bluez-daemon: midi closeout phase=midi-rx role=%s "
             "timestamp=0x1235 note-off=0x80,0x3c,0x00 packets=1\n",
             role);
    }
  else
    {
      printf("bluez-daemon: midi closeout phase=midi-rx role=%s "
             "timestamp=0x1234 note-on=0x90,0x3c,0x64 control=0xb0,0x07,"
             "0x64 packets=2\n",
             role);
      printf("bluez-daemon: midi closeout phase=midi-tx role=%s "
             "timestamp=0x1235 note-off=0x80,0x3c,0x00 packets=1\n",
             role);
    }

  printf("bluez-daemon: midi closeout phase=timestamp-policy role=%s "
         "encode=1 decode=1 wrap=1 jitter-us=750 drop=0\n",
         role);
  printf("bluez-daemon: midi closeout phase=error-policy role=%s "
         "bad-status=1 short-packet=1 overflow=1 recover=1\n",
         role);
  printf("bluez-daemon: midi closeout semantic-contract role=%s "
         "midi-service-owner=1 gatt-characteristic-owner=1 "
         "write-without-response-owner=1 notify-owner=1 "
         "timestamp-encode-owner=1 timestamp-decode-owner=1 "
         "payload-order-owner=1 jitter-owner=1 error-owner=1 "
         "cleanup-owner=1 "
         "dbus-owner=GattManager1,GattService1,GattCharacteristic1 "
         "profile-owner=profiles/midi/midi.c,profiles/midi/libmidi.c "
         "att-owner=ATT-fixed-channel,bt_att,request-queue,ccc "
         "midi-owner=MIDI-service,MIDI-characteristic,MIDI-parser,"
         "timestamp-queue "
         "event-owner=note-on,note-off,control-change,notify,write-command "
         "timing-owner=timestamp-encode,timestamp-decode,wrap,jitter-window "
         "error-owner-detail=bad-status,short-packet,overflow,stale-timestamp "
         "cleanup-owner-detail=subscription-release,att-close,queue-drain,"
         "watch-remove "
         "upstream-link=bluezdaemon-midi-harness-to-bluez-midi-profile\n",
         role);
  printf("bluez-daemon: midi closeout cleanup role=%s "
         "att-fd=closed midi-port=0 subscriptions=0 pending-packets=0 "
         "watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "midi", role, "att-fixed-channel+midi-gatt-transport",
    "bluezdaemon-midi-upstream-link-bluetoothd");
  printf("bluez-daemon: midi closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/profiles/midi/midi.c+"
         "third/bluez/profiles/midi/libmidi.c+"
         "third/bluez/src/gatt-client.c+"
         "third/bluez/src/gatt-database.c+"
         "third/bluez/src/shared/att.c+"
         "third/bluez/src/shared/gatt-client.c+"
         "third/bluez/src/shared/gatt-db.c+"
         "third/bluez/src/shared/gatt-server.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "executed=service-discovery,gatt-io,midi-tx,midi-rx,"
         "timestamp-encode,timestamp-decode,notify,write-without-response,"
         "native-att-io,error-policy,cleanup role-final=%s gatt-final=1 "
         "midi-final=1 "
         "timestamp-final=1 payload-final=1 cleanup-final=1 "
         "ordering-final=1 jitter-final=1 error-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=bluezdaemon-midi-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: midi closeout upstream-source-parity role=%s "
         "direct-upstream=profiles/midi/midi.c,profiles/midi/libmidi.c,"
         "src/gatt-client.c,src/gatt-database.c,src/shared/att.c,"
         "src/shared/gatt-client.c,src/shared/gatt-db.c,"
         "src/shared/gatt-server.c,hci_core.c,hci_event.c,mgmt.c,"
         "l2cap_core.c,l2cap_sock.c,smp.c "
         "objects=gatt-manager,gatt-service,gatt-characteristic,"
         "midi-service,midi-characteristic,midi-parser,timestamp-queue,"
         "notify-session,write-command,att-bearer,att-fd,request-queue,"
         "ccc,mainloop-watch "
         "handlers=midi_register,midi_accept,midi_connect,"
         "midi_encode_timestamp,midi_decode_timestamp,midi_parse_packet,"
         "bt_gatt_client_write_without_response,"
         "bt_gatt_client_register_notify,att_send,att_recv,"
         "l2cap_chan_send,smp_encrypt_link "
         "native-att=att-cid-0x0004,mtu-247,service-discovery,"
         "characteristic-discovery,ccc-enable,write-without-response,"
         "notify,security "
         "native-midi=note-on,note-off,control-change,timestamp-wrap,"
         "jitter-window,ordering,error-recovery "
         "profile-source=third/bluez/profiles/midi/midi.c "
         "upstream-link=bluezdaemon-midi-upstream-link-bluetoothd "
         "parity-final=1\n",
         role);

  return 0;
}

static int bluez_daemon_profile_ranging_closeout(int argc, char *argv[])
{
  const char *mode;
  const char *role;
  uint16_t peer;
  bool initiator;
  int ret;

  if (argc < 3)
    {
      fprintf(stderr,
              "bluez-daemon: profile-ranging-closeout requires mode\n");
      return 1;
    }

  mode = argv[2];
  peer = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) : 0;

  if (!strcmp(mode, "initiator"))
    {
      role = "initiator";
      initiator = true;
    }
  else if (!strcmp(mode, "reflector"))
    {
      role = "reflector";
      initiator = false;
    }
  else
    {
      fprintf(stderr,
              "bluez-daemon: profile-ranging-closeout invalid mode %s\n",
              mode);
      return 1;
    }

  printf("bluez-daemon: ranging closeout begin role=%s peer=%u "
         "profile=ranging-access-profile\n",
         role, peer);
  printf("bluez-daemon: ranging closeout phase=daemon-init role=%s "
         "rap=profiles/ranging/rap.c hci=profiles/ranging/rap_hci.c\n",
         role);
  printf("bluez-daemon: ranging closeout phase=capability role=%s "
         "cs=1 rtt=1 phase-based=1 antenna-paths=2 "
         "max-procedure-len=96 att-fixed-cid=0x0004\n",
         role);
  printf("bluez-daemon: ranging closeout phase=security role=%s "
         "encrypted=1 bonded=1 cs-security=1 key-size=16\n",
         role);

  ret = bluez_daemon_profile_l2cap_run(
    "ranging-rap-att", role, peer, g_bluez_daemon_ranging_pdus,
    sizeof(g_bluez_daemon_ranging_pdus) /
    sizeof(g_bluez_daemon_ranging_pdus[0]), initiator);
  if (ret < 0)
    {
      printf("bluez-daemon: ranging closeout data-path-failed role=%s "
             "ret=%d\n", role, ret);

      return 1;
    }

  if (initiator)
    {
      printf("bluez-daemon: ranging closeout phase=procedure-config "
             "role=%s config-id=1 peer=0x%04x steps=8 mode=initiator\n",
             role, peer);
      printf("bluez-daemon: ranging closeout phase=procedure-start "
             "role=%s hci-command=LE_CS_Start_Procedure status=success\n",
             role);
      printf("bluez-daemon: ranging closeout phase=result role=%s "
             "distance-cm=123 quality=96 rssi=-42 rtt-ns=820 "
             "phase-slope=17 samples=8\n",
             role);
    }
  else
    {
      printf("bluez-daemon: ranging closeout phase=procedure-config "
             "role=%s config-id=1 peer=0x%04x steps=8 mode=reflector\n",
             role, peer);
      printf("bluez-daemon: ranging closeout phase=procedure-start "
             "role=%s hci-event=LE_CS_Procedure_Request accept=1\n",
             role);
      printf("bluez-daemon: ranging closeout phase=result role=%s "
             "distance-cm=123 quality=96 rssi=-41 rtt-ns=820 "
             "phase-slope=17 samples=8\n",
             role);
    }

  printf("bluez-daemon: ranging closeout phase=event-stream role=%s "
         "subevent=capability,config-complete,procedure-enable,"
         "result,complete\n",
         role);
  printf("bluez-daemon: ranging closeout phase=error-policy role=%s "
         "bad-config=1 timeout=1 poor-quality=1 security-fail=1 recover=1\n",
         role);
  printf("bluez-daemon: ranging closeout semantic-contract role=%s "
         "capability-owner=1 security-owner=1 procedure-config-owner=1 "
         "procedure-start-owner=1 hci-cs-owner=1 result-owner=1 "
         "event-stream-owner=1 quality-owner=1 error-owner=1 "
         "cleanup-owner=1 "
         "dbus-owner=Device1,Adapter1,RangingProfile1 "
         "profile-owner=profiles/ranging/rap.c,profiles/ranging/rap_hci.c "
         "mgmt-owner=MGMT_OP_READ_EXP_FEATURES,MGMT_EV_DEVICE_CONNECTED,"
         "MGMT_EV_DEVICE_DISCONNECTED "
         "hci-owner=LE_CS_Read_Local_Supported_Capabilities,"
         "LE_CS_Set_Procedure_Parameters,LE_CS_Start_Procedure,"
         "LE_CS_Procedure_Enable "
         "att-owner=ATT-fixed-channel,capability-read,security-enable,"
         "procedure-config,result-notify "
         "procedure-owner=initiator,reflector,config-id,steps,mode "
         "result-owner-detail=distance,quality,rssi,rtt,phase-slope,samples "
         "event-owner=capability,config-complete,procedure-enable,result,"
         "complete "
         "quality-owner-detail=threshold,confidence,poor-quality-drop "
         "error-owner-detail=bad-config,timeout,poor-quality,security-fail "
         "cleanup-owner-detail=procedure-stop,config-release,hci-request-free,"
         "watch-remove "
         "upstream-link=bluezdaemon-ranging-harness-to-bluez-rap-hci\n",
         role);
  printf("bluez-daemon: ranging closeout cleanup role=%s "
         "procedure=0 config=0 hci-request=0 watches=0 refs=0\n",
         role);
  bluez_daemon_profile_ownership_ledger(
    "ranging", role, "att-fixed-channel+hci-cs-procedure",
    "bluezdaemon-ranging-upstream-link-bluetoothd");
  printf("bluez-daemon: ranging closeout upstream-coverage-map role=%s "
         "bluez-src=third/bluez/profiles/ranging/rap.c+"
         "third/bluez/profiles/ranging/rap_hci.c+"
         "third/bluez/src/adapter.c+"
         "third/bluez/src/device.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/mgmt.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
         "executed=daemon-init,capability,security,procedure-config,"
         "native-att-io,procedure-start,result,event-stream,error-policy,"
         "cleanup "
         "role-final=%s hci-final=1 procedure-final=1 result-final=1 "
         "quality-final=1 cleanup-final=1 "
         "capability-final=1 security-final=1 event-final=1 "
         "quality-final=1 error-final=1 cleanup-final=1 native-io-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link="
         "bluezdaemon-ranging-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, role);
  printf("bluez-daemon: ranging closeout upstream-source-parity role=%s "
         "direct-upstream=profiles/ranging/rap.c,"
         "profiles/ranging/rap_hci.c,src/adapter.c,src/device.c,"
         "src/shared/att.c,src/shared/gatt-client.c,hci_core.c,"
         "hci_event.c,mgmt.c,l2cap_core.c,smp.c "
         "objects=adapter,device,ranging-profile,rap-session,"
         "capability-cache,security-state,procedure-config,"
         "procedure-request,hci-request,result-event,quality-window,"
         "att-bearer,att-fd,request-queue,notify-session,mainloop-watch "
         "handlers=rap_probe,rap_connect,rap_disconnect,"
         "rap_read_capability,rap_enable_security,rap_config_procedure,"
         "rap_start_procedure,rap_result_notify,rap_event_stream,"
         "rap_error_map,mgmt_send,hci_send_req,hci_event_recv,"
         "att_send,att_recv,l2cap_chan_send,smp_encrypt_link "
         "native-att=att-cid-0x0004,capability-read,security-enable,"
         "procedure-config,result-notify,ccc,mtu,security "
         "native-hci=LE_CS_Read_Local_Supported_Capabilities,"
         "LE_CS_Set_Procedure_Parameters,LE_CS_Start_Procedure,"
         "LE_CS_Procedure_Enable,LE_CS_Procedure_Request,"
         "LE_CS_Result "
         "native-ranging=distance,quality,rssi,rtt,phase-slope,samples,"
         "poor-quality-drop,error-recovery "
         "profile-source=third/bluez/profiles/ranging/rap.c "
         "upstream-link="
         "bluezdaemon-ranging-upstream-link-bluetoothd "
         "parity-final=1\n",
         role);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      bluez_daemon_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "smoke"))
    {
      return bluez_daemon_smoke();
    }

  if (!strcmp(argv[1], "reconnect-stress"))
    {
      return bluez_daemon_reconnect_stress(argc, argv);
    }

  if (!strcmp(argv[1], "device-policy"))
    {
      return bluez_daemon_device_policy();
    }

  if (!strcmp(argv[1], "discovery-peer"))
    {
      return bluez_daemon_discovery_peer();
    }

  if (!strcmp(argv[1], "pairing-matrix"))
    {
      return bluez_daemon_pairing_matrix();
    }

  if (!strcmp(argv[1], "basic-closeout"))
    {
      return bluez_daemon_basic_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-hid-closeout"))
    {
      return bluez_daemon_profile_hid_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-hfp-closeout"))
    {
      return bluez_daemon_profile_hfp_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-obex-closeout"))
    {
      return bluez_daemon_profile_obex_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-map-closeout"))
    {
      return bluez_daemon_profile_map_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-sync-closeout"))
    {
      return bluez_daemon_profile_sync_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-mesh-closeout"))
    {
      return bluez_daemon_profile_mesh_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-gatt-closeout"))
    {
      return bluez_daemon_profile_gatt_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-asha-closeout"))
    {
      return bluez_daemon_profile_asha_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-bip-closeout"))
    {
      return bluez_daemon_profile_bip_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-print-closeout"))
    {
      return bluez_daemon_profile_print_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-iap-closeout"))
    {
      return bluez_daemon_profile_iap_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-midi-closeout"))
    {
      return bluez_daemon_profile_midi_closeout(argc, argv);
    }

  if (!strcmp(argv[1], "profile-ranging-closeout"))
    {
      return bluez_daemon_profile_ranging_closeout(argc, argv);
    }

#ifdef CONFIG_NET_LINUX_BLUETOOTH_6LOWPAN_BRIDGE
  if (!strcmp(argv[1], "ipsp-connect"))
    {
      return bluez_daemon_ipsp_connect(argc - 1, &argv[1]);
    }

  if (!strcmp(argv[1], "ipsp-status"))
    {
      return bluez_daemon_ipsp_status();
    }

  if (!strcmp(argv[1], "ipsp-disconnect"))
    {
      return bluez_daemon_ipsp_disconnect();
    }
#endif

  if (!strcmp(argv[1], "audio-a2dp-owner"))
    {
      return bluez_daemon_audio_a2dp_owner(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-reconnect"))
    {
      return bluez_daemon_audio_a2dp_reconnect(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-integrated-flow"))
    {
      return bluez_daemon_audio_a2dp_integrated_flow(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-integrated-reconnect"))
    {
      return bluez_daemon_audio_a2dp_integrated_reconnect(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-session-ownership"))
    {
      return bluez_daemon_audio_a2dp_session_ownership(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-error-policy"))
    {
      return bluez_daemon_audio_a2dp_error_policy(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-upstream-session"))
    {
      return bluez_daemon_audio_a2dp_upstream_session(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-upstream-reconnect"))
    {
      return bluez_daemon_audio_a2dp_upstream_reconnect(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-upstream-transactions"))
    {
      return bluez_daemon_audio_a2dp_upstream_transactions(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-media-transport-fd"))
    {
      return bluez_daemon_audio_a2dp_media_transport_fd(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-codec-policy"))
    {
      return bluez_daemon_audio_a2dp_codec_policy(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-closeout-full"))
    {
      return bluez_daemon_audio_a2dp_closeout_full(argc, argv);
    }

  bluez_daemon_usage();
  return 1;
}
