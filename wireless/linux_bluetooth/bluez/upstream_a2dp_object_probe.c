/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_a2dp_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "btio/btio.h"
#include "upstream_a2dp_object_probe.h"
#include "upstream_avdtp_object_probe.h"

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

#define HAVE_A2DP 1

static struct
{
  struct
  {
    BtIOMode session_mode;
    BtIOMode stream_mode;
  } avdtp;
} btd_opts =
{
  {
    BT_IO_MODE_BASIC,
    BT_IO_MODE_BASIC
  }
};

#define a2dp_parse_config_error bluez_upstream_object_a2dp_parse_config_error
#define a2dp_add_sep bluez_upstream_object_a2dp_add_sep
#define a2dp_remove_sep bluez_upstream_object_a2dp_remove_sep
#define a2dp_discover bluez_upstream_object_a2dp_discover
#define a2dp_select_capabilities bluez_upstream_object_a2dp_select_capabilities
#define a2dp_config bluez_upstream_object_a2dp_config
#define a2dp_resume bluez_upstream_object_a2dp_resume
#define a2dp_suspend bluez_upstream_object_a2dp_suspend
#define a2dp_cancel bluez_upstream_object_a2dp_cancel
#define a2dp_sep_lock bluez_upstream_object_a2dp_sep_lock
#define a2dp_sep_unlock bluez_upstream_object_a2dp_sep_unlock
#define a2dp_sep_get_stream bluez_upstream_object_a2dp_sep_get_stream
#define a2dp_setup_get_device bluez_upstream_object_a2dp_setup_get_device
#define a2dp_setup_remote_path bluez_upstream_object_a2dp_setup_remote_path
#define a2dp_avdtp_get bluez_upstream_object_a2dp_avdtp_get

#define sink_new_stream bluez_upstream_object_sink_new_stream
#define source_new_stream bluez_upstream_object_source_new_stream

#define adapter_service_add bluez_upstream_a2dp_probe_adapter_service_add
#define adapter_service_remove bluez_upstream_a2dp_probe_adapter_service_remove

#define avdtp_ref bluez_upstream_object_avdtp_ref
#define avdtp_unref bluez_upstream_object_avdtp_unref
#define avdtp_service_cap_new bluez_upstream_object_avdtp_service_cap_new
#define avdtp_register_remote_sep bluez_upstream_object_avdtp_register_remote_sep
#define avdtp_unregister_remote_sep bluez_upstream_object_avdtp_unregister_remote_sep
#define avdtp_error_init bluez_upstream_object_avdtp_error_init
#define avdtp_error_category bluez_upstream_object_avdtp_error_category
#define avdtp_error_error_code bluez_upstream_object_avdtp_error_error_code
#define avdtp_error_posix_errno bluez_upstream_object_avdtp_error_posix_errno
#define avdtp_get_seid bluez_upstream_object_avdtp_get_seid
#define avdtp_stream_get_transport bluez_upstream_object_avdtp_stream_get_transport
#define avdtp_stream_get_state bluez_upstream_a2dp_probe_avdtp_stream_get_state
#define avdtp_stream_set_transport bluez_upstream_object_avdtp_stream_set_transport
#define avdtp_stream_get_io bluez_upstream_object_avdtp_stream_get_io
#define avdtp_stream_get_imtu bluez_upstream_object_avdtp_stream_get_imtu
#define avdtp_stream_get_omtu bluez_upstream_object_avdtp_stream_get_omtu
#define avdtp_stream_add_cb bluez_upstream_object_avdtp_stream_add_cb
#define avdtp_stream_remove_cb bluez_upstream_object_avdtp_stream_remove_cb
#define avdtp_get bluez_upstream_object_avdtp_get
#define avdtp_connect bluez_upstream_object_avdtp_connect
#define avdtp_disconnect bluez_upstream_object_avdtp_disconnect
#define avdtp_discover bluez_upstream_object_avdtp_discover
#define avdtp_get_seps bluez_upstream_object_avdtp_get_seps
#define avdtp_get_all_capabilities bluez_upstream_object_avdtp_get_all_capabilities
#define avdtp_set_configuration bluez_upstream_object_avdtp_set_configuration
#define avdtp_get_configuration bluez_upstream_object_avdtp_get_configuration
#define avdtp_open bluez_upstream_object_avdtp_open
#define avdtp_start bluez_upstream_object_avdtp_start
#define avdtp_suspend bluez_upstream_object_avdtp_suspend
#define avdtp_close bluez_upstream_object_avdtp_close
#define avdtp_abort bluez_upstream_object_avdtp_abort
#define avdtp_delay_report bluez_upstream_object_avdtp_delay_report
#define avdtp_register_sep bluez_upstream_object_avdtp_register_sep
#define avdtp_unregister_sep bluez_upstream_object_avdtp_unregister_sep
#define avdtp_add_state_cb bluez_upstream_object_avdtp_add_state_cb
#define avdtp_remove_state_cb bluez_upstream_object_avdtp_remove_state_cb
#define avdtp_get_version bluez_upstream_object_avdtp_get_version

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#include "upstream/profiles/audio/a2dp.c"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

unsigned char bluez_upstream_probe_adapter_identity;
static struct avdtp *bluez_upstream_a2dp_prepared_resume_session;
static avdtp_state_t bluez_upstream_a2dp_prepared_stream_state =
  AVDTP_STATE_CONFIGURED;
static struct avdtp_stream *bluez_upstream_a2dp_prepared_media_stream;
static GIOChannel *bluez_upstream_a2dp_prepared_media_io;
static uint16_t bluez_upstream_a2dp_prepared_media_imtu;
static uint16_t bluez_upstream_a2dp_prepared_media_omtu;
static bool bluez_upstream_a2dp_prepared_media_valid;
static struct avdtp_remote_sep *bluez_upstream_a2dp_last_avdtp_remote_sep;
static struct a2dp_remote_sep *bluez_upstream_a2dp_last_a2dp_remote_sep;

avdtp_state_t bluez_upstream_a2dp_probe_avdtp_stream_get_state(
    struct avdtp_stream *stream)
{
  return stream != NULL ? bluez_upstream_a2dp_prepared_stream_state :
         AVDTP_STATE_IDLE;
}

gboolean bluez_upstream_object_avdtp_stream_set_transport(
    struct avdtp_stream *stream, int fd, size_t imtu, size_t omtu)
{
  if (stream == NULL)
    {
      return FALSE;
    }

  bluez_upstream_a2dp_prepared_media_stream = stream;
  bluez_upstream_a2dp_prepared_media_io = g_io_channel_unix_new(fd);
  if (bluez_upstream_a2dp_prepared_media_io == NULL)
    {
      return FALSE;
    }

  bluez_upstream_a2dp_prepared_media_imtu = imtu;
  bluez_upstream_a2dp_prepared_media_omtu = omtu;
  bluez_upstream_a2dp_prepared_media_valid = true;
  return TRUE;
}

int bluez_upstream_a2dp_probe_adapter_service_add(
    struct btd_adapter *adapter, sdp_record_t *record)
{
  static uint32_t next_handle = 0x10000;

  (void)adapter;

  if (record != NULL && record->handle == 0)
    {
      record->handle = ++next_handle;
    }

  return 0;
}

void bluez_upstream_a2dp_probe_adapter_service_remove(
    struct btd_adapter *adapter, uint32_t handle)
{
  (void)adapter;
  (void)handle;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_a2dp_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/a2dp.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/a2dp.c\n",
         role);
}

unsigned int
bluez_upstream_a2dp_object_adapter_profiles_ready(
  struct btd_adapter *adapter)
{
  struct a2dp_server *server;

  if (adapter == NULL)
    {
      return 0;
    }

  if (a2dp_sink_server_probe(NULL, adapter) < 0)
    {
      return 0;
    }

  if (a2dp_source_server_probe(NULL, adapter) < 0)
    {
      return 0;
    }

  server = find_server(servers, adapter);
  return server != NULL && server->sink_enabled &&
         server->source_enabled && server->seps != NULL &&
         server->channels != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_a2dp_object_prepare_resume_stream(
  struct btd_device *device, struct a2dp_sep *sep)
{
  struct avdtp *session;
  struct a2dp_stream *stream;

  if (device == NULL || sep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_adapter_profiles_ready(
        device_get_adapter(device)) != 1)
    {
      return 0;
    }

  session = a2dp_avdtp_get(device);
  if (session == NULL)
    {
      return 0;
    }

  stream = a2dp_stream_get(sep, session);
  if (stream == NULL)
    {
      return 0;
    }

  stream->stream = (struct avdtp_stream *)stream;
  bluez_upstream_a2dp_prepared_resume_session = session;
  bluez_upstream_a2dp_prepared_stream_state = AVDTP_STATE_CONFIGURED;
  return a2dp_sep_get_stream(sep, session) == stream->stream ? 1 : 0;
}

struct avdtp *
bluez_upstream_a2dp_object_get_prepared_resume_session(void)
{
  return bluez_upstream_a2dp_prepared_resume_session;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_synthetic_setup(
  struct btd_device *device, const char *remote_path)
{
  static struct a2dp_setup setup;
  static struct a2dp_remote_sep rsep;

  memset(&setup, 0, sizeof(setup));
  memset(&rsep, 0, sizeof(rsep));

  rsep.path = (char *)remote_path;
  setup.session =
    bluez_upstream_avdtp_object_create_synthetic_session(device);
  setup.rsep = &rsep;
  setup.ref = 1;
  return &setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_prepared_setup(
  struct btd_device *device, struct a2dp_sep *sep,
  const char *remote_path)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  static struct a2dp_remote_sep rsep;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL)
    {
      return NULL;
    }

  memset(&rsep, 0, sizeof(rsep));
  rsep.path = (char *)remote_path;
  setup->rsep = &rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_registered_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  struct avdtp_service_capability *cap;
  struct avdtp_media_codec_capability codec;
  GSList *caps = NULL;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(&codec, 0, sizeof(codec));
  codec.media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec.media_codec_type = sep->codec;

  cap = avdtp_service_cap_new(AVDTP_MEDIA_CODEC, &codec, sizeof(codec));
  if (cap == NULL)
    {
      return NULL;
    }

  caps = g_slist_append(caps, cap);
  avdtp_rsep = avdtp_register_remote_sep(session, seid,
                                         AVDTP_SEP_TYPE_SOURCE, caps,
                                         false);
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_parsed_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t raw[sizeof(struct avdtp_service_capability) +
              sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)raw;
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(raw, 0, sizeof(raw));
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_raw_caps(
      session, seid, AVDTP_SEP_TYPE_SOURCE, raw, sizeof(raw));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_getcap_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_getcap_response(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_dispatch_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_parse_response(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_packet_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_parse_data(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_session_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_session_cb(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_discover_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_discover(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

struct a2dp_setup *
bluez_upstream_a2dp_object_create_l2cap_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *avdtp_rsep;
  struct a2dp_remote_sep *a2dp_rsep;
  uint8_t response[(sizeof(struct avdtp_service_capability) * 2) +
                   sizeof(struct avdtp_media_codec_capability)];
  struct avdtp_service_capability *transport =
    (struct avdtp_service_capability *)response;
  struct avdtp_service_capability *cap =
    (struct avdtp_service_capability *)(response + sizeof(*transport));
  struct avdtp_media_codec_capability *codec =
    (struct avdtp_media_codec_capability *)cap->data;

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return NULL;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  if (setup == NULL || setup->chan == NULL)
    {
      return NULL;
    }

  memset(response, 0, sizeof(response));
  transport->category = AVDTP_MEDIA_TRANSPORT;
  transport->length = 0;
  cap->category = AVDTP_MEDIA_CODEC;
  cap->length = sizeof(*codec);
  codec->media_type = AVDTP_MEDIA_TYPE_AUDIO;
  codec->media_codec_type = sep->codec;

  avdtp_rsep =
    bluez_upstream_avdtp_object_register_remote_sep_from_l2cap_connect(
      session, seid, AVDTP_SEP_TYPE_SOURCE, response, sizeof(response));
  if (avdtp_rsep == NULL)
    {
      return NULL;
    }

  a2dp_rsep = register_remote_sep(avdtp_rsep, setup->chan);
  if (a2dp_rsep == NULL || a2dp_rsep->path == NULL)
    {
      return NULL;
    }

  setup->rsep = a2dp_rsep;
  bluez_upstream_a2dp_last_avdtp_remote_sep = avdtp_rsep;
  bluez_upstream_a2dp_last_a2dp_remote_sep = a2dp_rsep;
  return setup;
}

unsigned int
bluez_upstream_a2dp_object_setup_matches(
  struct a2dp_setup *setup, struct btd_device *device,
  const char *remote_path)
{
  return setup != NULL &&
         a2dp_setup_get_device(setup) == device &&
         a2dp_setup_remote_path(setup) != NULL &&
         remote_path != NULL &&
         strcmp(a2dp_setup_remote_path(setup), remote_path) == 0 ? 1 : 0;
}

const char *
bluez_upstream_a2dp_object_setup_remote_path(struct a2dp_setup *setup)
{
  return setup != NULL ? a2dp_setup_remote_path(setup) : NULL;
}

unsigned int
bluez_upstream_a2dp_object_registered_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_registered_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_parsed_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_parsed_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_getcap_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_getcap_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_dispatch_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_dispatch_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_packet_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_packet_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_session_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_session_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_discover_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_discover_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_l2cap_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid)
{
  struct a2dp_setup *setup;
  struct avdtp_remote_sep *session_sep;
  struct a2dp_remote_sep *channel_sep;
  unsigned int ready = 0;

  setup = bluez_upstream_a2dp_object_create_l2cap_remote_setup(
    device, sep, seid);
  if (setup == NULL || setup->chan == NULL || setup->session == NULL ||
      setup->rsep == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_last_avdtp_remote_sep != NULL &&
      avdtp_get_seid(bluez_upstream_a2dp_last_avdtp_remote_sep) == seid &&
      avdtp_get_codec(bluez_upstream_a2dp_last_avdtp_remote_sep) != NULL)
    {
      ready |= 1;
    }

  session_sep = avdtp_find_remote_sep(setup->session, sep->lsep);
  if (session_sep != NULL && avdtp_get_codec(session_sep) != NULL)
    {
      ready |= 2;
    }

  channel_sep = find_remote_sep(setup->chan, sep);
  if (channel_sep != NULL && channel_sep->path != NULL)
    {
      ready |= 4;
    }

  if (setup->rsep->path != NULL &&
      strstr(setup->rsep->path, "/sep") != NULL)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_a2dp_object_prepare_resume_setup(
  struct btd_device *device, struct a2dp_sep *sep)
{
  struct avdtp *session;
  struct a2dp_setup *setup;
  struct a2dp_stream *stream;
  gboolean locked = FALSE;
  avdtp_state_t state = AVDTP_STATE_IDLE;
  unsigned int total;

  if (bluez_upstream_a2dp_object_prepare_resume_stream(device, sep) != 1)
    {
      return 0;
    }

  session = bluez_upstream_a2dp_prepared_resume_session;
  setup = session != NULL ? a2dp_setup_get(session) : NULL;
  stream = session != NULL ? queue_find(sep->streams, match_stream_session,
                                        session) : NULL;
  if (stream != NULL && stream->stream != NULL)
    {
      state = avdtp_stream_get_state(stream->stream);
    }

  if (session != NULL)
    {
      locked = a2dp_sep_lock(sep, session);
      if (locked)
        {
          a2dp_sep_unlock(sep, session);
        }
    }

  total = (setup != NULL ? 1 : 0) + (stream != NULL ? 1 : 0) +
          (stream != NULL && stream->stream != NULL ? 1 : 0) +
          (state != AVDTP_STATE_IDLE ? 1 : 0) +
          (locked ? 1 : 0) +
          (setup != NULL && !setup->reconfigure ? 1 : 0);

  return total == 6 ? 1 : 0;
}

unsigned int
bluez_upstream_a2dp_object_mark_prepared_streaming(void)
{
  if (bluez_upstream_a2dp_prepared_resume_session == NULL)
    {
      return 0;
    }

  bluez_upstream_a2dp_prepared_stream_state = AVDTP_STATE_STREAMING;
  return bluez_upstream_a2dp_prepared_stream_state == AVDTP_STATE_STREAMING ?
         1 : 0;
}

unsigned int
bluez_upstream_a2dp_object_prepare_stream_media_transport(
  struct avdtp_stream *stream, int fd, uint16_t imtu, uint16_t omtu)
{
  return bluez_upstream_object_avdtp_stream_set_transport(stream, fd, imtu,
                                                          omtu) == TRUE ?
         1 : 0;
}

unsigned int
bluez_upstream_a2dp_object_get_stream_media_transport(
  struct avdtp_stream *stream, int *fd, uint16_t *imtu, uint16_t *omtu)
{
  if (!bluez_upstream_a2dp_prepared_media_valid ||
      stream == NULL || stream != bluez_upstream_a2dp_prepared_media_stream)
    {
      return 0;
    }

  if (fd != NULL)
    {
      *fd = g_io_channel_unix_get_fd(bluez_upstream_a2dp_prepared_media_io);
    }

  if (imtu != NULL)
    {
      *imtu = bluez_upstream_a2dp_prepared_media_imtu;
    }

  if (omtu != NULL)
    {
      *omtu = bluez_upstream_a2dp_prepared_media_omtu;
    }

  return 1;
}
