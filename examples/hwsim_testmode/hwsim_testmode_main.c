/****************************************************************************
 * apps/examples/hwsim_testmode/hwsim_testmode_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <inttypes.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/nl80211.h>

#include <netlink/attr.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

#ifndef CONFIG_EXAMPLES_HWSIM_TESTMODE_IFNAME
#  define CONFIG_EXAMPLES_HWSIM_TESTMODE_IFNAME "wlan0"
#endif

enum hwsim_testmode_attr
{
  HWSIM_TM_ATTR_CMD = 1,
  HWSIM_TM_ATTR_PS = 2,
  HWSIM_TM_ATTR_TWT_SP = 3,
  HWSIM_TM_ATTR_MAX = HWSIM_TM_ATTR_TWT_SP,
};

enum hwsim_testmode_cmd
{
  HWSIM_TM_CMD_SET_PS = 0,
  HWSIM_TM_CMD_GET_PS = 1,
  HWSIM_TM_CMD_STOP_QUEUES = 2,
  HWSIM_TM_CMD_WAKE_QUEUES = 3,
  HWSIM_TM_CMD_SET_TWT_SP = 4,
  HWSIM_TM_CMD_GET_TWT_SP = 5,
};

enum hwsim_ps_mode
{
  HWSIM_PS_DISABLED = 0,
  HWSIM_PS_ENABLED = 1,
  HWSIM_PS_AUTO_POLL = 2,
  HWSIM_PS_MANUAL_POLL = 3,
  HWSIM_PS_DYNAMIC_IDLE = 4,
};

struct hwsim_tm_result
{
  int err;
  int saw_valid;
  int saw_ps;
  int saw_twt_sp;
  uint32_t ps;
  uint32_t twt_sp;
};

static void usage(FAR const char *progname)
{
  printf("Usage:\n");
  printf("  %s [-i ifname] get_ps\n", progname);
  printf("  %s [-i ifname] set_ps <0|1|2|4>\n", progname);
  printf("  %s [-i ifname] poll\n", progname);
  printf("  %s [-i ifname] stop_queues\n", progname);
  printf("  %s [-i ifname] wake_queues\n", progname);
  printf("  %s [-i ifname] get_twt_sp\n", progname);
  printf("  %s [-i ifname] set_twt_sp <0|1>\n", progname);
}

static int parse_u32(FAR const char *text, FAR uint32_t *value)
{
  FAR char *endp;
  unsigned long tmp;

  errno = 0;
  tmp = strtoul(text, &endp, 0);
  if (errno != 0 || endp == text || *endp != '\0' || tmp > UINT32_MAX)
    {
      return -EINVAL;
    }

  *value = (uint32_t)tmp;
  return 0;
}

static int error_handler(FAR struct sockaddr_nl *nla,
                         FAR struct nlmsgerr *err,
                         FAR void *arg)
{
  FAR struct hwsim_tm_result *result = arg;

  (void)nla;
  result->err = err->error;
  return NL_STOP;
}

static int finish_handler(FAR struct nl_msg *msg, FAR void *arg)
{
  FAR int *ret = arg;

  (void)msg;
  *ret = 0;
  return NL_SKIP;
}

static int ack_handler(FAR struct nl_msg *msg, FAR void *arg)
{
  FAR int *ret = arg;

  (void)msg;
  *ret = 0;
  return NL_STOP;
}

static int valid_handler(FAR struct nl_msg *msg, FAR void *arg)
{
  FAR struct hwsim_tm_result *result = arg;
  FAR struct genlmsghdr *gnlh;
  FAR struct nlattr *testdata;
  FAR struct nlattr *tb[HWSIM_TM_ATTR_MAX + 1];
  FAR struct nlmsghdr *nlh;
  int ret;

  nlh = nlmsg_hdr(msg);
  gnlh = genlmsg_hdr(nlh);
  result->saw_valid = 1;

  testdata = nla_find(genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0),
                      NL80211_ATTR_TESTDATA);
  if (testdata == NULL)
    {
      return NL_SKIP;
    }

  memset(tb, 0, sizeof(tb));
  ret = nla_parse_nested(tb, HWSIM_TM_ATTR_MAX, testdata, NULL);
  if (ret < 0)
    {
      result->err = ret;
      return NL_STOP;
    }

  if (tb[HWSIM_TM_ATTR_PS] != NULL)
    {
      result->ps = nla_get_u32(tb[HWSIM_TM_ATTR_PS]);
      result->saw_ps = 1;
      printf("hwsim_tm: ps=%" PRIu32 "\n", result->ps);
    }

  if (tb[HWSIM_TM_ATTR_TWT_SP] != NULL)
    {
      result->twt_sp = nla_get_u32(tb[HWSIM_TM_ATTR_TWT_SP]);
      result->saw_twt_sp = 1;
      printf("hwsim_tm: twt_sp=%" PRIu32 "\n", result->twt_sp);
    }

  return NL_SKIP;
}

static int recv_until_done(FAR struct nl_sock *sock,
                           FAR struct hwsim_tm_result *result)
{
  FAR struct nl_cb *cb;
  int ret;

  cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (cb == NULL)
    {
      return -ENOMEM;
    }

  result->err = 1;
  nl_cb_err(cb, NL_CB_CUSTOM, error_handler, result);
  nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &result->err);
  nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &result->err);
  nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, result);

  while (result->err > 0)
    {
      ret = nl_recvmsgs(sock, cb);
      if (ret < 0)
        {
          result->err = ret;
          break;
        }
    }

  nl_cb_put(cb);
  return result->err;
}

static int send_testmode(FAR const char *ifname, uint32_t cmd,
                         int has_ps, uint32_t ps,
                         int has_twt_sp, uint32_t twt_sp,
                         FAR struct hwsim_tm_result *result)
{
  FAR struct nl_sock *sock;
  FAR struct nl_msg *msg;
  FAR struct nlattr *testdata;
  unsigned int ifindex;
  int family;
  int ret;

  memset(result, 0, sizeof(*result));

  ifindex = if_nametoindex(ifname);
  if (ifindex == 0)
    {
      ret = -errno;
      fprintf(stderr, "hwsim_tm: if_nametoindex(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  sock = nl_socket_alloc();
  if (sock == NULL)
    {
      return -ENOMEM;
    }

  ret = genl_connect(sock);
  if (ret < 0)
    {
      fprintf(stderr, "hwsim_tm: genl_connect failed: %d\n", ret);
      goto out_socket;
    }

  family = genl_ctrl_resolve(sock, "nl80211");
  if (family < 0)
    {
      fprintf(stderr, "hwsim_tm: cannot resolve nl80211: %d\n", family);
      ret = family;
      goto out_socket;
    }

  msg = nlmsg_alloc();
  if (msg == NULL)
    {
      ret = -ENOMEM;
      goto out_socket;
    }

  if (genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family, 0, 0,
                  NL80211_CMD_TESTMODE, 0) == NULL ||
      nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex) < 0)
    {
      ret = -ENOBUFS;
      goto out_msg;
    }

  testdata = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
  if (testdata == NULL ||
      nla_put_u32(msg, HWSIM_TM_ATTR_CMD, cmd) < 0 ||
      (has_ps && nla_put_u32(msg, HWSIM_TM_ATTR_PS, ps) < 0) ||
      (has_twt_sp &&
       nla_put_u32(msg, HWSIM_TM_ATTR_TWT_SP, twt_sp) < 0))
    {
      ret = -ENOBUFS;
      goto out_msg;
    }

  nla_nest_end(msg, testdata);

  ret = nl_send_auto_complete(sock, msg);
  if (ret < 0)
    {
      fprintf(stderr, "hwsim_tm: send failed: %d\n", ret);
      goto out_msg;
    }

  ret = recv_until_done(sock, result);
  if (ret < 0)
    {
      fprintf(stderr, "hwsim_tm: command failed: %d\n", ret);
    }

out_msg:
  nlmsg_free(msg);
out_socket:
  nl_socket_free(sock);
  return ret;
}

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_EXAMPLES_HWSIM_TESTMODE_IFNAME;
  FAR const char *command;
  struct hwsim_tm_result result;
  uint32_t cmd;
  uint32_t ps = 0;
  uint32_t twt_sp = 0;
  int has_ps = 0;
  int has_twt_sp = 0;
  int argi = 1;
  int ret;

  if (argi + 1 < argc && strcmp(argv[argi], "-i") == 0)
    {
      ifname = argv[argi + 1];
      argi += 2;
    }

  if (argi >= argc)
    {
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  command = argv[argi++];

  if (strcmp(command, "get_ps") == 0)
    {
      cmd = HWSIM_TM_CMD_GET_PS;
    }
  else if (strcmp(command, "set_ps") == 0)
    {
      if (argi >= argc || parse_u32(argv[argi], &ps) < 0 ||
          (ps > HWSIM_PS_AUTO_POLL && ps != HWSIM_PS_DYNAMIC_IDLE))
        {
          usage(argv[0]);
          return EXIT_FAILURE;
        }

      cmd = HWSIM_TM_CMD_SET_PS;
      has_ps = 1;
    }
  else if (strcmp(command, "poll") == 0)
    {
      cmd = HWSIM_TM_CMD_SET_PS;
      ps = HWSIM_PS_MANUAL_POLL;
      has_ps = 1;
    }
  else if (strcmp(command, "stop_queues") == 0)
    {
      cmd = HWSIM_TM_CMD_STOP_QUEUES;
    }
  else if (strcmp(command, "wake_queues") == 0)
    {
      cmd = HWSIM_TM_CMD_WAKE_QUEUES;
    }
  else if (strcmp(command, "get_twt_sp") == 0)
    {
      cmd = HWSIM_TM_CMD_GET_TWT_SP;
    }
  else if (strcmp(command, "set_twt_sp") == 0)
    {
      if (argi >= argc || parse_u32(argv[argi], &twt_sp) < 0 ||
          twt_sp > 1)
        {
          usage(argv[0]);
          return EXIT_FAILURE;
        }

      cmd = HWSIM_TM_CMD_SET_TWT_SP;
      has_twt_sp = 1;
    }
  else
    {
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  ret = send_testmode(ifname, cmd, has_ps, ps, has_twt_sp, twt_sp,
                      &result);
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  if (cmd == HWSIM_TM_CMD_GET_PS && !result.saw_ps)
    {
      fprintf(stderr, "hwsim_tm: no PS value in testmode reply\n");
      return EXIT_FAILURE;
    }

  if (cmd == HWSIM_TM_CMD_GET_TWT_SP && !result.saw_twt_sp)
    {
      fprintf(stderr, "hwsim_tm: no TWT SP value in testmode reply\n");
      return EXIT_FAILURE;
    }

  printf("hwsim_tm: OK\n");
  return EXIT_SUCCESS;
}
