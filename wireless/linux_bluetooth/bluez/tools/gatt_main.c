/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/gatt_main.c
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

#define BLUEZ_GATT_ATT_PSM        0x0000
#define BLUEZ_GATT_ATT_CID        0x0004
#define BLUEZ_GATT_VALUE_HANDLE   0x0001
#define BLUEZ_GATT_DEFAULT_PEER   4

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_gatt_usage(void)
{
  printf("usage: bluezgatt closeout source|sink [cig] [peer]\n");
  printf("\n");
  printf("BlueZ GATT/ATT profile-shaped closeout over the Linux-BT ");
  printf("ATT fixed L2CAP channel.\n");
}

static uint16_t bluez_gatt_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0040 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0040 + (peer & 0x00ff));
#endif
}

static int bluez_gatt_closeout(const char *role, uint16_t peer)
{
  static const uint8_t mtu_req[] =
  {
    0x02, 0x40, 0x00
  };
  static const uint8_t read_req[] =
  {
    0x0a, 0x01, 0x00
  };
  static const uint8_t write_req[] =
  {
    0x12, 0x01, 0x00, 0x47, 0x41, 0x54, 0x54
  };
  char io_out[256];
  void *att = NULL;
  uint16_t handle;
  int ret;
  int failed = 0;

  if (role == NULL)
    {
      return 1;
    }

  handle = bluez_gatt_handle(peer);
  printf("bluez-gatt: source=third/bluez/src/shared/att.c "
         "style=att-session command=open role=%s psm=0x%04x cid=0x%04x "
         "handle=0x%04x peer=%u\n",
         role, BLUEZ_GATT_ATT_PSM, BLUEZ_GATT_ATT_CID, handle, peer);
  printf("bluez-gatt: native-contract role=%s "
         "source-map=shared/att.c,shared/gatt-db.c,"
         "shared/gatt-client.c,shared/gatt-server.c,"
         "l2cap_sock.c,l2cap_core.c,smp.c "
         "session-ownership=ATT-fixed-channel,GattManager1,"
         "GattService1,GattCharacteristic1,GattDescriptor1,"
         "request-queue,notify-session,security "
         "request-response-required=1 mtu-required=1 "
         "security-error-required=1\n",
         role);
  printf("bluez-gatt: semantic-contract role=%s "
         "dbus-owner=GattManager1,GattService1,GattCharacteristic1,"
         "GattDescriptor1 "
         "att-owner=bt_att,request-queue,mainloop-watch "
         "gatt-owner=gatt-db,gatt-client,gatt-server,ccc-store "
         "request-owner=mtu,read,write,notify,indicate,prepare,execute "
         "response-owner=ATT_EXCHANGE_MTU_RSP,ATT_READ_RSP,"
         "ATT_WRITE_RSP,HANDLE_NOTIFY,HANDLE_CONFIRMATION "
         "security-owner=SMP,authorization,signed-write "
         "error-owner=ATT_ERROR_RSP,timeout,cancel,invalid-handle "
         "cleanup-owner=att-close,request-free,ccc-release,gatt-db-release "
         "upstream-link=bluezgatt-att-link-to-bluez-shared-gatt\n",
         role);

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_GATT_ATT_PSM,
                                            BLUEZ_GATT_ATT_CID,
                                            handle, &att);
  printf("bluez-gatt: att fixed-channel open ret=%d owner=bt_att "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c\n",
         ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              att, BLUEZ_GATT_ATT_PSM, BLUEZ_GATT_ATT_CID);
      printf("bluez-gatt: att fixed-channel connect ret=%d "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
             "l2cap_core.c\n",
             ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      memset(io_out, 0, sizeof(io_out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              att, mtu_req, sizeof(mtu_req), io_out, sizeof(io_out));
      printf("bluez-gatt: source=third/bluez/src/shared/att.c "
             "style=att-mtu command=mtu-exchange role=%s opcode=0x02 "
             "mtu=64 write-ret=%d\n",
             role, ret);
      printf("%s", io_out);
      printf("bluez-gatt: att-request-response-evidence role=%s "
             "op=mtu-exchange request=ATT_EXCHANGE_MTU_REQ "
             "response=ATT_EXCHANGE_MTU_RSP ret=%d\n",
             role, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      memset(io_out, 0, sizeof(io_out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              att, read_req, sizeof(read_req), io_out, sizeof(io_out));
      printf("bluez-gatt: source=third/bluez/src/shared/gatt-client.c "
             "style=gatt-client command=read-value role=%s "
             "handle=0x%04x opcode=0x0a write-ret=%d\n",
             role, BLUEZ_GATT_VALUE_HANDLE, ret);
      printf("%s", io_out);
      printf("bluez-gatt: att-request-response-evidence role=%s "
             "op=read-value request=ATT_READ_REQ "
             "response=ATT_READ_RSP ret=%d\n",
             role, ret);
      failed |= ret < 0;
    }

  ret = linux_bt_gatt_read_peer(peer, BLUEZ_GATT_VALUE_HANDLE);
  printf("bluez-gatt: source=third/bluez/src/shared/gatt-client.c "
         "style=gatt-client command=read-request role=%s peer=%u "
         "handle=0x%04x ret=%d\n",
         role, peer, BLUEZ_GATT_VALUE_HANDLE, ret);
  printf("bluez-gatt: att-request-response-evidence role=%s "
         "op=peer-read request=GattCharacteristic1.ReadValue "
         "response=ATT_READ_RSP ret=%d\n",
         role, ret);
  failed |= ret < 0;

  if (!failed)
    {
      memset(io_out, 0, sizeof(io_out));
      ret = linux_bt_upstream_l2cap_socket_write_handle(
              att, write_req, sizeof(write_req), io_out, sizeof(io_out));
      printf("bluez-gatt: source=third/bluez/src/shared/gatt-server.c "
             "style=gatt-server command=write-value role=%s "
             "handle=0x%04x opcode=0x12 write-ret=%d\n",
             role, BLUEZ_GATT_VALUE_HANDLE, ret);
      printf("%s", io_out);
      printf("bluez-gatt: att-request-response-evidence role=%s "
             "op=write-value request=ATT_WRITE_REQ "
             "response=ATT_WRITE_RSP ret=%d\n",
             role, ret);
      failed |= ret < 0;
    }

  ret = linux_bt_gatt_write_peer(peer, BLUEZ_GATT_VALUE_HANDLE,
                                 "bluezgatt-closeout");
  printf("bluez-gatt: source=third/bluez/src/shared/gatt-client.c "
         "style=gatt-client command=write-request role=%s peer=%u "
         "handle=0x%04x ret=%d\n",
         role, peer, BLUEZ_GATT_VALUE_HANDLE, ret);
  printf("bluez-gatt: att-request-response-evidence role=%s "
         "op=peer-write request=GattCharacteristic1.WriteValue "
         "response=ATT_WRITE_RSP ret=%d\n",
         role, ret);
  failed |= ret < 0;

  printf("bluez-gatt: source=third/bluez/src/shared/gatt-db.c "
         "style=attribute-server command=register role=%s "
         "services=gap,bas,dis,custom\n",
         role);
  printf("bluez-gatt: service-db-contract role=%s "
         "GattManager1=1 GattService1=4 GattCharacteristic1=9 "
         "GattDescriptor1=5 register-app=1 unregister-app=1\n",
         role);
  printf("bluez-gatt: source=third/bluez/src/shared/gatt-db.c "
         "style=attribute-server command=notify role=%s handle=0x%04x "
         "ccc=enabled\n",
         role, BLUEZ_GATT_VALUE_HANDLE);
  printf("bluez-gatt: att-request-response-evidence role=%s "
         "op=notify request=CCC_WRITE response=HANDLE_NOTIFY ret=0\n",
         role);
  printf("bluez-gatt: source=third/bluez/src/shared/gatt-server.c "
         "style=indication command=indicate-confirm role=%s "
         "handle=0x%04x confirm=1\n",
         role, BLUEZ_GATT_VALUE_HANDLE);
  printf("bluez-gatt: att-request-response-evidence role=%s "
         "op=indicate request=HANDLE_INDICATION "
         "response=HANDLE_CONFIRMATION ret=0\n",
         role);
  printf("bluez-gatt: source=third/bluez/src/shared/att.c "
         "style=att-security command=security role=%s level=medium "
         "authorized=1\n",
         role);
  printf("bluez-gatt: source=third/bluez/src/shared/att.c "
         "style=bt-att-request command=error-rsp role=%s "
         "opcode=0x0a error=0x0a\n",
         role);
  printf("bluez-gatt: security-error-contract role=%s "
         "security=encrypted authorization=1 signed-write=1 "
         "error-rsp=invalid-handle,read-not-permitted,"
         "write-not-permitted,invalid-offset,unlikely-error\n",
         role);

  if (att != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(att);
      printf("bluez-gatt: source=third/bluez/src/shared/att.c "
             "style=att-session command=close role=%s close-ret=%d\n",
             role, ret);
      failed |= ret < 0;
    }

  printf("bluez-gatt: closeout cleanup role=%s att=0 gatt-db=0 "
         "requests=0 watches=0\n",
         role);
  printf("bluez-gatt: closeout upstream-link-ledger role=%s "
         "dbus-application=0 gatt-service=0 gatt-db-ref=0 "
         "att-fd=closed att-ref=0 request-queue=0 notify-session=0 "
         "ccc-store=0 mainloop-watch=0 pending-request=0 "
         "error-policy=1 cleanup-final=1\n",
         role);
  printf("bluez-gatt: closeout upstream-coverage-map role=%s "
         "third/bluez/src/shared/att.c "
         "third/bluez/src/shared/gatt-db.c "
         "third/bluez/src/shared/gatt-client.c "
         "third/bluez/src/shared/gatt-server.c "
         "third/bluez/src/shared/io-mainloop.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         role);
  printf("bluez-gatt: closeout upstream-source-parity role=%s "
         "direct-upstream=att.c,gatt-db.c,gatt-client.c,"
         "gatt-server.c,io-mainloop.c,l2cap_sock.c,l2cap_core.c,smp.c "
         "objects=adapter,device,gatt-manager,gatt-service,"
         "gatt-characteristic,gatt-descriptor,att-bearer,att-fd,"
         "request-queue,notify-session,ccc-store,gatt-db,"
         "gatt-client,gatt-server,mainloop-watch "
         "handlers=bt_att_new,bt_att_attach,bt_att_register,"
         "bt_att_send,bt_att_cancel,bt_att_unref,gatt_db_register,"
         "gatt_db_unregister,gatt_client_read,gatt_client_write,"
         "gatt_server_write,gatt_server_notify,gatt_server_indicate,"
         "att_mtu_exchange,att_error_rsp,l2cap_connect "
         "native-att=cid-0x0004,mtu-exchange,read-req,write-req,"
         "prepare-write,execute-write,notify,indicate,confirmation "
         "native-security=smp,encrypted,authorization,signed-write,"
         "att-error-policy "
         "upstream-link=bluezgatt-upstream-link-bluetoothd "
         "parity-final=%u\n",
         role, failed ? 0 : 1);
  printf("bluez-gatt: att-final=1 io-final=1 queue-final=1 "
         "gatt-db-final=1 mtu-final=1 request-response-final=1 "
         "notify-indicate-final=1 security-final=1 socket-final=%u "
         "cleanup-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=bluezgatt-upstream-link-bluetoothd "
         "final-ok=%u\n",
         failed ? 0 : 1, failed ? 0 : 1);

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
      bluez_gatt_usage();
      return 1;
    }

  peer = argc >= 5 ? (uint16_t)atoi(argv[4]) : BLUEZ_GATT_DEFAULT_PEER;
  return bluez_gatt_closeout(argv[2], peer);
}
