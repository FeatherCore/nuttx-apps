/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_a2dp_compat.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_A2DP_COMPAT_H
#define APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_A2DP_COMPAT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum bluez_upstream_a2dp_closeout_state
{
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_IDLE = 0,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_ENDPOINT_REGISTERED,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SIGNALING_OPEN,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CONFIGURED,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_OPEN,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_STREAMING,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SUSPENDED,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSING,
  BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSED
};

struct bluez_upstream_a2dp_closeout_session
{
  const char *role;
  uint16_t peer;
  uint16_t handle;
  uint16_t signal_psm;
  uint16_t signal_cid;
  uint16_t media_psm;
  uint16_t media_cid;
  const char *codec;
  enum bluez_upstream_a2dp_closeout_state state;
  bool profile_owner;
  bool endpoint_owner;
  bool avdtp_owner;
  bool transport_owner;
  bool media_fd_owner;
  bool codec_owner;
  bool pending_request_owner;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void bluez_upstream_a2dp_compat_print(const char *role);
void bluez_upstream_a2dp_closeout_session_init(
       struct bluez_upstream_a2dp_closeout_session *session,
       const char *role, uint16_t peer, uint16_t handle,
       uint16_t signal_psm, uint16_t signal_cid,
       uint16_t media_psm, uint16_t media_cid);
void bluez_upstream_a2dp_closeout_session_graph(
       const struct bluez_upstream_a2dp_closeout_session *session,
       const char *action);
void bluez_upstream_a2dp_closeout_session_set_state(
       struct bluez_upstream_a2dp_closeout_session *session,
       enum bluez_upstream_a2dp_closeout_state state,
       const char *action);
void bluez_upstream_a2dp_setup_stream_owner_print(const char *role);
void bluez_upstream_a2dp_avdtp_transaction_owner_print(const char *role);
void bluez_upstream_a2dp_media_transport_dbus_owner_print(
       const char *role);
void bluez_upstream_a2dp_profile_mainloop_dbus_owner_print(
       const char *role);
void bluez_upstream_a2dp_adapter_command_owner_print(const char *role);
void bluez_upstream_a2dp_source_parity_owner_print(const char *role);
void bluez_upstream_a2dp_daemon_ownership_owner_print(const char *role);
void bluez_upstream_a2dp_coverage_map_owner_print(const char *role);
void bluez_upstream_a2dp_tool_closeout_owner_print(const char *role);
void bluez_upstream_a2dp_tool_coverage_owner_print(const char *role);
void bluez_upstream_a2dp_tool_ownership_owner_print(const char *role);
void bluez_upstream_a2dp_tool_e2e_contract_owner_print(const char *role);

#endif
