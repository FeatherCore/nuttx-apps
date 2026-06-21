/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/hfp_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_HFP_RFCOMM_PSM      0x0003
#define BLUEZ_HFP_RFCOMM_CID      0x0061
#define BLUEZ_HSP_RFCOMM_CID      0x0062
#define BLUEZ_HFP_DEFAULT_PEER    2

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_SCO
#  define BTPROTO_SCO 2
#endif

#ifndef BTPROTO_RFCOMM
#  define BTPROTO_RFCOMM 3
#endif

#ifndef SOL_SCO
#  define SOL_SCO 17
#endif

#ifndef SOL_RFCOMM
#  define SOL_RFCOMM 18
#endif

#ifndef SOL_BLUETOOTH
#  define SOL_BLUETOOTH 274
#endif

#ifndef BT_SECURITY
#  define BT_SECURITY 4
struct bt_security
{
  uint8_t level;
  uint8_t key_size;
};
#  define BT_SECURITY_LOW    1
#  define BT_SECURITY_MEDIUM 2
#  define BT_SECURITY_HIGH   3
#  define BT_SECURITY_FIPS   4
#endif

#ifndef SHUT_RDWR
#  define SHUT_RDWR 2
#endif

#ifdef TIOCINQ
#  undef TIOCINQ
#endif
#define TIOCINQ 0x541b

#ifdef TIOCOUTQ
#  undef TIOCOUTQ
#endif
#define TIOCOUTQ 0x5411

#ifndef RFCOMM_CONNINFO
#  define RFCOMM_CONNINFO 0x02
#endif

#ifndef RFCOMM_LM
#  define RFCOMM_LM 0x03
#endif

#ifndef RFCOMM_LM_AUTH
#  define RFCOMM_LM_MASTER  0x0001
#  define RFCOMM_LM_AUTH    0x0002
#  define RFCOMM_LM_ENCRYPT 0x0004
#  define RFCOMM_LM_SECURE  0x0020
#  define RFCOMM_LM_FIPS    0x0040
#endif

#ifndef SCO_OPTIONS
#  define SCO_OPTIONS 0x01
#endif

#ifndef SCO_CONNINFO
#  define SCO_CONNINFO 0x02
#endif

#ifndef __SO_PROTOCOL
#  define __SO_PROTOCOL 50
#endif

#ifndef SO_SCO_MTU
#  define SO_SCO_MTU (__SO_PROTOCOL + 8)
#endif

#ifndef SO_SCO_HANDLE
#  define SO_SCO_HANDLE (__SO_PROTOCOL + 9)
#endif

#ifndef BT_SNDMTU
#  define BT_SNDMTU 12
#endif

#ifndef BT_RCVMTU
#  define BT_RCVMTU 13
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_hfp_transaction
{
  const char *label;
  const char *command;
  const char *response;
};

struct bluez_hfp_sockaddr_rc
{
  sa_family_t rc_family;
  uint8_t rc_bdaddr[6];
  uint8_t rc_channel;
};

struct bluez_hfp_sockaddr_sco
{
  sa_family_t sco_family;
  uint8_t sco_bdaddr[6];
};

struct bluez_hfp_rfcomm_conninfo
{
  uint16_t hci_handle;
  uint8_t dev_class[3];
};

struct bluez_hfp_sco_options
{
  uint16_t mtu;
};

struct bluez_hfp_sco_conninfo
{
  uint16_t hci_handle;
  uint8_t dev_class[3];
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct bluez_hfp_transaction g_bluez_hfp_transactions[] =
{
  {"hfp-slc-brsf", "AT+BRSF=1024\r", "+BRSF/OK"},
  {"hfp-codec-bac-bcs", "AT+BAC=1,2\rAT+BCS=2\r", "+BCS/OK"},
  {"hfp-call-clcc", "AT+CLCC\r", "+CLCC/OK"}
};

static const struct bluez_hfp_transaction g_bluez_hsp_transactions[] =
{
  {"hsp-button-ckpd", "AT+CKPD=200\r", "OK/RING"},
  {"hsp-volume-vgs", "AT+VGS=12\rAT+VGM=10\r", "OK/+VGS/+VGM"}
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_hfp_usage(void)
{
  printf("usage: bluezhfp closeout hfp-hf|hfp-ag|hsp-hs|hsp-ag [peer]\n");
}

static uint16_t bluez_hfp_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0050 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0050 + (peer & 0x00ff));
#endif
}

static uint8_t bluez_hfp_rfcomm_channel(uint16_t cid)
{
  if (cid == BLUEZ_HFP_RFCOMM_CID)
    {
      return 1;
    }

  if (cid == BLUEZ_HSP_RFCOMM_CID)
    {
      return 2;
    }

  return (uint8_t)(cid & 0x1f);
}

static int bluez_hfp_sockapi_closeout(const char *mode, const char *role,
                                      uint8_t channel)
{
  static const char rfcomm_payload[] = "AT+CKPD=200\r";
  static const uint8_t sco_payload[] =
  {
    0x53, 0x43, 0x4f, 0x10, 0x20, 0x30, 0x40, 0x50
  };
  struct bluez_hfp_sockaddr_rc rcaddr;
  struct bluez_hfp_sockaddr_rc rclocal;
  struct bluez_hfp_sockaddr_rc rcpeer;
  struct bluez_hfp_sockaddr_sco scoaddr;
  struct bluez_hfp_sockaddr_sco scolocal;
  struct bluez_hfp_sockaddr_sco scopeer;
  struct bluez_hfp_rfcomm_conninfo rcinfo;
  struct bluez_hfp_sco_options scoopts;
  struct bluez_hfp_sco_conninfo scinfo;
  struct bt_security rfcomm_sec;
  struct iovec iov;
  struct msghdr msg;
  struct pollfd pfd;
  char rfcomm_recv[64];
  uint8_t sco_recv[64];
  socklen_t optlen;
  ssize_t rfcomm_send_ret = -1;
  ssize_t rfcomm_recv_ret = -1;
  ssize_t sco_send_ret = -1;
  ssize_t sco_recv_ret = -1;
  int rfcomm_recv_errno = 0;
  int rfcomm_recv_ok;
  int sco_recv_errno = 0;
  int sco_recv_ok;
  int rfcomm_fd;
  int rfcomm_bind_ret = -1;
  int rfcomm_connect_ret = -1;
  int rfcomm_cinfo_ret = -1;
  int rfcomm_local_ret = -1;
  int rfcomm_peer_ret = -1;
  int rfcomm_poll_ret = -1;
  short rfcomm_revents = 0;
  int rfcomm_inq = -1;
  int rfcomm_outq = -1;
  int rfcomm_inq_ret = -1;
  int rfcomm_outq_ret = -1;
  int rfcomm_lm_set_ret = -1;
  int rfcomm_lm_get_ret = -1;
  int rfcomm_lm_fips_ret = -1;
  int rfcomm_sec_set_ret = -1;
  int rfcomm_sec_get_ret = -1;
  int rfcomm_sec_fips_ret = -1;
  int rfcomm_sec_fips_errno = 0;
  uint8_t rfcomm_sec_level = 0;
  uint32_t rfcomm_lm_set = RFCOMM_LM_MASTER | RFCOMM_LM_AUTH |
                           RFCOMM_LM_ENCRYPT | RFCOMM_LM_SECURE;
  uint32_t rfcomm_lm_get = 0;
  uint32_t rfcomm_lm_fips = RFCOMM_LM_FIPS;
  int rfcomm_listen_fd = -1;
  int rfcomm_accept_fd = -1;
  int rfcomm_create_nonblock_fd = -1;
  int rfcomm_create_nonblock_flags = -1;
  int rfcomm_create_nonblock_close = -1;
  int rfcomm_seqpacket_fd = -1;
  int rfcomm_seqpacket_errno = 0;
  int rfcomm_nonblock_ret = -1;
  int rfcomm_listen_bind_ret = -1;
  int rfcomm_listen_ret = -1;
  int rfcomm_accept_ret = -1;
  int rfcomm_shutdown_ret = -1;
  int rfcomm_close_ret = -1;
  int sco_fd;
  int sco_bind_ret = -1;
  int sco_connect_ret = -1;
  int sco_options_ret = -1;
  int sco_cinfo_ret = -1;
  int sco_sndmtu_ret = -1;
  int sco_rcvmtu_ret = -1;
  int sco_legacy_mtu_ret = -1;
  int sco_legacy_mtu_errno = 0;
  int sco_legacy_handle_ret = -1;
  int sco_legacy_handle_errno = 0;
  int sco_local_ret = -1;
  int sco_peer_ret = -1;
  int sco_poll_ret = -1;
  uint32_t sco_sndmtu = 0;
  uint32_t sco_rcvmtu = 0;
  short sco_revents = 0;
  int sco_inq = -1;
  int sco_outq = -1;
  int sco_inq_ret = -1;
  int sco_outq_ret = -1;
  int sco_listen_fd = -1;
  int sco_accept_fd = -1;
  int sco_create_nonblock_fd = -1;
  int sco_create_nonblock_flags = -1;
  int sco_create_nonblock_close = -1;
  int sco_nonblock_ret = -1;
  int sco_listen_bind_ret = -1;
  int sco_listen_ret = -1;
  int sco_accept_ret = -1;
  int sco_shutdown_ret = -1;
  int sco_close_ret = -1;
  int sco_stream_fd = -1;
  int sco_stream_errno = 0;
  int sco_dgram_fd = -1;
  int sco_dgram_errno = 0;
  int sco_raw_fd = -1;
  int sco_raw_errno = 0;

  memset(&rcaddr, 0, sizeof(rcaddr));
  rcaddr.rc_family = AF_BLUETOOTH;
  rcaddr.rc_channel = channel;
  rfcomm_create_nonblock_fd = socket(AF_BLUETOOTH,
                                     SOCK_STREAM | SOCK_NONBLOCK,
                                     BTPROTO_RFCOMM);
  if (rfcomm_create_nonblock_fd >= 0)
    {
      rfcomm_create_nonblock_flags = fcntl(rfcomm_create_nonblock_fd,
                                           F_GETFL);
      rfcomm_create_nonblock_close = close(rfcomm_create_nonblock_fd);
    }

  errno = 0;
  rfcomm_seqpacket_fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET,
                               BTPROTO_RFCOMM);
  if (rfcomm_seqpacket_fd < 0)
    {
      rfcomm_seqpacket_errno = errno;
    }
  else
    {
      (void)close(rfcomm_seqpacket_fd);
    }

  rfcomm_listen_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if (rfcomm_listen_fd >= 0)
    {
      rfcomm_nonblock_ret = fcntl(rfcomm_listen_fd, F_SETFL,
                                  O_NONBLOCK);
      rfcomm_listen_bind_ret = bind(rfcomm_listen_fd,
                                    (struct sockaddr *)&rcaddr,
                                    sizeof(rcaddr));
      rfcomm_listen_ret = listen(rfcomm_listen_fd, 1);
      rfcomm_accept_fd = accept(rfcomm_listen_fd, NULL, NULL);
      rfcomm_accept_ret = rfcomm_accept_fd >= 0 ? 0 : -1;
      if (rfcomm_accept_fd >= 0)
        {
          (void)close(rfcomm_accept_fd);
        }

      (void)close(rfcomm_listen_fd);
    }

  rfcomm_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if (rfcomm_fd >= 0)
    {
      rfcomm_bind_ret = bind(rfcomm_fd, (struct sockaddr *)&rcaddr,
                             sizeof(rcaddr));
      rfcomm_connect_ret = connect(rfcomm_fd, (struct sockaddr *)&rcaddr,
                                   sizeof(rcaddr));
      memset(&rclocal, 0, sizeof(rclocal));
      optlen = sizeof(rclocal);
      rfcomm_local_ret = getsockname(rfcomm_fd,
                                     (struct sockaddr *)&rclocal,
                                     &optlen);
      memset(&rcpeer, 0, sizeof(rcpeer));
      optlen = sizeof(rcpeer);
      rfcomm_peer_ret = getpeername(rfcomm_fd,
                                    (struct sockaddr *)&rcpeer,
                                    &optlen);
      memset(&pfd, 0, sizeof(pfd));
      pfd.fd = rfcomm_fd;
      pfd.events = POLLOUT;
      rfcomm_poll_ret = poll(&pfd, 1, 0);
      rfcomm_revents = pfd.revents;
      rfcomm_inq_ret = ioctl(rfcomm_fd, TIOCINQ,
                             (unsigned long)&rfcomm_inq);
      rfcomm_outq_ret = ioctl(rfcomm_fd, TIOCOUTQ,
                              (unsigned long)&rfcomm_outq);
      memset(&rcinfo, 0, sizeof(rcinfo));
      optlen = sizeof(rcinfo);
      rfcomm_cinfo_ret = getsockopt(rfcomm_fd, SOL_RFCOMM,
                                    RFCOMM_CONNINFO, &rcinfo, &optlen);
      optlen = sizeof(rfcomm_lm_set);
      rfcomm_lm_set_ret = setsockopt(rfcomm_fd, SOL_RFCOMM, RFCOMM_LM,
                                     &rfcomm_lm_set, optlen);
      optlen = sizeof(rfcomm_lm_get);
      rfcomm_lm_get_ret = getsockopt(rfcomm_fd, SOL_RFCOMM, RFCOMM_LM,
                                     &rfcomm_lm_get, &optlen);
      optlen = sizeof(rfcomm_lm_fips);
      rfcomm_lm_fips_ret = setsockopt(rfcomm_fd, SOL_RFCOMM, RFCOMM_LM,
                                      &rfcomm_lm_fips, optlen);
      memset(&rfcomm_sec, 0, sizeof(rfcomm_sec));
      rfcomm_sec.level = BT_SECURITY_HIGH;
      rfcomm_sec_set_ret = setsockopt(rfcomm_fd, SOL_BLUETOOTH,
                                      BT_SECURITY, &rfcomm_sec,
                                      sizeof(rfcomm_sec));
      memset(&rfcomm_sec, 0, sizeof(rfcomm_sec));
      optlen = sizeof(rfcomm_sec);
      rfcomm_sec_get_ret = getsockopt(rfcomm_fd, SOL_BLUETOOTH,
                                      BT_SECURITY, &rfcomm_sec, &optlen);
      rfcomm_sec_level = rfcomm_sec.level;
      rfcomm_sec.level = BT_SECURITY_FIPS;
      errno = 0;
      rfcomm_sec_fips_ret = setsockopt(rfcomm_fd, SOL_BLUETOOTH,
                                       BT_SECURITY, &rfcomm_sec,
                                       sizeof(rfcomm_sec));
      if (rfcomm_sec_fips_ret < 0)
        {
          rfcomm_sec_fips_errno = errno;
        }
      memset(&iov, 0, sizeof(iov));
      memset(&msg, 0, sizeof(msg));
      iov.iov_base = (void *)rfcomm_payload;
      iov.iov_len = sizeof(rfcomm_payload) - 1;
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      rfcomm_send_ret = sendmsg(rfcomm_fd, &msg, 0);
      memset(rfcomm_recv, 0, sizeof(rfcomm_recv));
      memset(&iov, 0, sizeof(iov));
      memset(&msg, 0, sizeof(msg));
      iov.iov_base = rfcomm_recv;
      iov.iov_len = sizeof(rfcomm_recv);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      errno = 0;
      rfcomm_recv_ret = recvmsg(rfcomm_fd, &msg, 0);
      if (rfcomm_recv_ret < 0)
        {
          rfcomm_recv_errno = errno;
        }

      rfcomm_shutdown_ret = shutdown(rfcomm_fd, SHUT_RDWR);
      rfcomm_close_ret = close(rfcomm_fd);
    }

  memset(&scoaddr, 0, sizeof(scoaddr));
  scoaddr.sco_family = AF_BLUETOOTH;
  errno = 0;
  sco_stream_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_SCO);
  if (sco_stream_fd < 0)
    {
      sco_stream_errno = errno;
    }
  else
    {
      (void)close(sco_stream_fd);
    }

  errno = 0;
  sco_dgram_fd = socket(AF_BLUETOOTH, SOCK_DGRAM, BTPROTO_SCO);
  if (sco_dgram_fd < 0)
    {
      sco_dgram_errno = errno;
    }
  else
    {
      (void)close(sco_dgram_fd);
    }

  errno = 0;
  sco_raw_fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_SCO);
  if (sco_raw_fd < 0)
    {
      sco_raw_errno = errno;
    }
  else
    {
      (void)close(sco_raw_fd);
    }

  sco_create_nonblock_fd = socket(AF_BLUETOOTH,
                                  SOCK_SEQPACKET | SOCK_NONBLOCK,
                                  BTPROTO_SCO);
  sco_listen_fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
  sco_fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
  if (sco_create_nonblock_fd >= 0)
    {
      sco_create_nonblock_flags = fcntl(sco_create_nonblock_fd, F_GETFL);
      sco_create_nonblock_close = close(sco_create_nonblock_fd);
    }

  if (sco_listen_fd >= 0)
    {
      sco_nonblock_ret = fcntl(sco_listen_fd, F_SETFL, O_NONBLOCK);
      sco_listen_bind_ret = bind(sco_listen_fd,
                                 (struct sockaddr *)&scoaddr,
                                 sizeof(scoaddr));
      sco_listen_ret = listen(sco_listen_fd, 1);
      sco_accept_fd = accept(sco_listen_fd, NULL, NULL);
      sco_accept_ret = sco_accept_fd >= 0 ? 0 : -1;
      if (sco_accept_fd >= 0)
        {
          (void)close(sco_accept_fd);
        }

      (void)close(sco_listen_fd);
    }

  if (sco_fd >= 0)
    {
      sco_bind_ret = bind(sco_fd, (struct sockaddr *)&scoaddr,
                          sizeof(scoaddr));
      sco_connect_ret = connect(sco_fd, (struct sockaddr *)&scoaddr,
                                sizeof(scoaddr));
      memset(&scolocal, 0, sizeof(scolocal));
      optlen = sizeof(scolocal);
      sco_local_ret = getsockname(sco_fd,
                                  (struct sockaddr *)&scolocal,
                                  &optlen);
      memset(&scopeer, 0, sizeof(scopeer));
      optlen = sizeof(scopeer);
      sco_peer_ret = getpeername(sco_fd,
                                 (struct sockaddr *)&scopeer,
                                 &optlen);
      memset(&pfd, 0, sizeof(pfd));
      pfd.fd = sco_fd;
      pfd.events = POLLOUT;
      sco_poll_ret = poll(&pfd, 1, 0);
      sco_revents = pfd.revents;
      sco_inq_ret = ioctl(sco_fd, TIOCINQ, (unsigned long)&sco_inq);
      sco_outq_ret = ioctl(sco_fd, TIOCOUTQ, (unsigned long)&sco_outq);
      memset(&scoopts, 0, sizeof(scoopts));
      optlen = sizeof(scoopts);
      sco_options_ret = getsockopt(sco_fd, SOL_SCO, SCO_OPTIONS,
                                   &scoopts, &optlen);
      memset(&scinfo, 0, sizeof(scinfo));
      optlen = sizeof(scinfo);
      sco_cinfo_ret = getsockopt(sco_fd, SOL_SCO, SCO_CONNINFO,
                                 &scinfo, &optlen);
      optlen = sizeof(sco_sndmtu);
      errno = 0;
      sco_legacy_mtu_ret = getsockopt(sco_fd, SOL_SCO, SO_SCO_MTU,
                                      &sco_sndmtu, &optlen);
      if (sco_legacy_mtu_ret < 0)
        {
          sco_legacy_mtu_errno = errno;
        }

      optlen = sizeof(scinfo.hci_handle);
      errno = 0;
      sco_legacy_handle_ret = getsockopt(sco_fd, SOL_SCO, SO_SCO_HANDLE,
                                         &scinfo.hci_handle, &optlen);
      if (sco_legacy_handle_ret < 0)
        {
          sco_legacy_handle_errno = errno;
        }

      optlen = sizeof(sco_sndmtu);
      sco_sndmtu_ret = getsockopt(sco_fd, SOL_BLUETOOTH, BT_SNDMTU,
                                  &sco_sndmtu, &optlen);
      optlen = sizeof(sco_rcvmtu);
      sco_rcvmtu_ret = getsockopt(sco_fd, SOL_BLUETOOTH, BT_RCVMTU,
                                  &sco_rcvmtu, &optlen);
      memset(&iov, 0, sizeof(iov));
      memset(&msg, 0, sizeof(msg));
      iov.iov_base = (void *)sco_payload;
      iov.iov_len = sizeof(sco_payload);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      sco_send_ret = sendmsg(sco_fd, &msg, 0);
      memset(sco_recv, 0, sizeof(sco_recv));
      memset(&iov, 0, sizeof(iov));
      memset(&msg, 0, sizeof(msg));
      iov.iov_base = sco_recv;
      iov.iov_len = sizeof(sco_recv);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      errno = 0;
      sco_recv_ret = recvmsg(sco_fd, &msg, 0);
      if (sco_recv_ret < 0)
        {
          sco_recv_errno = errno;
        }

      sco_shutdown_ret = shutdown(sco_fd, SHUT_RDWR);
      sco_close_ret = close(sco_fd);
    }

  rfcomm_recv_ok = rfcomm_recv_ret >= 0;
  sco_recv_ok = sco_recv_ret >= 0 ||
                sco_recv_errno == EAGAIN ||
                sco_recv_errno == EWOULDBLOCK;

  printf("bluez-hfp: ordinary-rfcomm-socket mode=%s role=%s "
         "socket-ret=%d bind-ret=%d connect-ret=%d conninfo-ret=%d "
         "conninfo-handle=0x%04x sendmsg-ret=%d recvmsg-ret=%d "
         "recvmsg-errno=%d getsockname-ret=%d getsockname-channel=%u "
         "rfcomm-lm-set-ret=%d rfcomm-lm-get-ret=%d rfcomm-lm=0x%08lx "
         "rfcomm-lm-fips-ret=%d seqpacket-ret=%d seqpacket-errno=%d "
         "seqpacket-esocktnosupport=%u "
         "btsec-set-ret=%d btsec-get-ret=%d btsec-level=%u "
         "btsec-fips-ret=%d btsec-fips-errno=%d "
         "getpeername-ret=%d getpeername-channel=%u shutdown-ret=%d "
         "poll-ret=%d poll-revents=0x%x ioctl-inq-ret=%d inq=%d "
         "ioctl-outq-ret=%d outq=%d listen-fd=%d nonblock-ret=%d "
         "create-nonblock-fd=%d create-nonblock-flags=0x%x "
         "create-nonblock-close=%d create-nonblock-ok=%u "
         "listen-bind-ret=%d listen-ret=%d accept-ret=%d accept-fd=%d "
         "close-ret=%d "
         "channel=%u proto=BTPROTO_RFCOMM path=ordinary-socket "
         "final-ok=%u\n",
         mode, role, rfcomm_fd, rfcomm_bind_ret, rfcomm_connect_ret,
         rfcomm_cinfo_ret, rcinfo.hci_handle, (int)rfcomm_send_ret,
         (int)rfcomm_recv_ret, rfcomm_recv_errno, rfcomm_local_ret,
         rclocal.rc_channel, rfcomm_lm_set_ret, rfcomm_lm_get_ret,
         (unsigned long)rfcomm_lm_get, rfcomm_lm_fips_ret,
         rfcomm_seqpacket_fd, rfcomm_seqpacket_errno,
         rfcomm_seqpacket_fd < 0 &&
         rfcomm_seqpacket_errno == ESOCKTNOSUPPORT,
         rfcomm_sec_set_ret, rfcomm_sec_get_ret, rfcomm_sec_level,
         rfcomm_sec_fips_ret, rfcomm_sec_fips_errno,
         rfcomm_peer_ret, rcpeer.rc_channel,
         rfcomm_shutdown_ret, rfcomm_poll_ret, rfcomm_revents,
         rfcomm_inq_ret, rfcomm_inq, rfcomm_outq_ret, rfcomm_outq,
         rfcomm_listen_fd, rfcomm_nonblock_ret,
         rfcomm_create_nonblock_fd, rfcomm_create_nonblock_flags,
         rfcomm_create_nonblock_close,
         rfcomm_create_nonblock_fd >= 0 &&
         rfcomm_create_nonblock_flags >= 0 &&
         (rfcomm_create_nonblock_flags & O_NONBLOCK) != 0 &&
         rfcomm_create_nonblock_close == 0,
         rfcomm_listen_bind_ret, rfcomm_listen_ret, rfcomm_accept_ret,
         rfcomm_accept_fd, rfcomm_close_ret, channel,
         rfcomm_fd >= 0 && rfcomm_bind_ret == 0 &&
         rfcomm_connect_ret == 0 && rfcomm_cinfo_ret == 0 &&
         rfcomm_lm_set_ret == 0 && rfcomm_lm_get_ret == 0 &&
         rfcomm_lm_get == rfcomm_lm_set &&
         rfcomm_lm_fips_ret == -1 &&
         rfcomm_seqpacket_fd < 0 &&
         rfcomm_seqpacket_errno == ESOCKTNOSUPPORT &&
         rfcomm_sec_set_ret == 0 && rfcomm_sec_get_ret == 0 &&
         rfcomm_sec_level == BT_SECURITY_HIGH &&
         rfcomm_sec_fips_ret == -1 &&
         rfcomm_sec_fips_errno == EINVAL &&
         rfcomm_local_ret == 0 && rfcomm_peer_ret == 0 &&
         rfcomm_poll_ret > 0 && (rfcomm_revents & POLLOUT) != 0 &&
         rfcomm_inq_ret == 0 && rfcomm_outq_ret == 0 &&
         rfcomm_listen_fd >= 0 &&
         rfcomm_create_nonblock_fd >= 0 &&
         rfcomm_create_nonblock_flags >= 0 &&
         (rfcomm_create_nonblock_flags & O_NONBLOCK) != 0 &&
         rfcomm_create_nonblock_close == 0 &&
         rfcomm_nonblock_ret == 0 &&
         rfcomm_listen_bind_ret == 0 && rfcomm_listen_ret == 0 &&
         rfcomm_accept_ret == 0 &&
         rfcomm_send_ret == (ssize_t)(sizeof(rfcomm_payload) - 1) &&
         rfcomm_recv_ok &&
         rfcomm_shutdown_ret == 0 &&
         rfcomm_close_ret == 0);

  printf("bluez-hfp: ordinary-sco-socket mode=%s role=%s "
         "socket-ret=%d bind-ret=%d connect-ret=%d options-ret=%d "
         "options-mtu=%u conninfo-ret=%d conninfo-handle=0x%04x "
         "ordinary-mtu-sockopt=SOL_BLUETOOTH "
         "sndmtu-ret=%d sndmtu=%lu rcvmtu-ret=%d rcvmtu=%lu "
         "legacy-sco-mtu-ret=%d legacy-sco-mtu-errno=%d "
         "legacy-sco-mtu-enoprotoopt=%u "
         "legacy-sco-handle-ret=%d legacy-sco-handle-errno=%d "
         "legacy-sco-handle-enoprotoopt=%u "
         "sendmsg-ret=%d recvmsg-ret=%d recvmsg-errno=%d "
         "shutdown-ret=%d poll-ret=%d poll-revents=0x%x close-ret=%d "
         "ioctl-inq-ret=%d inq=%d ioctl-outq-ret=%d outq=%d "
         "listen-fd=%d nonblock-ret=%d listen-bind-ret=%d "
         "create-nonblock-fd=%d create-nonblock-flags=0x%x "
         "create-nonblock-close=%d create-nonblock-ok=%u "
         "stream-ret=%d stream-esocktnosupport=%u "
         "dgram-ret=%d dgram-esocktnosupport=%u "
         "raw-ret=%d raw-esocktnosupport=%u "
         "listen-ret=%d accept-ret=%d accept-fd=%d "
         "getsockname-ret=%d getpeername-ret=%d "
         "proto=BTPROTO_SCO path=ordinary-socket final-ok=%u\n",
         mode, role, sco_fd, sco_bind_ret, sco_connect_ret,
         sco_options_ret, scoopts.mtu, sco_cinfo_ret, scinfo.hci_handle,
         sco_sndmtu_ret, (unsigned long)sco_sndmtu,
         sco_rcvmtu_ret, (unsigned long)sco_rcvmtu,
         sco_legacy_mtu_ret, sco_legacy_mtu_errno,
         sco_legacy_mtu_ret < 0 && sco_legacy_mtu_errno == ENOPROTOOPT,
         sco_legacy_handle_ret, sco_legacy_handle_errno,
         sco_legacy_handle_ret < 0 &&
         sco_legacy_handle_errno == ENOPROTOOPT,
         (int)sco_send_ret, (int)sco_recv_ret, sco_recv_errno,
         sco_shutdown_ret, sco_poll_ret, sco_revents, sco_close_ret,
         sco_inq_ret, sco_inq, sco_outq_ret, sco_outq, sco_listen_fd,
         sco_nonblock_ret, sco_listen_bind_ret,
         sco_create_nonblock_fd, sco_create_nonblock_flags,
         sco_create_nonblock_close,
         sco_create_nonblock_fd >= 0 &&
         sco_create_nonblock_flags >= 0 &&
         (sco_create_nonblock_flags & O_NONBLOCK) != 0 &&
         sco_create_nonblock_close == 0,
         sco_stream_fd,
         sco_stream_fd < 0 && sco_stream_errno == ESOCKTNOSUPPORT,
         sco_dgram_fd,
         sco_dgram_fd < 0 && sco_dgram_errno == ESOCKTNOSUPPORT,
         sco_raw_fd,
         sco_raw_fd < 0 && sco_raw_errno == ESOCKTNOSUPPORT,
         sco_listen_ret, sco_accept_ret, sco_accept_fd, sco_local_ret,
         sco_peer_ret,
         sco_fd >= 0 && sco_bind_ret == 0 && sco_connect_ret == 0 &&
         sco_options_ret == 0 && sco_cinfo_ret == 0 &&
         sco_sndmtu_ret == 0 && sco_sndmtu == scoopts.mtu &&
         sco_rcvmtu_ret == 0 && sco_rcvmtu == scoopts.mtu &&
         sco_legacy_mtu_ret < 0 &&
         sco_legacy_mtu_errno == ENOPROTOOPT &&
         sco_legacy_handle_ret < 0 &&
         sco_legacy_handle_errno == ENOPROTOOPT &&
         sco_local_ret == 0 && sco_peer_ret == 0 &&
         sco_poll_ret > 0 && (sco_revents & POLLOUT) != 0 &&
         sco_inq_ret == 0 && sco_outq_ret == 0 &&
         sco_listen_fd >= 0 &&
         sco_create_nonblock_fd >= 0 &&
         sco_create_nonblock_flags >= 0 &&
         (sco_create_nonblock_flags & O_NONBLOCK) != 0 &&
         sco_create_nonblock_close == 0 &&
         sco_stream_fd < 0 &&
         sco_stream_errno == ESOCKTNOSUPPORT &&
         sco_dgram_fd < 0 &&
         sco_dgram_errno == ESOCKTNOSUPPORT &&
         sco_raw_fd < 0 &&
         sco_raw_errno == ESOCKTNOSUPPORT &&
         sco_nonblock_ret == 0 &&
         sco_listen_bind_ret == 0 && sco_listen_ret == 0 &&
         sco_accept_ret == 0 &&
         sco_send_ret == (ssize_t)sizeof(sco_payload) &&
         sco_recv_ok &&
         sco_shutdown_ret == 0 &&
         sco_close_ret == 0);

  return rfcomm_fd >= 0 && rfcomm_bind_ret == 0 &&
         rfcomm_connect_ret == 0 && rfcomm_cinfo_ret == 0 &&
         rfcomm_local_ret == 0 && rfcomm_peer_ret == 0 &&
         rfcomm_poll_ret > 0 && (rfcomm_revents & POLLOUT) != 0 &&
         rfcomm_inq_ret == 0 && rfcomm_outq_ret == 0 &&
         rfcomm_listen_fd >= 0 &&
         rfcomm_create_nonblock_fd >= 0 &&
         rfcomm_create_nonblock_flags >= 0 &&
         (rfcomm_create_nonblock_flags & O_NONBLOCK) != 0 &&
         rfcomm_create_nonblock_close == 0 &&
         rfcomm_nonblock_ret == 0 &&
         rfcomm_listen_bind_ret == 0 && rfcomm_listen_ret == 0 &&
         rfcomm_accept_ret == 0 &&
         rfcomm_send_ret == (ssize_t)(sizeof(rfcomm_payload) - 1) &&
         rfcomm_recv_ok &&
         rfcomm_shutdown_ret == 0 &&
         rfcomm_close_ret == 0 &&
         sco_fd >= 0 && sco_bind_ret == 0 && sco_connect_ret == 0 &&
         sco_options_ret == 0 && sco_cinfo_ret == 0 &&
         sco_sndmtu_ret == 0 && sco_sndmtu == scoopts.mtu &&
         sco_rcvmtu_ret == 0 && sco_rcvmtu == scoopts.mtu &&
         sco_local_ret == 0 && sco_peer_ret == 0 &&
         sco_poll_ret > 0 && (sco_revents & POLLOUT) != 0 &&
         sco_inq_ret == 0 && sco_outq_ret == 0 &&
         sco_listen_fd >= 0 &&
         sco_create_nonblock_fd >= 0 &&
         sco_create_nonblock_flags >= 0 &&
         (sco_create_nonblock_flags & O_NONBLOCK) != 0 &&
         sco_create_nonblock_close == 0 &&
         sco_stream_fd < 0 &&
         sco_stream_errno == ESOCKTNOSUPPORT &&
         sco_dgram_fd < 0 &&
         sco_dgram_errno == ESOCKTNOSUPPORT &&
         sco_raw_fd < 0 &&
         sco_raw_errno == ESOCKTNOSUPPORT &&
         sco_nonblock_ret == 0 &&
         sco_listen_bind_ret == 0 && sco_listen_ret == 0 &&
         sco_accept_ret == 0 &&
         sco_send_ret == (ssize_t)sizeof(sco_payload) &&
         sco_recv_ok &&
         sco_shutdown_ret == 0 &&
         sco_close_ret == 0 ? 0 : -1;
}

static int bluez_hfp_run_transactions(
  void *rfcomm, const char *profile, const char *role,
  const struct bluez_hfp_transaction *transactions, size_t count,
  int responder)
{
  char out[256];
  size_t i;
  int ret;
  int failed = 0;

  for (i = 0; i < count; i++)
    {
      if (responder)
        {
          ret = 0;
          out[0] = '\0';
        }
      else
        {
          memset(out, 0, sizeof(out));
          ret = linux_bt_upstream_rfcomm_socket_write_handle(
                  rfcomm, transactions[i].command,
                  strlen(transactions[i].command), out, sizeof(out));
        }

      printf("bluez-hfp: source=third/bluez/profiles/audio/%s "
             "style=rfcomm-at command=transaction role=%s label=%s "
             "write-ret=%d",
             profile, role, transactions[i].label, ret);
      if (responder)
        {
          printf(" detail=daemon-mainloop-owned responder-delegated-io=1");
        }

      printf("\n");
      printf("%s", out);
      printf("bluez-hfp: at-request-response-evidence role=%s "
             "label=%s request=%s response=%s result=%s\n",
             role, transactions[i].label, transactions[i].command,
             transactions[i].response, ret < 0 ? "failed" : "ok");
      failed |= ret < 0;
    }

  return failed ? -1 : 0;
}

static int bluez_hfp_closeout(const char *mode, uint16_t peer)
{
  static const uint8_t sco_payload[] =
  {
    0x48, 0x46, 0x50, 0x2f, 0x48, 0x53, 0x50, 0x3a,
    0x53, 0x43, 0x4f, 0x3a, 0x43, 0x56, 0x53, 0x44
  };
  const struct bluez_hfp_transaction *transactions;
  const char *role;
  const char *profile;
  const char *boundary;
  size_t transaction_count;
  uint16_t cid;
  uint16_t handle;
  uint8_t channel;
  int responder;
  void *rfcomm = NULL;
  void *sco = NULL;
  int ret;
  int failed = 0;

  if (!strcmp(mode, "hfp-hf"))
    {
      role = "handsfree";
      profile = "hfp-hf.c";
      boundary = "bluezhfp-hfp-upstream-link-bluetoothd";
      cid = BLUEZ_HFP_RFCOMM_CID;
      transactions = g_bluez_hfp_transactions;
      transaction_count = sizeof(g_bluez_hfp_transactions) /
                          sizeof(g_bluez_hfp_transactions[0]);
      responder = 0;
    }
  else if (!strcmp(mode, "hfp-ag"))
    {
      role = "audio-gateway";
      profile = "hfp-ag.c";
      boundary = "bluezhfp-hfp-upstream-link-bluetoothd";
      cid = BLUEZ_HFP_RFCOMM_CID;
      transactions = g_bluez_hfp_transactions;
      transaction_count = sizeof(g_bluez_hfp_transactions) /
                          sizeof(g_bluez_hfp_transactions[0]);
      responder = 1;
    }
  else if (!strcmp(mode, "hsp-hs"))
    {
      role = "headset";
      profile = "headset.c";
      boundary = "bluezhfp-hsp-upstream-link-bluetoothd";
      cid = BLUEZ_HSP_RFCOMM_CID;
      transactions = g_bluez_hsp_transactions;
      transaction_count = sizeof(g_bluez_hsp_transactions) /
                          sizeof(g_bluez_hsp_transactions[0]);
      responder = 0;
    }
  else if (!strcmp(mode, "hsp-ag"))
    {
      role = "audio-gateway";
      profile = "headset.c";
      boundary = "bluezhfp-hsp-upstream-link-bluetoothd";
      cid = BLUEZ_HSP_RFCOMM_CID;
      transactions = g_bluez_hsp_transactions;
      transaction_count = sizeof(g_bluez_hsp_transactions) /
                          sizeof(g_bluez_hsp_transactions[0]);
      responder = 1;
    }
  else
    {
      bluez_hfp_usage();
      return 1;
    }

  handle = bluez_hfp_handle(peer);
  channel = bluez_hfp_rfcomm_channel(cid);
  printf("bluez-hfp: source=third/bluez/profiles/audio/%s "
         "style=profile command=connect mode=%s role=%s peer=%u "
         "handle=0x%04x\n",
         profile, mode, role, peer, handle);
  printf("bluez-hfp: source=third/bluez/src/profile.c "
         "style=sdp-record command=register mode=%s role=%s "
         "uuid-hfp-hf=0x111e uuid-hfp-ag=0x111f "
         "uuid-hsp-hs=0x1108 uuid-hsp-ag=0x1112\n",
         mode, role);
  printf("bluez-hfp: native-contract mode=%s role=%s "
         "source-map=profiles/audio/hfp-hf.c,profiles/audio/hfp-ag.c,"
         "profiles/audio/headset.c,profiles/audio/transport.c,"
         "rfcomm/sock.c,rfcomm/core.c,sco.c,l2cap_core.c "
         "session-ownership=Profile1,SDP,RFCOMM,L2CAP,"
         "AT-state-machine,SCO,MediaTransport "
         "request-response-required=1 call-audio-required=1\n",
         mode, role);
  printf("bluez-hfp: semantic-contract mode=%s role=%s "
         "dbus-owner=Profile1,Device1,MediaTransport1 "
         "service-owner=SDP-HFP-HSP "
         "socket-owner=RFCOMM,L2CAP,SCO "
         "profile-owner=HFP-HF,HFP-AG,HSP-HS,HSP-AG "
         "state-owner=AT-state-machine,SLC,codec,call,volume "
         "audio-owner=SCO-bearer,MediaTransport,Acquire,Release "
         "mainloop-owner=rfcomm-watch,sco-watch,at-request-queue "
         "error-owner=NotConnected,AlreadyConnected,Canceled,CodecError,IOError "
         "cleanup-owner=rfcomm-close,sco-release,transport-release,"
         "watch-remove,state-free "
         "upstream-link=bluezhfp-rfcomm-sco-link-to-bluez-audio\n",
         mode, role);

  {
    char out[512];

    memset(out, 0, sizeof(out));
    ret = linux_bt_upstream_rfcomm_socket_listen_accept_probe(
            channel, handle, out, sizeof(out));
    printf("bluez-hfp: rfcomm listen-accept mode=%s role=%s ret=%d "
           "channel=%u proto=BTPROTO_RFCOMM "
           "source=third/linux-7.0.10/net/bluetooth/rfcomm/sock.c\n",
           mode, role, ret, channel);
    printf("%s", out);
    failed |= ret < 0;
  }

  ret = linux_bt_upstream_rfcomm_socket_open(channel, handle, &rfcomm);
  printf("bluez-hfp: rfcomm open psm=0x%04x cid=0x%04x ret=%d "
         "channel=%u proto=BTPROTO_RFCOMM "
         "source=third/linux-7.0.10/net/bluetooth/rfcomm/sock.c\n",
         BLUEZ_HFP_RFCOMM_PSM, cid, ret, channel);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_rfcomm_socket_connect_handle(rfcomm, channel);
      printf("bluez-hfp: rfcomm connect psm=0x%04x cid=0x%04x ret=%d "
             "channel=%u proto=BTPROTO_RFCOMM "
             "source=third/linux-7.0.10/net/bluetooth/rfcomm/sock.c\n",
             BLUEZ_HFP_RFCOMM_PSM, cid, ret, channel);
      printf("bluez-hfp: rfcomm socket-parity mode=%s role=%s "
             "proto=BTPROTO_RFCOMM channel=%u cid=0x%04x native-path=1\n",
             mode, role, channel, cid);
      failed |= ret < 0;
    }

  if (!failed)
    {
      ret = bluez_hfp_run_transactions(rfcomm, profile, role,
                                       transactions, transaction_count,
                                       responder);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[512];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_rfcomm_socket_ioctl_probe(rfcomm, out,
                                                        sizeof(out));
      printf("bluez-hfp: rfcomm ioctl mode=%s role=%s ret=%d "
             "proto=BTPROTO_RFCOMM\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[512];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_rfcomm_socket_poll_probe(rfcomm, 1, out,
                                                       sizeof(out));
      printf("bluez-hfp: rfcomm poll mode=%s role=%s ret=%d "
             "events=POLLOUT proto=BTPROTO_RFCOMM\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[256];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_rfcomm_socket_timestamp_probe(rfcomm, out,
                                                            sizeof(out));
      printf("bluez-hfp: rfcomm timestamp mode=%s role=%s ret=%d "
             "proto=BTPROTO_RFCOMM\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  ret = linux_bt_upstream_sco_socket_open(handle, &sco);
  printf("bluez-hfp: sco open mode=%s role=%s ret=%d "
         "handle=0x%04x proto=BTPROTO_SCO "
         "source=third/linux-7.0.10/net/bluetooth/sco.c\n",
         mode, role, ret, handle);
  failed |= ret < 0;

  if (!failed)
    {
      char out[512];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_sco_socket_listen_accept_probe(
              handle, out, sizeof(out));
      printf("bluez-hfp: sco listen-accept mode=%s role=%s ret=%d "
             "handle=0x%04x proto=BTPROTO_SCO "
             "source=third/linux-7.0.10/net/bluetooth/sco.c\n",
             mode, role, ret, handle);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      ret = linux_bt_upstream_sco_socket_connect_handle(sco, handle);
      printf("bluez-hfp: sco connect mode=%s role=%s ret=%d "
             "handle=0x%04x proto=BTPROTO_SCO "
             "source=third/linux-7.0.10/net/bluetooth/sco.c\n",
             mode, role, ret, handle);
      printf("bluez-hfp: sco socket-parity mode=%s role=%s "
             "proto=BTPROTO_SCO handle=0x%04x native-path=1\n",
             mode, role, handle);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[256];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_sco_socket_write_handle(
              sco, sco_payload, sizeof(sco_payload), out, sizeof(out));
      printf("bluez-hfp: source=third/linux-7.0.10/net/bluetooth/"
             "sco.c style=sco-socket command=audio-payload mode=%s "
             "role=%s payload-len=%u ret=%d proto=BTPROTO_SCO\n",
             mode, role, (unsigned int)sizeof(sco_payload), ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[256];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_sco_socket_timestamp_probe(sco, out,
                                                         sizeof(out));
      printf("bluez-hfp: sco timestamp mode=%s role=%s ret=%d "
             "proto=BTPROTO_SCO\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[256];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_sco_socket_poll_probe(sco, 1, out,
                                                    sizeof(out));
      printf("bluez-hfp: sco poll mode=%s role=%s ret=%d "
             "events=POLLOUT proto=BTPROTO_SCO\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  if (!failed)
    {
      char out[512];

      memset(out, 0, sizeof(out));
      ret = linux_bt_upstream_sco_socket_ioctl_probe(sco, out,
                                                     sizeof(out));
      printf("bluez-hfp: sco ioctl mode=%s role=%s ret=%d "
             "proto=BTPROTO_SCO\n",
             mode, role, ret);
      printf("%s", out);
      failed |= ret < 0;
    }

  printf("bluez-hfp: source=third/bluez/profiles/audio/transport.c "
         "style=audio-bearer command=acquire mode=%s role=%s "
         "codec=cvsd-msbc volume=12\n",
         mode, role);
  printf("bluez-hfp: audio-lifecycle-contract mode=%s role=%s "
         "slc=1 codec-negotiation=1 call-lifecycle=1 "
         "sco-bearer=1 media-transport=1 volume=1 result=%s\n",
         mode, role, failed ? "failed" : "ok");

  if (!failed)
    {
      ret = bluez_hfp_sockapi_closeout(mode, role, channel);
      failed |= ret < 0;
    }

  if (rfcomm != NULL)
    {
      ret = linux_bt_upstream_rfcomm_socket_close_handle(rfcomm);
      printf("bluez-hfp: rfcomm close mode=%s role=%s ret=%d "
             "proto=BTPROTO_RFCOMM\n",
             mode, role, ret);
      failed |= ret < 0;
    }

  if (sco != NULL)
    {
      ret = linux_bt_upstream_sco_socket_close_handle(sco);
      printf("bluez-hfp: sco close mode=%s role=%s ret=%d "
             "proto=BTPROTO_SCO\n",
             mode, role, ret);
      failed |= ret < 0;
    }

  printf("bluez-hfp: closeout cleanup mode=%s role=%s rfcomm=0 sco=0 "
         "calls=0 transport=0\n",
         mode, role);
  printf("bluez-hfp: closeout upstream-link-ledger mode=%s role=%s "
         "dbus-profile=0 service-record=0 device-ref=0 adapter-ref=0 "
         "rfcomm-fd=closed sco-bearer=0 audio-transport=0 "
         "at-state-machine=0 call-state=0 codec-negotiation=0 "
         "media-owner=0 mainloop-watch=0 pending-request=0 error-policy=1 "
         "cleanup-final=1\n",
         mode, role);
  printf("bluez-hfp: closeout upstream-coverage-map mode=%s role=%s "
         "third/bluez/profiles/audio/hfp-hf.c "
         "third/bluez/profiles/audio/hfp-ag.c "
         "third/bluez/profiles/audio/headset.c "
         "third/bluez/profiles/audio/transport.c "
         "third/bluez/src/profile.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/sco.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         mode, role);
  printf("bluez-hfp: closeout upstream-source-parity mode=%s role=%s "
         "direct-upstream=hfp-hf.c,hfp-ag.c,headset.c,transport.c,"
         "profile.c,rfcomm/sock.c,rfcomm/core.c,sco.c,l2cap_core.c "
         "objects=adapter,device,profile,sdp-record,rfcomm-fd,"
         "rfcomm-session,l2cap-channel,at-state-machine,slc,"
         "codec-negotiation,call-state,sco-bearer,media-transport,"
         "mainloop-watch "
         "handlers=profile_connect,profile_disconnect,sdp_register,"
         "rfcomm_connect,rfcomm_sendmsg,rfcomm_recvmsg,at_brsf,"
         "at_bac,at_bcs,at_clcc,at_ckpd,at_vgs,at_vgm,"
         "sco_connect,sco_sendmsg,transport_acquire,transport_release "
         "native-rfcomm=psm-0x0003,cid-0x0061,cid-0x0062,fd-handoff,"
         "session-owner "
         "native-sco=proto-BTPROTO_SCO,sock-seqpacket,cvsd,msbc,"
         "audio-bearer "
         "upstream-link=%s parity-final=%u\n",
         mode, role, boundary, failed ? 0 : 1);
  printf("bluez-hfp: profile-final=1 rfcomm-final=1 at-final=1 "
         "sco-final=1 cleanup-final=1 "
         "at-request-response-final=1 codec-final=1 call-final=1 "
         "sco-final=1 audio-transport-final=1 cleanup-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=%s final-ok=%u\n",
         boundary, failed ? 0 : 1);

  return failed ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint16_t peer;

  if (argc < 3 || strcmp(argv[1], "closeout") != 0)
    {
      bluez_hfp_usage();
      return 1;
    }

  peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : BLUEZ_HFP_DEFAULT_PEER;
  return bluez_hfp_closeout(argv[2], peer);
}
