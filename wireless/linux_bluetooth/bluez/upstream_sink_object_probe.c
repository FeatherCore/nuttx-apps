/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_sink_object_probe.c
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

#define sink_init bluez_upstream_object_sink_init
#define sink_unregister bluez_upstream_object_sink_unregister
#define sink_is_active bluez_upstream_object_sink_is_active
#define sink_connect bluez_upstream_object_sink_connect
#define sink_disconnect bluez_upstream_object_sink_disconnect
#define sink_setup_stream bluez_upstream_object_sink_setup_stream
#define sink_new_stream bluez_upstream_object_sink_new_stream
#define sink_add_state_cb bluez_upstream_object_sink_add_state_cb
#define sink_remove_state_cb bluez_upstream_object_sink_remove_state_cb

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

#include "upstream/profiles/audio/sink.c"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_sink_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/sink.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/sink.c "
         "profile=AudioSink owner=bluetoothd\n",
         role);
}
