#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_SRC_SHARED_MAINLOOP_H
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_SRC_SHARED_MAINLOOP_H

#include <stdint.h>

typedef void (*mainloop_event_func)(int fd, uint32_t events,
                                    void *user_data);
typedef void (*mainloop_timeout_func)(int id, void *user_data);
typedef void (*mainloop_destroy_func)(void *user_data);

void mainloop_init(void);
void mainloop_quit(void);
void mainloop_exit_success(void);
void mainloop_exit_failure(void);
int mainloop_run(void);
int mainloop_add_fd(int fd, uint32_t events, mainloop_event_func callback,
                    void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_fd(int fd, uint32_t events);
int mainloop_remove_fd(int fd);
int mainloop_add_timeout(unsigned int msec, mainloop_timeout_func callback,
                         void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_timeout(int id, unsigned int msec);
int mainloop_remove_timeout(int id);
int mainloop_sd_notify(const char *state);

#endif
