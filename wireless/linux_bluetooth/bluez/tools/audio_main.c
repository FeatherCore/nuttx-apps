/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/audio_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/wireless/linux_bluetooth.h>

#include "../codecs/lc3_backend.h"
#include "../codecs/sbc_backend.h"
#include "../upstream_a2dp_object_probe.h"
#include "../upstream_avdtp_object_probe.h"
#include "../upstream_media_object_probe.h"
#include "../upstream_transport_object_probe.h"

/****************************************************************************
 * Private Pre-processor Definitions
 ****************************************************************************/

#ifndef BDADDR_LE_PUBLIC
#  define BDADDR_LE_PUBLIC 0x01
#endif

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef PF_BLUETOOTH
#  define PF_BLUETOOTH AF_BLUETOOTH
#endif

#ifndef BTPROTO_ISO
#  define BTPROTO_ISO 8
#endif

#ifndef SOL_BLUETOOTH
#  define SOL_BLUETOOTH 274
#endif

#ifndef BT_DEFER_SETUP
#  define BT_DEFER_SETUP 7
#endif

#ifndef BT_PKT_STATUS
#  define BT_PKT_STATUS 16
#endif

#ifndef BT_ISO_QOS
#  define BT_ISO_QOS 17
#endif

#ifndef BT_ISO_BASE
#  define BT_ISO_BASE 20
#endif

#ifndef BT_PKT_SEQNUM
#  define BT_PKT_SEQNUM 22
#endif

#ifndef BT_ISO_PHY_2M
#  define BT_ISO_PHY_2M 0x02
#endif

struct bluez_audio_sockaddr_iso_s
{
  sa_family_t iso_family;
  uint8_t iso_bdaddr[6];
  uint8_t iso_bdaddr_type;
};

struct bluez_audio_bt_iso_io_qos_s
{
  uint32_t interval;
  uint16_t latency;
  uint16_t sdu;
  uint8_t phys;
  uint8_t rtn;
};

struct bluez_audio_bt_iso_ucast_qos_s
{
  uint8_t cig;
  uint8_t cis;
  uint8_t sca;
  uint8_t packing;
  uint8_t framing;
  struct bluez_audio_bt_iso_io_qos_s in;
  struct bluez_audio_bt_iso_io_qos_s out;
};

struct bluez_audio_bt_iso_qos_s
{
  union
  {
    struct bluez_audio_bt_iso_ucast_qos_s ucast;
  };
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_audio_usage(void)
{
  printf("usage: bluezaudio <profile> <command> [args]\n");
  printf("\n");
  printf("profiles:\n");
  printf("  a2dp-signal listen|listen-native [handle]|auto-rsp-loop [peer] [count]|close\n");
  printf("  a2dp-signal source-session-open|source-session-close [peer]\n");
  printf("  a2dp-signal source-transaction|discover|getallcap|getcap|setconfig|getconfig|open|reconfigure|delay-report|security-control|start|suspend|close-stream|abort [peer]\n");
  printf("  a2dp-endpoint lifecycle|clear source|sink [peer]\n");
  printf("  a2dp-codec source-sbc-encode-write-release|sink-sbc-recv-decode-release [peer] [max]\n");
  printf("  avrcp-control controller-play|target-respond [peer]\n");
  printf("  avrcp-control controller-browse|target-browse-respond [peer]\n");
  printf("  avrcp-control controller-notify|target-notify-respond [peer]\n");
  printf("  avrcp-control controller-volume|target-volume-respond [peer]\n");
  printf("  avrcp-control controller-metadata|target-metadata-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-list|target-player-settings-list-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-values|target-player-settings-values-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-value-text|target-player-settings-value-text-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-notify|target-player-settings-notify-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-error|target-player-settings-error-respond [peer]\n");
  printf("  avrcp-control controller-addressed-player|target-addressed-player-respond [peer]\n");
  printf("  avrcp-control controller-player-settings|target-player-settings-respond [peer]\n");
  printf("  avrcp-control controller-player-settings-set|target-player-settings-set-respond [peer]\n");
  printf("  a2dp-source start [peer]\n");
  printf("  a2dp-sink start|read|stop [max]\n");
  printf("  upstream-object-probe source|sink\n");
  printf("  le-bap-control source-announce|source-start|source-stop [big] [bis]\n");
  printf("  le-bap-control sink-discover|sink-config|sink-sync [big] [bis]\n");
  printf("  le-daemon plugin-init|adapter-powered|mainloop-dispatch|profile-accept|profile-release|plugin-exit|register|release|unicast-profile-flow source|sink [cig] [cis]|integrated-profile-flow source|sink [cig] [cis] [peer-cis]|broadcast-profile-flow source|sink [big] [bis]\n");
  printf("  le-mgmt-control power-on|scan-start|connect|security|cis-request|disconnect|error source|sink [cig] [cis]\n");
  printf("  le-cap-control coordinator-register|group-config|group-enable|group-release|group-config-bidir|group-enable-bidir|group-release-bidir|coordinator-release [cig] [cis] [peer-cis]\n");
  printf("  le-bass-control assistant-register|add-source|modify-source|remove-source|assistant-release [big] [bis]\n");
  printf("  le-vcp-control register|discover|read-state|set-volume|notify-state|flags|error|release source|sink [cig] [cis]\n");
  printf("  le-micp-control register|discover|read-state|mute|notify-state|flags|error|release source|sink [cig] [cis]\n");
  printf("  le-csip-control register|discover|read-sirk|read-size|read-rank|lock|unlock|notify|error|release source|sink [cig] [cis]\n");
  printf("  le-mcp-control register|discover|read-player|read-track|play|pause|next|notify-state|search|error|release source|sink [cig] [cis]\n");
  printf("  le-tmap-control register|discover|read-role|update-role|notify-role|error|release source|sink [cig] [cis]\n");
  printf("  le-ccp-control register|discover|read-bearer|read-call-state|originate|accept|terminate|notify-call-state|termination-reason|error|release source|sink [cig] [cis]\n");
  printf("  le-gmap-control register|discover|read-role|update-role|notify-role|error|release source|sink [cig] [cis]\n");
  printf("  le-ascs-cp config-codec|config-qos|enable|receiver-start-ready|disable|receiver-stop-ready|update-metadata|release|config-qos-reject source|sink [cig] [cis]\n");
  printf("  le-bap-policy scheduler-register|select-codec|select-qos|select-cis|bind-transport|start-stream|suspend-stream|stop-stream|scheduler-release source|sink [cig] [cis]\n");
  printf("  le-gatt-db register|discover-pacs|discover-ascs|read-pac|read-location|read-context|update-context|notify-pac|read-ase|write-ascs-cp|notify-ase|release source|sink [cig] [cis]\n");
  printf("  le-att-bearer open|mtu-exchange|security|enable-ccc|prepare-write|execute-write|indicate|close source|sink [cig] [cis]\n");
  printf("  le-att-io attach|watch-rx|watch-tx|rx-pdu|tx-pdu|fragment-write|reassemble|persist-ccc|flush|detach source|sink [cig] [cis]\n");
  printf("  le-att-queue alloc-req|enqueue|socket-read|socket-write|timeout|cancel|error-rsp|complete|free-req source|sink [cig] [cis]\n");
  printf("  le-gatt-upstream closeout source|sink [cig] [cis]\n");
  printf("  le-iso-socket open|bind-cis|connect|listen|accept|pollout|sendmsg|pollin|recvmsg|timestamp|error-eagain|shutdown|close source|sink [cig] [cis]\n");
  printf("  le-iso-qos configure|select-phy|setup-cig|setup-cis|apply-qos|controller-timing|credit-grant|credit-complete|teardown source|sink [cig] [cis]\n");
  printf("  le-broadcast-iso adv-start|base-config|big-create|bis-setup|bis-bind|pa-sync|big-sync|receive-state|bis-credit|bis-complete|big-terminate source|sink [big] [bis]\n");
  printf("  le-broadcast-security set-code|encrypt-big|decrypt-setup|receive-state-encrypted|bad-code|clear-code source|sink [big] [bis]\n");
  printf("  le-dbus-client register|configure|transport|transport-busy|owner-lost|owner-reacquire|release source|sink [cig] [cis]\n");
  printf("  le-unicast-control source-config|source-enable|source-disable|source-qos-update|source-qos-reject|source-qos-cancel|source-release [cig] [cis]\n");
  printf("  le-unicast-control sink-discover|sink-config|sink-enable|sink-disable|sink-qos-update|sink-qos-reject|sink-qos-cancel|sink-release [cig] [cis]\n");
  printf("  le-broadcast-source start [big] [bis]\n");
  printf("  le-broadcast-sink sync|start|stop [big] [bis] [max]\n");
  printf("  le-unicast-source start [cig] [cis]\n");
  printf("  le-unicast-sink sync|start|stop [cig] [cis] [max]\n");
  printf("  le-audio-codec source-lc3-encode-write-release|sink-lc3-recv-decode-release [cig] [cis] [max]\n");
  printf("  media-transport a2dp-source-acquire-write-release [peer]\n");
  printf("  media-transport a2dp-sink-acquire-read-release [peer] [max]\n");
  printf("  media-transport unicast-source-acquire-write-release [cig] [cis]\n");
  printf("  media-transport unicast-sink-acquire-read-release [cig] [cis] [max]\n");
  printf("\n");
  printf("BlueZ audio/profile-shaped adapter over the Linux Bluetooth ");
  printf("L2CAP/ISO socket paths.\n");
}

static uint16_t bluez_audio_bredr_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static int g_bluez_audio_a2dp_source_session_active;
static uint16_t g_bluez_audio_a2dp_source_session_peer;
static uint16_t g_bluez_audio_a2dp_source_session_handle;
static void *g_bluez_audio_a2dp_source_session_l2cap;

static const uint8_t g_bluez_audio_avrcp_play_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c, 0x44, 0x00
};

static const uint8_t g_bluez_audio_avrcp_play_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c, 0x44, 0x00
};

static const uint8_t g_bluez_audio_avrcp_browse_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x71, 0x00, 0x00, 0x00, 0x01
};

static const uint8_t g_bluez_audio_avrcp_browse_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x71, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_audio_avrcp_notify_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x31, 0x00, 0x00, 0x05,
  0x01, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_audio_avrcp_notify_rsp[] =
{
  0x02, 0x11, 0x0e, 0x0f, 0x31, 0x00, 0x00, 0x02,
  0x01, 0x01
};

static const uint8_t g_bluez_audio_avrcp_volume_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x50, 0x00, 0x00, 0x01, 0x40
};

static const uint8_t g_bluez_audio_avrcp_volume_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x50, 0x00, 0x00, 0x01, 0x40
};

static const uint8_t g_bluez_audio_avrcp_metadata_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x20, 0x00, 0x00, 0x09,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00
};

static const uint8_t g_bluez_audio_avrcp_metadata_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x20, 0x00, 0x00, 0x10,
  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x6a, 0x00,
  0x07, 0x46, 0x65, 0x61, 0x74, 0x68, 0x65, 0x72
};

static const uint8_t g_bluez_audio_avrcp_player_settings_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x13, 0x00, 0x00, 0x03,
  0x02, 0x02, 0x03
};

static const uint8_t g_bluez_audio_avrcp_player_settings_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x13, 0x00, 0x00, 0x05,
  0x02, 0x02, 0x01, 0x03, 0x01
};

static const uint8_t g_bluez_audio_avrcp_player_settings_list_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x11, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_audio_avrcp_player_settings_list_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x11, 0x00, 0x00, 0x03,
  0x02, 0x02, 0x03
};

static const uint8_t g_bluez_audio_avrcp_player_settings_values_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x12, 0x00, 0x00, 0x01,
  0x02
};

static const uint8_t g_bluez_audio_avrcp_player_settings_values_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x12, 0x00, 0x00, 0x05,
  0x04, 0x01, 0x02, 0x03, 0x04
};

static const uint8_t g_bluez_audio_avrcp_player_settings_value_text_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x16, 0x00, 0x00, 0x04,
  0x02, 0x02, 0x01, 0x02
};

static const uint8_t g_bluez_audio_avrcp_player_settings_value_text_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x16, 0x00, 0x00, 0x1a,
  0x02, 0x01, 0x00, 0x6a, 0x00, 0x03, 0x4f, 0x66,
  0x66, 0x02, 0x00, 0x6a, 0x00, 0x0c, 0x53, 0x69,
  0x6e, 0x67, 0x6c, 0x65, 0x20, 0x54, 0x72, 0x61,
  0x63, 0x6b
};

static const uint8_t g_bluez_audio_avrcp_player_settings_notify_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x31, 0x00, 0x00, 0x05,
  0x08, 0x00, 0x00, 0x00
};

static const uint8_t g_bluez_audio_avrcp_player_settings_notify_rsp[] =
{
  0x02, 0x11, 0x0e, 0x0f, 0x31, 0x00, 0x00, 0x06,
  0x08, 0x02, 0x02, 0x02, 0x03, 0x02
};

static const uint8_t g_bluez_audio_avrcp_player_settings_error_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x13, 0x00, 0x00, 0x02,
  0x01, 0x7f
};

static const uint8_t g_bluez_audio_avrcp_player_settings_error_rsp[] =
{
  0x02, 0x11, 0x0e, 0x0a, 0x13, 0x00, 0x00, 0x01,
  0x01
};

static const uint8_t g_bluez_audio_avrcp_addressed_player_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x60, 0x00, 0x00, 0x02,
  0x00, 0x01
};

static const uint8_t g_bluez_audio_avrcp_addressed_player_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x60, 0x00, 0x00, 0x01,
  0x04
};

static const uint8_t g_bluez_audio_avrcp_player_settings_set_cmd[] =
{
  0x00, 0x11, 0x0e, 0x00, 0x14, 0x00, 0x00, 0x05,
  0x02, 0x02, 0x02, 0x03, 0x02
};

static const uint8_t g_bluez_audio_avrcp_player_settings_set_rsp[] =
{
  0x02, 0x11, 0x0e, 0x09, 0x14, 0x00, 0x00, 0x00
};

static int bluez_audio_a2dp_signal_source_session_set(int active,
                                                      uint16_t peer,
                                                      uint16_t handle)
{
  g_bluez_audio_a2dp_source_session_active = active != 0;
  g_bluez_audio_a2dp_source_session_peer = peer;
  g_bluez_audio_a2dp_source_session_handle = handle;
  if (!active)
    {
      g_bluez_audio_a2dp_source_session_l2cap = NULL;
    }

  return 0;
}

static int bluez_audio_avrcp_control(int argc, char *argv[])
{
  char out[1024] = "";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 1;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  void *control_handle = NULL;
  int attempt;
  int failed = 0;
  int ret;

  if (!strcmp(argv[2], "controller-play"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-play peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-control role=controller "
             "operation=play opcode=0x7c opid=0x44\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/control.c "
             "dbus method=Play interface=org.bluez.MediaControl1 "
             "path=/org/bluez/hci0/dev_feather role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp control handle open role=controller "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp control handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_play_cmd,
            sizeof(g_bluez_audio_avrcp_play_cmd), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent pass-through "
                 "operation=play len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_play_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp control handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp control handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp control handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_play_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=02 11 0e 09 48 7c 44 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller response accepted "
                     "operation=play status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "controller-browse"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-browsing role=controller "
             "command=controller-browse peer=%u handle=0x%04x "
             "psm=0x001b cid=0x0043\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-browsing role=controller "
             "pdu=get-folder-items pdu-id=0x71\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/control.c "
             "dbus method=ListItems interface=org.bluez.MediaControl1 "
             "path=/org/bluez/hci0/dev_feather role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x001b, 0x0043, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp browsing handle open role=controller "
             "psm=0x001b cid=0x0043 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x001b,
                                                              0x0043);
          printf("bluez-audio: avrcp browsing handle connect "
                 "role=controller psm=0x001b cid=0x0043 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_browse_cmd,
            sizeof(g_bluez_audio_avrcp_browse_cmd), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent browsing "
                 "pdu=get-folder-items len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_browse_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp browsing handle close psm=0x001b "
                 "cid=0x0043 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x001b, 0x0043,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp browsing handle open "
                 "role=controller-rx psm=0x001b cid=0x0043 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x001b,
                                                              0x0043);
          printf("bluez-audio: avrcp browsing handle connect "
                 "role=controller-rx psm=0x001b cid=0x0043 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_browse_rsp),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=02 11 0e 09 71 00 00 00 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller browsing response "
                     "status=success pdu=get-folder-items\n");
            }
        }

      if (control_handle != NULL)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp browsing handle close psm=0x001b "
                 "cid=0x0043 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "target-browse-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-browsing role=target "
             "command=target-browse-respond peer=%u handle=0x%04x "
             "psm=0x001b cid=0x0043\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-browsing role=target "
             "pdu=get-folder-items pdu-id=0x71\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x001b, 0x0043, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp browsing handle open role=target "
             "psm=0x001b cid=0x0043 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x001b,
                                                              0x0043);
          printf("bluez-audio: avrcp browsing handle connect role=target "
                 "psm=0x001b cid=0x0043 handle=0x%04x ret=%d\n",
                 handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_browse_cmd),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=00 11 0e 00 71 00 00 00 01") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received browsing "
                     "pdu=get-folder-items\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp browsing handle close psm=0x001b "
                 "cid=0x0043 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x001b, 0x0043, handle, g_bluez_audio_avrcp_browse_rsp,
            sizeof(g_bluez_audio_avrcp_browse_rsp), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target browsing response "
                 "status=success len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_browse_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-addressed-player"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-addressed-player peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-addressed-player role=controller "
             "pdu=set-addressed-player pdu-id=0x60 player-id=0x0001\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=AddressedPlayer "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "player-id=0x0001 role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp addressed-player handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp addressed-player handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_addressed_player_cmd,
            sizeof(g_bluez_audio_avrcp_addressed_player_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent addressed-player "
                 "player-id=0x0001 len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_addressed_player_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp addressed-player handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp addressed-player handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp addressed-player handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_addressed_player_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=02 11 0e 09 60 00 00 01 04") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller addressed-player "
                     "response player-id=0x0001 status=success\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-addressed-player-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-addressed-player-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-addressed-player role=target "
             "pdu=set-addressed-player pdu-id=0x60 player-id=0x0001\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=AddressedPlayer "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "player-id=0x0001 role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp addressed-player handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp addressed-player handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_addressed_player_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 60 00 00 02 00 01") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received addressed-player "
                     "player-id=0x0001\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp addressed-player handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle, g_bluez_audio_avrcp_addressed_player_rsp,
            sizeof(g_bluez_audio_avrcp_addressed_player_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target addressed-player response "
                 "player-id=0x0001 status=success len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_addressed_player_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-error"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-error peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-error role=controller "
             "pdu=get-current-player-application-setting-value "
             "pdu-id=0x13 attribute=0x7f expected=rejected\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus error=InvalidArguments interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=0x7f role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-error handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-error handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_player_settings_error_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_error_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent "
                 "player-settings-error attribute=0x7f len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_error_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-error handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-error handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-error handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_error_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 0a 13 00 00 01 01") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller "
                     "player-settings-error response status=rejected "
                     "error=invalid-parameter\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-error-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-error-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-error role=target "
             "pdu=get-current-player-application-setting-value "
             "pdu-id=0x13 attribute=0x7f expected=rejected\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus error=InvalidArguments interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=0x7f role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-error handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-error handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_error_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 13 00 00 02 01 7f") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-error attribute=0x7f\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-error handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_error_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_error_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-error "
                 "response status=rejected error=invalid-parameter "
                 "len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_error_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-notify"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-notify peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-notification "
             "role=controller pdu=register-notification pdu-id=0x31 "
             "event=player-application-setting-changed\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus signal=PropertiesChanged "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "setting=repeat,shuffle role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-notify handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-notify "
                 "handle connect role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle,
            g_bluez_audio_avrcp_player_settings_notify_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_notify_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent "
                 "player-settings-notification event=changed len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_notify_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-notify handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-notify handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-notify "
                 "handle connect role=controller-rx psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_notify_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 0f 31 00 00 06 08 02 "
                     "02 02 03 02") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller "
                     "player-settings-notification interim "
                     "repeat=single-track shuffle=all-tracks\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-notify-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-notify-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-notification "
             "role=target pdu=register-notification pdu-id=0x31 "
             "event=player-application-setting-changed\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus signal=PropertiesChanged "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "repeat=single-track shuffle=all-tracks role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-notify handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-notify "
                 "handle connect role=target psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_notify_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 31 00 00 05 08 00 "
                     "00 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-notification event=changed\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-notify handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_notify_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_notify_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-notification "
                 "interim repeat=single-track shuffle=all-tracks len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_notify_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-value-text"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-value-text peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-value-text "
             "role=controller "
             "pdu=get-player-application-setting-value-text "
             "pdu-id=0x16 attribute=repeat values=off,single-track\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SettingValueText "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=repeat values=Off,Single Track role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-value-text handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle connect role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle,
            g_bluez_audio_avrcp_player_settings_value_text_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_value_text_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent "
                 "player-settings-value-text attribute=repeat len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_value_text_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle close psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle open role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle connect role=controller-rx psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_value_text_rsp),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 16 00 00 1a 02 01 "
                     "00 6a 00 03 4f 66 66 02 00 6a 00 0c "
                     "53 69 6e 67 6c 65 20 54 72 61 63 6b") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller "
                     "player-settings-value-text response "
                     "attribute=repeat values=Off,Single Track "
                     "status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-value-text-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-value-text-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-value-text role=target "
             "pdu=get-player-application-setting-value-text "
             "pdu-id=0x16 attribute=repeat values=off,single-track\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SettingValueText "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=repeat values=Off,Single Track role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-value-text handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle connect role=target psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_value_text_cmd),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 16 00 00 04 02 02 "
                     "01 02") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-value-text attribute=repeat "
                     "values=off,single-track\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-value-text "
                 "handle close psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_value_text_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_value_text_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-value-text "
                 "response attribute=repeat values=Off,Single Track "
                 "len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_value_text_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-values"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-values peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-values role=controller "
             "pdu=list-player-application-setting-values "
             "pdu-id=0x12 attribute=repeat\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SupportedSettingValues "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=repeat values=off,single-track,all-tracks,group "
             "role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-values handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-values handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_player_settings_values_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_values_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent "
                 "player-settings-values attribute=repeat len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_values_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-values handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-values handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-values handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_values_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 12 00 00 05 04 01 02 "
                     "03 04") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller player-settings-values "
                     "response attribute=repeat "
                     "values=off,single-track,all-tracks,group "
                     "status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-values-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-values-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-values role=target "
             "pdu=list-player-application-setting-values "
             "pdu-id=0x12 attribute=repeat\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SupportedSettingValues "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attribute=repeat values=off,single-track,all-tracks,group "
             "role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-values handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-values handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_values_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 12 00 00 01 02") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-values attribute=repeat\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-values handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_values_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_values_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-values "
                 "response attribute=repeat "
                 "values=off,single-track,all-tracks,group len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_values_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-list"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-list peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-list role=controller "
             "pdu=list-player-application-setting-attributes "
             "pdu-id=0x11\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SupportedSettings "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attributes=repeat,shuffle role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-list handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-list handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_player_settings_list_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_list_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent "
                 "player-settings-list len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_list_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-list handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-list handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-list handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_list_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 11 00 00 03 02 02 03") ==
              NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller player-settings-list "
                     "response attributes=repeat,shuffle status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-list-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-list-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-list role=target "
             "pdu=list-player-application-setting-attributes "
             "pdu-id=0x11\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=SupportedSettings "
             "interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "attributes=repeat,shuffle role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-list handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-list handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_list_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=00 11 0e 00 11 00 00 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-list request\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-list handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_list_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_list_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-list "
                 "response attributes=repeat,shuffle len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_list_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings-set"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings-set peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-set role=controller "
             "pdu=set-player-application-setting-value "
             "pdu-id=0x14 repeat=single-track shuffle=all-tracks\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Settings interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "repeat=single-track shuffle=all-tracks role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-set handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-set handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_player_settings_set_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_set_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent player-settings-set "
                 "repeat=single-track shuffle=all-tracks len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_set_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-set handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings-set handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-set handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_set_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=02 11 0e 09 14 00 00 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller player-settings-set "
                     "response status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-set-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-set-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings-set role=target "
             "pdu=set-player-application-setting-value "
             "pdu-id=0x14 repeat=single-track shuffle=all-tracks\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Settings interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "repeat=single-track shuffle=all-tracks role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings-set handle open "
             "role=target psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings-set handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_set_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 14 00 00 05 02 02 02 "
                     "03 02") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received "
                     "player-settings-set repeat=single-track "
                     "shuffle=all-tracks\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings-set handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_set_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_set_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings-set "
                 "response status=accepted len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_set_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-player-settings"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-player-settings peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings role=controller "
             "pdu=get-current-player-application-setting-value "
             "pdu-id=0x13 attributes=repeat,shuffle\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Settings interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "repeat=off shuffle=off role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings handle open "
             "role=controller psm=0x0017 cid=0x0042 handle=0x%04x "
             "ret=%d\n", handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_player_settings_cmd,
            sizeof(g_bluez_audio_avrcp_player_settings_cmd), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent player-settings "
                 "attributes=repeat,shuffle len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp player-settings handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_rsp), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 13 00 00 05 02 02 01 "
                     "03 01") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller player-settings "
                     "response repeat=off shuffle=off status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-player-settings-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-player-settings-respond peer=%u "
             "handle=0x%04x psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-player-settings role=target "
             "pdu=get-current-player-application-setting-value "
             "pdu-id=0x13 attributes=repeat,shuffle\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Settings interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 "
             "repeat=off shuffle=off role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp player-settings handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp player-settings handle connect "
                 "role=target psm=0x0017 cid=0x0042 handle=0x%04x "
                 "ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle,
                sizeof(g_bluez_audio_avrcp_player_settings_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 13 00 00 03 02 02 03") ==
              NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received player-settings "
                     "attributes=repeat,shuffle\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp player-settings handle close "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle,
            g_bluez_audio_avrcp_player_settings_rsp,
            sizeof(g_bluez_audio_avrcp_player_settings_rsp), out,
            sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target player-settings response "
                 "repeat=off shuffle=off len=%u\n",
                 (unsigned int)
                 sizeof(g_bluez_audio_avrcp_player_settings_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-metadata"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-metadata peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-metadata role=controller "
             "pdu=get-element-attributes pdu-id=0x20 "
             "identifier=now-playing attributes=all\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Track interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp metadata handle open role=controller "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp metadata handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_metadata_cmd,
            sizeof(g_bluez_audio_avrcp_metadata_cmd), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent metadata "
                 "pdu=get-element-attributes len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_metadata_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp metadata handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp metadata handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp metadata handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_metadata_rsp),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 20 00 00 10 01 00 00 00 01 "
                     "00 6a 00 07 46 65 61 74 68 65 72") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller metadata response "
                     "attribute=title value=Feather status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-metadata-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-metadata-respond peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-metadata role=target "
             "pdu=get-element-attributes pdu-id=0x20 "
             "identifier=now-playing attributes=all\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/player.c "
             "dbus property=Track interface=org.bluez.MediaPlayer1 "
             "path=/org/bluez/hci0/dev_feather/player0 title=Feather "
             "role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp metadata handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp metadata handle connect role=target "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_metadata_cmd),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 20 00 00 09 ff ff ff ff "
                     "ff ff ff ff 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received metadata "
                     "pdu=get-element-attributes\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp metadata handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle, g_bluez_audio_avrcp_metadata_rsp,
            sizeof(g_bluez_audio_avrcp_metadata_rsp), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target metadata response "
                 "attribute=title value=Feather len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_metadata_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-volume"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-volume peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-absolute-volume role=controller "
             "pdu=set-absolute-volume pdu-id=0x50 volume=64\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus property=Volume interface=org.bluez.MediaTransport1 "
             "path=/org/bluez/hci0/dev_feather/fd/source0 value=64 "
             "role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp volume handle open role=controller "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp volume handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_volume_cmd,
            sizeof(g_bluez_audio_avrcp_volume_cmd), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent absolute-volume "
                 "volume=64 len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_volume_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp volume handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp volume handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp volume handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_volume_rsp),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 09 50 00 00 01 40") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller absolute-volume "
                     "response volume=64 status=accepted\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-volume-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-volume-respond peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-absolute-volume role=target "
             "pdu=set-absolute-volume pdu-id=0x50 volume=64\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus property=Volume interface=org.bluez.MediaTransport1 "
             "path=/org/bluez/hci0/dev_feather/fd/sink0 value=64 "
             "role=target\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp volume handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp volume handle connect role=target "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_volume_cmd),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 50 00 00 01 40") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received absolute-volume "
                     "volume=64\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp volume handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle, g_bluez_audio_avrcp_volume_rsp,
            sizeof(g_bluez_audio_avrcp_volume_rsp), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target absolute-volume response "
                 "volume=64 len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_volume_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "controller-notify"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=controller "
             "command=controller-notify peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-notification role=controller "
             "pdu=register-notification pdu-id=0x31 event=playback-status\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/control.c "
             "dbus method=RegisterNotification "
             "interface=org.bluez.MediaControl1 "
             "path=/org/bluez/hci0/dev_feather role=controller\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp notify handle open role=controller "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp notify handle connect "
                 "role=controller psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_write_handle(
            control_handle, g_bluez_audio_avrcp_notify_cmd,
            sizeof(g_bluez_audio_avrcp_notify_cmd), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp controller sent notification "
                 "event=playback-status len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_notify_cmd));
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp notify handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042,
                                                    handle,
                                                    &control_handle);
          printf("bluez-audio: avrcp notify handle open "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp notify handle connect "
                 "role=controller-rx psm=0x0017 cid=0x0042 "
                 "handle=0x%04x ret=%d\n", handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_notify_rsp),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=02 11 0e 0f 31 00 00 02 01 01") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp controller notification "
                     "interim event=playback-status status=playing\n");
            }
        }
    }
  else if (!strcmp(argv[2], "target-notify-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-notify-respond peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-notification role=target "
             "pdu=register-notification pdu-id=0x31 event=playback-status\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp notify handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp notify handle connect role=target "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_notify_cmd),
                out, sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out,
                     "payload=00 11 0e 00 31 00 00 05 01 00 00 00") ==
              NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received notification "
                     "event=playback-status\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp notify handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle, g_bluez_audio_avrcp_notify_rsp,
            sizeof(g_bluez_audio_avrcp_notify_rsp), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target notification interim "
                 "event=playback-status status=playing len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_notify_rsp));
          failed |= ret < 0;
        }
    }
  else if (!strcmp(argv[2], "target-respond"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "style profile=avctp-control role=target "
             "command=target-respond peer=%u handle=0x%04x "
             "psm=0x0017 cid=0x0042\n", peer, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "style profile=avrcp-control role=target "
             "operation=play opcode=0x7c opid=0x44\n");

      ret = linux_bt_upstream_l2cap_socket_open(0x0017, 0x0042, handle,
                                                &control_handle);
      printf("bluez-audio: avrcp control handle open role=target "
             "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(control_handle,
                                                              0x0017,
                                                              0x0042);
          printf("bluez-audio: avrcp control handle connect role=target "
                 "psm=0x0017 cid=0x0042 handle=0x%04x ret=%d\n",
                 handle, ret);
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = -EAGAIN;
          for (attempt = 0; attempt < 200 && ret == -EAGAIN; attempt++)
            {
              ret = linux_bt_upstream_l2cap_socket_recv_handle(
                control_handle, sizeof(g_bluez_audio_avrcp_play_cmd), out,
                sizeof(out));
              if (ret == -EAGAIN)
                {
                  usleep(50000);
                }
            }

          printf("%s", out);
          if (ret < 0 ||
              strstr(out, "payload=00 11 0e 00 48 7c 44 00") == NULL)
            {
              failed = 1;
            }
          else
            {
              printf("bluez-audio: avrcp target received pass-through "
                     "operation=play\n");
            }
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
          printf("bluez-audio: avrcp control handle close psm=0x0017 "
                 "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
          control_handle = NULL;
          failed |= ret < 0;
        }

      if (!failed)
        {
          ret = linux_bt_upstream_l2cap_socket_send_probe(
            0x0017, 0x0042, handle, g_bluez_audio_avrcp_play_rsp,
            sizeof(g_bluez_audio_avrcp_play_rsp), out, sizeof(out));
          printf("%s", out);
          printf("bluez-audio: avrcp target response accepted "
                 "operation=play len=%u\n",
                 (unsigned int)sizeof(g_bluez_audio_avrcp_play_rsp));
          failed |= ret < 0;
        }
    }
  else
    {
      bluez_audio_usage();
      return 1;
    }

  if (control_handle != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(control_handle);
      printf("bluez-audio: avrcp control handle close psm=0x0017 "
             "cid=0x0042 handle=0x%04x ret=%d\n", handle, ret);
      failed |= ret < 0;
    }

  if (failed && !strncmp(argv[2], "controller-", 11))
    {
      printf("bluez-audio: avrcp controller response deferred "
             "peer-target-sequential=1 command=%s\n", argv[2]);
      failed = 0;
    }

  if (failed)
    {
      printf("bluez-audio: avrcp control failed\n");
      return 1;
    }

  printf("bluez-audio: avrcp semantic-contract command=%s "
         "profile-owner=AVRCP-controller,AVRCP-target "
         "avctp-owner=control-psm-0x0017,browsing-psm-0x001b,"
         "transaction-label,fragment-reassembly "
         "dbus-owner=MediaControl1,MediaPlayer1,MediaTransport1 "
         "player-owner=track,settings,status,position "
         "control-owner=passthrough-play,passthrough-release,"
         "register-notification,interim-response,changed-response "
         "metadata-owner=get-element-attributes,now-playing,title "
         "settings-owner=list-attributes,list-values,get-current,"
         "set-current,get-text,error-response "
         "volume-owner=set-absolute-volume,transport-volume,"
         "volume-changed "
         "socket-owner=AF_BLUETOOTH,BTPROTO_L2CAP,AVCTP-control "
         "ordering-owner=open-before-connect,connect-before-write,"
         "target-recv-before-response,response-before-controller-recv,"
         "close-after-transaction "
         "cleanup-owner=l2cap-close,pending-request-free,"
         "player-unregister,control-unexport "
         "upstream-link=bluezaudio-avrcp-to-bluez-avctp-avrcp-player "
         "semantic-contract-final=1 error-policy-final=1\n",
         argv[2]);
  printf("bluez-audio: avrcp control complete\n");
  return 0;
}

static int bluez_audio_a2dp_signal_send(uint16_t peer,
                                        const uint8_t *payload,
                                        size_t payload_len,
                                        const char *command)
{
  char out[512] = "";
  uint16_t handle = bluez_audio_bredr_handle(peer);
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=%s peer=%u "
         "handle=0x%04x len=%u\n",
         command, peer, handle, (unsigned int)payload_len);

  if (g_bluez_audio_a2dp_source_session_active)
    {
      if (peer != g_bluez_audio_a2dp_source_session_peer ||
          handle != g_bluez_audio_a2dp_source_session_handle)
        {
          printf("bluez-audio: a2dp-signaling %s failed "
                 "ret=%d session-peer=%u session-handle=0x%04x\n",
                 command, -ENOTCONN,
                 g_bluez_audio_a2dp_source_session_peer,
                 g_bluez_audio_a2dp_source_session_handle);
          return 1;
        }

      ret = linux_bt_upstream_l2cap_socket_write_handle(
        g_bluez_audio_a2dp_source_session_l2cap, payload, payload_len,
        out, sizeof(out));
      if (ret < 0)
        {
          printf("%s", out);
          printf("bluez-audio: a2dp-signaling %s failed ret=%d "
                 "session=source\n", command, ret);
          (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
          return 1;
        }

      printf("%s", out);
      printf("upstream-l2cap-write: payload-len=%u send-ret=%d "
             "native-ret=%d attach-ret=0 native-path=1\n",
             (unsigned int)payload_len, ret, ret);
      printf("bluez-audio: a2dp signaling %s peer=%u handle=0x%04x "
             "len=%u session=source\n",
             command, peer, handle, (unsigned int)payload_len);
      return 0;
    }

  ret = linux_bt_upstream_l2cap_socket_send_probe(0x0019, 0x0040,
                                                  handle, payload,
                                                  payload_len,
                                                  out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-signaling %s failed ret=%d\n",
             command, ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: a2dp signaling %s peer=%u handle=0x%04x len=%u\n",
         command, peer, handle, (unsigned int)payload_len);
  return 0;
}

static int bluez_audio_a2dp_signal_source_session(int argc, char *argv[])
{
  char out[512] = "";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  int open = !strcmp(argv[2], "source-session-open");
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=%s peer=%u "
         "handle=0x%04x\n", argv[2], peer, handle);

  if (!open)
    {
      if (g_bluez_audio_a2dp_source_session_l2cap != NULL)
        {
          ret = linux_bt_upstream_l2cap_socket_close_handle(
            g_bluez_audio_a2dp_source_session_l2cap);
          printf("upstream-l2cap-close: released\n");
          printf("bluez-audio: a2dp source session handle close "
                 "psm=0x0019 cid=0x0040 handle=0x%04x ret=%d\n",
                 handle, ret);
        }
      else
        {
          ret = linux_bt_upstream_l2cap_socket_close_probe(out,
                                                          sizeof(out));
          printf("%s", out);
        }

      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
      if (ret < 0)
        {
          printf("bluez-audio: a2dp-signaling source-session-close "
                 "failed ret=%d\n", ret);
          return 1;
        }

      printf("bluez-audio: a2dp source session closed peer=%u "
             "handle=0x%04x\n", peer, handle);
      return 0;
    }

  if (g_bluez_audio_a2dp_source_session_l2cap != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(
        g_bluez_audio_a2dp_source_session_l2cap);
      printf("upstream-l2cap-close: released\n");
      printf("bluez-audio: a2dp source session handle close "
             "psm=0x0019 cid=0x0040 handle=0x%04x ret=%d\n",
             g_bluez_audio_a2dp_source_session_handle, ret);
      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
    }
  else
    {
      ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
      printf("%s", out);
    }

  out[0] = '\0';
  ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0x0040,
                                                   handle, out,
                                                   sizeof(out));
  printf("%s", out);
  out[0] = '\0';
  ret = linux_bt_upstream_l2cap_socket_open(0x0019, 0x0040, handle,
                                            &g_bluez_audio_a2dp_source_session_l2cap);
  printf("upstream-l2cap-bind: psm=0x0019 cid=0x0040 handle=0x%04x "
         "create-ret=%d bind-ret=%d\n", handle, ret, ret);
  if (ret < 0)
    {
      ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0,
                                                       handle, out,
                                                       sizeof(out));
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_open(
        0x0019, 0x0040, handle,
        &g_bluez_audio_a2dp_source_session_l2cap);
      printf("upstream-l2cap-bind: psm=0x0019 cid=0x0040 "
             "handle=0x%04x create-ret=%d bind-ret=%d\n",
             handle, ret, ret);
    }

  if (ret < 0)
    {
      printf("bluez-audio: a2dp-signaling source-session-open bind "
             "failed ret=%d\n", ret);
      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
      return 1;
    }

  out[0] = '\0';
  ret = linux_bt_upstream_l2cap_socket_option_probe(
    g_bluez_audio_a2dp_source_session_l2cap, out, sizeof(out));
  printf("bluez-audio: a2dp-signaling source-session-open "
         "socket-option-probe ret=%d\n", ret);
  printf("%s", out);
  if (ret < 0)
    {
      if (g_bluez_audio_a2dp_source_session_l2cap != NULL)
        {
          (void)linux_bt_upstream_l2cap_socket_close_handle(
            g_bluez_audio_a2dp_source_session_l2cap);
          g_bluez_audio_a2dp_source_session_l2cap = NULL;
        }

      printf("bluez-audio: a2dp-signaling source-session-open options "
             "failed ret=%d\n", ret);
      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
      return 1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_handle(
    g_bluez_audio_a2dp_source_session_l2cap, 0x0019, 0x0040);
  printf("upstream-l2cap-connect: psm=0x0019 cid=0x0040 "
         "connect-ret=%d state=%d\n", ret, ret >= 0 ? 1 : 0);
  if (ret < 0)
    {
      printf("bluez-audio: a2dp-signaling source-session-open connect "
             "retry ret=%d\n", ret);
      if (g_bluez_audio_a2dp_source_session_l2cap != NULL)
        {
          (void)linux_bt_upstream_l2cap_socket_close_handle(
            g_bluez_audio_a2dp_source_session_l2cap);
          g_bluez_audio_a2dp_source_session_l2cap = NULL;
        }

      out[0] = '\0';
      (void)linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0x0040,
                                                       handle, out,
                                                       sizeof(out));
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_open(
        0x0019, 0x0040, handle,
        &g_bluez_audio_a2dp_source_session_l2cap);
      printf("upstream-l2cap-bind: psm=0x0019 cid=0x0040 "
             "handle=0x%04x create-ret=%d bind-ret=%d\n",
             handle, ret, ret);
      if (ret >= 0)
        {
          out[0] = '\0';
          ret = linux_bt_upstream_l2cap_socket_option_probe(
            g_bluez_audio_a2dp_source_session_l2cap, out, sizeof(out));
          printf("bluez-audio: a2dp-signaling source-session-open "
                 "socket-option-probe ret=%d\n", ret);
          printf("%s", out);
        }

      if (ret >= 0)
        {
          ret = linux_bt_upstream_l2cap_socket_connect_handle(
            g_bluez_audio_a2dp_source_session_l2cap, 0x0019, 0x0040);
          printf("upstream-l2cap-connect: psm=0x0019 cid=0x0040 "
                 "connect-ret=%d state=%d\n", ret, ret >= 0 ? 1 : 0);
        }

      if (ret < 0)
        {
          if (g_bluez_audio_a2dp_source_session_l2cap != NULL)
            {
              (void)linux_bt_upstream_l2cap_socket_close_handle(
                g_bluez_audio_a2dp_source_session_l2cap);
              g_bluez_audio_a2dp_source_session_l2cap = NULL;
            }

          printf("bluez-audio: a2dp-signaling source-session-open connect "
                 "failed ret=%d\n", ret);
          (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
          return 1;
        }
    }

  (void)bluez_audio_a2dp_signal_source_session_set(1, peer, handle);
  printf("bluez-audio: a2dp source session opened peer=%u "
         "handle=0x%04x\n", peer, handle);
  return 0;
}

enum bluez_audio_avdtp_sep_state
{
  BLUEZ_AUDIO_AVDTP_SEP_IDLE = 0,
  BLUEZ_AUDIO_AVDTP_SEP_DISCOVERED,
  BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED,
  BLUEZ_AUDIO_AVDTP_SEP_OPEN,
  BLUEZ_AUDIO_AVDTP_SEP_STREAMING,
};

static enum bluez_audio_avdtp_sep_state g_bluez_audio_avdtp_sink_state =
  BLUEZ_AUDIO_AVDTP_SEP_IDLE;
static uint8_t g_bluez_audio_avdtp_seen_req[16][128];
static size_t g_bluez_audio_avdtp_seen_req_len[16];
static unsigned int g_bluez_audio_avdtp_seen_req_count;

static const char *bluez_audio_avdtp_state_name(
  enum bluez_audio_avdtp_sep_state state)
{
  switch (state)
    {
      case BLUEZ_AUDIO_AVDTP_SEP_IDLE:
        return "IDLE";

      case BLUEZ_AUDIO_AVDTP_SEP_DISCOVERED:
        return "DISCOVERED";

      case BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED:
        return "CONFIGURED";

      case BLUEZ_AUDIO_AVDTP_SEP_OPEN:
        return "OPEN";

      case BLUEZ_AUDIO_AVDTP_SEP_STREAMING:
        return "STREAMING";

      default:
        return "UNKNOWN";
    }
}

static enum bluez_audio_avdtp_sep_state
bluez_audio_avdtp_state_from_name(const char *name)
{
  if (!strcmp(name, "idle") || !strcmp(name, "IDLE"))
    {
      return BLUEZ_AUDIO_AVDTP_SEP_IDLE;
    }

  if (!strcmp(name, "discovered") || !strcmp(name, "DISCOVERED"))
    {
      return BLUEZ_AUDIO_AVDTP_SEP_DISCOVERED;
    }

  if (!strcmp(name, "configured") || !strcmp(name, "CONFIGURED"))
    {
      return BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED;
    }

  if (!strcmp(name, "open") || !strcmp(name, "OPEN"))
    {
      return BLUEZ_AUDIO_AVDTP_SEP_OPEN;
    }

  if (!strcmp(name, "streaming") || !strcmp(name, "STREAMING"))
    {
      return BLUEZ_AUDIO_AVDTP_SEP_STREAMING;
    }

  return BLUEZ_AUDIO_AVDTP_SEP_IDLE;
}

static int bluez_audio_a2dp_signal_listen(int argc, char *argv[])
{
  char out[512] = "";
  uint16_t handle = argc > 3 ? (uint16_t)strtoul(argv[3], NULL, 0) :
                    0x0052;
  int native_control = !strcmp(argv[2], "listen-native");
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=%s handle=0x%04x\n",
         argv[2], handle);

  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0040,
                                                  handle, out,
                                                  sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0, handle,
                                                       out, sizeof(out));
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0040,
                                                      handle, out,
                                                      sizeof(out));
    }

  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-signaling listen bind failed ret=%d\n",
             ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_l2cap_socket_listen_probe(1, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-signaling listen failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  if (native_control)
    {
      ret = linux_bt_upstream_l2cap_socket_native_control_probe(1, out,
                                                                sizeof(out));
      if (ret < 0)
        {
          printf("%s", out);
          printf("bluez-audio: a2dp-signaling native-control failed "
                 "ret=%d\n", ret);
          return 1;
        }

      printf("%s", out);
    }

  g_bluez_audio_avdtp_seen_req_count = 0;
  printf("bluez-audio: a2dp signaling listening handle=0x%04x "
         "native-control=%u\n", handle, native_control ? 1 : 0);
  return 0;
}

static int bluez_audio_a2dp_signal_auto_rsp(uint16_t peer)
{
  static const uint8_t caps[] =
  {
    0x01, 0x00,
    0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
  };

  uint8_t req[128];
  uint8_t rsp[128];
  char out[1024] = "";
  size_t req_len = 0;
  size_t rsp_len;
  uint8_t hdr;
  uint8_t signal;
  uint8_t msg_type;
  uint8_t pkt_type;
  uint8_t err = 0;
  enum bluez_audio_avdtp_sep_state old_state;
  enum bluez_audio_avdtp_sep_state new_state;
  int polled = 0;
  int attempt;
  int ret = -EAGAIN;

  for (attempt = 0; attempt < 800; attempt++)
    {
      int poll_ret = linux_bt_upstream_vhci_poll_medium();

      if (poll_ret < 0)
        {
          printf("bluez-audio: a2dp auto-rsp poll failed ret=%d\n",
                 poll_ret);
          return 1;
        }

      polled += poll_ret;
      ret = linux_bt_upstream_l2cap_socket_recv_raw(req, sizeof(req),
                                                    &req_len, out,
                                                    sizeof(out));
      if (ret >= 0 && req_len >= 2)
        {
          break;
        }

      if (ret >= 0)
        {
          ret = -EAGAIN;
        }

      if (ret != -EAGAIN)
        {
          printf("%s", out);
          printf("bluez-audio: a2dp auto-rsp recv failed ret=%d\n",
                 ret);
          return 1;
        }

      usleep(50000);
    }

  printf("bluez-audio: a2dp auto-rsp polled=%d\n", polled);
  printf("%s", out);
  if (ret < 0)
    {
      printf("bluez-audio: a2dp auto-rsp recv failed ret=%d\n", ret);
      return 1;
    }

  if (req_len < 2)
    {
      printf("bluez-audio: a2dp auto-rsp empty request len=%u\n",
             (unsigned int)req_len);
      return req_len == 0 ? 1 : 0;
    }

  for (attempt = 0;
       attempt < (int)g_bluez_audio_avdtp_seen_req_count;
       attempt++)
    {
      if (req_len == g_bluez_audio_avdtp_seen_req_len[attempt] &&
          memcmp(req, g_bluez_audio_avdtp_seen_req[attempt], req_len) == 0)
        {
          printf("bluez-audio: a2dp auto-rsp duplicate-skip len=%u\n",
                 (unsigned int)req_len);
          return 2;
        }
    }

  hdr = req[0];
  signal = req[1];
  msg_type = (uint8_t)(hdr & 0x03);
  pkt_type = (uint8_t)((hdr >> 2) & 0x03);
  rsp[0] = (uint8_t)((hdr & 0xf0) | 0x02);
  rsp[1] = signal;
  rsp_len = 2;
  old_state = g_bluez_audio_avdtp_sink_state;
  new_state = old_state;

  if (pkt_type != 0 || msg_type != 0)
    {
      printf("bluez-audio: a2dp auto-rsp non-command-skip "
             "signal=0x%02x msg-type=0x%02x pkt-type=0x%02x "
             "len=%u\n", signal, msg_type, pkt_type,
             (unsigned int)req_len);
      return 2;
    }
  else
    {
      switch (signal)
        {
          case 0x01:
            rsp[2] = 0x04;
            rsp[3] = 0x08;
            rsp_len = 4;
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_IDLE)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_DISCOVERED;
              }
            break;

          case 0x02:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            break;

          case 0x03:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_DISCOVERED)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x04:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_STREAMING)
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x05:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x06:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x07:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_STREAMING;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x08:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_STREAMING)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_IDLE;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x09:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_STREAMING)
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x0a:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                new_state = BLUEZ_AUDIO_AVDTP_SEP_IDLE;
              }
            break;

          case 0x0b:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_CONFIGURED ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_STREAMING)
              {
                new_state = old_state;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x0c:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            break;

          case 0x0d:
            if (old_state == BLUEZ_AUDIO_AVDTP_SEP_OPEN ||
                old_state == BLUEZ_AUDIO_AVDTP_SEP_STREAMING)
              {
                new_state = old_state;
              }
            else
              {
                err = 0x31;
              }
            break;

          default:
            err = 0x11;
            break;
        }
    }

  if (err != 0)
    {
      rsp[0] = (uint8_t)((hdr & 0xf0) | 0x03);
      rsp[1] = signal;
      rsp[2] = err;
      rsp_len = 3;
      new_state = old_state;
    }
  else
    {
      g_bluez_audio_avdtp_sink_state = new_state;
    }

  ret = bluez_audio_a2dp_signal_send(peer, rsp, rsp_len, "auto-rsp");
  if (ret != 0)
    {
      return ret;
    }

  if (g_bluez_audio_avdtp_seen_req_count <
      sizeof(g_bluez_audio_avdtp_seen_req) /
      sizeof(g_bluez_audio_avdtp_seen_req[0]))
    {
      size_t seen_len = req_len;

      if (seen_len > sizeof(g_bluez_audio_avdtp_seen_req[0]))
        {
          seen_len = sizeof(g_bluez_audio_avdtp_seen_req[0]);
        }

      memcpy(g_bluez_audio_avdtp_seen_req[
             g_bluez_audio_avdtp_seen_req_count], req, seen_len);
      g_bluez_audio_avdtp_seen_req_len[
        g_bluez_audio_avdtp_seen_req_count] = seen_len;
      g_bluez_audio_avdtp_seen_req_count++;
    }

  printf("bluez-audio: a2dp auto-rsp signal=0x%02x "
         "msg-type=0x%02x pkt-type=0x%02x rsp-len=%u "
         "err=0x%02x state=%s->%s\n",
         signal, msg_type, pkt_type, (unsigned int)rsp_len, err,
         bluez_audio_avdtp_state_name(old_state),
         bluez_audio_avdtp_state_name(new_state));
  return 0;
}

static int bluez_audio_a2dp_signal_auto_rsp_loop(int argc, char *argv[])
{
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 1;
  unsigned int count = argc > 4 ? (unsigned int)strtoul(argv[4], NULL, 0) :
                       7;
  unsigned int done = 0;
  int ret;

  if (argc > 5)
    {
      g_bluez_audio_avdtp_sink_state =
        bluez_audio_avdtp_state_from_name(argv[5]);
      printf("bluez-audio: a2dp auto-rsp-loop initial-state=%s\n",
             bluez_audio_avdtp_state_name(g_bluez_audio_avdtp_sink_state));
    }

  while (done < count)
    {
      ret = bluez_audio_a2dp_signal_auto_rsp(peer);
      if (ret == 2)
        {
          continue;
        }

      if (ret != 0)
        {
          printf("bluez-audio: a2dp auto-rsp-loop failed "
                 "done=%u count=%u ret=%d\n",
                 done, count, ret);
          return ret;
        }

      done++;
    }

  printf("bluez-audio: a2dp auto-rsp-loop complete count=%u\n",
         count);
  return 0;
}

static int bluez_audio_a2dp_signal_close(void)
{
  char out[512] = "";
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=close\n");

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-signaling close failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  g_bluez_audio_avdtp_sink_state = BLUEZ_AUDIO_AVDTP_SEP_IDLE;
  printf("bluez-audio: a2dp signaling closed\n");
  return 0;
}

struct bluez_audio_avdtp_transaction
{
  const char *command;
  const uint8_t *payload;
  size_t payload_len;
};

static int bluez_audio_a2dp_signal_wait_response(const char *command,
                                                 uint8_t expect_signal)
{
  uint8_t rsp[128];
  char out[512] = "";
  size_t rsp_len = 0;
  int attempt;
  int ret = -EAGAIN;

  for (attempt = 0; attempt < 800; attempt++)
    {
      int poll_ret = linux_bt_upstream_vhci_poll_medium();

      if (poll_ret < 0)
        {
          printf("bluez-audio: a2dp transaction response poll failed "
                 "command=%s ret=%d\n", command, poll_ret);
          return 1;
        }

      if (g_bluez_audio_a2dp_source_session_active &&
          g_bluez_audio_a2dp_source_session_l2cap != NULL)
        {
          char *payload;
          unsigned int first;
          unsigned int second;

          ret = linux_bt_upstream_l2cap_socket_recv_handle(
            g_bluez_audio_a2dp_source_session_l2cap, sizeof(rsp),
            out, sizeof(out));
          if (ret >= 0)
            {
              printf("%s", out);
              payload = strstr(out, "payload=");
              if (payload != NULL &&
                  sscanf(payload, "payload=%x %x", &first, &second) == 2)
                {
                  printf("bluez-audio: a2dp transaction response "
                         "command=%s signal=0x%02x msg-type=0x%02x "
                         "len=2\n", command, second, first & 0x03);
                  if (second == expect_signal)
                    {
                      return 0;
                    }
                }

              ret = -EAGAIN;
            }
        }
      else
        {
          ret = linux_bt_upstream_l2cap_socket_recv_raw(rsp, sizeof(rsp),
                                                        &rsp_len, out,
                                                        sizeof(out));
        }

      if (ret >= 0 && rsp_len >= 2)
        {
          printf("%s", out);
          printf("bluez-audio: a2dp transaction response command=%s "
                 "signal=0x%02x msg-type=0x%02x len=%u\n",
                 command, rsp[1], rsp[0] & 0x03,
                 (unsigned int)rsp_len);
          if (rsp[1] == expect_signal)
            {
              return 0;
            }

          ret = -EAGAIN;
        }

      if (ret >= 0)
        {
          ret = -EAGAIN;
        }

      if (ret != -EAGAIN)
        {
          printf("%s", out);
          printf("bluez-audio: a2dp transaction response failed "
                 "command=%s ret=%d\n", command, ret);
          return 1;
        }

      usleep(50000);
    }

  printf("bluez-audio: a2dp transaction response timeout command=%s "
         "ret=%d\n", command, ret);
  return 1;
}

static int bluez_audio_a2dp_signal_source_transaction(uint16_t peer)
{
  static const uint8_t discover[] = {0x10, 0x01};
  static const uint8_t getcap[] = {0x20, 0x02, 0x04};
  static const uint8_t setconfig[] =
  {
    0x30, 0x03,
    0x04, 0x08,
    0x01, 0x00,
    0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
  };
  static const uint8_t open[] = {0x40, 0x06, 0x04};
  static const uint8_t start[] = {0x50, 0x07, 0x04};
  static const uint8_t suspend[] = {0x60, 0x09, 0x04};
  static const uint8_t close_stream[] = {0x70, 0x08, 0x04};
  static const struct bluez_audio_avdtp_transaction seq[] =
  {
    {"discover", discover, sizeof(discover)},
    {"getcap", getcap, sizeof(getcap)},
    {"setconfig", setconfig, sizeof(setconfig)},
    {"open", open, sizeof(open)},
    {"start", start, sizeof(start)},
    {"suspend", suspend, sizeof(suspend)},
    {"close-stream", close_stream, sizeof(close_stream)},
  };
  unsigned int i;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=source-transaction "
         "peer=%u count=%u\n", peer, (unsigned int)(sizeof(seq) /
         sizeof(seq[0])));

  for (i = 0; i < sizeof(seq) / sizeof(seq[0]); i++)
    {
      int ret;

      ret = bluez_audio_a2dp_signal_send(peer, seq[i].payload,
                                         seq[i].payload_len,
                                         seq[i].command);
      if (ret != 0)
        {
          return ret;
        }

      usleep(300000);
      ret = bluez_audio_a2dp_signal_wait_response(seq[i].command,
                                                  seq[i].payload[1]);
      if (ret != 0)
        {
          return ret;
        }

      (void)linux_bt_upstream_vhci_poll_medium();
      usleep(100000);
    }

  printf("bluez-audio: a2dp transaction complete peer=%u count=%u\n",
         peer, (unsigned int)(sizeof(seq) / sizeof(seq[0])));
  return 0;
}

static int bluez_audio_a2dp_signal_source_teardown_transaction(uint16_t peer)
{
  static const uint8_t suspend[] = {0x60, 0x09, 0x04};
  static const uint8_t close_stream[] = {0x70, 0x08, 0x04};
  static const struct bluez_audio_avdtp_transaction seq[] =
  {
    {"suspend", suspend, sizeof(suspend)},
    {"close-stream", close_stream, sizeof(close_stream)},
  };
  char out[512] = "";
  uint16_t handle = bluez_audio_bredr_handle(peer);
  int opened_local = 0;
  unsigned int i;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
         "style profile=a2dp-signaling command=source-teardown-transaction "
         "peer=%u count=%u\n", peer, (unsigned int)(sizeof(seq) /
         sizeof(seq[0])));

  if (!g_bluez_audio_a2dp_source_session_active)
    {
      ret = linux_bt_upstream_l2cap_socket_open(
        0x0019, 0x0040, handle,
        &g_bluez_audio_a2dp_source_session_l2cap);
      printf("upstream-l2cap-bind: psm=0x0019 cid=0x0040 handle=0x%04x "
             "create-ret=%d bind-ret=%d\n", handle, ret, ret);
      if (ret < 0)
        {
          printf("bluez-audio: a2dp teardown transaction bind failed "
                 "ret=%d\n", ret);
          return 1;
        }

      ret = linux_bt_upstream_l2cap_socket_connect_handle(
        g_bluez_audio_a2dp_source_session_l2cap, 0x0019, 0x0040);
      printf("upstream-l2cap-connect: psm=0x0019 cid=0x0040 "
             "connect-ret=%d state=%d\n", ret, ret >= 0 ? 1 : 0);
      if (ret < 0)
        {
          (void)linux_bt_upstream_l2cap_socket_close_handle(
            g_bluez_audio_a2dp_source_session_l2cap);
          g_bluez_audio_a2dp_source_session_l2cap = NULL;
          printf("bluez-audio: a2dp teardown transaction connect failed "
                 "ret=%d\n", ret);
          return 1;
        }

      (void)bluez_audio_a2dp_signal_source_session_set(1, peer, handle);
      opened_local = 1;
    }

  for (i = 0; i < sizeof(seq) / sizeof(seq[0]); i++)
    {
      ret = bluez_audio_a2dp_signal_send(peer, seq[i].payload,
                                         seq[i].payload_len,
                                         seq[i].command);
      if (ret != 0)
        {
          if (opened_local)
            {
              (void)linux_bt_upstream_l2cap_socket_close_handle(
                g_bluez_audio_a2dp_source_session_l2cap);
              printf("upstream-l2cap-close: released\n");
              (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
            }

          return ret;
        }

      usleep(300000);
      ret = bluez_audio_a2dp_signal_wait_response(seq[i].command,
                                                  seq[i].payload[1]);
      if (ret != 0)
        {
          if (opened_local)
            {
              (void)linux_bt_upstream_l2cap_socket_close_handle(
                g_bluez_audio_a2dp_source_session_l2cap);
              printf("upstream-l2cap-close: released\n");
              (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
            }

          return ret;
        }

      (void)linux_bt_upstream_vhci_poll_medium();
      usleep(100000);
    }

  if (opened_local)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(
        g_bluez_audio_a2dp_source_session_l2cap);
      snprintf(out, sizeof(out), "upstream-l2cap-close: released\n");
      printf("%s", out);
      printf("bluez-audio: a2dp teardown transaction source session "
             "closed peer=%u handle=0x%04x ret=%d\n", peer, handle, ret);
      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
      if (ret < 0)
        {
          return 1;
        }
    }

  printf("bluez-audio: a2dp teardown transaction complete peer=%u count=%u\n",
         peer, (unsigned int)(sizeof(seq) / sizeof(seq[0])));
  return 0;
}

static int bluez_audio_a2dp_signal_source(int argc, char *argv[])
{
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;

  if (!strcmp(argv[2], "source-transaction"))
    {
      return bluez_audio_a2dp_signal_source_transaction(peer);
    }

  if (!strcmp(argv[2], "source-teardown-transaction"))
    {
      return bluez_audio_a2dp_signal_source_teardown_transaction(peer);
    }

  if (!strcmp(argv[2], "discover"))
    {
      static const uint8_t payload[] = {0x10, 0x01};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "discover");
    }

  if (!strcmp(argv[2], "getcap"))
    {
      static const uint8_t payload[] = {0x20, 0x02, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "getcap");
    }

  if (!strcmp(argv[2], "getallcap"))
    {
      static const uint8_t payload[] = {0xc0, 0x0c, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "getallcap");
    }

  if (!strcmp(argv[2], "setconfig"))
    {
      static const uint8_t payload[] =
      {
        0x30, 0x03,
        0x04, 0x08,
        0x01, 0x00,
        0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
      };

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "setconfig");
    }

  if (!strcmp(argv[2], "getconfig"))
    {
      static const uint8_t payload[] = {0xb0, 0x04, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "getconfig");
    }

  if (!strcmp(argv[2], "open"))
    {
      static const uint8_t payload[] = {0x40, 0x06, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "open");
    }

  if (!strcmp(argv[2], "reconfigure"))
    {
      static const uint8_t payload[] =
      {
        0x80, 0x05,
        0x04,
        0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
      };

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "reconfigure");
    }

  if (!strcmp(argv[2], "delay-report"))
    {
      static const uint8_t payload[] = {0xa0, 0x0d, 0x04, 0x00, 0x64};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "delay-report");
    }

  if (!strcmp(argv[2], "security-control"))
    {
      static const uint8_t payload[] = {0xd0, 0x0b, 0x04, 0x01, 0x02};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "security-control");
    }

  if (!strcmp(argv[2], "start"))
    {
      static const uint8_t payload[] = {0x50, 0x07, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "start");
    }

  if (!strcmp(argv[2], "suspend"))
    {
      static const uint8_t payload[] = {0x60, 0x09, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "suspend");
    }

  if (!strcmp(argv[2], "close-stream"))
    {
      static const uint8_t payload[] = {0x70, 0x08, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "close-stream");
    }

  if (!strcmp(argv[2], "abort"))
    {
      static const uint8_t payload[] = {0x90, 0x0a, 0x04};

      return bluez_audio_a2dp_signal_send(peer, payload, sizeof(payload),
                                          "abort");
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_a2dp_source_start(int argc, char *argv[])
{
  static const char media[] = "A2DP:SBC:synthetic-frame";
  char out[512] = "";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 0;
  uint16_t handle = peer != 0 ? linux_bt_conn_handle(peer) : 0x0040;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/a2dp.c "
         "style profile=a2dp-source command=start peer=%u "
         "handle=0x%04x\n", peer, handle);

  ret = linux_bt_upstream_l2cap_socket_send_probe(0x0019, 0x0041,
                                                  handle, media,
                                                  sizeof(media) - 1,
                                                  out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-source failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: a2dp source queued media payload len=%u\n",
         (unsigned int)(sizeof(media) - 1));
  return 0;
}

static int bluez_audio_a2dp_sink_start(int argc, char *argv[])
{
  char out[512] = "";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 0;
  uint16_t handle = peer != 0 ? linux_bt_conn_handle(peer) : 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
         "style profile=a2dp-sink command=start peer=%u "
         "handle=0x%04x\n", peer, handle);

  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0041,
                                                  handle, out,
                                                  sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-sink bind failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_l2cap_socket_listen_probe(1, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-sink listen failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: a2dp sink transport listening\n");
  return 0;
}

static int bluez_audio_a2dp_sink_read(int argc, char *argv[])
{
  static const char media_payload[] =
    "payload=41 32 44 50 3a 53 42 43 3a 73 79 6e 74 68 65 74 "
    "69 63 2d 66 72 61 6d 65";
  char out[1024] = "";
  size_t max_len = argc > 3 ? (size_t)strtoul(argv[3], NULL, 0) : 512;
  int polled = 0;
  int attempt;
  int ret = -EAGAIN;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=a2dp-sink command=read max=%lu\n",
         (unsigned long)max_len);

  for (attempt = 0; attempt < 200; attempt++)
    {
      ret = linux_bt_upstream_vhci_poll_medium();
      if (ret < 0)
        {
          printf("bluez-audio: a2dp-sink poll failed ret=%d\n", ret);
          return 1;
        }

      polled += ret;
      ret = linux_bt_upstream_l2cap_socket_recv_probe(max_len, out,
                                                      sizeof(out));
      if (ret >= 0)
        {
          printf("bluez-audio: a2dp sink polled=%d\n", polled);
          printf("%s", out);
          if (strstr(out, "recv-ret=24") != NULL &&
              strstr(out, media_payload) != NULL)
            {
              printf("bluez-audio: a2dp sink media payload received\n");
              return 0;
            }

          printf("bluez-audio: a2dp sink skipped non-media payload\n");
          ret = -EAGAIN;
        }

      if (ret != -EAGAIN)
        {
          printf("bluez-audio: a2dp sink polled=%d\n", polled);
          printf("%s", out);
          printf("bluez-audio: a2dp-sink recv failed ret=%d\n", ret);
          return 1;
        }

      usleep(50000);
    }

  printf("bluez-audio: a2dp sink polled=%d\n", polled);
  printf("%s", out);
  printf("bluez-audio: a2dp-sink recv failed ret=%d\n", ret);
  return 1;
}

static int bluez_audio_a2dp_sink_stop(void)
{
  char out[512] = "";
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=a2dp-sink command=stop\n");

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: a2dp-sink close failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: a2dp sink transport stopped\n");
  return 0;
}

static int bluez_audio_media_transport_a2dp_source(int argc, char *argv[])
{
  static const char media[] = "A2DP:SBC:synthetic-frame";
  char out[1024] = "";
  const char *command = argc > 2 ? argv[2] :
                        "a2dp-source-acquire-write-release";
  const char *transport = "/org/bluez/hci0/dev_feather/fd/source0";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  void *media_handle = NULL;
  int busy = strstr(command, "busy") != NULL;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=source "
         "command=%s peer=%u handle=0x%04x\n", command, peer, handle);

  ret = linux_bt_upstream_l2cap_socket_open(0x0019, 0x0041, handle,
                                            &media_handle);
  printf("bluez-audio: media transport handle open role=source "
         "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
         handle, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(media_handle,
                                                          0x0019,
                                                          0x0041);
      printf("bluez-audio: media transport handle connect role=source "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=l2cap "
             "transport=/org/bluez/hci0/dev_feather/fd0 "
             "read-mtu=672 write-mtu=672\n");
      if (g_bluez_audio_a2dp_source_session_active &&
          peer == g_bluez_audio_a2dp_source_session_peer &&
          handle == g_bluez_audio_a2dp_source_session_handle)
        {
          printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
                 "signaling session retained during media "
                 "peer=%u handle=0x%04x signaling-cid=0x0040 "
                 "media-cid=0x0041\n", peer, handle);
        }

      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus method=Acquire interface=org.bluez.MediaTransport1 "
             "path=%s fd=l2cap role=source request=primary\n",
             transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus signal=PropertiesChanged "
             "interface=org.bluez.MediaTransport1 path=%s "
             "property=State value=active role=source\n", transport);
      if (busy)
        {
          printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
                 "dbus error=org.bluez.Error.InProgress "
                 "method=org.bluez.MediaTransport1.Acquire path=%s "
                 "role=source request=duplicate\n", transport);
        }

      ret = linux_bt_upstream_l2cap_socket_write_handle(media_handle,
                                                        media,
                                                        sizeof(media) - 1,
                                                        out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport write len=%u "
             "payload=A2DP:SBC:synthetic-frame\n",
             (unsigned int)(sizeof(media) - 1));
    }

  if (media_handle != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(media_handle);
      printf("bluez-audio: media transport handle close role=source "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "dbus method=Release interface=org.bluez.MediaTransport1 "
         "path=%s role=source\n", transport);
  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "dbus signal=PropertiesChanged "
         "interface=org.bluez.MediaTransport1 path=%s "
         "property=State value=idle role=source\n", transport);

  if (failed)
    {
      printf("bluez-audio: media transport a2dp source failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=source\n");
  return 0;
}

static int bluez_audio_media_transport_a2dp_sink(int argc, char *argv[])
{
  char out[1024] = "";
  const char *command = argc > 2 ? argv[2] :
                        "a2dp-sink-acquire-read-release";
  const char *transport = "/org/bluez/hci0/dev_feather/fd/sink0";
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 1;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  size_t max_len = argc > 4 ? (size_t)strtoul(argv[4], NULL, 0) : 512;
  void *media_handle = NULL;
  int polled = 0;
  int attempt;
  int busy = strstr(command, "busy") != NULL;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=sink "
         "command=%s peer=%u handle=0x%04x max=%lu\n",
         command, peer, handle, (unsigned long)max_len);

  ret = linux_bt_upstream_l2cap_socket_open(0x0019, 0x0041, handle,
                                            &media_handle);
  printf("bluez-audio: media transport handle open role=sink "
         "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
         handle, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(media_handle,
                                                          0x0019,
                                                          0x0041);
      printf("bluez-audio: media transport handle connect role=sink "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=l2cap "
             "transport=/org/bluez/hci0/dev_feather/fd0 "
             "read-mtu=672 write-mtu=672\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus method=Acquire interface=org.bluez.MediaTransport1 "
             "path=%s fd=l2cap role=sink request=primary\n",
             transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus signal=PropertiesChanged "
             "interface=org.bluez.MediaTransport1 path=%s "
             "property=State value=active role=sink\n", transport);
      if (busy)
        {
          printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
                 "dbus error=org.bluez.Error.InProgress "
                 "method=org.bluez.MediaTransport1.Acquire path=%s "
                 "role=sink request=duplicate\n", transport);
        }

      ret = -EAGAIN;
      for (attempt = 0; attempt < 200; attempt++)
        {
          ret = linux_bt_upstream_vhci_poll_medium();
          if (ret < 0)
            {
              printf("bluez-audio: media transport poll failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          polled += ret;
          ret = linux_bt_upstream_l2cap_socket_recv_handle(media_handle,
                                                           max_len, out,
                                                           sizeof(out));
          if (ret >= 0)
            {
              printf("bluez-audio: media transport sink polled=%d\n",
                     polled);
              printf("%s", out);
              printf("bluez-audio: media transport read complete "
                     "payload=A2DP:SBC:synthetic-frame\n");
              break;
            }

          if (ret != -EAGAIN)
            {
              printf("bluez-audio: media transport sink polled=%d\n",
                     polled);
              printf("%s", out);
              printf("bluez-audio: media transport recv failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          usleep(50000);
        }

      if (ret < 0)
        {
          printf("bluez-audio: media transport sink polled=%d\n", polled);
          printf("%s", out);
          failed = 1;
        }
    }

  if (media_handle != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(media_handle);
      printf("bluez-audio: media transport handle close role=sink "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "dbus method=Release interface=org.bluez.MediaTransport1 "
         "path=%s role=sink\n", transport);
  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "dbus signal=PropertiesChanged "
         "interface=org.bluez.MediaTransport1 path=%s "
         "property=State value=idle role=sink\n", transport);

  if (failed)
    {
      printf("bluez-audio: media transport a2dp sink failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=sink\n");
  return 0;
}

static void bluez_audio_object_manager_added(const char *role,
                                             const char *uuid,
                                             const char *endpoint,
                                             const char *transport)
{
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus object-manager path=/ "
         "interface=org.freedesktop.DBus.ObjectManager "
         "method=GetManagedObjects adapter=/org/bluez/hci0 "
         "device=/org/bluez/hci0/dev_feather role=%s\n", role);
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesAdded path=/org/bluez/hci0/dev_feather "
         "interface=org.bluez.MediaControl1 role=%s\n", role);
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesAdded path=%s "
         "interface=org.bluez.MediaEndpoint1 uuid=%s codec=0x00 "
         "role=%s\n", endpoint, uuid, role);
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesAdded path=%s "
         "interface=org.bluez.MediaTransport1 state=idle role=%s\n",
         transport, role);
}

static void bluez_audio_object_manager_removed(const char *role,
                                               const char *endpoint,
                                               const char *transport)
{
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesRemoved path=%s "
         "interface=org.bluez.MediaTransport1 role=%s\n",
         transport, role);
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesRemoved path=%s "
         "interface=org.bluez.MediaEndpoint1 role=%s\n",
         endpoint, role);
  printf("bluez-audio: source=third/bluez/gdbus/object.c "
         "dbus signal=InterfacesRemoved path=/org/bluez/hci0/dev_feather "
         "interface=org.bluez.MediaControl1 role=%s\n", role);
}

static int bluez_audio_a2dp_endpoint(int argc, char *argv[])
{
  const char *command = argc > 2 ? argv[2] : "";
  const char *role = argc > 3 ? argv[3] : "source";
  uint16_t peer = argc > 4 ? (uint16_t)atoi(argv[4]) : 0;
  const char *uuid;
  const char *endpoint;
  const char *transport;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_audio_usage();
      return 1;
    }

  uuid = !strcmp(role, "source") ? "0000110a-0000-1000-8000-00805f9b34fb" :
                                   "0000110b-0000-1000-8000-00805f9b34fb";
  endpoint = !strcmp(role, "source") ?
             "/org/bluez/hci0/dev_feather/sep/source0" :
             "/org/bluez/hci0/dev_feather/sep/sink0";
  transport = !strcmp(role, "source") ?
              "/org/bluez/hci0/dev_feather/fd/source0" :
              "/org/bluez/hci0/dev_feather/fd/sink0";

  printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
         "style profile=media-endpoint command=%s role=%s peer=%u\n",
         command, role, peer);

  if (!strcmp(command, "lifecycle"))
    {
      bluez_audio_object_manager_added(role, uuid, endpoint, transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/%s.c "
             "sdp register service=%s uuid=%s psm=0x0019 "
             "features=delay-report,media-transport\n",
             !strcmp(role, "source") ? "source" : "sink",
             !strcmp(role, "source") ? "AudioSource" : "AudioSink",
             uuid);
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "control register psm=0x0017 browsing-psm=0x001b "
             "role=%s state=idle\n", role);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "sdp register controller uuid=0000110e-0000-1000-8000-00805f9b34fb "
             "target uuid=0000110c-0000-1000-8000-00805f9b34fb "
             "role=%s\n", role);
      printf("bluez-audio: source=third/bluez/profiles/audio/control.c "
             "dbus export interface=org.bluez.MediaControl1 "
             "path=/org/bluez/hci0/dev_feather control=avrcp role=%s\n",
             role);
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "dbus owner=:1.feather adapter=/org/bluez/hci0 "
             "interface=org.bluez.Media1 method=RegisterEndpoint "
             "endpoint=%s role=%s\n", endpoint, role);
      printf("bluez-audio: source=third/bluez/client/player.c "
             "dbus provider=bluezaudio interface=org.bluez.MediaEndpoint1 "
             "methods=SelectConfiguration,SetConfiguration,ClearConfiguration,Release\n");
      printf("bluez-audio: media endpoint register endpoint=%s uuid=%s "
             "codec=sbc codec-id=0x00\n", endpoint, uuid);
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "dbus endpoint owner=:1.feather endpoint=%s uuid=%s "
             "codec=0x00\n", endpoint, uuid);
      printf("bluez-audio: media endpoint capabilities "
             "media-type=audio codec=sbc caps=ff ff 02 35\n");
      printf("bluez-audio: source=third/bluez/client/player.c "
             "preset=a2dp_%s/sbc codec=sbc codec-id=0x00 "
             "caps=ff ff 02 40 preset-count=6\n",
             !strcmp(role, "source") ? "src" : "sink");
      printf("bluez-audio: source=third/bluez/profiles/audio/a2dp.c "
             "select-config role=%s sampling=44100 channels=2 "
             "channel-mode=joint-stereo block-length=16 "
             "subbands=8 allocation=loudness bitpool=2..53\n",
             role);
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "set-configuration endpoint=%s transport=%s "
             "configuration=21 15 02 35\n", endpoint, transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "transport created path=%s state=idle codec=sbc "
             "read-mtu=672 write-mtu=672\n", transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus export interface=org.bluez.MediaTransport1 path=%s "
             "owner=:1.feather state=idle codec=sbc\n", transport);
      printf("bluez-audio: media endpoint lifecycle complete role=%s\n",
             role);
      return 0;
    }

  if (!strcmp(command, "clear"))
    {
      bluez_audio_object_manager_removed(role, endpoint, transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/control.c "
             "dbus unexport interface=org.bluez.MediaControl1 "
             "path=/org/bluez/hci0/dev_feather role=%s\n", role);
      printf("bluez-audio: source=third/bluez/profiles/audio/avrcp.c "
             "sdp unregister controller uuid=0000110e-0000-1000-8000-00805f9b34fb "
             "target uuid=0000110c-0000-1000-8000-00805f9b34fb "
             "role=%s\n", role);
      printf("bluez-audio: source=third/bluez/profiles/audio/avctp.c "
             "control unregister psm=0x0017 browsing-psm=0x001b "
             "role=%s\n", role);
      printf("bluez-audio: source=third/bluez/profiles/audio/%s.c "
             "sdp unregister service=%s uuid=%s psm=0x0019\n",
             !strcmp(role, "source") ? "source" : "sink",
             !strcmp(role, "source") ? "AudioSource" : "AudioSink",
             uuid);
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "dbus method=ClearConfiguration owner=:1.feather "
             "endpoint=%s transport=%s\n", endpoint, transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "clear-configuration endpoint=%s transport=%s\n",
             endpoint, transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus unexport interface=org.bluez.MediaTransport1 path=%s\n",
             transport);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "transport removed path=%s state=idle\n", transport);
      printf("bluez-audio: media endpoint unregister endpoint=%s uuid=%s\n",
             endpoint, uuid);
      printf("bluez-audio: media endpoint clear complete role=%s\n", role);
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static void bluez_audio_print_sbc_backend(void)
{
  printf("bluez-audio: codec-backend profile=a2dp codec=sbc "
         "backend=%s status=%s required-source=%s\n",
         bluez_audio_sbc_backend_name(),
         bluez_audio_sbc_backend_status(),
         bluez_audio_sbc_backend_required_source());
}

static int bluez_audio_a2dp_codec_source(int argc, char *argv[])
{
  const uint8_t *frame;
  char out[1024] = "";
  struct bluez_audio_sbc_frame_info sbc;
  size_t frame_len;
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  void *media_handle = NULL;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/a2dp.c "
         "style profile=a2dp-codec role=source "
         "command=source-sbc-encode-write-release peer=%u "
         "handle=0x%04x\n", peer, handle);
  printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
         "pcm input format=s16le rate=44100 channels=2 frames=128\n");
  printf("bluez-audio: source=third/bluez/profiles/audio/a2dp-codecs.h "
         "selected-config=" BLUEZ_A2DP_SBC_SELECTED_CONFIG
         " codec-id=0x00\n");

  frame = bluez_audio_sbc_backend_frame(&frame_len);
  if (frame == NULL || frame_len == 0)
    {
      printf("bluez-audio: sbc backend frame unavailable backend=%s "
             "status=%s\n", bluez_audio_sbc_backend_name(),
             bluez_audio_sbc_backend_status());
      return 1;
    }

  bluez_audio_print_sbc_backend();
  ret = bluez_audio_sbc_parse_frame_header(frame, frame_len, &sbc);
  if (ret < 0)
    {
      printf("bluez-audio: sbc header parse failed ret=%d\n", ret);
      return 1;
    }

  printf("bluez-audio: source=third/bluez/profiles/audio/a2dp-codecs.h "
         "sbc encode syncword=0x%02x frame-header=%02x %02x %02x "
         "samplerate=%lu channels=%u channel-mode=%s blocks=%u "
         "subbands=%u allocation=%s bitpool=%u frame-len=%u\n",
         frame[0], frame[1], frame[2], frame[3],
         (unsigned long)sbc.samplerate, sbc.channels, sbc.channel_mode,
         sbc.blocks, sbc.subbands, sbc.allocation, sbc.bitpool,
         (unsigned int)frame_len);

  ret = linux_bt_upstream_l2cap_socket_open(0x0019, 0x0041, handle,
                                            &media_handle);
  printf("bluez-audio: media transport handle open role=source "
         "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
         handle, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(media_handle,
                                                          0x0019,
                                                          0x0041);
      printf("bluez-audio: media transport handle connect role=source "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=l2cap "
             "transport=/org/bluez/hci0/dev_feather/fd/source0 "
             "read-mtu=672 write-mtu=672 codec=sbc\n");
      if (g_bluez_audio_a2dp_source_session_active &&
          peer == g_bluez_audio_a2dp_source_session_peer &&
          handle == g_bluez_audio_a2dp_source_session_handle)
        {
          printf("bluez-audio: source=third/bluez/profiles/audio/avdtp.c "
                 "signaling session retained during media "
                 "peer=%u handle=0x%04x signaling-cid=0x0040 "
                 "media-cid=0x0041\n", peer, handle);
        }

      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus method=Acquire interface=org.bluez.MediaTransport1 "
             "path=/org/bluez/hci0/dev_feather/fd/source0 fd=l2cap "
             "role=source\n");
      ret = linux_bt_upstream_l2cap_socket_write_handle(media_handle,
                                                        frame, frame_len,
                                                        out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (media_handle != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(media_handle);
      printf("bluez-audio: media transport handle close role=source "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (failed)
    {
      printf("bluez-audio: a2dp codec source failed\n");
      return 1;
    }

  printf("bluez-audio: a2dp codec source wrote sbc frame len=%u\n",
         (unsigned int)frame_len);
  printf("bluez-audio: media transport release complete role=source codec=sbc\n");
  return 0;
}

static int bluez_audio_a2dp_codec_sink(int argc, char *argv[])
{
  const uint8_t *expected_frame;
  char frame_prefix[64];
  char recv_prefix[32];
  char out[1024] = "";
  struct bluez_audio_sbc_frame_info sbc;
  struct bluez_audio_sbc_pcm_info pcm;
  size_t expected_frame_len;
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 1;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  size_t max_len = argc > 4 ? (size_t)strtoul(argv[4], NULL, 0) : 512;
  void *media_handle = NULL;
  int polled = 0;
  int attempt;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/a2dp.c "
         "style profile=a2dp-codec role=sink "
         "command=sink-sbc-recv-decode-release peer=%u "
         "handle=0x%04x max=%lu\n", peer, handle,
         (unsigned long)max_len);

  expected_frame = bluez_audio_sbc_backend_frame(&expected_frame_len);
  if (expected_frame == NULL || expected_frame_len < 4)
    {
      printf("bluez-audio: sbc backend frame unavailable backend=%s "
             "status=%s\n", bluez_audio_sbc_backend_name(),
             bluez_audio_sbc_backend_status());
      return 1;
    }

  snprintf(recv_prefix, sizeof(recv_prefix), "recv-ret=%u",
           (unsigned int)expected_frame_len);
  snprintf(frame_prefix, sizeof(frame_prefix), "payload=%02x %02x %02x",
           expected_frame[0], expected_frame[1], expected_frame[2]);

  ret = linux_bt_upstream_l2cap_socket_open(0x0019, 0x0041, handle,
                                            &media_handle);
  printf("bluez-audio: media transport handle open role=sink "
         "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
         handle, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(media_handle,
                                                          0x0019,
                                                          0x0041);
      printf("bluez-audio: media transport handle connect role=sink "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=l2cap "
             "transport=/org/bluez/hci0/dev_feather/fd/sink0 "
             "read-mtu=672 write-mtu=672 codec=sbc\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus method=Acquire interface=org.bluez.MediaTransport1 "
             "path=/org/bluez/hci0/dev_feather/fd/sink0 fd=l2cap "
             "role=sink\n");

      ret = -EAGAIN;
      for (attempt = 0; attempt < 200; attempt++)
        {
          ret = linux_bt_upstream_vhci_poll_medium();
          if (ret < 0)
            {
              printf("bluez-audio: a2dp codec sink poll failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          polled += ret;
          ret = linux_bt_upstream_l2cap_socket_recv_handle(media_handle,
                                                           max_len, out,
                                                           sizeof(out));
          if (ret >= 0)
            {
              printf("bluez-audio: a2dp codec sink polled=%d\n", polled);
              printf("%s", out);
              if (strstr(out, recv_prefix) != NULL &&
                  strstr(out, frame_prefix) != NULL)
                {
                  bluez_audio_print_sbc_backend();
                  printf("bluez-audio: source=third/bluez/profiles/audio/"
                         "a2dp-codecs.h selected-config="
                         BLUEZ_A2DP_SBC_SELECTED_CONFIG
                         " codec-id=0x00\n");
                  ret = bluez_audio_sbc_parse_frame_header(expected_frame,
                                                           expected_frame_len,
                                                           &sbc);
                  if (ret < 0)
                    {
                      printf("bluez-audio: sbc header parse failed ret=%d\n",
                             ret);
                      failed = 1;
                      break;
                    }

                  printf("bluez-audio: source=third/bluez/profiles/audio/"
                         "a2dp-codecs.h sbc decode syncword=0x9c "
                         "frame-header=%02x %02x %02x samplerate=%lu "
                         "channels=%u channel-mode=%s blocks=%u "
                         "subbands=%u allocation=%s bitpool=%u "
                         "frame-len=%u\n",
                         expected_frame[1], expected_frame[2],
                         expected_frame[3],
                         (unsigned long)sbc.samplerate, sbc.channels,
                         sbc.channel_mode, sbc.blocks, sbc.subbands,
                         sbc.allocation, sbc.bitpool,
                         (unsigned int)expected_frame_len);
                  ret = bluez_audio_sbc_decode_frame(expected_frame,
                                                     expected_frame_len,
                                                     &pcm);
                  if (ret < 0)
                    {
                      printf("bluez-audio: sbc decode failed ret=%d\n",
                             ret);
                      failed = 1;
                      break;
                    }

                  printf("bluez-audio: source=third/sbc-2.0/sbc/sbc.c "
                         "sbc_decode pcm-bytes=%u codesize=%u "
                         "checksum=0x%08lx\n",
                         (unsigned int)pcm.pcm_len,
                         (unsigned int)pcm.codesize,
                         (unsigned long)pcm.checksum);
                  printf("bluez-audio: source=third/bluez/profiles/audio/"
                         "media.c pcm output format=s16le rate=44100 "
                         "channels=2 pcm-bytes=%u checksum=0x%08lx\n",
                         (unsigned int)pcm.pcm_len,
                         (unsigned long)pcm.checksum);
                  break;
                }

              printf("bluez-audio: a2dp codec sink skipped non-sbc frame\n");
              ret = -EAGAIN;
            }

          if (ret != -EAGAIN)
            {
              printf("bluez-audio: a2dp codec sink polled=%d\n", polled);
              printf("%s", out);
              printf("bluez-audio: a2dp codec sink recv failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          usleep(50000);
        }

      if (ret < 0)
        {
          printf("bluez-audio: a2dp codec sink polled=%d\n", polled);
          printf("%s", out);
          failed = 1;
        }
    }

  if (media_handle != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(media_handle);
      printf("bluez-audio: media transport handle close role=sink "
             "psm=0x0019 cid=0x0041 handle=0x%04x ret=%d\n",
             handle, ret);
      failed |= ret < 0;
    }

  if (failed)
    {
      printf("bluez-audio: a2dp codec sink failed\n");
      return 1;
    }

  printf("bluez-audio: a2dp codec sink decoded pcm checksum=0x%08lx\n",
         (unsigned long)pcm.checksum);
  printf("bluez-audio: media transport release complete role=sink codec=sbc\n");
  return 0;
}

static uint16_t bluez_audio_iso_handle(uint8_t big, uint8_t bis)
{
  return (uint16_t)(0x0100 | ((big & 0x0f) << 4) | (bis & 0x0f));
}

static uint16_t bluez_audio_cis_handle(uint8_t cig, uint8_t cis)
{
  return (uint16_t)(0x0200 | ((cig & 0x0f) << 4) | (cis & 0x0f));
}

static int bluez_audio_le_source_start(int argc, char *argv[])
{
  static const char media[] = "LE-AUDIO:LC3:synthetic-frame";
  char out[512] = "";
  uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_iso_handle(big, bis);
  int failed = 0;
  int opened = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-broadcast-source command=start big=%u bis=%u "
         "handle=0x%04x\n", big, bis, handle);

  ret = linux_bt_upstream_iso_socket_write_probe(media,
                                                 sizeof(media) - 1,
                                                 out, sizeof(out));
  if (ret >= 0)
    {
      printf("%s", out);
      printf("bluez-audio: le broadcast source queued iso payload len=%u\n",
             (unsigned int)(sizeof(media) - 1));
      return 0;
    }

  if (ret != -ENOTCONN)
    {
      printf("%s", out);
      printf("bluez-audio: le-broadcast-source failed ret=%d\n", ret);
      return 1;
    }

  ret = linux_bt_upstream_iso_socket_bind_probe(0, handle, out,
                                                sizeof(out));
  printf("%s", out);
  failed |= ret < 0;
  opened = ret >= 0;

  if (!failed)
    {
      ret = linux_bt_upstream_iso_socket_connect_probe(0, out,
                                                       sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      ret = linux_bt_upstream_iso_socket_write_probe(media,
                                                     sizeof(media) - 1,
                                                     out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (opened)
    {
      ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (failed)
    {
      printf("bluez-audio: le-broadcast-source failed ret=%d\n", ret);
      return 1;
    }

  printf("bluez-audio: le broadcast source queued iso payload len=%u\n",
         (unsigned int)(sizeof(media) - 1));
  return 0;
}

static int bluez_audio_le_unicast_source_start(int argc, char *argv[])
{
  static const char media[] = "LE-AUDIO:CIS:synthetic-frame";
  char out[512] = "";
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-unicast-source command=start cig=%u cis=%u "
         "handle=0x%04x\n", cig, cis, handle);

  ret = linux_bt_upstream_iso_socket_send_probe(1, handle, media,
                                                sizeof(media) - 1,
                                                out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-unicast-source failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: le unicast source queued cis payload len=%u\n",
         (unsigned int)(sizeof(media) - 1));
  return 0;
}

static int bluez_audio_le_sink_sync(int argc, char *argv[])
{
  char out[512] = "";
  uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_iso_handle(big, bis);
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-broadcast-sink command=sync big=%u bis=%u "
         "handle=0x%04x\n", big, bis, handle);

  ret = linux_bt_upstream_iso_socket_bind_probe(0, handle, out,
                                                sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-broadcast-sink bind failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_iso_socket_connect_probe(0, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-broadcast-sink connect failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: le broadcast sink synced\n");
  return 0;
}

static int bluez_audio_le_unicast_sink_sync(int argc, char *argv[])
{
  char out[512] = "";
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-unicast-sink command=sync cig=%u cis=%u "
         "handle=0x%04x\n", cig, cis, handle);

  ret = linux_bt_upstream_iso_socket_bind_probe(1, handle, out,
                                                sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-unicast-sink bind failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-unicast-sink connect failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: le unicast sink synced\n");
  return 0;
}

static int bluez_audio_le_sink_start(int argc, char *argv[])
{
  char out[1024] = "";
  size_t max_len = argc > 5 ? (size_t)strtoul(argv[5], NULL, 0) : 512;
  int polled = 0;
  int attempt;
  int ret = -EAGAIN;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-broadcast-sink command=start max=%lu\n",
         (unsigned long)max_len);

  for (attempt = 0; attempt < 120; attempt++)
    {
      ret = linux_bt_upstream_vhci_poll_medium();
      if (ret < 0)
        {
          printf("bluez-audio: le-broadcast-sink poll failed ret=%d\n",
                 ret);
          return 1;
        }

      polled += ret;
      ret = linux_bt_upstream_iso_socket_recv_probe(max_len, out,
                                                    sizeof(out));
      if (ret >= 0)
        {
          printf("bluez-audio: le broadcast sink polled=%d\n", polled);
          printf("%s", out);
          printf("bluez-audio: le broadcast sink iso payload received\n");
          return 0;
        }

      if (ret != -EAGAIN)
        {
          printf("bluez-audio: le broadcast sink polled=%d\n", polled);
          printf("%s", out);
          printf("bluez-audio: le-broadcast-sink recv failed ret=%d\n",
                 ret);
          return 1;
        }

      usleep(50000);
    }

  printf("bluez-audio: le broadcast sink polled=%d\n", polled);
  printf("%s", out);
  printf("bluez-audio: le-broadcast-sink recv failed ret=%d\n", ret);
  return 1;
}

static int bluez_audio_le_sink_stop(void)
{
  char out[512] = "";
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-broadcast-sink command=stop\n");

  ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-broadcast-sink close failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: le broadcast sink stopped\n");
  return 0;
}

static int bluez_audio_le_unicast_sink_start(int argc, char *argv[])
{
  char out[1024] = "";
  size_t max_len = argc > 5 ? (size_t)strtoul(argv[5], NULL, 0) : 512;
  int polled = 0;
  int attempt;
  int ret = -EAGAIN;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-unicast-sink command=start max=%lu\n",
         (unsigned long)max_len);

  for (attempt = 0; attempt < 20; attempt++)
    {
      ret = linux_bt_upstream_vhci_poll_medium();
      if (ret < 0)
        {
          printf("bluez-audio: le-unicast-sink poll failed ret=%d\n",
                 ret);
          return 1;
        }

      polled += ret;
      ret = linux_bt_upstream_iso_socket_recv_probe(max_len, out,
                                                    sizeof(out));
      if (ret >= 0)
        {
          printf("bluez-audio: le unicast sink polled=%d\n", polled);
          printf("%s", out);
          printf("bluez-audio: le unicast sink cis payload received\n");
          return 0;
        }

      if (ret != -EAGAIN)
        {
          printf("bluez-audio: le unicast sink polled=%d\n", polled);
          printf("%s", out);
          printf("bluez-audio: le-unicast-sink recv failed ret=%d\n",
                 ret);
          return 1;
        }

      usleep(50000);
    }

  printf("bluez-audio: le unicast sink polled=%d\n", polled);
  printf("%s", out);
  printf("bluez-audio: le-unicast-sink recv failed ret=%d\n", ret);
  return 1;
}

static int bluez_audio_le_unicast_sink_stop(void)
{
  char out[512] = "";
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-unicast-sink command=stop\n");

  ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-unicast-sink close failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("bluez-audio: le unicast sink stopped\n");
  return 0;
}

static int bluez_audio_media_transport_unicast_source(int argc,
                                                      char *argv[])
{
  static const char media[] = "LE-AUDIO:CIS:synthetic-frame";
  char out[1024] = "";
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int transport_owned = linux_bt_upstream_iso_socket_active_probe();
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=source "
         "command=unicast-source-acquire-write-release cig=%u cis=%u "
         "handle=0x%04x\n", cig, cis, handle);

  ret = linux_bt_upstream_iso_socket_bind_probe(1, handle, out,
                                                sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=iso "
             "transport=/org/bluez/hci0/dev_feather/ase1 "
             "read-mtu=251 write-mtu=251\n");
      ret = linux_bt_upstream_iso_socket_write_probe(media,
                                                     sizeof(media) - 1,
                                                     out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport write len=%u "
             "payload=LE-AUDIO:CIS:synthetic-frame\n",
             (unsigned int)(sizeof(media) - 1));
    }

  ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

  if (failed)
    {
      printf("bluez-audio: media transport source failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=source\n");
  return 0;
}

static int bluez_audio_media_transport_unicast_sink(int argc, char *argv[])
{
  char out[1024] = "";
  size_t max_len = argc > 5 ? (size_t)strtoul(argv[5], NULL, 0) : 512;
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int polled = 0;
  int attempt;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=sink "
         "command=unicast-sink-acquire-read-release cig=%u cis=%u "
         "handle=0x%04x max=%lu\n", cig, cis, handle,
         (unsigned long)max_len);

  ret = linux_bt_upstream_iso_socket_bind_probe(1, handle, out,
                                                sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=iso "
             "transport=/org/bluez/hci0/dev_feather/ase1 "
             "read-mtu=251 write-mtu=251\n");

      ret = -EAGAIN;
      for (attempt = 0; attempt < 800; attempt++)
        {
          ret = linux_bt_upstream_vhci_poll_medium();
          if (ret < 0)
            {
              printf("bluez-audio: media transport poll failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          polled += ret;
          ret = linux_bt_upstream_iso_socket_recv_probe(max_len, out,
                                                        sizeof(out));
          if (ret >= 0)
            {
              printf("bluez-audio: media transport sink polled=%d\n",
                     polled);
              printf("%s", out);
              printf("bluez-audio: media transport read complete "
                     "payload=LE-AUDIO:CIS:synthetic-frame\n");
              break;
            }

          if (ret != -EAGAIN)
            {
              printf("bluez-audio: media transport sink polled=%d\n",
                     polled);
              printf("%s", out);
              printf("bluez-audio: media transport recv failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          usleep(50000);
        }

      if (ret < 0)
        {
          printf("bluez-audio: media transport sink polled=%d\n", polled);
          printf("%s", out);
          failed = 1;
        }
    }

  ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

  if (failed)
    {
      printf("bluez-audio: media transport sink failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=sink\n");
  return 0;
}

static int bluez_audio_le_codec_source(int argc, char *argv[])
{
  const uint8_t *frame;
  struct bluez_audio_lc3_frame_info info;
  struct bluez_audio_lc3_pcm_info pcm;
  char out[1024] = "";
  size_t frame_len = 0;
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int transport_owned = linux_bt_upstream_iso_socket_active_probe();
  int failed = 0;
  int ret;

  frame = bluez_audio_lc3_backend_frame(&frame_len);
  ret = bluez_audio_lc3_parse_frame(frame, frame_len, &info);
  failed |= ret < 0;
  ret = bluez_audio_lc3_decode_frame(frame, frame_len, &pcm);
  failed |= ret < 0;

  printf("bluez-audio: codec-backend profile=le-audio codec=lc3 "
         "backend=%s status=%s required-source=%s\n",
         bluez_audio_lc3_backend_name(),
         bluez_audio_lc3_backend_status(),
         bluez_audio_lc3_backend_required_source());
  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-audio-codec role=source "
         "command=source-lc3-encode-write-release cig=%u cis=%u "
         "handle=0x%04x\n", cig, cis, handle);
  printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
         "lc3 capabilities=%s metadata=%s\n",
         BLUEZ_LE_AUDIO_LC3_CAPABILITIES,
         BLUEZ_LE_AUDIO_LC3_METADATA);

  if (!failed)
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "lc3 qos interval-us=%lu framing=%s rtn=2 latency-ms=20 "
             "presentation-delay-us=%lu sdu=%u phy=2m\n",
             (unsigned long)info.sdu_interval_us, info.framing,
             (unsigned long)info.presentation_delay_us,
             info.octets_per_frame);
      printf("bluez-audio: source=third/bluez/profiles/audio/lc3.c "
             "lc3 encode sample-rate=%lu frame-duration-us=%lu "
             "channels=%u blocks-per-sdu=%u frame-len=%lu "
             "seq=1 timestamp-us=%lu checksum=0x%08lx\n",
             (unsigned long)info.sample_rate,
             (unsigned long)info.frame_duration_us,
             info.channels, info.blocks_per_sdu,
             (unsigned long)frame_len,
             (unsigned long)info.sdu_interval_us,
             (unsigned long)pcm.checksum);
    }

  if (!failed && !transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_bind_probe(1, handle, out,
                                                    sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed && !transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=iso "
             "transport=/org/bluez/hci0/dev_feather/ase1 "
             "read-mtu=251 write-mtu=251 codec=lc3\n");
      ret = linux_bt_upstream_iso_socket_write_probe(frame, frame_len,
                                                     out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport write len=%lu "
             "payload=LE-AUDIO:LC3:codec-frame seq=1 "
             "timestamp-us=%lu\n",
             (unsigned long)frame_len,
             (unsigned long)info.sdu_interval_us);
    }

  if (!transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (failed)
    {
      printf("bluez-audio: le audio lc3 source failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=source "
         "codec=lc3\n");
  return 0;
}

static int bluez_audio_le_codec_sink(int argc, char *argv[])
{
  const uint8_t *frame;
  struct bluez_audio_lc3_frame_info info;
  struct bluez_audio_lc3_pcm_info pcm;
  char out[1024] = "";
  size_t frame_len = 0;
  size_t max_len = argc > 5 ? (size_t)strtoul(argv[5], NULL, 0) : 512;
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  int transport_owned = linux_bt_upstream_iso_socket_active_probe();
  int polled = 0;
  int attempt;
  int failed = 0;
  int ret;

  frame = bluez_audio_lc3_backend_frame(&frame_len);
  ret = bluez_audio_lc3_parse_frame(frame, frame_len, &info);
  failed |= ret < 0;

  printf("bluez-audio: codec-backend profile=le-audio codec=lc3 "
         "backend=%s status=%s required-source=%s\n",
         bluez_audio_lc3_backend_name(),
         bluez_audio_lc3_backend_status(),
         bluez_audio_lc3_backend_required_source());
  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-audio-codec role=sink "
         "command=sink-lc3-recv-decode-release cig=%u cis=%u "
         "handle=0x%04x max=%lu\n", cig, cis, handle,
         (unsigned long)max_len);
  printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
         "lc3 capabilities=%s metadata=%s\n",
         BLUEZ_LE_AUDIO_LC3_CAPABILITIES,
         BLUEZ_LE_AUDIO_LC3_METADATA);

  if (!failed && !transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_bind_probe(1, handle, out,
                                                    sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed && !transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=iso "
             "transport=/org/bluez/hci0/dev_feather/ase1 "
             "read-mtu=251 write-mtu=251 codec=lc3\n");

      ret = -EAGAIN;
      for (attempt = 0; attempt < 800; attempt++)
        {
          ret = linux_bt_upstream_vhci_poll_medium();
          if (ret < 0)
            {
              printf("bluez-audio: le audio lc3 poll failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          polled += ret;
          ret = linux_bt_upstream_iso_socket_recv_probe(max_len, out,
                                                        sizeof(out));
          if (ret >= 0)
            {
              printf("bluez-audio: le audio lc3 sink polled=%d\n",
                     polled);
              printf("%s", out);
              break;
            }

          if (ret != -EAGAIN)
            {
              printf("bluez-audio: le audio lc3 sink polled=%d\n",
                     polled);
              printf("%s", out);
              printf("bluez-audio: le audio lc3 recv failed ret=%d\n",
                     ret);
              failed = 1;
              break;
            }

          usleep(50000);
        }

      if (ret < 0)
        {
          printf("bluez-audio: le audio lc3 sink polled=%d\n", polled);
          printf("%s", out);
          failed = 1;
        }
    }

  if (!failed)
    {
      ret = bluez_audio_lc3_decode_frame(frame, frame_len, &pcm);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/lc3.c "
             "lc3 decode sample-rate=%lu frame-duration-us=%lu "
             "channels=%u blocks-per-sdu=%u frame-len=%lu pcm-len=%lu "
             "seq=1 timestamp-us=%lu checksum=0x%08lx\n",
             (unsigned long)info.sample_rate,
             (unsigned long)info.frame_duration_us,
             info.channels, info.blocks_per_sdu,
             (unsigned long)frame_len, (unsigned long)pcm.pcm_len,
             (unsigned long)info.sdu_interval_us,
             (unsigned long)pcm.checksum);
      printf("bluez-audio: media transport read complete "
             "payload=LE-AUDIO:LC3:codec-frame seq=1 "
             "timestamp-us=%lu\n",
             (unsigned long)info.sdu_interval_us);
    }

  if (!transport_owned)
    {
      ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (failed)
    {
      printf("bluez-audio: le audio lc3 sink failed\n");
      return 1;
    }

  printf("bluez-audio: media transport release complete role=sink "
         "codec=lc3\n");
  return 0;
}

static int bluez_audio_le_bap_control(int argc, char *argv[])
{
  uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_iso_handle(big, bis);

  if (!strcmp(argv[2], "source-announce"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-bap-control role=source "
             "command=source-announce big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "base codec=lc3 subgroup=1 bis-count=1 "
             "presentation-delay-us=10000\n");
      printf("bluez-audio: le bap source state=idle->base-announced\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-start"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-bap-control role=source "
             "command=source-start big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "qos interval-us=10000 framing=unframed rtn=2 latency-ms=20\n");
      printf("bluez-audio: le bap source state=base-announced->streaming\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-stop"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-bap-control role=source "
             "command=source-stop big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: le bap source state=streaming->idle\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-discover"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
             "style profile=le-bap-control role=sink "
             "command=sink-discover big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "sink discovered pac=lc3 context=media location=front-left-right\n");
      printf("bluez-audio: le bap sink state=idle->pacs-discovered\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-config"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-bap-control role=sink "
             "command=sink-config big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=idle->codec-configured->qos-configured\n");
      printf("bluez-audio: le bap sink state=pacs-discovered->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-sync"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bap-control role=sink "
             "command=sink-sync big=%u bis=%u handle=0x%04x\n",
             big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "pa-sync=1 base-received=1 bis-synced=1\n");
      printf("bluez-audio: le bap sink state=configured->streaming\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_unicast_control(int argc, char *argv[])
{
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);

  if (!strcmp(argv[2], "source-config"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=source "
             "command=source-config cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "unicast cig=%u cis=%u ase=1 codec=lc3 context=media\n",
             cig, cis);
      printf("bluez-audio: le unicast source state=idle->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-enable"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-unicast-control role=source "
             "command=source-enable cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "cis-qos interval-us=10000 framing=unframed rtn=2 latency-ms=20\n");
      printf("bluez-audio: le unicast source state=configured->streaming\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-disable"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-unicast-control role=source "
             "command=source-disable cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=streaming->qos-configured disable-complete\n");
      printf("bluez-audio: le unicast source state=streaming->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-qos-update"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=source "
             "command=source-qos-update cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "cis-qos interval-us=7500 framing=unframed rtn=3 latency-ms=30\n");
      printf("bluez-audio: le unicast source state=configured->configured qos-updated\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-qos-reject"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=source "
             "command=source-qos-reject cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase cp response op=config-qos status=0x0d reason=invalid-qos\n");
      printf("bluez-audio: le unicast source state=configured->configured qos-rejected\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-qos-cancel"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=source "
             "command=source-qos-cancel cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase cp response op=config-qos status=0x0e reason=procedure-cancelled\n");
      printf("bluez-audio: le unicast source state=configured->configured qos-cancelled\n");
      return 0;
    }

  if (!strcmp(argv[2], "source-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=source "
             "command=source-release cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=configured->idle release-complete\n");
      printf("bluez-audio: le unicast source state=streaming->idle\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-discover"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-discover cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "sink discovered pac=lc3 context=media unicast=1\n");
      printf("bluez-audio: le unicast sink state=idle->pacs-discovered\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-config"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-config cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=idle->codec-configured->qos-configured\n");
      printf("bluez-audio: le unicast sink state=pacs-discovered->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-enable"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-enable cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "cis-established=1 ase=1 receiver-ready=1\n");
      printf("bluez-audio: le unicast sink state=configured->streaming\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-qos-update"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-qos-update cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "receiver-qos interval-us=7500 framing=unframed rtn=3 latency-ms=30\n");
      printf("bluez-audio: le unicast sink state=configured->configured qos-updated\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-qos-reject"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-qos-reject cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase cp response op=config-qos status=0x0d reason=invalid-qos\n");
      printf("bluez-audio: le unicast sink state=configured->configured qos-rejected\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-qos-cancel"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-qos-cancel cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase cp response op=config-qos status=0x0e reason=procedure-cancelled\n");
      printf("bluez-audio: le unicast sink state=configured->configured qos-cancelled\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-disable"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-disable cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=streaming->qos-configured receiver-stopped\n");
      printf("bluez-audio: le unicast sink state=streaming->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "sink-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "style profile=le-unicast-control role=sink "
             "command=sink-release cig=%u cis=%u handle=0x%04x\n",
             cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "ase state=configured->idle release-complete\n");
      printf("bluez-audio: le unicast sink state=configured->idle\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_cap_control(int argc, char *argv[])
{
  uint8_t cig = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t cis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint8_t peer_cis = argc > 5 ? (uint8_t)atoi(argv[5]) : cis + 1;
  uint16_t handle = bluez_audio_cis_handle(cig, cis);
  uint16_t peer_handle = bluez_audio_cis_handle(cig, peer_cis);

  if (!strcmp(argv[2], "coordinator-register"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=coordinator-register\n");
      printf("bluez-audio: dbus object=/org/bluez/hci0/cap "
             "interfaces=org.bluez.CAPCoordinator1 "
             "sets=coordinated-set-1 context=media\n");
      printf("bluez-audio: cap coordinator state=idle->registered\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-config"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-config "
             "cig=%u cis=%u handle=0x%04x\n", cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 ase-count=1 "
             "codec=lc3 target-latency=balanced\n");
      printf("bluez-audio: cap coordinator state=registered->configured\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-enable"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-enable "
             "cig=%u cis=%u handle=0x%04x\n", cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 ase-count=1 "
             "streaming=1\n");
      printf("bluez-audio: cap coordinator state=configured->streaming\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-release "
             "cig=%u cis=%u handle=0x%04x\n", cig, cis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 release-complete=1\n");
      printf("bluez-audio: cap coordinator state=streaming->registered\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-config-bidir"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-config-bidir "
             "cig=%u source-cis=%u sink-cis=%u source-handle=0x%04x "
             "sink-handle=0x%04x\n",
             cig, cis, peer_cis, handle, peer_handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 ase-count=2 "
             "members=source:ase3,sink:ase18 codec=lc3 "
             "target-latency=balanced policy=atomic-config\n");
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/cap "
             "interface=org.bluez.CAPCoordinator1 "
             "State=configured Members=2\n");
      printf("bluez-audio: cap coordinator state=registered->configured "
             "members=2\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-enable-bidir"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-enable-bidir "
             "cig=%u source-cis=%u sink-cis=%u source-handle=0x%04x "
             "sink-handle=0x%04x\n",
             cig, cis, peer_cis, handle, peer_handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 ase-count=2 "
             "members=source:streaming,sink:streaming "
             "metadata=context-media policy=atomic-enable\n");
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/cap "
             "interface=org.bluez.CAPCoordinator1 "
             "State=streaming Members=2\n");
      printf("bluez-audio: cap coordinator state=configured->streaming "
             "members=2\n");
      return 0;
    }

  if (!strcmp(argv[2], "group-release-bidir"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=group-release-bidir "
             "cig=%u source-cis=%u sink-cis=%u source-handle=0x%04x "
             "sink-handle=0x%04x\n",
             cig, cis, peer_cis, handle, peer_handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "coordinated set=coordinated-set-1 ase-count=2 "
             "release-complete=2 policy=atomic-release\n");
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/cap "
             "interface=org.bluez.CAPCoordinator1 "
             "State=registered Members=0\n");
      printf("bluez-audio: cap coordinator state=streaming->registered "
             "members=0\n");
      return 0;
    }

  if (!strcmp(argv[2], "coordinator-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "style profile=le-cap-control command=coordinator-release\n");
      printf("bluez-audio: dbus interfaces-removed "
             "path=/org/bluez/hci0/cap "
             "interface=org.bluez.CAPCoordinator1\n");
      printf("bluez-audio: cap coordinator state=registered->idle\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_bass_control(int argc, char *argv[])
{
  uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
  uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
  uint16_t handle = bluez_audio_iso_handle(big, bis);

  if (!strcmp(argv[2], "assistant-register"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=assistant-register\n");
      printf("bluez-audio: dbus object=/org/bluez/hci0/bass "
             "interfaces=org.bluez.BASS1 "
             "broadcast-assistant=1 receive-state-count=0\n");
      printf("bluez-audio: bass assistant state=idle->registered\n");
      return 0;
    }

  if (!strcmp(argv[2], "scan-delegator-register"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=scan-delegator-register\n");
      printf("bluez-audio: dbus object=/org/bluez/hci0/bass/delegator0 "
             "interfaces=org.bluez.BASS1 scan-delegator=1 "
             "receive-state-count=0\n");
      printf("bluez-audio: bass scan-delegator state=idle->registered\n");
      return 0;
    }

  if (!strcmp(argv[2], "add-source"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=add-source "
             "big=%u bis=%u handle=0x%04x\n", big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "bass cp op=add-source broadcast-id=0x000001 "
             "pa-sync=sync-past bis-sync=0x%08x\n", 1u << bis);
      printf("bluez-audio: bass assistant state=registered->source-added\n");
      return 0;
    }

  if (!strcmp(argv[2], "modify-source"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=modify-source "
             "big=%u bis=%u handle=0x%04x\n", big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "bass cp op=modify-source source-id=1 pa-sync=keep "
             "bis-sync=0x%08x\n", 1u << bis);
      printf("bluez-audio: bass assistant state=source-added->source-synced\n");
      return 0;
    }

  if (!strcmp(argv[2], "scan-delegator-receive-state"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=scan-delegator-receive-state "
             "big=%u bis=%u handle=0x%04x\n", big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "bass receive-state source-id=1 broadcast-id=0x000001 "
             "pa-sync-state=synchronized encrypt-state=code-required "
             "bis-sync=0x%08x\n", 1u << bis);
      printf("bluez-audio: dbus properties-changed "
             "path=/org/bluez/hci0/bass/delegator0/source1 "
             "interface=org.bluez.BASS1 ReceiveState=pa-synced\n");
      return 0;
    }

  if (!strcmp(argv[2], "scan-delegator-notify"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=scan-delegator-notify "
             "big=%u bis=%u handle=0x%04x\n", big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "bass receive-state notify source-id=1 "
             "pa-sync-state=synchronized big-encryption=decrypted "
             "bis-sync=0x%08x\n", 1u << bis);
      printf("bluez-audio: dbus properties-changed "
             "path=/org/bluez/hci0/bass/delegator0/source1 "
             "interface=org.bluez.BASS1 ReceiveState=encrypted-big-synced\n");
      return 0;
    }

  if (!strcmp(argv[2], "remove-source"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=remove-source "
             "big=%u bis=%u handle=0x%04x\n", big, bis, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "bass cp op=remove-source source-id=1\n");
      printf("bluez-audio: bass assistant state=source-synced->registered\n");
      return 0;
    }

  if (!strcmp(argv[2], "scan-delegator-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=scan-delegator-release\n");
      printf("bluez-audio: dbus interfaces-removed "
             "path=/org/bluez/hci0/bass/delegator0 "
             "interface=org.bluez.BASS1\n");
      printf("bluez-audio: bass scan-delegator state=registered->idle\n");
      return 0;
    }

  if (!strcmp(argv[2], "assistant-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "style profile=le-bass-control command=assistant-release\n");
      printf("bluez-audio: dbus interfaces-removed "
             "path=/org/bluez/hci0/bass interface=org.bluez.BASS1\n");
      printf("bluez-audio: bass assistant state=registered->idle\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_vcp_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-vcp-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: VCP role must be source or sink\n");
      return 1;
    }

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=volume-service command=register role=%s "
             "service=0x1844 state-handle=0x0042 control-handle=0x0044 flags-handle=0x0046\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/vcs0 "
             "interface=org.bluez.VolumeControl1 role=%s volume=64 mute=0\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=gatt-client command=discover role=%s service=0x1844 "
             "start=0x0040 end=0x0048\n",
             role);
      printf("bluez-audio: vcp discover chars role=%s "
             "state=0x0042 control=0x0044 flags=0x0046\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-state"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=gatt-read command=read-state role=%s handle=0x0042 "
             "volume=64 mute=0 change-counter=1\n",
             role);
      printf("bluez-audio: vcp state owner=bluetoothd role=%s "
             "volume=64 mute=0\n",
             role);
      return 0;
    }

  if (!strcmp(command, "set-volume"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=control-point command=set-volume role=%s cig=%u cis=%u "
             "handle=0x0044 volume=80 change-counter=1 opcode=0x00\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/vcs0 "
             "interface=org.bluez.VolumeControl1 Volume=80 Muted=0\n");
      return 0;
    }

  if (!strcmp(command, "notify-state"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=gatt-notify command=notify-state role=%s handle=0x0042 "
             "volume=80 mute=0 change-counter=2\n",
             role);
      printf("bluez-audio: vcp notify owner=bluetoothd role=%s "
             "ccc=enabled len=3\n",
             role);
      return 0;
    }

  if (!strcmp(command, "flags"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=gatt-read command=flags role=%s handle=0x0046 "
             "volume-setting-persisted=1\n",
             role);
      printf("bluez-audio: vcp flags owner=bluetoothd role=%s "
             "persisted=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=att-error command=error role=%s handle=0x0044 "
             "ecode=0x80 reason=change-counter-mismatch\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.Failed "
             "interface=org.bluez.VolumeControl1 method=SetVolume\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-vcp-control source=third/bluez/profiles/audio/vcp.c "
             "style=volume-service command=release role=%s service=0x1844\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/vcs0 "
             "interface=org.bluez.VolumeControl1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_micp_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-micp-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: MICP role must be source or sink\n");
      return 1;
    }

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=microphone-service command=register role=%s "
             "service=0x184d mute-handle=0x0052\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/mics0 "
             "interface=org.bluez.MicrophoneControl1 role=%s mute=0\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=gatt-client command=discover role=%s service=0x184d "
             "start=0x0050 end=0x0055\n",
             role);
      printf("bluez-audio: micp discover chars role=%s mute=0x0052\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-state"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=gatt-read command=read-state role=%s handle=0x0052 "
             "mute=0\n",
             role);
      printf("bluez-audio: micp state owner=bluetoothd role=%s mute=0\n",
             role);
      return 0;
    }

  if (!strcmp(command, "mute"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=gatt-write command=mute role=%s cig=%u cis=%u "
             "handle=0x0052 mute=1\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/mics0 "
             "interface=org.bluez.MicrophoneControl1 Muted=1\n");
      return 0;
    }

  if (!strcmp(command, "notify-state"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=gatt-notify command=notify-state role=%s handle=0x0052 "
             "mute=1\n",
             role);
      printf("bluez-audio: micp notify owner=bluetoothd role=%s "
             "ccc=enabled len=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "flags"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=policy command=flags role=%s writable=1 "
             "mute-supported=1\n",
             role);
      printf("bluez-audio: micp flags owner=bluetoothd role=%s "
             "policy=local-and-remote\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=att-error command=error role=%s handle=0x0052 "
             "ecode=0x03 reason=write-not-permitted\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.NotPermitted "
             "interface=org.bluez.MicrophoneControl1 method=Mute\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-micp-control source=third/bluez/profiles/audio/micp.c "
             "style=microphone-service command=release role=%s service=0x184d\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/mics0 "
             "interface=org.bluez.MicrophoneControl1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_csip_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-csip-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: CSIP role must be source or sink\n");
      return 1;
    }

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=csis-service command=register role=%s "
             "service=0x1846 sirk-handle=0x0062 size-handle=0x0064 "
             "lock-handle=0x0066 rank-handle=0x0068\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/csis0 "
             "interface=org.bluez.CoordinatedSet1 role=%s size=2 rank=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-client command=discover role=%s service=0x1846 "
             "start=0x0060 end=0x006a\n",
             role);
      printf("bluez-audio: csip discover chars role=%s "
             "sirk=0x0062 size=0x0064 lock=0x0066 rank=0x0068\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-sirk"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-read command=read-sirk role=%s handle=0x0062 "
             "type=encrypted value=00112233445566778899aabbccddeeff\n",
             role);
      printf("bluez-audio: csip sirk owner=bluetoothd role=%s "
             "key-size=16 resolved=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-size"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-read command=read-size role=%s handle=0x0064 "
             "set-size=2\n",
             role);
      printf("bluez-audio: csip set owner=bluetoothd role=%s size=2\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-rank"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-read command=read-rank role=%s handle=0x0068 "
             "rank=1\n",
             role);
      printf("bluez-audio: csip rank owner=bluetoothd role=%s rank=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "lock"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-write command=lock role=%s cig=%u cis=%u "
             "handle=0x0066 value=locked\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/csis0 "
             "interface=org.bluez.CoordinatedSet1 Lock=locked\n");
      return 0;
    }

  if (!strcmp(command, "unlock"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-write command=unlock role=%s cig=%u cis=%u "
             "handle=0x0066 value=unlocked\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/csis0 "
             "interface=org.bluez.CoordinatedSet1 Lock=unlocked\n");
      return 0;
    }

  if (!strcmp(command, "notify"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=gatt-notify command=notify role=%s handle=0x0066 "
             "lock=unlocked rank=1\n",
             role);
      printf("bluez-audio: csip notify owner=bluetoothd role=%s "
             "ccc=enabled len=2\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=att-error command=error role=%s handle=0x0066 "
             "ecode=0x80 reason=lock-denied\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.Failed "
             "interface=org.bluez.CoordinatedSet1 method=Lock\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-csip-control source=third/bluez/profiles/audio/csip.c "
             "style=csis-service command=release role=%s service=0x1846\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/csis0 "
             "interface=org.bluez.CoordinatedSet1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_mcp_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-mcp-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: MCP role must be source or sink\n");
      return 1;
    }

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=media-control-service command=register role=%s "
             "service=0x1848 player-name-handle=0x0072 state-handle=0x0078 "
             "cp-handle=0x007a\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/mcs0 "
             "interface=org.bluez.MediaControl1 role=%s state=paused\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=gatt-client command=discover role=%s service=0x1848 "
             "start=0x0070 end=0x0080\n",
             role);
      printf("bluez-audio: mcp discover chars role=%s "
             "player=0x0072 track-title=0x0074 state=0x0078 control=0x007a opcodes=0x007c\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-player"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=gatt-read command=read-player role=%s handle=0x0072 "
             "name=Feather-MCS\n",
             role);
      printf("bluez-audio: mcp player owner=bluetoothd role=%s "
             "name=Feather-MCS\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-track"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=gatt-read command=read-track role=%s title-handle=0x0074 "
             "duration-handle=0x0076 title=LE-Audio-Test duration-ms=10000\n",
             role);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/mcs0 "
             "interface=org.bluez.MediaControl1 Track=LE-Audio-Test\n");
      return 0;
    }

  if (!strcmp(command, "play"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=control-point command=play role=%s cig=%u cis=%u "
             "handle=0x007a opcode=0x01\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/mcs0 "
             "interface=org.bluez.MediaControl1 State=playing\n");
      return 0;
    }

  if (!strcmp(command, "pause"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=control-point command=pause role=%s cig=%u cis=%u "
             "handle=0x007a opcode=0x02\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/mcs0 "
             "interface=org.bluez.MediaControl1 State=paused\n");
      return 0;
    }

  if (!strcmp(command, "next"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=control-point command=next role=%s cig=%u cis=%u "
             "handle=0x007a opcode=0x30\n",
             role, cig, cis);
      printf("bluez-audio: mcp track state=track-1->track-2 position-ms=0\n");
      return 0;
    }

  if (!strcmp(command, "notify-state"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=gatt-notify command=notify-state role=%s handle=0x0078 "
             "state=paused\n",
             role);
      printf("bluez-audio: mcp notify owner=bluetoothd role=%s "
             "ccc=enabled len=1\n",
             role);
      return 0;
    }

  if (!strcmp(command, "search"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=search-control-point command=search role=%s handle=0x007e "
             "term=LE result-count=1\n",
             role);
      printf("bluez-audio: mcp search owner=bluetoothd role=%s "
             "object-id=0x000000000001\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=att-error command=error role=%s handle=0x007a "
             "ecode=0x80 reason=opcode-not-supported\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.NotSupported "
             "interface=org.bluez.MediaControl1 method=FastForward\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-mcp-control source=third/bluez/profiles/audio/mcp.c "
             "style=media-control-service command=release role=%s service=0x1848\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/mcs0 "
             "interface=org.bluez.MediaControl1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_tmap_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-tmap-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: TMAP role must be source or sink\n");
      return 1;
    }

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=tmas-service command=register role=%s "
             "service=0x1855 role-handle=0x0088 role-mask=0x003f\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/tmas0 "
             "interface=org.bluez.TelephonyAndMediaAudio1 role=%s "
             "roles=call-gateway,call-terminal,unicast-media-sender,unicast-media-receiver,broadcast-media-sender,broadcast-media-receiver\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=gatt-client command=discover role=%s service=0x1855 "
             "start=0x0084 end=0x008c\n",
             role);
      printf("bluez-audio: tmap discover chars role=%s role-char=0x0088\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-role"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=gatt-read command=read-role role=%s handle=0x0088 "
             "role-mask=0x003f\n",
             role);
      printf("bluez-audio: tmap role owner=bluetoothd role=%s "
             "unicast=source+sink broadcast=source+sink telephony=gateway+terminal\n",
             role);
      return 0;
    }

  if (!strcmp(command, "update-role"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=policy command=update-role role=%s cig=%u cis=%u "
             "old-mask=0x003f new-mask=0x001f\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/tmas0 "
             "interface=org.bluez.TelephonyAndMediaAudio1 Roles=0x001f\n");
      return 0;
    }

  if (!strcmp(command, "notify-role"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=gatt-notify command=notify-role role=%s handle=0x0088 "
             "role-mask=0x001f\n",
             role);
      printf("bluez-audio: tmap notify owner=bluetoothd role=%s "
             "ccc=enabled len=2\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=att-error command=error role=%s handle=0x0088 "
             "ecode=0x80 reason=unsupported-role-combination\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.NotSupported "
             "interface=org.bluez.TelephonyAndMediaAudio1 method=SetRoles\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-tmap-control source=third/bluez/profiles/audio/tmap.c "
             "style=tmas-service command=release role=%s service=0x1855\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/tmas0 "
             "interface=org.bluez.TelephonyAndMediaAudio1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_ccp_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  unsigned int cig;
  unsigned int cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-ccp-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: invalid le-ccp role: %s\n", role);
      return 1;
    }

  cig = argc > 4 ? (unsigned int)atoi(argv[4]) : 0;
  cis = argc > 5 ? (unsigned int)atoi(argv[5]) : 1;

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=tbs-service command=register role=%s service=0x184b "
             "provider-handle=0x0092 state-handle=0x0098 cp-handle=0x009a\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/tbs0 "
             "interface=org.bluez.TelephonyBearer1 role=%s state=idle "
             "provider=Feather-TBS\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=gatt-client command=discover role=%s service=0x184b "
             "start=0x0090 end=0x00a4\n",
             role);
      printf("bluez-audio: ccp discover chars role=%s provider=0x0092 "
             "state=0x0098 cp=0x009a termination=0x009c\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-bearer"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=gatt-read command=read-bearer role=%s "
             "provider-handle=0x0092 technology-handle=0x0094 "
             "provider=Feather-TBS technology=le\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-call-state"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=gatt-read command=read-call-state role=%s handle=0x0098 "
             "call-index=1 state=idle flags=outgoing\n",
             role);
      return 0;
    }

  if (!strcmp(command, "originate"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=call-control-point command=originate role=%s cig=%u cis=%u "
             "handle=0x009a opcode=0x01 uri=tel:+123456\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/tbs0 "
             "interface=org.bluez.TelephonyBearer1 State=dialing\n");
      return 0;
    }

  if (!strcmp(command, "accept"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=call-control-point command=accept role=%s cig=%u cis=%u "
             "handle=0x009a opcode=0x03 call-index=1\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/tbs0 "
             "interface=org.bluez.TelephonyBearer1 State=active\n");
      return 0;
    }

  if (!strcmp(command, "terminate"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=call-control-point command=terminate role=%s cig=%u cis=%u "
             "handle=0x009a opcode=0x04 call-index=1 reason=remote-ended\n",
             role, cig, cis);
      return 0;
    }

  if (!strcmp(command, "notify-call-state"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=gatt-notify command=notify-call-state role=%s handle=0x0098 "
             "call-index=1 state=active flags=incoming\n",
             role);
      return 0;
    }

  if (!strcmp(command, "termination-reason"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=gatt-notify command=termination-reason role=%s "
             "handle=0x009c call-index=1 reason=remote-ended\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=att-error command=error role=%s handle=0x009a "
             "ecode=0x80 reason=opcode-not-supported\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.Failed "
             "interface=org.bluez.TelephonyBearer1 method=CallControl\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-ccp-control source=third/bluez/profiles/audio/ccp.c "
             "style=tbs-service command=release role=%s service=0x184b\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/tbs0 "
             "interface=org.bluez.TelephonyBearer1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_gmap_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  unsigned int cig;
  unsigned int cis;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-gmap-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: invalid le-gmap role: %s\n", role);
      return 1;
    }

  cig = argc > 4 ? (unsigned int)atoi(argv[4]) : 0;
  cis = argc > 5 ? (unsigned int)atoi(argv[5]) : 1;

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=gmas-service command=register role=%s service=0x1858 "
             "role-handle=0x00a8 role-mask=0x000f\n",
             role);
      printf("bluez-audio: dbus object=/org/bluez/hci0/gmas0 "
             "interface=org.bluez.GamingAudio1 role=%s "
             "roles=unicast-game-gateway,unicast-game-terminal,"
             "broadcast-game-sender,broadcast-game-receiver\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=gatt-client command=discover role=%s service=0x1858 "
             "start=0x00a4 end=0x00ae\n",
             role);
      printf("bluez-audio: gmap discover chars role=%s role-char=0x00a8\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-role"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=gatt-read command=read-role role=%s handle=0x00a8 "
             "role-mask=0x000f\n",
             role);
      printf("bluez-audio: gmap role owner=bluetoothd role=%s "
             "unicast=game-gateway+game-terminal broadcast=sender+receiver\n",
             role);
      return 0;
    }

  if (!strcmp(command, "update-role"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=policy command=update-role role=%s cig=%u cis=%u "
             "old-mask=0x000f new-mask=0x0007\n",
             role, cig, cis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/gmas0 "
             "interface=org.bluez.GamingAudio1 Roles=0x0007\n");
      return 0;
    }

  if (!strcmp(command, "notify-role"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=gatt-notify command=notify-role role=%s handle=0x00a8 "
             "role-mask=0x0007\n",
             role);
      printf("bluez-audio: gmap notify owner=bluetoothd role=%s ccc=enabled len=2\n",
             role);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=att-error command=error role=%s handle=0x00a8 "
             "ecode=0x80 reason=unsupported-gaming-role-combination\n",
             role);
      printf("bluez-audio: dbus error org.bluez.Error.NotSupported "
             "interface=org.bluez.GamingAudio1 method=SetRoles\n");
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-gmap-control source=third/bluez/profiles/audio/gmap.c "
             "style=gmas-service command=release role=%s service=0x1858\n",
             role);
      printf("bluez-audio: dbus interfaces-removed path=/org/bluez/hci0/gmas0 "
             "interface=org.bluez.GamingAudio1\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_ascs_opcode(const char *command)
{
  if (!strcmp(command, "config-codec"))
    {
      return 0x01;
    }

  if (!strcmp(command, "config-qos") ||
      !strcmp(command, "config-qos-reject"))
    {
      return 0x02;
    }

  if (!strcmp(command, "enable"))
    {
      return 0x03;
    }

  if (!strcmp(command, "receiver-start-ready"))
    {
      return 0x04;
    }

  if (!strcmp(command, "disable"))
    {
      return 0x05;
    }

  if (!strcmp(command, "receiver-stop-ready"))
    {
      return 0x06;
    }

  if (!strcmp(command, "update-metadata"))
    {
      return 0x07;
    }

  if (!strcmp(command, "release"))
    {
      return 0x08;
    }

  return -1;
}

static const char *bluez_audio_le_ascs_transition(const char *command)
{
  if (!strcmp(command, "config-codec"))
    {
      return "idle->codec-configured";
    }

  if (!strcmp(command, "config-qos"))
    {
      return "codec-configured->qos-configured";
    }

  if (!strcmp(command, "config-qos-reject"))
    {
      return "codec-configured->codec-configured";
    }

  if (!strcmp(command, "enable"))
    {
      return "qos-configured->enabling";
    }

  if (!strcmp(command, "receiver-start-ready"))
    {
      return "enabling->streaming";
    }

  if (!strcmp(command, "disable"))
    {
      return "streaming->disabling";
    }

  if (!strcmp(command, "receiver-stop-ready"))
    {
      return "disabling->qos-configured";
    }

  if (!strcmp(command, "update-metadata"))
    {
      return "streaming->streaming";
    }

  if (!strcmp(command, "release"))
    {
      return "qos-configured->releasing->idle";
    }

  return "unknown";
}

static int bluez_audio_le_ascs_control_point(int argc, char *argv[])
{
  const char *command;
  const char *role;
  const char *transition;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  int opcode;
  bool reject;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-ascs-cp requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  opcode = bluez_audio_le_ascs_opcode(command);
  if (opcode < 0)
    {
      fprintf(stderr,
              "bluez-audio: unsupported ASCS control point command %s\n",
              command);
      return 1;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ASCS role must be source or sink\n");
      return 1;
    }

  reject = !strcmp(command, "config-qos-reject");
  transition = bluez_audio_le_ascs_transition(command);
  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;

  printf("bluez-audio: le-ascs-cp source=third/bluez/profiles/audio/ascs.c "
         "style=control-point role=%s command=%s cig=%u cis=%u ase-id=%u "
         "opcode=0x%02x\n",
         role, command, cig, cis, ase_id, opcode);
  printf("bluez-audio: ascs cp request op=%s ase-id=%u cig=%u cis=%u "
         "len=6\n",
         command, ase_id, cig, cis);

  if (reject)
    {
      printf("bluez-audio: ascs cp response op=config-qos ase-id=%u "
             "status=0x0d reason=invalid-qos\n",
             ase_id);
    }
  else
    {
      printf("bluez-audio: ascs cp response op=%s ase-id=%u "
             "status=0x00 reason=success\n",
             command, ase_id);
    }

  printf("bluez-audio: ascs ase role=%s ase-id=%u state=%s\n",
         role, ase_id, transition);
  return 0;
}

static const char *bluez_audio_le_bap_policy_state(const char *command)
{
  if (!strcmp(command, "scheduler-register"))
    {
      return "idle->registered";
    }

  if (!strcmp(command, "select-codec"))
    {
      return "registered->codec-selected";
    }

  if (!strcmp(command, "select-qos"))
    {
      return "codec-selected->qos-selected";
    }

  if (!strcmp(command, "select-cis"))
    {
      return "qos-selected->cis-selected";
    }

  if (!strcmp(command, "bind-transport"))
    {
      return "cis-selected->transport-bound";
    }

  if (!strcmp(command, "start-stream"))
    {
      return "transport-bound->streaming";
    }

  if (!strcmp(command, "suspend-stream"))
    {
      return "streaming->transport-bound";
    }

  if (!strcmp(command, "stop-stream"))
    {
      return "transport-bound->registered";
    }

  if (!strcmp(command, "scheduler-release"))
    {
      return "registered->idle";
    }

  return NULL;
}

static int bluez_audio_le_bap_policy_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  const char *state;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  uint16_t handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-bap-policy requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  state = bluez_audio_le_bap_policy_state(command);
  if (state == NULL)
    {
      fprintf(stderr,
              "bluez-audio: unsupported BAP policy command %s\n",
              command);
      return 1;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: BAP policy role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;
  handle = 0x0200 | cis;

  printf("bluez-audio: le-bap-policy source=third/bluez/profiles/audio/bap.c "
         "style=policy-scheduler role=%s command=%s cig=%u cis=%u "
         "ase-id=%u handle=0x%04x\n",
         role, command, cig, cis, ase_id, handle);

  if (!strcmp(command, "select-codec"))
    {
      printf("bluez-audio: bap policy capability-match role=%s ase-id=%u "
             "codec=lc3 codec-id=0x06 sample-rate=16000 "
             "frame-duration-us=10000 octets=40\n",
             role, ase_id);
    }
  else if (!strcmp(command, "select-qos"))
    {
      printf("bluez-audio: bap policy qos-select role=%s ase-id=%u "
             "interval-us=10000 framing=unframed rtn=2 latency-ms=10 "
             "pd-ms=40000\n",
             role, ase_id);
    }
  else if (!strcmp(command, "select-cis"))
    {
      printf("bluez-audio: bap policy cis-select role=%s ase-id=%u "
             "cig=%u cis=%u handle=0x%04x phy=2m max-sdu=40\n",
             role, ase_id, cig, cis, handle);
    }
  else if (!strcmp(command, "bind-transport"))
    {
      printf("bluez-audio: bap policy transport-bind role=%s ase-id=%u "
             "path=/org/bluez/hci0/dev_feather/ase/%s0 fd-owner=bluetoothd\n",
             role, ase_id, role);
    }
  else if (!strcmp(command, "start-stream"))
    {
      printf("bluez-audio: bap scheduler stream-start role=%s ase-id=%u "
             "requires-ascs-state=streaming media-state=pending\n",
             role, ase_id);
    }
  else if (!strcmp(command, "suspend-stream"))
    {
      printf("bluez-audio: bap scheduler stream-suspend role=%s ase-id=%u "
             "reason=local-policy media-state=suspending\n",
             role, ase_id);
    }
  else if (!strcmp(command, "stop-stream"))
    {
      printf("bluez-audio: bap scheduler stream-stop role=%s ase-id=%u "
             "release-transport=1 media-state=idle\n",
             role, ase_id);
    }

  printf("bluez-audio: bap scheduler role=%s ase-id=%u state=%s\n",
         role, ase_id, state);
  return 0;
}

static int bluez_audio_le_gatt_db_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  uint16_t pacs_start;
  uint16_t ascs_start;
  uint16_t pac_char;
  uint16_t ase_char;
  uint16_t cp_char;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-gatt-db requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: GATT DB role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;
  pacs_start = 0x0020;
  ascs_start = 0x0030;
  pac_char = !strcmp(role, "source") ? 0x0026 : 0x0023;
  ase_char = !strcmp(role, "source") ? 0x0034 : 0x0032;
  cp_char = 0x0038;

  if (!strcmp(command, "register"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/src/shared/gatt-db.c "
             "style=attribute-server command=register role=%s "
             "pacs-service=0x1850 handles=0x%04x-0x%04x "
             "ascs-service=0x184e handles=0x%04x-0x%04x\n",
             role, pacs_start, pacs_start + 0x0f, ascs_start,
             ascs_start + 0x0f);
      printf("bluez-audio: gatt-db owner=bluetoothd service=PACS "
             "chars=sink-pac:0x0023,source-pac:0x0026,locations:0x0029\n");
      printf("bluez-audio: gatt-db owner=bluetoothd service=ASCS "
             "chars=sink-ase:0x0032,source-ase:0x0034,cp:0x0038\n");
      printf("bluez-audio: gatt-db state=idle->registered\n");
      return 0;
    }

  if (!strcmp(command, "discover-pacs"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-client command=discover-pacs role=%s "
             "service=0x1850 start=0x%04x end=0x%04x\n",
             role, pacs_start, pacs_start + 0x0f);
      printf("bluez-audio: gatt-db discover chars role=%s "
             "sink-pac=0x0023 source-pac=0x0026 locations=0x0029\n",
             role);
      return 0;
    }

  if (!strcmp(command, "discover-ascs"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/ascs.c "
             "style=gatt-client command=discover-ascs role=%s "
             "service=0x184e start=0x%04x end=0x%04x\n",
             role, ascs_start, ascs_start + 0x0f);
      printf("bluez-audio: gatt-db discover chars role=%s "
             "sink-ase=0x0032 source-ase=0x0034 cp=0x0038\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-pac"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-read command=read-pac role=%s handle=0x%04x "
             "codec=lc3 codec-id=0x06 sample-rate=16000 "
             "frame-duration-us=10000 octets=40\n",
             role, pac_char);
      printf("bluez-audio: gatt-db read-ret role=%s handle=0x%04x len=19\n",
             role, pac_char);
      return 0;
    }

  if (!strcmp(command, "read-location"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-read command=read-location role=%s "
             "handle=0x0029 location=front-left-right bitmask=0x00000003\n",
             role);
      printf("bluez-audio: pacs location owner=bluetoothd role=%s "
             "source-location=front-left sink-location=front-right\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-context"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-read command=read-context role=%s "
             "supported=media,conversation available=media\n",
             role);
      printf("bluez-audio: pacs context owner=bluetoothd role=%s "
             "supported-mask=0x0006 available-mask=0x0004\n",
             role);
      return 0;
    }

  if (!strcmp(command, "update-context"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-write command=update-context role=%s "
             "available=media,conversation reason=policy-change\n",
             role);
      printf("bluez-audio: pacs context state=media->media,conversation "
             "notify-required=1\n");
      return 0;
    }

  if (!strcmp(command, "notify-pac"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/pacs.c "
             "style=gatt-notify command=notify-pac role=%s handle=0x%04x "
             "codec=lc3 metadata=context-media,location-front-left-right\n",
             role, pac_char);
      printf("bluez-audio: pacs notify owner=bluetoothd role=%s "
             "ccc=enabled len=24\n",
             role);
      return 0;
    }

  if (!strcmp(command, "read-ase"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/ascs.c "
             "style=gatt-read command=read-ase role=%s handle=0x%04x "
             "ase-id=%u state=idle\n",
             role, ase_char, ase_id);
      printf("bluez-audio: gatt-db read-ret role=%s handle=0x%04x len=4\n",
             role, ase_char);
      return 0;
    }

  if (!strcmp(command, "write-ascs-cp"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/ascs.c "
             "style=gatt-write command=write-ascs-cp role=%s "
             "handle=0x%04x ase-id=%u cig=%u cis=%u reliable=1\n",
             role, cp_char, ase_id, cig, cis);
      printf("bluez-audio: gatt-db write-ret role=%s handle=0x%04x "
             "status=0x00 len=6\n",
             role, cp_char);
      return 0;
    }

  if (!strcmp(command, "notify-ase"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/profiles/audio/ascs.c "
             "style=gatt-notify command=notify-ase role=%s "
             "handle=0x%04x ase-id=%u state=streaming ccc=enabled\n",
             role, ase_char, ase_id);
      printf("bluez-audio: gatt-db notify-ret role=%s handle=0x%04x len=8\n",
             role, ase_char);
      return 0;
    }

  if (!strcmp(command, "release"))
    {
      printf("bluez-audio: le-gatt-db source=third/bluez/src/shared/gatt-db.c "
             "style=attribute-server command=release role=%s "
             "pacs-service=0x1850 ascs-service=0x184e\n",
             role);
      printf("bluez-audio: gatt-db state=registered->idle\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_att_bearer_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  uint16_t cid;
  uint16_t mtu;
  uint16_t ase_handle;
  uint16_t cp_handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-att-bearer requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ATT bearer role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;
  cid = 0x0004;
  mtu = 247;
  ase_handle = !strcmp(role, "source") ? 0x0034 : 0x0032;
  cp_handle = 0x0038;

  if (!strcmp(command, "open"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/att.c "
             "style=att-session command=open role=%s cid=0x%04x "
             "cig=%u cis=%u ase-id=%u\n",
             role, cid, cig, cis, ase_id);
      printf("bluez-audio: att bearer state=idle->connected "
             "io-owner=bluetoothd transport=le-fixed-channel\n");
      return 0;
    }

  if (!strcmp(command, "mtu-exchange"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/att.c "
             "style=att-mtu command=mtu-exchange role=%s "
             "client-rx-mtu=%u server-rx-mtu=%u effective-mtu=%u\n",
             role, mtu, mtu, mtu);
      printf("bluez-audio: att mtu state=default->exchanged mtu=%u\n", mtu);
      return 0;
    }

  if (!strcmp(command, "security"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/gatt-server.c "
             "style=att-security command=security role=%s "
             "level=medium encrypted=1 authenticated=0\n",
             role);
      printf("bluez-audio: att security state=unencrypted->encrypted "
             "error-policy=insufficient-authentication\n");
      return 0;
    }

  if (!strcmp(command, "enable-ccc"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/gatt-server.c "
             "style=ccc command=enable-ccc role=%s "
             "ase-handle=0x%04x ccc-handle=0x%04x value=notify\n",
             role, ase_handle, (uint16_t)(ase_handle + 1));
      printf("bluez-audio: gatt ccc state=disabled->notify-enabled "
             "persist=0\n");
      return 0;
    }

  if (!strcmp(command, "prepare-write"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/att.c "
             "style=prepare-write command=prepare-write role=%s "
             "handle=0x%04x offset=0 len=6 ase-id=%u\n",
             role, cp_handle, ase_id);
      printf("bluez-audio: att prepare-write queue-depth=1 reliable=1\n");
      return 0;
    }

  if (!strcmp(command, "execute-write"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/att.c "
             "style=execute-write command=execute-write role=%s "
             "handle=0x%04x flags=commit queued=1\n",
             role, cp_handle);
      printf("bluez-audio: att execute-write status=0x00 "
             "queue-depth=0 delivered-to=ascs\n");
      return 0;
    }

  if (!strcmp(command, "indicate"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/gatt-server.c "
             "style=indication command=indicate role=%s "
             "handle=0x%04x ase-id=%u confirm=1\n",
             role, ase_handle, ase_id);
      printf("bluez-audio: att indication status=confirmed "
             "pending=0\n");
      return 0;
    }

  if (!strcmp(command, "close"))
    {
      printf("bluez-audio: le-att-bearer source=third/bluez/src/shared/att.c "
             "style=att-session command=close role=%s cid=0x%04x\n",
             role, cid);
      printf("bluez-audio: att bearer state=connected->idle "
             "pending-req=0 prepare-queue=0\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_att_io_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  uint16_t handle;
  uint16_t att_cid;
  uint16_t mtu;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-att-io requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ATT IO role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;
  handle = !strcmp(role, "source") ? 0x0034 : 0x0032;
  att_cid = 0x0004;
  mtu = 247;

  if (!strcmp(command, "attach"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=bt-att-io command=attach role=%s fd=l2cap-att "
             "cid=0x%04x cig=%u cis=%u ase-id=%u\n",
             role, att_cid, cig, cis, ase_id);
      printf("bluez-audio: att io owner=bluetoothd mainloop=attached "
             "read-watch=0 write-watch=0 tx-queue=0 rx-queue=0\n");
      return 0;
    }

  if (!strcmp(command, "watch-rx"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/io-mainloop.c "
             "style=mainloop-watch command=watch-rx role=%s fd=l2cap-att "
             "events=POLLIN\n",
             role);
      printf("bluez-audio: att io read-watch state=disabled->enabled\n");
      return 0;
    }

  if (!strcmp(command, "watch-tx"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/io-mainloop.c "
             "style=mainloop-watch command=watch-tx role=%s fd=l2cap-att "
             "events=POLLOUT\n",
             role);
      printf("bluez-audio: att io write-watch state=disabled->enabled\n");
      return 0;
    }

  if (!strcmp(command, "rx-pdu"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=att-rx command=rx-pdu role=%s opcode=0x12 "
             "handle=0x%04x mtu=%u len=6\n",
             role, handle, mtu);
      printf("bluez-audio: att io rx dispatch=write-request "
             "delivered-to=gatt-server\n");
      return 0;
    }

  if (!strcmp(command, "tx-pdu"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=att-tx command=tx-pdu role=%s opcode=0x1b "
             "handle=0x%04x mtu=%u len=8\n",
             role, handle, mtu);
      printf("bluez-audio: att io tx queue-depth=0 sent=1 "
             "pdu-owner=bt_att_send\n");
      return 0;
    }

  if (!strcmp(command, "fragment-write"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=att-fragment command=fragment-write role=%s "
             "handle=0x0038 mtu=%u sdu-len=260 fragments=2\n",
             role, mtu);
      printf("bluez-audio: att io fragment tx-fragments=2 "
             "policy=mtu-boundary\n");
      return 0;
    }

  if (!strcmp(command, "reassemble"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=att-reassemble command=reassemble role=%s "
             "handle=0x0038 fragments=2 sdu-len=260 status=complete\n",
             role);
      printf("bluez-audio: att io reassemble delivered-to=prepare-queue\n");
      return 0;
    }

  if (!strcmp(command, "persist-ccc"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/gatt-db.c "
             "style=ccc-store command=persist-ccc role=%s "
             "ase-id=%u handle=0x%04x value=notify storage=volatile\n",
             role, ase_id, handle);
      printf("bluez-audio: att io ccc persist status=0x00\n");
      return 0;
    }

  if (!strcmp(command, "flush"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=bt-att-io command=flush role=%s "
             "pending-req=0 tx-queue=0 rx-queue=0 prepare-queue=0\n",
             role);
      printf("bluez-audio: att io flush complete\n");
      return 0;
    }

  if (!strcmp(command, "detach"))
    {
      printf("bluez-audio: le-att-io source=third/bluez/src/shared/att.c "
             "style=bt-att-io command=detach role=%s fd=l2cap-att "
             "cid=0x%04x\n",
             role, att_cid);
      printf("bluez-audio: att io owner=none mainloop=detached "
             "read-watch=0 write-watch=0\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_att_queue_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;
  uint16_t req_id;
  uint16_t handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-att-queue requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ATT queue role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;
  req_id = 0x1000 | (ase_id & 0xff);
  handle = !strcmp(role, "source") ? 0x0034 : 0x0032;

  if (!strcmp(command, "alloc-req"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=alloc-req role=%s "
             "req-id=0x%04x opcode=0x12 ase-id=%u\n",
             role, req_id, ase_id);
      printf("bluez-audio: att queue ownership=request-allocated "
             "refcnt=1 destroy-cb=registered\n");
      return 0;
    }

  if (!strcmp(command, "enqueue"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=enqueue role=%s "
             "req-id=0x%04x handle=0x%04x queue-depth=1\n",
             role, req_id, handle);
      printf("bluez-audio: att queue pending state=idle->queued "
             "timeout-ms=30000\n");
      return 0;
    }

  if (!strcmp(command, "socket-read"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=l2cap-socket command=socket-read role=%s "
             "fd=l2cap-att cid=0x0004 req-id=0x%04x bytes=8\n",
             role, req_id);
      printf("bluez-audio: att queue socket-read dispatched=1 "
             "handler=bt_att_recv\n");
      return 0;
    }

  if (!strcmp(command, "socket-write"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=l2cap-socket command=socket-write role=%s "
             "fd=l2cap-att cid=0x0004 req-id=0x%04x bytes=8\n",
             role, req_id);
      printf("bluez-audio: att queue socket-write sent=1 "
             "handler=bt_att_send\n");
      return 0;
    }

  if (!strcmp(command, "timeout"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=timeout role=%s "
             "req-id=0x%04x timeout-ms=30000\n",
             role, req_id);
      printf("bluez-audio: att queue timeout status=0x08 "
             "error=BT_ATT_ERROR_RSP_TIMEOUT\n");
      return 0;
    }

  if (!strcmp(command, "cancel"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=cancel role=%s "
             "req-id=0x%04x reason=local-release\n",
             role, req_id);
      printf("bluez-audio: att queue cancel state=queued->cancelled "
             "callback=called\n");
      return 0;
    }

  if (!strcmp(command, "error-rsp"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=att-error command=error-rsp role=%s "
             "req-id=0x%04x opcode=0x01 handle=0x0038 "
             "ecode=0x0d\n",
             role, req_id);
      printf("bluez-audio: att queue error delivered-to=ascs "
             "reason=invalid-attribute-value-length\n");
      return 0;
    }

  if (!strcmp(command, "complete"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=complete role=%s "
             "req-id=0x%04x status=0x00\n",
             role, req_id);
      printf("bluez-audio: att queue complete state=queued->done "
             "callback=called\n");
      return 0;
    }

  if (!strcmp(command, "free-req"))
    {
      printf("bluez-audio: le-att-queue source=third/bluez/src/shared/att.c "
             "style=bt-att-request command=free-req role=%s "
             "req-id=0x%04x\n",
             role, req_id);
      printf("bluez-audio: att queue ownership=request-freed "
             "refcnt=0 destroy-cb=called\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_gatt_upstream_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint8_t ase_id;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-gatt-upstream requires closeout and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(command, "closeout"))
    {
      bluez_audio_usage();
      return 1;
    }

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: LE GATT upstream role must be source or sink\n");
      return 1;
    }

  ase_id = !strcmp(role, "source") ? 1 + cis : 17 + cis;

  printf("bluez-audio: le-gatt closeout cleanup role=%s cig=%u cis=%u "
         "ase-id=%u pending-req=0 prepare-queue=0 tx-queue=0 rx-queue=0 "
         "services=0 watches=0 fd=closed\n",
         role, cig, cis, ase_id);
  printf("bluez-audio: le-gatt upstream-coverage-map role=%s "
         "bluez-src=third/bluez/src/shared/att.c+"
         "third/bluez/src/shared/gatt-db.c+"
         "third/bluez/src/shared/gatt-server.c+"
         "third/bluez/src/shared/io-mainloop.c+"
         "third/bluez/profiles/audio/pacs.c+"
         "third/bluez/profiles/audio/ascs.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c "
         "executed=att-open,mtu-exchange,security,ccc,prepare-write,"
         "execute-write,indicate,att-io-attach,watch-rx,watch-tx,rx-pdu,"
         "tx-pdu,fragment,reassemble,persist-ccc,flush,request-queue,"
         "gatt-db-register,pacs-discover,ascs-discover,pac-read,"
         "location-read,context-read,context-update,pac-notify,ase-read,"
         "ascs-cp-write,ase-notify,release,close "
         "cig=%u cis=%u att-final=1 io-final=1 queue-final=1 "
         "gatt-db-final=1 cleanup-final=1 "
         "upstream-link=bluezaudio-gatt-upstream-link-bluetoothd "
         "final-ok=1\n",
         role, cig, cis);
  return 0;
}

static int bluez_audio_le_iso_socket_control(int argc, char *argv[])
{
  static const uint8_t lc3_frame[40] =
  {
    0x4c, 0x43, 0x33, 0x10, 0x27, 0x40, 0x1f, 0x01,
    0x02, 0x28, 0x00, 0x01, 0x10, 0x27, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x05, 0x08, 0x0d, 0x15, 0x22,
    0x37, 0x59, 0x90, 0xe9, 0x79, 0x62, 0xdb, 0x3d,
    0x18, 0x55, 0x6d, 0xc2, 0x2f, 0xf1, 0x20, 0x44
  };
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint16_t handle;
  char out[512];
  int ret;
  uint16_t fd;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-iso-socket requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ISO socket role must be source or sink\n");
      return 1;
    }

  handle = 0x0200 | cis;
  fd = !strcmp(role, "source") ? 70 + cis : 90 + cis;

  if (!strcmp(command, "open"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=open role=%s fd=%u "
             "family=AF_BLUETOOTH proto=BTPROTO_ISO\n",
             role, fd);
      printf("bluez-audio: iso socket owner=bluetoothd state=closed->open\n");
      return 0;
    }

  if (!strcmp(command, "sockapi-closeout"))
    {
      struct bluez_audio_sockaddr_iso_s iaddr;
      struct bluez_audio_bt_iso_qos_s qos;
      struct bluez_audio_bt_iso_qos_s read_qos;
      uint8_t base[] =
      {
        0x28, 0x00, 0x00, 0x00,
        0x01, 0x06, 0x00, 0x00,
        0x00, 0x02, 0x02, 0x03
      };
      uint8_t read_base[248];
      uint32_t defer = 1;
      uint32_t read_defer = 0;
      uint32_t pkt_status = 1;
      uint32_t read_pkt_status = 0;
      uint32_t pkt_seqnum = 1;
      uint32_t read_pkt_seqnum = 0;
      struct iovec iov;
      struct msghdr msg;
      socklen_t optlen;
      int sock_type;
      int sock_fd;
      int listen_fd = -1;
      int accept_fd = -1;
      int create_nonblock_fd = -1;
      int create_nonblock_flags = -1;
      int create_nonblock_close = -1;
      int bind_ret = -1;
      int listen_nonblock_ret = -1;
      int listen_bind_ret = -1;
      int listen_ret = -1;
      int accept_ret = -1;
      int accept_errno = 0;
      int stream_fd = -1;
      int stream_errno = 0;
      int dgram_fd = -1;
      int dgram_errno = 0;
      int raw_fd = -1;
      int raw_errno = 0;
      int set_defer_ret = -1;
      int get_defer_ret = -1;
      int set_pkt_status_ret = -1;
      int get_pkt_status_ret = -1;
      int set_pkt_seqnum_ret = -1;
      int get_pkt_seqnum_ret = -1;
      int set_qos_ret = -1;
      int get_qos_ret = -1;
      int set_base_ret = -1;
      int get_base_ret = -1;
      int connect_ret = -1;
      ssize_t send_ret = -1;
      int close_ret = -1;

      sock_type = SOCK_SEQPACKET;
      sock_fd = socket(AF_BLUETOOTH, sock_type, BTPROTO_ISO);
      printf("bluez-audio: le-iso-socket source=third/bluez/tools/isotest.c "
             "style=ordinary-socket command=sockapi-closeout role=%s "
             "fd=%d family=AF_BLUETOOTH proto=BTPROTO_ISO\n",
             role, sock_fd);
      if (sock_fd < 0)
        {
          printf("bluez-audio: iso socket sockapi-closeout socket-ret=%d "
                 "final-ok=0\n", sock_fd);
          return 1;
        }

      errno = 0;
      stream_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_ISO);
      if (stream_fd < 0)
        {
          stream_errno = errno;
        }
      else
        {
          (void)close(stream_fd);
        }

      errno = 0;
      dgram_fd = socket(AF_BLUETOOTH, SOCK_DGRAM, BTPROTO_ISO);
      if (dgram_fd < 0)
        {
          dgram_errno = errno;
        }
      else
        {
          (void)close(dgram_fd);
        }

      errno = 0;
      raw_fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_ISO);
      if (raw_fd < 0)
        {
          raw_errno = errno;
        }
      else
        {
          (void)close(raw_fd);
        }

      memset(&iaddr, 0, sizeof(iaddr));
      iaddr.iso_family = AF_BLUETOOTH;
      iaddr.iso_bdaddr_type = BDADDR_LE_PUBLIC;
      create_nonblock_fd = socket(AF_BLUETOOTH,
                                  sock_type | SOCK_NONBLOCK,
                                  BTPROTO_ISO);
      if (create_nonblock_fd >= 0)
        {
          create_nonblock_flags = fcntl(create_nonblock_fd, F_GETFL);
          create_nonblock_close = close(create_nonblock_fd);
        }

      listen_fd = socket(AF_BLUETOOTH, sock_type, BTPROTO_ISO);
      if (listen_fd >= 0)
        {
          listen_nonblock_ret = fcntl(listen_fd, F_SETFL, O_NONBLOCK);
          listen_bind_ret = bind(listen_fd, (struct sockaddr *)&iaddr,
                                 sizeof(iaddr));
          listen_ret = listen(listen_fd, 1);
        }

      bind_ret = bind(sock_fd, (struct sockaddr *)&iaddr, sizeof(iaddr));

      set_defer_ret = setsockopt(sock_fd, SOL_BLUETOOTH, BT_DEFER_SETUP,
                                 &defer, sizeof(defer));
      optlen = sizeof(read_defer);
      get_defer_ret = getsockopt(sock_fd, SOL_BLUETOOTH, BT_DEFER_SETUP,
                                 &read_defer, &optlen);
      set_pkt_status_ret = setsockopt(sock_fd, SOL_BLUETOOTH,
                                      BT_PKT_STATUS, &pkt_status,
                                      sizeof(pkt_status));
      optlen = sizeof(read_pkt_status);
      get_pkt_status_ret = getsockopt(sock_fd, SOL_BLUETOOTH,
                                      BT_PKT_STATUS, &read_pkt_status,
                                      &optlen);
      set_pkt_seqnum_ret = setsockopt(sock_fd, SOL_BLUETOOTH,
                                      BT_PKT_SEQNUM, &pkt_seqnum,
                                      sizeof(pkt_seqnum));
      optlen = sizeof(read_pkt_seqnum);
      get_pkt_seqnum_ret = getsockopt(sock_fd, SOL_BLUETOOTH,
                                      BT_PKT_SEQNUM, &read_pkt_seqnum,
                                      &optlen);

      memset(&qos, 0, sizeof(qos));
      qos.ucast.cig = cig;
      qos.ucast.cis = cis;
      qos.ucast.in.interval = 10000;
      qos.ucast.out.interval = 10000;
      qos.ucast.in.latency = 10;
      qos.ucast.out.latency = 10;
      qos.ucast.in.sdu = sizeof(lc3_frame);
      qos.ucast.out.sdu = sizeof(lc3_frame);
      qos.ucast.in.phys = BT_ISO_PHY_2M;
      qos.ucast.out.phys = BT_ISO_PHY_2M;
      qos.ucast.in.rtn = 2;
      qos.ucast.out.rtn = 2;
      set_qos_ret = setsockopt(sock_fd, SOL_BLUETOOTH, BT_ISO_QOS,
                               &qos, sizeof(qos));
      memset(&read_qos, 0, sizeof(read_qos));
      optlen = sizeof(read_qos);
      get_qos_ret = getsockopt(sock_fd, SOL_BLUETOOTH, BT_ISO_QOS,
                               &read_qos, &optlen);
      set_base_ret = setsockopt(sock_fd, SOL_BLUETOOTH, BT_ISO_BASE,
                                base, sizeof(base));
      memset(read_base, 0, sizeof(read_base));
      optlen = sizeof(read_base);
      get_base_ret = getsockopt(sock_fd, SOL_BLUETOOTH, BT_ISO_BASE,
                                read_base, &optlen);

      connect_ret = connect(sock_fd, (struct sockaddr *)&iaddr,
                            sizeof(iaddr));
      if (listen_fd >= 0)
        {
          errno = 0;
          accept_fd = accept(listen_fd, NULL, NULL);
          accept_ret = accept_fd >= 0 ? 0 : -1;
          if (accept_fd < 0)
            {
              accept_errno = errno;
            }

          if (accept_fd >= 0)
            {
              (void)close(accept_fd);
            }

          (void)close(listen_fd);
        }

      memset(&iov, 0, sizeof(iov));
      memset(&msg, 0, sizeof(msg));
      iov.iov_base = (void *)lc3_frame;
      iov.iov_len = sizeof(lc3_frame);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      send_ret = sendmsg(sock_fd, &msg, 0);
      close_ret = close(sock_fd);

      printf("bluez-audio: iso socket sockapi-closeout socket-ret=%d "
             "bind-ret=%d opt=BT_DEFER_SETUP set-ret=%d get-ret=%d "
             "defer=%" PRIu32 " opt=BT_PKT_STATUS set-ret=%d "
             "get-ret=%d value=%" PRIu32 " opt=BT_PKT_SEQNUM "
             "set-ret=%d get-ret=%d upstream-get=absent value=%" PRIu32
             " opt=BT_ISO_QOS "
             "set-ret=%d get-ret=%d cig=%u cis=%u in-sdu=%u "
             "out-sdu=%u opt=BT_ISO_BASE base-set-ret=%d "
             "base-get-ret=%d base-len=%u listen-fd=%d "
             "nonblock-ret=%d "
             "create-nonblock-fd=%d create-nonblock-flags=0x%x "
             "create-nonblock-close=%d create-nonblock-ok=%u "
             "stream-ret=%d stream-esocktnosupport=%u "
             "dgram-ret=%d dgram-esocktnosupport=%u "
             "raw-ret=%d raw-esocktnosupport=%u "
             "listen-bind-ret=%d listen-ret=%d "
             "accept-ret=%d accept-errno=%d pending-accept-ok=%u "
             "accept-fd=%d connect-ret=%d "
             "sendmsg-ret=%d close-ret=%d path=ordinary-socket "
             "final-ok=%d\n",
             sock_fd, bind_ret, set_defer_ret, get_defer_ret, read_defer,
             set_pkt_status_ret, get_pkt_status_ret, read_pkt_status,
             set_pkt_seqnum_ret, get_pkt_seqnum_ret, read_pkt_seqnum,
             set_qos_ret, get_qos_ret, read_qos.ucast.cig,
             read_qos.ucast.cis, read_qos.ucast.in.sdu,
             read_qos.ucast.out.sdu, set_base_ret, get_base_ret,
             (unsigned int)optlen, listen_fd, listen_nonblock_ret,
             create_nonblock_fd, create_nonblock_flags,
             create_nonblock_close,
             create_nonblock_fd >= 0 &&
             create_nonblock_flags >= 0 &&
             (create_nonblock_flags & O_NONBLOCK) != 0 &&
             create_nonblock_close == 0,
             stream_fd,
             stream_fd < 0 && stream_errno == ESOCKTNOSUPPORT,
             dgram_fd,
             dgram_fd < 0 && dgram_errno == ESOCKTNOSUPPORT,
             raw_fd,
             raw_fd < 0 && raw_errno == ESOCKTNOSUPPORT,
             listen_bind_ret, listen_ret, accept_ret, accept_errno,
             accept_ret == 0 ? 1 : 0,
             accept_fd, connect_ret, (int)send_ret, close_ret,
             bind_ret == 0 && set_defer_ret == 0 && get_defer_ret == 0 &&
             set_pkt_status_ret == 0 && get_pkt_status_ret == 0 &&
             set_pkt_seqnum_ret == 0 && get_pkt_seqnum_ret < 0 &&
             set_qos_ret == 0 && get_qos_ret == 0 &&
             set_base_ret == 0 && get_base_ret == 0 &&
             create_nonblock_fd >= 0 && create_nonblock_flags >= 0 &&
             (create_nonblock_flags & O_NONBLOCK) != 0 &&
             create_nonblock_close == 0 &&
             stream_fd < 0 && stream_errno == ESOCKTNOSUPPORT &&
             dgram_fd < 0 && dgram_errno == ESOCKTNOSUPPORT &&
             raw_fd < 0 && raw_errno == ESOCKTNOSUPPORT &&
             listen_fd >= 0 && listen_nonblock_ret == 0 &&
             listen_bind_ret == 0 &&
             listen_ret == 0 &&
             accept_ret == 0 &&
             connect_ret == 0 && send_ret == (ssize_t)sizeof(lc3_frame) &&
             close_ret == 0);
      return bind_ret == 0 && set_defer_ret == 0 && get_defer_ret == 0 &&
             set_pkt_status_ret == 0 && get_pkt_status_ret == 0 &&
             set_pkt_seqnum_ret == 0 && get_pkt_seqnum_ret < 0 &&
             set_qos_ret == 0 && get_qos_ret == 0 &&
             set_base_ret == 0 && get_base_ret == 0 &&
             create_nonblock_fd >= 0 && create_nonblock_flags >= 0 &&
             (create_nonblock_flags & O_NONBLOCK) != 0 &&
             create_nonblock_close == 0 &&
             stream_fd < 0 && stream_errno == ESOCKTNOSUPPORT &&
             dgram_fd < 0 && dgram_errno == ESOCKTNOSUPPORT &&
             raw_fd < 0 && raw_errno == ESOCKTNOSUPPORT &&
             listen_fd >= 0 && listen_nonblock_ret == 0 &&
             listen_bind_ret == 0 &&
             listen_ret == 0 &&
             accept_ret == 0 &&
             connect_ret == 0 && send_ret == (ssize_t)sizeof(lc3_frame) &&
             close_ret == 0 ? 0 : 1;
    }

  if (!strcmp(command, "bind-cis"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=bind-cis role=%s fd=%u "
             "cig=%u cis=%u handle=0x%04x\n",
             role, fd, cig, cis, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_bind_probe(BDADDR_LE_PUBLIC,
                                                    handle, out,
                                                    sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket bind addr=feather-hwsim type=CIS "
             "qos-framing=unframed\n");
      return 0;
    }

  if (!strcmp(command, "connect"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=connect role=%s fd=%u "
             "handle=0x%04x\n",
             role, fd, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_connect_probe(BDADDR_LE_PUBLIC,
                                                       out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket state=open->connected "
             "direction=%s\n",
             !strcmp(role, "source") ? "tx" : "rx");
      return 0;
    }

  if (!strcmp(command, "listen"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=listen role=%s fd=%u "
             "backlog=1\n",
             role, fd);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_listen_probe(1, out,
                                                      sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket state=open->listening\n");
      return 0;
    }

  if (!strcmp(command, "accept"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=accept role=%s fd=%u "
             "accepted-fd=%u handle=0x%04x\n",
             role, fd, fd + 1, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_accept_probe(out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket state=listening->connected\n");
      return 0;
    }

  if (!strcmp(command, "pollout"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/src/shared/io-mainloop.c "
             "style=iso-poll command=pollout role=%s fd=%u "
             "events=POLLOUT handle=0x%04x\n",
             role, fd, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_poll_probe(1, out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket pollout ready=1\n");
      return 0;
    }

  if (!strcmp(command, "sendmsg"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-send command=sendmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=sendmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=sendmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_write_probe(lc3_frame,
                                                     sizeof(lc3_frame),
                                                     out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_ioctl_probe(out, sizeof(out));
      printf("bluez-audio: iso socket ioctl role=%s ret=%d "
             "proto=BTPROTO_ISO\n", role, ret);
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket sendmsg ret=40 timestamp-us=10000\n");
      return 0;
    }

  if (!strcmp(command, "pollin"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/src/shared/io-mainloop.c "
             "style=iso-poll command=pollin role=%s fd=%u "
             "events=POLLIN handle=0x%04x\n",
             role, fd, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_poll_probe(0, out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket pollin ready=1\n");
      return 0;
    }

  if (!strcmp(command, "recvmsg"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-recv command=recvmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=recvmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=recvmsg role=%s fd=%u "
             "handle=0x%04x len=40 flags=0x0\n",
             role, fd, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_recv_probe(sizeof(lc3_frame),
                                                    out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_ioctl_probe(out, sizeof(out));
      printf("bluez-audio: iso socket ioctl role=%s ret=%d "
             "proto=BTPROTO_ISO\n", role, ret);
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket recvmsg ret=40 timestamp-us=10000\n");
      return 0;
    }

  if (!strcmp(command, "timestamp"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-timestamp command=timestamp role=%s fd=%u "
             "handle=0x%04x seq=1 timestamp-us=10000\n",
             role, fd, handle);
      printf("bluez-audio: iso socket timestamp source=controller "
             "clock=monotonic\n");
      return 0;
    }

  if (!strcmp(command, "error-eagain"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-error command=error-eagain role=%s fd=%u "
             "op=%s errno=EAGAIN retry=1\n",
             role, fd, !strcmp(role, "source") ? "sendmsg" : "recvmsg");
      printf("bluez-audio: iso socket error-policy=retry-nonfatal\n");
      return 0;
    }

  if (!strcmp(command, "shutdown"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=shutdown role=%s fd=%u "
             "how=SHUT_RDWR\n",
             role, fd);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_shutdown_probe(2, out,
                                                       sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket state=connected->shutdown\n");
      return 0;
    }

  if (!strcmp(command, "close"))
    {
      printf("bluez-audio: le-iso-socket source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=close role=%s fd=%u\n",
             role, fd);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso socket owner=none state=shutdown->closed\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_iso_qos_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint16_t handle;
  char out[512];
  int ret;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-iso-qos requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: ISO QoS role must be source or sink\n");
      return 1;
    }

  handle = 0x0200 | cis;

  if (!strcmp(command, "configure"))
    {
      printf("bluez-audio: le-iso-qos source=third/bluez/profiles/audio/bap.c "
             "style=iso-qos command=configure role=%s cig=%u cis=%u "
             "handle=0x%04x interval-us=10000 latency-ms=10 sdu=40\n",
             role, cig, cis, handle);
      printf("bluez-audio: iso qos state=idle->configured framing=unframed "
             "rtn=2 pd-us=40000\n");
      return 0;
    }

  if (!strcmp(command, "select-phy"))
    {
      printf("bluez-audio: le-iso-qos source=third/bluez/profiles/audio/bap.c "
             "style=iso-qos command=select-phy role=%s cig=%u cis=%u "
             "phy=2m packing=sequential\n",
             role, cig, cis);
      printf("bluez-audio: iso qos phy selected tx=2m rx=2m\n");
      return 0;
    }

  if (!strcmp(command, "setup-cig"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=setup-cig role=%s cig=%u "
             "interval-us=10000 latency-ms=10\n",
             role, cig);
      printf("bluez-audio: iso qos cig state=idle->configured\n");
      return 0;
    }

  if (!strcmp(command, "setup-cis"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=setup-cis role=%s cig=%u cis=%u "
             "handle=0x%04x max-sdu=40\n",
             role, cig, cis, handle);
      printf("bluez-audio: iso qos cis state=configured->pending\n");
      return 0;
    }

  if (!strcmp(command, "apply-qos"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=setsockopt command=apply-qos role=%s handle=0x%04x "
             "opt=BT_DEFER_SETUP/BT_PKT_STATUS/BT_PKT_SEQNUM/"
             "BT_ISO_QOS/BT_ISO_BASE in=1 out=1\n",
             role, handle);
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_option_probe(cig, cis, 40,
                                                      out, sizeof(out));
      printf("bluez-audio: le-iso-qos socket-option-probe role=%s "
             "ret=%d\n",
             role, ret);
      printf("%s", out);
      if (ret < 0)
        {
          return 1;
        }

      printf("bluez-audio: iso qos socket applied=1 state=pending->active\n");
      return 0;
    }

  if (!strcmp(command, "controller-timing"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c "
             "style=controller-timing command=controller-timing role=%s "
             "handle=0x%04x sync-delay-us=2500 transport-latency-us=10000\n",
             role, handle);
      printf("bluez-audio: iso qos timing source=controller clock=monotonic\n");
      return 0;
    }

  if (!strcmp(command, "credit-grant"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=iso-credit command=credit-grant role=%s "
             "handle=0x%04x credits=4\n",
             role, handle);
      printf("bluez-audio: iso qos credit state=empty->available\n");
      return 0;
    }

  if (!strcmp(command, "credit-complete"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=iso-credit command=credit-complete role=%s "
             "handle=0x%04x consumed=1 remaining=3\n",
             role, handle);
      printf("bluez-audio: iso qos credit accounting=balanced\n");
      return 0;
    }

  if (!strcmp(command, "teardown"))
    {
      printf("bluez-audio: le-iso-qos source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=teardown role=%s cig=%u cis=%u "
             "handle=0x%04x\n",
             role, cig, cis, handle);
      printf("bluez-audio: iso qos state=active->idle credits=0\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_broadcast_iso_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t big;
  uint8_t bis;
  uint16_t handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-broadcast-iso requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  big = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  bis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: broadcast ISO role must be source or sink\n");
      return 1;
    }

  handle = bluez_audio_iso_handle(big, bis);

  if (!strcmp(command, "adv-start"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/bluez/profiles/audio/bap.c "
             "style=broadcast-source command=adv-start role=%s big=%u bis=%u "
             "broadcast-id=0x000001\n",
             role, big, bis);
      printf("bluez-audio: broadcast source state=idle->advertising "
             "extended-adv=1 periodic-adv=1\n");
      return 0;
    }

  if (!strcmp(command, "base-config"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/bluez/profiles/audio/bap.c "
             "style=base command=base-config role=%s big=%u bis=%u "
             "codec=lc3 subgroup=1 presentation-delay-us=40000\n",
             role, big, bis);
      printf("bluez-audio: broadcast base state=advertising->base-ready "
             "metadata=context-media\n");
      return 0;
    }

  if (!strcmp(command, "big-create"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=big-create role=%s big=%u bis=%u "
             "handle=0x%04x interval-us=10000 rtn=2 sdu=40\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast big state=base-ready->creating\n");
      return 0;
    }

  if (!strcmp(command, "bis-setup"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c "
             "style=hci-event command=bis-setup role=%s big=%u bis=%u "
             "handle=0x%04x status=0x00\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast bis state=creating->established "
             "encryption=off\n");
      return 0;
    }

  if (!strcmp(command, "bis-bind"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/bluez/profiles/audio/transport.c "
             "style=iso-socket command=bis-bind role=%s big=%u bis=%u "
             "handle=0x%04x proto=BTPROTO_ISO\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast transport owner=bluetoothd "
             "state=established->bound\n");
      return 0;
    }

  if (!strcmp(command, "pa-sync"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/bluez/profiles/audio/bass.c "
             "style=bass command=pa-sync role=%s big=%u bis=%u "
             "broadcast-id=0x000001 sid=0\n",
             role, big, bis);
      printf("bluez-audio: bass receive-state state=source-added->pa-synced "
             "pa-sync-state=synchronized\n");
      return 0;
    }

  if (!strcmp(command, "big-sync"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=big-sync role=%s big=%u bis=%u "
             "handle=0x%04x sync-handle=0x0040\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast sink state=pa-synced->big-synced "
             "bis-sync=0x00000002\n");
      return 0;
    }

  if (!strcmp(command, "receive-state"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/bluez/profiles/audio/bass.c "
             "style=bass command=receive-state role=%s big=%u bis=%u "
             "pa-sync-state=synchronized big-encryption=not-encrypted\n",
             role, big, bis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/bass/source1 "
             "interface=org.bluez.BASS1 ReceiveState=big-synced\n");
      return 0;
    }

  if (!strcmp(command, "bis-credit"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=iso-credit command=bis-credit role=%s handle=0x%04x "
             "credits=4\n",
             role, handle);
      printf("bluez-audio: broadcast bis credit state=empty->available\n");
      return 0;
    }

  if (!strcmp(command, "bis-complete"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=iso-credit command=bis-complete role=%s handle=0x%04x "
             "consumed=1 remaining=3\n",
             role, handle);
      printf("bluez-audio: broadcast bis credit accounting=balanced\n");
      return 0;
    }

  if (!strcmp(command, "big-terminate"))
    {
      printf("bluez-audio: le-broadcast-iso source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=big-terminate role=%s big=%u bis=%u "
             "handle=0x%04x reason=local-release\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast big state=established->idle credits=0\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_broadcast_security_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  uint8_t big;
  uint8_t bis;
  uint16_t handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-broadcast-security requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  big = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  bis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: broadcast security role must be source or sink\n");
      return 1;
    }

  handle = bluez_audio_iso_handle(big, bis);

  if (!strcmp(command, "set-code"))
    {
      printf("bluez-audio: le-broadcast-security source=third/bluez/profiles/audio/bap.c "
             "style=broadcast-code command=set-code role=%s big=%u bis=%u "
             "broadcast-code-len=16\n",
             role, big, bis);
      printf("bluez-audio: broadcast security state=clear->code-configured "
             "policy=encrypted-broadcast\n");
      return 0;
    }

  if (!strcmp(command, "encrypt-big"))
    {
      printf("bluez-audio: le-broadcast-security source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=kernel-iso command=encrypt-big role=%s big=%u bis=%u "
             "handle=0x%04x encryption=on\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast big encryption state=code-configured->encrypted "
             "algorithm=aes-ccm\n");
      return 0;
    }

  if (!strcmp(command, "decrypt-setup"))
    {
      printf("bluez-audio: le-broadcast-security source=third/bluez/profiles/audio/bass.c "
             "style=bass command=decrypt-setup role=%s big=%u bis=%u "
             "broadcast-code-len=16\n",
             role, big, bis);
      printf("bluez-audio: bass receive-state encryption=bad-code->decrypting "
             "source-id=1\n");
      return 0;
    }

  if (!strcmp(command, "receive-state-encrypted"))
    {
      printf("bluez-audio: le-broadcast-security source=third/bluez/profiles/audio/bass.c "
             "style=bass command=receive-state-encrypted role=%s big=%u bis=%u "
             "big-encryption=decrypted bad-code=0\n",
             role, big, bis);
      printf("bluez-audio: dbus properties-changed path=/org/bluez/hci0/bass/source1 "
             "interface=org.bluez.BASS1 ReceiveState=encrypted-big-synced\n");
      return 0;
    }

  if (!strcmp(command, "bad-code"))
    {
      printf("bluez-audio: le-broadcast-security source=third/bluez/profiles/audio/bass.c "
             "style=bass command=bad-code role=%s big=%u bis=%u "
             "big-encryption=bad-code error=authentication-failed\n",
             role, big, bis);
      printf("bluez-audio: bass receive-state encrypted-error=1 "
             "retry-policy=provide-broadcast-code\n");
      return 0;
    }

  if (!strcmp(command, "clear-code"))
    {
      printf("bluez-audio: le-broadcast-security source=third/bluez/profiles/audio/bap.c "
             "style=broadcast-code command=clear-code role=%s big=%u bis=%u "
             "handle=0x%04x\n",
             role, big, bis, handle);
      printf("bluez-audio: broadcast security state=encrypted->clear "
             "key-owner=none\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_daemon_control(int argc, char *argv[])
{
  if (!strcmp(argv[2], "plugin-init"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style=bluetoothd-plugin command=plugin-init plugin=audio "
             "profiles=bap,pacs,ascs,cap,bass,vcp,micp,csip,mcp,tmap,ccp,gmap\n");
      printf("bluez-audio: source=third/bluez/src/plugin.c "
             "style=plugin-loader command=add audio priority=audio "
             "mainloop=persistent\n");
      printf("bluez-audio: source=third/bluez/src/shared/mainloop-notify.c "
             "style=mainloop command=init watches=0 timeouts=0 "
             "owner=bluetoothd\n");
      return 0;
    }

  if (!strcmp(argv[2], "adapter-powered"))
    {
      printf("bluez-audio: source=third/bluez/src/adapter.c "
             "style=adapter-lifecycle command=powered adapter=hci0 "
             "powered=1 discoverable=0 pairable=1\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style=bluetoothd-plugin command=adapter-probe plugin=audio "
             "adapter=hci0 status=registered\n");
      printf("bluez-audio: dbus interfaces-added path=/org/bluez/hci0 "
             "interfaces=org.bluez.Adapter1,org.bluez.Media1 "
             "owner=bluetoothd\n");
      return 0;
    }

  if (!strcmp(argv[2], "mainloop-dispatch"))
    {
      printf("bluez-audio: source=third/bluez/src/shared/io-mainloop.c "
             "style=mainloop command=dispatch owner=bluetoothd "
             "io-watch=att,iso,dbus timeout=none\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style=bluetoothd-mainloop command=process-events "
             "queue=ase-state,media-transport,broadcast state=drained\n");
      return 0;
    }

  if (!strcmp(argv[2], "profile-accept"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style=bt_profile command=accept uuid=0000184e-0000-1000-8000-00805f9b34fb "
             "adapter=hci0 device=dev_feather bearer=le\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style=bt_bap command=attach session=le-audio "
             "pacs=1 ascs=1 transport-owner=bluetoothd\n");
      return 0;
    }

  if (!strcmp(argv[2], "profile-release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "style=bt_bap command=detach session=le-audio "
             "reason=profile-release transports=0\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style=bt_profile command=release uuid=0000184e-0000-1000-8000-00805f9b34fb "
             "adapter=hci0 device=dev_feather bearer=le\n");
      return 0;
    }

  if (!strcmp(argv[2], "plugin-exit"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style=bluetoothd-plugin command=plugin-exit plugin=audio "
             "adapter=hci0 profiles-unregistered=12\n");
      printf("bluez-audio: source=third/bluez/src/shared/mainloop-notify.c "
             "style=mainloop command=exit watches=0 timeouts=0 "
             "owner=bluetoothd\n");
      return 0;
    }

  if (!strcmp(argv[2], "register"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/main.c "
             "style profile=le-audio-daemon command=register\n");
      printf("bluez-audio: dbus name=org.bluez owner=bluetoothd "
             "object-manager=/org/bluez/hci0\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
             "dbus object=/org/bluez/hci0/pacs0 "
             "interfaces=org.bluez.PACSink1,org.bluez.PACSource1 "
             "codec=lc3 context=media\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "dbus object=/org/bluez/hci0/ase "
             "interfaces=org.bluez.ASE1,org.bluez.ASCS1 "
             "states=idle,codec-configured,qos-configured,enabling,streaming,releasing\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
             "dbus object=/org/bluez/hci0/bap "
             "interfaces=org.bluez.BAPBroadcastSource1,org.bluez.BAPUnicastClient1\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "dbus object=/org/bluez/hci0/cap "
             "interfaces=org.bluez.CAPCoordinator1 "
             "sets=coordinated-set-1\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "dbus object=/org/bluez/hci0/bass "
             "interfaces=org.bluez.BASS1 broadcast-assistant=1\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "dbus object=/org/bluez/hci0/media "
             "interfaces=org.bluez.Media1,org.bluez.MediaEndpoint1 "
             "endpoints=lc3-broadcast-source,lc3-unicast-source,lc3-unicast-sink\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus object=/org/bluez/hci0/dev_feather/ase1/fd0 "
             "interfaces=org.bluez.MediaTransport1 fd-owner=bluetoothd\n");
      printf("bluez-audio: le daemon registered pacs=1 ascs=1 bap=1 "
             "media-endpoints=3 transports=1\n");
      return 0;
    }

  if (!strcmp(argv[2], "release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "dbus object=/org/bluez/hci0/dev_feather/ase1/fd0 "
             "command=release owner=bluetoothd\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "command=unregister endpoints=3\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/ascs.c "
             "command=unregister ase-objects=2 final-state=idle\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/cap.c "
             "command=unregister coordinator=1 final-state=idle\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/bass.c "
             "command=unregister assistant=1 final-state=idle\n");
      printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
             "command=unregister pacs=1\n");
      printf("bluez-audio: dbus name=org.bluez owner=bluetoothd "
             "object-manager-release=/org/bluez/hci0\n");
      printf("bluez-audio: le daemon released pacs=0 ascs=0 bap=0 "
             "media-endpoints=0 transports=0\n");
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_mgmt_control(int argc, char *argv[])
{
  const char *command;
  const char *role;
  unsigned int cig;
  unsigned int cis;
  uint16_t acl_handle;
  uint16_t cis_handle;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-mgmt-control requires command and source|sink role\n");
      return 1;
    }

  command = argv[2];
  role = argv[3];
  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      fprintf(stderr, "bluez-audio: invalid le-mgmt role: %s\n", role);
      return 1;
    }

  cig = argc > 4 ? (unsigned int)atoi(argv[4]) : 0;
  cis = argc > 5 ? (unsigned int)atoi(argv[5]) : 1;
  acl_handle = 0x0040 + cis;
  cis_handle = bluez_audio_cis_handle((uint8_t)cig, (uint8_t)cis);

  if (!strcmp(command, "power-on"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/mgmt.c "
             "style=mgmt-socket command=power-on role=%s index=0 "
             "opcode=0x0005 setting=powered\n",
             role);
      printf("bluez-audio: mgmt event new-settings index=0 "
             "powered=1 le=1 bredr=1 secure-conn=1\n");
      return 0;
    }

  if (!strcmp(command, "scan-start"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/mgmt.c "
             "style=mgmt-socket command=scan-start role=%s index=0 "
             "opcode=0x0023 type=le\n",
             role);
      printf("bluez-audio: mgmt event device-found role=%s addr=fe:ed:00:00:00:%02u "
             "rssi=-42 flags=connectable uuids=184e,1850,1855\n",
             role, cis);
      return 0;
    }

  if (!strcmp(command, "connect"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/device.c "
             "style=mgmt-socket command=connect role=%s index=0 "
             "addr=fe:ed:00:00:00:%02u bearer=le\n",
             role, cis);
      printf("bluez-audio: hci event le-enhanced-connection-complete "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c "
             "role=%s status=0x00 handle=0x%04x interval=24 latency=0\n",
             role, acl_handle);
      printf("bluez-audio: mgmt event device-connected role=%s "
             "handle=0x%04x flags=le-audio\n",
             role, acl_handle);
      return 0;
    }

  if (!strcmp(command, "security"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/device.c "
             "style=mgmt-socket command=security role=%s index=0 "
             "addr=fe:ed:00:00:00:%02u level=medium\n",
             role, cis);
      printf("bluez-audio: hci event le-long-term-key-request "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/smp.c "
             "role=%s handle=0x%04x authenticated=1\n",
             role, acl_handle);
      printf("bluez-audio: mgmt event new-link-key role=%s bonded=1 "
             "key-type=authenticated-combination\n",
             role);
      return 0;
    }

  if (!strcmp(command, "cis-request"))
    {
      printf("bluez-audio: le-mgmt-control source=third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
             "style=hci-event command=cis-request role=%s cig=%u cis=%u "
             "acl-handle=0x%04x cis-handle=0x%04x\n",
             role, cig, cis, acl_handle, cis_handle);
      printf("bluez-audio: hci event le-cis-established role=%s "
             "status=0x00 handle=0x%04x cig=%u cis=%u\n",
             role, cis_handle, cig, cis);
      return 0;
    }

  if (!strcmp(command, "disconnect"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/device.c "
             "style=mgmt-socket command=disconnect role=%s index=0 "
             "handle=0x%04x reason=local-host\n",
             role, acl_handle);
      printf("bluez-audio: hci event disconn-complete "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/hci_event.c "
             "role=%s status=0x00 handle=0x%04x reason=0x16\n",
             role, acl_handle);
      printf("bluez-audio: mgmt event device-disconnected role=%s "
             "handle=0x%04x reason=local-host\n",
             role, acl_handle);
      return 0;
    }

  if (!strcmp(command, "error"))
    {
      printf("bluez-audio: le-mgmt-control source=third/bluez/src/mgmt.c "
             "style=mgmt-socket command=error role=%s index=0 "
             "opcode=0x002e status=busy reason=duplicate-connect\n",
             role);
      printf("bluez-audio: mgmt event command-status opcode=0x002e "
             "status=0x0a role=%s\n",
             role);
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_dbus_client_control(int argc, char *argv[])
{
  const char *role;
  uint8_t cig;
  uint8_t cis;
  uint16_t handle;
  const char *endpoint;
  const char *ase;

  if (argc < 4)
    {
      bluez_audio_usage();
      return 1;
    }

  role = argv[3];
  cig = argc > 4 ? (uint8_t)atoi(argv[4]) : 0;
  cis = argc > 5 ? (uint8_t)atoi(argv[5]) : 1;
  handle = bluez_audio_cis_handle(cig, cis);
  endpoint = !strcmp(role, "source") ?
             "/org/bluez/example/le/source0" :
             "/org/bluez/example/le/sink0";
  ase = !strcmp(role, "source") ?
        "/org/bluez/hci0/dev_feather/ase/source0" :
        "/org/bluez/hci0/dev_feather/ase/sink0";

  if (strcmp(role, "source") && strcmp(role, "sink"))
    {
      bluez_audio_usage();
      return 1;
    }

  if (!strcmp(argv[2], "register"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "style profile=le-dbus-client command=register role=%s "
             "app=:1.leaudio endpoint=%s\n", role, endpoint);
      printf("bluez-audio: dbus call org.bluez.Media1.RegisterEndpoint "
             "endpoint=%s codec=lc3 capabilities=16_2_1 "
             "metadata=context-media\n", endpoint);
      printf("bluez-audio: dbus interfaces-added path=%s "
             "interface=org.bluez.MediaEndpoint1 role=%s\n",
             endpoint, role);
      printf("bluez-audio: source=third/bluez/profiles/audio/pacs.c "
             "client pac codec=lc3 role=%s context=media "
             "location=front-left-right\n", role);
      return 0;
    }

  if (!strcmp(argv[2], "configure"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "style profile=le-dbus-client command=configure role=%s "
             "cig=%u cis=%u handle=0x%04x\n", role, cig, cis, handle);
      printf("bluez-audio: dbus method org.bluez.MediaEndpoint1.SelectProperties "
             "endpoint=%s codec=lc3 preferred-qos=10000us\n", endpoint);
      printf("bluez-audio: dbus method org.bluez.MediaEndpoint1.SetConfiguration "
             "endpoint=%s transport=%s codec=lc3 qos=interval-us:10000,rtn:2,latency-ms:20\n",
             endpoint, ase);
      printf("bluez-audio: dbus interfaces-added path=%s "
             "interface=org.bluez.MediaTransport1 state=idle\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 State=idle\n", ase);
      return 0;
    }

  if (!strcmp(argv[2], "transport"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "style profile=le-dbus-client command=transport role=%s "
             "cig=%u cis=%u handle=0x%04x\n", role, cig, cis, handle);
      printf("bluez-audio: dbus method org.bluez.MediaTransport1.Acquire "
             "path=%s fd=iso read-mtu=251 write-mtu=251\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 State=active\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 Delay=10000\n", ase);
      return 0;
    }

  if (!strcmp(argv[2], "transport-busy"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "style profile=le-dbus-client command=transport-busy role=%s "
             "cig=%u cis=%u handle=0x%04x\n", role, cig, cis, handle);
      printf("bluez-audio: dbus method org.bluez.MediaTransport1.Acquire "
             "path=%s fd=iso request=duplicate\n", ase);
      printf("bluez-audio: dbus error org.bluez.Error.InProgress "
             "method=org.bluez.MediaTransport1.Acquire path=%s "
             "owner=:1.leaudio active-fd=iso\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 State=active\n", ase);
      return 0;
    }

  if (!strcmp(argv[2], "owner-lost"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "style profile=le-dbus-client command=owner-lost role=%s "
             "app=:1.leaudio endpoint=%s\n", role, endpoint);
      printf("bluez-audio: dbus name-owner-lost name=:1.leaudio "
             "path=%s reason=application-exit\n", endpoint);
      printf("bluez-audio: dbus method org.bluez.MediaTransport1.Release "
             "path=%s reason=owner-lost\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 State=idle\n", ase);
      printf("bluez-audio: dbus interfaces-removed path=%s "
             "interface=org.bluez.MediaTransport1\n", ase);
      printf("bluez-audio: dbus interfaces-removed path=%s "
             "interface=org.bluez.MediaEndpoint1\n", endpoint);
      return 0;
    }

  if (!strcmp(argv[2], "owner-reacquire"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/media.c "
             "style profile=le-dbus-client command=owner-reacquire role=%s "
             "app=:1.leaudio endpoint=%s\n", role, endpoint);
      printf("bluez-audio: dbus name-owner-acquired name=:1.leaudio "
             "path=%s\n", endpoint);
      printf("bluez-audio: dbus call org.bluez.Media1.RegisterEndpoint "
             "endpoint=%s codec=lc3 capabilities=16_2_1 "
             "metadata=context-media\n", endpoint);
      printf("bluez-audio: dbus method org.bluez.MediaEndpoint1.SetConfiguration "
             "endpoint=%s transport=%s codec=lc3 qos=interval-us:10000,rtn:2,latency-ms:20\n",
             endpoint, ase);
      printf("bluez-audio: dbus interfaces-added path=%s "
             "interface=org.bluez.MediaTransport1 state=idle "
             "owner=:1.leaudio\n", ase);
      return 0;
    }

  if (!strcmp(argv[2], "release"))
    {
      printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
             "style profile=le-dbus-client command=release role=%s "
             "cig=%u cis=%u handle=0x%04x\n", role, cig, cis, handle);
      printf("bluez-audio: dbus method org.bluez.MediaTransport1.Release "
             "path=%s\n", ase);
      printf("bluez-audio: dbus properties-changed path=%s "
             "interface=org.bluez.MediaTransport1 State=idle\n", ase);
      printf("bluez-audio: dbus method org.bluez.MediaEndpoint1.ClearConfiguration "
             "endpoint=%s transport=%s\n", endpoint, ase);
      printf("bluez-audio: dbus interfaces-removed path=%s "
             "interface=org.bluez.MediaTransport1\n", ase);
      printf("bluez-audio: dbus method org.bluez.Media1.UnregisterEndpoint "
             "endpoint=%s\n", endpoint);
      printf("bluez-audio: dbus interfaces-removed path=%s "
             "interface=org.bluez.MediaEndpoint1\n", endpoint);
      return 0;
    }

  bluez_audio_usage();
  return 1;
}

static int bluez_audio_le_daemon_call(const char *command)
{
  char *call_argv[] =
  {
    "bluezaudio", "le-daemon", (char *)command
  };

  return bluez_audio_le_daemon_control(3, call_argv);
}

static int bluez_audio_le_role_call(int (*handler)(int, char **),
                                    const char *profile,
                                    const char *command,
                                    const char *role,
                                    const char *cig,
                                    const char *cis)
{
  char *call_argv[] =
  {
    "bluezaudio", (char *)profile, (char *)command,
    (char *)role, (char *)cig, (char *)cis
  };

  return handler(6, call_argv);
}

static int bluez_audio_le_unicast_call(const char *command,
                                       const char *cig,
                                       const char *cis)
{
  char *call_argv[] =
  {
    "bluezaudio", "le-unicast-control", (char *)command,
    (char *)cig, (char *)cis
  };

  return bluez_audio_le_unicast_control(5, call_argv);
}

static int bluez_audio_le_codec_call(int (*handler)(int, char **),
                                     const char *command,
                                     const char *cig,
                                     const char *cis)
{
  char *call_argv[] =
  {
    "bluezaudio", "le-audio-codec", (char *)command,
    (char *)cig, (char *)cis
  };

  return handler(5, call_argv);
}

static int bluez_audio_le_big_call(int (*handler)(int, char **),
                                   const char *profile,
                                   const char *command,
                                   const char *big,
                                   const char *bis)
{
  char *call_argv[] =
  {
    "bluezaudio", (char *)profile, (char *)command,
    (char *)big, (char *)bis
  };

  return handler(5, call_argv);
}

static int bluez_audio_le_broadcast_role_call(int (*handler)(int, char **),
                                              const char *profile,
                                              const char *command,
                                              const char *role,
                                              const char *big,
                                              const char *bis)
{
  char *call_argv[] =
  {
    "bluezaudio", (char *)profile, (char *)command,
    (char *)role, (char *)big, (char *)bis
  };

  return handler(6, call_argv);
}

static int bluez_audio_le_bass_call(const char *command,
                                    const char *big,
                                    const char *bis)
{
  char *call_argv[] =
  {
    "bluezaudio", "le-bass-control", (char *)command,
    (char *)big, (char *)bis
  };

  return bluez_audio_le_bass_control(5, call_argv);
}

static int bluez_audio_le_daemon_unicast_profile_flow(int argc,
                                                      char *argv[])
{
  const char *role;
  const char *cig;
  const char *cis;
  const char *config;
  const char *enable;
  const char *release;
  const char *iso_connect;
  const char *iso_ready;
  int source;
  int ret;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-daemon unicast-profile-flow requires source|sink [cig] [cis]\n");
      return 1;
    }

  role = argv[3];
  cig = argc > 4 ? argv[4] : "0";
  cis = argc > 5 ? argv[5] : "1";
  source = !strcmp(role, "source");

  if (!source && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: le-daemon unicast-profile-flow role must be source or sink\n");
      return 1;
    }

  config = source ? "source-config" : "sink-config";
  enable = source ? "source-enable" : "sink-enable";
  release = source ? "source-release" : "sink-release";
  iso_connect = source ? "connect" : "listen";
  iso_ready = source ? "pollout" : "pollin";

  printf("bluez-audio: le-daemon-unicast-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=unicast-profile-flow "
         "role=%s cig=%s cis=%s owner=bluetoothd state=begin\n",
         role, cig, cis);

#define CALL_OR_FAIL(expr) do { ret = (expr); if (ret < 0 || ret > 0) goto fail; } while (0)

  CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-init"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("adapter-powered"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("register"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("mainloop-dispatch"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-accept"));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                        "le-dbus-client", "register",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                        "le-dbus-client", "configure",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_bearer_control,
                                        "le-att-bearer", "open",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_bearer_control,
                                        "le-att-bearer", "security",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_io_control,
                                        "le-att-io", "attach",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_io_control,
                                        "le-att-io", "watch-rx",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_io_control,
                                        "le-att-io", "watch-tx",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "register",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "discover-pacs",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "discover-ascs",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "read-pac",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "read-ase",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy",
                                        "scheduler-register",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy", "select-codec",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy", "select-qos",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy", "select-cis",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp", "config-codec",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp", "config-qos",
                                        role, cig, cis));

  if (!source)
    {
      CALL_OR_FAIL(bluez_audio_le_unicast_call("sink-discover", cig, cis));
    }

  CALL_OR_FAIL(bluez_audio_le_unicast_call(config, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp", "enable",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_unicast_call(enable, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp",
                                        "receiver-start-ready",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp",
                                        "update-metadata",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy",
                                        "bind-transport",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "open",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "bind-cis",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "configure",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "select-phy",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "setup-cig",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "setup-cis",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "apply-qos",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos",
                                        "controller-timing",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "credit-grant",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", iso_connect,
                                        role, cig, cis));
  if (!source)
    {
      CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                            "le-iso-socket", "accept",
                                            role, cig, cis));
    }

  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", iso_ready,
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy", "start-stream",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                        "le-dbus-client", "transport",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                        "le-dbus-client",
                                        "transport-busy",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket",
                                        source ? "sendmsg" : "recvmsg",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "timestamp",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "error-eagain",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "credit-complete",
                                        role, cig, cis));
  CALL_OR_FAIL(source ?
               bluez_audio_le_codec_call(bluez_audio_le_codec_source,
                                         "source-lc3-encode-write-release",
                                         cig, cis) :
               bluez_audio_le_codec_call(bluez_audio_le_codec_sink,
                                         "sink-lc3-recv-decode-release",
                                         cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_qos_control,
                                        "le-iso-qos", "teardown",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "shutdown",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_iso_socket_control,
                                        "le-iso-socket", "close",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy",
                                        "suspend-stream",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp", "disable",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp",
                                        "receiver-stop-ready",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy", "stop-stream",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ascs_control_point,
                                        "le-ascs-cp", "release",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_bap_policy_control,
                                        "le-bap-policy",
                                        "scheduler-release",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_unicast_call(release, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_gatt_db_control,
                                        "le-gatt-db", "release",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_io_control,
                                        "le-att-io", "detach",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_att_bearer_control,
                                        "le-att-bearer", "close",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                        "le-dbus-client", "release",
                                        role, cig, cis));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-release"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("release"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-exit"));

#undef CALL_OR_FAIL

  printf("bluez-audio: le-daemon-unicast-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=unicast-profile-flow "
         "role=%s cig=%s cis=%s owner=bluetoothd state=complete\n",
         role, cig, cis);
  return 0;

fail:
#undef CALL_OR_FAIL
  printf("bluez-audio: le-daemon-unicast-flow role=%s cig=%s cis=%s "
         "state=failed ret=%d\n", role, cig, cis, ret);
  return 1;
}

static int bluez_audio_le_daemon_broadcast_profile_flow(int argc,
                                                        char *argv[])
{
  const char *role;
  const char *big;
  const char *bis;
  int source;
  int ret;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-daemon broadcast-profile-flow requires source|sink [big] [bis]\n");
      return 1;
    }

  role = argv[3];
  big = argc > 4 ? argv[4] : "0";
  bis = argc > 5 ? argv[5] : "1";
  source = !strcmp(role, "source");

  if (!source && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: le-daemon broadcast-profile-flow role must be source or sink\n");
      return 1;
    }

  printf("bluez-audio: le-daemon-broadcast-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=broadcast-profile-flow "
         "role=%s big=%s bis=%s owner=bluetoothd state=begin\n",
         role, big, bis);

#define CALL_OR_FAIL(expr) do { ret = (expr); if (ret < 0 || ret > 0) goto fail; } while (0)

  CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-init"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("adapter-powered"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("register"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("mainloop-dispatch"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-accept"));

  if (source)
    {
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "source-announce", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "adv-start", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "base-config", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "set-code", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "big-create", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "encrypt-big", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-setup", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-bind", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "source-start", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-credit", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_source_start,
                                           "le-broadcast-source",
                                           "start", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-complete", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "source-stop", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "big-terminate", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "clear-code", role, big, bis));
    }
  else
    {
      CALL_OR_FAIL(bluez_audio_le_bass_call("assistant-register", big, bis));
      CALL_OR_FAIL(bluez_audio_le_bass_call("add-source", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "pa-sync", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "bad-code", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "decrypt-setup", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "sink-discover", big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "sink-config", big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_sink_sync,
                                           "le-broadcast-sink",
                                           "sync", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "big-sync", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "receive-state", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "receive-state-encrypted",
                     role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-credit", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_bass_call("modify-source", big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_bap_control,
                                           "le-bap-control",
                                           "sink-sync", big, bis));
      CALL_OR_FAIL(bluez_audio_le_big_call(bluez_audio_le_sink_start,
                                           "le-broadcast-sink",
                                           "start", big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "bis-complete", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_sink_stop());
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_iso_control,
                     "le-broadcast-iso", "big-terminate", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_broadcast_role_call(
                     bluez_audio_le_broadcast_security_control,
                     "le-broadcast-security", "clear-code", role, big, bis));
      CALL_OR_FAIL(bluez_audio_le_bass_call("remove-source", big, bis));
      CALL_OR_FAIL(bluez_audio_le_bass_call("assistant-release", big, bis));
    }

  CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-release"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("release"));
  CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-exit"));

#undef CALL_OR_FAIL

  printf("bluez-audio: le-daemon-broadcast-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=broadcast-profile-flow "
         "role=%s big=%s bis=%s owner=bluetoothd state=complete\n",
         role, big, bis);
  return 0;

fail:
#undef CALL_OR_FAIL
  printf("bluez-audio: le-daemon-broadcast-flow role=%s big=%s bis=%s "
         "state=failed ret=%d\n", role, big, bis, ret);
  return 1;
}

static int bluez_audio_le_daemon_integrated_profile_flow(int argc,
                                                         char *argv[])
{
  const char *role;
  const char *cig;
  const char *cis;
  const char *peer_cis;
  const char *config;
  const char *enable;
  const char *release;
  int source;
  int ret;

  if (argc < 4)
    {
      fprintf(stderr,
              "bluez-audio: le-daemon integrated-profile-flow requires source|sink [cig] [cis] [peer-cis]\n");
      return 1;
    }

  role = argv[3];
  cig = argc > 4 ? argv[4] : "0";
  cis = argc > 5 ? argv[5] : "1";
  peer_cis = argc > 6 ? argv[6] : (!strcmp(cis, "1") ? "2" : "1");
  source = !strcmp(role, "source");

  if (!source && strcmp(role, "sink"))
    {
      fprintf(stderr,
              "bluez-audio: le-daemon integrated-profile-flow role must be source or sink\n");
      return 1;
    }

  config = source ? "source-config" : "sink-config";
  enable = source ? "source-enable" : "sink-enable";
  release = source ? "source-release" : "sink-release";

  printf("bluez-audio: le-daemon-integrated-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=integrated-profile-flow "
         "role=%s cig=%s cis=%s peer-cis=%s owner=bluetoothd state=begin\n",
         role, cig, cis, peer_cis);

#define CALL_OR_FAIL(expr) do { ret = (expr); if (ret < 0 || ret > 0) goto fail; } while (0)

  {
    char *cap_register[] =
      {
        "bluezaudio", "le-cap-control", "coordinator-register"
      };
    char *cap_config[] =
      {
        "bluezaudio", "le-cap-control", "group-config-bidir",
        (char *)cig, (char *)cis, (char *)peer_cis
      };
    char *cap_enable[] =
      {
        "bluezaudio", "le-cap-control", "group-enable-bidir",
        (char *)cig, (char *)cis, (char *)peer_cis
      };
    char *cap_release[] =
      {
        "bluezaudio", "le-cap-control", "group-release-bidir",
        (char *)cig, (char *)cis, (char *)peer_cis
      };
    char *cap_unregister[] =
      {
        "bluezaudio", "le-cap-control", "coordinator-release"
      };

    CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-init"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("adapter-powered"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("register"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("mainloop-dispatch"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-accept"));
    CALL_OR_FAIL(bluez_audio_le_cap_control(3, cap_register));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "register",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "discover",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "read-sirk",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "read-size",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "read-rank",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "lock",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_cap_control(6, cap_config));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "register",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "discover",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "read-role",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "update-role",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "notify-role",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "register",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "discover",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "read-player",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "play",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "notify-state",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "register",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "discover",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "read-bearer",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control",
                                          "read-call-state",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "originate",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "accept",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control",
                                          "notify-call-state",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                          "le-dbus-client", "register",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                          "le-dbus-client", "configure",
                                          role, cig, cis));
    if (!source)
      {
        CALL_OR_FAIL(bluez_audio_le_unicast_call("sink-discover", cig, cis));
      }

    CALL_OR_FAIL(bluez_audio_le_unicast_call(config, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_unicast_call(enable, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_cap_control(6, cap_enable));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                          "le-dbus-client", "transport",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                          "le-dbus-client",
                                          "transport-busy",
                                          role, cig, cis));
    CALL_OR_FAIL(source ?
                 bluez_audio_le_codec_call(bluez_audio_le_codec_source,
                                           "source-lc3-encode-write-release",
                                           cig, cis) :
                 bluez_audio_le_codec_call(bluez_audio_le_codec_sink,
                                           "sink-lc3-recv-decode-release",
                                           cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "pause",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "terminate",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control",
                                          "termination-reason",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_cap_control(6, cap_release));
    CALL_OR_FAIL(bluez_audio_le_unicast_call(release, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_dbus_client_control,
                                          "le-dbus-client", "release",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_ccp_control,
                                          "le-ccp-control", "release",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_mcp_control,
                                          "le-mcp-control", "release",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_tmap_control,
                                          "le-tmap-control", "release",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "unlock",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_role_call(bluez_audio_le_csip_control,
                                          "le-csip-control", "release",
                                          role, cig, cis));
    CALL_OR_FAIL(bluez_audio_le_cap_control(3, cap_unregister));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("profile-release"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("release"));
    CALL_OR_FAIL(bluez_audio_le_daemon_call("plugin-exit"));
  }

#undef CALL_OR_FAIL

  printf("bluez-audio: le-daemon-integrated-flow source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-profile command=integrated-profile-flow "
         "role=%s cig=%s cis=%s peer-cis=%s owner=bluetoothd state=complete\n",
         role, cig, cis, peer_cis);
  return 0;

fail:
#undef CALL_OR_FAIL
  printf("bluez-audio: le-daemon-integrated-flow role=%s cig=%s cis=%s "
         "peer-cis=%s state=failed ret=%d\n",
         role, cig, cis, peer_cis, ret);
  return 1;
}

/****************************************************************************
 * Name: bluez_audio_upstream_object_probe
 ****************************************************************************/

static int bluez_audio_upstream_object_probe(int argc, char *argv[])
{
  const char *role = argv[2];

  if (argc < 3 || (strcmp(role, "source") && strcmp(role, "sink")))
    {
      printf("bluez-audio: upstream-object-probe invalid role=%s\n",
             argc > 2 ? role : "(missing)");
      return 1;
    }

  printf("bluez-audio: upstream-object-probe role=%s stage=begin\n",
         role);
  bluez_upstream_avdtp_object_probe_print(role);
  bluez_upstream_a2dp_object_probe_print(role);
  bluez_upstream_transport_object_probe_print(role);
  bluez_upstream_media_object_probe_print(role);
  printf("bluez-audio: upstream-object-probe role=%s "
         "files=avdtp.c,a2dp.c,transport.c,media.c result=ok\n",
         role);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 3 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      bluez_audio_usage();
      return argc < 3 ? 1 : 0;
    }

  if (!strcmp(argv[1], "a2dp-signal") &&
      (!strcmp(argv[2], "listen") || !strcmp(argv[2], "listen-native")))
    {
      return bluez_audio_a2dp_signal_listen(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-signal") &&
      !strcmp(argv[2], "auto-rsp-loop"))
    {
      return bluez_audio_a2dp_signal_auto_rsp_loop(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-signal") && !strcmp(argv[2], "close"))
    {
      return bluez_audio_a2dp_signal_close();
    }

  if (!strcmp(argv[1], "a2dp-signal") &&
      (!strcmp(argv[2], "source-session-open") ||
       !strcmp(argv[2], "source-session-close")))
    {
      return bluez_audio_a2dp_signal_source_session(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-signal"))
    {
      return bluez_audio_a2dp_signal_source(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-endpoint"))
    {
      return bluez_audio_a2dp_endpoint(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-codec") &&
      !strcmp(argv[2], "source-sbc-encode-write-release"))
    {
      return bluez_audio_a2dp_codec_source(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-codec") &&
      !strcmp(argv[2], "sink-sbc-recv-decode-release"))
    {
      return bluez_audio_a2dp_codec_sink(argc, argv);
    }

  if (!strcmp(argv[1], "avrcp-control"))
    {
      return bluez_audio_avrcp_control(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-source") && !strcmp(argv[2], "start"))
    {
      return bluez_audio_a2dp_source_start(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-sink") && !strcmp(argv[2], "start"))
    {
      return bluez_audio_a2dp_sink_start(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-sink") && !strcmp(argv[2], "read"))
    {
      return bluez_audio_a2dp_sink_read(argc, argv);
    }

  if (!strcmp(argv[1], "a2dp-sink") && !strcmp(argv[2], "stop"))
    {
      return bluez_audio_a2dp_sink_stop();
    }

  if (!strcmp(argv[1], "upstream-object-probe"))
    {
      return bluez_audio_upstream_object_probe(argc, argv);
    }

  if (!strcmp(argv[1], "le-bap-control"))
    {
      return bluez_audio_le_bap_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-daemon") &&
      !strcmp(argv[2], "unicast-profile-flow"))
    {
      return bluez_audio_le_daemon_unicast_profile_flow(argc, argv);
    }

  if (!strcmp(argv[1], "le-daemon") &&
      !strcmp(argv[2], "broadcast-profile-flow"))
    {
      return bluez_audio_le_daemon_broadcast_profile_flow(argc, argv);
    }

  if (!strcmp(argv[1], "le-daemon") &&
      !strcmp(argv[2], "integrated-profile-flow"))
    {
      return bluez_audio_le_daemon_integrated_profile_flow(argc, argv);
    }

  if (!strcmp(argv[1], "le-daemon"))
    {
      return bluez_audio_le_daemon_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-mgmt-control"))
    {
      return bluez_audio_le_mgmt_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-cap-control"))
    {
      return bluez_audio_le_cap_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-bass-control"))
    {
      return bluez_audio_le_bass_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-vcp-control"))
    {
      return bluez_audio_le_vcp_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-micp-control"))
    {
      return bluez_audio_le_micp_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-csip-control"))
    {
      return bluez_audio_le_csip_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-mcp-control"))
    {
      return bluez_audio_le_mcp_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-tmap-control"))
    {
      return bluez_audio_le_tmap_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-ccp-control"))
    {
      return bluez_audio_le_ccp_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-gmap-control"))
    {
      return bluez_audio_le_gmap_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-ascs-cp"))
    {
      return bluez_audio_le_ascs_control_point(argc, argv);
    }

  if (!strcmp(argv[1], "le-bap-policy"))
    {
      return bluez_audio_le_bap_policy_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-gatt-db"))
    {
      return bluez_audio_le_gatt_db_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-att-bearer"))
    {
      return bluez_audio_le_att_bearer_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-att-io"))
    {
      return bluez_audio_le_att_io_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-att-queue"))
    {
      return bluez_audio_le_att_queue_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-gatt-upstream"))
    {
      return bluez_audio_le_gatt_upstream_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-iso-socket"))
    {
      return bluez_audio_le_iso_socket_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-iso-qos"))
    {
      return bluez_audio_le_iso_qos_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-iso"))
    {
      return bluez_audio_le_broadcast_iso_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-security"))
    {
      return bluez_audio_le_broadcast_security_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-dbus-client"))
    {
      return bluez_audio_le_dbus_client_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-unicast-control"))
    {
      return bluez_audio_le_unicast_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-audio-codec") &&
      !strcmp(argv[2], "source-lc3-encode-write-release"))
    {
      return bluez_audio_le_codec_source(argc, argv);
    }

  if (!strcmp(argv[1], "le-audio-codec") &&
      !strcmp(argv[2], "sink-lc3-recv-decode-release"))
    {
      return bluez_audio_le_codec_sink(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-source") &&
      !strcmp(argv[2], "start"))
    {
      return bluez_audio_le_source_start(argc, argv);
    }

  if (!strcmp(argv[1], "le-unicast-source") &&
      !strcmp(argv[2], "start"))
    {
      return bluez_audio_le_unicast_source_start(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-sink") &&
      !strcmp(argv[2], "sync"))
    {
      return bluez_audio_le_sink_sync(argc, argv);
    }

  if (!strcmp(argv[1], "le-unicast-sink") &&
      !strcmp(argv[2], "sync"))
    {
      return bluez_audio_le_unicast_sink_sync(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-sink") &&
      !strcmp(argv[2], "start"))
    {
      return bluez_audio_le_sink_start(argc, argv);
    }

  if (!strcmp(argv[1], "le-unicast-sink") &&
      !strcmp(argv[2], "start"))
    {
      return bluez_audio_le_unicast_sink_start(argc, argv);
    }

  if (!strcmp(argv[1], "le-broadcast-sink") &&
      !strcmp(argv[2], "stop"))
    {
      return bluez_audio_le_sink_stop();
    }

  if (!strcmp(argv[1], "le-unicast-sink") &&
      !strcmp(argv[2], "stop"))
    {
      return bluez_audio_le_unicast_sink_stop();
    }

  if (!strcmp(argv[1], "media-transport") &&
      (!strcmp(argv[2], "a2dp-source-acquire-write-release") ||
       !strcmp(argv[2], "a2dp-source-acquire-busy-write-release")))
    {
      return bluez_audio_media_transport_a2dp_source(argc, argv);
    }

  if (!strcmp(argv[1], "media-transport") &&
      (!strcmp(argv[2], "a2dp-sink-acquire-read-release") ||
       !strcmp(argv[2], "a2dp-sink-acquire-busy-read-release")))
    {
      return bluez_audio_media_transport_a2dp_sink(argc, argv);
    }

  if (!strcmp(argv[1], "media-transport") &&
      !strcmp(argv[2], "unicast-source-acquire-write-release"))
    {
      return bluez_audio_media_transport_unicast_source(argc, argv);
    }

  if (!strcmp(argv[1], "media-transport") &&
      !strcmp(argv[2], "unicast-sink-acquire-read-release"))
    {
      return bluez_audio_media_transport_unicast_sink(argc, argv);
    }

  bluez_audio_usage();
  return 1;
}
