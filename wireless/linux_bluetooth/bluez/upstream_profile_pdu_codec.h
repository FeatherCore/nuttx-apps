/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_profile_pdu_codec.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_PROFILE_PDU_CODEC_H
#define __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_PROFILE_PDU_CODEC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_PROFILE_PDU_MAX      160
#define BLUEZ_PROFILE_PDU_HEX_MAX  (BLUEZ_PROFILE_PDU_MAX * 3 + 1)

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct bluez_profile_pdu
{
  uint8_t payload[BLUEZ_PROFILE_PDU_MAX];
  size_t payload_len;
  const char *semantic;
  const char *opcode;
  const char *wire;
  const char *codec_source;
  const char *upstream_owner;
  const char *encoder;
  const char *decoder;
  const char *decoded_opcode;
  const char *parser_owner;
  const char *handler_owner;
  const char *policy_owner;
  const char *error_owner;
  const char *cleanup_owner;
  const char *dispatch_result;
  const char *error_status;
  const char *cleanup_state;
  const char *parse_detail;
  const char *parse_backend;
  const char *handler_backend;
  const char *att_error_rsp;
  const char *att_request_state;
  const char *att_security_state;
  const char *att_completion_cb;
  const char *att_queue_backend;
  const char *att_request_owner;
  const char *att_request_lifecycle;
  const char *att_timeout_state;
  const char *att_cancel_state;
  uint16_t parsed_handle;
  uint16_t parsed_value_len;
  uint16_t att_mtu;
  uint8_t parsed_opcode;
  uint8_t att_error_code;
  uint8_t att_requires_security;
  uint8_t att_request_id;
  uint8_t att_pending_before;
  uint8_t att_pending_after;
  uint8_t att_request_ref_before;
  uint8_t att_request_ref_after;
  uint8_t att_timer_before;
  uint8_t att_timer_after;
  int roundtrip_ok;
  int parse_ok;
  int handler_ok;
  int policy_ok;
  int error_map_ok;
  int cleanup_ok;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int bluez_profile_pdu_encode(const char *family, const char *label,
                             const char *transport,
                             struct bluez_profile_pdu *pdu);
int bluez_profile_pdu_encode_error_probe(const char *family,
                                         const char *transport,
                                         struct bluez_profile_pdu *pdu);
int bluez_profile_pdu_encode_cancel_probe(const char *family,
                                          const char *transport,
                                          struct bluez_profile_pdu *pdu);
void bluez_profile_pdu_hex(const struct bluez_profile_pdu *pdu,
                           char *out, size_t out_len);

#endif /* __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_PROFILE_PDU_CODEC_H */
