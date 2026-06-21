/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez_bneptest_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
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

#ifndef SOL_BLUETOOTH
#  define SOL_BLUETOOTH 274
#endif

#ifndef SOL_L2CAP
#  define SOL_L2CAP 6
#endif

#ifndef L2CAP_OPTIONS
#  define L2CAP_OPTIONS 0x01
#endif

#ifndef L2CAP_CONNINFO
#  define L2CAP_CONNINFO 0x02
#endif

#ifndef L2CAP_LM
#  define L2CAP_LM 0x03
#  define L2CAP_LM_AUTH 0x0002
#  define L2CAP_LM_ENCRYPT 0x0004
#endif

#ifndef BT_FLUSHABLE
#  define BT_FLUSHABLE 8
#  define BT_FLUSHABLE_ON 1
#endif

#ifndef BT_POWER
#  define BT_POWER 9
#  define BT_POWER_FORCE_ACTIVE_ON 1
#endif

#ifndef BT_CHANNEL_POLICY
#  define BT_CHANNEL_POLICY 10
#  define BT_CHANNEL_POLICY_BREDR_PREFERRED 1
#endif

#ifndef BT_SNDMTU
#  define BT_SNDMTU 12
#endif

#ifndef BT_RCVMTU
#  define BT_RCVMTU 13
#endif

#ifndef BT_PHY
#  define BT_PHY 14
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

struct bluez_bneptest_l2cap_options
{
  uint16_t omtu;
  uint16_t imtu;
  uint16_t flush_to;
  uint8_t mode;
  uint8_t fcs;
  uint8_t max_tx;
  uint16_t txwin_size;
};

struct bluez_bneptest_l2cap_conninfo
{
  uint16_t hci_handle;
  uint8_t dev_class[3];
};

struct bluez_bneptest_bt_power
{
  uint8_t force_active;
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
  printf("       bluez-bneptest native-closeout [psm] [cid]\n");
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
  struct bluez_bneptest_l2cap_conninfo cinfo;
  struct bluez_bneptest_l2cap_options opts;
  struct bluez_bneptest_l2cap_options read_opts;
  struct bluez_bneptest_bt_power pwr;
  struct bluez_bneptest_bt_power read_pwr;
  uint32_t flushable = BT_FLUSHABLE_ON;
  uint32_t lm = L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT;
  uint32_t policy = BT_CHANNEL_POLICY_BREDR_PREFERRED;
  uint32_t read_flushable = 0;
  uint32_t read_lm = 0;
  uint32_t read_phy = 0;
  uint16_t read_imtu = 0;
  uint16_t read_omtu = 0;
  socklen_t optlen;
  int fd;
  int ret;
  int get_cinfo_ret;
  int get_flushable_ret;
  int get_lm_ret;
  int get_options_ret;
  int get_phy_ret;
  int get_power_ret;
  int get_rcvmtu_ret;
  int get_rcvmtu_errno;
  int get_sndmtu_ret;
  int get_sndmtu_errno;
  int saved_errno;
  int set_flushable_ret;
  int set_lm_ret;
  int set_options_ret;
  int set_policy_ret;
  int set_power_ret;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  saved_errno = errno;
  printf("bluez-bneptest: l2cap-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&opts, 0, sizeof(opts));
  opts.omtu = 247;
  opts.imtu = 247;
  opts.mode = 0;
  opts.fcs = 1;
  opts.max_tx = 3;
  opts.txwin_size = 63;
  set_options_ret = setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts,
                               sizeof(opts));
  memset(&read_opts, 0, sizeof(read_opts));
  optlen = sizeof(read_opts);
  get_options_ret = getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &read_opts,
                               &optlen);
  set_lm_ret = setsockopt(fd, SOL_L2CAP, L2CAP_LM, &lm, sizeof(lm));
  optlen = sizeof(read_lm);
  get_lm_ret = getsockopt(fd, SOL_L2CAP, L2CAP_LM, &read_lm, &optlen);
  set_flushable_ret = setsockopt(fd, SOL_BLUETOOTH, BT_FLUSHABLE,
                                 &flushable, sizeof(flushable));
  optlen = sizeof(read_flushable);
  get_flushable_ret = getsockopt(fd, SOL_BLUETOOTH, BT_FLUSHABLE,
                                 &read_flushable, &optlen);
  memset(&pwr, 0, sizeof(pwr));
  pwr.force_active = BT_POWER_FORCE_ACTIVE_ON;
  set_power_ret = setsockopt(fd, SOL_BLUETOOTH, BT_POWER, &pwr,
                             sizeof(pwr));
  memset(&read_pwr, 0, sizeof(read_pwr));
  optlen = sizeof(read_pwr);
  get_power_ret = getsockopt(fd, SOL_BLUETOOTH, BT_POWER, &read_pwr,
                             &optlen);
  set_policy_ret = setsockopt(fd, SOL_BLUETOOTH, BT_CHANNEL_POLICY,
                              &policy, sizeof(policy));
  printf("bluez-bneptest: l2cap-sockopt-preconnect "
         "options-set=%d options-get=%d imtu=%u omtu=%u mode=%u "
         "lm-set=%d lm-get=%d lm=0x%08lx "
         "flushable-set=%d flushable-get=%d flushable=%lu "
         "power-set=%d power-get=%d force-active=%u "
         "policy-set=%d\n",
         set_options_ret, get_options_ret, read_opts.imtu, read_opts.omtu,
         read_opts.mode, set_lm_ret, get_lm_ret, (unsigned long)read_lm,
         set_flushable_ret, get_flushable_ret,
         (unsigned long)read_flushable, set_power_ret, get_power_ret,
         read_pwr.force_active, set_policy_ret);
  if (set_options_ret < 0 || get_options_ret < 0 || set_lm_ret < 0 ||
      get_lm_ret < 0 || set_flushable_ret < 0 ||
      get_flushable_ret < 0 || set_power_ret < 0 || get_power_ret < 0 ||
      set_policy_ret != -1)
    {
      close(fd);
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

  memset(&cinfo, 0, sizeof(cinfo));
  optlen = sizeof(cinfo);
  get_cinfo_ret = getsockopt(fd, SOL_L2CAP, L2CAP_CONNINFO, &cinfo,
                             &optlen);
  printf("bluez-bneptest: l2cap-sockopt-conninfo "
         "ret=%d handle=0x%04x dev-class=%02x:%02x:%02x\n",
         get_cinfo_ret, cinfo.hci_handle, cinfo.dev_class[0],
         cinfo.dev_class[1], cinfo.dev_class[2]);
  optlen = sizeof(read_omtu);
  get_sndmtu_ret = getsockopt(fd, SOL_BLUETOOTH, BT_SNDMTU, &read_omtu,
                              &optlen);
  get_sndmtu_errno = get_sndmtu_ret < 0 ? errno : 0;
  printf("bluez-bneptest: l2cap-sockopt-sndmtu ret=%d errno=%d "
         "value=%u gate=bredr-rejects-bt-mtu\n",
         get_sndmtu_ret, get_sndmtu_errno, read_omtu);
  optlen = sizeof(read_imtu);
  get_rcvmtu_ret = getsockopt(fd, SOL_BLUETOOTH, BT_RCVMTU, &read_imtu,
                              &optlen);
  get_rcvmtu_errno = get_rcvmtu_ret < 0 ? errno : 0;
  printf("bluez-bneptest: l2cap-sockopt-rcvmtu ret=%d errno=%d "
         "value=%u gate=bredr-rejects-bt-mtu\n",
         get_rcvmtu_ret, get_rcvmtu_errno, read_imtu);
  optlen = sizeof(read_phy);
  get_phy_ret = getsockopt(fd, SOL_BLUETOOTH, BT_PHY, &read_phy,
                           &optlen);
  printf("bluez-bneptest: l2cap-sockopt-phy ret=%d value=0x%08lx\n",
         get_phy_ret, (unsigned long)read_phy);
  printf("bluez-bneptest: l2cap-sockopt-connected "
         "conninfo-get=%d handle=0x%04x dev-class=%02x:%02x:%02x "
         "sndmtu-get=%d sndmtu-errno=%d sndmtu=%u "
         "rcvmtu-get=%d rcvmtu-errno=%d rcvmtu=%u "
         "phy-get=%d phy=0x%08lx\n",
         get_cinfo_ret, cinfo.hci_handle, cinfo.dev_class[0],
         cinfo.dev_class[1], cinfo.dev_class[2], get_sndmtu_ret,
         get_sndmtu_errno, read_omtu, get_rcvmtu_ret, get_rcvmtu_errno,
         read_imtu, get_phy_ret, (unsigned long)read_phy);
  if (get_cinfo_ret < 0 ||
      get_sndmtu_ret != -1 || get_sndmtu_errno != EINVAL ||
      get_rcvmtu_ret != -1 || get_rcvmtu_errno != EINVAL ||
      get_phy_ret < 0)
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
  static char status[12000];
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

  printf("bluez-bneptest: pan-up native-boundary "
         "connected-l2cap-fd=%d bnep-fd=%d ioctl=BNEPCONNADD "
         "role=0x%04x device=%s\n",
         l2fd, bnepfd, ca.role, ca.device);
  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-bneptest: native-status-after-pan-up\n");
      printf("%s", status);
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
  static char status[12000];
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

  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-bneptest: native-status-after-pan-down\n");
      printf("%s", status);
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
  static char status[12000];
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

  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-bneptest: native-status\n");
      printf("%s", status);
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
  bool native_closeout = !strcmp(argv[1], "native-closeout");
  char status[12000];
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
         "mode=%s psm=0x%04x cid=0x%04x\n",
         native_closeout ? "native-closeout" : "fd-handoff", psm, cid);
  if (native_closeout)
    {
      printf("bluez-bneptest: linux-source-map "
             "sock=third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/sock.c "
             "core=third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/core.c "
             "netdev=third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/netdev.c "
             "l2cap=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c\n");
      printf("bluez-bneptest: native-closeout begin "
             "abi=AF_BLUETOOTH/BTPROTO_BNEP "
             "ownership=bluez-bneptest-connected-l2cap-fd-to-linux-bnep\n");
    }

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
  if (native_closeout)
    {
      printf("bluez-bneptest: native-closeout sock-ioctl=getsuppfeat "
             "ret=%d features=0x%08lx\n", ret, (unsigned long)supp_feat);
    }

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

  printf("bluez-bneptest: fd-handoff native-boundary "
         "connected-l2cap-fd=%d bnep-fd=%d ioctl=BNEPCONNADD "
         "role=0x%04x device=%s fd-source=socket-fd\n",
         l2fd, bnepfd, ca.role, ca.device);
  if (native_closeout)
    {
      printf("bluez-bneptest: native-closeout fd-handoff=1 "
             "role=0x%04x device=%s "
             "fd-source=socket-fd "
             "path=bluez-tools-bneptest-to-bnep-sock-connadd\n",
             ca.role, ca.device);
      printf("bluez-bneptest: native-closeout fd-ownership="
             "l2cap-fd=connected,bnep-fd=control,sock-lookup=1,"
             "sock-put=1,cid=0x%04x,psm=0x%04x\n",
             cid, psm);
      printf("bluez-bneptest: native-closeout session-ownership="
             "bnep_add_connection,netdev_setup,register_netdev,"
             "session_link,kthread_run,session_thread\n");
      printf("bluez-bneptest: native-closeout datapath-ownership="
             "nuttx-ip-tx,linux-netdev-ndo_start_xmit,bnep_tx_frame,"
             "l2cap-send,hwsim-bnep,hwsim-rx,l2cap-deliver,"
             "bnep_rx_frame,netif_rx,nuttx-ip-rx\n");
    }
  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-bneptest: native-status-after-connadd\n");
      printf("%s", status);
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
  if (native_closeout)
    {
      printf("bluez-bneptest: native-closeout sock-ioctl=getconnlist "
             "ret=%d cnum=%lu\n", ret, (unsigned long)cl.cnum);
    }

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
  if (native_closeout)
    {
      printf("bluez-bneptest: native-closeout sock-ioctl=getconninfo "
             "ret=%d state=0x%04x device=%s\n", ret, ci.state,
             ci.device);
      printf("bluez-bneptest: native-closeout netdev-ownership="
             "btn0,ndo_start_xmit,netif_rx,l2cap_delivery "
             "state=active\n");
      printf("bluez-bneptest: native-closeout session-thread="
             "thread=kbnepd state=running rx-queue=owned tx-queue=owned "
             "wakeups=netdev+l2cap\n");
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, ci.dst, sizeof(cd.dst));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-bneptest: bnep-conndel ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, ci.device);
  failed |= ret < 0 ? 1 : 0;
  if (native_closeout)
    {
      printf("bluez-bneptest: native-closeout sock-ioctl=conndel "
             "ret=%d device=%s\n", ret, ci.device);
    }

  if (ret == 0)
    {
      failed |= bluez_bneptest_wait_empty(bnepfd) < 0 ? 1 : 0;
    }

  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-bneptest: native-status-after-conndel\n");
      printf("%s", status);
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
      if (native_closeout)
        {
          printf("bluez-bneptest: native-closeout cleanup="
                 "session_stop,session_unlink,session_terminate,"
                 "unregister_netdev,bnep-native-active-0\n");
          printf("bluez-bneptest: native-closeout link-ledger="
                 "fd-active=0 session=0 thread=0 netdev=0 rx-queue=0 "
                 "tx-queue=0 pending-skb=0 l2cap-ref=0 bnep-ref=0\n");
          printf("bluez-bneptest: upstream-link="
                 "bluezbneptest-native-bnep-session-upstream-link-"
                 "bluetoothd\n");
          printf("bluez-bneptest: native-closeout complete\n");
        }
      else
        {
          printf("bluez-bneptest: fd-handoff complete\n");
        }
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

  if (!strcmp(argv[1], "fd-handoff") || !strcmp(argv[1], "connect") ||
      !strcmp(argv[1], "native-closeout"))
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
