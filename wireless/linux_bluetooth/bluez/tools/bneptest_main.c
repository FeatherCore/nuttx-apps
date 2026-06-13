/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez_bneptest_main.c
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

#ifndef BNEPCONNADD
#  define BNEPCONNADD      0x42c8
#  define BNEPCONNDEL      0x42c9
#  define BNEPGETCONNLIST  0x42d2
#  define BNEPGETCONNINFO  0x42d3
#  define BNEPGETSUPPFEAT  0x42d4
#endif

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_L2CAP
#  define BTPROTO_L2CAP 0
#endif

#ifndef BTPROTO_BNEP
#  define BTPROTO_BNEP 4
#endif

#ifndef BT_PSM_BNEP
#  define BT_PSM_BNEP 0x000f
#endif

#ifndef ETH_ALEN
#  define ETH_ALEN 6
#endif

#define BLUEZ_BNEPTEST_PANU_ROLE 0x1115

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_bneptest_sockaddr_l2
{
  sa_family_t l2_family;
  uint16_t l2_psm;
  uint8_t l2_bdaddr[6];
  uint16_t l2_cid;
  uint8_t l2_bdaddr_type;
};

struct bluez_bneptest_connadd_req
{
  int sock;
  uint32_t flags;
  uint16_t role;
  char device[16];
};

struct bluez_bneptest_conndel_req
{
  uint32_t flags;
  uint8_t dst[ETH_ALEN];
};

struct bluez_bneptest_conninfo
{
  uint32_t flags;
  uint16_t role;
  uint16_t state;
  uint8_t dst[ETH_ALEN];
  char device[16];
};

struct bluez_bneptest_connlist_req
{
  uint32_t cnum;
  struct bluez_bneptest_conninfo *ci;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_bneptest_usage(void)
{
  printf("usage: bluez-bneptest fd-handoff [psm] [cid]\n");
  printf("       bluez-bneptest connect [psm] [cid]\n");
  printf("       bluez-bneptest pan-up [psm] [cid]\n");
  printf("       bluez-bneptest pan-down\n");
  printf("       bluez-bneptest status\n");
  printf("\n");
  printf("This is the first NuttX apps-side BlueZ bneptest adapter.\n");
  printf("It preserves the BlueZ network-profile fd handoff shape:\n");
  printf("connected L2CAP fd -> BNEPCONNADD -> BNEP netdev session.\n");
}

static int bluez_bneptest_open_l2cap(uint16_t psm, uint16_t cid)
{
  struct bluez_bneptest_sockaddr_l2 addr;
  int fd;
  int ret;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  saved_errno = errno;
  printf("bluez-bneptest: l2cap-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.l2_family = AF_BLUETOOTH;
  addr.l2_psm = psm;
  addr.l2_cid = cid;

  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-bneptest: l2cap-bind ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-bneptest: l2cap-connect ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_bneptest_open_bnep(void)
{
  int fd;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  return fd;
}

static int bluez_bneptest_wait_empty(int bnepfd)
{
  struct bluez_bneptest_conninfo list[4];
  struct bluez_bneptest_connlist_req cl;
  int attempt;
  int ret = 0;
  int saved_errno = 0;

  for (attempt = 0; attempt < 10; attempt++)
    {
      memset(list, 0, sizeof(list));
      memset(&cl, 0, sizeof(cl));
      cl.cnum = sizeof(list) / sizeof(list[0]);
      cl.ci = list;
      ret = ioctl(bnepfd, BNEPGETCONNLIST, (unsigned long)&cl);
      saved_errno = errno;
      if (ret < 0 || cl.cnum == 0)
        {
          break;
        }

      usleep(10000);
    }

  printf("bluez-bneptest: bnep-connlist-postdel ret=%d errno=%d "
         "cnum=%lu attempts=%d\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
         attempt < 10 ? attempt + 1 : attempt);
  return (ret == 0 && cl.cnum == 0) ? 0 : -1;
}

static int bluez_bneptest_get_first_conn(int bnepfd,
                                         struct bluez_bneptest_conninfo *info)
{
  struct bluez_bneptest_conninfo list[4];
  struct bluez_bneptest_connlist_req cl;
  int ret;
  int saved_errno;

  memset(list, 0, sizeof(list));
  memset(&cl, 0, sizeof(cl));
  cl.cnum = sizeof(list) / sizeof(list[0]);
  cl.ci = list;

  ret = ioctl(bnepfd, BNEPGETCONNLIST, (unsigned long)&cl);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-connlist ret=%d errno=%d cnum=%lu\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum);
  if (ret < 0 || cl.cnum == 0)
    {
      return ret < 0 ? ret : -ENOENT;
    }

  if (info != NULL)
    {
      *info = list[0];
    }

  return 0;
}

static int bluez_bneptest_pan_up(int argc, char *argv[])
{
  struct bluez_bneptest_connadd_req ca;
  uint16_t psm = 0x0019;
  uint16_t cid = 0x0041;
  int bnepfd;
  int l2fd;
  int ret;
  int saved_errno;

  if (argc > 2)
    {
      psm = (uint16_t)strtoul(argv[2], NULL, 0);
    }

  if (argc > 3)
    {
      cid = (uint16_t)strtoul(argv[3], NULL, 0);
    }

  printf("bluez-bneptest: source=third/bluez/tools/bneptest.c "
         "mode=pan-up psm=0x%04x cid=0x%04x\n", psm, cid);

  l2fd = bluez_bneptest_open_l2cap(psm, cid);
  if (l2fd < 0)
    {
      return 1;
    }

  bnepfd = bluez_bneptest_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = BLUEZ_BNEPTEST_PANU_ROLE;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-connadd-fd ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ca.device);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  ret = close(bnepfd);
  printf("bluez-bneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  ret = close(l2fd);
  printf("bluez-bneptest: l2cap-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);

  printf("bluez-bneptest: pan-up complete\n");
  return 0;
}

static int bluez_bneptest_pan_down(void)
{
  struct bluez_bneptest_conndel_req cd;
  struct bluez_bneptest_conninfo info;
  int bnepfd;
  int ret;
  int saved_errno;
  int failed = 0;

  printf("bluez-bneptest: pan-down\n");

  bnepfd = bluez_bneptest_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = bluez_bneptest_get_first_conn(bnepfd, &info);
  if (ret < 0)
    {
      close(bnepfd);
      return 1;
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, info.dst, sizeof(cd.dst));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-conndel-fd ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, info.device);
  failed |= ret < 0 ? 1 : 0;

  if (ret == 0)
    {
      failed |= bluez_bneptest_wait_empty(bnepfd) < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-bneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-bneptest: pan-down complete\n");
    }

  return failed;
}

static int bluez_bneptest_status(void)
{
  struct bluez_bneptest_conninfo info;
  int bnepfd;
  int ret;
  int saved_errno;
  int failed = 0;

  bnepfd = bluez_bneptest_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = bluez_bneptest_get_first_conn(bnepfd, &info);
  failed |= ret < 0 ? 1 : 0;
  if (ret == 0)
    {
      ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&info);
      saved_errno = errno;
      printf("bluez-bneptest: bnep-conninfo-fd ret=%d errno=%d "
             "role=0x%04x state=0x%04x device=%s\n",
             ret, ret < 0 ? saved_errno : 0, info.role, info.state,
             info.device);
      failed |= ret < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-bneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  return failed;
}

static int bluez_bneptest_fd_handoff(int argc, char *argv[])
{
  struct bluez_bneptest_connadd_req ca;
  struct bluez_bneptest_conndel_req cd;
  struct bluez_bneptest_conninfo list[4];
  struct bluez_bneptest_conninfo ci;
  struct bluez_bneptest_connlist_req cl;
  uint16_t psm = BT_PSM_BNEP;
  uint16_t cid = 0x0041;
  uint32_t supp_feat = 0;
  int saved_errno;
  int l2fd;
  int bnepfd;
  int ret;
  int failed = 0;

  if (argc > 2)
    {
      psm = (uint16_t)strtoul(argv[2], NULL, 0);
    }

  if (argc > 3)
    {
      cid = (uint16_t)strtoul(argv[3], NULL, 0);
    }

  printf("bluez-bneptest: source=third/bluez/tools/bneptest.c "
         "mode=fd-handoff psm=0x%04x cid=0x%04x\n", psm, cid);

  l2fd = bluez_bneptest_open_l2cap(psm, cid);
  if (l2fd < 0)
    {
      return 1;
    }

  bnepfd = bluez_bneptest_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  ret = ioctl(bnepfd, BNEPGETSUPPFEAT, (unsigned long)&supp_feat);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-suppfeat ret=%d errno=%d features=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)supp_feat);
  failed |= ret < 0 ? 1 : 0;

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = BLUEZ_BNEPTEST_PANU_ROLE;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-connadd ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ca.device);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(list, 0, sizeof(list));
  memset(&cl, 0, sizeof(cl));
  cl.cnum = sizeof(list) / sizeof(list[0]);
  cl.ci = list;
  ret = ioctl(bnepfd, BNEPGETCONNLIST, (unsigned long)&cl);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-connlist ret=%d errno=%d cnum=%lu "
         "state=0x%04x device=%s\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
         cl.cnum > 0 ? list[0].state : 0,
         cl.cnum > 0 ? list[0].device : "");
  failed |= (ret < 0 || cl.cnum == 0) ? 1 : 0;

  memset(&ci, 0, sizeof(ci));
  if (cl.cnum > 0)
    {
      ci = list[0];
    }

  ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&ci);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-conninfo ret=%d errno=%d state=0x%04x "
         "device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.state, ci.device);
  failed |= ret < 0 ? 1 : 0;

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, ci.dst, sizeof(cd.dst));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-conndel ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.device);
  failed |= ret < 0 ? 1 : 0;

  if (ret == 0)
    {
      failed |= bluez_bneptest_wait_empty(bnepfd) < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-bneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  ret = close(l2fd);
  printf("bluez-bneptest: l2cap-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-bneptest: fd-handoff complete\n");
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
      bluez_bneptest_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "fd-handoff") || !strcmp(argv[1], "connect"))
    {
      return bluez_bneptest_fd_handoff(argc, argv);
    }

  if (!strcmp(argv[1], "pan-up"))
    {
      return bluez_bneptest_pan_up(argc, argv);
    }

  if (!strcmp(argv[1], "pan-down"))
    {
      return bluez_bneptest_pan_down();
    }

  if (!strcmp(argv[1], "status"))
    {
      return bluez_bneptest_status();
    }

  bluez_bneptest_usage();
  return 1;
}
