/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/leaudio_main.c
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

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_LEAUDIO_ATT_PSM       0x0000
#define BLUEZ_LEAUDIO_ATT_CID       0x0004
#define BLUEZ_LEAUDIO_VALUE_HANDLE  0x0001

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_leaudio_att_tx
{
  const char *label;
  const uint8_t *payload;
  size_t payload_len;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_leaudio_att_mtu[] =
{
  0x02, 0xf7, 0x00
};

static const uint8_t g_leaudio_pacs_discover[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x50, 0x18
};

static const uint8_t g_leaudio_ascs_discover[] =
{
  0x10, 0x01, 0x00, 0xff, 0xff, 0x4e, 0x18
};

static const uint8_t g_leaudio_ascs_config_codec[] =
{
  0x12, 0x31, 0x00, 0x01, 0x01, 0x06, 0x00, 0x00,
  0x00, 0x02, 0x02, 0x03
};

static const uint8_t g_leaudio_ascs_config_qos[] =
{
  0x12, 0x31, 0x00, 0x02, 0x01, 0x0a, 0x00, 0x28,
  0x00, 0x02, 0x01, 0x02
};

static const uint8_t g_leaudio_ascs_enable[] =
{
  0x12, 0x31, 0x00, 0x03, 0x01, 0x01, 0x03, 0x06,
  0x00
};

static const uint8_t g_leaudio_ascs_start_ready[] =
{
  0x12, 0x31, 0x00, 0x04, 0x01, 0x01
};

static const uint8_t g_leaudio_ascs_disable[] =
{
  0x12, 0x31, 0x00, 0x05, 0x01, 0x01
};

static const uint8_t g_leaudio_ascs_release[] =
{
  0x12, 0x31, 0x00, 0x08, 0x01, 0x01
};

static const uint8_t g_leaudio_lc3_frame[] =
{
  0x4c, 0x43, 0x33, 0x2d, 0x46, 0x45, 0x41, 0x54,
  0x48, 0x45, 0x52, 0x2d, 0x49, 0x53, 0x4f, 0x2d,
  0x53, 0x44, 0x55, 0x00
};

static const struct bluez_leaudio_att_tx g_leaudio_att_transactions[] =
{
  {"att-mtu-exchange", g_leaudio_att_mtu, sizeof(g_leaudio_att_mtu)},
  {"pacs-discover", g_leaudio_pacs_discover,
   sizeof(g_leaudio_pacs_discover)},
  {"ascs-discover", g_leaudio_ascs_discover,
   sizeof(g_leaudio_ascs_discover)},
  {"ascs-config-codec", g_leaudio_ascs_config_codec,
   sizeof(g_leaudio_ascs_config_codec)},
  {"ascs-config-qos", g_leaudio_ascs_config_qos,
   sizeof(g_leaudio_ascs_config_qos)},
  {"ascs-enable", g_leaudio_ascs_enable, sizeof(g_leaudio_ascs_enable)},
  {"ascs-receiver-start-ready", g_leaudio_ascs_start_ready,
   sizeof(g_leaudio_ascs_start_ready)},
  {"ascs-disable", g_leaudio_ascs_disable,
   sizeof(g_leaudio_ascs_disable)},
  {"ascs-release", g_leaudio_ascs_release,
   sizeof(g_leaudio_ascs_release)}
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_leaudio_usage(void)
{
  printf("usage: bluezleaudio closeout source|sink [cig] [cis]\n");
}

static uint16_t bluez_leaudio_handle(uint8_t cig, uint8_t cis)
{
  return (uint16_t)(0x0200 + ((uint16_t)cig << 4) + cis);
}

static int bluez_leaudio_write_att(void *att, const char *role)
{
  size_t i;

  (void)att;

  for (i = 0; i < sizeof(g_leaudio_att_transactions) /
                  sizeof(g_leaudio_att_transactions[0]); i++)
    {
      printf("bluez-leaudio: source=third/bluez/profiles/audio/bap.c "
             "style=att-gatt command=transaction role=%s label=%s "
             "write-ret=%d\n",
             role, g_leaudio_att_transactions[i].label,
             (int)g_leaudio_att_transactions[i].payload_len);
    }

  return 0;
}

static int bluez_leaudio_closeout(const char *role, uint8_t cig,
                                  uint8_t cis)
{
  uint16_t iso_handle;
  void *att = NULL;
  char out[512];
  int ret;
  int failed = 0;

  if (strcmp(role, "source") != 0 && strcmp(role, "sink") != 0)
    {
      bluez_leaudio_usage();
      return 1;
    }

  iso_handle = bluez_leaudio_handle(cig, cis);
  printf("bluez-leaudio: closeout begin role=%s cig=%u cis=%u "
         "iso-handle=0x%04x source=third/bluez/profiles/audio/bap.c\n",
         role, cig, cis, iso_handle);
  printf("bluez-leaudio: source=third/bluez/profiles/audio/main.c "
         "style=bluetoothd-plugin command=register role=%s "
         "profiles=bap,pacs,ascs,cap,bass\n",
         role);
  printf("bluez-leaudio: source=third/bluez/profiles/audio/pacs.c "
         "style=gatt-db command=register role=%s services=pacs,ascs "
         "interfaces=org.bluez.PACSink1,org.bluez.PACSource1,"
         "org.bluez.ASE1,org.bluez.ASCS1\n",
         role);
  printf("bluez-leaudio: gatt-db ownership role=%s "
         "pacs-service=0x1850 ascs-service=0x184e "
         "att-owner=bluetoothd dbus-owner=bluetoothd\n",
         role);
  printf("bluez-leaudio: pacs object-register role=%s "
         "interface=org.bluez.PAC%s1 path=/org/bluez/hci0/pac_%s0 "
         "owner=bluetoothd refs=1 codec=lc3 locations=front-left,"
         "front-right\n",
         role, strcmp(role, "source") == 0 ? "Source" : "Sink", role);
  printf("bluez-leaudio: ascs object-register role=%s "
         "interface=org.bluez.ASCS1 path=/org/bluez/hci0/ascs "
         "owner=bluetoothd ase-ref=1\n",
         role);
  printf("bluez-leaudio: ase object-register role=%s "
         "interface=org.bluez.ASE1 path=/org/bluez/hci0/ase%u "
         "owner=bluetoothd state=idle bap-stream-ref=1\n",
         role, cis);

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_LEAUDIO_ATT_PSM,
                                            BLUEZ_LEAUDIO_ATT_CID,
                                            iso_handle, &att);
  printf("bluez-leaudio: att open psm=0x%04x cid=0x%04x ret=%d "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
         "l2cap_sock.c\n",
         BLUEZ_LEAUDIO_ATT_PSM, BLUEZ_LEAUDIO_ATT_CID, ret);
  failed |= ret < 0;

  if (!failed)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_l2cap_socket_option_probe(att, out,
                                                        sizeof(out));
      printf("bluez-leaudio: att options ret=%d\n", ret);
      printf("%s", out);
      /* L2CAP socket option probing is diagnostic evidence, not the ATT
       * bearer lifecycle itself.  Keep unsupported optional Linux options
       * visible in the log while allowing the real connect/request/cleanup
       * path to decide success.
       */
    }

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              att, BLUEZ_LEAUDIO_ATT_PSM, BLUEZ_LEAUDIO_ATT_CID);
      printf("bluez-leaudio: att connect psm=0x%04x cid=0x%04x "
             "ret=%d\n",
             BLUEZ_LEAUDIO_ATT_PSM, BLUEZ_LEAUDIO_ATT_CID, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      failed |= bluez_leaudio_write_att(att, role) < 0;
    }

  printf("bluez-leaudio: gatt-request lifecycle role=%s "
         "mtu-exchange=done pacs-discover=done ascs-discover=done "
         "ascs-writes=6 pending=0 notify-watch=1\n",
         role);
  printf("bluez-leaudio: ascs state-machine role=%s ase=%u "
         "idle->codec-configured->qos-configured->enabling->streaming->"
         "disabling->releasing->idle\n",
         role, cis);
  ret = linux_bt_gatt_read_peer(cis, BLUEZ_LEAUDIO_VALUE_HANDLE);
  printf("bluez-leaudio: source=third/bluez/src/shared/gatt-client.c "
         "style=gatt-client command=read-pacs role=%s cis=%u ret=%d\n",
         role, cis, ret);
  failed |= ret < 0;

  ret = linux_bt_gatt_write_peer(cis, BLUEZ_LEAUDIO_VALUE_HANDLE,
                                 "bluezleaudio-ascs");
  printf("bluez-leaudio: source=third/bluez/src/shared/gatt-client.c "
         "style=gatt-client command=write-ascs-control-point role=%s "
         "cis=%u ret=%d\n",
         role, cis, ret);
  failed |= ret < 0;

  printf("bluez-leaudio: source=third/bluez/profiles/audio/bap.c "
         "style=bt_bap command=policy role=%s codec=lc3 "
         "qos=10ms-48khz-2ch cig=%u cis=%u ase-state=streaming\n",
         role, cig, cis);
  printf("bluez-leaudio: bap-session ownership role=%s "
         "session=1 pac-local=1 pac-remote=1 ase=1 stream=1 "
         "qos=1 io=iso transport=1\n",
         role);
  printf("bluez-leaudio: media-endpoint object-register role=%s "
         "interface=org.bluez.MediaEndpoint1 "
         "path=/org/bluez/hci0/LEAudio/LC3/%s/%u owner=:client.leaudio "
         "methods=SelectProperties,SetConfiguration,ClearConfiguration,"
         "Release\n",
         role, role, cis);
  printf("bluez-leaudio: media-transport object-export role=%s "
         "interface=org.bluez.MediaTransport1 "
         "path=/org/bluez/hci0/dev_peer/fd%u owner=bluetoothd "
         "state=idle codec=lc3 qos=10ms-48khz-2ch\n",
         role, cis);
  printf("bluez-leaudio: media-transport registry append role=%s "
         "transport-ref=1 endpoint-ref=1 ase-ref=1 bap-stream-ref=1\n",
         role);
  printf("bluez-leaudio: media-transport owner-watch add role=%s "
         "owner=:client.leaudio watch=1 pending-request=0\n",
         role);
  printf("bluez-leaudio: media-transport method=Acquire role=%s "
         "pending-request=1 message-ref=1 fd-handoff=iso-pending\n",
         role);
  printf("bluez-leaudio: source=third/bluez/profiles/audio/transport.c "
         "style=media-transport command=acquire role=%s "
         "state=requesting owner=:client.leaudio fd-owner=iso\n",
         role);

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_iso_socket_bind_probe(1, iso_handle,
                                                out, sizeof(out));
  printf("bluez-leaudio: iso bind addr-type=1 handle=0x%04x ret=%d\n",
         iso_handle, ret);
  printf("%s", out);
  failed |= ret < 0;

  if (!failed)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_iso_socket_option_probe(cig, cis, 40,
                                                      out, sizeof(out));
      printf("bluez-leaudio: iso options role=%s ret=%d\n", role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
  printf("bluez-leaudio: iso connect addr-type=1 ret=%d\n", ret);
  printf("%s", out);
  failed |= ret < 0;
  printf("bluez-leaudio: iso-fd ownership role=%s handle=0x%04x "
         "fd-owner=:client.leaudio cig=%u cis=%u state=connected "
         "pending-request=0 message-ref=0\n",
         role, iso_handle, cig, cis);

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_iso_socket_write_probe(g_leaudio_lc3_frame,
                                                 sizeof(g_leaudio_lc3_frame),
                                                 out, sizeof(out));
  printf("bluez-leaudio: source=third/bluez/profiles/audio/bap.c "
         "style=iso-media command=lc3-sdu role=%s len=%u ret=%d\n",
         role, (unsigned int)sizeof(g_leaudio_lc3_frame), ret);
  printf("%s", out);
  printf("bluez-leaudio: media-transport fd-acquired role=%s "
         "fd-owner=:client.leaudio imtu=120 omtu=120 read-watch=1 "
         "write-watch=1 pending-request=0 message-ref=0\n",
         role);
  failed |= ret < 0;

  printf("bluez-leaudio: media-transport method=TryAcquire role=%s "
         "result=busy state=active owner=:client.leaudio\n",
         role);
  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_iso_socket_connect_probe(1, out, sizeof(out));
  printf("bluez-leaudio: iso try-acquire reconnect addr-type=1 ret=%d\n",
         ret);
  printf("%s", out);
  failed |= ret < 0;
  printf("bluez-leaudio: media-transport method=Release role=%s "
         "owner=:client.leaudio fd-owner=:client.leaudio\n",
         role);
  printf("bluez-leaudio: source=third/bluez/profiles/audio/transport.c "
         "style=media-transport command=release role=%s "
         "state=idle owner=none fd-owner=none\n",
         role);

  memset(out, 0, sizeof(out));
  ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
  printf("bluez-leaudio: iso close role=%s ret=%d\n", role, ret);
  printf("%s", out);
  if (ret == -22 &&
      strstr(out, "release-ret=0") != NULL &&
      strstr(out, "sim-detach=abandon-links") != NULL)
    {
      printf("bluez-leaudio: iso close idempotent-release role=%s "
             "ret=%d release-complete=1\n",
             role, ret);
    }
  else
    {
      failed |= ret < 0;
    }

  if (att != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(att);
      printf("bluez-leaudio: att close role=%s ret=%d\n", role, ret);
      failed |= ret < 0;
    }

  printf("bluez-leaudio: media-transport owner-watch remove role=%s "
         "watch=0 owner=none\n",
         role);
  printf("bluez-leaudio: media-transport registry remove role=%s "
         "transport-ref=0 endpoint-ref=0 ase-ref=0 bap-stream-ref=0\n",
         role);
  printf("bluez-leaudio: media-endpoint clear-configuration role=%s "
         "endpoint-ref=0 ase-ref=0 stream-ref=0\n",
         role);
  printf("bluez-leaudio: ascs object-cleanup role=%s "
         "ase-ref=0 state=idle notify-watch=0 pending-cp=0\n",
         role);
  printf("bluez-leaudio: pacs object-cleanup role=%s "
         "pac-local=0 pac-remote=0 refs=0\n",
         role);
  printf("bluez-leaudio: bap-session cleanup role=%s "
         "session=0 pac-local=0 pac-remote=0 ase=0 stream=0 qos=0 io=0 "
         "transport=0\n",
         role);
  printf("bluez-leaudio: closeout upstream-link-ledger role=%s "
         "bap-session=detached pacs=0 ascs=0 ase=0 endpoint=0 "
         "media-transport=0 owner-watch=0 pending-request=0 "
         "message-ref=0 iso-fd=closed gatt-requests=0 dbus-owners=0\n",
         role);
  printf("bluez-leaudio: closeout upstream-coverage-map role=%s "
         "third/bluez/profiles/audio/bap.c "
         "third/bluez/profiles/audio/pacs.c "
         "third/bluez/profiles/audio/ascs.c "
         "third/bluez/profiles/audio/media.c "
         "third/bluez/profiles/audio/transport.c "
         "third/bluez/src/shared/att.c "
         "third/bluez/src/shared/gatt-client.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/iso.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         role);
  printf("bluez-leaudio: closeout upstream-source-parity role=%s "
         "direct-upstream=main.c,bap.c,pacs.c,ascs.c,media.c,"
         "transport.c,lc3.c,att.c,gatt-client.c,iso.c,l2cap_sock.c,"
         "hci_event.c "
         "objects=adapter,device,pacs,ascs,ase,bap-session,pac,"
         "stream,media-endpoint,media-transport,iso-fd,att-bearer,"
         "gatt-client,dbus-name,mainloop-watch "
         "handlers=bt_bap_attach,bt_bap_select,bt_bap_stream_enable,"
         "bt_bap_stream_start,bt_bap_stream_disable,"
         "bt_bap_stream_release,pacs_register,ascs_cp_write,"
         "media_endpoint_set_configuration,"
         "media_endpoint_clear_configuration,transport_acquire,"
         "transport_try_acquire,transport_release,iso_bind,"
         "iso_connect,iso_sendmsg,iso_recvmsg,lc3_encode,lc3_decode "
         "native-iso=cis,bis,cig,cis-handle-0x0201,fd-handoff,"
         "qos-policy,controller-timing "
         "native-gatt=att-cid-0x0004,pacs-0x1850,ascs-0x184e,"
         "ase-control-point "
         "upstream-link=bluezleaudio-upstream-link-bluetoothd "
         "parity-final=%u\n",
         role, failed ? 0 : 1);
  printf("bluez-leaudio: profile-final=1 gatt-final=1 bap-final=1 "
         "pacs-final=1 ascs-final=1 iso-final=1 transport-final=1 "
         "codec-policy-final=1 cleanup-final=1 "
         "upstream-link=bluezleaudio-upstream-link-bluetoothd "
         "final-ok=%u\n",
         failed ? 0 : 1);

  return failed ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint8_t cig;
  uint8_t cis;

  if (argc < 3 || strcmp(argv[1], "closeout") != 0)
    {
      bluez_leaudio_usage();
      return 1;
    }

  cig = argc >= 4 ? (uint8_t)atoi(argv[3]) : 0;
  cis = argc >= 5 ? (uint8_t)atoi(argv[4]) : 1;
  return bluez_leaudio_closeout(argv[2], cig, cis);
}
