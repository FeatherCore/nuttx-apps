/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/mgmt_main.c
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

#ifndef HCI_CHANNEL_CONTROL
#  define HCI_CHANNEL_CONTROL 3
#endif

#ifndef HCI_DEV_NONE
#  define HCI_DEV_NONE 0xffff
#endif

#ifndef MGMT_INDEX_NONE
#  define MGMT_INDEX_NONE 0xffff
#endif

#define MGMT_OP_READ_VERSION       0x0001
#define MGMT_OP_READ_COMMANDS      0x0002
#define MGMT_OP_READ_INDEX_LIST    0x0003
#define MGMT_OP_READ_INFO          0x0004
#define MGMT_OP_SET_POWERED        0x0005
#define MGMT_OP_SET_DISCOVERABLE   0x0006
#define MGMT_OP_SET_CONNECTABLE    0x0007
#define MGMT_OP_SET_BONDABLE       0x0009
#define MGMT_OP_SET_LE             0x000d
#define MGMT_OP_SET_ADVERTISING    0x0029
#define MGMT_OP_SET_BREDR          0x002a
#define MGMT_OP_START_DISCOVERY    0x0023
#define MGMT_OP_STOP_DISCOVERY     0x0024
#define MGMT_OP_DISCONNECT         0x0014
#define MGMT_OP_SET_IO_CAPABILITY  0x0018
#define MGMT_OP_PAIR_DEVICE        0x0019
#define MGMT_OP_CANCEL_PAIR_DEVICE 0x001a
#define MGMT_OP_UNPAIR_DEVICE      0x001b
#define MGMT_OP_USER_CONFIRM_REPLY 0x001c
#define MGMT_OP_USER_CONFIRM_NEG_REPLY 0x001d
#define MGMT_OP_USER_PASSKEY_REPLY 0x001e
#define MGMT_OP_USER_PASSKEY_NEG_REPLY 0x001f
#define MGMT_OP_GET_CONN_INFO      0x0031

#define MGMT_EV_CMD_COMPLETE       0x0001
#define MGMT_EV_CMD_STATUS         0x0002
#define MGMT_EV_NEW_SETTINGS       0x0006
#define MGMT_EV_DEVICE_CONNECTED   0x000b
#define MGMT_EV_DEVICE_DISCONNECTED 0x000c
#define MGMT_EV_DEVICE_UNPAIRED    0x0016
#define MGMT_EV_DISCOVERING        0x0013

#define MGMT_STATUS_SUCCESS        0x00
#define MGMT_STATUS_FAILED         0x03
#define MGMT_STATUS_INVALID_PARAMS 0x0d
#define MGMT_STATUS_CANCELLED      0x10

#define BDADDR_LE_PUBLIC           0x01
#define BLUEZ_IO_CAP_DISPLAY_YESNO 0x01
#define BLUEZ_IO_CAP_KEYBOARD_ONLY 0x02
#define BLUEZ_IO_CAP_NO_INPUT_OUTPUT 0x03
#define BLUEZ_PASSKEY_STAGED       123456

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_mgmt_hdr
{
  uint16_t opcode;
  uint16_t index;
  uint16_t len;
};

struct bluez_sockaddr_hci
{
  uint16_t hci_family;
  uint16_t hci_dev;
  uint16_t hci_channel;
};

struct bluez_mgmt_ev_cmd_complete
{
  uint16_t opcode;
  uint8_t status;
  uint8_t data[0];
};

struct bluez_mgmt_addr_info
{
  uint8_t bdaddr[6];
  uint8_t type;
};

struct bluez_mgmt_cp_pair_device
{
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap;
};

struct bluez_mgmt_cp_unpair_device
{
  struct bluez_mgmt_addr_info addr;
  uint8_t disconnect;
};

struct bluez_mgmt_cp_user_passkey_reply
{
  struct bluez_mgmt_addr_info addr;
  uint32_t passkey;
} __attribute__((packed));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_mgmt_usage(void)
{
  printf("usage: bluezmgmt control|pair-noio|user-confirm|user-confirm-neg|passkey|passkey-neg|cancel-pair|cancel-pair-pending|pair-unpair|lifecycle|reconnect-stress [rounds]|error-path\n");
  printf("\n");
  printf("BlueZ-style mgmt client over AF_BLUETOOTH/BTPROTO_HCI control socket.\n");
}

static int bluez_mgmt_open_control(void)
{
  struct bluez_sockaddr_hci addr;
  int fd;
  int ret;
  int saved_errno;

  fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  saved_errno = errno;
  printf("bluez-mgmt: hci-socket fd=%d errno=%d\n",
         fd, fd < 0 ? saved_errno : 0);
  if (fd < 0)
    {
      return -1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = HCI_DEV_NONE;
  addr.hci_channel = HCI_CHANNEL_CONTROL;

  ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  saved_errno = errno;
  printf("bluez-mgmt: hci-bind-control ret=%d errno=%d\n",
         ret, ret < 0 ? saved_errno : 0);
  if (ret < 0)
    {
      close(fd);
      return -1;
    }

  return fd;
}

static int bluez_mgmt_recv_one(int fd, uint16_t expect_opcode)
{
  uint8_t buf[320];
  struct bluez_mgmt_hdr *hdr = (struct bluez_mgmt_hdr *)buf;
  ssize_t ret;
  uint16_t event;
  uint16_t index;
  uint16_t len;
  uint16_t complete_opcode = 0;
  uint8_t status = 0xff;

  memset(buf, 0, sizeof(buf));
  ret = recv(fd, buf, sizeof(buf), 0);
  if (ret < 0)
    {
      printf("bluez-mgmt: recv ret=%ld errno=%d expect=0x%04x\n",
             (long)ret, errno, expect_opcode);
      return -1;
    }

  if ((size_t)ret < sizeof(*hdr))
    {
      printf("bluez-mgmt: recv ret=%ld short expect=0x%04x\n",
             (long)ret, expect_opcode);
      return -1;
    }

  event = hdr->opcode;
  index = hdr->index;
  len = hdr->len;
  if (event == MGMT_EV_CMD_COMPLETE &&
      ret >= (ssize_t)(sizeof(*hdr) + sizeof(struct bluez_mgmt_ev_cmd_complete)))
    {
      struct bluez_mgmt_ev_cmd_complete *cc =
        (struct bluez_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

      complete_opcode = cc->opcode;
      status = cc->status;
    }
  else if (event == MGMT_EV_CMD_STATUS && ret >= (ssize_t)(sizeof(*hdr) + 3))
    {
      complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                        ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
      status = buf[sizeof(*hdr) + 2];
    }

  printf("bluez-mgmt: recv ret=%ld event=0x%04x index=0x%04x len=%u "
         "opcode=0x%04x status=0x%02x expect=0x%04x\n",
         (long)ret, event, index, len, complete_opcode, status,
         expect_opcode);
  return 0;
}

static int bluez_mgmt_recv_status(int fd, uint16_t expect_opcode,
                                  uint8_t expect_status)
{
  uint8_t buf[320];
  struct bluez_mgmt_hdr *hdr = (struct bluez_mgmt_hdr *)buf;
  ssize_t ret;
  uint16_t event;
  uint16_t index;
  uint16_t len;
  uint16_t complete_opcode = 0;
  uint8_t status = 0xff;

  memset(buf, 0, sizeof(buf));
  ret = recv(fd, buf, sizeof(buf), 0);
  if (ret < 0)
    {
      printf("bluez-mgmt: recv-status ret=%ld errno=%d expect=0x%04x status=0x%02x\n",
             (long)ret, errno, expect_opcode, expect_status);
      return -1;
    }

  if ((size_t)ret < sizeof(*hdr))
    {
      printf("bluez-mgmt: recv-status ret=%ld short expect=0x%04x status=0x%02x\n",
             (long)ret, expect_opcode, expect_status);
      return -1;
    }

  event = hdr->opcode;
  index = hdr->index;
  len = hdr->len;
  if (event == MGMT_EV_CMD_COMPLETE &&
      ret >= (ssize_t)(sizeof(*hdr) + sizeof(struct bluez_mgmt_ev_cmd_complete)))
    {
      struct bluez_mgmt_ev_cmd_complete *cc =
        (struct bluez_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

      complete_opcode = cc->opcode;
      status = cc->status;
    }
  else if (event == MGMT_EV_CMD_STATUS && ret >= (ssize_t)(sizeof(*hdr) + 3))
    {
      complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                        ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
      status = buf[sizeof(*hdr) + 2];
    }

  printf("bluez-mgmt: recv-status ret=%ld event=0x%04x index=0x%04x len=%u "
         "opcode=0x%04x status=0x%02x expect=0x%04x expect-status=0x%02x\n",
         (long)ret, event, index, len, complete_opcode, status,
         expect_opcode, expect_status);

  return (event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
         complete_opcode == expect_opcode &&
         status == expect_status ? 0 : -1;
}

static int bluez_mgmt_recv_until(int fd, uint16_t expect_event,
                                 uint16_t expect_opcode,
                                 const char *label)
{
  uint8_t buf[320];
  bool saw_event = false;
  bool saw_complete = false;
  unsigned int i;

  for (i = 0; i < 8 && (!saw_event || !saw_complete); i++)
    {
      struct bluez_mgmt_hdr *hdr = (struct bluez_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;
      uint16_t complete_opcode = 0;
      uint8_t status = 0xff;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          printf("bluez-mgmt: recv-until ret=%ld errno=%d label=%s\n",
                 (long)ret, errno, label);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-mgmt: recv-until ret=%ld short label=%s\n",
                 (long)ret, label);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      if (event == MGMT_EV_CMD_COMPLETE &&
          ret >= (ssize_t)(sizeof(*hdr) +
                           sizeof(struct bluez_mgmt_ev_cmd_complete)))
        {
          struct bluez_mgmt_ev_cmd_complete *cc =
            (struct bluez_mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));

          complete_opcode = cc->opcode;
          status = cc->status;
        }
      else if (event == MGMT_EV_CMD_STATUS &&
               ret >= (ssize_t)(sizeof(*hdr) + 3))
        {
          complete_opcode = (uint16_t)buf[sizeof(*hdr)] |
                            ((uint16_t)buf[sizeof(*hdr) + 1] << 8);
          status = buf[sizeof(*hdr) + 2];
        }

      if (event == expect_event)
        {
          saw_event = true;
        }

      if ((event == MGMT_EV_CMD_COMPLETE || event == MGMT_EV_CMD_STATUS) &&
          complete_opcode == expect_opcode && status == MGMT_STATUS_SUCCESS)
        {
          saw_complete = true;
        }

      printf("bluez-mgmt: recv-until label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u opcode=0x%04x status=0x%02x "
             "expect-event=0x%04x expect-opcode=0x%04x "
             "saw-event=%u saw-complete=%u\n",
             label, (long)ret, event, index, len, complete_opcode, status,
             expect_event, expect_opcode, saw_event ? 1 : 0,
             saw_complete ? 1 : 0);
    }

  return saw_event && saw_complete ? 0 : -1;
}

static int bluez_mgmt_recv_event(int fd, uint16_t expect_event,
                                 const char *label)
{
  uint8_t buf[320];
  unsigned int i;

  for (i = 0; i < 32; i++)
    {
      struct bluez_mgmt_hdr *hdr = (struct bluez_mgmt_hdr *)buf;
      ssize_t ret;
      uint16_t event;
      uint16_t index;
      uint16_t len;

      memset(buf, 0, sizeof(buf));
      ret = recv(fd, buf, sizeof(buf), 0);
      if (ret < 0)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
              usleep(20000);
              continue;
            }

          printf("bluez-mgmt: recv-event ret=%ld errno=%d label=%s\n",
                 (long)ret, errno, label);
          return -1;
        }

      if ((size_t)ret < sizeof(*hdr))
        {
          printf("bluez-mgmt: recv-event ret=%ld short label=%s\n",
                 (long)ret, label);
          return -1;
        }

      event = hdr->opcode;
      index = hdr->index;
      len = hdr->len;
      printf("bluez-mgmt: recv-event label=%s ret=%ld event=0x%04x "
             "index=0x%04x len=%u expect-event=0x%04x\n",
             label, (long)ret, event, index, len, expect_event);
      if (event == expect_event)
        {
          return 0;
        }
    }

  printf("bluez-mgmt: recv-event missing label=%s expect-event=0x%04x\n",
         label, expect_event);
  return -1;
}

static int bluez_mgmt_send_cmd(int fd, uint16_t opcode, uint16_t index,
                               const void *data, uint16_t data_len,
                               unsigned int reads)
{
  uint8_t buf[64];
  struct bluez_mgmt_hdr *hdr = (struct bluez_mgmt_hdr *)buf;
  ssize_t ret;
  unsigned int i;

  if (sizeof(*hdr) + data_len > sizeof(buf))
    {
      return -1;
    }

  memset(buf, 0, sizeof(buf));
  hdr->opcode = opcode;
  hdr->index = index;
  hdr->len = data_len;
  if (data_len > 0)
    {
      memcpy(buf + sizeof(*hdr), data, data_len);
    }

  ret = send(fd, buf, sizeof(*hdr) + data_len, 0);
  printf("bluez-mgmt: send opcode=0x%04x index=0x%04x len=%u ret=%ld errno=%d\n",
         opcode, index, data_len, (long)ret, ret < 0 ? errno : 0);
  if (ret < 0)
    {
      return -1;
    }

  for (i = 0; i < reads; i++)
    {
      if (bluez_mgmt_recv_one(fd, opcode) < 0)
        {
          return -1;
        }
    }

  return 0;
}

static int bluez_mgmt_send_setting(int fd, uint16_t opcode, uint8_t value)
{
  return bluez_mgmt_send_cmd(fd, opcode, 0, &value, sizeof(value), 2);
}

static int bluez_mgmt_control(void)
{
  uint8_t discoverable[3] = { 1, 0, 0 };
  uint8_t discovery[1] = { 1 };
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=control\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_READ_VERSION,
                                MGMT_INDEX_NONE, NULL, 0, 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_READ_COMMANDS,
                                MGMT_INDEX_NONE, NULL, 0, 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_READ_INDEX_LIST,
                                MGMT_INDEX_NONE, NULL, 0, 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_READ_INFO, 0, NULL, 0, 1) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_POWERED, 1) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_CONNECTABLE, 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_DISCOVERABLE, 0,
                                discoverable, sizeof(discoverable), 2) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_BONDABLE, 1) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_LE, 1) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_ADVERTISING, 1) < 0;
  failed |= bluez_mgmt_send_setting(fd, MGMT_OP_SET_BREDR, 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_START_DISCOVERY, 0,
                                discovery, sizeof(discovery), 2) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_STOP_DISCOVERY, 0,
                                discovery, sizeof(discovery), 2) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: control complete\n");
    }

  return failed;
}

static int bluez_mgmt_pair_noio(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=pair-noio\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&pair, 0, sizeof(pair));
  pair.addr.bdaddr[0] = 1;
  pair.addr.type = BDADDR_LE_PUBLIC;
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                &pair, sizeof(pair), 0) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE,
                                  "pair-noio-connected") < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: pair-noio complete\n");
    }

  return failed;
}

static int bluez_mgmt_cancel_pair(void)
{
  struct bluez_mgmt_addr_info addr;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=cancel-pair\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_CANCEL_PAIR_DEVICE, 0,
                            &addr, sizeof(addr), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_CANCEL_PAIR_DEVICE,
                                   MGMT_STATUS_INVALID_PARAMS) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: cancel-pair complete\n");
    }

  return failed;
}

static int bluez_mgmt_cancel_pair_pending(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=cancel-pair-pending\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                            &pair, sizeof(pair), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_one(fd, MGMT_OP_PAIR_DEVICE) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_CANCEL_PAIR_DEVICE, 0,
                            &addr, sizeof(addr), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                   MGMT_STATUS_CANCELLED) < 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_CANCEL_PAIR_DEVICE,
                                   MGMT_STATUS_SUCCESS) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: cancel-pair-pending complete\n");
    }

  return failed;
}

static int bluez_mgmt_user_confirm(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=user-confirm\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                            &pair, sizeof(pair), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_one(fd, MGMT_OP_PAIR_DEVICE) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_USER_CONFIRM_REPLY, 0,
                            &addr, sizeof(addr), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_USER_CONFIRM_REPLY,
                                   MGMT_STATUS_SUCCESS) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE,
                                  "user-confirm-connected") < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: user-confirm complete\n");
    }

  return failed;
}

static int bluez_mgmt_user_confirm_neg(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_DISPLAY_YESNO;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=user-confirm-neg\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                            &pair, sizeof(pair), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_one(fd, MGMT_OP_PAIR_DEVICE) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_USER_CONFIRM_NEG_REPLY, 0,
                            &addr, sizeof(addr), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_USER_CONFIRM_NEG_REPLY,
                                   MGMT_STATUS_SUCCESS) < 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                   MGMT_STATUS_FAILED) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: user-confirm-neg complete\n");
    }

  return failed;
}

static int bluez_mgmt_passkey(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_cp_user_passkey_reply passkey;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_KEYBOARD_ONLY;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=passkey\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  memset(&passkey, 0, sizeof(passkey));
  memcpy(&passkey.addr, &addr, sizeof(passkey.addr));
  passkey.passkey = BLUEZ_PASSKEY_STAGED;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                            &pair, sizeof(pair), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_one(fd, MGMT_OP_PAIR_DEVICE) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_USER_PASSKEY_REPLY, 0,
                            &passkey, sizeof(passkey), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_USER_PASSKEY_REPLY,
                                   MGMT_STATUS_SUCCESS) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE,
                                  "passkey-connected") < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: passkey complete\n");
    }

  return failed;
}

static int bluez_mgmt_passkey_neg(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_KEYBOARD_ONLY;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=passkey-neg\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                            &pair, sizeof(pair), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_one(fd, MGMT_OP_PAIR_DEVICE) < 0;

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_USER_PASSKEY_NEG_REPLY, 0,
                            &addr, sizeof(addr), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_USER_PASSKEY_NEG_REPLY,
                                   MGMT_STATUS_SUCCESS) < 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_PAIR_DEVICE,
                                   MGMT_STATUS_FAILED) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: passkey-neg complete\n");
    }

  return failed;
}

static int bluez_mgmt_lifecycle(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=lifecycle\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                &pair, sizeof(pair), 0) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE,
                                  "pair-connected") < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_GET_CONN_INFO, 0,
                                &addr, sizeof(addr), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_DISCONNECT, 0,
                                &addr, sizeof(addr), 0) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_DISCONNECTED,
                                  MGMT_OP_DISCONNECT,
                                  "disconnect") < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: lifecycle complete\n");
    }

  return failed;
}

static int bluez_mgmt_pair_unpair(void)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_cp_unpair_device unpair;
  struct bluez_mgmt_addr_info addr;
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int fd;
  int observer_fd = -1;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=pair-unpair\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  observer_fd = bluez_mgmt_open_control();
  failed |= observer_fd < 0 ? 1 : 0;

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  memset(&unpair, 0, sizeof(unpair));
  memcpy(&unpair.addr, &addr, sizeof(unpair.addr));
  unpair.disconnect = 0;

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                &pair, sizeof(pair), 0) < 0;
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE,
                                  "pair-unpair-connected") < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_UNPAIR_DEVICE, 0,
                                &unpair, sizeof(unpair), 0) < 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_UNPAIR_DEVICE,
                                   MGMT_STATUS_SUCCESS) < 0;
  if (observer_fd >= 0)
    {
      failed |= bluez_mgmt_recv_event(observer_fd,
                                      MGMT_EV_DEVICE_UNPAIRED,
                                      "unpair-observer") < 0;
    }

  if (observer_fd >= 0)
    {
      ret = close(observer_fd);
      printf("bluez-mgmt: hci-close-observer ret=%d errno=%d\n",
             ret, ret < 0 ? errno : 0);
      failed |= ret < 0 ? 1 : 0;
    }

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: pair-unpair complete\n");
    }

  return failed;
}

static int bluez_mgmt_reconnect_round(int fd, unsigned int round)
{
  struct bluez_mgmt_cp_pair_device pair;
  struct bluez_mgmt_addr_info addr;
  char label[32];
  uint8_t io_cap = BLUEZ_IO_CAP_NO_INPUT_OUTPUT;
  int failed = 0;

  memset(&addr, 0, sizeof(addr));
  addr.bdaddr[0] = 1;
  addr.type = BDADDR_LE_PUBLIC;

  memset(&pair, 0, sizeof(pair));
  memcpy(&pair.addr, &addr, sizeof(pair.addr));
  pair.io_cap = io_cap;

  printf("bluez-mgmt: reconnect round=%u begin\n", round);

  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                                &io_cap, sizeof(io_cap), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_PAIR_DEVICE, 0,
                                &pair, sizeof(pair), 0) < 0;
  snprintf(label, sizeof(label), "round%u-connected", round);
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_CONNECTED,
                                  MGMT_OP_PAIR_DEVICE, label) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_GET_CONN_INFO, 0,
                                &addr, sizeof(addr), 1) < 0;
  failed |= bluez_mgmt_send_cmd(fd, MGMT_OP_DISCONNECT, 0,
                                &addr, sizeof(addr), 0) < 0;
  snprintf(label, sizeof(label), "round%u-disconnected", round);
  failed |= bluez_mgmt_recv_until(fd, MGMT_EV_DEVICE_DISCONNECTED,
                                  MGMT_OP_DISCONNECT, label) < 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: reconnect round=%u complete\n", round);
    }

  return failed;
}

static int bluez_mgmt_reconnect_stress(unsigned int rounds)
{
  int fd;
  int ret;
  int failed = 0;
  unsigned int round;

  if (rounds == 0)
    {
      rounds = 3;
    }

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=reconnect-stress rounds=%u\n",
         rounds);

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  for (round = 1; round <= rounds; round++)
    {
      failed |= bluez_mgmt_reconnect_round(fd, round) != 0;
    }

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: reconnect-stress complete rounds=%u\n", rounds);
    }

  return failed;
}

static int bluez_mgmt_error_path(void)
{
  uint8_t invalid_io_cap = 0xff;
  int fd;
  int ret;
  int failed = 0;

  printf("bluez-mgmt: source=third/bluez/tools/btmgmt style mode=error-path\n");

  fd = bluez_mgmt_open_control();
  if (fd < 0)
    {
      return 1;
    }

  ret = bluez_mgmt_send_cmd(fd, MGMT_OP_SET_IO_CAPABILITY, 0,
                            &invalid_io_cap, sizeof(invalid_io_cap), 0);
  failed |= ret < 0 ? 1 : 0;
  failed |= bluez_mgmt_recv_status(fd, MGMT_OP_SET_IO_CAPABILITY,
                                   MGMT_STATUS_INVALID_PARAMS) < 0;

  ret = close(fd);
  printf("bluez-mgmt: hci-close ret=%d errno=%d\n",
         ret, ret < 0 ? errno : 0);
  failed |= ret < 0 ? 1 : 0;

  if (failed == 0)
    {
      printf("bluez-mgmt: error-path complete\n");
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
      bluez_mgmt_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "control"))
    {
      return bluez_mgmt_control();
    }

  if (!strcmp(argv[1], "pair-noio"))
    {
      return bluez_mgmt_pair_noio();
    }

  if (!strcmp(argv[1], "cancel-pair"))
    {
      return bluez_mgmt_cancel_pair();
    }

  if (!strcmp(argv[1], "user-confirm"))
    {
      return bluez_mgmt_user_confirm();
    }

  if (!strcmp(argv[1], "user-confirm-neg"))
    {
      return bluez_mgmt_user_confirm_neg();
    }

  if (!strcmp(argv[1], "passkey"))
    {
      return bluez_mgmt_passkey();
    }

  if (!strcmp(argv[1], "passkey-neg"))
    {
      return bluez_mgmt_passkey_neg();
    }

  if (!strcmp(argv[1], "cancel-pair-pending"))
    {
      return bluez_mgmt_cancel_pair_pending();
    }

  if (!strcmp(argv[1], "lifecycle"))
    {
      return bluez_mgmt_lifecycle();
    }

  if (!strcmp(argv[1], "pair-unpair"))
    {
      return bluez_mgmt_pair_unpair();
    }

  if (!strcmp(argv[1], "reconnect-stress"))
    {
      unsigned long rounds = 3;

      if (argc >= 3)
        {
          rounds = strtoul(argv[2], NULL, 0);
        }

      return bluez_mgmt_reconnect_stress((unsigned int)rounds);
    }

  if (!strcmp(argv[1], "error-path"))
    {
      return bluez_mgmt_error_path();
    }

  bluez_mgmt_usage();
  return 1;
}
