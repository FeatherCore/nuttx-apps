/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/audio_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/wireless/linux_bluetooth.h>

#include "../codecs/sbc_backend.h"

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
  printf("  le-bap-control source-announce|source-start|source-stop [big] [bis]\n");
  printf("  le-bap-control sink-discover|sink-config|sink-sync [big] [bis]\n");
  printf("  le-unicast-control source-config|source-enable|source-disable|source-qos-update|source-qos-reject|source-qos-cancel|source-release [cig] [cis]\n");
  printf("  le-unicast-control sink-discover|sink-config|sink-enable|sink-disable|sink-qos-update|sink-qos-reject|sink-qos-cancel|sink-release [cig] [cis]\n");
  printf("  le-broadcast-source start [big] [bis]\n");
  printf("  le-broadcast-sink sync|start|stop [big] [bis] [max]\n");
  printf("  le-unicast-source start [cig] [cis]\n");
  printf("  le-unicast-sink sync|start|stop [cig] [cis] [max]\n");
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

  if (failed)
    {
      printf("bluez-audio: avrcp control failed\n");
      return 1;
    }

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

      ret = linux_bt_upstream_l2cap_socket_write_probe(payload,
                                                       payload_len,
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
      ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
      printf("%s", out);
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

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  printf("%s", out);
  out[0] = '\0';
  ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0x0040,
                                                   handle, out,
                                                   sizeof(out));
  printf("%s", out);
  out[0] = '\0';
  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0040,
                                                  handle, out,
                                                  sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0,
                                                       handle, out,
                                                       sizeof(out));
      printf("%s", out);
      out[0] = '\0';
      ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0040,
                                                      handle, out,
                                                      sizeof(out));
    }

  printf("%s", out);
  if (ret < 0)
    {
      printf("bluez-audio: a2dp-signaling source-session-open bind "
             "failed ret=%d\n", ret);
      return 1;
    }

  ret = linux_bt_upstream_l2cap_socket_connect_probe(0x0019, 0x0040,
                                                     out, sizeof(out));
  printf("%s", out);
  if (ret < 0)
    {
      printf("bluez-audio: a2dp-signaling source-session-open connect "
             "failed ret=%d\n", ret);
      (void)bluez_audio_a2dp_signal_source_session_set(0, 0, 0);
      return 1;
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

  for (attempt = 0; attempt < 400; attempt++)
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

  for (attempt = 0; attempt < 400; attempt++)
    {
      int poll_ret = linux_bt_upstream_vhci_poll_medium();

      if (poll_ret < 0)
        {
          printf("bluez-audio: a2dp transaction response poll failed "
                 "command=%s ret=%d\n", command, poll_ret);
          return 1;
        }

      ret = linux_bt_upstream_l2cap_socket_recv_raw(rsp, sizeof(rsp),
                                                    &rsp_len, out,
                                                    sizeof(out));
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

static int bluez_audio_a2dp_signal_source(int argc, char *argv[])
{
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;

  if (!strcmp(argv[2], "source-transaction"))
    {
      return bluez_audio_a2dp_signal_source_transaction(peer);
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
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 2;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=source "
         "command=a2dp-source-acquire-write-release peer=%u "
         "handle=0x%04x\n", peer, handle);

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  printf("%s", out);
  ret = linux_bt_upstream_l2cap_socket_clear_probe(0x0019, 0, handle,
                                                   out, sizeof(out));
  printf("%s", out);

  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0041,
                                                  handle, out,
                                                  sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_probe(0x0019, 0x0041,
                                                         out, sizeof(out));
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      printf("bluez-audio: media transport acquire fd=l2cap "
             "transport=/org/bluez/hci0/dev_feather/fd0 "
             "read-mtu=672 write-mtu=672\n");
      ret = linux_bt_upstream_l2cap_socket_write_probe(media,
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

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  printf("%s", out);
  failed |= ret < 0;

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
  uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 1;
  uint16_t handle = bluez_audio_bredr_handle(peer);
  size_t max_len = argc > 4 ? (size_t)strtoul(argv[4], NULL, 0) : 512;
  void *media_handle = NULL;
  int polled = 0;
  int attempt;
  int failed = 0;
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/transport.c "
         "style profile=media-transport role=sink "
         "command=a2dp-sink-acquire-read-release peer=%u "
         "handle=0x%04x max=%lu\n", peer, handle, (unsigned long)max_len);

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
  int ret;

  printf("bluez-audio: source=third/bluez/profiles/audio/bap.c "
         "style profile=le-broadcast-source command=start big=%u bis=%u "
         "handle=0x%04x\n", big, bis, handle);

  ret = linux_bt_upstream_iso_socket_send_probe(0, handle, media,
                                                sizeof(media) - 1,
                                                out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("bluez-audio: le-broadcast-source failed ret=%d\n", ret);
      return 1;
    }

  printf("%s", out);
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

  for (attempt = 0; attempt < 20; attempt++)
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
      for (attempt = 0; attempt < 40; attempt++)
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

  if (!strcmp(argv[1], "le-bap-control"))
    {
      return bluez_audio_le_bap_control(argc, argv);
    }

  if (!strcmp(argv[1], "le-unicast-control"))
    {
      return bluez_audio_le_unicast_control(argc, argv);
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
      !strcmp(argv[2], "a2dp-source-acquire-write-release"))
    {
      return bluez_audio_media_transport_a2dp_source(argc, argv);
    }

  if (!strcmp(argv[1], "media-transport") &&
      !strcmp(argv[2], "a2dp-sink-acquire-read-release"))
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
