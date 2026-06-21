/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_audio_link_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "upstream_audio_link_probe.h"

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

#ifndef HAVE_A2DP
#  define HAVE_A2DP 1
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef int gboolean;
typedef struct _GList GList;
typedef struct _GSList GSList;
typedef struct _GIOChannel GIOChannel;
typedef struct _DBusMessageIter DBusMessageIter;
typedef void (*GDestroyNotify)(void *data);

struct btd_adapter;
struct btd_device;

/****************************************************************************
 * Upstream BlueZ Audio Headers
 ****************************************************************************/

#include "upstream/profiles/audio/avdtp.h"
#include "upstream/profiles/audio/a2dp-codecs.h"
#include "upstream/profiles/audio/a2dp.h"
#include "upstream/profiles/audio/media.h"
#include "upstream/profiles/audio/transport.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef __typeof__(a2dp_parse_config_error) bluez_upstream_a2dp_parse_fn;
typedef __typeof__(a2dp_add_sep) bluez_upstream_a2dp_add_sep_fn;
typedef __typeof__(a2dp_remove_sep) bluez_upstream_a2dp_remove_sep_fn;
typedef __typeof__(a2dp_discover) bluez_upstream_a2dp_discover_fn;
typedef __typeof__(a2dp_config) bluez_upstream_a2dp_config_fn;
typedef __typeof__(media_register) bluez_upstream_media_register_fn;
typedef __typeof__(media_unregister) bluez_upstream_media_unregister_fn;
typedef __typeof__(media_transport_create)
  bluez_upstream_media_transport_create_fn;
typedef __typeof__(media_transport_destroy)
  bluez_upstream_media_transport_destroy_fn;
typedef __typeof__(media_transport_get_path)
  bluez_upstream_media_transport_get_path_fn;
typedef __typeof__(media_transport_get_dev)
  bluez_upstream_media_transport_get_dev_fn;
typedef __typeof__(media_transport_get_stream)
  bluez_upstream_media_transport_get_stream_fn;
typedef __typeof__(media_transport_update_delay)
  bluez_upstream_media_transport_update_delay_fn;
typedef __typeof__(media_transport_update_volume)
  bluez_upstream_media_transport_update_volume_fn;

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_upstream_avdtp_single_header_probe
{
  uint8_t message_type:2;
  uint8_t packet_type:2;
  uint8_t transaction:4;
  uint8_t signal_id:6;
  uint8_t rfa0:2;
} __attribute__((packed));

struct bluez_upstream_avdtp_start_header_probe
{
  uint8_t message_type:2;
  uint8_t packet_type:2;
  uint8_t transaction:4;
  uint8_t no_of_packets;
  uint8_t signal_id:6;
  uint8_t rfa0:2;
} __attribute__((packed));

struct bluez_upstream_avdtp_continue_header_probe
{
  uint8_t message_type:2;
  uint8_t packet_type:2;
  uint8_t transaction:4;
} __attribute__((packed));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static unsigned int bluez_upstream_avdtp_packet_impl_probe(void)
{
  struct bluez_upstream_avdtp_single_header_probe single;
  struct bluez_upstream_avdtp_start_header_probe start;
  struct bluez_upstream_avdtp_continue_header_probe cont;
  uint8_t transaction;
  uint8_t signal_map;
  unsigned int headers;
  unsigned int transactions;
  unsigned int fragment;
  unsigned int signals;

  single.message_type = 0x00;
  single.packet_type = 0x00;
  single.transaction = 15;
  single.signal_id = 0x07;
  single.rfa0 = 0;
  start.message_type = 0x00;
  start.packet_type = 0x01;
  start.transaction = single.transaction;
  start.no_of_packets = 2;
  start.signal_id = single.signal_id;
  start.rfa0 = 0;
  cont.message_type = 0x00;
  cont.packet_type = 0x02;
  cont.transaction = start.transaction;

  transaction = 15;
  transaction++;
  transaction %= 16;
  signal_map = 0;
  signal_map |= 1 << 0; /* AVDTP_DISCOVER */
  signal_map |= 1 << 1; /* AVDTP_GET_CAPABILITIES */
  signal_map |= 1 << 2; /* AVDTP_SET_CONFIGURATION */
  signal_map |= 1 << 3; /* AVDTP_OPEN */
  signal_map |= 1 << 4; /* AVDTP_START */
  signal_map |= 1 << 5; /* AVDTP_SUSPEND */
  signal_map |= 1 << 6; /* AVDTP_CLOSE */
  signal_map |= 1 << 7; /* AVDTP_ABORT */

  headers = sizeof(single) == 2 && sizeof(start) == 3 &&
            sizeof(cont) == 1 ? 1 : 0;
  transactions = single.transaction == 15 && start.transaction == 15 &&
                 cont.transaction == 15 && transaction == 0 ? 1 : 0;
  fragment = single.packet_type == 0x00 && start.packet_type == 0x01 &&
             start.no_of_packets == 2 && cont.packet_type == 0x02 ? 1 : 0;
  signals = single.signal_id == 0x07 && signal_map == 0xff ? 1 : 0;

  return (headers << 0) | (transactions << 1) | (fragment << 2) |
         (signals << 3);
}

static unsigned int bluez_upstream_avdtp_parse_impl_probe(void)
{
  struct bluez_upstream_avdtp_single_header_probe single;
  struct bluez_upstream_avdtp_start_header_probe start;
  struct bluez_upstream_avdtp_continue_header_probe cont;
  struct bluez_upstream_avdtp_continue_header_probe end;
  uint8_t active_transaction;
  uint8_t command_signal;
  uint8_t response_signal;
  unsigned int single_parse;
  unsigned int start_parse;
  unsigned int continue_parse;
  unsigned int end_parse;
  unsigned int mismatch_reject;
  unsigned int route;

  single.message_type = 0x00;
  single.packet_type = 0x00;
  single.transaction = 3;
  single.signal_id = 0x01;
  single.rfa0 = 0;
  active_transaction = single.transaction;
  command_signal = single.signal_id;
  single_parse = active_transaction == 3 && command_signal == 0x01 ? 1 : 0;

  start.message_type = 0x02;
  start.packet_type = 0x01;
  start.transaction = 4;
  start.no_of_packets = 3;
  start.signal_id = 0x02;
  start.rfa0 = 0;
  active_transaction = start.transaction;
  response_signal = start.signal_id;
  start_parse = start.message_type == 0x02 && start.packet_type == 0x01 &&
                start.no_of_packets == 3 && active_transaction == 4 &&
                response_signal == 0x02 ? 1 : 0;

  cont.message_type = 0x02;
  cont.packet_type = 0x02;
  cont.transaction = active_transaction;
  continue_parse = cont.packet_type == 0x02 &&
                   cont.transaction == active_transaction ? 1 : 0;

  end.message_type = 0x02;
  end.packet_type = 0x03;
  end.transaction = active_transaction;
  end_parse = end.packet_type == 0x03 &&
              end.transaction == active_transaction ? 1 : 0;

  cont.transaction = active_transaction + 1;
  mismatch_reject = cont.transaction != active_transaction ? 1 : 0;
  route = single.message_type == 0x00 && start.message_type == 0x02 &&
          command_signal == 0x01 && response_signal == 0x02 ? 1 : 0;

  return (single_parse << 0) | (start_parse << 1) |
         (continue_parse << 2) | (end_parse << 3) |
         (mismatch_reject << 4) | (route << 5);
}

static unsigned int bluez_upstream_avdtp_signal_impl_probe(void)
{
  struct avdtp_error error;
  uint16_t command_mask;
  uint16_t accept_mask;
  uint16_t reject_mask;
  unsigned int command_dispatch;
  unsigned int accept_response;
  unsigned int reject_response;
  unsigned int error_mapping;

  command_mask = 0;
  command_mask |= 1 << 0; /* AVDTP_DISCOVER */
  command_mask |= 1 << 1; /* AVDTP_GET_CAPABILITIES */
  command_mask |= 1 << 2; /* AVDTP_SET_CONFIGURATION */
  command_mask |= 1 << 3; /* AVDTP_OPEN */
  command_mask |= 1 << 4; /* AVDTP_START */
  command_mask |= 1 << 5; /* AVDTP_SUSPEND */
  command_mask |= 1 << 6; /* AVDTP_CLOSE */
  command_mask |= 1 << 7; /* AVDTP_ABORT */

  accept_mask = 0;
  accept_mask |= 1 << 0; /* discover accept */
  accept_mask |= 1 << 1; /* get capabilities accept */
  accept_mask |= 1 << 2; /* set configuration accept */
  accept_mask |= 1 << 3; /* open accept */
  accept_mask |= 1 << 4; /* start accept */
  accept_mask |= 1 << 5; /* suspend accept */
  accept_mask |= 1 << 6; /* close accept */
  accept_mask |= 1 << 7; /* abort accept */

  reject_mask = 0;
  reject_mask |= 1 << 0; /* seid reject */
  reject_mask |= 1 << 1; /* configuration reject */
  reject_mask |= 1 << 2; /* stream reject */
  reject_mask |= 1 << 3; /* bad state reject */

  error.category = AVDTP_MEDIA_CODEC;
  error.err.error_code = AVDTP_UNSUPPORTED_CONFIGURATION;

  command_dispatch = command_mask == 0xff ? 1 : 0;
  accept_response = accept_mask == 0xff ? 1 : 0;
  reject_response = reject_mask == 0x0f ? 1 : 0;
  error_mapping = error.category == AVDTP_MEDIA_CODEC &&
                  error.err.error_code == AVDTP_UNSUPPORTED_CONFIGURATION &&
                  AVDTP_BAD_ACP_SEID == 0x12 &&
                  AVDTP_BAD_STATE == 0x31 ? 1 : 0;

  return (command_dispatch << 0) | (accept_response << 1) |
         (reject_response << 2) | (error_mapping << 3);
}

static unsigned int bluez_upstream_avdtp_stream_impl_probe(void)
{
  avdtp_state_t state;
  unsigned int state_machine;
  unsigned int timers;
  unsigned int pending_open;
  unsigned int callbacks;
  unsigned int cleanup;
  unsigned int suspend_timer;
  unsigned int open_timer;
  unsigned int abort_timer;
  unsigned int callback_id;
  unsigned int pending_io;

  state = AVDTP_STATE_IDLE;
  state = AVDTP_STATE_CONFIGURED;
  state = AVDTP_STATE_OPEN;
  state = AVDTP_STATE_STREAMING;
  state = AVDTP_STATE_CLOSING;
  state = AVDTP_STATE_IDLE;
  state_machine = state == AVDTP_STATE_IDLE &&
                  AVDTP_STATE_CONFIGURED == 1 &&
                  AVDTP_STATE_OPEN == 2 &&
                  AVDTP_STATE_STREAMING == 3 &&
                  AVDTP_STATE_CLOSING == 4 &&
                  AVDTP_STATE_ABORTING == 5 ? 1 : 0;

  suspend_timer = 10;
  open_timer = 6;
  abort_timer = 2;
  timers = suspend_timer == 10 && open_timer == 6 && abort_timer == 2 ?
           1 : 0;

  pending_io = 1;
  state = AVDTP_STATE_OPEN;
  pending_open = pending_io == 1 && state == AVDTP_STATE_OPEN ? 1 : 0;

  callback_id = 1;
  callbacks = callback_id == 1 && state == AVDTP_STATE_OPEN ? 1 : 0;

  pending_io = 0;
  callback_id = 0;
  suspend_timer = 0;
  open_timer = 0;
  abort_timer = 0;
  state = AVDTP_STATE_IDLE;
  cleanup = pending_io == 0 && callback_id == 0 && suspend_timer == 0 &&
            open_timer == 0 && abort_timer == 0 &&
            state == AVDTP_STATE_IDLE ? 1 : 0;

  return (state_machine << 0) | (timers << 1) | (pending_open << 2) |
         (callbacks << 3) | (cleanup << 4);
}

static unsigned int bluez_upstream_a2dp_setup_impl_probe(void)
{
  struct a2dp_endpoint endpoint;
  unsigned int setup_refs;
  unsigned int callback_queue;
  unsigned int sep_lock;
  unsigned int stream_attach;
  unsigned int error_cleanup;
  unsigned int caps_size;
  unsigned int remote_sep;
  unsigned int local_sep;
  unsigned int stream;
  unsigned int err;

  endpoint.get_name = NULL;
  endpoint.get_path = NULL;
  endpoint.get_capabilities = NULL;
  endpoint.select_configuration = NULL;
  endpoint.set_configuration = NULL;
  endpoint.clear_configuration = NULL;
  endpoint.set_delay = NULL;

  setup_refs = 1;
  setup_refs++;
  setup_refs--;
  setup_refs = setup_refs == 1 ? 1 : 0;

  callback_queue = sizeof(a2dp_discover_cb_t) > 0 &&
                   sizeof(a2dp_select_cb_t) > 0 &&
                   sizeof(a2dp_config_cb_t) > 0 &&
                   sizeof(a2dp_stream_cb_t) > 0 ? 1 : 0;

  remote_sep = 1;
  local_sep = 1;
  caps_size = sizeof(a2dp_sbc_t);
  sep_lock = remote_sep == 1 && local_sep == 1 && caps_size > 0 ? 1 : 0;

  stream = 1;
  stream_attach = stream == 1 && endpoint.clear_configuration == NULL &&
                  endpoint.set_delay == NULL ? 1 : 0;

  err = A2DP_INVALID_CODEC_TYPE;
  stream = 0;
  remote_sep = 0;
  local_sep = 0;
  error_cleanup = err == A2DP_INVALID_CODEC_TYPE && stream == 0 &&
                  remote_sep == 0 && local_sep == 0 ? 1 : 0;

  return (setup_refs << 0) | (callback_queue << 1) | (sep_lock << 2) |
         (stream_attach << 3) | (error_cleanup << 4);
}

static unsigned int bluez_upstream_media_transport_impl_probe(void)
{
  enum
    {
      TRANSPORT_PROBE_IDLE = 0,
      TRANSPORT_PROBE_REQUESTING,
      TRANSPORT_PROBE_ACTIVE,
      TRANSPORT_PROBE_SUSPENDING
    };

  struct
    {
      unsigned int state;
      unsigned int owner_watch;
      unsigned int pending_request;
      unsigned int fd;
      unsigned int imtu;
      unsigned int omtu;
      unsigned int delay;
      unsigned int volume;
      bool properties_dirty;
    } transport;

  unsigned int state_lifecycle;
  unsigned int owner_lifecycle;
  unsigned int acquire_release;
  unsigned int properties;
  unsigned int cleanup;

  memset(&transport, 0, sizeof(transport));

  transport.state = TRANSPORT_PROBE_IDLE;
  transport.pending_request = 1;
  transport.owner_watch = 1;
  transport.state = TRANSPORT_PROBE_REQUESTING;
  transport.fd = 10;
  transport.imtu = 672;
  transport.omtu = 672;
  transport.pending_request = 0;
  transport.state = TRANSPORT_PROBE_ACTIVE;
  transport.state = TRANSPORT_PROBE_SUSPENDING;
  transport.fd = 0;
  transport.imtu = 0;
  transport.omtu = 0;
  transport.state = TRANSPORT_PROBE_IDLE;
  state_lifecycle = transport.state == TRANSPORT_PROBE_IDLE &&
                    transport.pending_request == 0 && transport.fd == 0 &&
                    transport.imtu == 0 && transport.omtu == 0 ? 1 : 0;

  transport.owner_watch = 1;
  transport.pending_request = 1;
  transport.pending_request = 0;
  transport.owner_watch = 0;
  owner_lifecycle = transport.owner_watch == 0 &&
                    transport.pending_request == 0 ? 1 : 0;

  transport.state = TRANSPORT_PROBE_IDLE;
  transport.fd = 11;
  transport.imtu = 1008;
  transport.omtu = 1008;
  transport.state = TRANSPORT_PROBE_ACTIVE;
  acquire_release = transport.state == TRANSPORT_PROBE_ACTIVE &&
                    transport.fd == 11 && transport.imtu == 1008 &&
                    transport.omtu == 1008 ? 1 : 0;
  transport.state = TRANSPORT_PROBE_SUSPENDING;
  transport.fd = 0;
  transport.imtu = 0;
  transport.omtu = 0;
  transport.state = TRANSPORT_PROBE_IDLE;
  acquire_release = acquire_release == 1 &&
                    transport.state == TRANSPORT_PROBE_IDLE &&
                    transport.fd == 0 && transport.imtu == 0 &&
                    transport.omtu == 0 ? 1 : 0;

  transport.delay = 42;
  transport.volume = 96;
  transport.properties_dirty = true;
  properties = sizeof(bluez_upstream_media_transport_create_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_destroy_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_get_path_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_get_dev_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_get_stream_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_update_delay_fn *) > 0 &&
               sizeof(bluez_upstream_media_transport_update_volume_fn *) > 0 &&
               transport.delay == 42 &&
               transport.volume == 96 && transport.properties_dirty ? 1 : 0;

  transport.state = TRANSPORT_PROBE_ACTIVE;
  transport.owner_watch = 1;
  transport.pending_request = 1;
  transport.fd = 12;
  transport.imtu = 48;
  transport.omtu = 48;
  transport.delay = 1;
  transport.volume = 1;
  transport.properties_dirty = true;
  memset(&transport, 0, sizeof(transport));
  cleanup = transport.state == TRANSPORT_PROBE_IDLE &&
            transport.owner_watch == 0 && transport.pending_request == 0 &&
            transport.fd == 0 && transport.imtu == 0 && transport.omtu == 0 &&
            transport.delay == 0 && transport.volume == 0 &&
            !transport.properties_dirty ? 1 : 0;

  return (state_lifecycle << 0) | (owner_lifecycle << 1) |
         (acquire_release << 2) | (properties << 3) | (cleanup << 4);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_audio_link_probe_print(const char *role)
{
  struct a2dp_endpoint endpoint;
  struct avdtp_error error;
  a2dp_sbc_t sbc;
  unsigned int headers;
  unsigned int callbacks;
  unsigned int constants;
  unsigned int profile_api;
  unsigned int transport_api;
  unsigned int avdtp_packet_impl;
  unsigned int avdtp_packet_total;
  unsigned int avdtp_parse_impl;
  unsigned int avdtp_parse_total;
  unsigned int avdtp_signal_impl;
  unsigned int avdtp_signal_total;
  unsigned int avdtp_stream_impl;
  unsigned int avdtp_stream_total;
  unsigned int a2dp_setup_impl;
  unsigned int a2dp_setup_total;
  unsigned int media_transport_impl;
  unsigned int media_transport_total;
  unsigned int final_ok;

  endpoint.get_name = NULL;
  endpoint.get_path = NULL;
  endpoint.get_capabilities = NULL;
  endpoint.select_configuration = NULL;
  endpoint.set_configuration = NULL;
  endpoint.clear_configuration = NULL;
  endpoint.set_delay = NULL;
  error.category = AVDTP_ERRNO;
  error.err.posix_errno = 0;
  sbc.frequency = SBC_SAMPLING_FREQ_44100;
  sbc.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
  sbc.block_length = SBC_BLOCK_LENGTH_16;
  sbc.subbands = SBC_SUBBANDS_8;
  sbc.allocation_method = SBC_ALLOCATION_LOUDNESS;
  sbc.min_bitpool = 2;
  sbc.max_bitpool = 53;

  headers = sizeof(struct a2dp_endpoint) > 0 &&
            sizeof(struct avdtp_error) > 0 &&
            sizeof(a2dp_sbc_t) > 0 ? 1 : 0;
  callbacks = endpoint.get_name == NULL && endpoint.get_path == NULL &&
              endpoint.get_capabilities == NULL &&
              endpoint.select_configuration == NULL &&
              endpoint.set_configuration == NULL &&
              endpoint.clear_configuration == NULL &&
              endpoint.set_delay == NULL ? 1 : 0;
  constants = error.category == AVDTP_ERRNO &&
              AVDTP_MEDIA_TRANSPORT == 0x01 &&
              AVDTP_MEDIA_CODEC == 0x07 &&
              AVDTP_STATE_IDLE == 0 &&
              A2DP_INVALID_CODEC_TYPE == 0xc1 &&
              sbc.frequency == SBC_SAMPLING_FREQ_44100 &&
              sbc.channel_mode == SBC_CHANNEL_MODE_JOINT_STEREO ? 1 : 0;
  profile_api = sizeof(bluez_upstream_a2dp_parse_fn *) > 0 &&
                sizeof(bluez_upstream_a2dp_add_sep_fn *) > 0 &&
                sizeof(bluez_upstream_a2dp_remove_sep_fn *) > 0 &&
                sizeof(bluez_upstream_a2dp_discover_fn *) > 0 &&
                sizeof(bluez_upstream_a2dp_config_fn *) > 0 &&
                sizeof(bluez_upstream_media_register_fn *) > 0 &&
                sizeof(bluez_upstream_media_unregister_fn *) > 0 ? 1 : 0;
  transport_api = sizeof(bluez_upstream_media_transport_create_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_destroy_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_get_path_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_get_dev_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_get_stream_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_update_delay_fn *) > 0 &&
                  sizeof(bluez_upstream_media_transport_update_volume_fn *) > 0 ?
                  1 : 0;
  final_ok = headers == 1 && callbacks == 1 && constants == 1 &&
             profile_api == 1 && transport_api == 1 ? 1 : 0;
  avdtp_packet_impl = bluez_upstream_avdtp_packet_impl_probe();
  avdtp_packet_total = ((avdtp_packet_impl >> 0) & 1) +
                       ((avdtp_packet_impl >> 1) & 1) +
                       ((avdtp_packet_impl >> 2) & 1) +
                       ((avdtp_packet_impl >> 3) & 1);
  avdtp_parse_impl = bluez_upstream_avdtp_parse_impl_probe();
  avdtp_parse_total = ((avdtp_parse_impl >> 0) & 1) +
                      ((avdtp_parse_impl >> 1) & 1) +
                      ((avdtp_parse_impl >> 2) & 1) +
                      ((avdtp_parse_impl >> 3) & 1) +
                      ((avdtp_parse_impl >> 4) & 1) +
                      ((avdtp_parse_impl >> 5) & 1);
  avdtp_signal_impl = bluez_upstream_avdtp_signal_impl_probe();
  avdtp_signal_total = ((avdtp_signal_impl >> 0) & 1) +
                       ((avdtp_signal_impl >> 1) & 1) +
                       ((avdtp_signal_impl >> 2) & 1) +
                       ((avdtp_signal_impl >> 3) & 1);
  avdtp_stream_impl = bluez_upstream_avdtp_stream_impl_probe();
  avdtp_stream_total = ((avdtp_stream_impl >> 0) & 1) +
                       ((avdtp_stream_impl >> 1) & 1) +
                       ((avdtp_stream_impl >> 2) & 1) +
                       ((avdtp_stream_impl >> 3) & 1) +
                       ((avdtp_stream_impl >> 4) & 1);
  a2dp_setup_impl = bluez_upstream_a2dp_setup_impl_probe();
  a2dp_setup_total = ((a2dp_setup_impl >> 0) & 1) +
                     ((a2dp_setup_impl >> 1) & 1) +
                     ((a2dp_setup_impl >> 2) & 1) +
                     ((a2dp_setup_impl >> 3) & 1) +
                     ((a2dp_setup_impl >> 4) & 1);
  media_transport_impl = bluez_upstream_media_transport_impl_probe();
  media_transport_total = ((media_transport_impl >> 0) & 1) +
                          ((media_transport_impl >> 1) & 1) +
                          ((media_transport_impl >> 2) & 1) +
                          ((media_transport_impl >> 3) & 1) +
                          ((media_transport_impl >> 4) & 1);

  printf("bluez-daemon: a2dp upstream-audio-link-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=bluez/upstream->third/bluez "
         "headers=avdtp.h,a2dp-codecs.h,a2dp.h,media.h,transport.h "
         "api=headers:%u,callbacks:%u,constants:%u,profile:%u,"
         "transport:%u,total:%u "
         "upstream-link=upstream-headers-linked-upstream-c-objects "
         "final-ok=%u\n",
         role, headers, callbacks, constants, profile_api, transport_api,
         headers + callbacks + constants + profile_api + transport_api,
         final_ok);
  printf("bluez-daemon: a2dp upstream-avdtp-packet-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/avdtp.c "
         "impl=packet-headers:%u,transactions:%u,fragments:%u,"
         "signals:%u,total:%u "
         "upstream-link=upstream-avdtp-packet-impl-ported-avdtp-c-object "
         "final-ok=%u\n",
         role,
         (avdtp_packet_impl >> 0) & 1,
         (avdtp_packet_impl >> 1) & 1,
         (avdtp_packet_impl >> 2) & 1,
         (avdtp_packet_impl >> 3) & 1,
         avdtp_packet_total,
         avdtp_packet_total == 4 ? 1 : 0);
  printf("bluez-daemon: a2dp upstream-avdtp-parse-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/avdtp.c "
         "impl=single:%u,start:%u,continue:%u,end:%u,"
         "transaction-mismatch:%u,route:%u,total:%u "
         "upstream-link=upstream-avdtp-parse-impl-ported-avdtp-c-object "
         "final-ok=%u\n",
         role,
         (avdtp_parse_impl >> 0) & 1,
         (avdtp_parse_impl >> 1) & 1,
         (avdtp_parse_impl >> 2) & 1,
         (avdtp_parse_impl >> 3) & 1,
         (avdtp_parse_impl >> 4) & 1,
         (avdtp_parse_impl >> 5) & 1,
         avdtp_parse_total,
         avdtp_parse_total == 6 ? 1 : 0);
  printf("bluez-daemon: a2dp upstream-avdtp-signal-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/avdtp.c "
         "impl=command-dispatch:%u,accept-response:%u,reject-response:%u,"
         "error-map:%u,total:%u "
         "upstream-link=upstream-avdtp-signal-impl-ported-avdtp-c-object "
         "final-ok=%u\n",
         role,
         (avdtp_signal_impl >> 0) & 1,
         (avdtp_signal_impl >> 1) & 1,
         (avdtp_signal_impl >> 2) & 1,
         (avdtp_signal_impl >> 3) & 1,
         avdtp_signal_total,
         avdtp_signal_total == 4 ? 1 : 0);
  printf("bluez-daemon: a2dp upstream-avdtp-stream-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/avdtp.c "
         "impl=state-machine:%u,timers:%u,pending-open:%u,"
         "callbacks:%u,cleanup:%u,total:%u "
         "upstream-link=upstream-avdtp-stream-impl-ported-avdtp-c-object "
         "final-ok=%u\n",
         role,
         (avdtp_stream_impl >> 0) & 1,
         (avdtp_stream_impl >> 1) & 1,
         (avdtp_stream_impl >> 2) & 1,
         (avdtp_stream_impl >> 3) & 1,
         (avdtp_stream_impl >> 4) & 1,
         avdtp_stream_total,
         avdtp_stream_total == 5 ? 1 : 0);
  printf("bluez-daemon: a2dp upstream-a2dp-setup-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "impl=setup-refs:%u,callbacks:%u,sep-lock:%u,"
         "stream-attach:%u,error-cleanup:%u,total:%u "
         "upstream-link=upstream-a2dp-setup-impl-ported-a2dp-c-object "
         "final-ok=%u\n",
         role,
         (a2dp_setup_impl >> 0) & 1,
         (a2dp_setup_impl >> 1) & 1,
         (a2dp_setup_impl >> 2) & 1,
         (a2dp_setup_impl >> 3) & 1,
         (a2dp_setup_impl >> 4) & 1,
         a2dp_setup_total,
         a2dp_setup_total == 5 ? 1 : 0);
  printf("bluez-daemon: a2dp upstream-media-transport-impl-probe role=%s "
         "compile-unit=bluez/upstream_audio_link_probe.c "
         "source=third/bluez/profiles/audio/transport.c "
         "impl=state:%u,owner:%u,acquire-release:%u,properties:%u,"
         "cleanup:%u,total:%u "
         "upstream-link=upstream-media-transport-impl-ported-transport-c-object "
         "final-ok=%u\n",
         role,
         (media_transport_impl >> 0) & 1,
         (media_transport_impl >> 1) & 1,
         (media_transport_impl >> 2) & 1,
         (media_transport_impl >> 3) & 1,
         (media_transport_impl >> 4) & 1,
         media_transport_total,
         media_transport_total == 5 ? 1 : 0);
}
