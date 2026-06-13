/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/codecs/sbc_backend.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <nuttx/config.h>

#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
#  include <sbc.h>
#endif

#include "sbc_backend.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_A2DP_SBC_FRAME_MAX 256
#define BLUEZ_A2DP_SBC_PCM_MAX   1024

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
static uint8_t g_bluez_audio_sbc_frame[BLUEZ_A2DP_SBC_FRAME_MAX];
static size_t g_bluez_audio_sbc_frame_len;
static int g_bluez_audio_sbc_frame_ready;
static char g_bluez_audio_sbc_backend_status[] = "source-built-pending";
#else
static const uint8_t g_bluez_audio_sbc_frame[] =
{
  0x9c, 0xbd, 0x35, 0x00, 0x10, 0x20, 0x30, 0x40,
  0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0,
  0xd0, 0xe0, 0xf0, 0x00
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
static void bluez_audio_sbc_fill_pcm(uint8_t *pcm, size_t pcm_len)
{
  size_t i;

  for (i = 0; i < pcm_len; i += 2)
    {
      int16_t sample = (int16_t)((i * 31u) & 0x7fff);
      pcm[i] = (uint8_t)(sample & 0xff);
      if (i + 1 < pcm_len)
        {
          pcm[i + 1] = (uint8_t)((sample >> 8) & 0xff);
        }
    }
}

static int bluez_audio_sbc_build_libsbc_frame(void)
{
  static const uint8_t conf[] = { 0x21, 0x15, 0x02, 0x35 };
  uint8_t pcm[1024];
  ssize_t consumed;
  ssize_t written = 0;
  size_t codesize;
  sbc_t sbc;
  int ret;

  if (g_bluez_audio_sbc_frame_ready)
    {
      return 0;
    }

  memset(&sbc, 0, sizeof(sbc));
  ret = sbc_init_a2dp(&sbc, 0, conf, sizeof(conf));
  if (ret < 0)
    {
      strcpy(g_bluez_audio_sbc_backend_status, "source-built-init-failed");
      return ret;
    }

  codesize = sbc_get_codesize(&sbc);
  if (codesize == 0 || codesize > sizeof(pcm))
    {
      strcpy(g_bluez_audio_sbc_backend_status, "source-built-codesize-failed");
      sbc_finish(&sbc);
      return -EINVAL;
    }

  bluez_audio_sbc_fill_pcm(pcm, codesize);
  consumed = sbc_encode(&sbc, pcm, codesize, g_bluez_audio_sbc_frame,
                        sizeof(g_bluez_audio_sbc_frame), &written);
  if (consumed < 0 || written <= 0)
    {
      strcpy(g_bluez_audio_sbc_backend_status, "source-built-encode-failed");
      sbc_finish(&sbc);
      return consumed < 0 ? (int)consumed : -EINVAL;
    }

  g_bluez_audio_sbc_frame_len = (size_t)written;
  g_bluez_audio_sbc_frame_ready = 1;
  strcpy(g_bluez_audio_sbc_backend_status, "source-built");
  sbc_finish(&sbc);
  return 0;
}
#endif

static uint32_t bluez_audio_sbc_checksum(const uint8_t *data, size_t len)
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

/****************************************************************************
 * Public Functions
 ****************************************************************************/

const char *bluez_audio_sbc_backend_name(void)
{
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
  return "libsbc";
#else
  return "deterministic-sbc-frame";
#endif
}

const char *bluez_audio_sbc_backend_status(void)
{
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
  return g_bluez_audio_sbc_backend_status;
#else
  return "libsbc-missing";
#endif
}

const char *bluez_audio_sbc_backend_required_source(void)
{
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
  return "third/sbc-2.0";
#else
  return "third/sbc-or-apps-codec";
#endif
}

const uint8_t *bluez_audio_sbc_backend_frame(size_t *frame_len)
{
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
  if (bluez_audio_sbc_build_libsbc_frame() < 0)
    {
      if (frame_len != NULL)
        {
          *frame_len = 0;
        }

      return NULL;
    }
#endif

  if (frame_len != NULL)
    {
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
      *frame_len = g_bluez_audio_sbc_frame_len;
#else
      *frame_len = sizeof(g_bluez_audio_sbc_frame);
#endif
    }

  return g_bluez_audio_sbc_frame;
}

int bluez_audio_sbc_parse_frame_header(
    const uint8_t *frame, size_t frame_len,
    struct bluez_audio_sbc_frame_info *info)
{
  static const uint32_t samplerates[] = { 16000, 32000, 44100, 48000 };
  static const uint8_t blocks[] = { 4, 8, 12, 16 };
  static const uint8_t channels[] = { 1, 2, 2, 2 };
  static const uint8_t subbands[] = { 4, 8 };
  static const char *channel_modes[] =
  {
    "mono", "dual-channel", "stereo", "joint-stereo"
  };
  static const char *allocations[] = { "loudness", "snr" };
  uint8_t config;

  if (frame_len < 4 || frame == NULL || info == NULL ||
      frame[0] != BLUEZ_A2DP_SBC_SYNCWORD)
    {
      return -EINVAL;
    }

  config = frame[1];
  info->samplerate = samplerates[config >> 6];
  info->blocks = blocks[(config >> 4) & 0x03];
  info->channels = channels[(config >> 2) & 0x03];
  info->subbands = subbands[config & 0x01];
  info->bitpool = frame[2];
  info->channel_mode = channel_modes[(config >> 2) & 0x03];
  info->allocation = allocations[(config >> 1) & 0x01];
  return 0;
}

int bluez_audio_sbc_decode_frame(const uint8_t *frame, size_t frame_len,
                                 struct bluez_audio_sbc_pcm_info *info)
{
#ifdef CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC
  static const uint8_t conf[] = { 0x21, 0x15, 0x02, 0x35 };
  uint8_t pcm[BLUEZ_A2DP_SBC_PCM_MAX];
  size_t written = 0;
  ssize_t consumed;
  size_t codesize;
  sbc_t sbc;
  int ret;

  if (frame == NULL || frame_len == 0 || info == NULL)
    {
      return -EINVAL;
    }

  memset(&sbc, 0, sizeof(sbc));
  ret = sbc_init_a2dp(&sbc, 0, conf, sizeof(conf));
  if (ret < 0)
    {
      return ret;
    }

  codesize = sbc_get_codesize(&sbc);
  consumed = sbc_decode(&sbc, frame, frame_len, pcm, sizeof(pcm), &written);
  if (consumed < 0 || written == 0)
    {
      sbc_finish(&sbc);
      return consumed < 0 ? (int)consumed : -EINVAL;
    }

  info->codesize = codesize;
  info->pcm_len = written;
  info->checksum = bluez_audio_sbc_checksum(pcm, written);
  sbc_finish(&sbc);
  return 0;
#else
  if (frame == NULL || frame_len == 0 || info == NULL)
    {
      return -EINVAL;
    }

  info->codesize = 512;
  info->pcm_len = 512;
  info->checksum = bluez_audio_sbc_checksum(frame, frame_len);
  return 0;
#endif
}
