/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_media_owner_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct a2dp_sep;
struct btd_adapter;
struct btd_device;
struct media_endpoint;
struct media_transport;

/****************************************************************************
 * External Upstream Object Symbols
 ****************************************************************************/

extern struct a2dp_sep *bluez_upstream_object_media_endpoint_get_sep(
  struct media_endpoint *endpoint);
extern const char *bluez_upstream_object_media_endpoint_get_uuid(
  struct media_endpoint *endpoint);
extern bool bluez_upstream_object_media_endpoint_get_delay_reporting(
  struct media_endpoint *endpoint);
extern uint8_t bluez_upstream_object_media_endpoint_get_codec(
  struct media_endpoint *endpoint);
extern struct btd_adapter *
bluez_upstream_object_media_endpoint_get_btd_adapter(
  struct media_endpoint *endpoint);
extern bool bluez_upstream_object_media_endpoint_is_broadcast(
  struct media_endpoint *endpoint);
extern struct btd_device *bluez_upstream_object_media_transport_get_dev(
  struct media_transport *transport);
extern void bluez_upstream_object_media_transport_set_a2dp_volume(
  struct btd_device *dev, int volume);

/****************************************************************************
 * Public Upstream ABI Forwarders
 ****************************************************************************/

struct a2dp_sep *media_endpoint_get_sep(struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_get_sep(endpoint);
}

const char *media_endpoint_get_uuid(struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_get_uuid(endpoint);
}

bool media_endpoint_get_delay_reporting(struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_get_delay_reporting(endpoint);
}

uint8_t media_endpoint_get_codec(struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_get_codec(endpoint);
}

struct btd_adapter *media_endpoint_get_btd_adapter(
  struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_get_btd_adapter(endpoint);
}

bool media_endpoint_is_broadcast(struct media_endpoint *endpoint)
{
  return bluez_upstream_object_media_endpoint_is_broadcast(endpoint);
}

struct btd_device *media_transport_get_dev(struct media_transport *transport)
{
  return bluez_upstream_object_media_transport_get_dev(transport);
}

void media_transport_set_a2dp_volume(struct btd_device *dev, int volume)
{
  bluez_upstream_object_media_transport_set_a2dp_volume(dev, volume);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_media_owner_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/media-owner role=%s linked=1 "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "owner=bluetoothd api=media-endpoint-transport-forwarders\n",
         role);
}
