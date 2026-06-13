/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/hciraw_main.c
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

#ifndef SOL_HCI
#  define SOL_HCI 0
#endif

#define HCI_CHANNEL_RAW       0
#define HCI_CHANNEL_USER      1
#define HCI_CHANNEL_MONITOR   2
#define HCI_DEV_NONE          0xffff
#define HCI_COMMAND_PKT       0x01
#define HCI_EVENT_PKT         0x04
#define HCI_EV_CMD_COMPLETE   0x0e
#define HCI_EV_LE_META        0x3e
#define HCI_EV_LE_ADVERTISING_REPORT 0x02
#define HCI_FILTER            2
#define HCI_OP_SET_EVENT_MASK 0x0c01
#define HCI_OP_READ_LOCAL_NAME 0x0c14
#define HCI_OP_READ_LOCAL_VERSION 0x1001
#define HCI_OP_READ_LOCAL_COMMANDS 0x1002
#define HCI_OP_READ_LOCAL_FEATURES 0x1003
#define HCI_OP_READ_LOCAL_EXT_FEATURES 0x1004
#define HCI_OP_READ_BUFFER_SIZE 0x1005
#define HCI_OP_READ_BD_ADDR   0x1009
#define HCI_OP_WRITE_LE_HOST_SUPPORTED 0x0c6d
#define HCI_OP_LE_SET_EVENT_MASK 0x2001
#define HCI_OP_LE_READ_BUFFER_SIZE 0x2002
#define HCI_OP_LE_READ_LOCAL_FEATURES 0x2003
#define HCI_OP_LE_READ_ADV_TX_POWER 0x2007
#define HCI_OP_LE_READ_ACCEPT_LIST_SIZE 0x200f
#define HCI_OP_LE_READ_RESOLV_LIST_SIZE 0x202a
#define HCI_OP_LE_READ_SUPPORTED_STATES 0x201c
#define HCI_OP_LE_READ_DEF_DATA_LEN 0x2023
#define HCI_OP_LE_READ_MAX_DATA_LEN 0x202f
#define HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS 0x203b
#define HCI_OP_LE_READ_TRANSMIT_POWER 0x204b
#define HCI_OP_RESET          0x0c03
#define HCI_OP_UNKNOWN_TEST   0xffff
#define HCI_STATUS_UNKNOWN_COMMAND 0x01
#define HCI_MON_EVENT_PKT     0x0003
#define HCI_OP_SET_EVENT_MASK_PAGE_2 0x0c63
#define HCI_OP_LE_SET_RANDOM_ADDR 0x2005
#define HCI_OP_LE_SET_ADV_PARAM 0x2006
#define HCI_OP_LE_SET_ADV_DATA 0x2008
#define HCI_OP_LE_SET_SCAN_RSP_DATA 0x2009
#define HCI_OP_LE_SET_ADV_ENABLE 0x200a
#define HCI_OP_LE_SET_SCAN_PARAM 0x200b
#define HCI_OP_LE_SET_SCAN_ENABLE 0x200c
#define HCI_OP_LE_CLEAR_ACCEPT_LIST 0x2010
#define HCI_OP_LE_ADD_TO_ACCEPT_LIST 0x2011
#define HCI_OP_LE_DEL_FROM_ACCEPT_LIST 0x2012
#define HCI_OP_LE_CLEAR_RESOLV_LIST 0x2029
#define HCI_OP_LE_SET_ADDR_RESOLV_ENABLE 0x202d
#define HCI_OP_LE_SET_RPA_TIMEOUT 0x202e

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_hciraw_sockaddr_hci
{
  uint16_t hci_family;
  uint16_t hci_dev;
  uint16_t hci_channel;
};

struct bluez_hciraw_filter
{
  uint32_t type_mask;
  uint32_t event_mask[2];
  uint16_t opcode;
};

struct bluez_hciraw_mon_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_hciraw_usage(void)
{
  printf("usage: bluezhciraw command|user-command|user-command-monitor|user-command-sequence-monitor|user-command-error-monitor|user-command-init-sequence-monitor|user-advertise-enable|user-scan-report\n");
  printf("\n");
  printf("BlueZ hcitool-style raw HCI command smoke over AF_BLUETOOTH/BTPROTO_HCI.\n");
}

static int bluez_hciraw_open_hci(uint16_t channel, uint16_t dev,
                                 const char *name)
{
  struct bluez_hciraw_sockaddr_hci addr;
  int fd;
  int ret;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  printf("bluez-hciraw: hci-socket-%s fd=%d errno=%d\n",
         name, fd, fd < 0 ? errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = dev;
  addr.hci_channel = channel;
  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  printf("bluez-hciraw: hci-bind-%s ret=%d errno=%d\n", name, ret,
         ret < 0 ? errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_hciraw_recv_monitor_event(int fd, uint16_t expect_op,
                                           uint8_t expect_status)
{
  uint8_t buf[320];
  struct bluez_hciraw_mon_hdr *hdr = (struct bluez_hciraw_mon_hdr *)buf;
  unsigned int count = 0;
  unsigned int event_seen = 0;
  unsigned int tries;

  for (tries = 0; tries < 40; tries++)
    {
      uint16_t mon_event = 0;
      uint16_t index = 0;
      uint16_t len = 0;
      uint16_t opcode = 0;
      uint16_t manufacturer = 0;
      uint16_t lmp_subver = 0;
      uint8_t hci_event = 0;
      uint8_t hci_ver = 0;
      uint8_t status = 0xff;
      ssize_t n;

      memset(buf, 0, sizeof(buf));
      n = recv(fd, buf, sizeof(buf), 0);
      if (n < 0)
        {
          printf("bluez-hciraw: monitor-recv ret=%ld errno=%d count=%u\n",
                 (long)n, errno, count);
          break;
        }

      if ((size_t)n >= sizeof(*hdr))
        {
          mon_event = hdr->opcode;
          index = hdr->index;
          len = hdr->len;
        }

      if (mon_event == HCI_MON_EVENT_PKT &&
          (size_t)n >= sizeof(*hdr) + 6 &&
          buf[sizeof(*hdr)] == HCI_EV_CMD_COMPLETE)
        {
          hci_event = buf[sizeof(*hdr)];
          opcode = (uint16_t)buf[sizeof(*hdr) + 3] |
                   ((uint16_t)buf[sizeof(*hdr) + 4] << 8);
          status = buf[sizeof(*hdr) + 5];
          if (opcode == expect_op && status == expect_status)
            {
              event_seen = 1;
            }
        }

      printf("bluez-hciraw: monitor-recv ret=%ld mon-event=0x%04x index=0x%04x len=%u hci-event=0x%02x opcode=0x%04x status=0x%02x\n",
             (long)n, mon_event, index, len, hci_event, opcode, status);
      count++;
    }

  printf("bluez-hciraw: monitor-count=%u event-seen=%u\n", count,
         event_seen);
  return event_seen ? 0 : -1;
}

static int bluez_hciraw_recv_monitor_sequence(int fd, uint16_t first_op,
                                              uint16_t second_op,
                                              uint16_t third_op,
                                              uint16_t fourth_op,
                                              uint16_t fifth_op,
                                              uint16_t sixth_op,
                                              uint16_t seventh_op,
                                              uint16_t eighth_op,
                                              uint16_t ninth_op,
                                              uint16_t tenth_op,
                                              uint16_t eleventh_op,
                                              uint16_t twelfth_op)
{
  uint8_t buf[320];
  struct bluez_hciraw_mon_hdr *hdr = (struct bluez_hciraw_mon_hdr *)buf;
  unsigned int count = 0;
  unsigned int first_seen = 0;
  unsigned int second_seen = 0;
  unsigned int third_seen = 0;
  unsigned int fourth_seen = 0;
  unsigned int fifth_seen = 0;
  unsigned int sixth_seen = 0;
  unsigned int seventh_seen = 0;
  unsigned int eighth_seen = 0;
  unsigned int ninth_seen = 0;
  unsigned int tenth_seen = 0;
  unsigned int eleventh_seen = 0;
  unsigned int twelfth_seen = 0;
  unsigned int tries;

  for (tries = 0; tries < 32; tries++)
    {
      uint16_t mon_event = 0;
      uint16_t index = 0;
      uint16_t len = 0;
      uint16_t opcode = 0;
      uint16_t manufacturer = 0;
      uint16_t lmp_subver = 0;
      uint8_t hci_event = 0;
      uint8_t hci_ver = 0;
      uint8_t status = 0xff;
      unsigned int supported_len = 0;
      unsigned int features_len = 0;
      unsigned int le_features_len = 0;
      unsigned int le_states_len = 0;
      uint16_t acl_mtu = 0;
      uint16_t acl_pkts = 0;
      uint8_t sco_mtu = 0;
      uint16_t sco_pkts = 0;
      uint16_t le_mtu = 0;
      uint8_t le_pkts = 0;
      uint8_t accept_list_size = 0;
      uint8_t resolv_list_size = 0;
      uint8_t adv_sets = 0;
      char bdaddr[18] = "00:00:00:00:00:00";
      ssize_t n;

      memset(buf, 0, sizeof(buf));
      n = recv(fd, buf, sizeof(buf), 0);
      if (n < 0)
        {
          printf("bluez-hciraw: sequence-monitor-recv ret=%ld errno=%d count=%u\n",
                 (long)n, errno, count);
          break;
        }

      if ((size_t)n >= sizeof(*hdr))
        {
          mon_event = hdr->opcode;
          index = hdr->index;
          len = hdr->len;
        }

      if (mon_event == HCI_MON_EVENT_PKT &&
          (size_t)n >= sizeof(*hdr) + 6 &&
          buf[sizeof(*hdr)] == HCI_EV_CMD_COMPLETE)
        {
          hci_event = buf[sizeof(*hdr)];
          opcode = (uint16_t)buf[sizeof(*hdr) + 3] |
                   ((uint16_t)buf[sizeof(*hdr) + 4] << 8);
          status = buf[sizeof(*hdr) + 5];
          if ((size_t)n >= sizeof(*hdr) + 14 &&
              opcode == HCI_OP_READ_LOCAL_VERSION)
            {
              hci_ver = buf[sizeof(*hdr) + 6];
              manufacturer = (uint16_t)buf[sizeof(*hdr) + 10] |
                             ((uint16_t)buf[sizeof(*hdr) + 11] << 8);
              lmp_subver = (uint16_t)buf[sizeof(*hdr) + 12] |
                           ((uint16_t)buf[sizeof(*hdr) + 13] << 8);
            }
          if ((size_t)n >= sizeof(*hdr) + 12 &&
              opcode == HCI_OP_READ_BD_ADDR)
            {
              snprintf(bdaddr, sizeof(bdaddr),
                       "%02x:%02x:%02x:%02x:%02x:%02x",
                       buf[sizeof(*hdr) + 11], buf[sizeof(*hdr) + 10],
                       buf[sizeof(*hdr) + 9], buf[sizeof(*hdr) + 8],
                       buf[sizeof(*hdr) + 7], buf[sizeof(*hdr) + 6]);
            }
          if ((size_t)n >= sizeof(*hdr) + 70 &&
              opcode == HCI_OP_READ_LOCAL_COMMANDS)
            {
              supported_len = 64;
            }
          if ((size_t)n >= sizeof(*hdr) + 14 &&
              opcode == HCI_OP_READ_LOCAL_FEATURES)
            {
              features_len = 8;
            }
          if ((size_t)n >= sizeof(*hdr) + 13 &&
              opcode == HCI_OP_READ_BUFFER_SIZE)
            {
              acl_mtu = (uint16_t)buf[sizeof(*hdr) + 6] |
                        ((uint16_t)buf[sizeof(*hdr) + 7] << 8);
              sco_mtu = buf[sizeof(*hdr) + 8];
              acl_pkts = (uint16_t)buf[sizeof(*hdr) + 9] |
                         ((uint16_t)buf[sizeof(*hdr) + 10] << 8);
              sco_pkts = (uint16_t)buf[sizeof(*hdr) + 11] |
                         ((uint16_t)buf[sizeof(*hdr) + 12] << 8);
            }
          if ((size_t)n >= sizeof(*hdr) + 9 &&
              opcode == HCI_OP_LE_READ_BUFFER_SIZE)
            {
              le_mtu = (uint16_t)buf[sizeof(*hdr) + 6] |
                       ((uint16_t)buf[sizeof(*hdr) + 7] << 8);
              le_pkts = buf[sizeof(*hdr) + 8];
            }
          if ((size_t)n >= sizeof(*hdr) + 14 &&
              opcode == HCI_OP_LE_READ_LOCAL_FEATURES)
            {
              le_features_len = 8;
            }
          if ((size_t)n >= sizeof(*hdr) + 14 &&
              opcode == HCI_OP_LE_READ_SUPPORTED_STATES)
            {
              le_states_len = 8;
            }
          if ((size_t)n >= sizeof(*hdr) + 7 &&
              opcode == HCI_OP_LE_READ_ACCEPT_LIST_SIZE)
            {
              accept_list_size = buf[sizeof(*hdr) + 6];
            }
          if ((size_t)n >= sizeof(*hdr) + 7 &&
              opcode == HCI_OP_LE_READ_RESOLV_LIST_SIZE)
            {
              resolv_list_size = buf[sizeof(*hdr) + 6];
            }
          if ((size_t)n >= sizeof(*hdr) + 7 &&
              opcode == HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS)
            {
              adv_sets = buf[sizeof(*hdr) + 6];
            }
          if (opcode == first_op && status == 0)
            {
              first_seen = 1;
            }
          if (opcode == second_op && status == 0)
            {
              second_seen = 1;
            }
          if (opcode == third_op && status == 0)
            {
              third_seen = 1;
            }
          if (opcode == fourth_op && status == 0)
            {
              fourth_seen = 1;
            }
          if (opcode == fifth_op && status == 0)
            {
              fifth_seen = 1;
            }
          if (opcode == sixth_op && status == 0)
            {
              sixth_seen = 1;
            }
          if (opcode == seventh_op && status == 0)
            {
              seventh_seen = 1;
            }
          if (opcode == eighth_op && status == 0)
            {
              eighth_seen = 1;
            }
          if (opcode == ninth_op && status == 0)
            {
              ninth_seen = 1;
            }
          if (opcode == tenth_op && status == 0)
            {
              tenth_seen = 1;
            }
          if (opcode == eleventh_op && status == 0)
            {
              eleventh_seen = 1;
            }
          if (opcode == twelfth_op && status == 0)
            {
              twelfth_seen = 1;
            }
        }

      printf("bluez-hciraw: sequence-monitor-recv ret=%ld mon-event=0x%04x index=0x%04x len=%u hci-event=0x%02x opcode=0x%04x status=0x%02x hci-ver=0x%02x manufacturer=0x%04x lmp-subver=0x%04x supported-len=%u features-len=%u le-features-len=%u le-states-len=%u acl-mtu=%u acl-pkts=%u sco-mtu=%u sco-pkts=%u le-mtu=%u le-pkts=%u accept-list-size=%u resolv-list-size=%u adv-sets=%u bdaddr=%s first-seen=%u second-seen=%u third-seen=%u fourth-seen=%u fifth-seen=%u sixth-seen=%u seventh-seen=%u eighth-seen=%u ninth-seen=%u tenth-seen=%u eleventh-seen=%u twelfth-seen=%u\n",
             (long)n, mon_event, index, len, hci_event, opcode, status,
             hci_ver, manufacturer, lmp_subver, supported_len,
             features_len, le_features_len, le_states_len, acl_mtu,
             acl_pkts, sco_mtu, sco_pkts, le_mtu, le_pkts,
             accept_list_size, resolv_list_size, adv_sets, bdaddr,
             first_seen, second_seen, third_seen, fourth_seen, fifth_seen,
             sixth_seen, seventh_seen, eighth_seen, ninth_seen, tenth_seen,
             eleventh_seen, twelfth_seen);
      count++;
    }

  printf("bluez-hciraw: sequence-monitor-count=%u first-seen=%u second-seen=%u third-seen=%u fourth-seen=%u fifth-seen=%u sixth-seen=%u seventh-seen=%u eighth-seen=%u ninth-seen=%u tenth-seen=%u eleventh-seen=%u twelfth-seen=%u\n",
         count, first_seen, second_seen, third_seen, fourth_seen,
         fifth_seen, sixth_seen, seventh_seen, eighth_seen, ninth_seen,
         tenth_seen, eleventh_seen, twelfth_seen);
  return first_seen && second_seen && third_seen && fourth_seen &&
         fifth_seen && sixth_seen && seventh_seen && eighth_seen &&
         ninth_seen && tenth_seen && eleventh_seen && twelfth_seen ? 0 : -1;
}

static int bluez_hciraw_recv_monitor_ops(int fd, const uint16_t *ops,
                                         unsigned int op_count,
                                         const char *name)
{
  uint8_t buf[320];
  struct bluez_hciraw_mon_hdr *hdr = (struct bluez_hciraw_mon_hdr *)buf;
  unsigned int seen[32];
  unsigned int count = 0;
  unsigned int tries;
  unsigned int i;

  memset(seen, 0, sizeof(seen));

  for (tries = 0; tries < 80; tries++)
    {
      uint16_t mon_event = 0;
      uint16_t index = 0;
      uint16_t len = 0;
      uint16_t opcode = 0;
      uint8_t hci_event = 0;
      uint8_t status = 0xff;
      ssize_t n;

      memset(buf, 0, sizeof(buf));
      n = recv(fd, buf, sizeof(buf), 0);
      if (n < 0)
        {
          printf("bluez-hciraw: init-monitor-recv ret=%ld errno=%d count=%u\n",
                 (long)n, errno, count);
          break;
        }

      if ((size_t)n >= sizeof(*hdr))
        {
          mon_event = hdr->opcode;
          index = hdr->index;
          len = hdr->len;
        }

      if (mon_event == HCI_MON_EVENT_PKT &&
          (size_t)n >= sizeof(*hdr) + 6 &&
          buf[sizeof(*hdr)] == HCI_EV_CMD_COMPLETE)
        {
          hci_event = buf[sizeof(*hdr)];
          opcode = (uint16_t)buf[sizeof(*hdr) + 3] |
                   ((uint16_t)buf[sizeof(*hdr) + 4] << 8);
          status = buf[sizeof(*hdr) + 5];
          for (i = 0; i < op_count && i < 32; i++)
            {
              if (opcode == ops[i] && status == 0)
                {
                  seen[i] = 1;
                }
            }
        }

      printf("bluez-hciraw: %s-monitor-recv ret=%ld mon-event=0x%04x index=0x%04x len=%u hci-event=0x%02x opcode=0x%04x status=0x%02x",
             name, (long)n, mon_event, index, len, hci_event, opcode,
             status);
      for (i = 0; i < op_count && i < 32; i++)
        {
          printf(" seen%u=%u", i + 1, seen[i]);
        }

      printf("\n");
      count++;
    }

  printf("bluez-hciraw: %s-monitor-count=%u", name, count);
  for (i = 0; i < op_count && i < 32; i++)
    {
      printf(" seen%u=%u", i + 1, seen[i]);
    }

  printf("\n");

  for (i = 0; i < op_count && i < 32; i++)
    {
      if (!seen[i])
        {
          return -1;
        }
    }

  return op_count <= 32 ? 0 : -1;
}

#if 0
static int bluez_hciraw_recv_monitor_triple(int fd, uint16_t first_op,
                                            uint16_t second_op,
                                            uint16_t third_op)
{
  uint8_t buf[320];
  struct bluez_hciraw_mon_hdr *hdr = (struct bluez_hciraw_mon_hdr *)buf;
  unsigned int count = 0;
  unsigned int first_seen = 0;
  unsigned int second_seen = 0;
  unsigned int third_seen = 0;
  unsigned int tries;

  for (tries = 0; tries < 16; tries++)
    {
      uint16_t mon_event = 0;
      uint16_t index = 0;
      uint16_t len = 0;
      uint16_t opcode = 0;
      uint8_t hci_event = 0;
      uint8_t status = 0xff;
      ssize_t n;

      memset(buf, 0, sizeof(buf));
      n = recv(fd, buf, sizeof(buf), 0);
      if (n < 0)
        {
          printf("bluez-hciraw: init-monitor-recv ret=%ld errno=%d count=%u\n",
                 (long)n, errno, count);
          break;
        }

      if ((size_t)n >= sizeof(*hdr))
        {
          mon_event = hdr->opcode;
          index = hdr->index;
          len = hdr->len;
        }

      if (mon_event == HCI_MON_EVENT_PKT &&
          (size_t)n >= sizeof(*hdr) + 6 &&
          buf[sizeof(*hdr)] == HCI_EV_CMD_COMPLETE)
        {
          hci_event = buf[sizeof(*hdr)];
          opcode = (uint16_t)buf[sizeof(*hdr) + 3] |
                   ((uint16_t)buf[sizeof(*hdr) + 4] << 8);
          status = buf[sizeof(*hdr) + 5];
          if (opcode == first_op && status == 0)
            {
              first_seen = 1;
            }
          if (opcode == second_op && status == 0)
            {
              second_seen = 1;
            }
          if (opcode == third_op && status == 0)
            {
              third_seen = 1;
            }
        }

      printf("bluez-hciraw: init-monitor-recv ret=%ld mon-event=0x%04x index=0x%04x len=%u hci-event=0x%02x opcode=0x%04x status=0x%02x first-seen=%u second-seen=%u third-seen=%u\n",
             (long)n, mon_event, index, len, hci_event, opcode, status,
             first_seen, second_seen, third_seen);
      count++;
    }

  printf("bluez-hciraw: init-monitor-count=%u first-seen=%u second-seen=%u third-seen=%u\n",
         count, first_seen, second_seen, third_seen);
  return first_seen && second_seen && third_seen ? 0 : -1;
}
#endif

static int bluez_hciraw_send_recv_params_on_fd(int fd, uint16_t op,
                                               const uint8_t *params,
                                               uint8_t params_len,
                                               const char *tag,
                                               uint8_t expect_status)
{
  uint8_t cmd[260];
  uint8_t event[260];
  uint16_t opcode = 0;
  uint8_t status = 0xff;
  ssize_t n;
  int failed = 0;

  cmd[0] = HCI_COMMAND_PKT;
  cmd[1] = (uint8_t)(op & 0xff);
  cmd[2] = (uint8_t)(op >> 8);
  cmd[3] = params_len;
  if (params_len != 0 && params != NULL)
    {
      memcpy(&cmd[4], params, params_len);
    }

  n = send(fd, cmd, 4 + params_len, 0);
  printf("bluez-hciraw: init-send tag=%s opcode=0x%04x plen=%u ret=%ld errno=%d\n",
         tag, op, params_len, (long)n, n < 0 ? errno : 0);
  failed |= n != (ssize_t)(4 + params_len) ? 1 : 0;

  memset(event, 0, sizeof(event));
  n = recv(fd, event, sizeof(event), 0);
  if (n >= 7 && event[0] == HCI_EVENT_PKT && event[1] == HCI_EV_CMD_COMPLETE)
    {
      opcode = (uint16_t)event[4] | ((uint16_t)event[5] << 8);
      status = event[6];
    }

  printf("bluez-hciraw: init-recv tag=%s ret=%ld errno=%d pkt=0x%02x event=0x%02x opcode=0x%04x status=0x%02x\n",
         tag, (long)n, n < 0 ? errno : 0, event[0], event[1], opcode,
         status);
  failed |= n < 7 || event[0] != HCI_EVENT_PKT ||
            event[1] != HCI_EV_CMD_COMPLETE ||
            opcode != op || status != expect_status ? 1 : 0;

  return failed;
}

static int bluez_hciraw_send_recv_on_fd(int fd, uint16_t op,
                                        const char *tag,
                                        uint8_t expect_status)
{
  uint8_t cmd[4] =
  {
    HCI_COMMAND_PKT,
    (uint8_t)(op & 0xff),
    (uint8_t)(op >> 8),
    0,
  };
  uint8_t event[260];
  uint16_t opcode = 0;
  uint16_t manufacturer = 0;
  uint16_t lmp_subver = 0;
  uint8_t hci_ver = 0;
  uint8_t status = 0xff;
  unsigned int supported_len = 0;
  unsigned int features_len = 0;
  unsigned int le_features_len = 0;
  unsigned int le_states_len = 0;
  uint16_t acl_mtu = 0;
  uint16_t acl_pkts = 0;
  uint8_t sco_mtu = 0;
  uint16_t sco_pkts = 0;
  uint16_t le_mtu = 0;
  uint8_t le_pkts = 0;
  uint8_t accept_list_size = 0;
  uint8_t resolv_list_size = 0;
  uint8_t adv_sets = 0;
  char bdaddr[18] = "00:00:00:00:00:00";
  ssize_t n;
  int failed = 0;

  n = send(fd, cmd, sizeof(cmd), 0);
  printf("bluez-hciraw: sequence-send tag=%s opcode=0x%04x ret=%ld errno=%d\n",
         tag, op, (long)n, n < 0 ? errno : 0);
  failed |= n != (ssize_t)sizeof(cmd) ? 1 : 0;

  memset(event, 0, sizeof(event));
  n = recv(fd, event, sizeof(event), 0);
  if (n >= 7 && event[0] == HCI_EVENT_PKT && event[1] == HCI_EV_CMD_COMPLETE)
    {
      opcode = (uint16_t)event[4] | ((uint16_t)event[5] << 8);
      status = event[6];
      if (n >= 15 && opcode == HCI_OP_READ_LOCAL_VERSION)
        {
          hci_ver = event[7];
          manufacturer = (uint16_t)event[11] | ((uint16_t)event[12] << 8);
          lmp_subver = (uint16_t)event[13] | ((uint16_t)event[14] << 8);
        }
      if (n >= 13 && opcode == HCI_OP_READ_BD_ADDR)
        {
          snprintf(bdaddr, sizeof(bdaddr),
                   "%02x:%02x:%02x:%02x:%02x:%02x",
                   event[12], event[11], event[10], event[9], event[8],
                   event[7]);
        }
      if (n >= 71 && opcode == HCI_OP_READ_LOCAL_COMMANDS)
        {
          supported_len = 64;
        }
      if (n >= 15 && opcode == HCI_OP_READ_LOCAL_FEATURES)
        {
          features_len = 8;
        }
      if (n >= 14 && opcode == HCI_OP_READ_BUFFER_SIZE)
        {
          acl_mtu = (uint16_t)event[7] | ((uint16_t)event[8] << 8);
          sco_mtu = event[9];
          acl_pkts = (uint16_t)event[10] | ((uint16_t)event[11] << 8);
          sco_pkts = (uint16_t)event[12] | ((uint16_t)event[13] << 8);
        }
      if (n >= 10 && opcode == HCI_OP_LE_READ_BUFFER_SIZE)
        {
          le_mtu = (uint16_t)event[7] | ((uint16_t)event[8] << 8);
          le_pkts = event[9];
        }
      if (n >= 15 && opcode == HCI_OP_LE_READ_LOCAL_FEATURES)
        {
          le_features_len = 8;
        }
      if (n >= 15 && opcode == HCI_OP_LE_READ_SUPPORTED_STATES)
        {
          le_states_len = 8;
        }
      if (n >= 8 && opcode == HCI_OP_LE_READ_ACCEPT_LIST_SIZE)
        {
          accept_list_size = event[7];
        }
      if (n >= 8 && opcode == HCI_OP_LE_READ_RESOLV_LIST_SIZE)
        {
          resolv_list_size = event[7];
        }
      if (n >= 8 && opcode == HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS)
        {
          adv_sets = event[7];
        }
    }

  printf("bluez-hciraw: sequence-recv tag=%s ret=%ld errno=%d pkt=0x%02x event=0x%02x opcode=0x%04x status=0x%02x hci-ver=0x%02x manufacturer=0x%04x lmp-subver=0x%04x supported-len=%u features-len=%u le-features-len=%u le-states-len=%u acl-mtu=%u acl-pkts=%u sco-mtu=%u sco-pkts=%u le-mtu=%u le-pkts=%u accept-list-size=%u resolv-list-size=%u adv-sets=%u bdaddr=%s\n",
         tag, (long)n, n < 0 ? errno : 0, event[0], event[1], opcode,
         status, hci_ver, manufacturer, lmp_subver, supported_len,
         features_len, le_features_len, le_states_len, acl_mtu, acl_pkts,
         sco_mtu, sco_pkts, le_mtu, le_pkts, accept_list_size,
         resolv_list_size, adv_sets, bdaddr);
  failed |= n < 7 || event[0] != HCI_EVENT_PKT ||
            event[1] != HCI_EV_CMD_COMPLETE ||
            opcode != op || status != expect_status ? 1 : 0;

  return failed;
}

static int bluez_hciraw_command_common(uint16_t channel,
                                       const char *mode,
                                       const char *bind_name,
                                       uint16_t op,
                                       bool use_filter)
{
  struct bluez_hciraw_sockaddr_hci addr;
  struct bluez_hciraw_filter filter;
  struct bluez_hciraw_filter readback;
  socklen_t readback_len;
  uint8_t cmd[4] =
  {
    HCI_COMMAND_PKT,
    (uint8_t)(op & 0xff),
    (uint8_t)(op >> 8),
    0,
  };
  uint8_t event[260];
  uint16_t opcode = 0;
  uint8_t status = 0xff;
  int fd;
  int ret;
  int failed = 0;
  ssize_t n;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=%s\n",
         mode);

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  printf("bluez-hciraw: hci-socket fd=%d errno=%d\n", fd,
         fd < 0 ? errno : 0);
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = 0;
  addr.hci_channel = channel;
  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  printf("bluez-hciraw: hci-bind-%s ret=%d errno=%d\n", bind_name, ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (use_filter)
    {
      memset(&filter, 0, sizeof(filter));
      filter.type_mask = 1u << HCI_EVENT_PKT;
      filter.event_mask[0] = 0xffffffffu;
      filter.event_mask[1] = 0xffffffffu;
      filter.opcode = 0;
      ret = setsockopt(fd, SOL_HCI, HCI_FILTER, &filter, sizeof(filter));
      printf("bluez-hciraw: setsockopt-filter ret=%d errno=%d\n", ret,
             ret < 0 ? errno : 0);
      failed |= ret < 0 ? 1 : 0;

      memset(&readback, 0, sizeof(readback));
      readback_len = sizeof(readback);
      ret = getsockopt(fd, SOL_HCI, HCI_FILTER, &readback, &readback_len);
      printf("bluez-hciraw: getsockopt-filter ret=%d errno=%d len=%u type-mask=0x%08lx event0=0x%08lx opcode=0x%04x\n",
             ret, ret < 0 ? errno : 0, (unsigned int)readback_len,
             (unsigned long)readback.type_mask,
             (unsigned long)readback.event_mask[0], readback.opcode);
      failed |= ret < 0 ? 1 : 0;
    }

  n = send(fd, cmd, sizeof(cmd), 0);
  printf("bluez-hciraw: send-command opcode=0x%04x ret=%ld errno=%d\n",
         op, (long)n, n < 0 ? errno : 0);
  failed |= n != (ssize_t)sizeof(cmd) ? 1 : 0;

  memset(event, 0, sizeof(event));
  n = recv(fd, event, sizeof(event), 0);
  if (n >= 7 && event[0] == HCI_EVENT_PKT && event[1] == HCI_EV_CMD_COMPLETE)
    {
      opcode = (uint16_t)event[4] | ((uint16_t)event[5] << 8);
      status = event[6];
    }

  printf("bluez-hciraw: recv-event ret=%ld errno=%d pkt=0x%02x event=0x%02x opcode=0x%04x status=0x%02x\n",
         (long)n, n < 0 ? errno : 0, event[0], event[1], opcode, status);
  failed |= n < 7 || event[0] != HCI_EVENT_PKT ||
            event[1] != HCI_EV_CMD_COMPLETE ||
            opcode != op || status != 0 ? 1 : 0;

  ret = close(fd);
  printf("bluez-hciraw: hci-close ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: %s complete\n", mode);
    }

  return failed;
}

static int bluez_hciraw_command(void)
{
  return bluez_hciraw_command_common(HCI_CHANNEL_RAW, "command", "raw",
                                     HCI_OP_READ_LOCAL_VERSION, true);
}

static int bluez_hciraw_user_command(void)
{
  return bluez_hciraw_command_common(HCI_CHANNEL_USER, "user-command",
                                     "user", HCI_OP_RESET, false);
}

static int bluez_hciraw_user_command_monitor(void)
{
  int mon_fd;
  int ret;
  int failed = 0;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-command-monitor\n");

  mon_fd = bluez_hciraw_open_hci(HCI_CHANNEL_MONITOR, HCI_DEV_NONE,
                                 "monitor");
  if (mon_fd < 0)
    {
      return 1;
    }

  failed |= bluez_hciraw_command_common(HCI_CHANNEL_USER, "user-command",
                                        "user", HCI_OP_RESET, false);
  failed |= bluez_hciraw_recv_monitor_event(mon_fd, HCI_OP_RESET, 0) < 0;

  ret = close(mon_fd);
  printf("bluez-hciraw: hci-close-monitor ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-command-monitor complete\n");
    }

  return failed;
}

static int bluez_hciraw_user_command_sequence_monitor(void)
{
  int mon_fd;
  int user_fd;
  int ret;
  int failed = 0;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-command-sequence-monitor\n");

  mon_fd = bluez_hciraw_open_hci(HCI_CHANNEL_MONITOR, HCI_DEV_NONE,
                                 "monitor");
  if (mon_fd < 0)
    {
      return 1;
    }

  user_fd = bluez_hciraw_open_hci(HCI_CHANNEL_USER, 0, "user");
  if (user_fd < 0)
    {
      close(mon_fd);
      return 1;
    }

  failed |= bluez_hciraw_send_recv_on_fd(user_fd, HCI_OP_RESET, "reset",
                                         0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_READ_LOCAL_VERSION,
                                         "read-local-version", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_READ_LOCAL_COMMANDS,
                                         "read-local-commands", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_READ_LOCAL_FEATURES,
                                         "read-local-features", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd, HCI_OP_READ_BUFFER_SIZE,
                                         "read-buffer-size", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_BUFFER_SIZE,
                                         "le-read-buffer-size", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_LOCAL_FEATURES,
                                         "le-read-local-features", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_SUPPORTED_STATES,
                                         "le-read-supported-states", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_ACCEPT_LIST_SIZE,
                                         "le-read-accept-list-size", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_RESOLV_LIST_SIZE,
                                         "le-read-resolv-list-size", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd,
                                         HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS,
                                         "le-read-num-adv-sets", 0);
  failed |= bluez_hciraw_send_recv_on_fd(user_fd, HCI_OP_READ_BD_ADDR,
                                         "read-bd-addr", 0);

  ret = close(user_fd);
  printf("bluez-hciraw: hci-close-user ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  failed |= bluez_hciraw_recv_monitor_sequence(mon_fd, HCI_OP_RESET,
                                               HCI_OP_READ_LOCAL_VERSION,
                                               HCI_OP_READ_LOCAL_COMMANDS,
                                               HCI_OP_READ_LOCAL_FEATURES,
                                               HCI_OP_READ_BUFFER_SIZE,
                                               HCI_OP_LE_READ_BUFFER_SIZE,
                                               HCI_OP_LE_READ_LOCAL_FEATURES,
                                               HCI_OP_LE_READ_SUPPORTED_STATES,
                                               HCI_OP_LE_READ_ACCEPT_LIST_SIZE,
                                               HCI_OP_LE_READ_RESOLV_LIST_SIZE,
                                               HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS,
                                               HCI_OP_READ_BD_ADDR) < 0;

  ret = close(mon_fd);
  printf("bluez-hciraw: hci-close-monitor ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-command-sequence-monitor complete\n");
    }

  return failed;
}

static int bluez_hciraw_user_command_error_monitor(void)
{
  int mon_fd;
  int user_fd;
  int ret;
  int failed = 0;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-command-error-monitor\n");

  mon_fd = bluez_hciraw_open_hci(HCI_CHANNEL_MONITOR, HCI_DEV_NONE,
                                 "monitor");
  if (mon_fd < 0)
    {
      return 1;
    }

  user_fd = bluez_hciraw_open_hci(HCI_CHANNEL_USER, 0, "user");
  if (user_fd < 0)
    {
      close(mon_fd);
      return 1;
    }

  failed |= bluez_hciraw_send_recv_on_fd(user_fd, HCI_OP_UNKNOWN_TEST,
                                         "unknown",
                                         HCI_STATUS_UNKNOWN_COMMAND);

  ret = close(user_fd);
  printf("bluez-hciraw: hci-close-user ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  failed |= bluez_hciraw_recv_monitor_event(mon_fd, HCI_OP_UNKNOWN_TEST,
                                            HCI_STATUS_UNKNOWN_COMMAND) < 0;

  ret = close(mon_fd);
  printf("bluez-hciraw: hci-close-monitor ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-command-error-monitor complete\n");
    }

  return failed;
}

static int bluez_hciraw_user_command_init_sequence_monitor(void)
{
  static const uint16_t monitor_ops[] =
  {
    HCI_OP_SET_EVENT_MASK,
    HCI_OP_LE_SET_EVENT_MASK,
    HCI_OP_WRITE_LE_HOST_SUPPORTED,
    HCI_OP_READ_LOCAL_EXT_FEATURES,
    HCI_OP_READ_LOCAL_NAME,
    HCI_OP_LE_READ_ADV_TX_POWER,
    HCI_OP_LE_READ_DEF_DATA_LEN,
    HCI_OP_LE_READ_MAX_DATA_LEN,
    HCI_OP_LE_READ_TRANSMIT_POWER,
    HCI_OP_SET_EVENT_MASK_PAGE_2,
    HCI_OP_LE_SET_RANDOM_ADDR,
    HCI_OP_LE_SET_ADV_PARAM,
    HCI_OP_LE_SET_ADV_DATA,
    HCI_OP_LE_SET_SCAN_RSP_DATA,
    HCI_OP_LE_SET_ADV_ENABLE,
    HCI_OP_LE_SET_SCAN_PARAM,
    HCI_OP_LE_SET_SCAN_ENABLE,
    HCI_OP_LE_CLEAR_ACCEPT_LIST,
    HCI_OP_LE_ADD_TO_ACCEPT_LIST,
    HCI_OP_LE_DEL_FROM_ACCEPT_LIST,
    HCI_OP_LE_CLEAR_RESOLV_LIST,
    HCI_OP_LE_SET_ADDR_RESOLV_ENABLE,
    HCI_OP_LE_SET_RPA_TIMEOUT,
  };
  static const uint8_t event_mask[8] =
  {
    0xff, 0xff, 0xfb, 0xff, 0x07, 0xf8, 0xbf, 0x3d
  };
  static const uint8_t le_event_mask[8] =
  {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f
  };
  static const uint8_t le_host_supported[2] =
  {
    0x01, 0x00
  };
  static const uint8_t ext_features_page0[1] =
  {
    0x00
  };
  static const uint8_t event_mask_page2[8] =
  {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };
  static const uint8_t random_addr[6] =
  {
    0xc3, 0x00, 0x00, 0x00, 0xfe, 0x02
  };
  static const uint8_t adv_param[15] =
  {
    0xa0, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00
  };
  static const uint8_t adv_data[32] =
  {
    0
  };
  static const uint8_t scan_rsp_data[32] =
  {
    0
  };
  static const uint8_t adv_enable[1] =
  {
    0x01
  };
  static const uint8_t scan_param[7] =
  {
    0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00
  };
  static const uint8_t scan_enable[2] =
  {
    0x01, 0x00
  };
  static const uint8_t accept_list_addr[7] =
  {
    0x00, 0x02, 0xfe, 0x00, 0x00, 0x00, 0x04
  };
  static const uint8_t addr_resolv_enable[1] =
  {
    0x01
  };
  static const uint8_t rpa_timeout[2] =
  {
    0x84, 0x03
  };
  int mon_fd;
  int user_fd;
  int ret;
  int failed = 0;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-command-init-sequence-monitor\n");

  mon_fd = bluez_hciraw_open_hci(HCI_CHANNEL_MONITOR, HCI_DEV_NONE,
                                 "monitor");
  if (mon_fd < 0)
    {
      return 1;
    }

  user_fd = bluez_hciraw_open_hci(HCI_CHANNEL_USER, 0, "user");
  if (user_fd < 0)
    {
      close(mon_fd);
      return 1;
    }

  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_SET_EVENT_MASK,
                                                event_mask,
                                                sizeof(event_mask),
                                                "set-event-mask", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_EVENT_MASK,
                                                le_event_mask,
                                                sizeof(le_event_mask),
                                                "le-set-event-mask", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_WRITE_LE_HOST_SUPPORTED,
                                                le_host_supported,
                                                sizeof(le_host_supported),
                                                "write-le-host-supported",
                                                0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_READ_LOCAL_EXT_FEATURES,
                                                ext_features_page0,
                                                sizeof(ext_features_page0),
                                                "read-local-ext-features",
                                                0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_READ_LOCAL_NAME,
                                                NULL, 0,
                                                "read-local-name", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_READ_ADV_TX_POWER,
                                                NULL, 0,
                                                "le-read-adv-tx-power", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_READ_DEF_DATA_LEN,
                                                NULL, 0,
                                                "le-read-def-data-len", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_READ_MAX_DATA_LEN,
                                                NULL, 0,
                                                "le-read-max-data-len", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_READ_TRANSMIT_POWER,
                                                NULL, 0,
                                                "le-read-transmit-power", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_SET_EVENT_MASK_PAGE_2,
                                                event_mask_page2,
                                                sizeof(event_mask_page2),
                                                "set-event-mask-page-2", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_RANDOM_ADDR,
                                                random_addr,
                                                sizeof(random_addr),
                                                "le-set-random-addr", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_PARAM,
                                                adv_param,
                                                sizeof(adv_param),
                                                "le-set-adv-param", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_DATA,
                                                adv_data,
                                                sizeof(adv_data),
                                                "le-set-adv-data", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_SCAN_RSP_DATA,
                                                scan_rsp_data,
                                                sizeof(scan_rsp_data),
                                                "le-set-scan-rsp-data", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_ENABLE,
                                                adv_enable,
                                                sizeof(adv_enable),
                                                "le-set-adv-enable", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_SCAN_PARAM,
                                                scan_param,
                                                sizeof(scan_param),
                                                "le-set-scan-param", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_SCAN_ENABLE,
                                                scan_enable,
                                                sizeof(scan_enable),
                                                "le-set-scan-enable", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_CLEAR_ACCEPT_LIST,
                                                NULL, 0,
                                                "le-clear-accept-list", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_ADD_TO_ACCEPT_LIST,
                                                accept_list_addr,
                                                sizeof(accept_list_addr),
                                                "le-add-to-accept-list", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_DEL_FROM_ACCEPT_LIST,
                                                accept_list_addr,
                                                sizeof(accept_list_addr),
                                                "le-del-from-accept-list", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_CLEAR_RESOLV_LIST,
                                                NULL, 0,
                                                "le-clear-resolv-list", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADDR_RESOLV_ENABLE,
                                                addr_resolv_enable,
                                                sizeof(addr_resolv_enable),
                                                "le-set-addr-resolv-enable",
                                                0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_RPA_TIMEOUT,
                                                rpa_timeout,
                                                sizeof(rpa_timeout),
                                                "le-set-rpa-timeout", 0);

  ret = close(user_fd);
  printf("bluez-hciraw: hci-close-user ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  failed |= bluez_hciraw_recv_monitor_ops(mon_fd, monitor_ops,
                                          sizeof(monitor_ops) /
                                          sizeof(monitor_ops[0]),
                                          "init") < 0;

  ret = close(mon_fd);
  printf("bluez-hciraw: hci-close-monitor ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-command-init-sequence-monitor complete\n");
    }

  return failed;
}

static int bluez_hciraw_user_advertise_enable(void)
{
  static const uint8_t adv_param[15] =
  {
    0xa0, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00
  };
  static const uint8_t adv_data[32] =
  {
    0
  };
  static const uint8_t adv_enable[1] =
  {
    0x01
  };
  int user_fd;
  int ret;
  int failed = 0;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-advertise-enable\n");

  user_fd = bluez_hciraw_open_hci(HCI_CHANNEL_USER, 0, "user");
  if (user_fd < 0)
    {
      return 1;
    }

  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_PARAM,
                                                adv_param,
                                                sizeof(adv_param),
                                                "le-set-adv-param", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_DATA,
                                                adv_data,
                                                sizeof(adv_data),
                                                "le-set-adv-data", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_ADV_ENABLE,
                                                adv_enable,
                                                sizeof(adv_enable),
                                                "le-set-adv-enable", 0);

  ret = close(user_fd);
  printf("bluez-hciraw: hci-close-user ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-advertise-enable complete\n");
    }

  return failed;
}

static int bluez_hciraw_user_scan_report(void)
{
  static const uint8_t scan_param[7] =
  {
    0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00
  };
  static const uint8_t scan_enable[2] =
  {
    0x01, 0x00
  };
  uint8_t event[260];
  uint8_t subevent = 0;
  uint8_t reports = 0;
  uint8_t adv_type = 0xff;
  uint8_t addr_type = 0xff;
  uint8_t data_len = 0;
  int user_fd;
  int ret;
  int failed = 0;
  ssize_t n;

  printf("bluez-hciraw: source=third/bluez/tools/hcitool style mode=user-scan-report\n");

  user_fd = bluez_hciraw_open_hci(HCI_CHANNEL_USER, 0, "user");
  if (user_fd < 0)
    {
      return 1;
    }

  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_SCAN_PARAM,
                                                scan_param,
                                                sizeof(scan_param),
                                                "le-set-scan-param", 0);
  failed |= bluez_hciraw_send_recv_params_on_fd(user_fd,
                                                HCI_OP_LE_SET_SCAN_ENABLE,
                                                scan_enable,
                                                sizeof(scan_enable),
                                                "le-set-scan-enable", 0);

  memset(event, 0, sizeof(event));
  n = recv(user_fd, event, sizeof(event), 0);
  if (n >= 16 && event[0] == HCI_EVENT_PKT && event[1] == HCI_EV_LE_META)
    {
      subevent = event[3];
      reports = event[4];
      adv_type = event[5];
      addr_type = event[6];
      data_len = event[13];
    }

  printf("bluez-hciraw: scan-report-recv ret=%ld errno=%d pkt=0x%02x event=0x%02x subevent=0x%02x reports=%u adv-type=%u addr-type=%u addr=%02x:%02x:%02x:%02x:%02x:%02x data-len=%u\n",
         (long)n, n < 0 ? errno : 0, event[0], event[1], subevent,
         reports, adv_type, addr_type, event[12], event[11], event[10],
         event[9], event[8], event[7], data_len);
  failed |= n < 16 || event[0] != HCI_EVENT_PKT ||
            event[1] != HCI_EV_LE_META ||
            subevent != HCI_EV_LE_ADVERTISING_REPORT ||
            reports != 1 ? 1 : 0;

  ret = close(user_fd);
  printf("bluez-hciraw: hci-close-user ret=%d errno=%d\n", ret,
         ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-hciraw: user-scan-report complete\n");
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
      bluez_hciraw_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "command"))
    {
      return bluez_hciraw_command();
    }

  if (!strcmp(argv[1], "user-command"))
    {
      return bluez_hciraw_user_command();
    }

  if (!strcmp(argv[1], "user-command-monitor"))
    {
      return bluez_hciraw_user_command_monitor();
    }

  if (!strcmp(argv[1], "user-command-sequence-monitor"))
    {
      return bluez_hciraw_user_command_sequence_monitor();
    }

  if (!strcmp(argv[1], "user-command-error-monitor"))
    {
      return bluez_hciraw_user_command_error_monitor();
    }

  if (!strcmp(argv[1], "user-command-init-sequence-monitor"))
    {
      return bluez_hciraw_user_command_init_sequence_monitor();
    }

  if (!strcmp(argv[1], "user-advertise-enable"))
    {
      return bluez_hciraw_user_advertise_enable();
    }

  if (!strcmp(argv[1], "user-scan-report"))
    {
      return bluez_hciraw_user_scan_report();
    }

  bluez_hciraw_usage();
  return 1;
}
