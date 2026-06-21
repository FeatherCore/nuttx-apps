/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_avctp_object_probe.c
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

#define avctp_register bluez_upstream_object_avctp_register
#define avctp_unregister bluez_upstream_object_avctp_unregister
#define avctp_connect bluez_upstream_object_avctp_connect
#define avctp_get bluez_upstream_object_avctp_get
#define avctp_is_initiator bluez_upstream_object_avctp_is_initiator
#define avctp_connect_browsing bluez_upstream_object_avctp_connect_browsing
#define avctp_disconnect bluez_upstream_object_avctp_disconnect
#define avctp_register_passthrough_handler bluez_upstream_object_avctp_register_passthrough_handler
#define avctp_unregister_passthrough_handler bluez_upstream_object_avctp_unregister_passthrough_handler
#define avctp_register_pdu_handler bluez_upstream_object_avctp_register_pdu_handler
#define avctp_unregister_pdu_handler bluez_upstream_object_avctp_unregister_pdu_handler
#define avctp_register_browsing_pdu_handler bluez_upstream_object_avctp_register_browsing_pdu_handler
#define avctp_unregister_browsing_pdu_handler bluez_upstream_object_avctp_unregister_browsing_pdu_handler
#define avctp_send_passthrough bluez_upstream_object_avctp_send_passthrough
#define avctp_send_release_passthrough bluez_upstream_object_avctp_send_release_passthrough
#define avctp_send_vendordep bluez_upstream_object_avctp_send_vendordep
#define avctp_send_vendordep_req bluez_upstream_object_avctp_send_vendordep_req
#define avctp_send_browsing_req bluez_upstream_object_avctp_send_browsing_req
#define avctp_supports_avc bluez_upstream_object_avctp_supports_avc
#define avctp_add_state_cb bluez_upstream_object_avctp_add_state_cb
#define avctp_remove_state_cb bluez_upstream_object_avctp_remove_state_cb

#define avrcp_handle_vendor_reject bluez_upstream_object_avrcp_handle_vendor_reject
#define avrcp_browsing_general_reject bluez_upstream_object_avrcp_browsing_general_reject

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "upstream/profiles/audio/avctp.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_avctp_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/avctp.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/avctp.c "
         "profile=AVCTP owner=bluetoothd\n",
         role);
}

int btd_cancel_authorization(unsigned int id)
{
	(void) id;
	return 0;
}

int cpu_to_be16(int value)
{
	unsigned int v = (unsigned int) value & 0xffffu;

	return (int) (((v & 0x00ffu) << 8) | ((v & 0xff00u) >> 8));
}
