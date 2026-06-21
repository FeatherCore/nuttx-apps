/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_player_object_probe.c
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

#define media_player_controller_create bluez_upstream_object_media_player_controller_create
#define media_player_get_path bluez_upstream_object_media_player_get_path
#define media_player_destroy bluez_upstream_object_media_player_destroy
#define media_player_set_duration bluez_upstream_object_media_player_set_duration
#define media_player_set_position bluez_upstream_object_media_player_set_position
#define media_player_set_setting bluez_upstream_object_media_player_set_setting
#define media_player_get_status bluez_upstream_object_media_player_get_status
#define media_player_set_status bluez_upstream_object_media_player_set_status
#define media_player_clear_metadata bluez_upstream_object_media_player_clear_metadata
#define media_player_set_metadata bluez_upstream_object_media_player_set_metadata
#define media_player_metadata_changed bluez_upstream_object_media_player_metadata_changed
#define media_player_set_type bluez_upstream_object_media_player_set_type
#define media_player_set_subtype bluez_upstream_object_media_player_set_subtype
#define media_player_set_name bluez_upstream_object_media_player_set_name
#define media_player_set_browsable bluez_upstream_object_media_player_set_browsable
#define media_player_get_browsable bluez_upstream_object_media_player_get_browsable
#define media_player_set_searchable bluez_upstream_object_media_player_set_searchable
#define media_player_set_folder bluez_upstream_object_media_player_set_folder
#define media_player_set_playlist bluez_upstream_object_media_player_set_playlist
#define media_player_set_playlist_item bluez_upstream_object_media_player_set_playlist_item
#define media_player_clear_playlist bluez_upstream_object_media_player_clear_playlist
#define media_player_set_obex_port bluez_upstream_object_media_player_set_obex_port
#define media_player_create_folder bluez_upstream_object_media_player_create_folder
#define media_player_create_item bluez_upstream_object_media_player_create_item
#define media_player_play_item_complete bluez_upstream_object_media_player_play_item_complete
#define media_item_set_playable bluez_upstream_object_media_item_set_playable
#define media_player_list_complete bluez_upstream_object_media_player_list_complete
#define media_player_change_folder_complete bluez_upstream_object_media_player_change_folder_complete
#define media_player_search_complete bluez_upstream_object_media_player_search_complete
#define media_player_total_items_complete bluez_upstream_object_media_player_total_items_complete
#define media_player_set_callbacks bluez_upstream_object_media_player_set_callbacks

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "upstream/profiles/audio/player.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_player_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/player.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/player.c "
         "interface=org.bluez.MediaPlayer1 owner=bluetoothd\n",
         role);
}
