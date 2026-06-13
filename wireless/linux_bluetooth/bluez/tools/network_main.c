/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/network_main.c
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

#define BLUEZ_NETWORK_BNEP_SETUP_RESPONSE 0
#define BLUEZ_NETWORK_FLAG_SETUP_RESPONSE \
  (1u << BLUEZ_NETWORK_BNEP_SETUP_RESPONSE)

#define BLUEZ_NETWORK_SVC_PANU 0x1115
#define BLUEZ_NETWORK_SVC_NAP  0x1116
#define BLUEZ_NETWORK_SVC_GN   0x1117

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_network_sockaddr_l2
{
  sa_family_t l2_family;
  uint16_t l2_psm;
  uint8_t l2_bdaddr[6];
  uint16_t l2_cid;
  uint8_t l2_bdaddr_type;
};

struct bluez_network_connadd_req
{
  int sock;
  uint32_t flags;
  uint16_t role;
  char device[16];
};

struct bluez_network_conndel_req
{
  uint32_t flags;
  uint8_t dst[ETH_ALEN];
};

struct bluez_network_conninfo
{
  uint32_t flags;
  uint16_t role;
  uint16_t state;
  uint8_t dst[ETH_ALEN];
  char device[16];
};

struct bluez_network_connlist_req
{
  uint32_t cnum;
  struct bluez_network_conninfo *ci;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_network_usage(void)
{
  printf("usage: blueznetwork connect [panu|nap|gn] [psm] [cid]\n");
  printf("       blueznetwork disconnect\n");
  printf("       blueznetwork status\n");
  printf("       blueznetwork error-path\n");
  printf("       blueznetwork daemon-profile register|connect|status|disconnect|error-path|unregister [panu|nap|gn]\n");
  printf("\n");
  printf("BlueZ Network Profile-shaped adapter over the Linux BNEP ABI.\n");
  printf("It follows profiles/network/connection.c + bnep.c shape:\n");
  printf("profile connect -> connected L2CAP fd -> BNEPCONNADD -> btnX.\n");
}

static uint16_t bluez_network_role_from_service(const char *service)
{
  if (service == NULL || !strcmp(service, "panu"))
    {
      return BLUEZ_NETWORK_SVC_PANU;
    }

  if (!strcmp(service, "nap"))
    {
      return BLUEZ_NETWORK_SVC_NAP;
    }

  if (!strcmp(service, "gn"))
    {
      return BLUEZ_NETWORK_SVC_GN;
    }

  return 0;
}

static int bluez_network_open_l2cap(uint16_t psm, uint16_t cid)
{
  struct bluez_network_sockaddr_l2 addr;
  int fd;
  int ret;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
  saved_errno = errno;
  printf("bluez-network: l2cap-socket fd=%d errno=%d\n",
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
  printf("bluez-network: l2cap-bind ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-network: l2cap-connect ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_network_open_bnep(void)
{
  int fd;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
  saved_errno = errno;
  printf("bluez-network: bnep-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  return fd;
}

static int bluez_network_get_first_conn(int bnepfd,
                                        struct bluez_network_conninfo *info)
{
  struct bluez_network_conninfo list[4];
  struct bluez_network_connlist_req cl;
  int ret;
  int saved_errno;

  memset(list, 0, sizeof(list));
  memset(&cl, 0, sizeof(cl));
  cl.cnum = sizeof(list) / sizeof(list[0]);
  cl.ci = list;

  ret = ioctl(bnepfd, BNEPGETCONNLIST, (unsigned long)&cl);
  saved_errno = errno;
  printf("bluez-network: connlist ret=%d errno=%d cnum=%lu\n",
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

static int bluez_network_wait_empty(int bnepfd)
{
  struct bluez_network_conninfo list[4];
  struct bluez_network_connlist_req cl;
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

  printf("bluez-network: connlist-postdel ret=%d errno=%d cnum=%lu "
         "attempts=%d\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)cl.cnum,
         attempt < 10 ? attempt + 1 : attempt);
  return (ret == 0 && cl.cnum == 0) ? 0 : -1;
}

static int bluez_network_connect(int argc, char *argv[])
{
  struct bluez_network_connadd_req ca;
  uint16_t role;
  uint16_t psm = BT_PSM_BNEP;
  uint16_t cid = 0x0041;
  uint32_t supp_feat = 0;
  const char *service = argc > 2 ? argv[2] : "panu";
  int bnepfd;
  int l2fd;
  int ret;
  int saved_errno;
  int failed = 0;

  role = bluez_network_role_from_service(service);
  if (role == 0)
    {
      printf("bluez-network: unknown service=%s\n", service);
      return 1;
    }

  if (argc > 3)
    {
      psm = (uint16_t)strtoul(argv[3], NULL, 0);
    }

  if (argc > 4)
    {
      cid = (uint16_t)strtoul(argv[4], NULL, 0);
    }

  printf("bluez-network: source=third/bluez/profiles/network/"
         "connection.c+profiles/network/bnep.c mode=connect "
         "service=%s role=0x%04x psm=0x%04x cid=0x%04x\n",
         service, role, psm, cid);
  printf("bluez-network: profile interface=org.bluez.Network1 "
         "state=connecting\n");

  l2fd = bluez_network_open_l2cap(psm, cid);
  if (l2fd < 0)
    {
      return 1;
    }

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  ret = ioctl(bnepfd, BNEPGETSUPPFEAT, (unsigned long)&supp_feat);
  saved_errno = errno;
  printf("bluez-network: suppfeat ret=%d errno=%d features=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)supp_feat);
  failed |= ret < 0 ? 1 : 0;

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = role;
  ca.flags = BLUEZ_NETWORK_FLAG_SETUP_RESPONSE;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("bluez-network: bnep-connadd ret=%d errno=%d device=%s "
         "role=0x%04x flags=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, ca.device, ca.role,
         (unsigned long)ca.flags);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  printf("bluez-network: profile connected interface=%s uuid=0x%04x\n",
         ca.device, role);

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  ret = close(l2fd);
  printf("bluez-network: l2cap-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-network: connect complete\n");
    }

  return failed;
}

static int bluez_network_disconnect(void)
{
  struct bluez_network_conndel_req cd;
  struct bluez_network_conninfo info;
  int bnepfd;
  int ret;
  int saved_errno;
  int failed = 0;

  printf("bluez-network: mode=disconnect\n");

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = bluez_network_get_first_conn(bnepfd, &info);
  if (ret < 0)
    {
      close(bnepfd);
      return 1;
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, info.dst, sizeof(cd.dst));

  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-network: bnep-conndel ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, info.device);
  failed |= ret < 0 ? 1 : 0;

  if (ret == 0)
    {
      failed |= bluez_network_wait_empty(bnepfd) < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-network: profile disconnected\n");
      printf("bluez-network: disconnect complete\n");
    }

  return failed;
}

static int bluez_network_status(void)
{
  struct bluez_network_conninfo info;
  int bnepfd;
  int ret;
  int saved_errno;
  int failed = 0;

  printf("bluez-network: mode=status\n");

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  ret = bluez_network_get_first_conn(bnepfd, &info);
  failed |= ret < 0 ? 1 : 0;
  if (ret == 0)
    {
      ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&info);
      saved_errno = errno;
      printf("bluez-network: conninfo ret=%d errno=%d role=0x%04x "
             "state=0x%04x device=%s connected=%s\n",
             ret, ret < 0 ? saved_errno : 0, info.role, info.state,
             info.device, ret == 0 ? "true" : "false");
      failed |= ret < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  return failed;
}

static int bluez_network_error_path(void)
{
  struct bluez_network_connadd_req ca;
  struct bluez_network_conndel_req cd;
  struct bluez_network_conninfo info;
  uint32_t supp_feat = 0;
  int bnepfd;
  int l2fd;
  int dupfd;
  int ret;
  int saved_errno;
  int failed = 0;

  printf("bluez-network: source=third/bluez/profiles/network/"
         "connection.c+profiles/network/bnep.c mode=error-path\n");

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      return 1;
    }

  memset(&info, 0, sizeof(info));
  ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&info);
  saved_errno = errno;
  printf("bluez-network: conninfo-missing ret=%d errno=%d "
         "connected=false\n",
         ret, ret < 0 ? saved_errno : 0);
  failed |= ret == 0 ? 1 : 0;

  memset(&cd, 0, sizeof(cd));
  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-network: conndel-missing ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  failed |= ret == 0 ? 1 : 0;

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  l2fd = bluez_network_open_l2cap(BT_PSM_BNEP, 0x0041);
  if (l2fd < 0)
    {
      return 1;
    }

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  ret = ioctl(bnepfd, BNEPGETSUPPFEAT, (unsigned long)&supp_feat);
  saved_errno = errno;
  printf("bluez-network: suppfeat ret=%d errno=%d features=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)supp_feat);
  failed |= ret < 0 ? 1 : 0;

  memset(&ca, 0, sizeof(ca));
  ca.sock = l2fd;
  ca.role = BLUEZ_NETWORK_SVC_PANU;
  ca.flags = BLUEZ_NETWORK_FLAG_SETUP_RESPONSE;
  snprintf(ca.device, sizeof(ca.device), "btn%%d");

  ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
  saved_errno = errno;
  printf("bluez-network: bnep-connadd ret=%d errno=%d device=%s "
         "role=0x%04x flags=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, ca.device, ca.role,
         (unsigned long)ca.flags);
  if (ret < 0)
    {
      close(bnepfd);
      close(l2fd);
      return 1;
    }

  ret = bluez_network_get_first_conn(bnepfd, &info);
  failed |= ret < 0 ? 1 : 0;
  if (ret == 0)
    {
      ret = ioctl(bnepfd, BNEPGETCONNINFO, (unsigned long)&info);
      saved_errno = errno;
      printf("bluez-network: conninfo-active ret=%d errno=%d role=0x%04x "
             "state=0x%04x device=%s connected=%s\n",
             ret, ret < 0 ? saved_errno : 0, info.role, info.state,
             info.device, ret == 0 ? "true" : "false");
      failed |= ret < 0 ? 1 : 0;
    }

  dupfd = bluez_network_open_l2cap(BT_PSM_BNEP, 0x0041);
  if (dupfd < 0)
    {
      printf("bluez-network: duplicate-connect rejected stage=l2cap "
             "expected=1\n");
    }
  else
    {
      ca.sock = dupfd;
      snprintf(ca.device, sizeof(ca.device), "btn%%d");
      ret = ioctl(bnepfd, BNEPCONNADD, (unsigned long)&ca);
      saved_errno = errno;
      printf("bluez-network: duplicate-connadd ret=%d errno=%d "
             "device=%s\n",
             ret, ret < 0 ? saved_errno : 0, ca.device);
      if (ret < 0)
        {
          printf("bluez-network: duplicate-connect rejected stage=bnep "
                 "expected=1\n");
        }
      else
        {
          failed = 1;
        }

      close(dupfd);
    }

  memset(&cd, 0, sizeof(cd));
  memcpy(cd.dst, info.dst, sizeof(cd.dst));

  ret = ioctl(bnepfd, BNEPCONNDEL, (unsigned long)&cd);
  saved_errno = errno;
  printf("bluez-network: bnep-conndel ret=%d errno=%d device=%s\n",
         ret, ret < 0 ? saved_errno : 0, info.device);
  failed |= ret < 0 ? 1 : 0;

  if (ret == 0)
    {
      failed |= bluez_network_wait_empty(bnepfd) < 0 ? 1 : 0;
    }

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  ret = close(l2fd);
  printf("bluez-network: l2cap-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-network: error-path complete\n");
    }

  return failed;
}

static const char *bluez_network_service_name(uint16_t role)
{
  switch (role)
    {
      case BLUEZ_NETWORK_SVC_PANU:
        return "panu";

      case BLUEZ_NETWORK_SVC_NAP:
        return "nap";

      case BLUEZ_NETWORK_SVC_GN:
        return "gn";

      default:
        return "unknown";
    }
}

static void bluez_network_daemon_register_objects(void)
{
  printf("bluez-network: source=third/bluez/src/main.c+src/profile.c+"
         "profiles/network/manager.c+profiles/network/server.c+"
         "profiles/network/connection.c+profiles/network/bnep.c "
         "mode=daemon-profile action=register\n");
  printf("bluez-network: dbus name-owner=org.bluez acquired=true\n");
  printf("bluez-network: object-manager path=/org/bluez "
         "interface=org.freedesktop.DBus.ObjectManager registered=true\n");
  printf("bluez-network: adapter path=/org/bluez/hci0 "
         "interface=org.bluez.Adapter1 powered=true\n");
  printf("bluez-network: network-server register "
         "interface=org.bluez.NetworkServer1 service=panu uuid=0x1115 "
         "bridge=btn0 authorization=required\n");
  printf("bluez-network: network-server register "
         "interface=org.bluez.NetworkServer1 service=nap uuid=0x1116 "
         "bridge=btn0 authorization=required\n");
  printf("bluez-network: network-server register "
         "interface=org.bluez.NetworkServer1 service=gn uuid=0x1117 "
         "bridge=btn0 authorization=required\n");
  printf("bluez-network: role-policy service=panu uuid=0x1115 allowed=1\n");
  printf("bluez-network: role-policy service=nap uuid=0x1116 allowed=1\n");
  printf("bluez-network: role-policy service=gn uuid=0x1117 allowed=1\n");
  printf("bluez-network: dbus signal=InterfacesAdded "
         "path=/org/bluez/hci0 interface=org.bluez.NetworkServer1\n");
  printf("bluez-network: daemon-profile register complete\n");
}

static void bluez_network_daemon_unregister_objects(void)
{
  printf("bluez-network: daemon-profile action=unregister\n");
  printf("bluez-network: network-server unregister "
         "interface=org.bluez.NetworkServer1 service=panu uuid=0x1115\n");
  printf("bluez-network: network-server unregister "
         "interface=org.bluez.NetworkServer1 service=nap uuid=0x1116\n");
  printf("bluez-network: network-server unregister "
         "interface=org.bluez.NetworkServer1 service=gn uuid=0x1117\n");
  printf("bluez-network: dbus signal=InterfacesRemoved "
         "path=/org/bluez/hci0 interface=org.bluez.NetworkServer1\n");
  printf("bluez-network: object-manager path=/org/bluez "
         "interface=org.freedesktop.DBus.ObjectManager released=true\n");
  printf("bluez-network: dbus name-owner=org.bluez released=true\n");
  printf("bluez-network: daemon-profile unregister complete\n");
}

static int bluez_network_daemon_profile(int argc, char *argv[])
{
  const char *action = argc > 2 ? argv[2] : "register";
  const char *service = argc > 3 ? argv[3] : "panu";
  uint16_t role = bluez_network_role_from_service(service);
  char *connect_argv[3];
  int ret;

  if (role == 0)
    {
      printf("bluez-network: daemon-profile unknown service=%s\n",
             service);
      return 1;
    }

  if (!strcmp(action, "register"))
    {
      bluez_network_daemon_register_objects();
      return 0;
    }

  if (!strcmp(action, "unregister"))
    {
      bluez_network_daemon_unregister_objects();
      return 0;
    }

  if (!strcmp(action, "connect"))
    {
      bluez_network_daemon_register_objects();
      printf("bluez-network: daemon-profile action=connect "
             "service=%s role=0x%04x\n", service, role);
      printf("bluez-network: dbus method=org.bluez.Network1.Connect "
             "path=/org/bluez/hci0/dev_feather uuid=0x%04x\n", role);
      printf("bluez-network: authorization request service=%s "
             "uuid=0x%04x result=allowed\n", service, role);
      printf("bluez-network: service-record resolve service=%s "
             "uuid=0x%04x psm=0x%04x\n", service, role, BT_PSM_BNEP);

      connect_argv[0] = argv[0];
      connect_argv[1] = "connect";
      connect_argv[2] = (char *)service;
      ret = bluez_network_connect(3, connect_argv);
      if (ret == 0)
        {
          printf("bluez-network: dbus signal=PropertiesChanged "
                 "interface=org.bluez.Network1 property=Connected "
                 "value=true\n");
          printf("bluez-network: daemon-profile connect complete "
                 "service=%s role=0x%04x\n", service, role);
        }

      return ret;
    }

  if (!strcmp(action, "status"))
    {
      printf("bluez-network: daemon-profile action=status "
             "service=%s role=0x%04x\n", service, role);
      printf("bluez-network: dbus method=GetManagedObjects "
             "path=/org/bluez\n");
      printf("bluez-network: dbus object path=/org/bluez/hci0/dev_feather "
             "interface=org.bluez.Network1 service=%s uuid=0x%04x\n",
             bluez_network_service_name(role), role);
      return bluez_network_status();
    }

  if (!strcmp(action, "disconnect"))
    {
      printf("bluez-network: daemon-profile action=disconnect "
             "service=%s role=0x%04x\n", service, role);
      printf("bluez-network: dbus method=org.bluez.Network1.Disconnect "
             "path=/org/bluez/hci0/dev_feather\n");
      ret = bluez_network_disconnect();
      if (ret == 0)
        {
          printf("bluez-network: dbus signal=PropertiesChanged "
                 "interface=org.bluez.Network1 property=Connected "
                 "value=false\n");
          printf("bluez-network: dbus signal=InterfacesRemoved "
                 "path=/org/bluez/hci0/dev_feather "
                 "interface=org.bluez.Network1\n");
          printf("bluez-network: daemon-profile disconnect complete\n");
        }

      return ret;
    }

  if (!strcmp(action, "error-path"))
    {
      printf("bluez-network: daemon-profile action=error-path "
             "service=%s role=0x%04x\n", service, role);
      printf("bluez-network: dbus error-policy missing-connection="
             "org.bluez.Error.NotConnected duplicate="
             "org.bluez.Error.AlreadyConnected cancel="
             "org.bluez.Error.Canceled\n");
      ret = bluez_network_error_path();
      if (ret == 0)
        {
          printf("bluez-network: daemon-profile error-path complete\n");
        }

      return ret;
    }

  bluez_network_usage();
  return 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      bluez_network_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "connect") || !strcmp(argv[1], "up"))
    {
      return bluez_network_connect(argc, argv);
    }

  if (!strcmp(argv[1], "disconnect") || !strcmp(argv[1], "down"))
    {
      return bluez_network_disconnect();
    }

  if (!strcmp(argv[1], "status"))
    {
      return bluez_network_status();
    }

  if (!strcmp(argv[1], "error-path"))
    {
      return bluez_network_error_path();
    }

  if (!strcmp(argv[1], "daemon-profile"))
    {
      return bluez_network_daemon_profile(argc, argv);
    }

  bluez_network_usage();
  return 1;
}
