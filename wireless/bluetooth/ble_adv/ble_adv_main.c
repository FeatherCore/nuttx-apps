/****************************************************************************
 * apps/wireless/bluetooth/ble_adv/ble_adv_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netpacket/bluetooth.h>

#include <nuttx/wireless/bluetooth/bt_core.h>
#include <nuttx/wireless/bluetooth/bt_hci.h>
#include <nuttx/wireless/bluetooth/bt_ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_BLE_ADV_DEMO_IFNAME
#  define CONFIG_BLE_ADV_DEMO_IFNAME "bnep0"
#endif

#ifndef CONFIG_BLE_ADV_DEMO_NAME
#  define CONFIG_BLE_ADV_DEMO_NAME "Feather-ESP-BLE"
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void ble_adv_usage(FAR const char *progname, int exitcode)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s start [ifname] [name]\n", progname);
  fprintf(stderr, "  %s stop [ifname]\n", progname);
  fprintf(stderr, "\nDefaults:\n");
  fprintf(stderr, "  ifname: %s\n", CONFIG_BLE_ADV_DEMO_IFNAME);
  fprintf(stderr, "  name:   %s\n", CONFIG_BLE_ADV_DEMO_NAME);
  exit(exitcode);
}

static int ble_adv_socket(void)
{
  int sockfd;

  sockfd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  if (sockfd < 0)
    {
      fprintf(stderr, "ble_adv: socket failed: errno=%d\n", errno);
    }

  return sockfd;
}

static void ble_adv_fill_name(FAR struct bt_eir_s *eir,
                              FAR const char *name)
{
  size_t namelen;

  namelen = strnlen(name, sizeof(eir->data));
  memset(eir, 0, sizeof(*eir));
  eir->len = namelen + 1;
  eir->type = BT_EIR_NAME_COMPLETE;
  memcpy(eir->data, name, namelen);
}

static int ble_adv_start(FAR const char *ifname, FAR const char *name)
{
  struct btreq_s btreq;
  struct bt_eir_s ad[2];
  struct bt_eir_s sd[2];
  int sockfd;
  int ret;

  memset(ad, 0, sizeof(ad));
  ad[0].len = 2;
  ad[0].type = BT_EIR_FLAGS;
  ad[0].data[0] = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;

  memset(sd, 0, sizeof(sd));
  ble_adv_fill_name(&sd[0], name);

  memset(&btreq, 0, sizeof(btreq));
  strlcpy(btreq.btr_name, ifname, IFNAMSIZ);
  btreq.btr_advtype = BT_LE_ADV_IND;
  btreq.btr_advad = ad;
  btreq.btr_advsd = sd;

  sockfd = ble_adv_socket();
  if (sockfd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = ioctl(sockfd, SIOCBTADVSTART, (unsigned long)((uintptr_t)&btreq));
  if (ret < 0)
    {
      fprintf(stderr, "ble_adv: ioctl(SIOCBTADVSTART) failed: errno=%d\n",
              errno);
      close(sockfd);
      return EXIT_FAILURE;
    }

  close(sockfd);
  printf("ble_adv: advertising started on %s as \"%s\"\n", ifname, name);
  return EXIT_SUCCESS;
}

static int ble_adv_stop(FAR const char *ifname)
{
  struct btreq_s btreq;
  int sockfd;
  int ret;

  memset(&btreq, 0, sizeof(btreq));
  strlcpy(btreq.btr_name, ifname, IFNAMSIZ);

  sockfd = ble_adv_socket();
  if (sockfd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = ioctl(sockfd, SIOCBTADVSTOP, (unsigned long)((uintptr_t)&btreq));
  if (ret < 0)
    {
      fprintf(stderr, "ble_adv: ioctl(SIOCBTADVSTOP) failed: errno=%d\n",
              errno);
      close(sockfd);
      return EXIT_FAILURE;
    }

  close(sockfd);
  printf("ble_adv: advertising stopped on %s\n", ifname);
  return EXIT_SUCCESS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_BLE_ADV_DEMO_IFNAME;
  FAR const char *name = CONFIG_BLE_ADV_DEMO_NAME;

  if (argc < 2 || strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0)
    {
      ble_adv_usage(argv[0], argc < 2 ? EXIT_FAILURE : EXIT_SUCCESS);
    }

  if (argc >= 3)
    {
      ifname = argv[2];
    }

  if (strcmp(argv[1], "start") == 0)
    {
      if (argc >= 4)
        {
          name = argv[3];
        }

      return ble_adv_start(ifname, name);
    }

  if (strcmp(argv[1], "stop") == 0)
    {
      return ble_adv_stop(ifname);
    }

  fprintf(stderr, "ble_adv: unknown command: %s\n", argv[1]);
  ble_adv_usage(argv[0], EXIT_FAILURE);
  return EXIT_FAILURE;
}
