/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/codecs/lc3_backend.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_LC3_BACKEND_H
#define __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_LC3_BACKEND_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_LE_AUDIO_LC3_CAPABILITIES "16_2_1"
#define BLUEZ_LE_AUDIO_LC3_METADATA     "context-media,location-front-left-right"

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct bluez_audio_lc3_frame_info
{
  uint32_t sample_rate;
  uint32_t frame_duration_us;
  uint16_t octets_per_frame;
  uint8_t blocks_per_sdu;
  uint8_t channels;
  uint32_t presentation_delay_us;
  uint32_t sdu_interval_us;
  const char *framing;
  const char *context;
  const char *location;
};

struct bluez_audio_lc3_pcm_info
{
  size_t frame_len;
  size_t pcm_len;
  uint32_t checksum;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

const char *bluez_audio_lc3_backend_name(void);
const char *bluez_audio_lc3_backend_status(void);
const char *bluez_audio_lc3_backend_required_source(void);
const uint8_t *bluez_audio_lc3_backend_frame(size_t *frame_len);
int bluez_audio_lc3_parse_frame(const uint8_t *frame, size_t frame_len,
                                struct bluez_audio_lc3_frame_info *info);
int bluez_audio_lc3_decode_frame(const uint8_t *frame, size_t frame_len,
                                 struct bluez_audio_lc3_pcm_info *info);

#endif /* __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_LC3_BACKEND_H */
