/****************************************************************************
 * apps/wireless/bluetooth/ble_gatt/ble_gatt_main.c
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

#include <nuttx/wireless/bluetooth/bt_core.h>
#include <nuttx/wireless/bluetooth/bt_gatt.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_BLE_GATT_DEMO_DEFAULT_VALUE
#  define CONFIG_BLE_GATT_DEMO_DEFAULT_VALUE "Feather GATT ready"
#endif

#define BLE_GATT_DEMO_SERVICE_HANDLE      0x0100
#define BLE_GATT_DEMO_VALUE_CHRC_HANDLE   0x0101
#define BLE_GATT_DEMO_VALUE_HANDLE        0x0102
#define BLE_GATT_DEMO_MAX_VALUE_LEN       64

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct bt_uuid_s g_ble_gatt_demo_service_uuid =
{
  .type = BT_UUID_128,
  .u.u128 =
    {
      0x6d, 0xf1, 0x8a, 0x2a, 0x7d, 0x8d, 0x4b, 0x5d,
      0x9f, 0x9e, 0x46, 0x53, 0x54, 0x48, 0x45, 0x52
    }
};

static struct bt_uuid_s g_ble_gatt_demo_value_uuid =
{
  .type = BT_UUID_128,
  .u.u128 =
    {
      0x6d, 0xf1, 0x8a, 0x2b, 0x7d, 0x8d, 0x4b, 0x5d,
      0x9f, 0x9e, 0x46, 0x53, 0x54, 0x48, 0x45, 0x52
    }
};

static struct bt_gatt_chrc_s g_ble_gatt_demo_value_chrc =
{
  .properties   = BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
                  BT_GATT_CHRC_WRITE_WITHOUT_RESP,
  .value_handle = BLE_GATT_DEMO_VALUE_HANDLE,
  .uuid         = &g_ble_gatt_demo_value_uuid,
};

static uint8_t g_ble_gatt_demo_value[BLE_GATT_DEMO_MAX_VALUE_LEN] =
  CONFIG_BLE_GATT_DEMO_DEFAULT_VALUE;

static size_t g_ble_gatt_demo_value_len =
  sizeof(CONFIG_BLE_GATT_DEMO_DEFAULT_VALUE) - 1;

static bool g_ble_gatt_demo_registered;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int ble_gatt_demo_read(FAR struct bt_conn_s *conn,
                              FAR const struct bt_gatt_attr_s *attr,
                              FAR void *buf, uint8_t len, uint16_t offset)
{
  return bt_gatt_attr_read(conn, attr, buf, len, offset,
                           g_ble_gatt_demo_value,
                           g_ble_gatt_demo_value_len);
}

static int ble_gatt_demo_write(FAR struct bt_conn_s *conn,
                               FAR const struct bt_gatt_attr_s *attr,
                               FAR const void *buf, uint8_t len,
                               uint16_t offset)
{
  FAR const uint8_t *src = buf;
  size_t i;

  if (offset > BLE_GATT_DEMO_MAX_VALUE_LEN ||
      len > BLE_GATT_DEMO_MAX_VALUE_LEN - offset)
    {
      return -EINVAL;
    }

  memcpy(&g_ble_gatt_demo_value[offset], src, len);
  g_ble_gatt_demo_value_len = offset + len;

  printf("ble_gatt: write handle=0x%04x offset=%u len=%u data=",
         BLE_GATT_DEMO_VALUE_HANDLE, offset, len);

  for (i = 0; i < len; i++)
    {
      printf("%02x", src[i]);
    }

  printf(" text=\"");
  for (i = 0; i < g_ble_gatt_demo_value_len; i++)
    {
      uint8_t ch = g_ble_gatt_demo_value[i];
      putchar(ch >= 0x20 && ch <= 0x7e ? ch : '.');
    }

  printf("\"\n");
  return len;
}

static const struct bt_gatt_attr_s g_ble_gatt_demo_attrs[] =
{
  BT_GATT_PRIMARY_SERVICE(BLE_GATT_DEMO_SERVICE_HANDLE,
                          &g_ble_gatt_demo_service_uuid),
  BT_GATT_CHARACTERISTIC(BLE_GATT_DEMO_VALUE_CHRC_HANDLE,
                         &g_ble_gatt_demo_value_chrc),
  BT_GATT_DESCRIPTOR(BLE_GATT_DEMO_VALUE_HANDLE,
                     &g_ble_gatt_demo_value_uuid,
                     BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                     ble_gatt_demo_read, ble_gatt_demo_write, NULL),
};

static void ble_gatt_usage(FAR const char *progname, int exitcode)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s start\n", progname);
  fprintf(stderr, "  %s status\n", progname);
  fprintf(stderr, "\nService UUID:  6df18a2a-7d8d-4b5d-9f9e-465354484552\n");
  fprintf(stderr, "Value UUID:    6df18a2b-7d8d-4b5d-9f9e-465354484552\n");
  fprintf(stderr, "Value handle:  0x%04x\n", BLE_GATT_DEMO_VALUE_HANDLE);
  exit(exitcode);
}

static int ble_gatt_start(void)
{
  if (!g_ble_gatt_demo_registered)
    {
      bt_gatt_register(g_ble_gatt_demo_attrs,
                       nitems(g_ble_gatt_demo_attrs));
      g_ble_gatt_demo_registered = true;
    }

  printf("ble_gatt: registered service=6df18a2a-7d8d-4b5d-9f9e-465354484552 "
         "value=6df18a2b-7d8d-4b5d-9f9e-465354484552 handle=0x%04x\n",
         BLE_GATT_DEMO_VALUE_HANDLE);
  return EXIT_SUCCESS;
}

static int ble_gatt_status(void)
{
  size_t i;

  printf("ble_gatt: registered=%s handle=0x%04x len=%zu value=\"",
         g_ble_gatt_demo_registered ? "yes" : "no",
         BLE_GATT_DEMO_VALUE_HANDLE, g_ble_gatt_demo_value_len);

  for (i = 0; i < g_ble_gatt_demo_value_len; i++)
    {
      uint8_t ch = g_ble_gatt_demo_value[i];
      putchar(ch >= 0x20 && ch <= 0x7e ? ch : '.');
    }

  printf("\"\n");
  return EXIT_SUCCESS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  if (argc != 2 || strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0)
    {
      ble_gatt_usage(argv[0], argc == 2 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

  if (strcmp(argv[1], "start") == 0)
    {
      return ble_gatt_start();
    }

  if (strcmp(argv[1], "status") == 0)
    {
      return ble_gatt_status();
    }

  fprintf(stderr, "ble_gatt: unknown command: %s\n", argv[1]);
  ble_gatt_usage(argv[0], EXIT_FAILURE);
  return EXIT_FAILURE;
}
