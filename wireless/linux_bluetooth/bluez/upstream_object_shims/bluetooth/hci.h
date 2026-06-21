#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_H

#include <stdint.h>
#include <sys/socket.h>

#define HCI_BLOCKED_KEY_TYPE_LINKKEY 0x00
#define HCI_BLOCKED_KEY_TYPE_LTK     0x01
#define HCI_BLOCKED_KEY_TYPE_IRK     0x02

#define HCI_OE_USER_ENDED_CONNECTION 0x13

#ifndef HCI_MAX_ACL_SIZE
#  define HCI_MAX_ACL_SIZE 1024
#endif

#ifndef HCI_MAX_EIR_LENGTH
#  define HCI_MAX_EIR_LENGTH 240
#endif

#ifndef HCI_DEV_NONE
#  define HCI_DEV_NONE 0xffff
#endif

#ifndef HCI_CHANNEL_CONTROL
#  define HCI_CHANNEL_CONTROL 3
#endif

struct sockaddr_hci
{
  sa_family_t hci_family;
  unsigned short hci_dev;
  unsigned short hci_channel;
};

#endif /* BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_H */
