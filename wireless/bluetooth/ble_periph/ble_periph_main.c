/****************************************************************************
 * apps/wireless/bluetooth/ble_periph/ble_periph_main.c
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
#include <nuttx/wireless/bluetooth/bt_gatt.h>
#include <nuttx/wireless/bluetooth/bt_hci.h>
#include <nuttx/wireless/bluetooth/bt_ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_BLE_PERIPH_DEMO_IFNAME
#  define CONFIG_BLE_PERIPH_DEMO_IFNAME "bnep0"
#endif

#ifndef CONFIG_BLE_PERIPH_DEMO_NAME
#  define CONFIG_BLE_PERIPH_DEMO_NAME "Feather-ESP-BLE"
#endif

#ifndef CONFIG_BLE_PERIPH_DEMO_DEFAULT_VALUE
#  define CONFIG_BLE_PERIPH_DEMO_DEFAULT_VALUE "Feather peripheral ready"
#endif

#define BLE_PERIPH_SERVICE_HANDLE      0x0200
#define BLE_PERIPH_VALUE_CHRC_HANDLE   0x0201
#define BLE_PERIPH_VALUE_HANDLE        0x0202
#define BLE_PERIPH_MAX_VALUE_LEN       64

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct bt_uuid_s g_ble_periph_service_uuid =
{
  .type = BT_UUID_128,
  .u.u128 =
    {
      0x6d, 0xf1, 0x8a, 0x2c, 0x7d, 0x8d, 0x4b, 0x5d,
      0x9f, 0x9e, 0x46, 0x53, 0x54, 0x48, 0x45, 0x52
    }
};

static struct bt_uuid_s g_ble_periph_value_uuid =
{
  .type = BT_UUID_128,
  .u.u128 =
    {
      0x6d, 0xf1, 0x8a, 0x2d, 0x7d, 0x8d, 0x4b, 0x5d,
      0x9f, 0x9e, 0x46, 0x53, 0x54, 0x48, 0x45, 0x52
    }
};

static struct bt_gatt_chrc_s g_ble_periph_value_chrc =
{
  .properties   = BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
                  BT_GATT_CHRC_WRITE_WITHOUT_RESP,
  .value_handle = BLE_PERIPH_VALUE_HANDLE,
  .uuid         = &g_ble_periph_value_uuid,
};

static uint8_t g_ble_periph_value[BLE_PERIPH_MAX_VALUE_LEN] =
  CONFIG_BLE_PERIPH_DEMO_DEFAULT_VALUE;

static size_t g_ble_periph_value_len =
  sizeof(CONFIG_BLE_PERIPH_DEMO_DEFAULT_VALUE) - 1;

static bool g_ble_periph_gatt_registered;
static unsigned int g_ble_periph_read_count;
static unsigned int g_ble_periph_write_count;
static unsigned int g_ble_periph_adv_start_count;
static unsigned int g_ble_periph_adv_stop_count;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int ble_periph_socket(void)
{
  int sockfd;

  sockfd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  if (sockfd < 0)
    {
      fprintf(stderr, "ble_periph: socket failed: errno=%d\n", errno);
    }

  return sockfd;
}

static int ble_periph_read(FAR struct bt_conn_s *conn,
                           FAR const struct bt_gatt_attr_s *attr,
                           FAR void *buf, uint8_t len, uint16_t offset)
{
  int ret;

  ret = bt_gatt_attr_read(conn, attr, buf, len, offset,
                          g_ble_periph_value, g_ble_periph_value_len);
  if (ret >= 0)
    {
      g_ble_periph_read_count++;
      printf("ble_periph: read handle=0x%04x offset=%u len=%u "
             "returned=%d count=%u\n",
             BLE_PERIPH_VALUE_HANDLE, offset, len, ret,
             g_ble_periph_read_count);
    }
  else
    {
      printf("ble_periph: read failed handle=0x%04x offset=%u len=%u "
             "ret=%d\n", BLE_PERIPH_VALUE_HANDLE, offset, len, ret);
    }

  return ret;
}

static int ble_periph_write(FAR struct bt_conn_s *conn,
                            FAR const struct bt_gatt_attr_s *attr,
                            FAR const void *buf, uint8_t len,
                            uint16_t offset)
{
  FAR const uint8_t *src = buf;
  size_t i;

  if (offset > BLE_PERIPH_MAX_VALUE_LEN ||
      len > BLE_PERIPH_MAX_VALUE_LEN - offset)
    {
      printf("ble_periph: write rejected handle=0x%04x offset=%u len=%u "
             "max=%u\n", BLE_PERIPH_VALUE_HANDLE, offset, len,
             BLE_PERIPH_MAX_VALUE_LEN);
      return -EINVAL;
    }

  memcpy(&g_ble_periph_value[offset], src, len);
  g_ble_periph_value_len = offset + len;
  g_ble_periph_write_count++;

  printf("ble_periph: write handle=0x%04x offset=%u len=%u count=%u data=",
         BLE_PERIPH_VALUE_HANDLE, offset, len, g_ble_periph_write_count);

  for (i = 0; i < len; i++)
    {
      printf("%02x", src[i]);
    }

  printf(" text=\"");
  for (i = 0; i < g_ble_periph_value_len; i++)
    {
      uint8_t ch = g_ble_periph_value[i];
      putchar(ch >= 0x20 && ch <= 0x7e ? ch : '.');
    }

  printf("\"\n");
  return len;
}

static const struct bt_gatt_attr_s g_ble_periph_attrs[] =
{
  BT_GATT_PRIMARY_SERVICE(BLE_PERIPH_SERVICE_HANDLE,
                          &g_ble_periph_service_uuid),
  BT_GATT_CHARACTERISTIC(BLE_PERIPH_VALUE_CHRC_HANDLE,
                         &g_ble_periph_value_chrc),
  BT_GATT_DESCRIPTOR(BLE_PERIPH_VALUE_HANDLE,
                     &g_ble_periph_value_uuid,
                     BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                     ble_periph_read, ble_periph_write, NULL),
};

static void ble_periph_fill_name(FAR struct bt_eir_s *eir,
                                 FAR const char *name)
{
  size_t namelen;

  namelen = strnlen(name, sizeof(eir->data));
  memset(eir, 0, sizeof(*eir));
  eir->len = namelen + 1;
  eir->type = BT_EIR_NAME_COMPLETE;
  memcpy(eir->data, name, namelen);
}

static void ble_periph_register_gatt(void)
{
  if (!g_ble_periph_gatt_registered)
    {
      bt_gatt_register(g_ble_periph_attrs, nitems(g_ble_periph_attrs));
      g_ble_periph_gatt_registered = true;
    }
}

static int ble_periph_check(FAR const char *ifname)
{
  struct btreq_s btreq;
  int sockfd;
  int ret;

  memset(&btreq, 0, sizeof(btreq));
  strlcpy(btreq.btr_name, ifname, IFNAMSIZ);

  sockfd = ble_periph_socket();
  if (sockfd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = ioctl(sockfd, SIOCGBTINFO, (unsigned long)((uintptr_t)&btreq));
  if (ret < 0)
    {
      fprintf(stderr, "ble_periph: ioctl(SIOCGBTINFO) failed: errno=%d\n",
              errno);
      close(sockfd);
      return EXIT_FAILURE;
    }

  close(sockfd);

  printf("ble_periph: host path ok ifname=%s "
         "bdaddr=%02x:%02x:%02x:%02x:%02x:%02x flags=0x%04x\n",
         ifname, btreq.btr_bdaddr.val[5], btreq.btr_bdaddr.val[4],
         btreq.btr_bdaddr.val[3], btreq.btr_bdaddr.val[2],
         btreq.btr_bdaddr.val[1], btreq.btr_bdaddr.val[0],
         btreq.btr_flags);
  printf("ble_periph: buffers cmd=%u acl=%u/%u sco=%u/%u "
         "mtu_acl=%u mtu_sco=%u\n",
         btreq.btr_num_cmd, btreq.btr_num_acl, btreq.btr_max_acl,
         btreq.btr_num_sco, btreq.btr_max_sco, btreq.btr_acl_mtu,
         btreq.btr_sco_mtu);
  printf("ble_periph: expected phone flow: scan name, connect, optional "
         "pair, discover service, read/write handle=0x%04x\n",
         BLE_PERIPH_VALUE_HANDLE);
  return EXIT_SUCCESS;
}

static int ble_periph_start_adv(FAR const char *ifname, FAR const char *name)
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
  ble_periph_fill_name(&sd[0], name);

  memset(&btreq, 0, sizeof(btreq));
  strlcpy(btreq.btr_name, ifname, IFNAMSIZ);
  btreq.btr_advtype = BT_LE_ADV_IND;
  btreq.btr_advad = ad;
  btreq.btr_advsd = sd;

  printf("ble_periph: adv request ifname=%s type=%u flags=0x%02x "
         "scan_rsp_name=\"%s\"\n", ifname, btreq.btr_advtype,
         ad[0].data[0], name);

  sockfd = ble_periph_socket();
  if (sockfd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = ioctl(sockfd, SIOCBTADVSTART, (unsigned long)((uintptr_t)&btreq));
  if (ret < 0)
    {
      fprintf(stderr, "ble_periph: ioctl(SIOCBTADVSTART) failed: errno=%d\n",
              errno);
      close(sockfd);
      return EXIT_FAILURE;
    }

  close(sockfd);
  g_ble_periph_adv_start_count++;
  return EXIT_SUCCESS;
}

static int ble_periph_stop_adv(FAR const char *ifname)
{
  struct btreq_s btreq;
  int sockfd;
  int ret;

  memset(&btreq, 0, sizeof(btreq));
  strlcpy(btreq.btr_name, ifname, IFNAMSIZ);

  sockfd = ble_periph_socket();
  if (sockfd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = ioctl(sockfd, SIOCBTADVSTOP, (unsigned long)((uintptr_t)&btreq));
  if (ret < 0)
    {
      fprintf(stderr, "ble_periph: ioctl(SIOCBTADVSTOP) failed: errno=%d\n",
              errno);
      close(sockfd);
      return EXIT_FAILURE;
    }

  close(sockfd);
  g_ble_periph_adv_stop_count++;
  printf("ble_periph: advertising stopped on %s stop_count=%u\n",
         ifname, g_ble_periph_adv_stop_count);
  return EXIT_SUCCESS;
}

static void ble_periph_usage(FAR const char *progname, int exitcode)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s start [ifname] [name]\n", progname);
  fprintf(stderr, "  %s stop [ifname]\n", progname);
  fprintf(stderr, "  %s check [ifname]\n", progname);
  fprintf(stderr, "  %s status\n", progname);
  fprintf(stderr, "\nDefaults:\n");
  fprintf(stderr, "  ifname: %s\n", CONFIG_BLE_PERIPH_DEMO_IFNAME);
  fprintf(stderr, "  name:   %s\n", CONFIG_BLE_PERIPH_DEMO_NAME);
  fprintf(stderr, "\nService UUID:  6df18a2c-7d8d-4b5d-9f9e-465354484552\n");
  fprintf(stderr, "Value UUID:    6df18a2d-7d8d-4b5d-9f9e-465354484552\n");
  fprintf(stderr, "Value handle:  0x%04x\n", BLE_PERIPH_VALUE_HANDLE);
  exit(exitcode);
}

static int ble_periph_start(FAR const char *ifname, FAR const char *name)
{
  int ret;

  ret = ble_periph_check(ifname);
  if (ret != EXIT_SUCCESS)
    {
      return ret;
    }

  ble_periph_register_gatt();
  ret = ble_periph_start_adv(ifname, name);
  if (ret != EXIT_SUCCESS)
    {
      return ret;
    }

  printf("ble_periph: advertising started on %s as \"%s\"\n", ifname, name);
  printf("ble_periph: GATT service=6df18a2c-7d8d-4b5d-9f9e-465354484552 "
         "value=6df18a2d-7d8d-4b5d-9f9e-465354484552 handle=0x%04x\n",
         BLE_PERIPH_VALUE_HANDLE);
  printf("ble_periph: counters adv_start=%u adv_stop=%u read=%u write=%u\n",
         g_ble_periph_adv_start_count, g_ble_periph_adv_stop_count,
         g_ble_periph_read_count, g_ble_periph_write_count);
  printf("ble_periph: use a BLE scanner app to connect/discover/read/write; "
         "phone system Bluetooth UI may pair without showing this custom "
         "GATT service as a classic Bluetooth device\n");
  return EXIT_SUCCESS;
}

static int ble_periph_status(void)
{
  size_t i;

  printf("ble_periph: gatt_registered=%s handle=0x%04x len=%zu value=\"",
         g_ble_periph_gatt_registered ? "yes" : "no",
         BLE_PERIPH_VALUE_HANDLE, g_ble_periph_value_len);

  for (i = 0; i < g_ble_periph_value_len; i++)
    {
      uint8_t ch = g_ble_periph_value[i];
      putchar(ch >= 0x20 && ch <= 0x7e ? ch : '.');
    }

  printf("\" read_count=%u write_count=%u adv_start=%u adv_stop=%u\n",
         g_ble_periph_read_count, g_ble_periph_write_count,
         g_ble_periph_adv_start_count, g_ble_periph_adv_stop_count);
  return EXIT_SUCCESS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_BLE_PERIPH_DEMO_IFNAME;
  FAR const char *name = CONFIG_BLE_PERIPH_DEMO_NAME;

  if (argc < 2 || strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0)
    {
      ble_periph_usage(argv[0], argc < 2 ? EXIT_FAILURE : EXIT_SUCCESS);
    }

  if (strcmp(argv[1], "status") == 0)
    {
      return ble_periph_status();
    }

  if (argc >= 3)
    {
      ifname = argv[2];
    }

  if (strcmp(argv[1], "check") == 0)
    {
      return ble_periph_check(ifname);
    }

  if (strcmp(argv[1], "start") == 0)
    {
      if (argc >= 4)
        {
          name = argv[3];
        }

      return ble_periph_start(ifname, name);
    }

  if (strcmp(argv[1], "stop") == 0)
    {
      return ble_periph_stop_adv(ifname);
    }

  fprintf(stderr, "ble_periph: unknown command: %s\n", argv[1]);
  ble_periph_usage(argv[0], EXIT_FAILURE);
  return EXIT_FAILURE;
}
