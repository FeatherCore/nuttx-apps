/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/a2dp_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/wireless/linux_bluetooth.h>

#include "../upstream_a2dp_compat.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_A2DP_SIGNAL_PSM      0x0019
#define BLUEZ_A2DP_SIGNAL_CID      0x0040
#define BLUEZ_A2DP_MEDIA_PSM       0x0019
#define BLUEZ_A2DP_MEDIA_CID       0x0041
#define BLUEZ_A2DP_DEFAULT_PEER    2

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_a2dp_transaction
{
  const char *label;
  const uint8_t *payload;
  size_t payload_len;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_avdtp_discover[] =
{
  0x00, 0x01
};

static const uint8_t g_avdtp_get_all_capabilities[] =
{
  0x10, 0x0c, 0x01
};

static const uint8_t g_avdtp_set_configuration[] =
{
  0x20, 0x03, 0x01, 0x02, 0x07, 0x06, 0x00, 0x00,
  0xff, 0xff, 0x02, 0x35
};

static const uint8_t g_avdtp_get_configuration[] =
{
  0x30, 0x04, 0x01
};

static const uint8_t g_avdtp_open[] =
{
  0x40, 0x06, 0x01
};

static const uint8_t g_avdtp_reconfigure[] =
{
  0x50, 0x05, 0x01, 0x07, 0x06, 0x00, 0x00, 0xff,
  0xff, 0x02, 0x35
};

static const uint8_t g_avdtp_delay_report[] =
{
  0x60, 0x0d, 0x01, 0x00, 0x20
};

static const uint8_t g_avdtp_security_control[] =
{
  0x70, 0x0b, 0x01, 0x01
};

static const uint8_t g_avdtp_start[] =
{
  0x80, 0x07, 0x01
};

static const uint8_t g_avdtp_suspend[] =
{
  0x90, 0x09, 0x01
};

static const uint8_t g_avdtp_close[] =
{
  0xa0, 0x08, 0x01
};

static const uint8_t g_avdtp_abort[] =
{
  0xb0, 0x0a, 0x01
};

static const uint8_t g_a2dp_rtp_sbc[] =
{
  0x80, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
  0x12, 0x34, 0x56, 0x78, 0x9c, 0xbd, 0x35, 0x00,
  0x11, 0x22, 0x33, 0x44, 0x55, 0x66
};

static const struct bluez_a2dp_transaction g_a2dp_transactions[] =
{
  {"avdtp-discover", g_avdtp_discover, sizeof(g_avdtp_discover)},
  {"avdtp-get-all-capabilities", g_avdtp_get_all_capabilities,
   sizeof(g_avdtp_get_all_capabilities)},
  {"avdtp-set-configuration", g_avdtp_set_configuration,
   sizeof(g_avdtp_set_configuration)},
  {"avdtp-get-configuration", g_avdtp_get_configuration,
   sizeof(g_avdtp_get_configuration)},
  {"avdtp-open", g_avdtp_open, sizeof(g_avdtp_open)},
  {"avdtp-reconfigure", g_avdtp_reconfigure, sizeof(g_avdtp_reconfigure)},
  {"avdtp-delay-report", g_avdtp_delay_report, sizeof(g_avdtp_delay_report)},
  {"avdtp-security-control", g_avdtp_security_control,
   sizeof(g_avdtp_security_control)},
  {"avdtp-start", g_avdtp_start, sizeof(g_avdtp_start)},
  {"avdtp-suspend", g_avdtp_suspend, sizeof(g_avdtp_suspend)},
  {"avdtp-close", g_avdtp_close, sizeof(g_avdtp_close)},
  {"avdtp-abort", g_avdtp_abort, sizeof(g_avdtp_abort)}
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_a2dp_usage(void)
{
  printf("usage: blueza2dp closeout source|sink [peer]\n");
}

static uint16_t bluez_a2dp_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static int bluez_a2dp_write(void *sock,
                            const struct bluez_a2dp_transaction *tx,
                            const char *role)
{
  char out[256];
  int ret;

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_l2cap_socket_write_handle(
          sock, tx->payload, tx->payload_len, out, sizeof(out));
  printf("%s", out);

  return ret;
}

static int bluez_a2dp_run_signaling(void *signal, const char *role)
{
  size_t i;
  int failed = 0;

  for (i = 0; i < sizeof(g_a2dp_transactions) /
                  sizeof(g_a2dp_transactions[0]); i++)
    {
      failed |= bluez_a2dp_write(signal, &g_a2dp_transactions[i],
                                 role) < 0;
    }

  return failed ? -1 : 0;
}

static int bluez_a2dp_closeout(const char *role, uint16_t peer)
{
  struct bluez_upstream_a2dp_closeout_session session;
  uint16_t handle;
  void *signal = NULL;
  void *media = NULL;
  char out[256];
  int ret;
  int failed = 0;

  if (strcmp(role, "source") != 0 && strcmp(role, "sink") != 0)
    {
      bluez_a2dp_usage();
      return 1;
    }

  handle = bluez_a2dp_handle(peer);
  bluez_upstream_a2dp_closeout_session_init(
    &session, role, peer, handle, BLUEZ_A2DP_SIGNAL_PSM,
    BLUEZ_A2DP_SIGNAL_CID, BLUEZ_A2DP_MEDIA_PSM, BLUEZ_A2DP_MEDIA_CID);
  bluez_upstream_a2dp_closeout_session_graph(&session, "closeout-init");
  session.profile_owner = true;
  session.endpoint_owner = true;
  session.codec_owner = true;
  bluez_upstream_a2dp_closeout_session_set_state(
    &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_ENDPOINT_REGISTERED,
    "endpoint-registered");
  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_A2DP_SIGNAL_PSM,
                                            BLUEZ_A2DP_SIGNAL_CID,
                                            handle, &signal);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              signal, BLUEZ_A2DP_SIGNAL_PSM, BLUEZ_A2DP_SIGNAL_CID);
      failed |= ret < 0;
      if (ret >= 0)
        {
          session.avdtp_owner = true;
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SIGNALING_OPEN,
            "signaling-open");
        }
    }

  if (!failed)
    {
      failed |= bluez_a2dp_run_signaling(signal, role) < 0;
      if (!failed)
        {
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CONFIGURED,
            "avdtp-configured");
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_OPEN, "avdtp-open");
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_STREAMING,
            "avdtp-streaming");
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SUSPENDED,
            "avdtp-suspended");
          bluez_upstream_a2dp_closeout_session_set_state(
            &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSING,
            "avdtp-closing");
        }
    }

  session.transport_owner = true;
  bluez_upstream_a2dp_closeout_session_graph(&session,
                                             "transport-exported");
  session.pending_request_owner = true;
  bluez_upstream_a2dp_closeout_session_graph(&session,
                                             "transport-acquire-request");

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_A2DP_MEDIA_PSM,
                                            BLUEZ_A2DP_MEDIA_CID,
                                            handle, &media);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              media, BLUEZ_A2DP_MEDIA_PSM, BLUEZ_A2DP_MEDIA_CID);
      failed |= ret < 0;
    }

  if (!failed)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              media, g_a2dp_rtp_sbc, sizeof(g_a2dp_rtp_sbc),
              out, sizeof(out));
      printf("%s", out);
      session.pending_request_owner = false;
      session.media_fd_owner = true;
      bluez_upstream_a2dp_closeout_session_set_state(
        &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_STREAMING,
        "transport-acquired");
      failed |= ret < 0;
    }

  session.media_fd_owner = false;
  session.pending_request_owner = false;
  bluez_upstream_a2dp_closeout_session_graph(&session,
                                             "transport-released");

  if (media != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(media);
      failed |= ret < 0;
    }

  if (signal != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(signal);
      failed |= ret < 0;
    }

  session.profile_owner = false;
  session.endpoint_owner = false;
  session.avdtp_owner = false;
  session.transport_owner = false;
  session.codec_owner = false;
  session.pending_request_owner = false;
  session.media_fd_owner = false;
  bluez_upstream_a2dp_closeout_session_set_state(
    &session, BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSED, "closeout-cleanup");

  return failed ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint16_t peer;

  if (argc < 3 || strcmp(argv[1], "closeout") != 0)
    {
      bluez_a2dp_usage();
      return 1;
    }

  peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : BLUEZ_A2DP_DEFAULT_PEER;
  return bluez_a2dp_closeout(argv[2], peer);
}
