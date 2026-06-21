/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_mainloop_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define mainloop_init bluez_upstream_object_mainloop_init
#define mainloop_quit bluez_upstream_object_mainloop_quit
#define mainloop_exit_success bluez_upstream_object_mainloop_exit_success
#define mainloop_exit_failure bluez_upstream_object_mainloop_exit_failure
#define mainloop_run bluez_upstream_object_mainloop_run
#define mainloop_add_fd bluez_upstream_object_mainloop_add_fd
#define mainloop_modify_fd bluez_upstream_object_mainloop_modify_fd
#define mainloop_remove_fd bluez_upstream_object_mainloop_remove_fd
#define mainloop_add_timeout bluez_upstream_object_mainloop_add_timeout
#define mainloop_modify_timeout bluez_upstream_object_mainloop_modify_timeout
#define mainloop_remove_timeout bluez_upstream_object_mainloop_remove_timeout
#define mainloop_sd_notify bluez_upstream_object_mainloop_sd_notify
#define mainloop_notify_exit bluez_upstream_object_mainloop_notify_exit

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "upstream/src/shared/mainloop.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int bluez_upstream_object_mainloop_sd_notify(const char *state)
{
  (void)state;
  return 0;
}

void bluez_upstream_object_mainloop_notify_exit(void)
{
}

void bluez_upstream_mainloop_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/shared/mainloop.c role=%s linked=1 "
         "source=third/bluez/src/shared/mainloop.c "
         "owner=bluetoothd api=mainloop\n",
         role);
}
