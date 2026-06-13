/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/btmon_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_HCI
#  define BTPROTO_HCI 1
#endif

#ifndef HCI_CHANNEL_MONITOR
#  define HCI_CHANNEL_MONITOR 2
#endif

#ifndef HCI_CHANNEL_CONTROL
#  define HCI_CHANNEL_CONTROL 3
#endif

#ifndef HCI_DEV_NONE
#  define HCI_DEV_NONE 0xffff
#endif

#ifndef MGMT_INDEX_NONE
#  define MGMT_INDEX_NONE 0xffff
#endif

#define MGMT_OP_READ_VERSION 0x0001
#define MGMT_OP_SET_POWERED  0x0005

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_btmon_sockaddr_hci
{
  uint16_t hci_family;
  uint16_t hci_dev;
  uint16_t hci_channel;
};

struct bluez_btmon_mgmt_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
};

struct bluez_btmon_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_btmon_usage(void)
{
  printf("usage: bluezbtmon control\n");
  printf("\n");
  printf("BlueZ btmon-style monitor over AF_BLUETOOTH/BTPROTO_HCI monitor socket.\n");
}

static int bluez_btmon_open_hci(uint16_t channel, const char *name)
{
  struct bluez_btmon_sockaddr_hci addr;
  int fd;
  int ret;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  saved_errno = errno;
  printf("bluez-btmon: hci-socket-%s fd=%d errno=%d\n",
         name, fd, fd < 0 ? saved_errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = HCI_DEV_NONE;
  addr.hci_channel = channel;

  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-btmon: hci-bind-%s ret=%d errno=%d\n",
         name, ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_btmon_send_mgmt(int fd, uint16_t opcode, uint16_t index,
                                 const void *data, uint16_t data_len)
{
  uint8_t buf[32];
  struct bluez_btmon_mgmt_hdr *hdr =
    (struct bluez_btmon_mgmt_hdr *)buf;
  ssize_t ret;

  if (sizeof(*hdr) + data_len > sizeof(buf))
    {
      return -1;
    }

  memset(buf, 0, sizeof(buf));
  hdr->opcode = opcode;
  hdr->index = index;
  hdr->len = data_len;
  if (data_len > 0)
    {
      memcpy(buf + sizeof(*hdr), data, data_len);
    }

  ret = send(fd, buf, sizeof(*hdr) + data_len, 0);
  printf("bluez-btmon: control-send opcode=0x%04x index=0x%04x len=%u ret=%ld errno=%d\n",
         opcode, index, data_len, (long)ret, ret < 0 ? errno : 0);
  return ret < 0 ? -1 : 0;
}

static int bluez_btmon_recv_available(int fd)
{
  uint8_t buf[320];
  struct bluez_btmon_hdr *hdr = (struct bluez_btmon_hdr *)buf;
  unsigned int count = 0;
  unsigned int tries;

  for (tries = 0; tries < 12; tries++)
    {
      ssize_t ret;
      uint16_t event = 0;
      uint16_t index = 0;
      uint16_t len = 0;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-btmon: monitor-recv ret=%ld errno=%d count=%u\n",
                 (long)ret, errno, count);
          break;
        }

      if ((size_t)ret >= sizeof(*hdr))
        {
          event = hdr->opcode;
          index = hdr->index;
          len = hdr->len;
        }

      printf("bluez-btmon: monitor-recv ret=%ld event=0x%04x index=0x%04x len=%u\n",
             (long)ret, event, index, len);
      count++;
    }

  printf("bluez-btmon: monitor-count=%u\n", count);
  return count >= 4 ? 0 : -1;
}

static int bluez_btmon_control(void)
{
  uint8_t powered = 1;
  int mon_fd;
  int ctrl_fd;
  int ret;
  int failed = 0;

  printf("bluez-btmon: source=third/bluez/monitor/bt.h style mode=control\n");

  mon_fd = bluez_btmon_open_hci(HCI_CHANNEL_MONITOR, "monitor");
  if (mon_fd < 0)
    {
      return 1;
    }

  ctrl_fd = bluez_btmon_open_hci(HCI_CHANNEL_CONTROL, "control");
  if (ctrl_fd < 0)
    {
      close(mon_fd);
      return 1;
    }

  failed |= bluez_btmon_send_mgmt(ctrl_fd, MGMT_OP_READ_VERSION,
                                  MGMT_INDEX_NONE, NULL, 0) < 0;
  failed |= bluez_btmon_send_mgmt(ctrl_fd, MGMT_OP_SET_POWERED, 0,
                                  &powered, sizeof(powered)) < 0;

  ret = close(ctrl_fd);
  printf("bluez-btmon: hci-close-control ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  failed |= bluez_btmon_recv_available(mon_fd) < 0;

  ret = close(mon_fd);
  printf("bluez-btmon: hci-close-monitor ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-btmon: control complete\n");
    }

  return failed;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      bluez_btmon_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "control"))
    {
      return bluez_btmon_control();
    }

  bluez_btmon_usage();
  return 1;
}
