/****************************************************************************
 * apps/wireless/linux_bluetooth/btbneptest_main.c
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

#include <nuttx/wireless/linux_bluetooth.h>

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

#define BTBNEPTEST_PANU_ROLE 0x1115

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct btbneptest_connadd_req
{
  int sock;
  uint32_t flags;
  uint16_t role;
  char device[16];
};

struct btbneptest_sockaddr_l2
{
  sa_family_t l2_family;
  uint16_t l2_psm;
  uint8_t l2_bdaddr[6];
  uint16_t l2_cid;
  uint8_t l2_bdaddr_type;
};

struct btbneptest_conndel_req
{
  uint32_t flags;
  uint8_t dst[ETH_ALEN];
};

struct btbneptest_conninfo
{
  uint32_t flags;
  uint16_t role;
  uint16_t state;
  uint8_t dst[ETH_ALEN];
  char device[16];
};

struct btbneptest_connlist_req
{
  uint32_t cnum;
  struct btbneptest_conninfo *ci;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void btbneptest_usage(void)
{
  printf("usage: btbneptest <command> [args]\n");
  printf("\n");
  printf("commands:\n");
  printf("  pan-up [psm] [cid] [handle]\n");
  printf("  pan-down\n");
  printf("  status\n");
  printf("  fd-probe [psm] [cid]\n");
  printf("\n");
  printf("defaults:\n");
  printf("  psm=0x0019 cid=0x0041 handle=0x0040\n");
}

static int btbneptest_print_step(const char *step, int ret,
                                 const char *out)
{
  printf("btbneptest: %s ret=%d\n", step, ret);
  if (out != NULL && out[0] != '\0')
    {
      printf("%s", out);
    }

  return ret < 0 ? 1 : 0;
}

static int btbneptest_open_l2cap(uint16_t psm, uint16_t cid)
{
  struct btbneptest_sockaddr_l2 addr;
  int fd;
  int ret;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  printf("btbneptest: l2cap-socket fd=%d errno=%d\n",
         fd, fd < 0 ? errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.l2_family = AF_BLUETOOTH;
  addr.l2_psm = psm;
  addr.l2_cid = cid;
  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  printf("btbneptest: l2cap-bind ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
  printf("btbneptest: l2cap-connect ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int btbneptest_open_bnep(void)
{
  int fd;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
  printf("btbneptest: bnep-socket fd=%d errno=%d\n",
         fd, fd < 0 ? errno : 0);
  return fd;
}

static int btbneptest_get_first_conn(int bnepfd,
                                     struct btbneptest_conninfo *info)
{
  struct btbneptest_conninfo list[4];
  struct btbneptest_connlist_req cl;
  int ret;

  memset(list, 0, sizeof(list));
  memset(&cl, 0, sizeof(cl));
  cl.cnum = sizeof(list) / sizeof(list[0]);
  cl.ci = list;

  ret = ioctl(bnepfd, BNEPGETCONNLIST, (unsigned long)&cl);
  printf("btbneptest: bnep-connlist ret=%d errno=%d cnum=%lu\n",
         ret, ret < 0 ? errno : 0, (unsigned long)cl.cnum);
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

static int btbneptest_pan_up(int argc, char *argv[])
{
  struct btbneptest_connadd_req ca;
  uint16_t psm = 0x0019;
  uint16_t cid = 0x0041;
  uint16_t handle = 0x0040;
  int bnepfd;
  int l2fd;
  int ret;

  if (argc > 2)
    {
      psm = (uint16_t)strtoul(argv[2], NULL, 0);
    }

  if (argc > 3)
    {
      cid = (uint16_t)strtoul(argv[3], NULL, 0);
    }

  if (argc > 4)
    {
      handle = (uint16_t)strtoul(argv[4], NULL, 0);
    }

  printf("btbneptest: pan-up psm=0x%04x cid=0x%04x handle=0x%04x\n",
         psm, cid, handle);

  l2fd = btbneptest_open_l2cap(psm, cid);
  if (l2fd < 0)
    {
      return 1;
    }

  bnepfd = btbneptest_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = BTBNEPTEST_PANU_ROLE;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  printf("btbneptest: bnep-connadd-fd ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? errno : 0, ca.device);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  ret = close(bnepfd);
  printf("btbneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  ret = close(l2fd);
  printf("btbneptest: l2cap-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);

  printf("btbneptest: pan-up complete\n");
  return 0;
}

static int btbneptest_pan_down(void)
{
  struct btbneptest_conndel_req cd;
  struct btbneptest_conninfo info;
  struct btbneptest_conninfo list[4];
  struct btbneptest_connlist_req cl;
  int saved_errno;
  int bnepfd;
  int attempt;
  int ret;
  int failed = 0;

  printf("btbneptest: pan-down\n");

  bnepfd = btbneptest_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = btbneptest_get_first_conn(bnepfd, &info);
  if (ret < 0)
    {
      close(bnepfd);
      return 1;
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, info.dst, sizeof(cd.dst));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  printf("btbneptest: bnep-conndel-fd ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? errno : 0, info.device);
  failed |= ret < 0 ? 1 : 0;

  if (ret == 0)
    {
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

      printf("btbneptest: bnep-connlist-postdel ret=%d errno=%d "
             "cnum=%lu attempts=%d\n",
             ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
             attempt < 10 ? attempt + 1 : attempt);
      failed |= (ret < 0 || cl.cnum != 0) ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("btbneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("btbneptest: pan-down complete\n");
    }

  return failed;
}

static int btbneptest_status(void)
{
  struct btbneptest_conninfo info;
  int bnepfd;
  int ret;
  int failed = 0;

  bnepfd = btbneptest_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = btbneptest_get_first_conn(bnepfd, &info);
  failed |= ret < 0 ? 1 : 0;
  if (ret == 0)
    {
      ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&info);
      printf("btbneptest: bnep-conninfo-fd ret=%d errno=%d "
             "role=0x%04x state=0x%04x device=%s\n",
             ret, ret < 0 ? errno : 0, info.role, info.state,
             info.device);
      failed |= ret < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("btbneptest: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  return failed;
}

static int btbneptest_fd_probe(int argc, char *argv[])
{
  struct btbneptest_connadd_req ca;
  struct btbneptest_conndel_req cd;
  struct btbneptest_conninfo ci;
  struct btbneptest_conninfo list[4];
  struct btbneptest_connlist_req cl;
  struct btbneptest_sockaddr_l2 laddr;
  struct btbneptest_sockaddr_l2 raddr;
  uint16_t psm = BT_PSM_BNEP;
  uint16_t cid = 0x0041;
  uint32_t supp_feat = 0;
  int saved_errno;
  int l2fd;
  int bnepfd;
  int attempt;
  int ret;

  if (argc > 2)
    {
      psm = (uint16_t)strtoul(argv[2], NULL, 0);
    }

  if (argc > 3)
    {
      cid = (uint16_t)strtoul(argv[3], NULL, 0);
    }

  printf("btbneptest: fd-probe psm=0x%04x cid=0x%04x\n", psm, cid);

  l2fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  saved_errno = errno;
  printf("btbneptest: fd-probe l2cap-socket fd=%d errno=%d\n",
         l2fd, l2fd < 0 ? saved_errno : 0);
  if (l2fd < 0)
    {
      return 1;
    }

  memset(&laddr, 0, sizeof(laddr));
  laddr.l2_family = AF_BLUETOOTH;
  laddr.l2_psm = psm;
  laddr.l2_cid = cid;
  ret = bind(l2fd, (struct sockaddr *)&laddr, sizeof(laddr));
  saved_errno = errno;
  printf("btbneptest: fd-probe l2cap-bind ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);

  memset(&raddr, 0, sizeof(raddr));
  raddr.l2_family = AF_BLUETOOTH;
  raddr.l2_psm = psm;
  raddr.l2_cid = cid;
  ret = connect(l2fd, (struct sockaddr *)&raddr, sizeof(raddr));
  saved_errno = errno;
  printf("btbneptest: fd-probe l2cap-connect ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);

  bnepfd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-socket fd=%d errno=%d\n",
         bnepfd, bnepfd < 0 ? saved_errno : 0);
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  ret = ioctl(bnepfd, BNEPGETSUPPFEAT, (unsigned long)&supp_feat);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-suppfeat ret=%d errno=%d "
         "features=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)supp_feat);
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
  printf("btbneptest: fd-probe bnep-connlist-empty ret=%d errno=%d "
         "cnum=%lu\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum);
  if (ret < 0 || cl.cnum != 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(&cd, 0, sizeof(cd));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-conndel-missing ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret >= 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(&ci, 0, sizeof(ci));
  ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&ci);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-conninfo-missing ret=%d errno=%d "
         "state=0x%04x device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.state, ci.device);
  if (ret >= 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(&ca, 0, sizeof(ca));
  ca.sock = -1;
  ca.role = 0x1115;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-connadd-invalid ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret >= 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = 0x1115;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-connadd ret=%d errno=%d device=%s\n",
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
  printf("btbneptest: fd-probe bnep-connlist-postadd ret=%d errno=%d "
         "cnum=%lu state=0x%04x device=%s\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
         cl.cnum > 0 ? list[0].state : 0,
         cl.cnum > 0 ? list[0].device : "");
  if (ret < 0 || cl.cnum == 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  ci = list[0];
  ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&ci);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-conninfo-postadd ret=%d errno=%d "
         "state=0x%04x device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.state, ci.device);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, ci.dst, sizeof(cd.dst));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("btbneptest: fd-probe bnep-conndel-postadd ret=%d errno=%d "
         "device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.device);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

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

  printf("btbneptest: fd-probe bnep-connlist-postdel ret=%d errno=%d "
         "cnum=%lu attempts=%d\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
         attempt < 10 ? attempt + 1 : attempt);
  if (ret < 0 || cl.cnum != 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  close(bnepfd);
  close(l2fd);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      btbneptest_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "pan-up"))
    {
      return btbneptest_pan_up(argc, argv);
    }

  if (!strcmp(argv[1], "pan-down"))
    {
      return btbneptest_pan_down();
    }

  if (!strcmp(argv[1], "status"))
    {
      return btbneptest_status();
    }

  if (!strcmp(argv[1], "fd-probe"))
    {
      return btbneptest_fd_probe(argc, argv);
    }

  btbneptest_usage();
  return 2;
}
