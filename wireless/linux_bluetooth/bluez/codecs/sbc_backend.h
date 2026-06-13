/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/codecs/sbc_backend.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_SBC_BACKEND_H
#define __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_SBC_BACKEND_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_A2DP_SBC_SELECTED_CONFIG "21 15 02 35"
#define BLUEZ_A2DP_SBC_SYNCWORD        0x9c

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct bluez_audio_sbc_frame_info
{
  uint32_t samplerate;
  uint8_t blocks;
  uint8_t channels;
  uint8_t subbands;
  uint8_t bitpool;
  const char *channel_mode;
  const char *allocation;
};

struct bluez_audio_sbc_pcm_info
{
  size_t codesize;
  size_t pcm_len;
  uint32_t checksum;
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

const char *bluez_audio_sbc_backend_name(void);
const char *bluez_audio_sbc_backend_status(void);
const char *bluez_audio_sbc_backend_required_source(void);
const uint8_t *bluez_audio_sbc_backend_frame(size_t *frame_len);
int bluez_audio_sbc_parse_frame_header(
    const uint8_t *frame, size_t frame_len,
    struct bluez_audio_sbc_frame_info *info);
int bluez_audio_sbc_decode_frame(const uint8_t *frame, size_t frame_len,
                                 struct bluez_audio_sbc_pcm_info *info);

#endif /* __APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_CODECS_SBC_BACKEND_H */
