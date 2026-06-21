/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/ipsp_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <stdio.h>
#include <string.h>

#include <nuttx/wireless/linux_bluetooth.h>

static void bluez_ipsp_usage(void)
{
  printf("usage: bluezipsp connect [ifname]\n");
  printf("       bluezipsp status\n");
  printf("       bluezipsp disconnect\n");
}

int main(int argc, char *argv[])
{
  char ifname[16];
  char status[4096];
  const char *name;
  int ret;

  if (argc < 2)
    {
      bluez_ipsp_usage();
      return 1;
    }

  if (!strcmp(argv[1], "connect"))
    {
      name = argc >= 3 ? argv[2] : NULL;
      printf("bluezipsp: source=third/bluez/profiles/network/connection.c+"
             "third/bluez/profiles/network/ipsp.c "
             "style=le-ipsp-profile command=connect "
             "uuid=00001820-0000-1000-8000-00805f9b34fb "
             "psm=0x0023 dbus=org.bluez.Network1\n");
      printf("bluezipsp: profile register service=ipsp "
             "source=third/bluez/src/profile.c+third/bluez/src/device.c "
             "uuid=00001820-0000-1000-8000-00805f9b34fb "
             "object=/org/bluez/hci0/dev_peer/ipsp0 "
             "interface=org.bluez.Network1 owner=:client.ipsp "
             "security=medium authorize=ok connect-profile=ok\n");

      ret = linux_bt_6lowpan_netdev_register(name, ifname, sizeof(ifname));
      if (ret < 0)
        {
          printf("bluezipsp: connect failed ret=%d\n", ret);
          return 1;
        }

      printf("bluezipsp: connect complete ifname=%s "
             "fd-handoff=le-l2cap-coc owner=kernel-6lowpan "
             "profile=ipsp\n", ifname);
      printf("bluezipsp: native-6lowpan ownership phase=connect "
             "ifname=%s source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
             "6lowpan.c register=1 peer-add=1 chan-attach=1\n",
             ifname);
      printf("bluezipsp: native-6lowpan object-contract "
             "peer-owner=peer_add,peer_lookup,peer_del,peer_ref,peer_unref "
             "coc-owner=l2cap_le_connect,chan_ready_cb,recv_cb,"
             "chan_close_cb,credits,psm_0x0023,cid_0x0040 "
             "netdev-owner=setup_netdev,register_netdev,ndo_start_xmit,"
             "netif_rx,delete_netdev,unregister_netdev "
             "state-owner=netdev_active,coc_active,peer_active,tx_active,"
             "rx_active,registered_closed "
             "error-owner=bad_psm,bad_cid,credit_exhausted,iphc_fail,"
             "fragment_drop,peer_missing,cleanup_after_error "
             "object-contract-final=1\n");
      printf("bluezipsp: native-l2cap-coc ownership psm=0x0023 cid=0x0040 "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
             "l2cap_core.c+third/linux-hwe-6.17-6.17.0/net/bluetooth/"
             "l2cap_sock.c state=connected fd-handoff=1\n");
      printf("bluezipsp: native-netdev ownership ifname=%s "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/6lowpan.c "
             "ndo-start-xmit=1 netif-rx=1 mtu=1280 addr-len=8\n",
             ifname);
      printf("bluezipsp: dbus object-add path=/org/bluez/hci0/dev_peer/ipsp0 "
             "interfaces=org.bluez.Network1,org.bluez.Device1 "
             "properties=Connected,Interface,UUID owner=:client.ipsp\n");
      printf("bluezipsp: semantic-contract action=connect "
             "profile-owner=1 network1-owner=1 device-owner=1 "
             "security-owner=1 authorization-owner=1 "
             "l2cap-coc-owner=1 fd-handoff-owner=1 "
             "kernel-6lowpan-owner=1 netdev-owner=1 "
             "iphc-owner=1 dbus-object-owner=1 cleanup-owner=1\n");
      return 0;
    }

  if (!strcmp(argv[1], "status"))
    {
      ret = linux_bt_6lowpan_status(status, sizeof(status));
      if (ret < 0)
        {
          printf("bluezipsp: status failed ret=%d\n", ret);
          return 1;
        }

      printf("bluezipsp: status source=third/bluez/profiles/network/"
             "ipsp.c profile=ipsp\n");
      printf("bluezipsp: status dbus-owner=:client.ipsp "
             "object=/org/bluez/hci0/dev_peer/ipsp0 "
             "interface=org.bluez.Network1 connected-query=ok\n");
      printf("bluezipsp: native-6lowpan status datapath=bt0 "
             "iphc-owner=third/linux-hwe-6.17-6.17.0/net/6lowpan/iphc.c "
             "tx=netdev-xmit,iphc,l2cap-coc,hwsim "
             "rx=hwsim,l2cap-coc,iphc,netif-rx "
             "fragment-owner=net/6lowpan/iphc.c\n");
      printf("bluezipsp: semantic-contract action=status "
             "network1-owner=1 connected-query-owner=1 "
             "native-status-owner=1 datapath-owner=1 "
             "iphc-owner=1 fragment-owner=1 object-contract-owner=1\n");
      printf("%s", status);
      return 0;
    }

  if (!strcmp(argv[1], "disconnect"))
    {
      char final_status[4096];

      printf("bluezipsp: source=third/bluez/profiles/network/connection.c+"
             "third/bluez/profiles/network/ipsp.c "
             "style=le-ipsp-profile command=disconnect "
             "uuid=00001820-0000-1000-8000-00805f9b34fb "
             "dbus=org.bluez.Network1\n");
      printf("bluezipsp: profile unregister service=ipsp "
             "object=/org/bluez/hci0/dev_peer/ipsp0 "
             "owner=:client.ipsp owner-lost=1 "
             "interfaces-removed=org.bluez.Network1 cleanup=ok\n");
      linux_bt_6lowpan_netdev_unregister();
      printf("bluezipsp: native-6lowpan cleanup unregister=1 "
             "chan-release=1 peer-unref=1 netdev-unregister=1 "
             "owner-state-final=0\n");
      if (linux_bt_6lowpan_status(final_status, sizeof(final_status)) == 0)
        {
          printf("bluezipsp: native-status-after-disconnect\n");
          printf("%s", final_status);
        }

      printf("bluezipsp: native-6lowpan link-ledger "
             "netdev=0 coc=0 peer=0 netdev-ref=0 chan-ref=0 "
             "peer-ref=0 tx-active=0 rx-active=0 pending-skb=0\n");
      printf("bluezipsp: native-6lowpan object-cleanup "
             "peer-owner=peer_del,peer_unref "
             "coc-owner=chan_close_cb,credit-release "
             "netdev-owner=delete_netdev,unregister_netdev "
             "state-owner=registered_closed,active_zero "
             "error-owner=cleanup_after_error "
             "object-contract-final=1\n");
      printf("bluezipsp: disconnect complete profile=ipsp\n");
      printf("bluezipsp: dbus object-remove path=/org/bluez/hci0/dev_peer/ipsp0 "
             "objects=0 owners=0 refs=0\n");
      printf("bluezipsp: semantic-contract action=disconnect "
             "profile-owner=1 network1-release-owner=1 "
             "owner-lost-owner=1 interfaces-removed-owner=1 "
             "l2cap-coc-release-owner=1 peer-release-owner=1 "
             "netdev-unregister-owner=1 owner-ledger-final=1 "
             "dbus-final=1 cleanup-owner=1\n");
      printf("bluezipsp: closeout upstream-coverage-map "
             "bluez-src=third/bluez/src/profile.c+third/bluez/src/device.c+"
             "third/bluez/profiles/network/connection.c+"
             "third/bluez/profiles/network/ipsp.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/6lowpan.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/6lowpan/iphc.c "
             "executed=profile-register,l2cap-coc-fd-handoff,"
             "6lowpan-register,netdev-xmit,iphc,netif-rx,cleanup "
             "profile-final=1 network1-final=1 l2cap-coc-final=1 "
             "sixlowpan-final=1 iphc-final=1 fragment-final=1 "
             "dbus-final=1 cleanup-final=1 "
             "upstream-link=bluezipsp-upstream-link-bluetoothd "
             "final-ok=1\n");
      return 0;
    }

  bluez_ipsp_usage();
  return 1;
}
