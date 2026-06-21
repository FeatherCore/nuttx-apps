/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/codecs/lc3_backend.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <lc3.h>

#include "lc3_backend.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint8_t g_bluez_audio_lc3_encoded_frame[40];
static bool g_bluez_audio_lc3_encoded_valid;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t bluez_audio_lc3_checksum(const uint8_t *data, size_t len)
{
  uint32_t checksum = 2166136261u;
  size_t i;

  for (i = 0; i < len; i++)
    {
      checksum ^= data[i];
      checksum *= 16777619u;
    }

  return checksum;
}

static void bluez_audio_lc3_fill_pcm(int16_t *pcm, size_t samples)
{
  size_t i;

  for (i = 0; i < samples; i++)
    {
      int32_t sample = ((int32_t)((i * 997u) % 4096u)) - 2048;

      pcm[i] = (int16_t)(sample << 3);
    }
}

static int bluez_audio_lc3_encode_frame(void)
{
  lc3_encoder_mem_16k_t enc_mem;
  lc3_encoder_t enc;
  int16_t pcm[160];
  int ret;

  memset(&enc_mem, 0, sizeof(enc_mem));
  memset(g_bluez_audio_lc3_encoded_frame, 0,
         sizeof(g_bluez_audio_lc3_encoded_frame));
  bluez_audio_lc3_fill_pcm(pcm, sizeof(pcm) / sizeof(pcm[0]));

  enc = lc3_setup_encoder(10000, 16000, 0, &enc_mem);
  if (enc == NULL)
    {
      return -EINVAL;
    }

  lc3_encoder_disable_ltpf(enc);
  ret = lc3_encode(enc, LC3_PCM_FORMAT_S16, pcm, 1,
                   sizeof(g_bluez_audio_lc3_encoded_frame),
                   g_bluez_audio_lc3_encoded_frame);
  if (ret < 0)
    {
      return -EINVAL;
    }

  g_bluez_audio_lc3_encoded_valid = true;
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

const char *bluez_audio_lc3_backend_name(void)
{
  return "google-liblc3";
}

const char *bluez_audio_lc3_backend_status(void)
{
  return "liblc3-linked";
}

const char *bluez_audio_lc3_backend_required_source(void)
{
  return "third/liblc3";
}

const uint8_t *bluez_audio_lc3_backend_frame(size_t *frame_len)
{
  if (!g_bluez_audio_lc3_encoded_valid &&
      bluez_audio_lc3_encode_frame() < 0)
    {
      if (frame_len != NULL)
        {
          *frame_len = 0;
        }

      return NULL;
    }

  if (frame_len != NULL)
    {
      *frame_len = sizeof(g_bluez_audio_lc3_encoded_frame);
    }

  return g_bluez_audio_lc3_encoded_frame;
}

int bluez_audio_lc3_parse_frame(const uint8_t *frame, size_t frame_len,
                                struct bluez_audio_lc3_frame_info *info)
{
  if (frame == NULL || info == NULL || frame_len < 20 ||
      frame_len > LC3_MAX_FRAME_BYTES)
    {
      return -EINVAL;
    }

  info->sample_rate = 16000;
  info->frame_duration_us = 10000;
  info->octets_per_frame = 40;
  info->blocks_per_sdu = 1;
  info->channels = 2;
  info->presentation_delay_us = 40000;
  info->sdu_interval_us = 10000;
  info->framing = "unframed";
  info->context = "media";
  info->location = "front-left-right";
  return 0;
}

int bluez_audio_lc3_decode_frame(const uint8_t *frame, size_t frame_len,
                                 struct bluez_audio_lc3_pcm_info *info)
{
  lc3_decoder_mem_16k_t dec_mem;
  lc3_decoder_t dec;
  int16_t pcm[160];
  int ret;

  if (frame == NULL || info == NULL || frame_len < 20 ||
      frame_len > LC3_MAX_FRAME_BYTES)
    {
      return -EINVAL;
    }

  memset(&dec_mem, 0, sizeof(dec_mem));
  memset(pcm, 0, sizeof(pcm));

  dec = lc3_setup_decoder(10000, 16000, 0, &dec_mem);
  if (dec == NULL)
    {
      return -EINVAL;
    }

  ret = lc3_decode(dec, frame, frame_len, LC3_PCM_FORMAT_S16, pcm, 1);
  if (ret < 0)
    {
      return -EINVAL;
    }

  info->frame_len = frame_len;
  info->pcm_len = sizeof(pcm);
  info->checksum = bluez_audio_lc3_checksum((const uint8_t *)pcm,
                                            sizeof(pcm));
  return 0;
}
