#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_IO_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_IO_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/uio.h>

typedef void (*io_destroy_func_t)(void *data);

struct io;

static inline struct io *io_new(int fd)
{
  (void)fd;
  return NULL;
}

static inline void io_destroy(struct io *io)
{
  (void)io;
}

static inline int io_get_fd(struct io *io)
{
  (void)io;
  return -1;
}

static inline bool io_set_close_on_destroy(struct io *io, bool do_close)
{
  (void)io;
  (void)do_close;
  return true;
}

static inline bool io_set_ignore_errqueue(struct io *io, bool do_ignore)
{
  (void)io;
  (void)do_ignore;
  return true;
}

static inline ssize_t io_send(struct io *io, const struct iovec *iov,
                              int iovcnt)
{
  (void)io;
  (void)iov;
  (void)iovcnt;
  return -1;
}

static inline bool io_shutdown(struct io *io)
{
  (void)io;
  return true;
}

typedef bool (*io_callback_func_t)(struct io *io, void *user_data);

static inline bool io_set_read_handler(struct io *io,
                                       io_callback_func_t callback,
                                       void *user_data,
                                       io_destroy_func_t destroy)
{
  (void)io;
  (void)callback;
  (void)user_data;
  (void)destroy;
  return true;
}

static inline bool io_set_write_handler(struct io *io,
                                        io_callback_func_t callback,
                                        void *user_data,
                                        io_destroy_func_t destroy)
{
  (void)io;
  (void)callback;
  (void)user_data;
  (void)destroy;
  return true;
}

static inline bool io_set_disconnect_handler(struct io *io,
                                             io_callback_func_t callback,
                                             void *user_data,
                                             io_destroy_func_t destroy)
{
  (void)io;
  (void)callback;
  (void)user_data;
  (void)destroy;
  return true;
}

typedef void (*io_glib_err_func_t)(int cond, void *user_data);

static inline unsigned int io_glib_add_err_watch(void *giochannel,
                                                 io_glib_err_func_t func,
                                                 void *user_data)
{
  (void)giochannel;
  (void)func;
  (void)user_data;
  return 1;
}

#endif
