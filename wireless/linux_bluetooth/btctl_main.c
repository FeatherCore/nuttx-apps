/****************************************************************************
 * apps/wireless/linux_bluetooth/btctl_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef LINUX_BT_NATIVE_CMTPCONNADD
#  define LINUX_BT_NATIVE_CMTPCONNADD      (('C') | 200)
#  define LINUX_BT_NATIVE_CMTPCONNDEL      (('C') | 201)
#  define LINUX_BT_NATIVE_CMTPGETCONNLIST  (('C') | 210)
#  define LINUX_BT_NATIVE_CMTPGETCONNINFO  (('C') | 211)
#endif

#ifndef LINUX_BT_NATIVE_HIDPCONNADD
#  define LINUX_BT_NATIVE_HIDPCONNADD      (('H') | 200)
#  define LINUX_BT_NATIVE_HIDPCONNDEL      (('H') | 201)
#  define LINUX_BT_NATIVE_HIDPGETCONNLIST  (('H') | 210)
#  define LINUX_BT_NATIVE_HIDPGETCONNINFO  (('H') | 211)
#endif

#define BTCTL_CMTP_LOOPBACK 0

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct btctl_bdaddr_s
{
  uint8_t b[6];
};

struct btctl_cmtp_connadd_req_s
{
  int sock;
  uint32_t flags;
};

struct btctl_cmtp_conndel_req_s
{
  struct btctl_bdaddr_s bdaddr;
  uint32_t flags;
};

struct btctl_cmtp_conninfo_s
{
  struct btctl_bdaddr_s bdaddr;
  uint32_t flags;
  uint16_t state;
  int num;
};

struct btctl_cmtp_connlist_req_s
{
  uint32_t cnum;
  struct btctl_cmtp_conninfo_s *ci;
};

struct btctl_hidp_connadd_req_s
{
  int ctrl_sock;
  int intr_sock;
  uint16_t parser;
  uint16_t rd_size;
  uint8_t *rd_data;
  uint8_t country;
  uint8_t subclass;
  uint16_t vendor;
  uint16_t product;
  uint16_t version;
  uint32_t flags;
  uint32_t idle_to;
  char name[128];
};

struct btctl_hidp_conndel_req_s
{
  struct btctl_bdaddr_s bdaddr;
  uint32_t flags;
};

struct btctl_hidp_conninfo_s
{
  struct btctl_bdaddr_s bdaddr;
  uint32_t flags;
  uint16_t state;
  uint16_t vendor;
  uint16_t product;
  uint16_t version;
  char name[128];
};

struct btctl_hidp_connlist_req_s
{
  uint32_t cnum;
  struct btctl_hidp_conninfo_s *ci;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void btctl_usage(void)
{
  printf("usage: btctl <command> [args]\\n");
  printf("\\n");
  printf("commands:\\n");
  printf("  info\\n");
  printf("  upstream [status|hci-status|open|create [opcode]|close|read|drain|drain-trace [max]|poll|pump|bridge [rounds] [max-records]|6lowpan-up [ifname]|6lowpan-status|6lowpan-down|socket hci [raw|user|monitor|control|logging] [dev]|socket l2cap [psm] [cid]|socket iso [addr-type]|socket cmtp|ordinary-cmtp-socket [handle]|ordinary-hidp-socket [handle]|socket-send raw|user <dev> cmd|acl|sco|iso|vendor <hex...>|socket-cmtp [handle]|socket-filter <dev> <type-mask> <event-mask0> <event-mask1> [opcode]|socket-ioctl [dev] [up|down|reset|restat|scan|auth|encrypt|ptype|linkpol|linkmode|aclmtu|scomtu|connlist|conninfo|authinfo|block|unblock] [dev-opt|acl|le|cis|bis]|l2cap-bind <psm> <cid> <handle>|l2cap-connect <psm> <cid>|l2cap-listen [backlog]|l2cap-recv [max]|l2cap-write <hex...>|l2cap-close|l2cap-send <psm> <cid> <handle> <hex...>|avdtp-listen <handle>|avdtp-recv [max]|avdtp-auto-rsp <peer>|avdtp-auto-rsp-loop <peer> <count>|avdtp-close|avdtp-discover <peer>|avdtp-discover-rsp <peer>|avdtp-getcap <peer>|avdtp-getcap-rsp <peer>|avdtp-setconfig <peer>|avdtp-setconfig-rsp <peer>|avdtp-open <peer>|avdtp-open-rsp <peer>|avdtp-start <peer>|avdtp-start-rsp <peer>|avdtp-suspend <peer>|avdtp-suspend-rsp <peer>|avdtp-close-stream <peer>|avdtp-close-stream-rsp <peer>|avdtp-reconfigure <peer>|avdtp-reconfigure-rsp <peer>|avdtp-abort <peer>|avdtp-abort-rsp <peer>|avdtp-delay-report <peer>|avdtp-delay-report-rsp <peer>|avdtp-getconfig <peer>|avdtp-getconfig-rsp <peer>|avdtp-getallcap <peer>|avdtp-getallcap-rsp <peer>|avdtp-security-control <peer>|avdtp-security-control-rsp <peer>|iso-bind <addr-type> <handle>|iso-connect <addr-type>|iso-recv [max]|iso-write <hex...>|iso-close|iso-send <addr-type> <handle> <hex...>|mgmt-listen|mgmt-read [max]|mgmt-send <opcode> [index] [param]|mgmt-close|mgmt-poll-discovery [max]|mgmt-socket <opcode> [index] [param]|hci-connect-br <peer>|hci-disconnect-br <peer>|hci-connect-le <peer>|hci-disconnect-le <peer>|a2dp-source-sample [peer]|le-audio-source-sample <big> <bis>|send|sendhex cmd|acl|iso|event <payload>]\\n");
  printf("    upstream bridge [rounds] [max-records]\\n");
  printf("    upstream 6lowpan-up [ifname]\\n");
  printf("    upstream 6lowpan-status\\n");
  printf("    upstream 6lowpan-down\\n");
  printf("    upstream socket bnep\\n");
  printf("    upstream bnep-ioctl suppfeat|connlist|conninfo|connadd|conndel [param|l2cap]\\n");
  printf("    upstream socket-filter <dev> <type-mask> <event-mask0> <event-mask1> [opcode]\\n");
  printf("    upstream socket-ioctl [dev] [up|down|reset|restat|scan|auth|encrypt|ptype|linkpol|linkmode|aclmtu|scomtu|connlist|conninfo|authinfo|block|unblock] [dev-opt|acl|le|cis|bis]\\n");
  printf("    upstream l2cap-bind <psm> <cid> <handle>\\n");
  printf("    upstream l2cap-connect <psm> <cid>\\n");
  printf("    upstream l2cap-listen [backlog]\\n");
  printf("    upstream l2cap-recv [max]\\n");
  printf("    upstream l2cap-write <hex...>\\n");
  printf("    upstream l2cap-close\\n");
  printf("    upstream l2cap-send <psm> <cid> <handle> <hex...>\\n");
  printf("    upstream avdtp-listen <handle>\\n");
  printf("    upstream avdtp-recv [max]\\n");
  printf("    upstream avdtp-auto-rsp <peer>\\n");
  printf("    upstream avdtp-auto-rsp-loop <peer> <count>\\n");
  printf("    upstream avdtp-close\\n");
  printf("    upstream avdtp-discover <peer>\\n");
  printf("    upstream avdtp-discover-rsp <peer>\\n");
  printf("    upstream avdtp-getcap <peer>\\n");
  printf("    upstream avdtp-getcap-rsp <peer>\\n");
  printf("    upstream avdtp-setconfig <peer>\\n");
  printf("    upstream avdtp-setconfig-rsp <peer>\\n");
  printf("    upstream avdtp-open <peer>\\n");
  printf("    upstream avdtp-open-rsp <peer>\\n");
  printf("    upstream avdtp-start <peer>\\n");
  printf("    upstream avdtp-start-rsp <peer>\\n");
  printf("    upstream avdtp-suspend <peer>\\n");
  printf("    upstream avdtp-suspend-rsp <peer>\\n");
  printf("    upstream avdtp-close-stream <peer>\\n");
  printf("    upstream avdtp-close-stream-rsp <peer>\\n");
  printf("    upstream avdtp-reconfigure <peer>\\n");
  printf("    upstream avdtp-reconfigure-rsp <peer>\\n");
  printf("    upstream avdtp-abort <peer>\\n");
  printf("    upstream avdtp-abort-rsp <peer>\\n");
  printf("    upstream avdtp-delay-report <peer>\\n");
  printf("    upstream avdtp-delay-report-rsp <peer>\\n");
  printf("    upstream avdtp-getconfig <peer>\\n");
  printf("    upstream avdtp-getconfig-rsp <peer>\\n");
  printf("    upstream avdtp-getallcap <peer>\\n");
  printf("    upstream avdtp-getallcap-rsp <peer>\\n");
  printf("    upstream avdtp-security-control <peer>\\n");
  printf("    upstream avdtp-security-control-rsp <peer>\\n");
  printf("    upstream iso-bind <addr-type> <handle>\\n");
  printf("    upstream iso-connect <addr-type>\\n");
  printf("    upstream iso-recv [max]\\n");
  printf("    upstream iso-write <hex...>\\n");
  printf("    upstream iso-close\\n");
  printf("    upstream iso-send <addr-type> <handle> <hex...>\\n");
  printf("    upstream a2dp-source-sample [peer]\\n");
  printf("    upstream le-audio-source-sample <big> <bis>\\n");
  printf("  state\\n");
  printf("  events\\n");
  printf("  mgmt status\\n");
  printf("  mgmt power|connectable|discoverable|bondable on|off\\n");
  printf("  mgmt le|bredr|advertising on|off\\n");
  printf("  scan bredr|le\\n");
  printf("  advertise start|stop\\n");
  printf("  connect <peer>\\n");
  printf("  disconnect <peer>\\n");
  printf("  pair <peer>\\n");
  printf("  l2cap-connect <peer> <psm>\\n");
  printf("  l2cap-disconnect <peer> <cid>\\n");
  printf("  l2cap-send <peer> <payload>\\n");
  printf("  l2cap-echo <peer> <payload>\\n");
  printf("  gatt-read [peer] <handle>\\n");
  printf("  gatt-write [peer] <handle> <payload>\\n");
  printf("  poll ctrl|adv|acl|iso\\n");
}

static uint32_t btctl_hci_conn_type(const char *arg)
{
  if (arg == NULL || !strcmp(arg, "acl"))
    {
      return 0x01;
    }

  if (!strcmp(arg, "le"))
    {
      return 0x80;
    }

  if (!strcmp(arg, "cis"))
    {
      return 0x82;
    }

  if (!strcmp(arg, "bis"))
    {
      return 0x83;
    }

  return (uint32_t)strtoul(arg, NULL, 0);
}

static uint16_t btctl_peer(const char *arg)
{
  if (arg == NULL || !strcmp(arg, "broadcast"))
    {
      return LINUX_BT_HWSIM_DST_BROADCAST;
    }

  return (uint16_t)strtoul(arg, NULL, 0);
}

static uint16_t btctl_type(const char *arg)
{
  if (!strcmp(arg, "ctrl") || !strcmp(arg, "bredr"))
    {
      return LINUX_BT_HWSIM_TYPE_CTRL;
    }
  else if (!strcmp(arg, "adv") || !strcmp(arg, "le"))
    {
      return LINUX_BT_HWSIM_TYPE_ADV;
    }
  else if (!strcmp(arg, "acl"))
    {
      return LINUX_BT_HWSIM_TYPE_ACL;
    }
  else if (!strcmp(arg, "iso"))
    {
      return LINUX_BT_HWSIM_TYPE_ISO;
    }

  return 0;
}

static uint8_t btctl_hci_type(const char *arg)
{
  if (!strcmp(arg, "cmd") || !strcmp(arg, "command"))
    {
      return LINUX_BT_HCI_COMMAND_PKT;
    }
  else if (!strcmp(arg, "acl"))
    {
      return LINUX_BT_HCI_ACL_PKT;
    }
  else if (!strcmp(arg, "sco"))
    {
      return LINUX_BT_HCI_SCO_PKT;
    }
  else if (!strcmp(arg, "iso"))
    {
      return LINUX_BT_HCI_ISO_PKT;
    }
  else if (!strcmp(arg, "event") || !strcmp(arg, "evt"))
    {
      return LINUX_BT_HCI_EVENT_PKT;
    }
  else if (!strcmp(arg, "vendor"))
    {
      return LINUX_BT_HCI_VENDOR_PKT;
    }

  return 0;
}

static int btctl_btproto(const char *arg)
{
  if (!strcmp(arg, "l2cap"))
    {
      return LINUX_BT_BTPROTO_L2CAP;
    }
  else if (!strcmp(arg, "bnep"))
    {
      return LINUX_BT_BTPROTO_BNEP;
    }
  else if (!strcmp(arg, "cmtp"))
    {
      return LINUX_BT_BTPROTO_CMTP;
    }
  else if (!strcmp(arg, "hci") || !strcmp(arg, "mgmt"))
    {
      return LINUX_BT_BTPROTO_HCI;
    }
  else if (!strcmp(arg, "iso"))
    {
      return LINUX_BT_BTPROTO_ISO;
    }

  return (int)strtol(arg, NULL, 0);
}

static int btctl_bnep_ioctl_action(const char *arg)
{
  if (!strcmp(arg, "suppfeat") || !strcmp(arg, "getsuppfeat"))
    {
      return LINUX_BT_BNEP_IOCTL_GETSUPPFEAT;
    }
  else if (!strcmp(arg, "connlist") || !strcmp(arg, "getconnlist"))
    {
      return LINUX_BT_BNEP_IOCTL_GETCONNLIST;
    }
  else if (!strcmp(arg, "conninfo") || !strcmp(arg, "getconninfo"))
    {
      return LINUX_BT_BNEP_IOCTL_GETCONNINFO;
    }
  else if (!strcmp(arg, "connadd"))
    {
      return LINUX_BT_BNEP_IOCTL_CONNADD;
    }
  else if (!strcmp(arg, "conndel"))
    {
      return LINUX_BT_BNEP_IOCTL_CONNDEL;
    }

  return (int)strtol(arg, NULL, 0);
}

static void btctl_cmtp_fill_bdaddr(uint16_t seed,
                                   struct btctl_bdaddr_s *bdaddr)
{
  if (bdaddr == NULL)
    {
      return;
    }

  bdaddr->b[0] = (uint8_t)(seed & 0xff);
  bdaddr->b[1] = (uint8_t)(seed >> 8);
  bdaddr->b[2] = 0xc4;
  bdaddr->b[3] = 0x17;
  bdaddr->b[4] = 0x00;
  bdaddr->b[5] = 0x48;
}

static void btctl_hidp_fill_bdaddr(uint16_t seed,
                                   struct btctl_bdaddr_s *bdaddr)
{
  if (bdaddr == NULL)
    {
      return;
    }

  bdaddr->b[0] = (uint8_t)(seed & 0xff);
  bdaddr->b[1] = (uint8_t)(seed >> 8);
  bdaddr->b[2] = 0x24;
  bdaddr->b[3] = 0x11;
  bdaddr->b[4] = 0x00;
  bdaddr->b[5] = 0x48;
}

static int btctl_expected_ioctl_error(int ret, int err)
{
  return ret == -err || (ret == -1 && errno == err);
}

static int btctl_expected_saved_error(int ret, int saved_errno, int err)
{
  return ret == -err || (ret == -1 && saved_errno == err);
}

struct btctl_unsupported_ops_s
{
  int bind_ret;
  int bind_errno;
  int getsockname_ret;
  int getsockname_errno;
  int getpeername_ret;
  int getpeername_errno;
  int connect_ret;
  int connect_errno;
  int send_ret;
  int send_errno;
  int recv_ret;
  int recv_errno;
  int listen_ret;
  int listen_errno;
  int shutdown_ret;
  int shutdown_errno;
  int accept_ret;
  int accept_errno;
  int ok;
};

static void btctl_probe_unsupported_ops(int fd,
                                        struct btctl_unsupported_ops_s *ops)
{
  uint8_t tx = 0;
  uint8_t rx = 0;
  struct sockaddr btaddr;
  socklen_t btaddr_len;
  struct iovec tx_iov;
  struct iovec rx_iov;
  struct msghdr tx_msg;
  struct msghdr rx_msg;
  int accept_fd;

  memset(ops, 0, sizeof(*ops));
  memset(&btaddr, 0, sizeof(btaddr));
  memset(&tx_msg, 0, sizeof(tx_msg));
  memset(&rx_msg, 0, sizeof(rx_msg));

  btaddr.sa_family = AF_BLUETOOTH;

  errno = 0;
  ops->bind_ret = bind(fd, &btaddr, sizeof(btaddr));
  ops->bind_errno = errno;

  btaddr_len = sizeof(btaddr);
  errno = 0;
  ops->getsockname_ret = getsockname(fd, &btaddr, &btaddr_len);
  ops->getsockname_errno = errno;

  btaddr_len = sizeof(btaddr);
  errno = 0;
  ops->getpeername_ret = getpeername(fd, &btaddr, &btaddr_len);
  ops->getpeername_errno = errno;

  errno = 0;
  ops->connect_ret = connect(fd, &btaddr, sizeof(btaddr));
  ops->connect_errno = errno;

  tx_iov.iov_base = &tx;
  tx_iov.iov_len = sizeof(tx);
  tx_msg.msg_iov = &tx_iov;
  tx_msg.msg_iovlen = 1;

  rx_iov.iov_base = &rx;
  rx_iov.iov_len = sizeof(rx);
  rx_msg.msg_iov = &rx_iov;
  rx_msg.msg_iovlen = 1;

  errno = 0;
  ops->send_ret = (int)sendmsg(fd, &tx_msg, 0);
  ops->send_errno = errno;

  errno = 0;
  ops->recv_ret = (int)recvmsg(fd, &rx_msg, 0);
  ops->recv_errno = errno;

  errno = 0;
  ops->listen_ret = listen(fd, 1);
  ops->listen_errno = errno;

  errno = 0;
  ops->shutdown_ret = shutdown(fd, SHUT_RDWR);
  ops->shutdown_errno = errno;

  errno = 0;
  accept_fd = accept(fd, NULL, NULL);
  ops->accept_errno = errno;
  if (accept_fd >= 0)
    {
      ops->accept_ret = 0;
      close(accept_fd);
    }
  else
    {
      ops->accept_ret = accept_fd;
    }

  ops->ok = btctl_expected_saved_error(ops->bind_ret, ops->bind_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->getsockname_ret,
                                       ops->getsockname_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->getpeername_ret,
                                       ops->getpeername_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->connect_ret,
                                       ops->connect_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->send_ret, ops->send_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->recv_ret, ops->recv_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->listen_ret, ops->listen_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->shutdown_ret,
                                       ops->shutdown_errno,
                                       EOPNOTSUPP) &&
            btctl_expected_saved_error(ops->accept_ret, ops->accept_errno,
                                       EOPNOTSUPP);
}

static int btctl_upstream_ordinary_cmtp_socket(uint16_t handle)
{
  struct btctl_cmtp_connadd_req_s add_req;
  struct btctl_cmtp_conndel_req_s del_req;
  struct btctl_cmtp_conninfo_s info[1];
  struct btctl_cmtp_conninfo_s get_info;
  struct btctl_cmtp_connlist_req_s list_req;
  struct btctl_unsupported_ops_s unsupported;
  int fd;
  int create_nonblock_fd = -1;
  int create_nonblock_flags = -1;
  int create_nonblock_close = -1;
  int nonblock_ret = -1;
  int add_ret;
  int dup_add_ret;
  int dup_add_errno;
  int list_ret;
  int info_ret;
  int del_ret;
  int post_info_ret;
  int post_info_errno;
  int dup_del_ret;
  int dup_del_errno;
  int close_ret;
  int final_ok;
  int dup_add_ok;
  int post_info_ok;
  int dup_del_ok;

  create_nonblock_fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_NONBLOCK,
                              LINUX_BT_BTPROTO_CMTP);
  if (create_nonblock_fd >= 0)
    {
      create_nonblock_flags = fcntl(create_nonblock_fd, F_GETFL);
      create_nonblock_close = close(create_nonblock_fd);
    }

  fd = socket(AF_BLUETOOTH, SOCK_RAW, LINUX_BT_BTPROTO_CMTP);
  if (fd < 0)
    {
      printf("btctl: ordinary-cmtp-socket proto=BTPROTO_CMTP "
             "socket-ret=%d errno=%d final-ok=0\n", fd, errno);
      return 1;
    }

  nonblock_ret = fcntl(fd, F_SETFL, O_NONBLOCK);
  btctl_probe_unsupported_ops(fd, &unsupported);

  memset(&add_req, 0, sizeof(add_req));
  add_req.sock = handle;
  add_req.flags = (uint32_t)(1u << BTCTL_CMTP_LOOPBACK);
  add_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPCONNADD,
                  (unsigned long)&add_req);
  errno = 0;
  dup_add_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPCONNADD,
                      (unsigned long)&add_req);
  dup_add_errno = errno;

  memset(info, 0, sizeof(info));
  memset(&list_req, 0, sizeof(list_req));
  list_req.cnum = 1;
  list_req.ci = info;
  list_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPGETCONNLIST,
                   (unsigned long)&list_req);

  memset(&get_info, 0, sizeof(get_info));
  btctl_cmtp_fill_bdaddr(handle, &get_info.bdaddr);
  info_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPGETCONNINFO,
                   (unsigned long)&get_info);

  memset(&del_req, 0, sizeof(del_req));
  btctl_cmtp_fill_bdaddr(handle, &del_req.bdaddr);
  del_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPCONNDEL,
                  (unsigned long)&del_req);

  memset(&get_info, 0, sizeof(get_info));
  btctl_cmtp_fill_bdaddr(handle, &get_info.bdaddr);
  errno = 0;
  post_info_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPGETCONNINFO,
                        (unsigned long)&get_info);
  post_info_errno = errno;
  errno = 0;
  dup_del_ret = ioctl(fd, LINUX_BT_NATIVE_CMTPCONNDEL,
                      (unsigned long)&del_req);
  dup_del_errno = errno;
  close_ret = close(fd);

  errno = dup_add_errno;
  dup_add_ok = btctl_expected_ioctl_error(dup_add_ret, EALREADY);
  errno = post_info_errno;
  post_info_ok = btctl_expected_ioctl_error(post_info_ret, ENOENT);
  errno = dup_del_errno;
  dup_del_ok = btctl_expected_ioctl_error(dup_del_ret, ENOENT);
  final_ok = add_ret == 0 &&
             create_nonblock_fd >= 0 &&
             create_nonblock_flags >= 0 &&
             (create_nonblock_flags & O_NONBLOCK) != 0 &&
             create_nonblock_close == 0 &&
             nonblock_ret == 0 &&
             unsupported.ok &&
             dup_add_ok &&
             list_ret == 0 &&
             list_req.cnum == 1 &&
             info_ret == 0 &&
             del_ret == 0 &&
             post_info_ok &&
             dup_del_ok &&
             close_ret == 0;

  printf("btctl: ordinary-cmtp-socket proto=BTPROTO_CMTP "
         "socket-ret=%d nonblock-ret=%d create-nonblock-fd=%d "
         "create-nonblock-flags=0x%x create-nonblock-close=%d "
         "create-nonblock-ok=%u unsupported-bind-ret=%d "
         "unsupported-bind-errno=%d unsupported-getsockname-ret=%d "
         "unsupported-getsockname-errno=%d "
         "unsupported-getpeername-ret=%d "
         "unsupported-getpeername-errno=%d unsupported-connect-ret=%d "
         "unsupported-connect-errno=%d unsupported-send-ret=%d "
         "unsupported-send-errno=%d unsupported-recv-ret=%d "
         "unsupported-recv-errno=%d unsupported-listen-ret=%d "
         "unsupported-listen-errno=%d unsupported-shutdown-ret=%d "
         "unsupported-shutdown-errno=%d unsupported-accept-ret=%d "
         "unsupported-accept-errno=%d unsupported-ok=%u "
         "ioctl=CMTPCONNADD ret=%d duplicate-ret=%d "
         "ioctl=CMTPGETCONNLIST ret=%d cnum=%u "
         "ioctl=CMTPGETCONNINFO ret=%d state=%u num=%d "
         "flags=0x%08" PRIx32 " ioctl=CMTPCONNDEL ret=%d "
         "post-del-info-ret=%d missing-del-ret=%d close-ret=%d "
         "path=ordinary-socket final-ok=%d\n",
         fd, nonblock_ret, create_nonblock_fd, create_nonblock_flags,
         create_nonblock_close,
         create_nonblock_fd >= 0 &&
         create_nonblock_flags >= 0 &&
         (create_nonblock_flags & O_NONBLOCK) != 0 &&
         create_nonblock_close == 0,
         unsupported.bind_ret, unsupported.bind_errno,
         unsupported.getsockname_ret, unsupported.getsockname_errno,
         unsupported.getpeername_ret, unsupported.getpeername_errno,
         unsupported.connect_ret, unsupported.connect_errno,
         unsupported.send_ret, unsupported.send_errno,
         unsupported.recv_ret, unsupported.recv_errno,
         unsupported.listen_ret, unsupported.listen_errno,
         unsupported.shutdown_ret, unsupported.shutdown_errno,
         unsupported.accept_ret, unsupported.accept_errno,
         unsupported.ok,
         add_ret, dup_add_ret, list_ret, list_ret == 0 ?
         list_req.cnum : 0, info_ret, info_ret == 0 ? get_info.state : 0,
         info_ret == 0 ? get_info.num : 0,
         info_ret == 0 ? get_info.flags : 0, del_ret, post_info_ret,
         dup_del_ret, close_ret, final_ok);

  return final_ok ? 0 : 1;
}

static int btctl_upstream_ordinary_hidp_socket(uint16_t handle)
{
  struct btctl_hidp_connadd_req_s add_req;
  struct btctl_hidp_conndel_req_s del_req;
  struct btctl_hidp_conninfo_s info[1];
  struct btctl_hidp_conninfo_s get_info;
  struct btctl_hidp_connlist_req_s list_req;
  struct btctl_unsupported_ops_s unsupported;
  int fd;
  int create_nonblock_fd = -1;
  int create_nonblock_flags = -1;
  int create_nonblock_close = -1;
  int nonblock_ret = -1;
  int add_ret;
  int dup_add_ret;
  int dup_add_errno;
  int list_ret;
  int info_ret;
  int del_ret;
  int post_info_ret;
  int post_info_errno;
  int dup_del_ret;
  int dup_del_errno;
  int close_ret;
  int final_ok;
  int dup_add_ok;
  int post_info_ok;
  int dup_del_ok;

  create_nonblock_fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_NONBLOCK,
                              LINUX_BT_BTPROTO_HIDP);
  if (create_nonblock_fd >= 0)
    {
      create_nonblock_flags = fcntl(create_nonblock_fd, F_GETFL);
      create_nonblock_close = close(create_nonblock_fd);
    }

  fd = socket(AF_BLUETOOTH, SOCK_RAW, LINUX_BT_BTPROTO_HIDP);
  if (fd < 0)
    {
      printf("btctl: ordinary-hidp-socket proto=BTPROTO_HIDP "
             "socket-ret=%d errno=%d final-ok=0\n", fd, errno);
      return 1;
    }

  nonblock_ret = fcntl(fd, F_SETFL, O_NONBLOCK);
  btctl_probe_unsupported_ops(fd, &unsupported);

  memset(&add_req, 0, sizeof(add_req));
  add_req.ctrl_sock = handle;
  add_req.intr_sock = (int)handle + 1;
  add_req.parser = 1;
  add_req.subclass = 0x40;
  add_req.vendor = 0x05ac;
  add_req.product = 0x024f;
  add_req.version = 0x0111;
  snprintf(add_req.name, sizeof(add_req.name), "Feather HIDP ordinary");
  add_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPCONNADD,
                  (unsigned long)&add_req);
  errno = 0;
  dup_add_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPCONNADD,
                      (unsigned long)&add_req);
  dup_add_errno = errno;

  memset(info, 0, sizeof(info));
  memset(&list_req, 0, sizeof(list_req));
  list_req.cnum = 1;
  list_req.ci = info;
  list_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPGETCONNLIST,
                   (unsigned long)&list_req);

  memset(&get_info, 0, sizeof(get_info));
  btctl_hidp_fill_bdaddr(handle, &get_info.bdaddr);
  info_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPGETCONNINFO,
                   (unsigned long)&get_info);

  memset(&del_req, 0, sizeof(del_req));
  btctl_hidp_fill_bdaddr(handle, &del_req.bdaddr);
  del_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPCONNDEL,
                  (unsigned long)&del_req);

  memset(&get_info, 0, sizeof(get_info));
  btctl_hidp_fill_bdaddr(handle, &get_info.bdaddr);
  errno = 0;
  post_info_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPGETCONNINFO,
                        (unsigned long)&get_info);
  post_info_errno = errno;
  errno = 0;
  dup_del_ret = ioctl(fd, LINUX_BT_NATIVE_HIDPCONNDEL,
                      (unsigned long)&del_req);
  dup_del_errno = errno;
  close_ret = close(fd);

  errno = dup_add_errno;
  dup_add_ok = btctl_expected_ioctl_error(dup_add_ret, EALREADY);
  errno = post_info_errno;
  post_info_ok = btctl_expected_ioctl_error(post_info_ret, ENOENT);
  errno = dup_del_errno;
  dup_del_ok = btctl_expected_ioctl_error(dup_del_ret, ENOENT);
  final_ok = add_ret == 0 &&
             create_nonblock_fd >= 0 &&
             create_nonblock_flags >= 0 &&
             (create_nonblock_flags & O_NONBLOCK) != 0 &&
             create_nonblock_close == 0 &&
             nonblock_ret == 0 &&
             unsupported.ok &&
             dup_add_ok &&
             list_ret == 0 &&
             list_req.cnum == 1 &&
             info_ret == 0 &&
             del_ret == 0 &&
             post_info_ok &&
             dup_del_ok &&
             close_ret == 0;

  printf("btctl: ordinary-hidp-socket proto=BTPROTO_HIDP "
         "socket-ret=%d nonblock-ret=%d create-nonblock-fd=%d "
         "create-nonblock-flags=0x%x create-nonblock-close=%d "
         "create-nonblock-ok=%u unsupported-bind-ret=%d "
         "unsupported-bind-errno=%d unsupported-getsockname-ret=%d "
         "unsupported-getsockname-errno=%d "
         "unsupported-getpeername-ret=%d "
         "unsupported-getpeername-errno=%d unsupported-connect-ret=%d "
         "unsupported-connect-errno=%d unsupported-send-ret=%d "
         "unsupported-send-errno=%d unsupported-recv-ret=%d "
         "unsupported-recv-errno=%d unsupported-listen-ret=%d "
         "unsupported-listen-errno=%d unsupported-shutdown-ret=%d "
         "unsupported-shutdown-errno=%d unsupported-accept-ret=%d "
         "unsupported-accept-errno=%d unsupported-ok=%u "
         "ioctl=HIDPCONNADD ret=%d duplicate-ret=%d "
         "ioctl=HIDPGETCONNLIST ret=%d cnum=%u "
         "ioctl=HIDPGETCONNINFO ret=%d state=%u "
         "vendor=0x%04x product=0x%04x ioctl=HIDPCONNDEL ret=%d "
         "post-del-info-ret=%d missing-del-ret=%d close-ret=%d "
         "path=ordinary-socket final-ok=%d\n",
         fd, nonblock_ret, create_nonblock_fd, create_nonblock_flags,
         create_nonblock_close,
         create_nonblock_fd >= 0 &&
         create_nonblock_flags >= 0 &&
         (create_nonblock_flags & O_NONBLOCK) != 0 &&
         create_nonblock_close == 0,
         unsupported.bind_ret, unsupported.bind_errno,
         unsupported.getsockname_ret, unsupported.getsockname_errno,
         unsupported.getpeername_ret, unsupported.getpeername_errno,
         unsupported.connect_ret, unsupported.connect_errno,
         unsupported.send_ret, unsupported.send_errno,
         unsupported.recv_ret, unsupported.recv_errno,
         unsupported.listen_ret, unsupported.listen_errno,
         unsupported.shutdown_ret, unsupported.shutdown_errno,
         unsupported.accept_ret, unsupported.accept_errno,
         unsupported.ok,
         add_ret, dup_add_ret, list_ret, list_ret == 0 ?
         list_req.cnum : 0, info_ret, info_ret == 0 ? get_info.state : 0,
         info_ret == 0 ? get_info.vendor : 0,
         info_ret == 0 ? get_info.product : 0, del_ret, post_info_ret,
         dup_del_ret, close_ret, final_ok);

  return final_ok ? 0 : 1;
}

static int btctl_hci_channel(const char *arg)
{
  if (!strcmp(arg, "raw"))
    {
      return LINUX_BT_HCI_CHANNEL_RAW;
    }
  else if (!strcmp(arg, "user"))
    {
      return LINUX_BT_HCI_CHANNEL_USER;
    }
  else if (!strcmp(arg, "monitor"))
    {
      return LINUX_BT_HCI_CHANNEL_MONITOR;
    }
  else if (!strcmp(arg, "control") || !strcmp(arg, "mgmt"))
    {
      return LINUX_BT_HCI_CHANNEL_CONTROL;
    }
  else if (!strcmp(arg, "logging"))
    {
      return LINUX_BT_HCI_CHANNEL_LOGGING;
    }

  return (int)strtol(arg, NULL, 0);
}

static uint8_t btctl_enabled(const char *arg)
{
  if (!strcmp(arg, "on") || !strcmp(arg, "enable") ||
      !strcmp(arg, "enabled") || !strcmp(arg, "1"))
    {
      return 1;
    }

  return 0;
}

static int btctl_hex_nibble(char ch)
{
  if (ch >= '0' && ch <= '9')
    {
      return ch - '0';
    }
  else if (ch >= 'a' && ch <= 'f')
    {
      return ch - 'a' + 10;
    }
  else if (ch >= 'A' && ch <= 'F')
    {
      return ch - 'A' + 10;
    }

  return -1;
}

static int btctl_parse_hex_args(int argc, char *argv[],
                                uint8_t *out, size_t out_len)
{
  size_t used = 0;
  int i;

  for (i = 0; i < argc; i++)
    {
      const char *p = argv[i];

      while (*p != '\0')
        {
          int hi;
          int lo;

          while (*p == ':' || *p == ',' || *p == '-' ||
                 *p == '_' || *p == ' ')
            {
              p++;
            }

          if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
            {
              p += 2;
            }

          if (*p == '\0')
            {
              break;
            }

          hi = btctl_hex_nibble(p[0]);
          lo = btctl_hex_nibble(p[1]);
          if (hi < 0 || lo < 0)
            {
              return -EINVAL;
            }

          if (used >= out_len)
            {
              return -EMSGSIZE;
            }

          out[used++] = (uint8_t)((hi << 4) | lo);
          p += 2;
        }
    }

  return (int)used;
}

static int btctl_mgmt(int argc, char *argv[])
{
  char out[512];
  int ret;
  uint8_t enabled;
  uint16_t opcode;

  if (argc < 1 || !strcmp(argv[0], "status"))
    {
      ret = linux_bt_mgmt_dispatch(LINUX_BT_MGMT_OP_READ_INFO, 0,
                                   out, sizeof(out));
      if (ret < 0)
        {
          printf("btctl: mgmt status failed: %d\n", ret);
          return 1;
        }

      printf("%s", out);
      return 0;
    }

  if (argc < 2)
    {
      btctl_usage();
      return 2;
    }

  enabled = btctl_enabled(argv[1]);

  if (!strcmp(argv[0], "power"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_POWERED;
    }
  else if (!strcmp(argv[0], "connectable"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_CONNECTABLE;
    }
  else if (!strcmp(argv[0], "discoverable"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_DISCOVERABLE;
    }
  else if (!strcmp(argv[0], "bondable"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_BONDABLE;
    }
  else if (!strcmp(argv[0], "le"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_LE;
    }
  else if (!strcmp(argv[0], "bredr"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_BREDR;
    }
  else if (!strcmp(argv[0], "advertising"))
    {
      opcode = LINUX_BT_MGMT_OP_SET_ADVERTISING;
    }
  else
    {
      printf("btctl: unknown mgmt command: %s\n", argv[0]);
      return 2;
    }

  ret = linux_bt_mgmt_dispatch(opcode, enabled, out, sizeof(out));
  if (ret < 0)
    {
      printf("btctl: mgmt %s failed: %d\n", argv[0], ret);
      return 1;
    }

  printf("btctl: mgmt %s %s\n", argv[0], enabled ? "on" : "off");
  if (out[0] != '\0')
    {
      printf("%s", out);
    }

  return 0;
}

static int btctl_poll(uint16_t type)
{
  char out[1024];
  int ret;

  if (type == LINUX_BT_HWSIM_TYPE_ACL)
    {
      ret = linux_bt_acl_poll(out, sizeof(out));
    }
  else if (type == LINUX_BT_HWSIM_TYPE_CTRL)
    {
      ret = linux_bt_ctrl_poll(out, sizeof(out));
    }
  else
    {
      ret = linux_bt_hwsim_read(type, out, sizeof(out));
    }
  if (ret < 0)
    {
      printf("btctl: hwsim poll failed: %d\\n", ret);
      return 1;
    }

  printf("btctl: hwsim records=%d type=%u\\n", ret, type);
  if (out[0] != '\0')
    {
      printf("%s", out);
    }

  return 0;
}

static int btctl_upstream_hci_connect_br(uint16_t peer)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_hci_connect_br_probe(peer, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream hci-connect-br failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream hci-connect-br peer=%u\n", peer);
  return 0;
}

static int btctl_upstream_hci_connect_le(uint16_t peer)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_hci_connect_le_probe(peer, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream hci-connect-le failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream hci-connect-le peer=%u\n", peer);
  return 0;
}

static int btctl_upstream_hci_disconnect_br(uint16_t peer)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_hci_disconnect_br_probe(peer, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream hci-disconnect-br failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream hci-disconnect-br peer=%u\n", peer);
  return 0;
}

static int btctl_upstream_hci_disconnect_le(uint16_t peer)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_hci_disconnect_le_probe(peer, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream hci-disconnect-le failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream hci-disconnect-le peer=%u\n", peer);
  return 0;
}

static uint16_t btctl_upstream_bredr_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static int btctl_upstream_avdtp_send(uint16_t peer,
                                     const uint8_t *payload,
                                     size_t payload_len,
                                     const char *name)
{
  char out[512];
  uint16_t handle;
  int ret;

  handle = btctl_upstream_bredr_handle(peer);
  ret = linux_bt_upstream_l2cap_socket_send_probe(0x0019, 0x0040,
                                                  handle, payload,
                                                  payload_len,
                                                  out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream %s failed: %d\n", name, ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream %s peer=%u handle=0x%04x len=%u\n",
         name, peer, handle, (unsigned int)payload_len);
  return 0;
}

enum btctl_avdtp_sep_state
{
  BTCTL_AVDTP_SEP_IDLE = 0,
  BTCTL_AVDTP_SEP_DISCOVERED,
  BTCTL_AVDTP_SEP_CONFIGURED,
  BTCTL_AVDTP_SEP_OPEN,
  BTCTL_AVDTP_SEP_STREAMING,
};

static enum btctl_avdtp_sep_state g_btctl_avdtp_sink_state =
  BTCTL_AVDTP_SEP_IDLE;
static uint8_t g_btctl_avdtp_seen_req[16][128];
static uint8_t g_btctl_avdtp_seen_rsp[16][128];
static size_t g_btctl_avdtp_seen_req_len[16];
static size_t g_btctl_avdtp_seen_rsp_len[16];
static size_t g_btctl_avdtp_seen_req_count;

static const char *btctl_upstream_avdtp_state_name(
  enum btctl_avdtp_sep_state state)
{
  switch (state)
    {
      case BTCTL_AVDTP_SEP_IDLE:
        return "IDLE";

      case BTCTL_AVDTP_SEP_DISCOVERED:
        return "DISCOVERED";

      case BTCTL_AVDTP_SEP_CONFIGURED:
        return "CONFIGURED";

      case BTCTL_AVDTP_SEP_OPEN:
        return "OPEN";

      case BTCTL_AVDTP_SEP_STREAMING:
        return "STREAMING";

      default:
        return "UNKNOWN";
    }
}

static int btctl_upstream_avdtp_listen(uint16_t handle)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019, 0x0040,
                                                  handle, out,
                                                  sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream avdtp-listen bind failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_l2cap_socket_listen_probe(1, out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream avdtp-listen listen failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  ret = linux_bt_upstream_l2cap_socket_native_control_probe(1, out,
                                                            sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream avdtp-listen native-control failed: %d\n",
             ret);
      return 1;
    }

  printf("%s", out);
  printf("btctl: upstream avdtp signaling listening handle=0x%04x\n",
         handle);
  g_btctl_avdtp_sink_state = BTCTL_AVDTP_SEP_IDLE;
  g_btctl_avdtp_seen_req_count = 0;
  return 0;
}

static int btctl_upstream_avdtp_recv(size_t max_len)
{
  char out[1024];
  int ret;

  ret = linux_bt_upstream_l2cap_socket_recv_probe(max_len, out,
                                                  sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream avdtp-recv failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  return 0;
}

static int btctl_upstream_avdtp_auto_rsp(uint16_t peer)
{
  static const uint8_t caps[] =
  {
    0x01, 0x00,
    0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
  };

  uint8_t req[128];
  uint8_t rsp[128];
  char out[1024];
  size_t req_len;
  size_t rsp_len;
  uint8_t hdr;
  uint8_t signal;
  uint8_t msg_type;
  uint8_t pkt_type;
  uint8_t err = 0;
  enum btctl_avdtp_sep_state old_state;
  enum btctl_avdtp_sep_state new_state;
  int polled = 0;
  int attempt;
  int ret;

  ret = -EAGAIN;
  req_len = 0;
  out[0] = '\0';
  for (attempt = 0; attempt < 100; attempt++)
    {
      int poll_ret = linux_bt_upstream_vhci_poll_medium();

      if (poll_ret < 0)
        {
          printf("btctl: upstream avdtp-auto-rsp poll failed: %d\n",
                 poll_ret);
          return 1;
        }

      polled += poll_ret;
      ret = linux_bt_upstream_l2cap_socket_recv_raw(req, sizeof(req),
                                                    &req_len, out,
                                                    sizeof(out));
      if (ret >= 0 && req_len >= 2)
        {
          break;
        }

      if (ret >= 0)
        {
          ret = -EAGAIN;
        }

      if (ret != -EAGAIN)
        {
          printf("%s", out);
          printf("btctl: upstream avdtp-auto-rsp recv failed: %d\n", ret);
          return 1;
        }

      usleep(50000);
    }

  printf("btctl: upstream avdtp-auto-rsp polled=%d\n", polled);
  printf("%s", out);
  if (ret < 0)
    {
      printf("btctl: upstream avdtp-auto-rsp recv failed: %d\n", ret);
      return 1;
    }

  if (req_len < 2)
    {
      printf("btctl: upstream avdtp-auto-rsp empty request len=%u\n",
             (unsigned int)req_len);
      return req_len == 0 ? 1 : 0;
    }

  for (attempt = 0;
       attempt < (int)g_btctl_avdtp_seen_req_count;
       attempt++)
    {
      if (req_len == g_btctl_avdtp_seen_req_len[attempt] &&
          memcmp(req, g_btctl_avdtp_seen_req[attempt], req_len) == 0)
        {
          printf("btctl: upstream avdtp-auto-rsp duplicate-rsp len=%u\n",
                 (unsigned int)req_len);
          ret = btctl_upstream_avdtp_send(peer,
                                          g_btctl_avdtp_seen_rsp[attempt],
                                          g_btctl_avdtp_seen_rsp_len[attempt],
                                          "avdtp-auto-rsp-duplicate");
          return ret == 0 ? 2 : ret;
        }
    }

  hdr = req[0];
  signal = req[1];
  msg_type = (uint8_t)(hdr & 0x03);
  pkt_type = (uint8_t)((hdr >> 2) & 0x03);
  rsp[0] = (uint8_t)((hdr & 0xf0) | 0x02);
  rsp[1] = signal;
  rsp_len = 2;
  old_state = g_btctl_avdtp_sink_state;
  new_state = old_state;

  if (pkt_type != 0 || msg_type != 0)
    {
      printf("btctl: upstream avdtp-auto-rsp non-command-skip "
             "signal=0x%02x msg-type=0x%02x pkt-type=0x%02x len=%u\n",
             signal, msg_type, pkt_type, (unsigned int)req_len);
      return 2;
    }
  else
    {
      switch (signal)
        {
          case 0x01:
            rsp[2] = 0x04;
            rsp[3] = 0x08;
            rsp_len = 4;
            if (old_state == BTCTL_AVDTP_SEP_IDLE)
              {
                new_state = BTCTL_AVDTP_SEP_DISCOVERED;
              }
            break;

          case 0x02:
            if (old_state == BTCTL_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            break;

          case 0x03:
            if (old_state == BTCTL_AVDTP_SEP_DISCOVERED)
              {
                new_state = BTCTL_AVDTP_SEP_CONFIGURED;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x04:
            if (old_state == BTCTL_AVDTP_SEP_CONFIGURED ||
                old_state == BTCTL_AVDTP_SEP_OPEN ||
                old_state == BTCTL_AVDTP_SEP_STREAMING)
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x05:
            if (old_state == BTCTL_AVDTP_SEP_OPEN)
              {
                new_state = BTCTL_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x06:
            if (old_state == BTCTL_AVDTP_SEP_CONFIGURED)
              {
                new_state = BTCTL_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x07:
            if (old_state == BTCTL_AVDTP_SEP_OPEN)
              {
                new_state = BTCTL_AVDTP_SEP_STREAMING;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x08:
            if (old_state == BTCTL_AVDTP_SEP_OPEN ||
                old_state == BTCTL_AVDTP_SEP_STREAMING)
              {
                new_state = BTCTL_AVDTP_SEP_IDLE;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x09:
            if (old_state == BTCTL_AVDTP_SEP_STREAMING)
              {
                new_state = BTCTL_AVDTP_SEP_OPEN;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x0a:
            if (old_state == BTCTL_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                new_state = BTCTL_AVDTP_SEP_IDLE;
              }
            break;

          case 0x0b:
            if (old_state == BTCTL_AVDTP_SEP_CONFIGURED ||
                old_state == BTCTL_AVDTP_SEP_OPEN ||
                old_state == BTCTL_AVDTP_SEP_STREAMING)
              {
                new_state = old_state;
              }
            else
              {
                err = 0x31;
              }
            break;

          case 0x0c:
            if (old_state == BTCTL_AVDTP_SEP_IDLE)
              {
                err = 0x31;
              }
            else
              {
                memcpy(&rsp[2], caps, sizeof(caps));
                rsp_len = 2 + sizeof(caps);
              }
            break;

          case 0x0d:
            if (old_state == BTCTL_AVDTP_SEP_OPEN ||
                old_state == BTCTL_AVDTP_SEP_STREAMING)
              {
                new_state = old_state;
              }
            else
              {
                err = 0x31;
              }
            break;

          default:
            err = 0x11;
            break;
        }
    }

  if (err != 0)
    {
      rsp[0] = (uint8_t)((hdr & 0xf0) | 0x03);
      rsp[1] = signal;
      rsp[2] = err;
      rsp_len = 3;
      new_state = old_state;
    }
  else
    {
      g_btctl_avdtp_sink_state = new_state;
    }

  ret = btctl_upstream_avdtp_send(peer, rsp, rsp_len,
                                  "avdtp-auto-rsp");
  if (ret != 0)
    {
      return ret;
    }

  if (g_btctl_avdtp_seen_req_count <
      sizeof(g_btctl_avdtp_seen_req) /
      sizeof(g_btctl_avdtp_seen_req[0]))
    {
      size_t seen_len = req_len;

      if (seen_len > sizeof(g_btctl_avdtp_seen_req[0]))
        {
          seen_len = sizeof(g_btctl_avdtp_seen_req[0]);
        }

      memcpy(g_btctl_avdtp_seen_req[g_btctl_avdtp_seen_req_count],
             req, seen_len);
      g_btctl_avdtp_seen_req_len[g_btctl_avdtp_seen_req_count] =
        seen_len;
      memcpy(g_btctl_avdtp_seen_rsp[g_btctl_avdtp_seen_req_count],
             rsp, rsp_len);
      g_btctl_avdtp_seen_rsp_len[g_btctl_avdtp_seen_req_count] =
        rsp_len;
      g_btctl_avdtp_seen_req_count++;
    }

  printf("btctl: upstream avdtp-auto-rsp signal=0x%02x "
         "msg-type=0x%02x pkt-type=0x%02x rsp-len=%u "
         "err=0x%02x state=%s->%s\n",
         signal, msg_type, pkt_type, (unsigned int)rsp_len, err,
         btctl_upstream_avdtp_state_name(old_state),
         btctl_upstream_avdtp_state_name(new_state));
  return 0;
}

static int btctl_upstream_avdtp_auto_rsp_loop(uint16_t peer,
                                              unsigned int count)
{
  unsigned int done;
  int ret;

  done = 0;
  while (done < count)
    {
      ret = btctl_upstream_avdtp_auto_rsp(peer);
      if (ret == 2)
        {
          usleep(250000);
          continue;
        }

      if (ret != 0)
        {
          printf("btctl: upstream avdtp-auto-rsp-loop failed "
                 "done=%u count=%u ret=%d\n",
                 done, count, ret);
          return ret;
        }

      done++;
    }

  printf("btctl: upstream avdtp-auto-rsp-loop complete count=%u\n",
         count);
  return 0;
}

static int btctl_upstream_avdtp_close(void)
{
  char out[512];
  int ret;

  ret = linux_bt_upstream_l2cap_socket_close_probe(out, sizeof(out));
  if (ret < 0)
    {
      printf("%s", out);
      printf("btctl: upstream avdtp-close failed: %d\n", ret);
      return 1;
    }

  printf("%s", out);
  g_btctl_avdtp_sink_state = BTCTL_AVDTP_SEP_IDLE;
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
      btctl_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "info"))
    {
      char out[256];
      int ret = linux_bt_info(out, sizeof(out));

      if (ret < 0)
        {
          printf("btctl: info failed: %d\n", ret);
          return 1;
        }

      printf("%s", out);
      return 0;
    }

  if (!strcmp(argv[1], "upstream"))
    {
      char out[12000];
      int ret;

      if (argc >= 3 && !strcmp(argv[2], "status"))
        {
          ret = linux_bt_upstream_vhci_status(out, sizeof(out));
          if (ret < 0)
            {
              printf("btctl: upstream status failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "hci-status"))
        {
          ret = linux_bt_upstream_hci_status(out, sizeof(out));
          if (ret < 0)
            {
              printf("btctl: upstream hci-status failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "poll"))
        {
          ret = linux_bt_upstream_vhci_poll_medium();
          if (ret < 0)
            {
              printf("btctl: upstream poll failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream raw records=%d\n", ret);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "pump"))
        {
          int drained;
          int polled;

          drained = linux_bt_upstream_vhci_drain_default();
          if (drained < 0)
            {
              printf("btctl: upstream pump drain failed: %d\n", drained);
              return 1;
            }

          polled = linux_bt_upstream_vhci_poll_medium();
          if (polled < 0)
            {
              printf("btctl: upstream pump poll failed: %d\n", polled);
              return 1;
            }

          printf("btctl: upstream pump drained=%d polled=%d\n",
                 drained, polled);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "hci-pump"))
        {
          unsigned int rounds = argc >= 4 ?
            (unsigned int)strtoul(argv[3], NULL, 0) : 1;

          ret = linux_bt_upstream_hci_hwsim_pump(rounds);
          if (ret < 0)
            {
              printf("btctl: upstream hci-pump failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream hci-pump rounds=%u records=%d\n",
                 rounds, ret);
          return 0;
        }

#ifdef CONFIG_NET_LINUX_BLUETOOTH_6LOWPAN_BRIDGE
      if (argc >= 3 && !strcmp(argv[2], "6lowpan-up"))
        {
          char ifname[16];
          const char *name = argc >= 4 ? argv[3] : NULL;

          ret = linux_bt_6lowpan_netdev_register(name, ifname,
                                                 sizeof(ifname));
          if (ret < 0)
            {
              printf("btctl: upstream 6lowpan-up failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream 6lowpan-up ifname=%s\n", ifname);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "6lowpan-status"))
        {
          ret = linux_bt_6lowpan_status(out, sizeof(out));
          if (ret < 0)
            {
              printf("btctl: upstream 6lowpan-status failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "6lowpan-down"))
        {
          linux_bt_6lowpan_netdev_unregister();
          printf("btctl: upstream 6lowpan-down complete\n");
          return 0;
        }
#endif

      if (argc >= 3 && !strcmp(argv[2], "bridge"))
        {
          unsigned int rounds = argc >= 4 ?
            (unsigned int)strtoul(argv[3], NULL, 0) : 8;
          size_t max_records = argc >= 5 ?
            (size_t)strtoul(argv[4], NULL, 0) : 32;
          int total_drained = 0;
          int total_polled = 0;
          unsigned int i;

          if (rounds == 0)
            {
              rounds = 1;
            }

          if (max_records == 0)
            {
              max_records = 32;
            }

          for (i = 0; i < rounds; i++)
            {
              int drained;
              int polled;

              polled = linux_bt_upstream_vhci_poll_medium();
              if (polled < 0)
                {
                  printf("btctl: upstream bridge poll failed "
                         "round=%u ret=%d\n", i, polled);
                  return 1;
                }

              drained =
                linux_bt_upstream_vhci_drain_default_trace(max_records,
                                                           out,
                                                           sizeof(out));
              if (drained < 0)
                {
                  printf("btctl: upstream bridge drain failed "
                         "round=%u ret=%d\n", i, drained);
                  return 1;
                }

              printf("btctl: upstream bridge round=%u polled=%d "
                     "drained=%d\n", i, polled, drained);
              printf("%s", out);

              total_polled += polled;
              total_drained += drained;
            }

          printf("btctl: upstream bridge rounds=%u total-polled=%d "
                 "total-drained=%d\n",
                 rounds, total_polled, total_drained);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "socket"))
        {
          int proto = btctl_btproto(argv[3]);
          int channel = -1;
          int dev = -1;

          if (proto == LINUX_BT_BTPROTO_L2CAP)
            {
              if (argc >= 5)
                {
                  channel = (int)strtol(argv[4], NULL, 0);
                  dev = 0;
                }

              if (argc >= 6)
                {
                  dev = (int)strtol(argv[5], NULL, 0);
                }
            }
          else if (proto == LINUX_BT_BTPROTO_ISO)
            {
              if (argc >= 5)
                {
                  channel = (int)strtol(argv[4], NULL, 0);
                }
            }
          else if (argc >= 5)
            {
              channel = btctl_hci_channel(argv[4]);
              dev = channel == LINUX_BT_HCI_CHANNEL_RAW ||
                    channel == LINUX_BT_HCI_CHANNEL_USER ? 0 :
                    LINUX_BT_HCI_DEV_NONE;

              if (argc >= 6)
                {
                  dev = (int)strtol(argv[5], NULL, 0);
                }
            }

          ret = linux_bt_upstream_socket_probe_bind(proto, channel, dev,
                                                    out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream socket probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 7 && !strcmp(argv[2], "socket-send"))
        {
          uint8_t payload[256];
          int channel = btctl_hci_channel(argv[3]);
          int dev = (int)strtol(argv[4], NULL, 0);
          uint8_t pkt_type = btctl_hci_type(argv[5]);
          int payload_len;

          payload_len = btctl_parse_hex_args(argc - 6, &argv[6],
                                             payload, sizeof(payload));
          if (payload_len < 0)
            {
              printf("btctl: invalid socket-send payload: %d\n",
                     payload_len);
              return 1;
            }

          ret = linux_bt_upstream_socket_send_probe(channel, dev, pkt_type,
                                                    payload, payload_len,
                                                    out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream socket-send probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "socket-cmtp"))
        {
          uint16_t handle = argc >= 4 ?
            (uint16_t)strtoul(argv[3], NULL, 0) : 0x00c5;

          ret = linux_bt_upstream_cmtp_socket_session_probe(handle,
                                                            out,
                                                            sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream socket-cmtp probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "ordinary-cmtp-socket"))
        {
          uint16_t handle = argc >= 4 ?
            (uint16_t)strtoul(argv[3], NULL, 0) : 0x00c5;

          return btctl_upstream_ordinary_cmtp_socket(handle);
        }

      if (argc >= 3 && !strcmp(argv[2], "ordinary-hidp-socket"))
        {
          uint16_t handle = argc >= 4 ?
            (uint16_t)strtoul(argv[3], NULL, 0) : 0x00a5;

          return btctl_upstream_ordinary_hidp_socket(handle);
        }

      if (argc >= 4 && !strcmp(argv[2], "bnep-ioctl"))
        {
          int action = btctl_bnep_ioctl_action(argv[3]);
          uint32_t param = 0;

          if (argc >= 5)
            {
              param = !strcmp(argv[4], "l2cap") ||
                      !strcmp(argv[4], "kept-l2cap") ?
                      UINT32_MAX : (uint32_t)strtoul(argv[4], NULL, 0);
            }

          ret = linux_bt_upstream_bnep_ioctl_probe(action, param,
                                                   out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream bnep-ioctl probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 7 && !strcmp(argv[2], "socket-filter"))
        {
          int dev = (int)strtol(argv[3], NULL, 0);
          uint32_t type_mask = (uint32_t)strtoul(argv[4], NULL, 0);
          uint32_t event_mask0 = (uint32_t)strtoul(argv[5], NULL, 0);
          uint32_t event_mask1 = (uint32_t)strtoul(argv[6], NULL, 0);
          uint16_t opcode = argc >= 8 ? (uint16_t)strtoul(argv[7],
                                                          NULL, 0) : 0;

          ret = linux_bt_upstream_hci_filter_probe(dev, type_mask,
                                                   event_mask0,
                                                   event_mask1,
                                                   opcode, out,
                                                   sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream socket-filter probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "socket-ioctl"))
        {
          uint16_t dev = argc >= 4 ? (uint16_t)strtoul(argv[3],
                                                       NULL, 0) : 0;
          uint32_t dev_opt = argc >= 6 ? (uint32_t)strtoul(argv[5],
                                                          NULL, 0) : 0;
          int action = 0;

          if (argc >= 5)
            {
              if (!strcmp(argv[4], "up"))
                {
                  action = 1;
                }
              else if (!strcmp(argv[4], "down"))
                {
                  action = 2;
                }
              else if (!strcmp(argv[4], "reset"))
                {
                  action = 3;
                }
              else if (!strcmp(argv[4], "restat"))
                {
                  action = 4;
                }
              else if (!strcmp(argv[4], "scan"))
                {
                  action = 5;
                }
              else if (!strcmp(argv[4], "auth"))
                {
                  action = 6;
                }
              else if (!strcmp(argv[4], "encrypt"))
                {
                  action = 7;
                }
              else if (!strcmp(argv[4], "ptype"))
                {
                  action = 8;
                }
              else if (!strcmp(argv[4], "linkpol"))
                {
                  action = 9;
                }
              else if (!strcmp(argv[4], "linkmode"))
                {
                  action = 10;
                }
              else if (!strcmp(argv[4], "aclmtu"))
                {
                  action = 11;
                }
              else if (!strcmp(argv[4], "scomtu"))
                {
                  action = 12;
                }
              else if (!strcmp(argv[4], "connlist"))
                {
                  action = 13;
                }
              else if (!strcmp(argv[4], "conninfo"))
                {
                  action = 14;
                  dev_opt = argc >= 6 ? btctl_hci_conn_type(argv[5]) :
                                        btctl_hci_conn_type(NULL);
                }
              else if (!strcmp(argv[4], "authinfo"))
                {
                  action = 15;
                }
              else if (!strcmp(argv[4], "block"))
                {
                  action = 16;
                }
              else if (!strcmp(argv[4], "unblock"))
                {
                  action = 17;
                }
              else
                {
                  printf("btctl: unknown socket-ioctl action: %s\n",
                         argv[4]);
                  return 1;
                }
            }

          ret = linux_bt_upstream_hci_ioctl_probe(dev, action, dev_opt, out,
                                                  sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream socket-ioctl probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 6 && !strcmp(argv[2], "l2cap-bind"))
        {
          uint16_t psm = (uint16_t)strtoul(argv[3], NULL, 0);
          uint16_t cid = (uint16_t)strtoul(argv[4], NULL, 0);
          uint16_t handle = (uint16_t)strtoul(argv[5], NULL, 0);

          ret = linux_bt_upstream_l2cap_socket_bind_probe(psm, cid, handle,
                                                          out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-bind probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 5 && !strcmp(argv[2], "l2cap-connect"))
        {
          uint16_t psm = (uint16_t)strtoul(argv[3], NULL, 0);
          uint16_t cid = (uint16_t)strtoul(argv[4], NULL, 0);

          ret = linux_bt_upstream_l2cap_socket_connect_probe(psm, cid,
                                                             out,
                                                             sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-connect probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "l2cap-listen"))
        {
          int backlog = argc >= 4 ? (int)strtol(argv[3], NULL, 0) : 1;

          ret = linux_bt_upstream_l2cap_socket_listen_probe(backlog,
                                                            out,
                                                            sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-listen probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "l2cap-recv"))
        {
          size_t max_len = argc >= 4 ? (size_t)strtoul(argv[3], NULL, 0) :
                           0;

          ret = linux_bt_upstream_l2cap_socket_recv_probe(max_len, out,
                                                          sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-recv probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "l2cap-write"))
        {
          uint8_t payload[512];
          int payload_len;

          payload_len = btctl_parse_hex_args(argc - 3, &argv[3],
                                             payload, sizeof(payload));
          if (payload_len < 0)
            {
              printf("btctl: invalid l2cap-write payload: %d\n",
                     payload_len);
              return 1;
            }

          ret = linux_bt_upstream_l2cap_socket_write_probe(payload,
                                                           payload_len,
                                                           out,
                                                           sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-write probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "l2cap-close"))
        {
          ret = linux_bt_upstream_l2cap_socket_close_probe(out,
                                                           sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-close probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 7 && !strcmp(argv[2], "l2cap-send"))
        {
          uint8_t payload[512];
          uint16_t psm = (uint16_t)strtoul(argv[3], NULL, 0);
          uint16_t cid = (uint16_t)strtoul(argv[4], NULL, 0);
          uint16_t handle = (uint16_t)strtoul(argv[5], NULL, 0);
          int payload_len;

          payload_len = btctl_parse_hex_args(argc - 6, &argv[6],
                                             payload, sizeof(payload));
          if (payload_len < 0)
            {
              printf("btctl: invalid l2cap-send payload: %d\n",
                     payload_len);
              return 1;
            }

          ret = linux_bt_upstream_l2cap_socket_send_probe(psm, cid, handle,
                                                          payload,
                                                          payload_len,
                                                          out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream l2cap-send probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-listen"))
        {
          uint16_t handle = (uint16_t)strtoul(argv[3], NULL, 0);

          return btctl_upstream_avdtp_listen(handle);
        }

      if (argc >= 3 && !strcmp(argv[2], "avdtp-recv"))
        {
          size_t max_len = argc >= 4 ? (size_t)strtoul(argv[3], NULL, 0) :
                           64;

          return btctl_upstream_avdtp_recv(max_len);
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-auto-rsp"))
        {
          return btctl_upstream_avdtp_auto_rsp(btctl_peer(argv[3]));
        }

      if (argc >= 5 && !strcmp(argv[2], "avdtp-auto-rsp-loop"))
        {
          return btctl_upstream_avdtp_auto_rsp_loop(
                   btctl_peer(argv[3]),
                   (unsigned int)strtoul(argv[4], NULL, 0));
        }

      if (argc >= 3 && !strcmp(argv[2], "avdtp-close"))
        {
          return btctl_upstream_avdtp_close();
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-discover"))
        {
          static const uint8_t payload[] =
          {
            0x10, 0x01
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-discover");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-discover-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x12, 0x01, 0x04, 0x08
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-discover-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getcap"))
        {
          static const uint8_t payload[] =
          {
            0x20, 0x02, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getcap");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getcap-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x22, 0x02,
            0x01, 0x00,
            0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getcap-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-setconfig"))
        {
          static const uint8_t payload[] =
          {
            0x30, 0x03,
            0x04, 0x08,
            0x01, 0x00,
            0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-setconfig");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-setconfig-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x32, 0x03
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-setconfig-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-open"))
        {
          static const uint8_t payload[] =
          {
            0x40, 0x06, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-open");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-open-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x42, 0x06
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-open-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-start"))
        {
          static const uint8_t payload[] =
          {
            0x50, 0x07, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-start");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-start-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x52, 0x07
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-start-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-suspend"))
        {
          static const uint8_t payload[] =
          {
            0x60, 0x09, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-suspend");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-suspend-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x62, 0x09
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-suspend-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-close-stream"))
        {
          static const uint8_t payload[] =
          {
            0x70, 0x08, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-close-stream");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-close-stream-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x72, 0x08
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-close-stream-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-reconfigure"))
        {
          static const uint8_t payload[] =
          {
            0x80, 0x05,
            0x04,
            0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-reconfigure");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-reconfigure-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x82, 0x05
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-reconfigure-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-abort"))
        {
          static const uint8_t payload[] =
          {
            0x90, 0x0a, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-abort");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-abort-rsp"))
        {
          static const uint8_t payload[] =
          {
            0x92, 0x0a
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-abort-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-delay-report"))
        {
          static const uint8_t payload[] =
          {
            0xa0, 0x0d, 0x04, 0x00, 0x64
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-delay-report");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-delay-report-rsp"))
        {
          static const uint8_t payload[] =
          {
            0xa2, 0x0d
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-delay-report-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getconfig"))
        {
          static const uint8_t payload[] =
          {
            0xb0, 0x04, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getconfig");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getconfig-rsp"))
        {
          static const uint8_t payload[] =
          {
            0xb2, 0x04,
            0x01, 0x00,
            0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getconfig-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getallcap"))
        {
          static const uint8_t payload[] =
          {
            0xc0, 0x0c, 0x04
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getallcap");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-getallcap-rsp"))
        {
          static const uint8_t payload[] =
          {
            0xc2, 0x0c,
            0x01, 0x00,
            0x07, 0x06, 0x00, 0x00, 0xff, 0xff, 0x02, 0x35
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-getallcap-rsp");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-security-control"))
        {
          static const uint8_t payload[] =
          {
            0xd0, 0x0b, 0x04, 0x01, 0x02
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-security-control");
        }

      if (argc >= 4 && !strcmp(argv[2], "avdtp-security-control-rsp"))
        {
          static const uint8_t payload[] =
          {
            0xd2, 0x0b
          };

          return btctl_upstream_avdtp_send(btctl_peer(argv[3]),
                                           payload, sizeof(payload),
                                           "avdtp-security-control-rsp");
        }

      if (argc >= 5 && !strcmp(argv[2], "iso-bind"))
        {
          uint8_t addr_type = (uint8_t)strtoul(argv[3], NULL, 0);
          uint16_t handle = (uint16_t)strtoul(argv[4], NULL, 0);

          ret = linux_bt_upstream_iso_socket_bind_probe(addr_type, handle,
                                                        out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-bind probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "iso-connect"))
        {
          uint8_t addr_type = (uint8_t)strtoul(argv[3], NULL, 0);

          ret = linux_bt_upstream_iso_socket_connect_probe(addr_type, out,
                                                           sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-connect probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "iso-recv"))
        {
          size_t max_len = argc >= 4 ? (size_t)strtoul(argv[3], NULL, 0) :
                           0;

          ret = linux_bt_upstream_iso_socket_recv_probe(max_len, out,
                                                        sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-recv probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "iso-write"))
        {
          uint8_t payload[251];
          int payload_len;

          payload_len = btctl_parse_hex_args(argc - 3, &argv[3],
                                             payload, sizeof(payload));
          if (payload_len < 0)
            {
              printf("btctl: invalid iso-write payload: %d\n",
                     payload_len);
              return 1;
            }

          ret = linux_bt_upstream_iso_socket_write_probe(payload,
                                                         payload_len,
                                                         out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-write probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "iso-close"))
        {
          ret = linux_bt_upstream_iso_socket_close_probe(out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-close probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 6 && !strcmp(argv[2], "iso-send"))
        {
          uint8_t payload[251];
          uint8_t addr_type = (uint8_t)strtoul(argv[3], NULL, 0);
          uint16_t handle = (uint16_t)strtoul(argv[4], NULL, 0);
          int payload_len;

          payload_len = btctl_parse_hex_args(argc - 5, &argv[5],
                                             payload, sizeof(payload));
          if (payload_len < 0)
            {
              printf("btctl: invalid iso-send payload: %d\n",
                     payload_len);
              return 1;
            }

          ret = linux_bt_upstream_iso_socket_send_probe(addr_type, handle,
                                                        payload, payload_len,
                                                        out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream iso-send probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "mgmt-send"))
        {
          uint16_t opcode = (uint16_t)strtoul(argv[3], NULL, 0);
          uint16_t index = argc >= 5 ?
                           (uint16_t)strtoul(argv[4], NULL, 0) : 0;
          uint8_t param = argc >= 6 ?
                          (uint8_t)strtoul(argv[5], NULL, 0) : 0;

          ret = linux_bt_upstream_mgmt_send_probe(opcode, index, param,
                                                  out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt-send failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "mgmt-socket"))
        {
          uint16_t opcode = (uint16_t)strtoul(argv[3], NULL, 0);
          uint16_t index = 0;
          uint8_t param = argc >= 6 ? (uint8_t)strtoul(argv[5], NULL, 0) :
                          0;

          if (opcode == 0x0001 || opcode == 0x0002 || opcode == 0x0003)
            {
              index = LINUX_BT_MGMT_INDEX_NONE;
            }

          if (argc >= 5)
            {
              index = (uint16_t)strtoul(argv[4], NULL, 0);
            }

          ret = linux_bt_upstream_mgmt_socket_probe(opcode, index, param,
                                                    out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt socket probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "mgmt-listen"))
        {
          ret = linux_bt_upstream_mgmt_listen_probe(out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt-listen probe failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "mgmt-read"))
        {
          size_t max_len = argc >= 4 ? (size_t)strtoul(argv[3], NULL, 0) :
                           260;

          ret = linux_bt_upstream_mgmt_read_probe(max_len, out,
                                                  sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt-read probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "mgmt-close"))
        {
          ret = linux_bt_upstream_mgmt_close_probe(out, sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt-close probe failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "mgmt-poll-discovery"))
        {
          size_t max_records = argc >= 4 ? (size_t)strtoul(argv[3],
                                                           NULL, 0) : 8;

          ret = linux_bt_upstream_mgmt_poll_discovery_probe(max_records,
                                                            out,
                                                            sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btctl: upstream mgmt-poll-discovery failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "open"))
        {
          ret = linux_bt_upstream_vhci_open_default();
          if (ret < 0)
            {
              printf("btctl: upstream open failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream vhci opened\n");
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "close"))
        {
          ret = linux_bt_upstream_vhci_close_default();
          if (ret < 0)
            {
              printf("btctl: upstream close failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream vhci closed\n");
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "create"))
        {
          unsigned long opcode = 0;

          if (argc >= 4)
            {
              opcode = strtoul(argv[3], NULL, 0);
              if (opcode > 0xff)
                {
                  printf("btctl: invalid vhci opcode: %s\n", argv[3]);
                  return 2;
                }
            }

          ret = linux_bt_upstream_vhci_create_default((uint8_t)opcode);
          if (ret < 0)
            {
              printf("btctl: upstream create failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream vhci create opcode=0x%02lx\n", opcode);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "read"))
        {
          uint8_t buf[256];
          uint32_t len;
          unsigned int i;

          ret = linux_bt_upstream_vhci_read_default(buf, sizeof(buf),
                                                    &len);
          if (ret < 0)
            {
              printf("btctl: upstream read failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream read len=%u", (unsigned int)len);
          for (i = 0; i < len; i++)
            {
              printf(" %02x", buf[i]);
            }

          printf("\n");
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "drain"))
        {
          ret = linux_bt_upstream_vhci_drain_default();
          if (ret < 0)
            {
              printf("btctl: upstream drain failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream drained records=%d\n", ret);
          return 0;
        }

      if (argc >= 3 && !strcmp(argv[2], "drain-trace"))
        {
          size_t max_records = argc >= 4 ? (size_t)strtoul(argv[3],
                                                           NULL, 0) : 32;

          ret = linux_bt_upstream_vhci_drain_default_trace(max_records,
                                                           out,
                                                           sizeof(out));
          if (ret < 0)
            {
              printf("btctl: upstream drain-trace failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          printf("btctl: upstream drain-trace records=%d\n", ret);
          return 0;
        }

      if (argc >= 4 && !strcmp(argv[2], "hci-connect-br"))
        {
          return btctl_upstream_hci_connect_br(btctl_peer(argv[3]));
        }

      if (argc >= 4 && !strcmp(argv[2], "hci-disconnect-br"))
        {
          return btctl_upstream_hci_disconnect_br(btctl_peer(argv[3]));
        }

      if (argc >= 4 && !strcmp(argv[2], "hci-connect-le"))
        {
          return btctl_upstream_hci_connect_le(btctl_peer(argv[3]));
        }

      if (argc >= 4 && !strcmp(argv[2], "hci-disconnect-le"))
        {
          return btctl_upstream_hci_disconnect_le(btctl_peer(argv[3]));
        }

      if (argc >= 3 && !strcmp(argv[2], "a2dp-source-sample"))
        {
          uint16_t peer = argc >= 4 ? btctl_peer(argv[3]) : 0;

          ret = linux_bt_upstream_a2dp_source_sample_peer(peer);
          if (ret < 0)
            {
              printf("btctl: upstream a2dp-source-sample failed: %d\n",
                     ret);
              return 1;
            }

          printf("btctl: upstream a2dp source sample queued peer=%u\n",
                 peer);
          return 0;
        }

      if (argc >= 5 && !strcmp(argv[2], "le-audio-source-sample"))
        {
          uint8_t big = (uint8_t)strtoul(argv[3], NULL, 0);
          uint8_t bis = (uint8_t)strtoul(argv[4], NULL, 0);

          ret = linux_bt_upstream_le_audio_source_sample(big, bis);
          if (ret < 0)
            {
              printf("btctl: upstream le-audio-source-sample failed: %d\n",
                     ret);
              return 1;
            }

          printf("btctl: upstream le-audio source sample queued big=%u "
                 "bis=%u\n", big, bis);
          return 0;
        }

      if (argc >= 5 && !strcmp(argv[2], "send"))
        {
          uint8_t type = btctl_hci_type(argv[3]);

          if (type == 0)
            {
              printf("btctl: unknown upstream hci type: %s\n", argv[3]);
              return 2;
            }

          ret = linux_bt_upstream_hci_send(type, argv[4],
                                           strlen(argv[4]));
          if (ret < 0)
            {
              printf("btctl: upstream send failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream sent type=%s len=%u\n",
                 argv[3], (unsigned int)strlen(argv[4]));
          return 0;
        }

      if (argc >= 5 && !strcmp(argv[2], "sendhex"))
        {
          uint8_t payload[256];
          uint8_t type = btctl_hci_type(argv[3]);
          int len;

          if (type == 0)
            {
              printf("btctl: unknown upstream hci type: %s\n", argv[3]);
              return 2;
            }

          len = btctl_parse_hex_args(argc - 4, &argv[4],
                                     payload, sizeof(payload));
          if (len < 0)
            {
              printf("btctl: invalid upstream hex payload: %d\n", len);
              return 2;
            }

          ret = linux_bt_upstream_hci_send(type, payload, (size_t)len);
          if (ret < 0)
            {
              printf("btctl: upstream sendhex failed: %d\n", ret);
              return 1;
            }

          printf("btctl: upstream sent type=%s hex-len=%u\n",
                 argv[3], (unsigned int)len);
          return 0;
        }

      ret = linux_bt_upstream_vhci_status(out, sizeof(out));

      if (ret < 0)
        {
          printf("btctl: upstream status failed: %d\n", ret);
          return 1;
        }

      printf("%s", out);
      return 0;
    }

  if (!strcmp(argv[1], "state"))
    {
      char out[1024];
      int ret = linux_bt_state(out, sizeof(out));

      if (ret < 0)
        {
          printf("btctl: state failed: %d\n", ret);
          return 1;
        }

      printf("btctl: state\n");
      printf("%s", out);
      return 0;
    }

  if (!strcmp(argv[1], "events"))
    {
      char out[1024];
      int ret = linux_bt_events(out, sizeof(out));

      if (ret < 0)
        {
          printf("btctl: events failed: %d\n", ret);
          return 1;
        }

      printf("%s", out);
      return 0;
    }

  if (!strcmp(argv[1], "mgmt"))
    {
      return btctl_mgmt(argc - 2, &argv[2]);
    }

  if (!strcmp(argv[1], "scan") && argc >= 3)
    {
      char out[1024];
      int ret;

      if (!strcmp(argv[2], "bredr"))
        {
          ret = linux_bt_scan_bredr(out, sizeof(out));
        }
      else if (!strcmp(argv[2], "le"))
        {
          ret = linux_bt_scan_le(out, sizeof(out));
        }
      else
        {
          return btctl_poll(btctl_type(argv[2]));
        }

      if (ret < 0)
        {
          printf("btctl: scan failed: %d\n", ret);
          return 1;
        }

      printf("btctl: scan records=%d transport=%s\n", ret, argv[2]);
      if (out[0] != '\0')
        {
          printf("%s", out);
        }

      return 0;
    }

  if (!strcmp(argv[1], "poll") && argc >= 3)
    {
      return btctl_poll(btctl_type(argv[2]));
    }

  if (!strcmp(argv[1], "advertise") && argc >= 3 &&
      !strcmp(argv[2], "start"))
    {
      int ret = linux_bt_advertise_start();

      if (ret < 0)
        {
          printf("btctl: advertise start failed: %d\n", ret);
          return 1;
        }

      printf("btctl: advertise started\n");
      return 0;
    }

  if (!strcmp(argv[1], "advertise") && argc >= 3 &&
      !strcmp(argv[2], "stop"))
    {
      int ret = linux_bt_advertise_stop();

      if (ret < 0)
        {
          printf("btctl: advertise stop failed: %d\n", ret);
          return 1;
        }

      printf("btctl: advertise stopped\n");
      return 0;
    }

  if (!strcmp(argv[1], "connect") && argc >= 3)
    {
      int ret = linux_bt_connect(btctl_peer(argv[2]));

      if (ret < 0)
        {
          printf("btctl: connect failed: %d\n", ret);
          return 1;
        }

      printf("btctl: connect requested peer=%s\n", argv[2]);
      return 0;
    }

  if (!strcmp(argv[1], "disconnect") && argc >= 3)
    {
      int ret = linux_bt_disconnect(btctl_peer(argv[2]));

      if (ret < 0)
        {
          printf("btctl: disconnect failed: %d\n", ret);
          return 1;
        }

      printf("btctl: disconnect requested peer=%s\n", argv[2]);
      return 0;
    }

  if (!strcmp(argv[1], "pair") && argc >= 3)
    {
      int ret = linux_bt_pair(btctl_peer(argv[2]));

      if (ret < 0)
        {
          printf("btctl: pair failed: %d\n", ret);
          return 1;
        }

      printf("btctl: pairing requested peer=%s\n", argv[2]);
      return 0;
    }

  if (!strcmp(argv[1], "l2cap-connect") && argc >= 4)
    {
      int ret = linux_bt_l2cap_connect(btctl_peer(argv[2]),
                                       (uint16_t)strtoul(argv[3],
                                                         NULL, 0));

      if (ret < 0)
        {
          printf("btctl: l2cap-connect failed: %d\n", ret);
          return 1;
        }

      printf("btctl: l2cap connect requested peer=%s psm=%s\n",
             argv[2], argv[3]);
      return 0;
    }

  if (!strcmp(argv[1], "l2cap-disconnect") && argc >= 4)
    {
      int ret = linux_bt_l2cap_disconnect(btctl_peer(argv[2]),
                                          (uint16_t)strtoul(argv[3],
                                                            NULL, 0));

      if (ret < 0)
        {
          printf("btctl: l2cap-disconnect failed: %d\n", ret);
          return 1;
        }

      printf("btctl: l2cap disconnect requested peer=%s cid=%s\n",
             argv[2], argv[3]);
      return 0;
    }

  if (!strcmp(argv[1], "l2cap-send") && argc >= 4)
    {
      int ret = linux_bt_l2cap_send(btctl_peer(argv[2]), argv[3]);

      if (ret < 0)
        {
          printf("btctl: l2cap-send failed: %d\n", ret);
          return 1;
        }

      printf("btctl: l2cap payload sent peer=%s\n", argv[2]);
      return 0;
    }

  if (!strcmp(argv[1], "l2cap-echo") && argc >= 4)
    {
      int ret = linux_bt_l2cap_echo(btctl_peer(argv[2]), argv[3]);

      if (ret < 0)
        {
          printf("btctl: l2cap-echo failed: %d\n", ret);
          return 1;
        }

      printf("btctl: l2cap echo requested peer=%s\n", argv[2]);
      return 0;
    }

  if (!strcmp(argv[1], "gatt-read") && argc >= 3)
    {
      int ret;

      if (argc >= 4)
        {
          ret = linux_bt_gatt_read_peer(btctl_peer(argv[2]),
                                        (uint16_t)strtoul(argv[3],
                                                          NULL, 0));
        }
      else
        {
          ret = linux_bt_gatt_read((uint16_t)strtoul(argv[2], NULL, 0));
        }

      if (ret < 0)
        {
          printf("btctl: gatt-read failed: %d\n", ret);
          return 1;
        }

      printf("btctl: gatt read requested\n");
      return 0;
    }

  if (!strcmp(argv[1], "gatt-write") && argc >= 4)
    {
      int ret;

      if (argc >= 5)
        {
          ret = linux_bt_gatt_write_peer(btctl_peer(argv[2]),
                                         (uint16_t)strtoul(argv[3],
                                                           NULL, 0),
                                         argv[4]);
        }
      else
        {
          ret = linux_bt_gatt_write((uint16_t)strtoul(argv[2], NULL, 0),
                                    argv[3]);
        }

      if (ret < 0)
        {
          printf("btctl: gatt-write failed: %d\n", ret);
          return 1;
        }

      printf("btctl: gatt write requested\n");
      return 0;
    }

  printf("btctl: unknown command: %s\\n", argv[1]);
  btctl_usage();
  return 2;
}
