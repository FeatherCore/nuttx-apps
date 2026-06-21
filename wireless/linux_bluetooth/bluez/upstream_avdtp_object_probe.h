/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_avdtp_object_probe.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_AVDTP_OBJECT_PROBE_H
#define APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_AVDTP_OBJECT_PROBE_H

#include <stddef.h>
#include <stdint.h>

void bluez_upstream_avdtp_object_probe_print(const char *role);
unsigned int bluez_upstream_avdtp_control_dependency_bound(void);
struct btd_device;
struct avdtp;
struct avdtp_remote_sep;
struct avdtp *
bluez_upstream_avdtp_object_create_synthetic_session(
  struct btd_device *device);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_raw_caps(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_getcap_response(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_parse_response(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_parse_data(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_session_cb(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_discover(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);
struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_l2cap_connect(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size);

#endif
