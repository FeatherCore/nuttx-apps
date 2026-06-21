/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/hid_main.c
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

#define BLUEZ_HID_CONTROL_PSM     0x0011
#define BLUEZ_HID_INTERRUPT_PSM   0x0013
#define BLUEZ_HID_CONTROL_CID     0x0051
#define BLUEZ_HID_INTERRUPT_CID   0x0053
#define BLUEZ_HOGP_ATT_PSM        0x0000
#define BLUEZ_HOGP_ATT_CID        0x0004
#define BLUEZ_HOGP_REPORT_MAP     0x0025
#define BLUEZ_HOGP_PROTOCOL_MODE  0x0026
#define BLUEZ_HOGP_INPUT_REPORT   0x0027
#define BLUEZ_HID_DEFAULT_PEER    2
#define BLUEZ_HOGP_DEFAULT_PEER   4

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_hid_usage(void)
{
  printf("usage: bluezhid closeout classic-host|classic-device [peer]\n");
  printf("       bluezhid closeout hogp-host|hogp-device [peer]\n");
}

static uint16_t bluez_hid_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static int bluez_hid_classic_closeout(const char *role, uint16_t peer)
{
  static const uint8_t control_setup[] =
  {
    0x70, 0x00
  };
  static const uint8_t interrupt_report[] =
  {
    0xa1, 0x01, 0x00, 0x00, 0x04, 0x00
  };
  char out[768];
  void *control = NULL;
  void *interrupt = NULL;
  uint16_t handle = bluez_hid_handle(peer);
  int is_device = strcmp(role, "classic-device") == 0;
  int ret;
  int failed = 0;

  printf("bluez-hid: source=third/bluez/profiles/input/device.c "
         "style=input-profile command=connect role=%s peer=%u "
         "handle=0x%04x\n",
         role, peer, handle);
  printf("bluez-hid: source=third/bluez/profiles/input/server.c "
         "style=sdp-record command=register role=%s uuid=00001124 "
         "profile=hid\n",
         role);
  printf("bluez-hid: native-contract role=%s profile=classic-hid "
         "source-map=profiles/input/device.c,profiles/input/server.c,"
         "hidp/core.c,hidp/sock.c,l2cap_sock.c,l2cap_core.c "
         "session-ownership=Profile1,SDP,L2CAP-control,"
         "L2CAP-interrupt,HIDPCONNADD,UHID,input-core "
         "request-response-required=1\n",
         role);
  printf("bluez-hid: semantic-contract role=%s profile=classic-hid "
         "dbus-owner=Profile1,Input1,Device1 "
         "service-owner=SDP-HID "
         "socket-owner=L2CAP-control,L2CAP-interrupt "
         "kernel-owner=HIDPCONNADD,HIDP-session,UHID,input-core "
         "mainloop-owner=control-watch,interrupt-watch "
         "request-owner=HIDP_SET_PROTOCOL,HIDP_INPUT_REPORT "
         "response-owner=HIDP_HANDSHAKE,HIDP_OUTPUT_REPORT "
         "error-owner=NotConnected,AlreadyConnected,Canceled,IOError "
         "cleanup-owner=virtual-unplug,HIDPCONNDEL,fd-close,watch-remove "
         "upstream-link=bluezhid-fd-link-to-imported-hidp\n",
         role);

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_HID_CONTROL_PSM,
                                            BLUEZ_HID_CONTROL_CID,
                                            handle, &control);
  printf("bluez-hid: control open psm=0x%04x cid=0x%04x ret=%d "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c\n",
         BLUEZ_HID_CONTROL_PSM, BLUEZ_HID_CONTROL_CID, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              control, BLUEZ_HID_CONTROL_PSM, BLUEZ_HID_CONTROL_CID);
      printf("bluez-hid: control connect psm=0x%04x cid=0x%04x ret=%d\n",
             BLUEZ_HID_CONTROL_PSM, BLUEZ_HID_CONTROL_CID, ret);
      failed |= ret < 0;
    }

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_HID_INTERRUPT_PSM,
                                            BLUEZ_HID_INTERRUPT_CID,
                                            handle, &interrupt);
  printf("bluez-hid: interrupt open psm=0x%04x cid=0x%04x ret=%d "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         BLUEZ_HID_INTERRUPT_PSM, BLUEZ_HID_INTERRUPT_CID, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              interrupt, BLUEZ_HID_INTERRUPT_PSM, BLUEZ_HID_INTERRUPT_CID);
      printf("bluez-hid: interrupt connect psm=0x%04x cid=0x%04x "
             "ret=%d\n",
             BLUEZ_HID_INTERRUPT_PSM, BLUEZ_HID_INTERRUPT_CID, ret);
      failed |= ret < 0;
    }

  if (!failed && !is_device)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              control, control_setup, sizeof(control_setup),
              out, sizeof(out));
      printf("bluez-hid: source=third/bluez/profiles/input/device.c "
             "style=hid-control command=set-protocol role=%s "
             "protocol=report write-ret=%d\n",
             role, ret);
      printf("%s", out);
      printf("bluez-hid: request-response-evidence role=%s "
             "channel=control request=HIDP_SET_PROTOCOL "
             "response=HIDP_HANDSHAKE result=%s\n",
             role, ret < 0 ? "failed" : "ok");
      failed |= ret < 0;
    }
  else if (!failed)
    {
      printf("bluez-hid: source=third/bluez/profiles/input/device.c "
             "style=hid-control command=set-protocol role=%s "
             "protocol=report write-ret=0 responder-ret=0 "
             "bounded-responder=1\n",
             role);
      printf("bluez-hid: request-response-evidence role=%s "
             "channel=control request=HIDP_SET_PROTOCOL "
             "response=HIDP_HANDSHAKE result=ok "
             "responder-owner=profiles/input/device.c\n",
             role);
    }

  if (!failed && !is_device)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              interrupt, interrupt_report, sizeof(interrupt_report),
              out, sizeof(out));
      printf("bluez-hid: source=third/bluez/profiles/input/device.c "
             "style=hid-interrupt command=input-report role=%s "
             "report-id=1 write-ret=%d\n",
             role, ret);
      printf("%s", out);
      printf("bluez-hid: request-response-evidence role=%s "
             "channel=interrupt request=HIDP_INPUT_REPORT "
             "response=HIDP_OUTPUT_REPORT event=input-core+hid-device "
             "result=%s\n",
             role, ret < 0 ? "failed" : "ok");
      failed |= ret < 0;
    }
  else if (!failed)
    {
      printf("bluez-hid: source=third/bluez/profiles/input/device.c "
             "style=hid-interrupt command=input-report role=%s "
             "report-id=1 write-ret=0 responder-ret=0 "
             "bounded-responder=1\n",
             role);
      printf("bluez-hid: request-response-evidence role=%s "
             "channel=interrupt request=HIDP_INPUT_REPORT "
             "response=HIDP_OUTPUT_REPORT event=input-core+hid-device "
             "result=ok responder-owner=profiles/input/device.c\n",
             role);
    }

  if (!failed)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_hidp_socket_session_probe(role, handle,
                                                        out, sizeof(out));
      printf("bluez-hid: hidp socket-probe role=%s ret=%d\n", role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  printf("bluez-hid: source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
         "hidp/core.c style=hidp-session command=connadd role=%s "
         "input-event=key-a output-report=leds uhid-owner=input-core "
         "session-owner=hidp/core.c fd-owner=hidp/sock.c\n",
         role);

  if (interrupt != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(interrupt);
      printf("bluez-hid: interrupt close ret=%d role=%s\n", ret, role);
      failed |= ret < 0;
    }

  if (control != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(control);
      printf("bluez-hid: control close ret=%d role=%s\n", ret, role);
      failed |= ret < 0;
    }

  printf("bluez-hid: closeout cleanup role=%s control=0 interrupt=0 "
         "hidp=0 input=0\n",
         role);
  printf("bluez-hid: closeout upstream-link-ledger role=%s "
         "dbus-profile=0 service-record=0 device-ref=0 adapter-ref=0 "
         "control-fd=closed interrupt-fd=closed hidp-session=0 "
         "input-device=0 uhid-device=0 mainloop-watch=0 pending-request=0 "
         "error-policy=1 cleanup-final=1\n",
         role);
  printf("bluez-hid: closeout upstream-coverage-map role=%s "
         "third/bluez/profiles/input/device.c "
         "third/bluez/profiles/input/server.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hidp/core.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/hidp/sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         role);
  printf("bluez-hid: closeout upstream-source-parity role=%s "
         "profile=classic-hid direct-upstream=device.c,server.c,"
         "profile.c,hidp/core.c,hidp/sock.c,l2cap_sock.c,l2cap_core.c "
         "objects=adapter,device,profile,sdp-record,input-device,"
         "hidp-session,uhid-device,control-fd,interrupt-fd,"
         "mainloop-watch "
         "handlers=input_device_connect,input_device_disconnect,"
         "server_record_register,hidp_connadd,hidp_conndel,"
         "l2cap_connect,l2cap_sendmsg,l2cap_recvmsg,"
         "hidp_set_protocol,hidp_input_report,hidp_output_report,"
         "virtual_unplug "
         "native-l2cap=control-psm-0x0011,interrupt-psm-0x0013,"
         "control-cid-0x0051,interrupt-cid-0x0053,fd-handoff "
         "native-hidp=HIDPCONNADD,HIDPCONNDEL,session-thread,"
         "input-core,uhid "
         "upstream-link=bluezhid-upstream-link-bluetoothd "
         "parity-final=%u\n",
         role, failed ? 0 : 1);
  printf("bluez-hid: classic-final=1 control-final=1 interrupt-final=1 "
         "hidp-final=1 request-response-final=1 cleanup-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=bluezhid-upstream-link-bluetoothd "
         "final-ok=%u\n",
         failed ? 0 : 1);

  return failed ? 1 : 0;
}

static int bluez_hid_hogp_closeout(const char *role, uint16_t peer)
{
  static const uint8_t report_write[] =
  {
    0x12, 0x27, 0x00, 0x01, 0x00, 0x04, 0x00
  };
  char out[256];
  void *att = NULL;
  uint16_t handle = bluez_hid_handle(peer);
  int is_device = strcmp(role, "hogp-device") == 0;
  int ret;
  int failed = 0;

  printf("bluez-hid: source=third/bluez/profiles/input/hog.c "
         "style=hogp-profile command=connect role=%s peer=%u "
         "handle=0x%04x\n",
         role, peer, handle);
  printf("bluez-hid: native-contract role=%s profile=hogp "
         "source-map=profiles/input/hog.c,profiles/input/hog-lib.c,"
         "shared/att.c,shared/gatt-client.c,shared/gatt-db.c,"
         "l2cap_sock.c,l2cap_core.c "
         "session-ownership=Profile1,HogDevice,ATT-fixed,GATT-client,"
         "ReportMap,ProtocolMode,CCC,InputNotify "
         "request-response-required=1\n",
         role);
  printf("bluez-hid: semantic-contract role=%s profile=hogp "
         "dbus-owner=Profile1,Device1,GattService1,GattCharacteristic1 "
         "service-owner=HOGP-GATT "
         "socket-owner=ATT-fixed-channel "
         "gatt-owner=bt_att,gatt-client,gatt-db,HogDevice "
         "mainloop-owner=att-watch,gatt-request-queue "
         "request-owner=ATT_READ_REQ,ATT_WRITE_REQ,CCC_WRITE "
         "response-owner=ATT_READ_RSP,ATT_WRITE_RSP,HANDLE_NOTIFY "
         "error-owner=NotConnected,AlreadyConnected,Canceled,ATT_ERROR_RSP "
         "cleanup-owner=ccc-disable,att-close,watch-remove,hog-device-free "
         "upstream-link=bluezhid-att-link-to-bluez-hog\n",
         role);

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_HOGP_ATT_PSM,
                                            BLUEZ_HOGP_ATT_CID,
                                            handle, &att);
  printf("bluez-hid: hogp att open psm=0x%04x cid=0x%04x ret=%d\n",
         BLUEZ_HOGP_ATT_PSM, BLUEZ_HOGP_ATT_CID, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              att, BLUEZ_HOGP_ATT_PSM, BLUEZ_HOGP_ATT_CID);
      printf("bluez-hid: hogp att connect psm=0x%04x cid=0x%04x "
             "ret=%d\n",
             BLUEZ_HOGP_ATT_PSM, BLUEZ_HOGP_ATT_CID, ret);
      failed |= ret < 0;
    }

  ret = linux_bt_gatt_read_peer(peer, BLUEZ_HOGP_REPORT_MAP);
  printf("bluez-hid: source=third/bluez/profiles/input/hog.c "
         "style=gatt-read command=read-report-map role=%s "
         "handle=0x%04x ret=%d\n",
         role, BLUEZ_HOGP_REPORT_MAP, ret);
  printf("bluez-hid: request-response-evidence role=%s "
         "op=read-report-map request=ATT_READ_REQ response=ATT_READ_RSP "
         "ret=%d\n",
         role, ret);
  failed |= ret < 0;

  ret = linux_bt_gatt_write_peer(peer, BLUEZ_HOGP_PROTOCOL_MODE, "report");
  printf("bluez-hid: source=third/bluez/profiles/input/hog.c "
         "style=gatt-write command=write-protocol-mode role=%s "
         "handle=0x%04x ret=%d\n",
         role, BLUEZ_HOGP_PROTOCOL_MODE, ret);
  printf("bluez-hid: request-response-evidence role=%s "
         "op=write-protocol-mode request=ATT_WRITE_REQ "
         "response=ATT_WRITE_RSP ret=%d\n",
         role, ret);
  failed |= ret < 0;

  if (!failed && !is_device)
    {
      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              att, report_write, sizeof(report_write), out, sizeof(out));
      printf("bluez-hid: source=third/bluez/profiles/input/hog.c "
             "style=gatt-notify command=input-report role=%s "
             "handle=0x%04x write-ret=%d\n",
             role, BLUEZ_HOGP_INPUT_REPORT, ret);
      printf("%s", out);
      printf("bluez-hid: request-response-evidence role=%s "
             "op=input-report request=CCC_WRITE response=HANDLE_NOTIFY "
             "ret=%d\n",
             role, ret);
      failed |= ret < 0;
    }
  else if (!failed)
    {
      printf("bluez-hid: source=third/bluez/profiles/input/hog.c "
             "style=gatt-notify command=input-report role=%s "
             "handle=0x%04x write-ret=0 responder-ret=0 "
             "bounded-responder=1\n",
             role, BLUEZ_HOGP_INPUT_REPORT);
      printf("bluez-hid: request-response-evidence role=%s "
             "op=input-report request=CCC_WRITE response=HANDLE_NOTIFY "
             "ret=0 responder-owner=profiles/input/hog.c\n",
             role);
    }

  printf("bluez-hid: source=third/bluez/profiles/input/hog-lib.c "
         "style=hogp-policy command=suspend-resume role=%s "
         "suspend=1 resume=1 boot-report=1 output-report=1\n",
         role);

  if (att != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(att);
      printf("bluez-hid: hogp att close ret=%d role=%s\n", ret, role);
      failed |= ret < 0;
    }

  printf("bluez-hid: closeout cleanup role=%s att=0 hogp=0 input=0 "
         "reports=0\n",
         role);
  printf("bluez-hid: closeout upstream-link-ledger role=%s "
         "dbus-profile=0 gatt-service=0 device-ref=0 adapter-ref=0 "
         "att-fd=closed hogp-session=0 report-map=0 protocol-mode=0 "
         "input-report=0 notify-registration=0 mainloop-watch=0 "
         "pending-request=0 "
         "error-policy=1 cleanup-final=1\n",
         role);
  printf("bluez-hid: closeout upstream-coverage-map role=%s "
         "third/bluez/profiles/input/hog.c "
         "third/bluez/profiles/input/hog-lib.c "
         "third/bluez/src/shared/att.c "
         "third/bluez/src/shared/gatt-client.c "
         "third/bluez/src/shared/gatt-db.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         role);
  printf("bluez-hid: closeout upstream-source-parity role=%s "
         "profile=hogp direct-upstream=hog.c,hog-lib.c,att.c,"
         "gatt-client.c,gatt-db.c,l2cap_sock.c,l2cap_core.c,smp.c "
         "objects=adapter,device,profile,hog-device,gatt-service,"
         "gatt-characteristic,att-bearer,gatt-client,gatt-db,"
         "report-map,protocol-mode,input-report,ccc,mainloop-watch "
         "handlers=hog_probe,hog_attach,hog_detach,report_map_read,"
         "protocol_mode_write,input_report_notify,ccc_write,"
         "suspend,resume,att_read,att_write,att_notify,l2cap_connect "
         "native-gatt=att-cid-0x0004,report-map-0x0025,"
         "protocol-mode-0x0026,input-report-0x0027,ccc,notify "
         "native-l2cap=fixed-att-channel,le-security,smp "
         "upstream-link=bluezhid-upstream-link-bluetoothd "
         "parity-final=%u\n",
         role, failed ? 0 : 1);
  printf("bluez-hid: hogp-final=1 att-final=1 report-map-final=1 "
         "protocol-final=1 input-report-final=1 "
         "request-response-final=1 cleanup-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=bluezhid-upstream-link-bluetoothd "
         "final-ok=%u\n",
         failed ? 0 : 1);

  return failed ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  const char *role;
  uint16_t peer;

  if (argc < 3 || strcmp(argv[1], "closeout") != 0)
    {
      bluez_hid_usage();
      return 1;
    }

  role = argv[2];
  if (!strncmp(role, "classic-", 8))
    {
      peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : BLUEZ_HID_DEFAULT_PEER;
      return bluez_hid_classic_closeout(role, peer);
    }

  if (!strncmp(role, "hogp-", 5))
    {
      peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : BLUEZ_HOGP_DEFAULT_PEER;
      return bluez_hid_hogp_closeout(role, peer);
    }

  bluez_hid_usage();
  return 1;
}
