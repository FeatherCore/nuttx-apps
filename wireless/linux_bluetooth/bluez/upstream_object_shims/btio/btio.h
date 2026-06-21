#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_BTIO_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_BTIO_H

#include <stdarg.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include "glib.h"

typedef enum
{
  BT_IO_MODE_BASIC = 0,
  BT_IO_MODE_ERTM = 1,
  BT_IO_MODE_STREAMING = 2
} BtIOMode;

typedef enum
{
  BT_IO_SEC_LOW = 0,
  BT_IO_SEC_MEDIUM = 1,
  BT_IO_SEC_HIGH = 2
} BtIOSecLevel;

#define BT_IO_OPT_INVALID       0
#define BT_IO_OPT_SOURCE_BDADDR 1
#define BT_IO_OPT_DEST_BDADDR   2
#define BT_IO_OPT_PSM           3
#define BT_IO_OPT_MODE          4
#define BT_IO_OPT_SEC_LEVEL     5
#define BT_IO_OPT_IMTU          6
#define BT_IO_OPT_OMTU          7
#define BT_IO_OPT_FLUSHABLE     8
#define BT_IO_OPT_PHY           9
#define BT_IO_OPT_DEST          10
#define BT_IO_OPT_CENTRAL       11

static int bt_io_shim_last_peer_fd = -1;

static inline gboolean bt_io_set(GIOChannel *io, GError **err, ...)
{
  (void)io;
  if (err != NULL)
    {
      *err = NULL;
    }

  return TRUE;
}

static inline int bt_io_shim_take_last_peer_fd(void)
{
  int fd = bt_io_shim_last_peer_fd;

  bt_io_shim_last_peer_fd = -1;
  return fd;
}

static inline void bt_io_shim_set_last_peer_fd(int fd)
{
  int old = bt_io_shim_take_last_peer_fd();

  if (old >= 0)
    {
      close(old);
    }

  bt_io_shim_last_peer_fd = fd;
}

static inline GIOChannel *bt_io_connect(GIOFunc func, gpointer user_data,
                                        GDestroyNotify destroy,
                                        GError **err, ...)
{
  int fds[2];
  GIOChannel *channel;

  (void)func;
  (void)user_data;
  (void)destroy;
  if (err != NULL)
    {
      *err = NULL;
    }

  if (bt_io_shim_last_peer_fd >= 0)
    {
      close(bt_io_shim_last_peer_fd);
      bt_io_shim_last_peer_fd = -1;
    }

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
    {
      return NULL;
    }

  channel = g_io_channel_unix_new(fds[0]);
  bt_io_shim_last_peer_fd = fds[1];
  return channel;
}

static inline gboolean bt_io_get(GIOChannel *io, GError **err, ...)
{
  va_list ap;
  int opt;

  (void)io;
  if (err != NULL)
    {
      *err = NULL;
    }

  va_start(ap, err);
  while ((opt = va_arg(ap, int)) != BT_IO_OPT_INVALID)
    {
      void *ptr = va_arg(ap, void *);

      if (ptr == NULL)
        {
          continue;
        }

      switch (opt)
        {
          case BT_IO_OPT_IMTU:
          case BT_IO_OPT_OMTU:
            *(uint16_t *)ptr = 672;
            break;

          case BT_IO_OPT_PHY:
            *(uint32_t *)ptr = 0;
            break;

          default:
            break;
        }
    }
  va_end(ap);

  return TRUE;
}

static inline gboolean bt_io_accept(GIOChannel *io, GIOFunc func,
                                    gpointer user_data,
                                    GDestroyNotify destroy,
                                    GError **err)
{
  (void)io;
  (void)func;
  (void)user_data;
  (void)destroy;
  if (err != NULL)
    {
      *err = NULL;
    }

  return TRUE;
}

static inline GIOChannel *bt_io_listen(GIOFunc connect, GIOFunc confirm,
                                       gpointer user_data,
                                       GDestroyNotify destroy,
                                       GError **err, ...)
{
  (void)connect;
  (void)confirm;
  (void)user_data;
  (void)destroy;
  if (err != NULL)
    {
      *err = NULL;
    }

  return g_new0(GIOChannel, 1);
}
#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_BTIO_PROFILE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_BTIO_PROFILE_COMPAT

typedef void (*BtIOConfirm)(GIOChannel *io, gpointer user_data);
typedef void (*BtIOConnect)(GIOChannel *io, GError *err, gpointer user_data);

#ifndef BT_IO_OPT_CHANNEL
#define BT_IO_OPT_CHANNEL 12
#endif

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_BTIO_DEVICE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_BTIO_DEVICE_COMPAT
#define BT_IO_OPT_CID 13
#define BT_IO_OPT_SOURCE_TYPE 14
#define BT_IO_OPT_DEST_TYPE 15
#define BT_IO_ERROR 1
#endif
