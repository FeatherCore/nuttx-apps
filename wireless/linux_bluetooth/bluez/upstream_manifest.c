/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_manifest.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

#include "upstream_manifest.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char *const g_bluez_upstream_a2dp_audio_files[] =
{
  "bluez/upstream/profiles/audio/a2dp.c",
  "bluez/upstream/profiles/audio/a2dp.h",
  "bluez/upstream/profiles/audio/a2dp-codecs.h",
  "bluez/upstream/profiles/audio/avdtp.c",
  "bluez/upstream/profiles/audio/avdtp.h",
  "bluez/upstream/profiles/audio/avctp.c",
  "bluez/upstream/profiles/audio/avctp.h",
  "bluez/upstream/profiles/audio/avrcp.c",
  "bluez/upstream/profiles/audio/avrcp.h",
  "bluez/upstream/profiles/audio/avrcp-player.c",
  "bluez/upstream/profiles/audio/media.c",
  "bluez/upstream/profiles/audio/media.h",
  "bluez/upstream/profiles/audio/transport.c",
  "bluez/upstream/profiles/audio/transport.h",
  "bluez/upstream/profiles/audio/source.c",
  "bluez/upstream/profiles/audio/source.h",
  "bluez/upstream/profiles/audio/sink.c",
  "bluez/upstream/profiles/audio/sink.h",
  "bluez/upstream/profiles/audio/player.c",
  "bluez/upstream/profiles/audio/player.h",
};

static const char *const g_bluez_upstream_a2dp_core_files[] =
{
  "bluez/upstream/src/main.c",
  "bluez/upstream/src/plugin.c",
  "bluez/upstream/src/profile.c",
  "bluez/upstream/src/device.c",
  "bluez/upstream/src/adapter.c",
  "bluez/upstream/src/dbus-common.c",
  "bluez/upstream/src/sdpd-service.c",
  "bluez/upstream/src/shared/mainloop.c",
  "bluez/upstream/src/shared/io-mainloop.c",
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_a2dp_manifest_print(const char *role)
{
  printf("bluez-daemon: a2dp upstream-source-manifest role=%s "
         "apps-link=bluez/upstream target=third/bluez "
         "audio-files=%u core-files=%u "
         "compile-unit=bluez/upstream_manifest.c "
         "source-mirror=apps/wireless/linux_bluetooth/bluez/upstream "
         "upstream-link=source-mirror-upstream-plugin\n",
         role,
         (unsigned int)(sizeof(g_bluez_upstream_a2dp_audio_files) /
                        sizeof(g_bluez_upstream_a2dp_audio_files[0])),
         (unsigned int)(sizeof(g_bluez_upstream_a2dp_core_files) /
                        sizeof(g_bluez_upstream_a2dp_core_files[0])));

  printf("bluez-daemon: a2dp upstream-source-manifest audio role=%s "
         "files=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
         role,
         g_bluez_upstream_a2dp_audio_files[0],
         g_bluez_upstream_a2dp_audio_files[1],
         g_bluez_upstream_a2dp_audio_files[2],
         g_bluez_upstream_a2dp_audio_files[3],
         g_bluez_upstream_a2dp_audio_files[4],
         g_bluez_upstream_a2dp_audio_files[5],
         g_bluez_upstream_a2dp_audio_files[6],
         g_bluez_upstream_a2dp_audio_files[7],
         g_bluez_upstream_a2dp_audio_files[8],
         g_bluez_upstream_a2dp_audio_files[9],
         g_bluez_upstream_a2dp_audio_files[10],
         g_bluez_upstream_a2dp_audio_files[11],
         g_bluez_upstream_a2dp_audio_files[12],
         g_bluez_upstream_a2dp_audio_files[13],
         g_bluez_upstream_a2dp_audio_files[14],
         g_bluez_upstream_a2dp_audio_files[15],
         g_bluez_upstream_a2dp_audio_files[16],
         g_bluez_upstream_a2dp_audio_files[17],
         g_bluez_upstream_a2dp_audio_files[18],
         g_bluez_upstream_a2dp_audio_files[19]);

  printf("bluez-daemon: a2dp upstream-source-manifest core role=%s "
         "files=%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
         role,
         g_bluez_upstream_a2dp_core_files[0],
         g_bluez_upstream_a2dp_core_files[1],
         g_bluez_upstream_a2dp_core_files[2],
         g_bluez_upstream_a2dp_core_files[3],
         g_bluez_upstream_a2dp_core_files[4],
         g_bluez_upstream_a2dp_core_files[5],
         g_bluez_upstream_a2dp_core_files[6],
         g_bluez_upstream_a2dp_core_files[7],
         g_bluez_upstream_a2dp_core_files[8]);
}

