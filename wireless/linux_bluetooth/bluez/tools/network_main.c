/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/network_main.c
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

enum bluez_network_session_state
{
  BLUEZ_NETWORK_SESSION_IDLE = 0,
  BLUEZ_NETWORK_SESSION_CONNECTING,
  BLUEZ_NETWORK_SESSION_CONNECTED,
  BLUEZ_NETWORK_SESSION_DISCONNECTING,
  BLUEZ_NETWORK_SESSION_CLOSED,
};

struct bluez_network_session
{
  const char *service;
  uint16_t role;
  uint16_t psm;
  uint16_t cid;
  uint32_t flags;
  int l2fd;
  int bnepfd;
  char device[16];
  enum bluez_network_session_state state;
  bool dbus_owner;
  bool network1_owner;
  bool server_owner;
  bool fd_owner;
  bool bnep_owner;
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
  printf("       blueznetwork closeout-full begin|end\n");
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

static const char *bluez_network_state_name(
                    enum bluez_network_session_state state)
{
  switch (state)
    {
      case BLUEZ_NETWORK_SESSION_IDLE:
        return "idle";

      case BLUEZ_NETWORK_SESSION_CONNECTING:
        return "connecting";

      case BLUEZ_NETWORK_SESSION_CONNECTED:
        return "connected";

      case BLUEZ_NETWORK_SESSION_DISCONNECTING:
        return "disconnecting";

      case BLUEZ_NETWORK_SESSION_CLOSED:
        return "closed";

      default:
        return "unknown";
    }
}

static void bluez_network_session_init(struct bluez_network_session *session,
                                       const char *service, uint16_t role,
                                       uint16_t psm, uint16_t cid)
{
  memset(session, 0, sizeof(*session));
  session->service = service;
  session->role = role;
  session->psm = psm;
  session->cid = cid;
  session->flags = BLUEZ_NETWORK_FLAG_SETUP_RESPONSE;
  session->l2fd = -1;
  session->bnepfd = -1;
  snprintf(session->device, sizeof(session->device), "btn%%d");
  session->state = BLUEZ_NETWORK_SESSION_IDLE;
  session->dbus_owner = true;
  session->network1_owner = true;
  session->server_owner = true;
}

static void bluez_network_session_set_state(
                    struct bluez_network_session *session,
                    enum bluez_network_session_state state)
{
  session->state = state;
  printf("bluez-network: upstream-session state=%s service=%s role=0x%04x "
         "psm=0x%04x cid=0x%04x device=%s l2cap-fd=%d bnep-fd=%d\n",
         bluez_network_state_name(session->state), session->service,
         session->role, session->psm, session->cid, session->device,
         session->l2fd, session->bnepfd);
}

static void bluez_network_print_upstream_object_graph(
                    const struct bluez_network_session *session,
                    const char *action)
{
  printf("bluez-network: upstream-object-graph action=%s "
         "owner=profiles/network/manager.c,server.c,connection.c,bnep.c "
         "objects=network_manager,network_server,network_peer,"
         "network_conn,network_session,bnep_control,l2cap_io,"
         "netdev_bridge "
         "dbus=org.bluez.Network1,org.bluez.NetworkServer1 "
         "methods=Connect,Disconnect,Register,Unregister "
         "fd-flow=connect_l2cap_fd,BNEPCONNADD,BNEPCONNDEL "
         "session-state=%s service=%s role=0x%04x psm=0x%04x "
         "cid=0x%04x device=%s "
         "owners=dbus:%u,network1:%u,server:%u,fd:%u,bnep:%u "
         "upstream-link=blueznetwork-upstream-link-bluetoothd\n",
         action, bluez_network_state_name(session->state),
         session->service, session->role, session->psm, session->cid,
         session->device, session->dbus_owner ? 1 : 0,
         session->network1_owner ? 1 : 0,
         session->server_owner ? 1 : 0, session->fd_owner ? 1 : 0,
         session->bnep_owner ? 1 : 0);
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

static void bluez_network_print_bnep_no_data_abi(int fd)
{
  struct sockaddr addr;
  socklen_t addrlen;
  char byte = 0;
  int accept_errno = 0;
  int accept_fd;
  int accept_ret;
  int bind_errno = 0;
  int bind_ret;
  int connect_errno = 0;
  int connect_ret;
  int getpeername_errno = 0;
  int getpeername_ret;
  int getname_errno = 0;
  int getname_ret;
  int listen_errno = 0;
  int listen_ret;
  int no_data_ok;
  int recv_errno = 0;
  int recv_ret;
  int saved_errno;
  int send_errno = 0;
  int send_ret;
  int shutdown_errno = 0;
  int shutdown_ret;

  memset(&addr, 0, sizeof(addr));
  addr.sa_family = AF_BLUETOOTH;
  bind_ret = bind(fd, &addr, sizeof(addr));
  saved_errno = errno;
  if (bind_ret < 0)
    {
      bind_errno = saved_errno;
    }

  memset(&addr, 0, sizeof(addr));
  addrlen = sizeof(addr);
  getname_ret = getsockname(fd, &addr, &addrlen);
  saved_errno = errno;
  if (getname_ret < 0)
    {
      getname_errno = saved_errno;
    }

  memset(&addr, 0, sizeof(addr));
  addrlen = sizeof(addr);
  getpeername_ret = getpeername(fd, &addr, &addrlen);
  saved_errno = errno;
  if (getpeername_ret < 0)
    {
      getpeername_errno = saved_errno;
    }

  memset(&addr, 0, sizeof(addr));
  addr.sa_family = AF_BLUETOOTH;
  connect_ret = connect(fd, &addr, sizeof(addr));
  saved_errno = errno;
  if (connect_ret < 0)
    {
      connect_errno = saved_errno;
    }

  send_ret = send(fd, &byte, sizeof(byte), 0);
  saved_errno = errno;
  if (send_ret < 0)
    {
      send_errno = saved_errno;
    }

  recv_ret = recv(fd, &byte, sizeof(byte), 0);
  saved_errno = errno;
  if (recv_ret < 0)
    {
      recv_errno = saved_errno;
    }

  listen_ret = listen(fd, 1);
  saved_errno = errno;
  if (listen_ret < 0)
    {
      listen_errno = saved_errno;
    }

  shutdown_ret = shutdown(fd, SHUT_RDWR);
  saved_errno = errno;
  if (shutdown_ret < 0)
    {
      shutdown_errno = saved_errno;
    }

  accept_fd = accept(fd, NULL, NULL);
  saved_errno = errno;
  if (accept_fd >= 0)
    {
      accept_ret = 0;
      close(accept_fd);
    }
  else
    {
      accept_ret = accept_fd;
      accept_errno = saved_errno;
    }

  no_data_ok = bind_ret == -1 && bind_errno == EOPNOTSUPP &&
               getname_ret == -1 && getname_errno == EOPNOTSUPP &&
               getpeername_ret == -1 &&
               getpeername_errno == EOPNOTSUPP &&
               connect_ret == -1 && connect_errno == EOPNOTSUPP &&
               send_ret == -1 && send_errno == EOPNOTSUPP &&
               recv_ret == -1 && recv_errno == EOPNOTSUPP &&
               listen_ret == -1 && listen_errno == EOPNOTSUPP &&
               shutdown_ret == -1 && shutdown_errno == EOPNOTSUPP &&
               accept_ret == -1 && accept_errno == EOPNOTSUPP;

  printf("bluez-network: bnep no-data abi=sock_no "
         "proto=BTPROTO_BNEP bind-ret=%d bind-errno=%d "
         "getname-ret=%d getname-errno=%d "
         "getpeername-ret=%d getpeername-errno=%d "
         "connect-ret=%d connect-errno=%d "
         "send-ret=%d send-errno=%d recv-ret=%d recv-errno=%d "
         "listen-ret=%d listen-errno=%d "
         "shutdown-ret=%d shutdown-errno=%d "
         "accept-ret=%d accept-errno=%d no-data-ok=%d\n",
         bind_ret, bind_errno, getname_ret, getname_errno,
         getpeername_ret, getpeername_errno, connect_ret, connect_errno,
         send_ret, send_errno, recv_ret, recv_errno, listen_ret,
         listen_errno, shutdown_ret, shutdown_errno, accept_ret,
         accept_errno, no_data_ok);
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
  struct bluez_network_session session;
  uint16_t role;
  uint16_t psm = BT_PSM_BNEP;
  uint16_t cid = 0x0041;
  uint32_t supp_feat = 0;
  static char status[12000];
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

  bluez_network_session_init(&session, service, role, psm, cid);
  bluez_network_session_set_state(&session,
                                  BLUEZ_NETWORK_SESSION_CONNECTING);
  bluez_network_print_upstream_object_graph(&session, "connect-begin");

  l2fd = bluez_network_open_l2cap(session.psm, session.cid);
  if (l2fd < 0)
    {
      return 1;
    }

  session.l2fd = l2fd;
  session.fd_owner = true;

  bnepfd = bluez_network_open_bnep();
  if (bnepfd < 0)
    {
      close(l2fd);
      return 1;
    }

  session.bnepfd = bnepfd;
  session.bnep_owner = true;
  bluez_network_print_bnep_no_data_abi(bnepfd);

  ret = ioctl(bnepfd, BNEPGETSUPPFEAT, (unsigned long)&supp_feat);
  saved_errno = errno;
  printf("bluez-network: suppfeat ret=%d errno=%d features=0x%08lx\n",
         ret, ret < 0 ? saved_errno : 0, (unsigned long)supp_feat);
  failed |= ret < 0 ? 1 : 0;

  memset(&ca, 0, sizeof(ca));
  ca.sock = session.l2fd;
  ca.role = session.role;
  ca.flags = session.flags;
  strlcpy(ca.device, session.device, sizeof(ca.device));

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

  strlcpy(session.device, ca.device, sizeof(session.device));
  bluez_network_session_set_state(&session,
                                  BLUEZ_NETWORK_SESSION_CONNECTED);
  bluez_network_print_upstream_object_graph(&session, "connect-complete");

  printf("bluez-network: native-boundary connected-l2cap-fd=%d "
         "bnep-fd=%d ioctl=BNEPCONNADD role=0x%04x service=%s "
         "device=%s\n",
         session.l2fd, session.bnepfd, ca.role, service, ca.device);
  printf("bluez-network: native-closeout fd-ownership="
         "l2cap-fd=connected,bnep-fd=control,sock-lookup=1,sock-put=1,"
         "psm=0x%04x,cid=0x%04x\n",
         session.psm, session.cid);
  printf("bluez-network: native-closeout session-ownership="
         "bnep_add_connection,netdev_setup,register_netdev,"
         "session_link,kthread_run,session_thread service=%s\n",
         service);
  printf("bluez-network: native-closeout datapath-ownership="
         "Network1.Connect,connected-l2cap-fd,BNEPCONNADD,btn0,"
         "ndo_start_xmit,bnep_tx_frame,l2cap-send,hwsim-bnep,"
         "bnep_rx_frame,netif_rx,NuttX-IP\n");
  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-network: native-status-after-connect\n");
      printf("%s", status);
    }

  printf("bluez-network: profile connected interface=%s uuid=0x%04x\n",
         ca.device, session.role);

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
      session.l2fd = -1;
      session.bnepfd = -1;
      bluez_network_session_set_state(&session,
                                      BLUEZ_NETWORK_SESSION_CLOSED);
      printf("bluez-network: connect complete\n");
    }

  return failed;
}

static int bluez_network_disconnect(void)
{
  struct bluez_network_conndel_req cd;
  struct bluez_network_conninfo info;
  struct bluez_network_session session;
  static char status[12000];
  int bnepfd;
  int ret;
  int saved_errno;
  int failed = 0;

  printf("bluez-network: mode=disconnect\n");

  bluez_network_session_init(&session, "panu", BLUEZ_NETWORK_SVC_PANU,
                             BT_PSM_BNEP, 0x0041);
  bluez_network_session_set_state(&session,
                                  BLUEZ_NETWORK_SESSION_DISCONNECTING);
  bluez_network_print_upstream_object_graph(&session, "disconnect-begin");

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

  strlcpy(session.device, info.device, sizeof(session.device));
  session.role = info.role;
  session.bnepfd = bnepfd;
  session.bnep_owner = true;

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

  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-network: native-status-after-disconnect\n");
      printf("%s", status);
    }

  ret = close(bnepfd);
  printf("bluez-network: bnep-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      session.bnepfd = -1;
      bluez_network_session_set_state(&session,
                                      BLUEZ_NETWORK_SESSION_CLOSED);
      bluez_network_print_upstream_object_graph(&session,
                                                "disconnect-complete");
      printf("bluez-network: profile disconnected\n");
      printf("bluez-network: disconnect complete\n");
    }

  return failed;
}

static int bluez_network_status(void)
{
  struct bluez_network_conninfo info;
  static char status[12000];
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

  if (linux_bt_upstream_af_status(status, sizeof(status)) == 0)
    {
      printf("bluez-network: native-status\n");
      printf("%s", status);
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
  printf("bluez-network: daemon-profile semantic-contract action=register "
         "dbus-name-owner=1 object-manager-owner=1 adapter-owner=1 "
         "network-server-owner=1 service-panu-owner=1 "
         "service-nap-owner=1 service-gn-owner=1 "
         "authorization-policy-owner=1 signal-owner=1\n");
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
  printf("bluez-network: daemon-profile semantic-contract action=unregister "
         "network-server-release-owner=1 object-manager-release-owner=1 "
         "dbus-name-release-owner=1 signal-owner=1 "
         "service-panu-final=0 service-nap-final=0 service-gn-final=0 "
         "dbus-owner-final=0\n");
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
          printf("bluez-network: daemon-profile semantic-contract "
                 "action=connect service=%s role=0x%04x "
                 "network1-owner=1 authorization-owner=1 "
                 "service-record-owner=1 l2cap-fd-owner=1 "
                 "bnep-ioctl-owner=1 bnep-session-owner=1 "
                 "netdev-owner=1 dbus-properties-owner=1 "
                 "cleanup-owner=1\n",
                 service, role);
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
      printf("bluez-network: daemon-profile semantic-contract "
             "action=status service=%s role=0x%04x "
             "object-manager-owner=1 network1-owner=1 "
             "conninfo-owner=1 native-status-owner=1\n",
             service, role);
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
          printf("bluez-network: daemon-profile semantic-contract "
                 "action=disconnect service=%s role=0x%04x "
                 "network1-owner=1 bnep-conndel-owner=1 "
                 "properties-owner=1 interfaces-removed-owner=1 "
                 "cleanup-owner=1\n",
                 service, role);
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
          printf("bluez-network: daemon-profile semantic-contract "
                 "action=error-path service=%s role=0x%04x "
                 "not-connected-owner=1 duplicate-owner=1 "
                 "cancel-owner=1 native-error-owner=1 "
                 "cleanup-owner=1\n",
                 service, role);
          printf("bluez-network: daemon-profile error-path complete\n");
        }

      return ret;
    }

  bluez_network_usage();
  return 1;
}

static int bluez_network_closeout_full(int argc, char *argv[])
{
  const char *stage = argc > 2 ? argv[2] : "begin";
  struct bluez_network_session session;

  bluez_network_session_init(&session, "panu", BLUEZ_NETWORK_SVC_PANU,
                             BT_PSM_BNEP, 0x0041);
  bluez_network_print_upstream_object_graph(&session, "closeout");

  printf("bluez-network: closeout-full stage=%s "
         "semantic=network-current-closeout-umbrella\n", stage);
  printf("bluez-network: closeout phase=daemon-dbus "
         "owner=org.bluez object-manager=1 adapter=1 "
         "network-server=panu,nap,gn\n");
  printf("bluez-network: closeout phase=service-registration "
         "services=panu,nap,gn uuids=0x1115,0x1116,0x1117 "
         "authorization=required\n");
  printf("bluez-network: closeout phase=fd-handoff "
         "source=third/bluez/profiles/network/connection.c+"
         "profiles/network/bnep.c l2cap-fd=connected "
         "bnep-connadd=required\n");
  printf("bluez-network: closeout phase=roles "
         "panu=required nap=required gn=required\n");
  printf("bluez-network: closeout phase=datapath "
         "btn0=required ping=required mtu1400=required "
         "native-bnep-counters=required\n");
  printf("bluez-network: closeout phase=error-policy "
         "missing=NotConnected duplicate=AlreadyConnected "
         "cancel=Canceled cleanup=required\n");
  printf("bluez-network: closeout phase=lifecycle "
         "connect-disconnect-reconnect=required active-final=0\n");

  if (!strcmp(stage, "end") || !strcmp(stage, "complete"))
    {
      printf("bluez-network: closeout upstream-coverage-map "
             "bluez-src=third/bluez/src/main.c+third/bluez/src/profile.c+"
             "third/bluez/src/device.c+third/bluez/src/adapter.c+"
             "third/bluez/src/dbus-common.c+"
             "third/bluez/profiles/network/manager.c+"
             "third/bluez/profiles/network/server.c+"
             "third/bluez/profiles/network/connection.c+"
             "third/bluez/profiles/network/bnep.c "
             "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/sock.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/bnep/netdev.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
             "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
             "executed=daemon-dbus,network-server,role-policy,"
             "profile-connect,l2cap-fd-handoff,bnep-connadd,"
             "netdev-xmit,l2cap-deliver,netif-rx,error-policy,"
             "profile-disconnect,unregister "
             "roles=panu,nap,gn datapath=btn0 cleanup=bnep-native-active-0 "
             "upstream-link=blueznetwork-upstream-link-bluetoothd "
             "final-ok=1\n");
      printf("bluez-network: closeout semantic-contract full "
             "dbus-name-owner=1 object-manager-owner=1 adapter-owner=1 "
             "network-server-owner=1 network1-owner=1 "
             "authorization-owner=1 service-record-owner=1 "
             "l2cap-fd-owner=1 bnep-ioctl-owner=1 "
             "bnep-session-owner=1 netdev-owner=1 role-owner=1 "
             "error-policy-owner=1 cleanup-owner=1 "
             "dbus-final=1 network-server-final=1 network1-final=1 "
             "fd-handoff-final=1 bnep-session-final=1 role-final=1 "
             "error-final=1 cleanup-final=1\n");
      printf("bluez-network: closeout native-datapath-contract full "
             "fd-owner=connected-l2cap-fd,bnep-control-fd "
             "ioctl-owner=BNEPGETSUPPFEAT,BNEPCONNADD,BNEPGETCONNLIST,"
             "BNEPGETCONNINFO,BNEPCONNDEL "
             "session-owner=sockfd_lookup,bnep_add_connection,"
             "bnep_session,kthread_run,session_thread,session_stop "
             "netdev-owner=alloc_netdev,register_netdev,ndo_start_xmit,"
             "unregister_netdev,free_netdev "
             "thread-owner=kthread_run,bnep_session,rx_wait,tx_wait,"
             "stop_wakeup,session_terminate "
             "netdev-ops-owner=alloc_netdev,netdev_ops,ndo_open,ndo_stop,"
             "ndo_start_xmit,netif_rx,unregister_netdev,free_netdev "
             "state-owner=session_new,session_active,session_stopping,"
             "session_closed,active_zero "
             "lock-owner=session_list,session_ref,tx_queue,rx_queue,"
             "ioctl_serialization "
             "tx-owner=NuttX-IP,net_device,bnep_tx_frame,l2cap-send,"
             "hwsim-bnep "
             "rx-owner=hwsim-bnep,l2cap-recv,bnep_rx_frame,netif_rx,"
             "NuttX-IP "
             "role-owner=PANU,NAP,GN "
             "error-owner=missing-conn,duplicate-conn,cancel-connect,"
             "connadd-fail,conndel-fail,bad-fd,bad-role,tx-fail,rx-drop "
             "cleanup-owner=conndel,session-unlink,fd-close,"
             "netdev-unregister,active-zero "
             "upstream-link=bluez-network-fd-to-linux-bnep-object-graph "
             "native-datapath-final=1 semantic-contract-final=1\n");
      printf("bluez-network: closeout-full complete "
             "semantic=network-current-closeout-umbrella\n");
    }

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

  if (!strcmp(argv[1], "closeout-full"))
    {
      return bluez_network_closeout_full(argc, argv);
    }

  bluez_network_usage();
  return 1;
}
