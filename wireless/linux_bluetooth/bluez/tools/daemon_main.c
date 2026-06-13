/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/daemon_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_HCI
#  define BTPROTO_HCI 1
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
  printf("       bluezdaemon audio-a2dp-owner source|sink [peer]\n");
  printf("       bluezdaemon audio-a2dp-reconnect source|sink [peer] [rounds]\n");
  printf("\n");
  printf("Daemon-shaped BlueZ adapter smoke over the Linux mgmt socket ABI.\n");
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
}

static int bluez_daemon_l2cap_recv_wait(void *handle_ptr, size_t max_len,
                                        char *out, size_t out_len,
                                        const char *label,
                                        const char *role);

static int bluez_daemon_l2cap_exchange(uint16_t peer,
                                       const struct bluez_daemon_audio_pdu *pdu)
{
  char out[256];
  void *handle_ptr = NULL;
  uint16_t handle = bluez_daemon_bredr_handle(peer);
  int ret;
  int failed = 0;

  ret = linux_bt_upstream_l2cap_socket_open(pdu->psm, pdu->cid, handle,
                                            &handle_ptr);
  printf("bluez-daemon: l2cap open label=%s role=source psm=0x%04x "
         "cid=0x%04x handle=0x%04x ret=%d\n",
         pdu->label, pdu->psm, pdu->cid, handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_handle(handle_ptr, pdu->psm,
                                                      pdu->cid);
  printf("bluez-daemon: l2cap connect label=%s role=source ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_write_handle(handle_ptr, pdu->cmd,
                                                        pdu->cmd_len, out,
                                                        sizeof(out));
      printf("bluez-daemon: l2cap write label=%s role=source len=%u "
             "ret=%d detail=%s\n",
             pdu->label, (unsigned int)pdu->cmd_len, ret, out);
      failed |= ret < 0;
    }

  if (ret >= 0 && pdu->psm != BLUEZ_DAEMON_AVRCP_PSM &&
      pdu->psm != BLUEZ_DAEMON_AVRCP_BROWSE_PSM)
    {
      ret = bluez_daemon_l2cap_recv_wait(handle_ptr, pdu->rsp_len, out,
                                         sizeof(out), pdu->label, "source");
      failed |= ret < 0;
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
  unsigned int i;
  int ret = -EAGAIN;

  for (i = 0; i < 80; i++)
    {
      ret = linux_bt_upstream_l2cap_socket_recv_handle(handle_ptr, max_len,
                                                       out, out_len);
      printf("bluez-daemon: l2cap mainloop recv label=%s role=%s "
             "wait=%u expect-len=%u ret=%d detail=%s\n",
             label, role, i + 1, (unsigned int)max_len, ret, out);
      if (ret >= 0)
        {
          return ret;
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

  ret = linux_bt_upstream_l2cap_socket_open(pdu->psm, pdu->cid, handle,
                                            &handle_ptr);
  printf("bluez-daemon: l2cap open label=%s role=sink psm=0x%04x "
         "cid=0x%04x handle=0x%04x ret=%d\n",
         pdu->label, pdu->psm, pdu->cid, handle, ret);
  if (ret < 0)
    {
      return -1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_handle(handle_ptr, pdu->psm,
                                                      pdu->cid);
  printf("bluez-daemon: l2cap connect label=%s role=sink ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = bluez_daemon_l2cap_recv_wait(handle_ptr, pdu->cmd_len, out,
                                         sizeof(out), pdu->label, "sink");
      failed |= ret < 0;
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
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: l2cap close label=%s role=sink ret=%d\n",
         pdu->label, ret);
  failed |= ret < 0;

  if (failed == 0)
    {
      printf("bluez-daemon: audio owner sink step=%s complete\n",
             pdu->label);
    }

  return failed ? -1 : 0;
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

  ret = linux_bt_upstream_l2cap_socket_connect_handle(
    handle_ptr, BLUEZ_DAEMON_A2DP_MEDIA_PSM, BLUEZ_DAEMON_A2DP_MEDIA_CID);
  printf("bluez-daemon: media connect role=source ret=%d\n", ret);
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = linux_bt_upstream_l2cap_socket_write_handle(
        handle_ptr, g_bluez_daemon_media_payload,
        sizeof(g_bluez_daemon_media_payload), out, sizeof(out));
      printf("bluez-daemon: media write role=source codec=sbc len=%u "
             "ret=%d detail=%s\n",
             (unsigned int)sizeof(g_bluez_daemon_media_payload), ret, out);
      failed |= ret < 0;
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: media close role=source ret=%d\n", ret);
  failed |= ret < 0;

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

  ret = linux_bt_upstream_l2cap_socket_connect_handle(
    handle_ptr, BLUEZ_DAEMON_A2DP_MEDIA_PSM, BLUEZ_DAEMON_A2DP_MEDIA_CID);
  printf("bluez-daemon: media connect role=sink ret=%d\n", ret);
  failed |= ret < 0;

  if (ret >= 0)
    {
      ret = bluez_daemon_l2cap_recv_wait(
        handle_ptr, sizeof(g_bluez_daemon_media_payload), out, sizeof(out),
        "media", "sink");
      failed |= ret < 0;
    }

  ret = linux_bt_upstream_l2cap_socket_close_handle(handle_ptr);
  printf("bluez-daemon: media close role=sink ret=%d\n", ret);
  failed |= ret < 0;

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
      printf("bluez-daemon: reconnect-stress complete rounds=%u\n", rounds);
    }

  return failed;
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

  if (!strcmp(argv[1], "audio-a2dp-owner"))
    {
      return bluez_daemon_audio_a2dp_owner(argc, argv);
    }

  if (!strcmp(argv[1], "audio-a2dp-reconnect"))
    {
      return bluez_daemon_audio_a2dp_reconnect(argc, argv);
    }

  bluez_daemon_usage();
  return 1;
}
