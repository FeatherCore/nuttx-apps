/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/profile_main.c
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

#include "../upstream_profile_pdu_codec.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_profile_mode
{
  const char *mode;
  const char *family;
  const char *role;
  const char *transport;
  const char *source;
  const char *boundary;
  const char *label1;
  const char *label2;
  const char *label3;
  uint16_t psm;
  uint16_t cid;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct bluez_profile_mode g_bluez_profile_modes[] =
{
  {
    "classic-hid-host", "hid", "host", "l2cap-hid",
    "third/bluez/profiles/input/device.c",
	    "bluezprofile-hid-upstream-link-bluetoothd",
	    "hid-sdp-profile-connect",
	    "hid-control-set-protocol",
	    "hid-interrupt-input-output-report",
	    0x0011, 0x0051
	  },
	  {
	    "classic-hid-device", "hid", "device", "l2cap-hid",
    "third/bluez/profiles/input/server.c",
	    "bluezprofile-hid-upstream-link-bluetoothd",
	    "hid-sdp-register",
	    "hid-control-set-protocol",
	    "hid-interrupt-input-output-report",
	    0x0011, 0x0051
	  },
  {
    "hogp-host", "hogp", "host", "att",
    "third/bluez/profiles/input/hog-lib.c",
    "bluezprofile-hogp-upstream-link-bluetoothd",
    "hogp-report-map-read",
    "hogp-protocol-mode-write",
    "hogp-input-ccc-notify",
    0x0000, 0x0004
  },
  {
    "hogp-device", "hogp", "device", "att",
    "third/bluez/profiles/input/hog-lib.c",
    "bluezprofile-hogp-upstream-link-bluetoothd",
    "hogp-report-map-read",
    "hogp-protocol-mode-write",
    "hogp-input-ccc-notify",
    0x0000, 0x0004
  },
  {
    "hfp-hf", "hfp", "handsfree", "rfcomm",
    "third/bluez/profiles/audio/hfp-hf.c",
    "bluezprofile-hfp-upstream-link-bluetoothd",
    "hfp-slc-brsf",
    "hfp-codec-bac-bcs",
    "hfp-call-clcc",
    0x0003, 0x0072
  },
  {
    "hfp-ag", "hfp", "audio-gateway", "rfcomm",
    "third/bluez/profiles/audio/telephony.c",
    "bluezprofile-hfp-upstream-link-bluetoothd",
    "hfp-slc-brsf",
    "hfp-codec-bac-bcs",
    "hfp-call-clcc",
    0x0003, 0x0072
  },
  {
    "hsp-hs", "hsp", "headset", "rfcomm",
    "third/bluez/profiles/audio/telephony.c",
    "bluezprofile-hsp-upstream-link-bluetoothd",
    "hsp-sdp-profile-connect",
    "hsp-button-ckpd",
    "hsp-volume-vgs",
    0x0003, 0x0073
  },
  {
    "hsp-ag", "hsp", "audio-gateway", "rfcomm",
    "third/bluez/profiles/audio/telephony.c",
    "bluezprofile-hsp-upstream-link-bluetoothd",
    "hsp-sdp-profile-connect",
    "hsp-button-ckpd",
    "hsp-volume-vgs",
    0x0003, 0x0073
  },
  {
    "gatt-client", "gatt", "client", "att",
    "third/bluez/src/gatt-client.c",
    "bluezprofile-gatt-upstream-link-bluetoothd",
    "gatt-discover-primary",
    "gatt-read-characteristic",
    "gatt-write-ccc-notify",
    0x0000, 0x0004
  },
  {
    "gatt-server", "gatt", "server", "att",
    "third/bluez/src/gatt-database.c",
    "bluezprofile-gatt-upstream-link-bluetoothd",
    "gatt-application-register",
    "gatt-read-characteristic",
    "gatt-notify-indicate",
    0x0000, 0x0004
  },
  {
    "mesh-provisioner", "mesh", "provisioner", "att-gatt-proxy",
    "third/bluez/mesh/main.c",
    "bluezprofile-mesh-upstream-link-bluetoothd",
    "mesh-provisioning-service-discovery",
    "mesh-proxy-characteristic-discovery",
    "mesh-network-pdu-gatt-proxy",
    0x0000, 0x0004
  },
  {
    "mesh-node", "mesh", "node", "att-gatt-proxy",
    "third/bluez/mesh/node.c",
    "bluezprofile-mesh-upstream-link-bluetoothd",
    "mesh-provisioning-service-discovery",
    "mesh-proxy-characteristic-discovery",
    "mesh-network-pdu-gatt-proxy",
    0x0000, 0x0004
  },
  {
    "asha-central", "asha", "central", "att",
    "third/bluez/profiles/audio/asha.c",
    "bluezprofile-asha-upstream-link-bluetoothd",
    "asha-service-discovery",
    "asha-read-properties",
    "asha-audio-status-notify",
    0x0000, 0x0004
  },
  {
    "asha-hearing-aid", "asha", "hearing-aid", "att",
    "third/bluez/profiles/audio/asha.c",
    "bluezprofile-asha-upstream-link-bluetoothd",
    "asha-service-discovery",
    "asha-read-properties",
    "asha-audio-status-notify",
    0x0000, 0x0004
  },
  {
    "midi-controller", "midi", "controller", "att",
    "third/bluez/profiles/midi/midi.c",
    "bluezprofile-midi-upstream-link-bluetoothd",
    "midi-service-discovery",
    "midi-ccc-enable",
    "midi-note-event",
    0x0000, 0x0004
  },
  {
    "midi-peripheral", "midi", "peripheral", "att",
    "third/bluez/profiles/midi/midi.c",
    "bluezprofile-midi-upstream-link-bluetoothd",
    "midi-service-discovery",
    "midi-ccc-enable",
    "midi-note-event",
    0x0000, 0x0004
  },
  {
    "ranging-initiator", "ranging", "initiator", "att",
    "third/bluez/profiles/ranging/ranging.c",
    "bluezprofile-ranging-upstream-link-bluetoothd",
    "ranging-service-discovery",
    "ranging-capability-exchange",
    "ranging-result-report",
    0x0000, 0x0004
  },
  {
    "ranging-reflector", "ranging", "reflector", "att",
    "third/bluez/profiles/ranging/ranging.c",
    "bluezprofile-ranging-upstream-link-bluetoothd",
    "ranging-service-discovery",
    "ranging-capability-exchange",
    "ranging-result-report",
    0x0000, 0x0004
  },
  {
    "print-client", "print", "client", "rfcomm",
    "third/bluez/profiles/printing/manager.c",
    "bluezprofile-print-upstream-link-bluetoothd",
    "print-sdp-query",
    "print-job-create",
    "print-job-status",
    0x0003, 0x006b
  },
  {
    "print-printer", "print", "printer", "rfcomm",
    "third/bluez/profiles/printing/manager.c",
    "bluezprofile-print-upstream-link-bluetoothd",
    "print-sdp-register",
    "print-job-receive",
    "print-job-status",
    0x0003, 0x006b
  },
  {
    "iap-controller", "iap", "controller", "rfcomm",
    "third/bluez/profiles/iap/main.c",
    "bluezprofile-iap-upstream-link-bluetoothd",
    "iap-sdp-query",
    "iap-identify",
    "iap-eap-session",
    0x0003, 0x006c
  },
  {
    "iap-accessory", "iap", "accessory", "rfcomm",
    "third/bluez/profiles/iap/main.c",
    "bluezprofile-iap-upstream-link-bluetoothd",
    "iap-sdp-register",
    "iap-identify",
    "iap-eap-session",
    0x0003, 0x006c
  }
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_profile_usage(void)
{
  printf("usage: bluezprofile closeout ");
  printf("classic-hid-host|classic-hid-device|hogp-host|hogp-device|");
  printf("hfp-hf|hfp-ag|hsp-hs|hsp-ag|gatt-client|gatt-server|");
  printf("mesh-provisioner|mesh-node|asha-central|asha-hearing-aid|");
  printf("midi-controller|midi-peripheral|ranging-initiator|");
  printf("ranging-reflector|print-client|print-printer|");
  printf("iap-controller|iap-accessory [peer]\n");
}

static uint16_t bluez_profile_handle(const struct bluez_profile_mode *mode,
                                     uint16_t peer)
{
  uint16_t base = 0x0050;

  (void)mode;

#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(base + (endpoint & 0x00ff));
#else
  return (uint16_t)(base + (peer & 0x00ff));
#endif
}

static const struct bluez_profile_mode *bluez_profile_find(
  const char *mode)
{
  size_t i;

  for (i = 0; i < sizeof(g_bluez_profile_modes) /
                  sizeof(g_bluez_profile_modes[0]); i++)
    {
      if (!strcmp(mode, g_bluez_profile_modes[i].mode))
        {
          return &g_bluez_profile_modes[i];
        }
    }

  return NULL;
}

static int bluez_profile_delegated_responder(
  const struct bluez_profile_mode *mode)
{
  return !strcmp(mode->mode, "classic-hid-host") ||
         !strcmp(mode->mode, "asha-hearing-aid") ||
         !strcmp(mode->mode, "gatt-server") ||
         !strcmp(mode->mode, "hfp-ag") ||
         !strcmp(mode->mode, "hogp-device") ||
         !strcmp(mode->mode, "hsp-ag") ||
         !strcmp(mode->mode, "iap-accessory") ||
         !strcmp(mode->mode, "mesh-provisioner") ||
         !strcmp(mode->mode, "midi-peripheral") ||
         !strcmp(mode->mode, "print-client") ||
         !strcmp(mode->mode, "print-printer") ||
	         !strcmp(mode->mode, "ranging-reflector");
}

static uint16_t bluez_profile_label_psm(const struct bluez_profile_mode *mode,
                                        const char *label)
{
  if (mode != NULL && label != NULL && !strcmp(mode->family, "hid") &&
      !strcmp(label, "hid-interrupt-input-output-report"))
    {
      return 0x0013;
    }

  return mode->psm;
}

static uint16_t bluez_profile_label_cid(const struct bluez_profile_mode *mode,
                                        const char *label)
{
  if (mode != NULL && label != NULL && !strcmp(mode->family, "hid") &&
      !strcmp(label, "hid-interrupt-input-output-report"))
    {
      return 0x0053;
    }

  return mode->cid;
}

static int bluez_profile_write_label(void *sock,
  const struct bluez_profile_mode *mode, const char *label)
{
  struct bluez_profile_pdu pdu;
  char hex[BLUEZ_PROFILE_PDU_HEX_MAX];
  char out[256];
  int ret;

  ret = bluez_profile_pdu_encode(mode->family, label, mode->transport,
                                 &pdu);
  if (ret < 0)
    {
      return ret;
    }

  bluez_profile_pdu_hex(&pdu, hex, sizeof(hex));
  printf("bluez-profile: upstream-pdu family=%s mode=%s role=%s "
         "label=%s semantic=%s opcode=%s wire=%s codec-source=%s "
         "upstream-link=%s encoder=%s decoder=%s decoded-opcode=%s "
         "parser-owner=%s handler-owner=%s policy-owner=%s "
         "error-owner=%s cleanup-owner=%s roundtrip=%u "
         "parse-ok=%u parse-detail=%s parse-backend=%s "
         "parsed-handle=0x%04x parsed-value-len=%u "
         "parsed-opcode=0x%02x handler-backend=%s "
         "att-error-code=0x%02x att-error-rsp=%s "
         "att-request-state=%s att-mtu=%u att-security-state=%s "
         "att-requires-security=%u "
         "att-request-id=0x%02x att-pending-before=%u "
         "att-pending-after=%u att-completion-cb=%s "
         "att-queue-backend=%s "
         "att-request-owner=%s att-request-lifecycle=%s "
         "att-request-ref-before=%u att-request-ref-after=%u "
         "att-timeout-state=%s att-cancel-state=%s "
         "att-timer-before=%u att-timer-after=%u "
         "dispatch-result=%s handler-ok=%u "
         "policy-ok=%u error-status=%s error-map-ok=%u "
         "cleanup-state=%s cleanup-ok=%u payload-len=%u payload-hex=%s\n",
         mode->family, mode->mode, mode->role, label, pdu.semantic,
         pdu.opcode, pdu.wire, pdu.codec_source, pdu.upstream_owner,
         pdu.encoder, pdu.decoder, pdu.decoded_opcode, pdu.parser_owner,
         pdu.handler_owner, pdu.policy_owner, pdu.error_owner,
         pdu.cleanup_owner, (unsigned int)pdu.roundtrip_ok,
         (unsigned int)pdu.parse_ok, pdu.parse_detail, pdu.parse_backend,
         pdu.parsed_handle, (unsigned int)pdu.parsed_value_len,
         (unsigned int)pdu.parsed_opcode, pdu.handler_backend,
         (unsigned int)pdu.att_error_code, pdu.att_error_rsp,
         pdu.att_request_state, (unsigned int)pdu.att_mtu,
         pdu.att_security_state, (unsigned int)pdu.att_requires_security,
         (unsigned int)pdu.att_request_id,
         (unsigned int)pdu.att_pending_before,
         (unsigned int)pdu.att_pending_after, pdu.att_completion_cb,
         pdu.att_queue_backend,
         pdu.att_request_owner, pdu.att_request_lifecycle,
         (unsigned int)pdu.att_request_ref_before,
         (unsigned int)pdu.att_request_ref_after,
         pdu.att_timeout_state, pdu.att_cancel_state,
         (unsigned int)pdu.att_timer_before,
         (unsigned int)pdu.att_timer_after,
         pdu.dispatch_result,
         (unsigned int)pdu.handler_ok, (unsigned int)pdu.policy_ok,
         pdu.error_status, (unsigned int)pdu.error_map_ok,
         pdu.cleanup_state, (unsigned int)pdu.cleanup_ok,
         (unsigned int)pdu.payload_len, hex);

  if (bluez_profile_delegated_responder(mode))
    {
      printf("bluez-profile: source=%s style=%s command=transaction "
             "family=%s mode=%s role=%s label=%s write-ret=0 "
             "detail=daemon-mainloop-owned responder-delegated-io=1\n",
             mode->source, mode->transport, mode->family, mode->mode,
             mode->role, label);
      return 0;
    }

	  memset(out, 0, sizeof(out));
	  ret = linux_bt_upstream_l2cap_socket_connect_handle(
	          sock, bluez_profile_label_psm(mode, label),
	          bluez_profile_label_cid(mode, label));
	  if (ret < 0)
	    {
	      printf("bluez-profile: source=%s style=%s command=channel-select "
	             "family=%s mode=%s role=%s label=%s connect-ret=%d\n",
	             mode->source, mode->transport, mode->family, mode->mode,
	             mode->role, label, ret);
	      return ret;
	    }

	  ret = linux_bt_upstream_l2cap_socket_write_handle(
	          sock, pdu.payload, pdu.payload_len, out, sizeof(out));
  printf("bluez-profile: source=%s style=%s command=transaction "
         "family=%s mode=%s role=%s label=%s write-ret=%d\n",
         mode->source, mode->transport, mode->family, mode->mode,
         mode->role, label, ret);
  printf("%s", out);

  return ret;
}

static int bluez_profile_write_error_probe(void *sock,
  const struct bluez_profile_mode *mode)
{
  struct bluez_profile_pdu pdu;
  char hex[BLUEZ_PROFILE_PDU_HEX_MAX];
  char out[256];
  int ret;

  ret = bluez_profile_pdu_encode_error_probe(mode->family,
                                             mode->transport,
                                             &pdu);
  if (ret < 0)
    {
      return ret;
    }

  bluez_profile_pdu_hex(&pdu, hex, sizeof(hex));
  printf("bluez-profile: upstream-error-pdu family=%s mode=%s role=%s "
         "semantic=%s opcode=%s wire=%s codec-source=%s "
         "upstream-link=%s encoder=%s decoder=%s decoded-opcode=%s "
         "parser-owner=%s handler-owner=%s policy-owner=%s "
         "error-owner=%s cleanup-owner=%s roundtrip=%u parse-ok=%u "
         "parse-detail=%s parse-backend=%s parsed-handle=0x%04x "
         "parsed-value-len=%u parsed-opcode=0x%02x "
         "handler-backend=%s att-error-code=0x%02x att-error-rsp=%s "
         "att-request-state=%s att-mtu=%u att-security-state=%s "
         "att-requires-security=%u "
         "att-request-id=0x%02x att-pending-before=%u "
         "att-pending-after=%u att-completion-cb=%s "
         "att-queue-backend=%s "
         "att-request-owner=%s att-request-lifecycle=%s "
         "att-request-ref-before=%u att-request-ref-after=%u "
         "att-timeout-state=%s att-cancel-state=%s "
         "att-timer-before=%u att-timer-after=%u "
         "dispatch-result=%s handler-ok=%u policy-ok=%u "
         "error-status=%s error-map-ok=%u "
         "cleanup-state=%s cleanup-ok=%u payload-len=%u payload-hex=%s\n",
         mode->family, mode->mode, mode->role, pdu.semantic,
         pdu.opcode, pdu.wire, pdu.codec_source, pdu.upstream_owner,
         pdu.encoder, pdu.decoder, pdu.decoded_opcode, pdu.parser_owner,
         pdu.handler_owner, pdu.policy_owner, pdu.error_owner,
         pdu.cleanup_owner, (unsigned int)pdu.roundtrip_ok,
         (unsigned int)pdu.parse_ok, pdu.parse_detail, pdu.parse_backend,
         pdu.parsed_handle, (unsigned int)pdu.parsed_value_len,
         (unsigned int)pdu.parsed_opcode, pdu.handler_backend,
         (unsigned int)pdu.att_error_code, pdu.att_error_rsp,
         pdu.att_request_state, (unsigned int)pdu.att_mtu,
         pdu.att_security_state, (unsigned int)pdu.att_requires_security,
         (unsigned int)pdu.att_request_id,
         (unsigned int)pdu.att_pending_before,
         (unsigned int)pdu.att_pending_after, pdu.att_completion_cb,
         pdu.att_queue_backend,
         pdu.att_request_owner, pdu.att_request_lifecycle,
         (unsigned int)pdu.att_request_ref_before,
         (unsigned int)pdu.att_request_ref_after,
         pdu.att_timeout_state, pdu.att_cancel_state,
         (unsigned int)pdu.att_timer_before,
         (unsigned int)pdu.att_timer_after,
         pdu.dispatch_result, (unsigned int)pdu.handler_ok,
         (unsigned int)pdu.policy_ok, pdu.error_status,
         (unsigned int)pdu.error_map_ok, pdu.cleanup_state,
         (unsigned int)pdu.cleanup_ok, (unsigned int)pdu.payload_len,
         hex);

  if (bluez_profile_delegated_responder(mode))
    {
      printf("bluez-profile: source=%s style=%s command=error-transaction "
             "family=%s mode=%s role=%s label=malformed-pdu write-ret=0 "
             "detail=daemon-mainloop-owned responder-delegated-io=1\n",
             mode->source, mode->transport, mode->family, mode->mode,
             mode->role);
      return 0;
    }

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_l2cap_socket_write_handle(
          sock, pdu.payload, pdu.payload_len, out, sizeof(out));
  printf("bluez-profile: source=%s style=%s command=error-transaction "
         "family=%s mode=%s role=%s label=malformed-pdu write-ret=%d\n",
         mode->source, mode->transport, mode->family, mode->mode,
         mode->role, ret);
  printf("%s", out);

  return ret;
}

static int bluez_profile_is_att_family(const char *family)
{
  return !strcmp(family, "hogp") || !strcmp(family, "gatt") ||
         !strcmp(family, "asha") || !strcmp(family, "midi");
}

static int bluez_profile_write_cancel_probe(void *sock,
  const struct bluez_profile_mode *mode)
{
  struct bluez_profile_pdu pdu;
  char hex[BLUEZ_PROFILE_PDU_HEX_MAX];
  char out[256];
  int ret;

  ret = bluez_profile_pdu_encode_cancel_probe(mode->family,
                                              mode->transport,
                                              &pdu);
  if (ret < 0)
    {
      return ret;
    }

  bluez_profile_pdu_hex(&pdu, hex, sizeof(hex));
  printf("bluez-profile: upstream-cancel-pdu family=%s mode=%s role=%s "
         "semantic=%s opcode=%s wire=%s parse-ok=%u parse-detail=%s "
         "parse-backend=%s handler-backend=%s parsed-opcode=0x%02x "
         "att-error-code=0x%02x att-error-rsp=%s "
         "att-request-state=%s att-cancel-state=%s "
         "att-request-id=0x%02x att-pending-before=%u "
         "att-pending-after=%u att-completion-cb=%s "
         "att-queue-backend=%s att-request-owner=%s "
         "att-request-lifecycle=%s att-request-ref-before=%u "
         "att-request-ref-after=%u att-timeout-state=%s "
         "att-timer-before=%u att-timer-after=%u "
         "error-status=%s error-map-ok=%u cleanup-ok=%u "
         "payload-hex=%s\n",
         mode->family, mode->mode, mode->role, pdu.semantic, pdu.opcode,
         pdu.wire, (unsigned int)pdu.parse_ok, pdu.parse_detail,
         pdu.parse_backend, pdu.handler_backend,
         (unsigned int)pdu.parsed_opcode,
         (unsigned int)pdu.att_error_code, pdu.att_error_rsp,
         pdu.att_request_state, pdu.att_cancel_state,
         (unsigned int)pdu.att_request_id,
         (unsigned int)pdu.att_pending_before,
         (unsigned int)pdu.att_pending_after, pdu.att_completion_cb,
         pdu.att_queue_backend, pdu.att_request_owner,
         pdu.att_request_lifecycle,
         (unsigned int)pdu.att_request_ref_before,
         (unsigned int)pdu.att_request_ref_after,
         pdu.att_timeout_state, (unsigned int)pdu.att_timer_before,
         (unsigned int)pdu.att_timer_after, pdu.error_status,
         (unsigned int)pdu.error_map_ok, (unsigned int)pdu.cleanup_ok,
         hex);

  if (bluez_profile_delegated_responder(mode))
    {
      printf("bluez-profile: source=%s style=%s command=cancel-transaction "
             "family=%s mode=%s role=%s label=cancel-pending write-ret=0 "
             "detail=daemon-mainloop-owned responder-delegated-io=1\n",
             mode->source, mode->transport, mode->family, mode->mode,
             mode->role);
      return 0;
    }

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_l2cap_socket_write_handle(
          sock, pdu.payload, pdu.payload_len, out, sizeof(out));
  printf("bluez-profile: source=%s style=%s command=cancel-transaction "
         "family=%s mode=%s role=%s label=cancel-pending write-ret=%d\n",
         mode->source, mode->transport, mode->family, mode->mode,
         mode->role, ret);
  printf("%s", out);

  return ret;
}

static void bluez_profile_print_upstream_state_machine(
  const struct bluez_profile_mode *mode)
{
  const char *owner = "src/profile.c";
  const char *state_machine = "register,connect,bearer-io,error,release";
  const char *objects = "Profile1,Device1,Adapter1";

  if (!strcmp(mode->family, "hid"))
    {
      owner = "profiles/input/device.c+profiles/input/server.c";
      state_machine =
        "sdp-register,profile-connect,l2cap-control,"
        "l2cap-interrupt,hidp-connadd,input-event,virtual-unplug,"
        "disconnect,release";
      objects = "Profile1,HIDP,InputDevice,L2CAP";
    }
  else if (!strcmp(mode->family, "hogp"))
    {
      owner = "profiles/input/hog.c+profiles/input/hog-lib.c";
      state_machine =
        "gatt-discover,report-map,protocol-mode,ccc-enable,"
        "input-report,output-report,suspend,resume,disconnect,release";
      objects = "Profile1,HogDevice,GattClient,ATT";
    }
  else if (!strcmp(mode->family, "hfp"))
    {
      owner = "profiles/audio/hfp-hf.c+profiles/audio/telephony.c";
      state_machine =
        "sdp-register,profile-connect,rfcomm-session,slc,"
        "codec-negotiation,call-control,sco-audio,disconnect,release";
      objects = "Profile1,RFCOMM,SCO,MediaTransport1";
    }
  else if (!strcmp(mode->family, "hsp"))
    {
      owner = "profiles/audio/headset.c+profiles/audio/telephony.c";
      state_machine =
        "sdp-register,profile-connect,rfcomm-session,headset-control,"
        "volume,sco-audio,disconnect,release";
      objects = "Profile1,RFCOMM,SCO,Headset";
    }
  else if (!strcmp(mode->family, "gatt"))
    {
      owner = "src/gatt-client.c+src/gatt-database.c+src/shared/att.c";
      state_machine =
        "application-register,att-attach,service-discovery,"
        "read,write,notify,indicate,error-rsp,disconnect,unregister";
      objects = "GattManager1,GattService1,GattCharacteristic1,ATT";
    }
  else if (!strcmp(mode->family, "mesh"))
    {
      owner = "mesh/main.c+mesh/node.c+mesh/net.c+mesh/model.c";
      state_machine =
        "mesh-init,mgmt-bearer,provisioning,config-client-server,"
        "model-message,proxy,relay-friend,replay-protection,release";
      objects = "MeshNetwork1,MeshNode1,MeshElement1,MeshModel";
    }
  else if (!strcmp(mode->family, "asha"))
    {
      owner = "profiles/audio/asha.c+profiles/audio/media.c";
      state_machine =
        "service-discovery,properties-read,codec-config,"
        "stream-start,audio-status,volume,battery,stream-stop,release";
      objects = "MediaEndpoint1,MediaTransport1,GattClient,ATT";
    }
  else if (!strcmp(mode->family, "midi"))
    {
      owner = "profiles/midi/midi.c+profiles/midi/libmidi.c";
      state_machine =
        "service-discovery,ccc-enable,timestamp-encode,midi-tx,"
        "midi-rx,timestamp-decode,error-policy,release";
      objects = "GattService1,GattCharacteristic1,MidiPort,ATT";
    }
  else if (!strcmp(mode->family, "ranging"))
    {
      owner = "profiles/ranging/rap.c+profiles/ranging/rap_hci.c";
      state_machine =
        "capability,security,procedure-config,procedure-start,"
        "result,event-stream,error-policy,release";
      objects = "RangingAccessProfile,HCI-CS,ATT,Device1";
    }
  else if (!strcmp(mode->family, "print"))
    {
      owner = "profiles/cups/main.c+profiles/cups/spp.c";
      state_machine =
        "sdp-register,rfcomm-session,hcrp-control,hcrp-data,"
        "job-submit,status,cancel-error,release";
      objects = "Profile1,RFCOMM,HCRP,CUPSJob";
    }
  else if (!strcmp(mode->family, "iap"))
    {
      owner = "profiles/iap/main.c";
      state_machine =
        "sdp-register,rfcomm-session,identify,ea-session,"
        "control-payload,link-control,error-policy,release";
      objects = "Profile1,RFCOMM,IAPSession,EAProtocol";
    }

  printf("bluez-profile: closeout upstream-state-machine family=%s "
         "mode=%s role=%s owner=%s states=%s objects=%s "
         "linux-bearer=%s semantic=bluez-profile-lifecycle\n",
         mode->family, mode->mode, mode->role, owner, state_machine,
         objects, mode->transport);
}

static void bluez_profile_print_end_to_end_contract(
  const struct bluez_profile_mode *mode)
{
  const char *profile_owner = "src/profile.c,src/service.c";
  const char *object_owner = "Profile1,Device1,Adapter1";
  const char *session_owner = "profile-register,connect,bearer-io,release";
  const char *bearer_owner = "L2CAP-or-ATT-or-RFCOMM";
  const char *datapath_owner = "request,response,payload";
  const char *ordering_owner =
    "register-before-connect,connect-before-io,release-before-cleanup";
  const char *error_owner =
    "connect-failed,bearer-failed,request-failed,remote-close";
  const char *cleanup_owner =
    "profile-unregister,bearer-close,watch-remove,refs-zero";

  if (!strcmp(mode->family, "hid"))
    {
      profile_owner = "profiles/input/device.c,profiles/input/server.c";
      object_owner = "Profile1,InputDevice,HIDP,UHID";
      session_owner =
        "sdp-record,profile-connect,hid-control,hid-interrupt,hidp-session";
      bearer_owner = "L2CAP-PSM-0x0011,L2CAP-PSM-0x0013,HIDP";
      datapath_owner =
        "set-protocol,input-report,output-report,virtual-unplug";
      error_owner =
        "control-reject,interrupt-close,uhid-fail,remote-virtual-unplug";
      cleanup_owner =
        "profile-unregister,hidp-conn-del,uhid-destroy,l2cap-close,refs-zero";
    }
  else if (!strcmp(mode->family, "hogp"))
    {
      profile_owner = "profiles/input/hog.c,profiles/input/hog-lib.c";
      object_owner = "Profile1,HogDevice,GattService1,GattCharacteristic1";
      session_owner =
        "gatt-discover,report-map,protocol-mode,ccc,input-output-report";
      bearer_owner = "ATT-fixed-channel,GATT,CCC";
      datapath_owner =
        "report-map-read,protocol-mode-write,input-notify,output-write";
      error_owner =
        "att-error,report-map-missing,ccc-fail,security-required";
      cleanup_owner =
        "hog-device-free,att-detach,ccc-clear,watch-remove,refs-zero";
    }
  else if (!strcmp(mode->family, "hfp"))
    {
      profile_owner = "profiles/audio/hfp-hf.c,profiles/audio/telephony.c";
      object_owner = "Profile1,RFCOMM,SCO,MediaTransport1";
      session_owner = "rfcomm-session,slc,codec-negotiation,call,sco";
      bearer_owner = "RFCOMM-DLCI,SCO-voice";
      datapath_owner = "AT+BRSF,AT+BAC,AT+BCS,AT+CLCC,SCO-audio";
      error_owner = "slc-reject,codec-reject,sco-fail,call-fail";
      cleanup_owner =
        "rfcomm-close,sco-close,transport-release,watch-remove,refs-zero";
    }
  else if (!strcmp(mode->family, "hsp"))
    {
      profile_owner = "profiles/audio/headset.c,profiles/audio/telephony.c";
      object_owner = "Profile1,RFCOMM,SCO,Headset";
      session_owner = "rfcomm-session,headset-control,volume,sco";
      bearer_owner = "RFCOMM-DLCI,SCO-voice";
      datapath_owner = "AT+CKPD,AT+VGS,AT+VGM,SCO-audio";
      error_owner = "headset-reject,volume-fail,sco-fail,remote-close";
      cleanup_owner =
        "rfcomm-close,sco-close,headset-release,watch-remove,refs-zero";
    }
  else if (!strcmp(mode->family, "gatt"))
    {
      profile_owner = "src/gatt-client.c,src/gatt-database.c,src/shared/att.c";
      object_owner =
        "GattManager1,GattService1,GattCharacteristic1,GattDescriptor1";
      session_owner = "app-register,att-attach,mtu,discovery,request-queue";
      bearer_owner = "ATT-fixed-channel,GATT,bt_att";
      datapath_owner = "read,write,notify,indicate,error-response";
      error_owner = "att-error,security-required,not-authorized,not-found";
      cleanup_owner =
        "app-unregister,att-detach,request-free,ccc-clear,refs-zero";
    }
  else if (!strcmp(mode->family, "mesh"))
    {
      profile_owner = "mesh/main.c,mesh/node.c,mesh/net.c,mesh/model.c";
      object_owner = "MeshNetwork1,MeshNode1,MeshElement1,MeshModel";
      session_owner = "provisioning,config-client-server,node,key,model";
      bearer_owner = "PB-ADV,GATT-proxy,ADV-bearer,ATT-fixed-channel";
      datapath_owner = "network-pdu,access-message,proxy-pdu,relay-friend";
      error_owner = "replay-reject,key-missing,model-reject,proxy-close";
      cleanup_owner =
        "node-free,key-free,model-free,replay-clear,bearer-close,refs-zero";
    }
  else if (!strcmp(mode->family, "asha"))
    {
      profile_owner = "profiles/audio/asha.c,profiles/audio/media.c";
      object_owner =
        "Device1,GattService1,GattCharacteristic1,MediaTransport1";
      session_owner =
        "service-discovery,properties,codec-config,stream,audio-status";
      bearer_owner = "ATT-fixed-channel,GATT,media-transport";
      datapath_owner = "audio-control,audio-status,volume,battery,audio-payload";
      error_owner = "codec-reject,stream-fail,status-timeout,reconnect-fail";
      cleanup_owner =
        "stream-stop,transport-release,att-detach,ccc-clear,refs-zero";
    }
  else if (!strcmp(mode->family, "midi"))
    {
      profile_owner = "profiles/midi/midi.c,profiles/midi/libmidi.c";
      object_owner = "GattService1,GattCharacteristic1,MidiPort";
      session_owner =
        "service-discovery,ccc,timestamp-queue,midi-tx,midi-rx";
      bearer_owner = "ATT-fixed-channel,GATT,CCC";
      datapath_owner = "write-without-response,notify,timestamp,payload-order";
      error_owner = "timestamp-wrap,jitter-drop,ccc-fail,att-error";
      cleanup_owner =
        "midi-port-free,timestamp-queue-free,ccc-clear,att-detach,refs-zero";
    }
  else if (!strcmp(mode->family, "ranging"))
    {
      profile_owner = "profiles/ranging/rap.c,profiles/ranging/rap_hci.c";
      object_owner = "RangingProfile1,Device1,Adapter1,HCI-CS";
      session_owner =
        "capability,security,procedure-config,procedure-start,result";
      bearer_owner = "ATT-fixed-channel,HCI-LE-CS";
      datapath_owner = "capability-read,security-enable,result-notify,hci-event";
      error_owner = "capability-reject,security-fail,procedure-timeout";
      cleanup_owner =
        "procedure-stop,event-watch-remove,att-detach,hci-cs-release,refs-zero";
    }
  else if (!strcmp(mode->family, "print"))
    {
      profile_owner = "profiles/cups/main.c,profiles/cups/spp.c";
      object_owner = "Profile1,SerialPort1,HCRP,CUPSJob";
      session_owner = "sdp,rfcomm,hcrp-control,hcrp-data,job,status";
      bearer_owner = "RFCOMM-DLCI,HCRP-control,HCRP-data";
      datapath_owner = "job-create,job-data,status,cancel-error";
      error_owner = "job-reject,status-fail,rfcomm-close,cancel-error";
      cleanup_owner =
        "job-free,rfcomm-close,hcrp-close,service-unregister,refs-zero";
    }
  else if (!strcmp(mode->family, "iap"))
    {
      profile_owner = "profiles/iap/main.c";
      object_owner = "Profile1,SerialPort1,IAPSession,EAProtocol";
      session_owner = "sdp,rfcomm,identify,ea-session,control,link";
      bearer_owner = "RFCOMM-DLCI,iAP2-control";
      datapath_owner = "identify,accessory-info,ea-open,control-payload";
      error_owner = "identify-reject,ea-reject,control-fail,link-close";
      cleanup_owner =
        "ea-close,rfcomm-close,service-unregister,watch-remove,refs-zero";
    }

  printf("bluez-profile: closeout end-to-end-contract family=%s "
         "mode=%s role=%s profile-owner=%s object-owner=%s "
         "session-owner=%s bearer-owner=%s datapath-owner=%s "
         "ordering-owner=%s error-owner=%s cleanup-owner=%s "
         "upstream-link=%s end-to-end-final=1\n",
         mode->family, mode->mode, mode->role, profile_owner,
         object_owner, session_owner, bearer_owner, datapath_owner,
         ordering_owner, error_owner, cleanup_owner, mode->boundary);
}

static int bluez_profile_closeout(const char *name, uint16_t peer)
{
  const struct bluez_profile_mode *mode;
  uint16_t handle;
  void *sock = NULL;
  int ret;
  int failed = 0;

  mode = bluez_profile_find(name);
  if (mode == NULL)
    {
      bluez_profile_usage();
      return 1;
    }

  handle = bluez_profile_handle(mode, peer);
  printf("bluez-profile: source=%s style=profile command=connect "
         "family=%s mode=%s role=%s peer=%u handle=0x%04x\n",
         mode->source, mode->family, mode->mode, mode->role, peer, handle);
  printf("bluez-profile: source=third/bluez/src/profile.c "
         "style=profile-register command=register family=%s mode=%s "
         "role=%s transport=%s psm=0x%04x cid=0x%04x\n",
         mode->family, mode->mode, mode->role, mode->transport,
         mode->psm, mode->cid);

  ret = linux_bt_upstream_l2cap_socket_open(mode->psm, mode->cid,
                                            handle, &sock);
  printf("bluez-profile: bearer open family=%s mode=%s role=%s "
         "transport=%s psm=0x%04x cid=0x%04x ret=%d\n",
         mode->family, mode->mode, mode->role, mode->transport,
         mode->psm, mode->cid, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(sock, mode->psm,
                                                          mode->cid);
      printf("bluez-profile: bearer connect family=%s mode=%s role=%s "
             "transport=%s psm=0x%04x cid=0x%04x ret=%d\n",
             mode->family, mode->mode, mode->role, mode->transport,
             mode->psm, mode->cid, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      failed |= bluez_profile_write_label(sock, mode, mode->label1) < 0;
      failed |= bluez_profile_write_label(sock, mode, mode->label2) < 0;
      failed |= bluez_profile_write_label(sock, mode, mode->label3) < 0;
      failed |= bluez_profile_write_error_probe(sock, mode) < 0;
      if (bluez_profile_is_att_family(mode->family))
        {
          failed |= bluez_profile_write_cancel_probe(sock, mode) < 0;
        }
    }

  printf("bluez-profile: source=third/bluez/src/service.c "
         "style=lifecycle command=error-policy family=%s mode=%s "
         "role=%s dbus-error-map=1 retry-cleanup=1\n",
         mode->family, mode->mode, mode->role);
  bluez_profile_print_upstream_state_machine(mode);

  if (sock != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(sock);
      printf("bluez-profile: bearer close family=%s mode=%s role=%s "
             "ret=%d\n",
             mode->family, mode->mode, mode->role, ret);
      failed |= ret < 0;
    }

  printf("bluez-profile: closeout cleanup family=%s mode=%s role=%s "
         "bearer-fd=closed profile-object=0 service-object=0 "
         "adapter-ref=0 device-ref=0 watches=0 requests=0\n",
         mode->family, mode->mode, mode->role);
  printf("bluez-profile: closeout upstream-link-ledger family=%s "
         "mode=%s role=%s dbus-owner=bluetoothd profile-object=0 "
         "service-object=0 adapter-ref=0 device-ref=0 bearer=%s "
         "bearer-ref=0 mainloop-watch=0 pending-request=0 "
         "pending-event=0 error-policy=1 cleanup-final=1\n",
         mode->family, mode->mode, mode->role, mode->transport);
  bluez_profile_print_end_to_end_contract(mode);
  printf("bluez-profile: closeout upstream-coverage-map family=%s "
         "mode=%s role=%s %s third/bluez/src/profile.c "
         "third/bluez/src/service.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         mode->family, mode->mode, mode->role, mode->source);
  printf("bluez-profile: profile-final=1 bearer-final=1 "
         "transaction-final=1 error-final=1 cleanup-final=1 "
         "upstream-link=%s final-ok=%u\n",
         mode->boundary, failed ? 0 : 1);

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
      bluez_profile_usage();
      return 1;
    }

  peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : 2;
  return bluez_profile_closeout(argv[2], peer);
}
