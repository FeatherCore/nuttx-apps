#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_H
#include <stdint.h>
#include <sys/socket.h>
typedef struct { uint8_t b[6]; } bdaddr_t;

#define BDADDR_BREDR 0
#define BTPROTO_L2CAP 0
#ifndef BTPROTO_HCI
#  define BTPROTO_HCI 1
#endif

#ifndef AF_BLUETOOTH
#  define AF_BLUETOOTH 31
#endif

#ifndef PF_BLUETOOTH
#  define PF_BLUETOOTH AF_BLUETOOTH
#endif

#ifndef SOL_BLUETOOTH
#  define SOL_BLUETOOTH 274
#endif

#ifndef BT_SNDMTU
#  define BT_SNDMTU 12
#endif

#ifndef SOCK_CLOEXEC
#  define SOCK_CLOEXEC 0
#endif

#ifndef SOCK_NONBLOCK
#  define SOCK_NONBLOCK 0
#endif

#ifndef SOCK_RAW
#  define SOCK_RAW 3
#endif

static inline uint16_t htobs(uint16_t value)
{
  return value;
}

static inline uint16_t btohs(uint16_t value)
{
  return value;
}

static inline uint32_t htobl(uint32_t value)
{
  return value;
}

static inline uint32_t btohl(uint32_t value)
{
  return value;
}

static inline int ba2str(const bdaddr_t *ba, char *str)
{
  (void)ba;
  if (str != NULL)
    {
      str[0] = '0';
      str[1] = '\0';
    }

  return 0;
}
#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_BLUETOOTH_PROFILE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_BLUETOOTH_PROFILE_COMPAT
#ifndef BTPROTO_RFCOMM
#define BTPROTO_RFCOMM 3
#endif
#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_BLUETOOTH_DEVICE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_BLUETOOTH_DEVICE_COMPAT
#define BDADDR_LE_PUBLIC 0x01
#define BDADDR_LE_RANDOM 0x02
#define HCI_MAX_NAME_LENGTH 248
static const bdaddr_t bluez_upstream_object_bdaddr_any = {{0, 0, 0, 0, 0, 0}};
#ifndef BDADDR_ANY
#define BDADDR_ANY (&bluez_upstream_object_bdaddr_any)
#endif
#endif
