/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_source_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef __LITTLE_ENDIAN
#  define __LITTLE_ENDIAN 1234
#endif

#ifndef __BIG_ENDIAN
#  define __BIG_ENDIAN 4321
#endif

#ifndef __BYTE_ORDER
#  define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#define source_init bluez_upstream_object_source_init
#define source_unregister bluez_upstream_object_source_unregister
#define source_connect bluez_upstream_object_source_connect
#define source_disconnect bluez_upstream_object_source_disconnect
#define source_setup_stream bluez_upstream_object_source_setup_stream
#define source_new_stream bluez_upstream_object_source_new_stream
#define source_add_state_cb bluez_upstream_object_source_add_state_cb
#define source_remove_state_cb bluez_upstream_object_source_remove_state_cb

#define a2dp_cancel bluez_upstream_object_a2dp_cancel
#define a2dp_config bluez_upstream_object_a2dp_config
#define a2dp_discover bluez_upstream_object_a2dp_discover
#define a2dp_select_capabilities bluez_upstream_object_a2dp_select_capabilities
#define a2dp_avdtp_get bluez_upstream_object_a2dp_avdtp_get

#define avdtp_ref bluez_upstream_object_avdtp_ref
#define avdtp_unref bluez_upstream_object_avdtp_unref
#define avdtp_stream_add_cb bluez_upstream_object_avdtp_stream_add_cb
#define avdtp_stream_remove_cb bluez_upstream_object_avdtp_stream_remove_cb

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#include "upstream/profiles/audio/source.c"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_source_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/source.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/source.c "
         "profile=AudioSource owner=bluetoothd\n",
         role);
}
