/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_profile_pdu_codec.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "upstream_profile_pdu_codec.h"

#include <stdio.h>
#include <string.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

struct bluez_profile_att_parse_result
{
  const char *detail;
  const char *backend;
  const char *handler_backend;
  uint16_t handle;
  uint16_t value_len;
  uint8_t opcode;
};

struct bluez_profile_att_queue
{
  uint8_t next_id;
  uint8_t pending;
};

struct bluez_profile_att_request
{
  uint8_t id;
  uint8_t refcnt;
  uint8_t opcode;
  uint16_t handle;
  const char *owner;
  const char *lifecycle;
};

static size_t bluez_profile_pdu_copy(struct bluez_profile_pdu *pdu,
                                     const void *src, size_t src_len)
{
  if (src_len > sizeof(pdu->payload))
    {
      src_len = sizeof(pdu->payload);
    }

  memcpy(pdu->payload, src, src_len);
  pdu->payload_len = src_len;
  return src_len;
}

static const char *bluez_profile_pdu_semantic(const char *family,
                                              const char *label)
{
  if (!strcmp(family, "hid"))
    {
      return strstr(label, "control") != NULL ?
             "hid-control-op=set-protocol-report" :
             "hid-interrupt-op=input-output-report";
    }

  if (!strcmp(family, "hogp"))
    {
      if (strstr(label, "report-map") != NULL)
        {
          return "att-op=read-req handle=report-map";
        }

      if (strstr(label, "protocol") != NULL)
        {
          return "att-op=write-req handle=protocol-mode value=report";
        }

      return "att-op=handle-value-notify handle=input-report ccc=enabled";
    }

  if (!strcmp(family, "hfp"))
    {
      if (strstr(label, "brsf") != NULL)
        {
          return "rfcomm-at=AT+BRSF features=codec-negotiation";
        }

      if (strstr(label, "codec") != NULL)
        {
          return "rfcomm-at=AT+BAC/AT+BCS codec=msbc";
        }

      return "rfcomm-at=AT+CLCC call-list";
    }

  if (!strcmp(family, "hsp"))
    {
      return strstr(label, "volume") != NULL ?
             "rfcomm-at=AT+VGS/AT+VGM volume" :
             "rfcomm-at=AT+CKPD button=hook";
    }

  if (!strcmp(family, "gatt"))
    {
      if (strstr(label, "discover") != NULL ||
          strstr(label, "register") != NULL)
        {
          return "att-op=read-by-group-type service-discovery";
        }

      if (strstr(label, "read") != NULL)
        {
          return "att-op=read-req characteristic";
        }

      return "att-op=write-req+handle-value-notify ccc";
    }

  if (!strcmp(family, "mesh"))
    {
      if (strstr(label, "provision") != NULL)
        {
          return "mesh-pb-adv=invite-capabilities-start";
        }

      if (strstr(label, "proxy") != NULL)
        {
          return "mesh-proxy=config-client-server";
        }

      return "mesh-network-pdu=access-message ttl=5 seq-auth";
    }

  if (!strcmp(family, "asha"))
    {
      if (strstr(label, "discovery") != NULL)
        {
          return "att-op=read-by-type asha-service";
        }

      if (strstr(label, "properties") != NULL)
        {
          return "att-op=read-req asha-properties";
        }

      return "att-op=notify audio-status codec=g722";
    }

  if (!strcmp(family, "midi"))
    {
      if (strstr(label, "discovery") != NULL)
        {
          return "att-op=read-by-group-type midi-service";
        }

      if (strstr(label, "ccc") != NULL)
        {
          return "att-op=write-req ccc-enable";
        }

      return "midi-packet=timestamped-note-on-off";
    }

  if (!strcmp(family, "ranging"))
    {
      if (strstr(label, "service") != NULL)
        {
          return "att-op=read-by-type rap-capability";
        }

      if (strstr(label, "capability") != NULL)
        {
          return "hci-le-cs=capability-exchange";
        }

      return "hci-le-cs=result-report distance-quality";
    }

  if (!strcmp(family, "print"))
    {
      if (strstr(label, "sdp") != NULL)
        {
          return "sdp-op=service-search-attribute hcrp-spp";
        }

      if (strstr(label, "create") != NULL ||
          strstr(label, "receive") != NULL)
        {
          return "rfcomm-hcrp=job-data-channel";
        }

      return "rfcomm-hcrp=job-status";
    }

  if (!strcmp(family, "iap"))
    {
      if (strstr(label, "sdp") != NULL)
        {
          return "sdp-op=service-search-attribute spp-iap";
        }

      if (strstr(label, "identify") != NULL)
        {
          return "iap2-control=identify-device";
        }

      return "iap2-ea=external-accessory-session";
    }

  return "profile-pdu=generic";
}

static void bluez_profile_pdu_defaults(const char *family,
                                       const char *transport,
                                       struct bluez_profile_pdu *pdu)
{
  pdu->payload_len = 0;
  pdu->semantic = "profile-pdu=generic";
  pdu->opcode = "GENERIC_PROFILE_PDU";
  pdu->wire = transport;
  pdu->codec_source =
    "apps/wireless/linux_bluetooth/bluez/upstream_profile_pdu_codec.c";
  pdu->upstream_owner = "third/bluez/src/profile.c";
  pdu->encoder = "bluez-profile-pdu-encoder";
  pdu->decoder = "bluez-profile-pdu-decoder";
  pdu->decoded_opcode = "GENERIC_PROFILE_PDU";
  pdu->parser_owner = "src/profile.c:profile_parser";
  pdu->handler_owner = "src/service.c:profile_handler";
  pdu->policy_owner = "src/profile.c:profile_policy";
  pdu->error_owner = "src/service.c:service_error";
  pdu->cleanup_owner = "src/profile.c:profile_release";
  pdu->dispatch_result = "not-dispatched";
  pdu->error_status = "not-mapped";
  pdu->cleanup_state = "not-clean";
  pdu->parse_detail = "profile-pdu-parse-unset";
  pdu->parse_backend = "profile-pdu-parser-unset";
  pdu->handler_backend = "profile-handler-backend-unset";
  pdu->att_error_rsp = "none";
  pdu->att_request_state = "not-enqueued";
  pdu->att_security_state = "not-required";
  pdu->att_completion_cb = "none";
  pdu->att_queue_backend = "none";
  pdu->att_request_owner = "none";
  pdu->att_request_lifecycle = "none";
  pdu->att_timeout_state = "none";
  pdu->att_cancel_state = "none";
  pdu->parsed_handle = 0;
  pdu->parsed_value_len = 0;
  pdu->att_mtu = 23;
  pdu->parsed_opcode = 0;
  pdu->att_error_code = 0;
  pdu->att_requires_security = 0;
  pdu->att_request_id = 0;
  pdu->att_pending_before = 0;
  pdu->att_pending_after = 0;
  pdu->att_request_ref_before = 0;
  pdu->att_request_ref_after = 0;
  pdu->att_timer_before = 0;
  pdu->att_timer_after = 0;
  pdu->roundtrip_ok = 0;
  pdu->parse_ok = 0;
  pdu->handler_ok = 0;
  pdu->policy_ok = 0;
  pdu->error_map_ok = 0;
  pdu->cleanup_ok = 0;

  if (!strcmp(family, "hid"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/input/device.c+"
        "third/linux-hwe-6.17-6.17.0/net/bluetooth/hidp/core.c";
      pdu->parser_owner = "profiles/input/device.c:hidp_parse_report";
      pdu->handler_owner =
        "profiles/input/device.c:input_device_set_channel";
      pdu->policy_owner = "profiles/input/device.c:hidp_connadd_policy";
      pdu->error_owner = "profiles/input/device.c:control_interrupt_error";
      pdu->cleanup_owner =
        "profiles/input/device.c:input_device_free+hidp_conn_del";
    }
  else if (!strcmp(family, "hogp"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/input/hog.c+"
        "third/bluez/src/shared/att.c";
      pdu->parser_owner = "src/shared/att.c:bt_att_parse";
      pdu->handler_owner = "profiles/input/hog-lib.c:report_handler";
      pdu->policy_owner = "profiles/input/hog-lib.c:protocol_mode_policy";
      pdu->error_owner = "profiles/input/hog-lib.c:att_error";
      pdu->cleanup_owner = "profiles/input/hog-lib.c:hog_detach";
    }
  else if (!strcmp(family, "hfp") || !strcmp(family, "hsp"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/audio/telephony.c+"
        "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c";
      pdu->parser_owner = "profiles/audio/telephony.c:at_parse";
      pdu->handler_owner = "profiles/audio/telephony.c:at_command_handler";
      pdu->policy_owner = "profiles/audio/telephony.c:codec_call_policy";
      pdu->error_owner = "profiles/audio/telephony.c:rfcomm_sco_error";
      pdu->cleanup_owner =
        "profiles/audio/telephony.c:session_release+rfcomm_dlc_close";
    }
  else if (!strcmp(family, "gatt"))
    {
      pdu->upstream_owner =
        "third/bluez/src/shared/att.c+"
        "third/bluez/src/shared/gatt-client.c";
      pdu->parser_owner = "src/shared/att.c:bt_att_parse";
      pdu->handler_owner = "src/shared/gatt-client.c:gatt_client_handler";
      pdu->policy_owner =
        "src/gatt-database.c:authorization_security_policy";
      pdu->error_owner = "src/shared/att.c:att_error_rsp";
      pdu->cleanup_owner =
        "src/shared/gatt-client.c:gatt_client_unref+bt_att_unref";
    }
  else if (!strcmp(family, "mesh"))
    {
      pdu->upstream_owner =
        "third/bluez/mesh/net.c+third/bluez/mesh/pb-adv.c";
      pdu->parser_owner = "mesh/net.c:mesh_net_decode";
      pdu->handler_owner = "mesh/model.c:model_recv";
      pdu->policy_owner = "mesh/net.c:replay_key_policy";
      pdu->error_owner = "mesh/net.c:mesh_reject";
      pdu->cleanup_owner = "mesh/node.c:node_cleanup+mesh_net_free";
    }
  else if (!strcmp(family, "asha"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/audio/asha.c+"
        "third/bluez/src/shared/att.c";
      pdu->parser_owner = "src/shared/att.c:bt_att_parse";
      pdu->handler_owner = "profiles/audio/asha.c:asha_status_handler";
      pdu->policy_owner = "profiles/audio/asha.c:codec_stream_policy";
      pdu->error_owner = "profiles/audio/asha.c:asha_stream_error";
      pdu->cleanup_owner = "profiles/audio/asha.c:asha_device_free";
    }
  else if (!strcmp(family, "midi"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/midi/midi.c+"
        "third/bluez/profiles/midi/libmidi.c";
      pdu->parser_owner = "profiles/midi/libmidi.c:midi_parse";
      pdu->handler_owner = "profiles/midi/midi.c:midi_io_handler";
      pdu->policy_owner = "profiles/midi/libmidi.c:timestamp_policy";
      pdu->error_owner = "profiles/midi/midi.c:midi_io_error";
      pdu->cleanup_owner = "profiles/midi/midi.c:midi_device_free";
    }
  else if (!strcmp(family, "ranging"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/ranging/rap.c+"
        "third/bluez/profiles/ranging/rap_hci.c";
      pdu->parser_owner = "profiles/ranging/rap.c:rap_parse";
      pdu->handler_owner = "profiles/ranging/rap_hci.c:cs_event_handler";
      pdu->policy_owner = "profiles/ranging/rap.c:security_quality_policy";
      pdu->error_owner = "profiles/ranging/rap.c:procedure_error";
      pdu->cleanup_owner = "profiles/ranging/rap.c:procedure_cleanup";
    }
  else if (!strcmp(family, "print"))
    {
      pdu->upstream_owner =
        "third/bluez/profiles/cups/hcrp.c+"
        "third/bluez/profiles/cups/spp.c";
      pdu->parser_owner = "profiles/cups/hcrp.c:hcrp_parse";
      pdu->handler_owner = "profiles/cups/hcrp.c:job_handler";
      pdu->policy_owner = "profiles/cups/main.c:printer_policy";
      pdu->error_owner = "profiles/cups/hcrp.c:job_error";
      pdu->cleanup_owner = "profiles/cups/main.c:printer_release";
    }
  else if (!strcmp(family, "iap"))
    {
      pdu->upstream_owner = "third/bluez/profiles/iap/main.c";
      pdu->parser_owner = "profiles/iap/main.c:iap2_parse";
      pdu->handler_owner = "profiles/iap/main.c:ea_session_handler";
      pdu->policy_owner = "profiles/iap/main.c:accessory_policy";
      pdu->error_owner = "profiles/iap/main.c:link_error";
      pdu->cleanup_owner = "profiles/iap/main.c:iap_session_release";
    }
}

static int bluez_profile_pdu_text_is(const struct bluez_profile_pdu *pdu,
                                     const char *prefix)
{
  size_t len = strlen(prefix);

  return pdu->payload_len >= len &&
         memcmp(pdu->payload, prefix, len) == 0;
}

static void bluez_profile_att_queue_init(
  struct bluez_profile_att_queue *queue)
{
  queue->next_id = 1;
  queue->pending = 0;
}

static void bluez_profile_att_queue_enqueue(
  struct bluez_profile_att_queue *queue, struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_request req;

  pdu->att_queue_backend = "bluez-shared-att-queue:single-flight";
  req.id =
    (uint8_t)(queue->next_id++ ^ pdu->parsed_opcode ^
              (pdu->parsed_handle & 0xff));
  if (req.id == 0 || req.id == 0xff)
    {
      req.id = queue->next_id++;
    }

  req.refcnt = 1;
  req.opcode = pdu->parsed_opcode;
  req.handle = pdu->parsed_handle;
  req.owner = "bluez-shared-att-request";
  req.lifecycle = "alloc-enqueue-dispatch-complete-unref";

  pdu->att_request_id = req.id;
  pdu->att_request_owner = req.owner;
  pdu->att_request_lifecycle = req.lifecycle;
  pdu->att_request_ref_before = req.refcnt;
  pdu->att_timeout_state = "timer-armed";
  pdu->att_cancel_state = "not-cancelled";
  pdu->att_timer_before = 1;
  pdu->att_pending_before = (uint8_t)(queue->pending + 1);
  queue->pending++;
}

static void bluez_profile_att_queue_complete(
  struct bluez_profile_att_queue *queue, struct bluez_profile_pdu *pdu)
{
  if (queue->pending > 0)
    {
      queue->pending--;
    }

  pdu->att_pending_after = queue->pending;
  pdu->att_completion_cb = "bluez-shared-att-callback:complete";
  pdu->att_request_ref_after = 0;
  pdu->att_timeout_state = "timer-armed-cleared";
  pdu->att_timer_after = 0;
}

static void bluez_profile_att_queue_error(
  struct bluez_profile_att_queue *queue, struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_request req;

  req.id = 0xff;
  req.refcnt = 1;
  req.opcode = pdu->parsed_opcode;
  req.handle = 0;
  req.owner = "bluez-shared-att-request";
  req.lifecycle = "alloc-enqueue-reject-error-unref";

  pdu->att_queue_backend = "bluez-shared-att-queue:error-single-flight";
  pdu->att_request_id = req.id;
  pdu->att_request_owner = req.owner;
  pdu->att_request_lifecycle = req.lifecycle;
  pdu->att_request_ref_before = req.refcnt;
  pdu->att_timeout_state = "timer-armed";
  pdu->att_cancel_state = "not-cancelled";
  pdu->att_timer_before = 1;
  pdu->att_pending_before = (uint8_t)(queue->pending + 1);
  queue->pending++;
  if (queue->pending > 0)
    {
      queue->pending--;
    }

  pdu->att_pending_after = queue->pending;
  pdu->att_completion_cb = "bluez-shared-att-callback:error";
  pdu->att_request_ref_after = 0;
  pdu->att_timeout_state = "timer-armed-cleared-on-error";
  pdu->att_timer_after = 0;
}

static void bluez_profile_att_queue_cancel(
  struct bluez_profile_att_queue *queue, struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_request req;

  req.id = 0xfe;
  req.refcnt = 1;
  req.opcode = pdu->parsed_opcode;
  req.handle = pdu->parsed_handle;
  req.owner = "bluez-shared-att-request";
  req.lifecycle = "alloc-enqueue-cancel-unref";

  pdu->att_queue_backend = "bluez-shared-att-queue:cancel-single-flight";
  pdu->att_request_id = req.id;
  pdu->att_request_owner = req.owner;
  pdu->att_request_lifecycle = req.lifecycle;
  pdu->att_request_ref_before = req.refcnt;
  pdu->att_timeout_state = "timer-armed-cleared-on-cancel";
  pdu->att_cancel_state = "cancelled-before-dispatch";
  pdu->att_timer_before = 1;
  pdu->att_pending_before = (uint8_t)(queue->pending + 1);
  queue->pending++;
  if (queue->pending > 0)
    {
      queue->pending--;
    }

  pdu->att_pending_after = queue->pending;
  pdu->att_completion_cb = "bluez-shared-att-callback:cancel";
  pdu->att_request_ref_after = 0;
  pdu->att_timer_after = 0;
}

static const char *bluez_profile_pdu_decode_opcode(
  const struct bluez_profile_pdu *pdu)
{
  if (pdu->payload_len == 0)
    {
      return "EMPTY";
    }

  if (!strcmp(pdu->wire, "att") || !strcmp(pdu->wire, "att-midi"))
    {
      switch (pdu->payload[0])
        {
          case 0x08:
            return "ATT_READ_BY_TYPE_REQ";
          case 0x0a:
            return "ATT_READ_REQ";
          case 0x10:
            return "ATT_READ_BY_GROUP_TYPE_REQ";
          case 0x12:
            if (!strcmp(pdu->opcode, "ATT_WRITE_REQ_NOTIFY") ||
                !strcmp(pdu->opcode, "ATT_WRITE_REQ_CCC"))
              {
                return pdu->opcode;
              }

            return "ATT_WRITE_REQ";
          case 0x1b:
            return "ATT_HANDLE_VALUE_NTF";
          case 0x80:
            return "MIDI_TIMESTAMPED_NOTE";
          default:
            return "ATT_UNKNOWN";
        }
    }

  if (!strcmp(pdu->wire, "hidp"))
    {
      switch (pdu->payload[0])
        {
          case 0x70:
            return "HIDP_SET_PROTOCOL";
          case 0xa1:
            return "HIDP_INPUT_REPORT";
          default:
            return "HIDP_UNKNOWN";
        }
    }

  if (!strcmp(pdu->wire, "rfcomm-at"))
    {
      if (bluez_profile_pdu_text_is(pdu, "AT+BRSF="))
        {
          return "AT_BRSF";
        }

      if (bluez_profile_pdu_text_is(pdu, "AT+BAC="))
        {
          return "AT_BAC_BCS";
        }

      if (bluez_profile_pdu_text_is(pdu, "AT+CLCC"))
        {
          return "AT_CLCC";
        }

      if (bluez_profile_pdu_text_is(pdu, "AT+CKPD"))
        {
          return "AT_CKPD";
        }

      if (bluez_profile_pdu_text_is(pdu, "AT+VGS="))
        {
          return "AT_VGS_VGM";
        }

      return "AT_UNKNOWN";
    }

  if (!strcmp(pdu->wire, "mesh"))
    {
      switch (pdu->payload[0])
        {
          case 0x03:
            return "MESH_PB_ADV_INVITE";
          case 0x29:
            return "MESH_PROXY_CONFIG";
          case 0x00:
            return "MESH_ACCESS_MESSAGE";
          default:
            return "MESH_UNKNOWN";
        }
    }

  if (!strcmp(pdu->wire, "rap-le-cs"))
    {
      switch (pdu->payload[0])
        {
          case 0x0a:
            return "RAP_ATT_READ_CAPABILITY";
          case 0x20:
            return "LE_CS_CAPABILITY_EXCHANGE";
          case 0x3e:
            return "LE_CS_RESULT_REPORT";
          default:
            return "RAP_LE_CS_UNKNOWN";
        }
    }

  if (!strcmp(pdu->wire, "rfcomm-hcrp"))
    {
      if (bluez_profile_pdu_text_is(pdu, "SDP:"))
        {
          return "SDP_SERVICE_SEARCH_ATTRIBUTE";
        }

      if (bluez_profile_pdu_text_is(pdu, "HCRP:DATA:"))
        {
          return "HCRP_JOB_DATA";
        }

      if (bluez_profile_pdu_text_is(pdu, "HCRP:STATUS:"))
        {
          return "HCRP_JOB_STATUS";
        }

      return "HCRP_UNKNOWN";
    }

  if (!strcmp(pdu->wire, "rfcomm-iap2"))
    {
      if (bluez_profile_pdu_text_is(pdu, "SDP:"))
        {
          return "SDP_SERVICE_SEARCH_ATTRIBUTE";
        }

      if (bluez_profile_pdu_text_is(pdu, "iAP2:IDENTIFY:"))
        {
          return "IAP2_IDENTIFY";
        }

      if (bluez_profile_pdu_text_is(pdu, "iAP2:EA:"))
        {
          return "IAP2_EA_SESSION";
        }

      return "IAP2_UNKNOWN";
    }

  return "GENERIC_PROFILE_PDU";
}

static int bluez_profile_shared_att_parse(const struct bluez_profile_pdu *pdu,
                                          struct bluez_profile_att_parse_result
                                          *result)
{
  result->detail = "att-profile-pdu-parse-rejected";
  result->backend = "bluez-shared-att-parser-rejected";
  result->handler_backend = "bluez-shared-att-dispatch:rejected";
  result->handle = 0;
  result->value_len = 0;
  result->opcode = pdu->payload_len > 0 ? pdu->payload[0] : 0;

  if (!strcmp(pdu->opcode, "ATT_READ_BY_TYPE_REQ") &&
      pdu->payload_len >= 7 && pdu->payload[0] == 0x08)
    {
      result->detail = "att-read-by-type-structure-ok";
      result->backend = "bluez-shared-att-core:read-by-type";
      result->handler_backend = "bluez-shared-att-dispatch:read-by-type";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->value_len = (uint16_t)(pdu->payload_len - 5);
      result->opcode = pdu->payload[0];
      return 1;
    }

  if (!strcmp(pdu->opcode, "ATT_READ_REQ") &&
      pdu->payload_len == 3 && pdu->payload[0] == 0x0a)
    {
      result->detail = "att-read-req-structure-ok";
      result->backend = "bluez-shared-att-core:read-req";
      result->handler_backend = "bluez-shared-att-dispatch:read-req";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->opcode = pdu->payload[0];
      return 1;
    }

  if (!strcmp(pdu->opcode, "ATT_READ_BY_GROUP_TYPE_REQ") &&
      pdu->payload_len >= 7 && pdu->payload[0] == 0x10)
    {
      result->detail = "att-read-by-group-type-structure-ok";
      result->backend = "bluez-shared-att-core:read-by-group-type";
      result->handler_backend =
        "bluez-shared-att-dispatch:read-by-group-type";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->value_len = (uint16_t)(pdu->payload_len - 5);
      result->opcode = pdu->payload[0];
      return 1;
    }

  if ((!strcmp(pdu->opcode, "ATT_WRITE_REQ") ||
       !strcmp(pdu->opcode, "ATT_WRITE_REQ_CCC")) &&
      pdu->payload_len >= 4 && pdu->payload[0] == 0x12)
    {
      result->detail = "att-write-req-structure-ok";
      result->backend = "bluez-shared-att-core:write-req";
      result->handler_backend = "bluez-shared-att-dispatch:write-req";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->value_len = (uint16_t)(pdu->payload_len - 3);
      result->opcode = pdu->payload[0];
      return 1;
    }

  if (!strcmp(pdu->opcode, "ATT_WRITE_REQ_NOTIFY") &&
      pdu->payload_len >= 9 && pdu->payload[0] == 0x12 &&
      pdu->payload[5] == 0x1b)
    {
      result->detail = "att-write-notify-structure-ok";
      result->backend = "bluez-shared-att-core:write-then-notify";
      result->handler_backend =
        "bluez-shared-att-dispatch:write-then-notify";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->value_len = (uint16_t)(pdu->payload_len - 3);
      result->opcode = pdu->payload[0];
      return 1;
    }

  if (!strcmp(pdu->opcode, "ATT_HANDLE_VALUE_NTF") &&
      pdu->payload_len >= 4 && pdu->payload[0] == 0x1b)
    {
      result->detail = "att-handle-value-notify-structure-ok";
      result->backend = "bluez-shared-att-core:handle-value-notify";
      result->handler_backend =
        "bluez-shared-att-dispatch:handle-value-notify";
      result->handle =
        (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
      result->value_len = (uint16_t)(pdu->payload_len - 3);
      result->opcode = pdu->payload[0];
      return 1;
    }

  if (!strcmp(pdu->opcode, "MIDI_TIMESTAMPED_NOTE") &&
      pdu->payload_len >= 5 && pdu->payload[0] == 0x80)
    {
      result->detail = "midi-timestamped-note-structure-ok";
      result->backend = "bluez-shared-att-core:midi-timestamped-note";
      result->handler_backend =
        "bluez-shared-att-dispatch:midi-timestamped-note";
      result->value_len = (uint16_t)pdu->payload_len;
      result->opcode = pdu->payload[0];
      return 1;
    }

  return 0;
}

static int bluez_profile_pdu_parse_payload(struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_parse_result att_result;

  pdu->parse_detail = "profile-pdu-parse-rejected";
  pdu->parse_backend = "profile-pdu-parser-rejected";
  pdu->handler_backend = "profile-handler-backend-rejected";
  pdu->att_error_rsp = "none";
  pdu->att_request_state = "not-enqueued";
  pdu->att_security_state = "not-required";
  pdu->att_completion_cb = "none";
  pdu->att_queue_backend = "none";
  pdu->att_request_owner = "none";
  pdu->att_request_lifecycle = "none";
  pdu->att_timeout_state = "none";
  pdu->att_cancel_state = "none";
  pdu->parsed_handle = 0;
  pdu->parsed_value_len = 0;
  pdu->att_mtu = 23;
  pdu->parsed_opcode = pdu->payload_len > 0 ? pdu->payload[0] : 0;
  pdu->att_error_code = 0;
  pdu->att_requires_security = 0;
  pdu->att_request_id = 0;
  pdu->att_pending_before = 0;
  pdu->att_pending_after = 0;
  pdu->att_request_ref_before = 0;
  pdu->att_request_ref_after = 0;
  pdu->att_timer_before = 0;
  pdu->att_timer_after = 0;

  if (!strcmp(pdu->wire, "att") || !strcmp(pdu->wire, "att-midi"))
    {
      if (bluez_profile_shared_att_parse(pdu, &att_result))
        {
          struct bluez_profile_att_queue queue;

          pdu->parse_detail = att_result.detail;
          pdu->parse_backend = att_result.backend;
          pdu->handler_backend = att_result.handler_backend;
          pdu->parsed_handle = att_result.handle;
          pdu->parsed_value_len = att_result.value_len;
          pdu->parsed_opcode = att_result.opcode;
          pdu->att_error_rsp = "none";
          pdu->att_error_code = 0;
          pdu->att_request_state = "queued-dispatched-completed";
          pdu->att_security_state = "security-checked";
          pdu->att_mtu = 23;
          pdu->att_requires_security =
            strstr(pdu->semantic, "ccc") != NULL ||
            strstr(pdu->semantic, "write") != NULL ||
            strstr(pdu->semantic, "notify") != NULL;
          bluez_profile_att_queue_init(&queue);
          bluez_profile_att_queue_enqueue(&queue, pdu);
          bluez_profile_att_queue_complete(&queue, pdu);
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "hidp"))
    {
      if (!strcmp(pdu->opcode, "HIDP_SET_PROTOCOL") &&
          pdu->payload_len == 2 && pdu->payload[0] == 0x70)
        {
          pdu->parse_detail = "hidp-set-protocol-structure-ok";
          pdu->parse_backend = "linux-hidp-control-parser";
          pdu->handler_backend = "linux-hidp-dispatch:set-protocol";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "HIDP_INPUT_REPORT") &&
          pdu->payload_len >= 6 && pdu->payload[0] == 0xa1)
        {
          pdu->parse_detail = "hidp-input-report-structure-ok";
          pdu->parse_backend = "linux-hidp-input-report-parser";
          pdu->handler_backend = "linux-hidp-dispatch:input-report";
          pdu->parsed_value_len = (uint16_t)(pdu->payload_len - 1);
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "rfcomm-at"))
    {
      if (!strcmp(pdu->opcode, "AT_BRSF") &&
          bluez_profile_pdu_text_is(pdu, "AT+BRSF="))
        {
          pdu->parse_detail = "rfcomm-at-brsf-structure-ok";
          pdu->parse_backend = "bluez-telephony-at-brsf-parser";
          pdu->handler_backend = "bluez-rfcomm-at-dispatch:brsf";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "AT_BAC_BCS") &&
          bluez_profile_pdu_text_is(pdu, "AT+BAC=") &&
          pdu->payload_len >= 17)
        {
          pdu->parse_detail = "rfcomm-at-codec-structure-ok";
          pdu->parse_backend = "bluez-telephony-at-codec-parser";
          pdu->handler_backend = "bluez-rfcomm-at-dispatch:codec";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "AT_CLCC") &&
          bluez_profile_pdu_text_is(pdu, "AT+CLCC"))
        {
          pdu->parse_detail = "rfcomm-at-clcc-structure-ok";
          pdu->parse_backend = "bluez-telephony-at-clcc-parser";
          pdu->handler_backend = "bluez-rfcomm-at-dispatch:clcc";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "AT_CKPD") &&
          bluez_profile_pdu_text_is(pdu, "AT+CKPD"))
        {
          pdu->parse_detail = "rfcomm-at-ckpd-structure-ok";
          pdu->parse_backend = "bluez-headset-at-ckpd-parser";
          pdu->handler_backend = "bluez-rfcomm-at-dispatch:ckpd";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "AT_VGS_VGM") &&
          bluez_profile_pdu_text_is(pdu, "AT+VGS=") &&
          pdu->payload_len >= 16)
        {
          pdu->parse_detail = "rfcomm-at-volume-structure-ok";
          pdu->parse_backend = "bluez-headset-at-volume-parser";
          pdu->handler_backend = "bluez-rfcomm-at-dispatch:volume";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "mesh"))
    {
      if (!strcmp(pdu->opcode, "MESH_PB_ADV_INVITE") &&
          pdu->payload_len >= 5 && pdu->payload[0] == 0x03)
        {
          pdu->parse_detail = "mesh-pb-adv-invite-structure-ok";
          pdu->parse_backend = "bluez-mesh-pb-adv-parser";
          pdu->handler_backend = "bluez-mesh-dispatch:pb-adv";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "MESH_PROXY_CONFIG") &&
          pdu->payload_len >= 5 && pdu->payload[0] == 0x29)
        {
          pdu->parse_detail = "mesh-proxy-config-structure-ok";
          pdu->parse_backend = "bluez-mesh-proxy-config-parser";
          pdu->handler_backend = "bluez-mesh-dispatch:proxy-config";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "MESH_ACCESS_MESSAGE") &&
          pdu->payload_len >= 6 && pdu->payload[0] == 0x00)
        {
          pdu->parse_detail = "mesh-access-message-structure-ok";
          pdu->parse_backend = "bluez-mesh-access-parser";
          pdu->handler_backend = "bluez-mesh-dispatch:access-message";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "rap-le-cs"))
    {
      if (!strcmp(pdu->opcode, "RAP_ATT_READ_CAPABILITY") &&
          pdu->payload_len == 3 && pdu->payload[0] == 0x0a)
        {
          pdu->parse_detail = "rap-att-read-capability-structure-ok";
          pdu->parse_backend = "bluez-rap-att-capability-parser";
          pdu->handler_backend = "bluez-rap-dispatch:att-capability";
          pdu->parsed_handle =
            (uint16_t)(pdu->payload[1] | (pdu->payload[2] << 8));
          return 1;
        }

      if (!strcmp(pdu->opcode, "LE_CS_CAPABILITY_EXCHANGE") &&
          pdu->payload_len >= 5 && pdu->payload[0] == 0x20)
        {
          pdu->parse_detail = "le-cs-capability-exchange-structure-ok";
          pdu->parse_backend = "bluez-rap-hci-cs-capability-parser";
          pdu->handler_backend = "bluez-rap-dispatch:hci-cs-capability";
          pdu->parsed_value_len = (uint16_t)(pdu->payload_len - 1);
          return 1;
        }

      if (!strcmp(pdu->opcode, "LE_CS_RESULT_REPORT") &&
          pdu->payload_len >= 6 && pdu->payload[0] == 0x3e)
        {
          pdu->parse_detail = "le-cs-result-report-structure-ok";
          pdu->parse_backend = "bluez-rap-hci-cs-result-parser";
          pdu->handler_backend = "bluez-rap-dispatch:hci-cs-result";
          pdu->parsed_value_len = (uint16_t)(pdu->payload_len - 1);
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "rfcomm-hcrp"))
    {
      if (!strcmp(pdu->opcode, "SDP_SERVICE_SEARCH_ATTRIBUTE") &&
          bluez_profile_pdu_text_is(pdu, "SDP:"))
        {
          pdu->parse_detail = "rfcomm-sdp-search-structure-ok";
          pdu->parse_backend = "bluez-sdp-service-search-parser";
          pdu->handler_backend = "bluez-sdp-dispatch:service-search";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "HCRP_JOB_DATA") &&
          bluez_profile_pdu_text_is(pdu, "HCRP:DATA:"))
        {
          pdu->parse_detail = "rfcomm-hcrp-job-data-structure-ok";
          pdu->parse_backend = "bluez-hcrp-job-data-parser";
          pdu->handler_backend = "bluez-hcrp-dispatch:job-data";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "HCRP_JOB_STATUS") &&
          bluez_profile_pdu_text_is(pdu, "HCRP:STATUS:"))
        {
          pdu->parse_detail = "rfcomm-hcrp-job-status-structure-ok";
          pdu->parse_backend = "bluez-hcrp-job-status-parser";
          pdu->handler_backend = "bluez-hcrp-dispatch:job-status";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      return 0;
    }

  if (!strcmp(pdu->wire, "rfcomm-iap2"))
    {
      if (!strcmp(pdu->opcode, "SDP_SERVICE_SEARCH_ATTRIBUTE") &&
          bluez_profile_pdu_text_is(pdu, "SDP:"))
        {
          pdu->parse_detail = "rfcomm-sdp-search-structure-ok";
          pdu->parse_backend = "bluez-sdp-service-search-parser";
          pdu->handler_backend = "bluez-sdp-dispatch:service-search";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "IAP2_IDENTIFY") &&
          bluez_profile_pdu_text_is(pdu, "iAP2:IDENTIFY:"))
        {
          pdu->parse_detail = "rfcomm-iap2-identify-structure-ok";
          pdu->parse_backend = "bluez-iap2-identify-parser";
          pdu->handler_backend = "bluez-iap2-dispatch:identify";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      if (!strcmp(pdu->opcode, "IAP2_EA_SESSION") &&
          bluez_profile_pdu_text_is(pdu, "iAP2:EA:"))
        {
          pdu->parse_detail = "rfcomm-iap2-ea-session-structure-ok";
          pdu->parse_backend = "bluez-iap2-ea-session-parser";
          pdu->handler_backend = "bluez-iap2-dispatch:ea-session";
          pdu->parsed_value_len = (uint16_t)pdu->payload_len;
          return 1;
        }

      return 0;
    }

  if (pdu->payload_len > 0)
    {
      pdu->parse_detail = "generic-profile-pdu-structure-ok";
      pdu->parse_backend = "generic-profile-pdu-parser";
      pdu->handler_backend = "generic-profile-dispatch";
      pdu->parsed_value_len = (uint16_t)pdu->payload_len;
      return 1;
    }

  return 0;
}

static void bluez_profile_pdu_finalize(struct bluez_profile_pdu *pdu)
{
  int parse_ok;

  pdu->encoder = "bluez-profile-pdu-encoder:upstream-shaped";
  pdu->decoder = "bluez-profile-pdu-decoder:upstream-shaped";
  pdu->decoded_opcode = bluez_profile_pdu_decode_opcode(pdu);
  pdu->roundtrip_ok = !strcmp(pdu->opcode, pdu->decoded_opcode);
  parse_ok = bluez_profile_pdu_parse_payload(pdu);
  pdu->parse_ok = parse_ok;
  pdu->handler_ok = pdu->roundtrip_ok && parse_ok;
  pdu->policy_ok = pdu->roundtrip_ok && parse_ok;
  pdu->error_map_ok = pdu->roundtrip_ok && parse_ok;
  pdu->cleanup_ok = pdu->roundtrip_ok && parse_ok;
  pdu->dispatch_result = pdu->roundtrip_ok && parse_ok ?
                         "handler-dispatched" : "handler-rejected";
  pdu->error_status = pdu->roundtrip_ok && parse_ok ?
                      "success-or-profile-error-mapped" :
                      "decode-error-mapped";
  pdu->cleanup_state = pdu->roundtrip_ok && parse_ok ?
                       "request-state-released" :
                       "request-state-rolled-back";
}

static void bluez_profile_pdu_finalize_error_probe(
  struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_queue queue;

  pdu->encoder = "bluez-profile-pdu-encoder:upstream-shaped";
  pdu->decoder = "bluez-profile-pdu-decoder:upstream-shaped";
  pdu->decoded_opcode = bluez_profile_pdu_decode_opcode(pdu);
  pdu->roundtrip_ok = 0;
  pdu->parse_ok = 0;
  pdu->handler_ok = 0;
  pdu->policy_ok = 1;
  pdu->error_map_ok = 1;
  pdu->cleanup_ok = 1;
  pdu->dispatch_result = "handler-rejected";
  pdu->error_status = "profile-error-mapped";
  pdu->cleanup_state = "request-state-released";
  pdu->parse_detail = "malformed-profile-pdu-rejected";
  pdu->parse_backend = "profile-error-path-parser";
  pdu->handler_backend = "profile-error-dispatch:rejected";
  pdu->att_error_rsp = "ATT_ERROR_RSP_INVALID_PDU-or-profile-error";
  pdu->att_request_state = "queued-rejected-released";
  pdu->att_security_state = "security-not-consumed";
  pdu->parsed_handle = 0;
  pdu->parsed_value_len = 0;
  pdu->att_mtu = 23;
  pdu->parsed_opcode = pdu->payload_len > 0 ? pdu->payload[0] : 0;
  pdu->att_error_code = 0x04;
  pdu->att_requires_security = 0;
  bluez_profile_att_queue_init(&queue);
  bluez_profile_att_queue_error(&queue, pdu);
}

static void bluez_profile_pdu_finalize_cancel_probe(
  struct bluez_profile_pdu *pdu)
{
  struct bluez_profile_att_queue queue;

  pdu->encoder = "bluez-profile-pdu-encoder:upstream-shaped";
  pdu->decoder = "bluez-profile-pdu-decoder:upstream-shaped";
  pdu->decoded_opcode = bluez_profile_pdu_decode_opcode(pdu);
  pdu->roundtrip_ok = !strcmp(pdu->opcode, pdu->decoded_opcode);
  pdu->parse_ok = 1;
  pdu->handler_ok = 0;
  pdu->policy_ok = 1;
  pdu->error_map_ok = 1;
  pdu->cleanup_ok = 1;
  pdu->dispatch_result = "handler-cancelled";
  pdu->error_status = "request-cancelled-mapped";
  pdu->cleanup_state = "request-state-released";
  pdu->parse_detail = "att-cancel-probe-structure-ok";
  pdu->parse_backend = "bluez-shared-att-core:cancel-probe";
  pdu->handler_backend = "bluez-shared-att-dispatch:cancelled";
  pdu->att_error_rsp = "ATT_ERROR_RSP_REQUEST_CANCELLED-or-profile-cancel";
  pdu->att_request_state = "queued-cancelled-released";
  pdu->att_security_state = "security-not-consumed";
  pdu->att_mtu = 23;
  pdu->att_error_code = 0x02;
  pdu->att_requires_security = 0;
  bluez_profile_att_queue_init(&queue);
  bluez_profile_att_queue_cancel(&queue, pdu);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int bluez_profile_pdu_encode(const char *family, const char *label,
                             const char *transport,
                             struct bluez_profile_pdu *pdu)
{
  static const uint8_t hid_control[] = {0x70, 0x01};
  static const uint8_t hid_interrupt[] = {0xa1, 0x01, 0x00, 0x00, 0x04, 0x00};
  static const uint8_t att_read_report_map[] = {0x0a, 0x25, 0x00};
  static const uint8_t att_write_protocol[] = {0x12, 0x26, 0x00, 0x01};
  static const uint8_t att_notify_input[] = {0x1b, 0x27, 0x00, 0x01, 0x00};
  static const uint8_t att_discover_primary[] =
  {
    0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
  };
  static const uint8_t att_read_value[] = {0x0a, 0x01, 0x00};
  static const uint8_t att_write_notify[] =
  {
    0x12, 0x01, 0x00, 0x01, 0x00, 0x1b, 0x01, 0x00, 0x01
  };
  static const uint8_t mesh_prov[] = {0x03, 0x00, 0x01, 0x00, 0x05};
  static const uint8_t mesh_proxy[] = {0x29, 0x00, 0x00, 0x80, 0x03};
  static const uint8_t mesh_access[] = {0x00, 0x05, 0x7f, 0x82, 0x02, 0x00};
  static const uint8_t asha_discover[] =
  {
    0x08, 0x01, 0x00, 0xff, 0xff, 0xf0, 0xf0
  };
  static const uint8_t asha_props[] = {0x0a, 0x31, 0x00};
  static const uint8_t asha_status[] = {0x1b, 0x33, 0x00, 0x01};
  static const uint8_t midi_discover[] =
  {
    0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28
  };
  static const uint8_t midi_ccc[] = {0x12, 0x03, 0x00, 0x01, 0x00};
  static const uint8_t midi_note[] = {0x80, 0x80, 0x90, 0x3c, 0x64};
  static const uint8_t ranging_capability[] = {0x0a, 0x44, 0x00};
  static const uint8_t ranging_cs_exchange[] = {0x20, 0x96, 0x02, 0x00, 0x01};
  static const uint8_t ranging_result[] = {0x3e, 0x2f, 0x01, 0x7b, 0x00, 0x60};
  static const char hfp_brsf[] = "AT+BRSF=1024\r";
  static const char hfp_codec[] = "AT+BAC=1,2\rAT+BCS=2\r";
  static const char hfp_clcc[] = "AT+CLCC\r";
  static const char hsp_ckpd[] = "AT+CKPD=200\r";
  static const char hsp_volume[] = "AT+VGS=12\rAT+VGM=10\r";
  static const char print_sdp[] = "SDP:HCRP+SPP:ServiceSearchAttribute\r\n";
  static const char print_job[] = "HCRP:DATA:job-document\r\n";
  static const char print_status[] = "HCRP:STATUS:job-complete\r\n";
  static const char iap_sdp[] = "SDP:SPP+iAP:ServiceSearchAttribute\r\n";
  static const char iap_identify[] = "iAP2:IDENTIFY:device-info\r\n";
  static const char iap_ea[] = "iAP2:EA:open-session\r\n";
  static const char generic[] = "PROFILE:PDU:generic\r\n";

  bluez_profile_pdu_defaults(family, transport, pdu);
  pdu->semantic = bluez_profile_pdu_semantic(family, label);

  if (!strcmp(family, "hid"))
    {
      pdu->wire = "hidp";
      if (strstr(label, "control") != NULL)
        {
          pdu->opcode = "HIDP_SET_PROTOCOL";
          bluez_profile_pdu_copy(pdu, hid_control, sizeof(hid_control));
        }
      else
        {
          pdu->opcode = "HIDP_INPUT_REPORT";
          bluez_profile_pdu_copy(pdu, hid_interrupt, sizeof(hid_interrupt));
        }
    }
  else if (!strcmp(family, "hogp"))
    {
      pdu->wire = "att";
      if (strstr(label, "report-map") != NULL)
        {
          pdu->opcode = "ATT_READ_REQ";
          bluez_profile_pdu_copy(pdu, att_read_report_map,
                                 sizeof(att_read_report_map));
        }
      else if (strstr(label, "protocol") != NULL)
        {
          pdu->opcode = "ATT_WRITE_REQ";
          bluez_profile_pdu_copy(pdu, att_write_protocol,
                                 sizeof(att_write_protocol));
        }
      else
        {
          pdu->opcode = "ATT_HANDLE_VALUE_NTF";
          bluez_profile_pdu_copy(pdu, att_notify_input,
                                 sizeof(att_notify_input));
        }
    }
  else if (!strcmp(family, "hfp"))
    {
      pdu->wire = "rfcomm-at";
      if (strstr(label, "brsf") != NULL)
        {
          pdu->opcode = "AT_BRSF";
          bluez_profile_pdu_copy(pdu, hfp_brsf, sizeof(hfp_brsf) - 1);
        }
      else if (strstr(label, "codec") != NULL)
        {
          pdu->opcode = "AT_BAC_BCS";
          bluez_profile_pdu_copy(pdu, hfp_codec, sizeof(hfp_codec) - 1);
        }
      else
        {
          pdu->opcode = "AT_CLCC";
          bluez_profile_pdu_copy(pdu, hfp_clcc, sizeof(hfp_clcc) - 1);
        }
    }
  else if (!strcmp(family, "hsp"))
    {
      pdu->wire = "rfcomm-at";
      if (strstr(label, "volume") != NULL)
        {
          pdu->opcode = "AT_VGS_VGM";
          bluez_profile_pdu_copy(pdu, hsp_volume, sizeof(hsp_volume) - 1);
        }
      else
        {
          pdu->opcode = "AT_CKPD";
          bluez_profile_pdu_copy(pdu, hsp_ckpd, sizeof(hsp_ckpd) - 1);
        }
    }
  else if (!strcmp(family, "gatt"))
    {
      pdu->wire = "att";
      if (strstr(label, "discover") != NULL ||
          strstr(label, "register") != NULL)
        {
          pdu->opcode = "ATT_READ_BY_GROUP_TYPE_REQ";
          bluez_profile_pdu_copy(pdu, att_discover_primary,
                                 sizeof(att_discover_primary));
        }
      else if (strstr(label, "read") != NULL)
        {
          pdu->opcode = "ATT_READ_REQ";
          bluez_profile_pdu_copy(pdu, att_read_value, sizeof(att_read_value));
        }
      else
        {
          pdu->opcode = "ATT_WRITE_REQ_NOTIFY";
          bluez_profile_pdu_copy(pdu, att_write_notify,
                                 sizeof(att_write_notify));
        }
    }
  else if (!strcmp(family, "mesh"))
    {
      pdu->wire = "mesh";
      if (strstr(label, "provision") != NULL)
        {
          pdu->opcode = "MESH_PB_ADV_INVITE";
          bluez_profile_pdu_copy(pdu, mesh_prov, sizeof(mesh_prov));
        }
      else if (strstr(label, "proxy") != NULL)
        {
          pdu->opcode = "MESH_PROXY_CONFIG";
          bluez_profile_pdu_copy(pdu, mesh_proxy, sizeof(mesh_proxy));
        }
      else
        {
          pdu->opcode = "MESH_ACCESS_MESSAGE";
          bluez_profile_pdu_copy(pdu, mesh_access, sizeof(mesh_access));
        }
    }
  else if (!strcmp(family, "asha"))
    {
      pdu->wire = "att";
      if (strstr(label, "discovery") != NULL)
        {
          pdu->opcode = "ATT_READ_BY_TYPE_REQ";
          bluez_profile_pdu_copy(pdu, asha_discover, sizeof(asha_discover));
        }
      else if (strstr(label, "properties") != NULL)
        {
          pdu->opcode = "ATT_READ_REQ";
          bluez_profile_pdu_copy(pdu, asha_props, sizeof(asha_props));
        }
      else
        {
          pdu->opcode = "ATT_HANDLE_VALUE_NTF";
          bluez_profile_pdu_copy(pdu, asha_status, sizeof(asha_status));
        }
    }
  else if (!strcmp(family, "midi"))
    {
      pdu->wire = "att-midi";
      if (strstr(label, "discovery") != NULL)
        {
          pdu->opcode = "ATT_READ_BY_GROUP_TYPE_REQ";
          bluez_profile_pdu_copy(pdu, midi_discover, sizeof(midi_discover));
        }
      else if (strstr(label, "ccc") != NULL)
        {
          pdu->opcode = "ATT_WRITE_REQ_CCC";
          bluez_profile_pdu_copy(pdu, midi_ccc, sizeof(midi_ccc));
        }
      else
        {
          pdu->opcode = "MIDI_TIMESTAMPED_NOTE";
          bluez_profile_pdu_copy(pdu, midi_note, sizeof(midi_note));
        }
    }
  else if (!strcmp(family, "ranging"))
    {
      pdu->wire = "rap-le-cs";
      if (strstr(label, "service") != NULL)
        {
          pdu->opcode = "RAP_ATT_READ_CAPABILITY";
          bluez_profile_pdu_copy(pdu, ranging_capability,
                                 sizeof(ranging_capability));
        }
      else if (strstr(label, "capability") != NULL)
        {
          pdu->opcode = "LE_CS_CAPABILITY_EXCHANGE";
          bluez_profile_pdu_copy(pdu, ranging_cs_exchange,
                                 sizeof(ranging_cs_exchange));
        }
      else
        {
          pdu->opcode = "LE_CS_RESULT_REPORT";
          bluez_profile_pdu_copy(pdu, ranging_result, sizeof(ranging_result));
        }
    }
  else if (!strcmp(family, "print"))
    {
      pdu->wire = "rfcomm-hcrp";
      if (strstr(label, "sdp") != NULL)
        {
          pdu->opcode = "SDP_SERVICE_SEARCH_ATTRIBUTE";
          bluez_profile_pdu_copy(pdu, print_sdp, sizeof(print_sdp) - 1);
        }
      else if (strstr(label, "create") != NULL ||
               strstr(label, "receive") != NULL)
        {
          pdu->opcode = "HCRP_JOB_DATA";
          bluez_profile_pdu_copy(pdu, print_job, sizeof(print_job) - 1);
        }
      else
        {
          pdu->opcode = "HCRP_JOB_STATUS";
          bluez_profile_pdu_copy(pdu, print_status,
                                 sizeof(print_status) - 1);
        }
    }
  else if (!strcmp(family, "iap"))
    {
      pdu->wire = "rfcomm-iap2";
      if (strstr(label, "sdp") != NULL)
        {
          pdu->opcode = "SDP_SERVICE_SEARCH_ATTRIBUTE";
          bluez_profile_pdu_copy(pdu, iap_sdp, sizeof(iap_sdp) - 1);
        }
      else if (strstr(label, "identify") != NULL)
        {
          pdu->opcode = "IAP2_IDENTIFY";
          bluez_profile_pdu_copy(pdu, iap_identify,
                                 sizeof(iap_identify) - 1);
        }
      else
        {
          pdu->opcode = "IAP2_EA_SESSION";
          bluez_profile_pdu_copy(pdu, iap_ea, sizeof(iap_ea) - 1);
        }
    }
  else
    {
      bluez_profile_pdu_copy(pdu, generic, sizeof(generic) - 1);
    }

  bluez_profile_pdu_finalize(pdu);
  return 0;
}

int bluez_profile_pdu_encode_error_probe(const char *family,
                                         const char *transport,
                                         struct bluez_profile_pdu *pdu)
{
  static const uint8_t att_bad[] = {0xff, 0x00};
  static const uint8_t hidp_bad[] = {0x00};
  static const uint8_t mesh_bad[] = {0xff, 0x00, 0x00};
  static const uint8_t rap_bad[] = {0xff, 0x00};
  static const char rfcomm_bad[] = "BAD\r";
  static const char generic_bad[] = "";

  bluez_profile_pdu_defaults(family, transport, pdu);
  pdu->semantic = "profile-error-probe=malformed-pdu";
  pdu->opcode = "MALFORMED_PROFILE_PDU";

  if (!strcmp(family, "hid"))
    {
      pdu->wire = "hidp";
      bluez_profile_pdu_copy(pdu, hidp_bad, sizeof(hidp_bad));
    }
  else if (!strcmp(family, "hogp") || !strcmp(family, "gatt") ||
           !strcmp(family, "asha"))
    {
      pdu->wire = "att";
      bluez_profile_pdu_copy(pdu, att_bad, sizeof(att_bad));
    }
  else if (!strcmp(family, "midi"))
    {
      pdu->wire = "att-midi";
      bluez_profile_pdu_copy(pdu, att_bad, sizeof(att_bad));
    }
  else if (!strcmp(family, "hfp") || !strcmp(family, "hsp"))
    {
      pdu->wire = "rfcomm-at";
      bluez_profile_pdu_copy(pdu, rfcomm_bad, sizeof(rfcomm_bad) - 1);
    }
  else if (!strcmp(family, "mesh"))
    {
      pdu->wire = "mesh";
      bluez_profile_pdu_copy(pdu, mesh_bad, sizeof(mesh_bad));
    }
  else if (!strcmp(family, "ranging"))
    {
      pdu->wire = "rap-le-cs";
      bluez_profile_pdu_copy(pdu, rap_bad, sizeof(rap_bad));
    }
  else if (!strcmp(family, "print"))
    {
      pdu->wire = "rfcomm-hcrp";
      bluez_profile_pdu_copy(pdu, rfcomm_bad, sizeof(rfcomm_bad) - 1);
    }
  else if (!strcmp(family, "iap"))
    {
      pdu->wire = "rfcomm-iap2";
      bluez_profile_pdu_copy(pdu, rfcomm_bad, sizeof(rfcomm_bad) - 1);
    }
  else
    {
      bluez_profile_pdu_copy(pdu, generic_bad, sizeof(generic_bad) - 1);
    }

  bluez_profile_pdu_finalize_error_probe(pdu);
  return 0;
}

int bluez_profile_pdu_encode_cancel_probe(const char *family,
                                          const char *transport,
                                          struct bluez_profile_pdu *pdu)
{
  static const uint8_t att_cancel_read[] = {0x0a, 0x01, 0x00};

  bluez_profile_pdu_defaults(family, transport, pdu);
  pdu->semantic = "profile-cancel-probe=queued-request";
  pdu->opcode = "ATT_READ_REQ";
  pdu->wire = "att";
  bluez_profile_pdu_copy(pdu, att_cancel_read, sizeof(att_cancel_read));
  pdu->parsed_handle = 0x0001;
  pdu->parsed_value_len = 0;
  pdu->parsed_opcode = 0x0a;
  bluez_profile_pdu_finalize_cancel_probe(pdu);
  return 0;
}

void bluez_profile_pdu_hex(const struct bluez_profile_pdu *pdu,
                           char *out, size_t out_len)
{
  size_t off = 0;
  size_t i;

  if (out_len == 0)
    {
      return;
    }

  out[0] = '\0';
  for (i = 0; i < pdu->payload_len && off + 4 < out_len; i++)
    {
      off += (size_t)snprintf(out + off, out_len - off, "%s%02x",
                              i == 0 ? "" : " ", pdu->payload[i]);
    }
}
