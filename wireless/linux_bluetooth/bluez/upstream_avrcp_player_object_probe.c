/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_avrcp_player_object_probe.c
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

#define avrcp_player_init bluez_upstream_object_avrcp_player_init
#define avrcp_player_exit bluez_upstream_object_avrcp_player_exit

#define avrcp_register_player bluez_upstream_object_avrcp_register_player
#define avrcp_unregister_player bluez_upstream_object_avrcp_unregister_player
#define avrcp_player_event bluez_upstream_object_avrcp_player_event

#define local_player_register_callbacks bluez_upstream_object_local_player_register_callbacks
#define local_player_unregister_callbacks bluez_upstream_object_local_player_unregister_callbacks
#define local_player_get_adapter bluez_upstream_object_local_player_get_adapter
#define local_player_list_settings bluez_upstream_object_local_player_list_settings
#define local_player_get_setting bluez_upstream_object_local_player_get_setting
#define local_player_set_setting bluez_upstream_object_local_player_set_setting
#define local_player_get_metadata bluez_upstream_object_local_player_get_metadata
#define local_player_list_metadata bluez_upstream_object_local_player_list_metadata
#define local_player_get_status bluez_upstream_object_local_player_get_status
#define local_player_get_position bluez_upstream_object_local_player_get_position
#define local_player_get_duration bluez_upstream_object_local_player_get_duration
#define local_player_get_player_name bluez_upstream_object_local_player_get_player_name
#define local_player_have_track bluez_upstream_object_local_player_have_track
#define local_player_play bluez_upstream_object_local_player_play
#define local_player_stop bluez_upstream_object_local_player_stop
#define local_player_pause bluez_upstream_object_local_player_pause
#define local_player_next bluez_upstream_object_local_player_next
#define local_player_previous bluez_upstream_object_local_player_previous
#define local_player_register_watch bluez_upstream_object_local_player_register_watch
#define local_player_unregister_watch bluez_upstream_object_local_player_unregister_watch

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#include "upstream/profiles/audio/avrcp-player.c"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct avrcp_player *bluez_upstream_object_avrcp_register_player(
    struct btd_adapter *adapter, struct avrcp_player_cb *cb,
    void *user_data, GDestroyNotify destroy)
{
  (void)adapter;
  (void)cb;
  (void)user_data;
  (void)destroy;
  return (struct avrcp_player *)(uintptr_t)1;
}

void bluez_upstream_object_avrcp_unregister_player(
    struct avrcp_player *player)
{
  (void)player;
}

void bluez_upstream_object_avrcp_player_event(struct avrcp_player *player,
                                              uint8_t id,
                                              const void *data)
{
  (void)player;
  (void)id;
  (void)data;
}

void bluez_upstream_avrcp_player_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/avrcp-player.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/avrcp-player.c "
         "profile=AVRCPPlayer owner=bluetoothd\n",
         role);
}
