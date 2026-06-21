/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_a2dp_object_probe.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef BLUEZ_UPSTREAM_A2DP_OBJECT_PROBE_H
#define BLUEZ_UPSTREAM_A2DP_OBJECT_PROBE_H

#include <stdint.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void bluez_upstream_a2dp_object_probe_print(const char *role);
struct btd_adapter;
struct btd_device;
struct a2dp_sep;
struct avdtp;
struct avdtp_stream;
struct a2dp_setup;
unsigned int
bluez_upstream_a2dp_object_adapter_profiles_ready(
  struct btd_adapter *adapter);
unsigned int
bluez_upstream_a2dp_object_prepare_resume_stream(
  struct btd_device *device, struct a2dp_sep *sep);
struct avdtp *
bluez_upstream_a2dp_object_get_prepared_resume_session(void);
unsigned int
bluez_upstream_a2dp_object_prepare_resume_setup(
  struct btd_device *device, struct a2dp_sep *sep);
unsigned int
bluez_upstream_a2dp_object_mark_prepared_streaming(void);
unsigned int
bluez_upstream_a2dp_object_prepare_stream_media_transport(
  struct avdtp_stream *stream, int fd, uint16_t imtu, uint16_t omtu);
unsigned int
bluez_upstream_a2dp_object_get_stream_media_transport(
  struct avdtp_stream *stream, int *fd, uint16_t *imtu, uint16_t *omtu);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_synthetic_setup(
  struct btd_device *device, const char *remote_path);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_prepared_setup(
  struct btd_device *device, struct a2dp_sep *sep,
  const char *remote_path);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_registered_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_parsed_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_getcap_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_dispatch_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_packet_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_session_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_discover_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
struct a2dp_setup *
bluez_upstream_a2dp_object_create_l2cap_remote_setup(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_setup_matches(
  struct a2dp_setup *setup, struct btd_device *device,
  const char *remote_path);
const char *
bluez_upstream_a2dp_object_setup_remote_path(struct a2dp_setup *setup);
unsigned int
bluez_upstream_a2dp_object_registered_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_parsed_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_getcap_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_dispatch_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_packet_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_session_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_discover_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);
unsigned int
bluez_upstream_a2dp_object_l2cap_remote_lookup_ready(
  struct btd_device *device, struct a2dp_sep *sep, uint8_t seid);

#endif /* BLUEZ_UPSTREAM_A2DP_OBJECT_PROBE_H */
