/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/hciioctl_main.c
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
#include <sys/ioctl.h>
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

#define HCIDEVUP       0x400448c9
#define HCIDEVDOWN     0x400448ca
#define HCIDEVRESET    0x400448cb
#define HCIDEVRESTAT   0x400448cc
#define HCIGETDEVLIST  0x800448d2
#define HCIGETDEVINFO  0x800448d3

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_hci_dev_req
{
  uint16_t dev_id;
  uint32_t dev_opt;
};

struct bluez_hci_dev_list_req_one
{
  uint16_t dev_num;
  struct bluez_hci_dev_req dev_req[4];
};

struct bluez_hci_dev_stats
{
  uint32_t err_rx;
  uint32_t err_tx;
  uint32_t cmd_tx;
  uint32_t evt_rx;
  uint32_t acl_tx;
  uint32_t acl_rx;
  uint32_t sco_tx;
  uint32_t sco_rx;
  uint32_t byte_rx;
  uint32_t byte_tx;
};

struct bluez_hci_dev_info
{
  uint16_t dev_id;
  char name[16];
  uint8_t bdaddr[6];
  uint32_t flags;
  uint8_t type;
  uint8_t features[8];
  uint32_t pkt_type;
  uint32_t link_policy;
  uint32_t link_mode;
  uint16_t acl_mtu;
  uint16_t acl_pkts;
  uint16_t sco_mtu;
  uint16_t sco_pkts;
  struct bluez_hci_dev_stats stat;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_hciioctl_usage(void)
{
  printf("usage: bluezhciioctl basic\n");
  printf("\n");
  printf("BlueZ hciconfig-style HCI ioctl smoke over AF_BLUETOOTH/BTPROTO_HCI.\n");
}

static int bluez_hciioctl_call(int fd, unsigned long req, unsigned long arg,
                               const char *name)
{
  int ret;
  int saved_errno;

  errno = 0;
  ret = ioctl(fd, req, arg);
  saved_errno = errno;
  printf("bluez-hciioctl: ioctl-%s ret=%d errno=%d\n",
         name, ret, ret < 0 ? saved_errno : 0);
  return ret;
}

static int bluez_hciioctl_call_devup(int fd, uint16_t dev,
                                     const char *name)
{
  int ret;

  ret = bluez_hciioctl_call(fd, HCIDEVUP, dev, name);
  if (ret < 0 && errno == EALREADY)
    {
      printf("bluez-hciioctl: ioctl-%s already-up accepted\n", name);
      return 0;
    }

  return ret;
}

static int bluez_hciioctl_basic(void)
{
  struct bluez_hci_dev_list_req_one list;
  struct bluez_hci_dev_info info;
  uint16_t dev = 0;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-hciioctl: source=third/bluez/tools/hciconfig style mode=basic\n");

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  printf("bluez-hciioctl: hci-socket fd=%d errno=%d\n",
         fd, fd < 0 ? errno : 0);
  if (fd < 0)
    {
      return 1;
    }

  memset(&list, 0, sizeof(list));
  list.dev_num = 4;
  ret = bluez_hciioctl_call(fd, HCIGETDEVLIST,
                            (unsigned long)(uintptr_t)&list,
                            "getdevlist");
  failed |= ret < 0 ? 1 : 0;
  printf("bluez-hciioctl: devlist num=%u dev0-id=%u dev0-opt=0x%08lx\n",
         list.dev_num, list.dev_req[0].dev_id,
         (unsigned long)list.dev_req[0].dev_opt);

  if (list.dev_num > 0)
    {
      dev = list.dev_req[0].dev_id;
    }

  memset(&info, 0, sizeof(info));
  info.dev_id = dev;
  ret = bluez_hciioctl_call(fd, HCIGETDEVINFO,
                            (unsigned long)(uintptr_t)&info,
                            "getdevinfo");
  failed |= ret < 0 ? 1 : 0;
  printf("bluez-hciioctl: devinfo id=%u name=%s flags=0x%08lx type=%u acl-mtu=%u acl-pkts=%u\n",
         info.dev_id, info.name, (unsigned long)info.flags, info.type,
         info.acl_mtu, info.acl_pkts);

  failed |= bluez_hciioctl_call_devup(fd, dev, "devup") < 0;
  failed |= bluez_hciioctl_call(fd, HCIDEVRESTAT, dev, "devrestat") < 0;
  failed |= bluez_hciioctl_call(fd, HCIDEVRESET, dev, "devreset") < 0;
  failed |= bluez_hciioctl_call(fd, HCIDEVDOWN, dev, "devdown") < 0;
  failed |= bluez_hciioctl_call_devup(fd, dev, "devup-final") < 0;

  ret = close(fd);
  printf("bluez-hciioctl: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciioctl: basic complete\n");
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
      bluez_hciioctl_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "basic"))
    {
      return bluez_hciioctl_basic();
    }

  bluez_hciioctl_usage();
  return 1;
}
