/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_io_mainloop_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/uio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_UPSTREAM_OBJECT_SHIM_IO_H

#define io_new bluez_upstream_object_io_new
#define io_destroy bluez_upstream_object_io_destroy
#define io_get_fd bluez_upstream_object_io_get_fd
#define io_set_close_on_destroy bluez_upstream_object_io_set_close_on_destroy
#define io_set_ignore_errqueue bluez_upstream_object_io_set_ignore_errqueue
#define io_set_read_handler bluez_upstream_object_io_set_read_handler
#define io_set_write_handler bluez_upstream_object_io_set_write_handler
#define io_set_disconnect_handler bluez_upstream_object_io_set_disconnect_handler
#define io_send bluez_upstream_object_io_send
#define io_shutdown bluez_upstream_object_io_shutdown
#define io_glib_add_err_watch bluez_upstream_object_io_glib_add_err_watch

#define mainloop_add_fd bluez_upstream_object_mainloop_add_fd
#define mainloop_modify_fd bluez_upstream_object_mainloop_modify_fd
#define mainloop_remove_fd bluez_upstream_object_mainloop_remove_fd

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef void (*io_destroy_func_t)(void *data);
struct io;
typedef bool (*io_callback_func_t)(struct io *io, void *user_data);
typedef void (*io_glib_err_func_t)(int cond, void *user_data);

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "upstream/src/shared/io-mainloop.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_io_mainloop_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/shared/io-mainloop.c role=%s linked=1 "
         "source=third/bluez/src/shared/io-mainloop.c "
         "owner=bluetoothd api=io-mainloop\n",
         role);
}
