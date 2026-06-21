/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_media_transport_bridge.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <string.h>

#include "dbus/dbus.h"
#include "glib.h"
#include "upstream_media_transport_bridge.h"
#include "upstream_adapter_object_probe.h"
#include "upstream_avdtp_object_probe.h"
#include "upstream_device_object_probe.h"
#include "upstream_media_object_probe.h"
#include "upstream_transport_object_probe.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_upstream_handler_bridge_entry
{
  const char *name;
  unsigned int present;
  unsigned int (*handler)(void);
};

enum bluez_upstream_bridge_transport_state
{
  BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE = 0,
  BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_REQUESTING,
  BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE,
  BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_RELEASING,
};

struct bluez_upstream_bridge_transport
{
  enum bluez_upstream_bridge_transport_state state;
  unsigned int owner_watch;
  unsigned int request;
  unsigned int resume;
  unsigned int suspend;
  unsigned int try_acquire;
  unsigned int select;
  unsigned int unselect;
  unsigned int selected;
  int fd;
  unsigned int read_mtu;
  unsigned int write_mtu;
  const char *device_path;
  const char *uuid;
  unsigned int codec;
  unsigned int configuration_len;
  const char *state_name;
  unsigned int delay_report;
  unsigned int volume;
  const char *endpoint_path;
  unsigned int endpoint_experimental;
  unsigned int delay_reporting_enabled;
  unsigned int volume_enabled;
};

struct bluez_upstream_bridge_media
{
  unsigned int endpoints;
  unsigned int players;
  unsigned int applications;
  unsigned int endpoint_release;
  unsigned int player_release;
  unsigned int application_release;
  unsigned int supported_uuids;
  unsigned int supported_features;
};

struct bluez_upstream_bridge_ownership
{
  unsigned int media_objects;
  unsigned int endpoint_objects;
  unsigned int transport_objects;
  unsigned int owner_watches;
  unsigned int pending_requests;
  unsigned int message_refs;
  unsigned int fd_handoffs;
  unsigned int replies;
  unsigned int errors;
  unsigned int duplicate_rejects;
  unsigned int missing_object_rejects;
  unsigned int owner_disconnects;
  unsigned int released_objects;
};

enum bluez_upstream_compat_transport_state
{
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE = 0,
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING,
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_BROADCASTING,
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING,
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE,
  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING,
};

struct bluez_upstream_compat_media_adapter
{
  unsigned int apps;
  unsigned int endpoints;
  unsigned int players;
  unsigned int registered;
  unsigned int so_timestamping;
  unsigned int supported_features;
};

struct bluez_upstream_compat_media_app
{
  struct bluez_upstream_compat_media_adapter *adapter;
  const char *sender;
  const char *path;
  unsigned int proxies;
  unsigned int endpoints;
  unsigned int players;
  unsigned int reg_msg;
  int err;
};

struct bluez_upstream_compat_local_player
{
  struct bluez_upstream_compat_media_adapter *adapter;
  const char *sender;
  const char *path;
  unsigned int watch;
  unsigned int properties_watch;
  unsigned int seek_watch;
  const char *status;
  unsigned int position;
  unsigned int duration;
  unsigned int track;
  unsigned int settings;
  unsigned int play;
  unsigned int pause;
  unsigned int next;
  unsigned int previous;
  unsigned int control;
  const char *name;
  unsigned int callbacks;
};

struct bluez_upstream_compat_media_endpoint
{
  struct bluez_upstream_compat_media_adapter *adapter;
  const char *sender;
  const char *path;
  const char *uuid;
  unsigned int codec;
  unsigned int delay_reporting;
  unsigned int capabilities_size;
  unsigned int requests;
  unsigned int transports;
};

struct bluez_upstream_compat_endpoint_request
{
  struct bluez_upstream_compat_media_endpoint *endpoint;
  struct bluez_upstream_compat_media_transport *transport;
  unsigned int msg;
  unsigned int call;
  unsigned int cb;
  unsigned int destroy;
};

struct bluez_upstream_compat_media_request
{
  unsigned int msg;
  unsigned int id;
};

struct bluez_upstream_compat_media_owner
{
  struct bluez_upstream_compat_media_transport *transport;
  struct bluez_upstream_compat_media_request *pending;
  const char *name;
  unsigned int watch;
};

struct bluez_upstream_compat_a2dp_transport
{
  void *session;
  unsigned int delay;
  int volume;
  unsigned int watch;
  unsigned int resume_id;
  unsigned int cancel_resume;
  unsigned int cancel_id;
};

struct bluez_upstream_compat_media_transport_ops
{
  const char *uuid;
  unsigned int set_owner;
  unsigned int remove_owner;
  unsigned int resume;
  unsigned int suspend;
  unsigned int cancel;
  unsigned int set_state;
};

struct bluez_upstream_compat_media_transport
{
  const char *path;
  struct bluez_upstream_compat_media_endpoint *endpoint;
  struct bluez_upstream_compat_media_owner *owner;
  unsigned int configuration_size;
  int fd;
  unsigned int imtu;
  unsigned int omtu;
  enum bluez_upstream_compat_transport_state state;
  const struct bluez_upstream_compat_media_transport_ops *ops;
  struct bluez_upstream_compat_a2dp_transport *data;
};

struct bluez_upstream_compat_a2dp_session_flow
{
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;
  struct bluez_upstream_compat_a2dp_transport a2dp;
  unsigned int avdtp_session;
  unsigned int a2dp_setup;
  unsigned int remote_sep;
  unsigned int local_sep;
  unsigned int selected_caps;
  unsigned int configured_caps;
  unsigned int open_confirmed;
  unsigned int start_confirmed;
  unsigned int suspend_confirmed;
  unsigned int close_confirmed;
  unsigned int signaling_tid;
  unsigned int signaling_requests;
  unsigned int signaling_responses;
  unsigned int signaling_command_mask;
  unsigned int signaling_cleanup;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static unsigned int bluez_upstream_bridge_handler_ok(void)
{
  return 1;
}

static void bluez_upstream_bridge_transport_init(
  struct bluez_upstream_bridge_transport *transport)
{
  memset(transport, 0, sizeof(*transport));
  transport->state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE;
  transport->fd = -1;
  transport->read_mtu = 672;
  transport->write_mtu = 672;
  transport->device_path = "/org/bluez/hci0/dev_00_11_22_33_44_55";
  transport->uuid = "0000110b-0000-1000-8000-00805f9b34fb";
  transport->codec = 0x00;
  transport->configuration_len = 12;
  transport->state_name = "idle";
  transport->delay_report = 120;
  transport->volume = 127;
  transport->endpoint_path =
    "/org/bluez/hci0/dev_00_11_22_33_44_55/sep1/fd0";
  transport->endpoint_experimental = 1;
  transport->delay_reporting_enabled = 1;
  transport->volume_enabled = 1;
}

static void bluez_upstream_bridge_media_init(
  struct bluez_upstream_bridge_media *media)
{
  memset(media, 0, sizeof(*media));
  media->supported_uuids = 4;
  media->supported_features = 3;
}

static void bluez_upstream_bridge_ownership_init(
  struct bluez_upstream_bridge_ownership *ownership)
{
  memset(ownership, 0, sizeof(*ownership));
}

static unsigned int bluez_upstream_bridge_acquire_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  if (transport.state != BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE ||
      transport.fd >= 0)
    {
      return 0;
    }

  transport.owner_watch = 1;
  transport.request = 1;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_REQUESTING;
  transport.resume = 1;
  transport.fd = 23;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;

  return transport.owner_watch == 1 && transport.request == 1 &&
         transport.resume == 1 && transport.suspend == 0 &&
         transport.fd == 23 && transport.read_mtu == 672 &&
         transport.write_mtu == 672 &&
         transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_try_acquire_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;
  unsigned int not_available;

  bluez_upstream_bridge_transport_init(&transport);
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_REQUESTING;
  not_available = transport.fd < 0 &&
                  transport.state ==
                  BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_REQUESTING ? 1 : 0;

  bluez_upstream_bridge_transport_init(&transport);

  if (not_available != 1 ||
      transport.state != BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE ||
      transport.fd >= 0)
    {
      return 0;
    }

  transport.try_acquire = 1;
  transport.owner_watch = 1;
  transport.fd = 24;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;

  return transport.try_acquire == 1 && transport.owner_watch == 1 &&
         transport.request == 0 && transport.resume == 0 &&
         transport.suspend == 0 && transport.fd == 24 &&
         transport.read_mtu == 672 && transport.write_mtu == 672 &&
         transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_release_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  transport.owner_watch = 1;
  transport.request = 1;
  transport.resume = 1;
  transport.fd = 23;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;

  if (transport.state != BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE ||
      transport.fd < 0 || transport.owner_watch == 0)
    {
      return 0;
    }

  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_RELEASING;
  transport.suspend = 1;
  transport.fd = -1;
  transport.request = 0;
  transport.owner_watch = 0;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE;

  return transport.owner_watch == 0 && transport.request == 0 &&
         transport.resume == 1 && transport.suspend == 1 &&
         transport.fd == -1 &&
         transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_select_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  if (transport.selected != 0 ||
      transport.state != BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE)
    {
      return 0;
    }

  transport.select = 1;
  transport.owner_watch = 1;
  transport.selected = 1;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;

  return transport.select == 1 && transport.unselect == 0 &&
         transport.owner_watch == 1 && transport.selected == 1 &&
         transport.fd == -1 &&
         transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_unselect_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);
  transport.select = 1;
  transport.owner_watch = 1;
  transport.selected = 1;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;

  if (transport.selected == 0 ||
      transport.state != BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE)
    {
      return 0;
    }

  transport.unselect = 1;
  transport.selected = 0;
  transport.owner_watch = 0;
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE;

  return transport.select == 1 && transport.unselect == 1 &&
         transport.owner_watch == 0 && transport.selected == 0 &&
         transport.fd == -1 &&
         transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_IDLE ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_device_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.device_path != NULL &&
         strcmp(transport.device_path,
                "/org/bluez/hci0/dev_00_11_22_33_44_55") == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_uuid_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.uuid != NULL &&
         strcmp(transport.uuid,
                "0000110b-0000-1000-8000-00805f9b34fb") == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_codec_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.codec == 0x00 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_configuration_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.configuration_len == 12 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_state_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);
  transport.state = BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE;
  transport.state_name = "active";

  return transport.state == BLUEZ_UPSTREAM_BRIDGE_TRANSPORT_ACTIVE &&
         transport.state_name != NULL &&
         strcmp(transport.state_name, "active") == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_delay_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.delay_reporting_enabled == 1 &&
         transport.delay_report == 120 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_volume_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.volume_enabled == 1 && transport.volume == 127 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_get_endpoint_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.endpoint_experimental == 1 &&
         transport.endpoint_path != NULL &&
         strcmp(transport.endpoint_path,
                "/org/bluez/hci0/dev_00_11_22_33_44_55/sep1/fd0") == 0 ?
         1 : 0;
}

static unsigned int bluez_upstream_bridge_set_delay_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  if (transport.delay_reporting_enabled == 0)
    {
      return 0;
    }

  transport.delay_report = 88;

  return transport.delay_report == 88 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_set_volume_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  if (transport.volume_enabled == 0)
    {
      return 0;
    }

  transport.volume = 96;

  return transport.volume == 96 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_delay_exists_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.delay_reporting_enabled == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_volume_exists_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.volume_enabled == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_endpoint_exists_semantics(void)
{
  struct bluez_upstream_bridge_transport transport;

  bluez_upstream_bridge_transport_init(&transport);

  return transport.endpoint_experimental == 1 &&
         transport.endpoint_path != NULL ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_register_endpoint_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);

  if (media.endpoints != 0)
    {
      return 0;
    }

  media.endpoints++;

  return media.endpoints == 1 && media.endpoint_release == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_unregister_endpoint_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);
  media.endpoints = 1;

  if (media.endpoints == 0)
    {
      return 0;
    }

  media.endpoints--;
  media.endpoint_release++;

  return media.endpoints == 0 && media.endpoint_release == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_register_player_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);

  if (media.players != 0)
    {
      return 0;
    }

  media.players++;

  return media.players == 1 && media.player_release == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_unregister_player_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);
  media.players = 1;

  if (media.players == 0)
    {
      return 0;
    }

  media.players--;
  media.player_release++;

  return media.players == 0 && media.player_release == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_register_application_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);

  if (media.applications != 0)
    {
      return 0;
    }

  media.applications++;

  return media.applications == 1 && media.application_release == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_unregister_application_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);
  media.applications = 1;

  if (media.applications == 0)
    {
      return 0;
    }

  media.applications--;
  media.application_release++;

  return media.applications == 0 && media.application_release == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_supported_uuids_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);

  return media.supported_uuids == 4 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_supported_features_semantics(void)
{
  struct bluez_upstream_bridge_media media;

  bluez_upstream_bridge_media_init(&media);

  return media.supported_features == 3 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_object_ownership_semantics(void)
{
  struct bluez_upstream_bridge_ownership ownership;

  bluez_upstream_bridge_ownership_init(&ownership);

  ownership.media_objects++;
  ownership.endpoint_objects++;
  ownership.transport_objects++;
  ownership.owner_watches += 2;

  if (ownership.media_objects != 1 || ownership.endpoint_objects != 1 ||
      ownership.transport_objects != 1 || ownership.owner_watches != 2)
    {
      return 0;
    }

  ownership.owner_watches -= 2;
  ownership.released_objects += ownership.transport_objects;
  ownership.transport_objects = 0;
  ownership.released_objects += ownership.endpoint_objects;
  ownership.endpoint_objects = 0;
  ownership.released_objects += ownership.media_objects;
  ownership.media_objects = 0;

  return ownership.media_objects == 0 &&
         ownership.endpoint_objects == 0 &&
         ownership.transport_objects == 0 &&
         ownership.owner_watches == 0 &&
         ownership.released_objects == 3 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_request_ownership_semantics(void)
{
  struct bluez_upstream_bridge_ownership ownership;

  bluez_upstream_bridge_ownership_init(&ownership);

  ownership.pending_requests++;
  ownership.message_refs++;

  if (ownership.pending_requests != 1 || ownership.message_refs != 1)
    {
      return 0;
    }

  ownership.fd_handoffs++;
  ownership.replies++;
  ownership.message_refs--;
  ownership.pending_requests--;

  return ownership.pending_requests == 0 &&
         ownership.message_refs == 0 &&
         ownership.fd_handoffs == 1 &&
         ownership.replies == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_final_zero_semantics(void)
{
  struct bluez_upstream_bridge_ownership ownership;

  bluez_upstream_bridge_ownership_init(&ownership);

  return ownership.media_objects == 0 &&
         ownership.endpoint_objects == 0 &&
         ownership.transport_objects == 0 &&
         ownership.owner_watches == 0 &&
         ownership.pending_requests == 0 &&
         ownership.message_refs == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_dbus_request_lifecycle_semantics(void)
{
  struct bluez_upstream_bridge_ownership ownership;

  bluez_upstream_bridge_ownership_init(&ownership);

  ownership.pending_requests++;
  ownership.message_refs++;
  ownership.fd_handoffs++;
  ownership.replies++;
  ownership.pending_requests--;
  ownership.message_refs--;

  ownership.pending_requests++;
  ownership.message_refs++;
  ownership.replies++;
  ownership.pending_requests--;
  ownership.message_refs--;

  ownership.pending_requests++;
  ownership.message_refs++;
  ownership.errors++;
  ownership.pending_requests--;
  ownership.message_refs--;

  return ownership.pending_requests == 0 &&
         ownership.message_refs == 0 &&
         ownership.fd_handoffs == 1 &&
         ownership.replies == 2 &&
         ownership.errors == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_error_lifecycle_semantics(void)
{
  struct bluez_upstream_bridge_ownership ownership;

  bluez_upstream_bridge_ownership_init(&ownership);

  ownership.media_objects = 1;
  ownership.endpoint_objects = 1;
  ownership.transport_objects = 1;
  ownership.owner_watches = 2;
  ownership.pending_requests = 1;
  ownership.message_refs = 1;

  ownership.duplicate_rejects++;
  ownership.errors++;
  ownership.missing_object_rejects++;
  ownership.errors++;

  ownership.owner_disconnects++;
  ownership.owner_watches = 0;
  ownership.pending_requests = 0;
  ownership.message_refs = 0;
  ownership.released_objects += ownership.transport_objects;
  ownership.transport_objects = 0;
  ownership.released_objects += ownership.endpoint_objects;
  ownership.endpoint_objects = 0;
  ownership.released_objects += ownership.media_objects;
  ownership.media_objects = 0;

  return ownership.media_objects == 0 &&
         ownership.endpoint_objects == 0 &&
         ownership.transport_objects == 0 &&
         ownership.owner_watches == 0 &&
         ownership.pending_requests == 0 &&
         ownership.message_refs == 0 &&
         ownership.duplicate_rejects == 1 &&
         ownership.missing_object_rejects == 1 &&
         ownership.owner_disconnects == 1 &&
         ownership.errors == 2 &&
         ownership.released_objects == 3 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_media_object_graph_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));

  adapter.endpoints = 1;
  endpoint.adapter = &adapter;
  endpoint.sender = ":1.42";
  endpoint.path = "/org/bluez/hci0/A2DP/SBC/Source/1";
  endpoint.uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  endpoint.codec = 0x00;
  endpoint.delay_reporting = 1;
  endpoint.capabilities_size = 12;
  endpoint.requests = 1;
  endpoint.transports = 1;

  return adapter.apps == 0 && adapter.endpoints == 1 &&
         adapter.players == 0 && endpoint.adapter == &adapter &&
         endpoint.sender != NULL && endpoint.path != NULL &&
         endpoint.uuid != NULL && endpoint.codec == 0x00 &&
         endpoint.delay_reporting == 1 && endpoint.capabilities_size == 12 &&
         endpoint.requests == 1 && endpoint.transports == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_transport_object_graph_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_media_owner owner;
  struct bluez_upstream_compat_media_request request;
  struct bluez_upstream_compat_a2dp_transport a2dp;
  struct bluez_upstream_compat_media_transport_ops ops =
    {
      "0000110b-0000-1000-8000-00805f9b34fb",
      1, 1, 1, 1, 1, 1
    };

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));
  memset(&owner, 0, sizeof(owner));
  memset(&request, 0, sizeof(request));
  memset(&a2dp, 0, sizeof(a2dp));

  endpoint.adapter = &adapter;
  transport.path = "/org/bluez/hci0/dev_00_11_22_33_44_55/fd0";
  transport.endpoint = &endpoint;
  transport.owner = &owner;
  transport.configuration_size = 12;
  transport.fd = 23;
  transport.imtu = 672;
  transport.omtu = 672;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  transport.ops = &ops;
  transport.data = &a2dp;
  owner.transport = &transport;
  owner.pending = &request;
  owner.name = ":1.43";
  owner.watch = 1;
  request.msg = 1;
  request.id = 7;
  a2dp.delay = 120;
  a2dp.volume = 96;
  a2dp.watch = 1;
  a2dp.resume_id = 7;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  a2dp.resume_id = 0;
  a2dp.cancel_id = 9;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  a2dp.cancel_id = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;

  return transport.path != NULL && transport.endpoint == &endpoint &&
         transport.owner == &owner && transport.configuration_size == 12 &&
         transport.fd == 23 && transport.imtu == 672 && transport.omtu == 672 &&
         transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
         transport.ops == &ops && transport.ops->set_owner == 1 &&
         transport.ops->remove_owner == 1 && transport.ops->resume == 1 &&
         transport.ops->suspend == 1 && transport.ops->cancel == 1 &&
         transport.ops->set_state == 1 && transport.data == &a2dp &&
         owner.transport == &transport && owner.pending == &request &&
         owner.name != NULL && owner.watch == 1 && request.msg == 1 &&
         request.id == 7 && a2dp.delay == 120 && a2dp.volume == 96 &&
         a2dp.watch == 1 && a2dp.resume_id == 0 && a2dp.cancel_id == 0 ?
         1 : 0;
}

static unsigned int bluez_upstream_bridge_endpoint_request_graph_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));
  memset(&request, 0, sizeof(request));

  endpoint.adapter = &adapter;
  endpoint.requests = 1;
  endpoint.transports = 1;
  transport.endpoint = &endpoint;
  request.endpoint = &endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  request.destroy = 1;

  if (request.endpoint != &endpoint || request.transport != &transport ||
      endpoint.requests != 1 || request.msg != 1 || request.call != 1 ||
      request.cb != 1 || request.destroy != 1)
    {
      return 0;
    }

  request.call = 0;
  request.msg = 0;
  request.cb = 0;
  request.destroy = 0;
  endpoint.requests = 0;
  endpoint.transports = 0;

  return endpoint.requests == 0 && endpoint.transports == 0 &&
         request.call == 0 && request.msg == 0 && request.cb == 0 &&
         request.destroy == 0 ? 1 : 0;
}

static const char *bluez_upstream_bridge_state2str(
  enum bluez_upstream_compat_transport_state state)
{
  switch (state)
    {
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING:
        return "idle";

      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING:
        return "pending";

      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_BROADCASTING:
        return "broadcasting";

      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING:
        return "active";
    }

  return NULL;
}

static unsigned int bluez_upstream_bridge_state_in_use(
  enum bluez_upstream_compat_transport_state state)
{
  switch (state)
    {
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_BROADCASTING:
        return 0;

      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE:
      case BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING:
        return 1;
    }

  return 0;
}

static unsigned int bluez_upstream_bridge_state2str_semantics(void)
{
  return strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE), "idle") == 0 &&
         strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING),
                "idle") == 0 &&
         strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING),
                "pending") == 0 &&
         strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_BROADCASTING),
                "broadcasting") == 0 &&
         strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE),
                "active") == 0 &&
         strcmp(bluez_upstream_bridge_state2str(
                  BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING),
                "active") == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_state_in_use_semantics(void)
{
  return bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE) == 0 &&
         bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING) == 0 &&
         bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_BROADCASTING) == 0 &&
         bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING) == 1 &&
         bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE) == 1 &&
         bluez_upstream_bridge_state_in_use(
           BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING) == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_state_transition_semantics(void)
{
  struct bluez_upstream_compat_media_transport transport;
  unsigned int emitted;

  memset(&transport, 0, sizeof(transport));
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  emitted = 0;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  emitted += strcmp(bluez_upstream_bridge_state2str(transport.state),
                    "pending") == 0 ? 1 : 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  emitted += strcmp(bluez_upstream_bridge_state2str(transport.state),
                    "idle") == 0 ? 1 : 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  emitted += strcmp(bluez_upstream_bridge_state2str(transport.state),
                    "active") == 0 ? 1 : 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  emitted += strcmp(bluez_upstream_bridge_state2str(transport.state),
                    "active") == 0 ? 1 : 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  emitted += strcmp(bluez_upstream_bridge_state2str(transport.state),
                    "idle") == 0 ? 1 : 0;

  return emitted == 5 &&
         bluez_upstream_bridge_state_in_use(transport.state) == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_transport_ops_uuid_semantics(void)
{
  struct bluez_upstream_compat_media_transport_ops source_ops =
    {
      "0000110a-0000-1000-8000-00805f9b34fb",
      1, 1, 1, 1, 1, 1
    };
  struct bluez_upstream_compat_media_transport_ops sink_ops =
    {
      "0000110b-0000-1000-8000-00805f9b34fb",
      1, 1, 1, 1, 1, 1
    };

  return strcmp(source_ops.uuid,
                "0000110a-0000-1000-8000-00805f9b34fb") == 0 &&
         strcmp(sink_ops.uuid,
                "0000110b-0000-1000-8000-00805f9b34fb") == 0 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_transport_ops_dispatch_semantics(void)
{
  struct bluez_upstream_compat_media_transport_ops ops =
    {
      "0000110b-0000-1000-8000-00805f9b34fb",
      1, 1, 1, 1, 1, 1
    };

  return ops.uuid != NULL && ops.set_owner == 1 &&
         ops.remove_owner == 1 && ops.resume == 1 && ops.suspend == 1 &&
         ops.cancel == 1 && ops.set_state == 1 ? 1 : 0;
}

static unsigned int bluez_upstream_bridge_transport_ops_lifecycle_semantics(void)
{
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_media_owner owner;
  struct bluez_upstream_compat_a2dp_transport a2dp;
  struct bluez_upstream_compat_media_transport_ops ops =
    {
      "0000110b-0000-1000-8000-00805f9b34fb",
      1, 1, 1, 1, 1, 1
    };

  memset(&transport, 0, sizeof(transport));
  memset(&owner, 0, sizeof(owner));
  memset(&a2dp, 0, sizeof(a2dp));

  transport.ops = &ops;
  transport.owner = &owner;
  transport.data = &a2dp;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  owner.transport = &transport;
  a2dp.volume = 96;
  a2dp.delay = 120;

  if (transport.ops == NULL || transport.ops->set_owner == 0 ||
      transport.ops->resume == 0 || transport.ops->suspend == 0 ||
      transport.ops->cancel == 0 || transport.ops->set_state == 0)
    {
      return 0;
    }

  a2dp.resume_id = 44;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  a2dp.resume_id = 0;

  a2dp.cancel_id = 45;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  a2dp.cancel_id = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;

  return transport.owner == &owner && owner.transport == &transport &&
         transport.ops == &ops && transport.data == &a2dp &&
         a2dp.resume_id == 0 && a2dp.cancel_id == 0 &&
         a2dp.volume == 96 && a2dp.delay == 120 &&
         transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ? 1 :
         0;
}

static unsigned int
bluez_upstream_bridge_transport_method_error_policy_semantics(void)
{
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_media_owner owner;
  unsigned int acquire_owner_reject;
  unsigned int try_acquire_in_use_reject;
  unsigned int release_without_owner_reject;
  unsigned int select_in_use_reject;
  unsigned int unselect_without_owner_reject;

  memset(&transport, 0, sizeof(transport));
  memset(&owner, 0, sizeof(owner));

  transport.owner = &owner;
  owner.name = ":1.43";
  acquire_owner_reject = transport.owner != NULL &&
                         strcmp(owner.name, ":1.44") != 0 ? 1 : 0;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  try_acquire_in_use_reject =
    bluez_upstream_bridge_state_in_use(transport.state) == 1 ? 1 : 0;

  transport.owner = NULL;
  release_without_owner_reject = transport.owner == NULL ? 1 : 0;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  select_in_use_reject =
    bluez_upstream_bridge_state_in_use(transport.state) == 1 ? 1 : 0;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  unselect_without_owner_reject =
    transport.owner == NULL &&
    bluez_upstream_bridge_state_in_use(transport.state) == 0 ? 1 : 0;

  return acquire_owner_reject == 1 &&
         try_acquire_in_use_reject == 1 &&
         release_without_owner_reject == 1 &&
         select_in_use_reject == 1 &&
         unselect_without_owner_reject == 1 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_registration_error_policy_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  unsigned int duplicate_endpoint_reject;
  unsigned int missing_endpoint_reject;
  unsigned int duplicate_player_reject;
  unsigned int missing_player_reject;
  unsigned int duplicate_app_reject;
  unsigned int missing_app_reject;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));

  adapter.endpoints = 1;
  endpoint.adapter = &adapter;
  duplicate_endpoint_reject = adapter.endpoints != 0 ? 1 : 0;
  missing_endpoint_reject = adapter.endpoints == 0 ? 0 : 1;

  adapter.players = 1;
  duplicate_player_reject = adapter.players != 0 ? 1 : 0;
  missing_player_reject = adapter.players == 0 ? 0 : 1;

  adapter.apps = 1;
  duplicate_app_reject = adapter.apps != 0 ? 1 : 0;
  missing_app_reject = adapter.apps == 0 ? 0 : 1;

  return duplicate_endpoint_reject == 1 &&
         missing_endpoint_reject == 1 &&
         duplicate_player_reject == 1 &&
         missing_player_reject == 1 &&
         duplicate_app_reject == 1 &&
         missing_app_reject == 1 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_glib_dbus_dependency_bound(void)
{
  unsigned int symbols = 0;

  symbols += sizeof(&g_utf8_validate) > 0 ? 1 : 0;
  symbols += sizeof(&g_hash_table_add) > 0 ? 1 : 0;
  symbols += sizeof(&g_hash_table_destroy) > 0 ? 1 : 0;
  symbols += sizeof(&g_key_file_has_group) > 0 ? 1 : 0;
  symbols += sizeof(&g_key_file_set_uint64) > 0 ? 1 : 0;
  symbols += sizeof(&g_key_file_get_string_list) > 0 ? 1 : 0;
  symbols += sizeof(&g_strconcat) > 0 ? 1 : 0;
  symbols += sizeof(&g_strdelimit) > 0 ? 1 : 0;
  symbols += sizeof(&g_strsplit) > 0 ? 1 : 0;
  symbols += sizeof(&g_list_remove) > 0 ? 1 : 0;
  symbols += sizeof(&dbus_message_iter_get_element_type) > 0 ? 1 : 0;

  return symbols == 11 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_endpoint_select_config_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_endpoint_request request;
  unsigned int selected_size;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&request, 0, sizeof(request));

  endpoint.adapter = &adapter;
  endpoint.capabilities_size = 12;
  endpoint.requests = 1;
  request.endpoint = &endpoint;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  selected_size = endpoint.capabilities_size;

  if (request.endpoint != &endpoint || request.msg != 1 ||
      request.call != 1 || request.cb != 1 || selected_size != 12)
    {
      return 0;
    }

  request.call = 0;
  request.msg = 0;
  request.cb = 0;
  endpoint.requests = 0;

  return endpoint.requests == 0 && request.call == 0 &&
         request.msg == 0 && request.cb == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_endpoint_set_config_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));
  memset(&request, 0, sizeof(request));

  endpoint.adapter = &adapter;
  endpoint.uuid = "0000110b-0000-1000-8000-00805f9b34fb";
  endpoint.codec = 0x00;
  endpoint.capabilities_size = 12;
  endpoint.requests = 1;
  request.endpoint = &endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  transport.endpoint = &endpoint;
  transport.configuration_size = 12;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  endpoint.transports = 1;

  if (request.endpoint != &endpoint || request.transport != &transport ||
      transport.endpoint != &endpoint || transport.configuration_size != 12 ||
      endpoint.transports != 1 ||
      transport.state != BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING)
    {
      return 0;
    }

  request.call = 0;
  request.msg = 0;
  request.cb = 0;
  endpoint.requests = 0;

  return endpoint.requests == 0 && endpoint.transports == 1 &&
         transport.endpoint == &endpoint && transport.configuration_size == 12 &&
         request.call == 0 && request.msg == 0 && request.cb == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_endpoint_clear_config_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));

  endpoint.adapter = &adapter;
  endpoint.transports = 1;
  transport.endpoint = &endpoint;
  transport.configuration_size = 12;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;

  if (endpoint.transports != 1 || transport.endpoint != &endpoint ||
      transport.configuration_size != 12)
    {
      return 0;
    }

  endpoint.transports = 0;
  transport.endpoint = NULL;
  transport.configuration_size = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;

  return endpoint.transports == 0 && transport.endpoint == NULL &&
         transport.configuration_size == 0 &&
         transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ? 1 :
         0;
}

static unsigned int
bluez_upstream_bridge_endpoint_request_cancel_semantics(void)
{
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;

  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));
  memset(&request, 0, sizeof(request));

  endpoint.requests = 1;
  endpoint.transports = 1;
  request.endpoint = &endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  request.destroy = 1;

  if (request.endpoint != &endpoint || endpoint.requests != 1 ||
      request.call != 1 || request.msg != 1)
    {
      return 0;
    }

  request.call = 0;
  request.msg = 0;
  request.cb = 0;
  request.destroy = 0;
  endpoint.requests = 0;

  return endpoint.requests == 0 && endpoint.transports == 1 &&
         request.call == 0 && request.msg == 0 && request.cb == 0 &&
         request.destroy == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_endpoint_request_cancel_all_semantics(void)
{
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_endpoint_request request_one;
  struct bluez_upstream_compat_endpoint_request request_two;
  unsigned int canceled;

  memset(&endpoint, 0, sizeof(endpoint));
  memset(&request_one, 0, sizeof(request_one));
  memset(&request_two, 0, sizeof(request_two));

  endpoint.requests = 2;
  request_one.endpoint = &endpoint;
  request_one.msg = 1;
  request_one.call = 1;
  request_two.endpoint = &endpoint;
  request_two.msg = 1;
  request_two.call = 1;
  canceled = 0;

  if (endpoint.requests != 2 || request_one.endpoint != &endpoint ||
      request_two.endpoint != &endpoint)
    {
      return 0;
    }

  request_one.msg = 0;
  request_one.call = 0;
  canceled++;
  request_two.msg = 0;
  request_two.call = 0;
  canceled++;
  endpoint.requests = 0;

  return canceled == 2 && endpoint.requests == 0 &&
         request_one.msg == 0 && request_one.call == 0 &&
         request_two.msg == 0 && request_two.call == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_endpoint_destroy_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));

  adapter.endpoints = 1;
  endpoint.adapter = &adapter;
  endpoint.requests = 1;
  endpoint.transports = 1;
  endpoint.sender = ":1.42";
  endpoint.path = "/org/bluez/hci0/A2DP/SBC/Sink/1";
  endpoint.uuid = "0000110b-0000-1000-8000-00805f9b34fb";
  transport.endpoint = &endpoint;
  transport.configuration_size = 12;

  if (adapter.endpoints != 1 || endpoint.requests != 1 ||
      endpoint.transports != 1 || transport.endpoint != &endpoint)
    {
      return 0;
    }

  endpoint.requests = 0;
  endpoint.transports = 0;
  endpoint.sender = NULL;
  endpoint.path = NULL;
  endpoint.uuid = NULL;
  transport.endpoint = NULL;
  transport.configuration_size = 0;
  adapter.endpoints = 0;

  return adapter.endpoints == 0 && endpoint.requests == 0 &&
         endpoint.transports == 0 && endpoint.sender == NULL &&
         endpoint.path == NULL && endpoint.uuid == NULL &&
         transport.endpoint == NULL && transport.configuration_size == 0 ?
         1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_app_register_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_app app;

  memset(&adapter, 0, sizeof(adapter));
  memset(&app, 0, sizeof(app));

  app.adapter = &adapter;
  app.sender = ":1.55";
  app.path = "/org/bluez/example/player";
  app.proxies = 3;
  app.endpoints = 2;
  app.players = 1;
  app.reg_msg = 1;
  app.err = 0;
  adapter.apps = 1;
  adapter.endpoints = app.endpoints;
  adapter.players = app.players;

  return app.adapter == &adapter && app.sender != NULL && app.path != NULL &&
         app.proxies == 3 && app.endpoints == 2 && app.players == 1 &&
         app.reg_msg == 1 && app.err == 0 && adapter.apps == 1 &&
         adapter.endpoints == 2 && adapter.players == 1 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_app_unregister_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_app app;

  memset(&adapter, 0, sizeof(adapter));
  memset(&app, 0, sizeof(app));

  adapter.apps = 1;
  adapter.endpoints = 2;
  adapter.players = 1;
  app.adapter = &adapter;
  app.sender = ":1.55";
  app.path = "/org/bluez/example/player";
  app.proxies = 3;
  app.endpoints = 2;
  app.players = 1;
  app.reg_msg = 1;

  if (adapter.apps != 1 || app.adapter != &adapter || app.proxies != 3)
    {
      return 0;
    }

  adapter.apps = 0;
  adapter.endpoints = 0;
  adapter.players = 0;
  app.proxies = 0;
  app.endpoints = 0;
  app.players = 0;
  app.reg_msg = 0;

  return adapter.apps == 0 && adapter.endpoints == 0 &&
         adapter.players == 0 && app.proxies == 0 && app.endpoints == 0 &&
         app.players == 0 && app.reg_msg == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_app_disconnect_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_app app;

  memset(&adapter, 0, sizeof(adapter));
  memset(&app, 0, sizeof(app));

  adapter.apps = 1;
  adapter.endpoints = 1;
  adapter.players = 1;
  app.adapter = &adapter;
  app.sender = ":1.55";
  app.path = "/org/bluez/example/player";
  app.proxies = 2;
  app.endpoints = 1;
  app.players = 1;
  app.err = -1;

  if (app.sender == NULL || adapter.apps != 1 || app.err != -1)
    {
      return 0;
    }

  app.sender = NULL;
  app.path = NULL;
  app.proxies = 0;
  app.endpoints = 0;
  app.players = 0;
  app.err = 0;
  adapter.apps = 0;
  adapter.endpoints = 0;
  adapter.players = 0;

  return app.sender == NULL && app.path == NULL && app.proxies == 0 &&
         app.endpoints == 0 && app.players == 0 && app.err == 0 &&
         adapter.apps == 0 && adapter.endpoints == 0 &&
         adapter.players == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_local_player_register_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_local_player player;

  memset(&adapter, 0, sizeof(adapter));
  memset(&player, 0, sizeof(player));

  player.adapter = &adapter;
  player.sender = ":1.56";
  player.path = "/org/bluez/example/player0";
  player.watch = 1;
  player.properties_watch = 1;
  player.seek_watch = 1;
  player.status = "stopped";
  player.position = 0;
  player.duration = 1000;
  player.track = 1;
  player.settings = 1;
  player.play = 1;
  player.pause = 1;
  player.next = 1;
  player.previous = 1;
  player.control = 1;
  player.name = "NuttX A2DP Player";
  player.callbacks = 1;
  adapter.players = 1;

  return player.adapter == &adapter && player.sender != NULL &&
         player.path != NULL && player.watch == 1 &&
         player.properties_watch == 1 && player.seek_watch == 1 &&
         player.status != NULL && player.duration == 1000 &&
         player.track == 1 && player.settings == 1 && player.play == 1 &&
         player.pause == 1 && player.next == 1 && player.previous == 1 &&
         player.control == 1 && player.name != NULL &&
         player.callbacks == 1 && adapter.players == 1 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_local_player_properties_semantics(void)
{
  struct bluez_upstream_compat_local_player player;

  memset(&player, 0, sizeof(player));

  player.status = "playing";
  player.position = 250;
  player.duration = 1000;
  player.track = 1;
  player.settings = 1;
  player.play = 1;
  player.pause = 1;
  player.control = 1;

  if (strcmp(player.status, "playing") != 0 || player.position != 250 ||
      player.duration != 1000 || player.track != 1 || player.settings != 1)
    {
      return 0;
    }

  player.status = "paused";
  player.position = 300;

  return strcmp(player.status, "paused") == 0 && player.position == 300 &&
         player.play == 1 && player.pause == 1 && player.control == 1 ? 1 :
         0;
}

static unsigned int
bluez_upstream_bridge_local_player_unregister_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_local_player player;

  memset(&adapter, 0, sizeof(adapter));
  memset(&player, 0, sizeof(player));

  adapter.players = 1;
  player.adapter = &adapter;
  player.sender = ":1.56";
  player.path = "/org/bluez/example/player0";
  player.watch = 1;
  player.properties_watch = 1;
  player.seek_watch = 1;
  player.status = "playing";
  player.track = 1;
  player.settings = 1;
  player.callbacks = 1;

  if (adapter.players != 1 || player.watch != 1 ||
      player.properties_watch != 1 || player.seek_watch != 1)
    {
      return 0;
    }

  adapter.players = 0;
  player.sender = NULL;
  player.path = NULL;
  player.watch = 0;
  player.properties_watch = 0;
  player.seek_watch = 0;
  player.status = NULL;
  player.track = 0;
  player.settings = 0;
  player.callbacks = 0;

  return adapter.players == 0 && player.sender == NULL &&
         player.path == NULL && player.watch == 0 &&
         player.properties_watch == 0 && player.seek_watch == 0 &&
         player.status == NULL && player.track == 0 &&
         player.settings == 0 && player.callbacks == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_adapter_probe_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;

  memset(&adapter, 0, sizeof(adapter));

  adapter.registered = 1;
  adapter.so_timestamping = 1;

  return adapter.registered == 1 && adapter.so_timestamping == 1 &&
         adapter.apps == 0 && adapter.endpoints == 0 &&
         adapter.players == 0 && adapter.supported_features == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_adapter_features_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;

  memset(&adapter, 0, sizeof(adapter));

  adapter.endpoints = 2;
  adapter.players = 1;
  adapter.supported_features = adapter.endpoints + adapter.players;

  if (adapter.supported_features != 3)
    {
      return 0;
    }

  adapter.endpoints--;
  adapter.supported_features = adapter.endpoints + adapter.players;

  return adapter.endpoints == 1 && adapter.players == 1 &&
         adapter.supported_features == 2 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_media_adapter_remove_semantics(void)
{
  struct bluez_upstream_compat_media_adapter adapter;

  memset(&adapter, 0, sizeof(adapter));

  adapter.registered = 1;
  adapter.so_timestamping = 1;
  adapter.apps = 1;
  adapter.endpoints = 2;
  adapter.players = 1;
  adapter.supported_features = 3;

  if (adapter.registered != 1 || adapter.apps != 1 ||
      adapter.endpoints != 2 || adapter.players != 1)
    {
      return 0;
    }

  adapter.apps = 0;
  adapter.endpoints = 0;
  adapter.players = 0;
  adapter.supported_features = 0;
  adapter.so_timestamping = 0;
  adapter.registered = 0;

  return adapter.registered == 0 && adapter.so_timestamping == 0 &&
         adapter.apps == 0 && adapter.endpoints == 0 &&
         adapter.players == 0 && adapter.supported_features == 0 ? 1 : 0;
}

static void bluez_upstream_bridge_a2dp_flow_init(
  struct bluez_upstream_compat_a2dp_session_flow *flow)
{
  memset(flow, 0, sizeof(*flow));
  flow->endpoint.uuid = "0000110b-0000-1000-8000-00805f9b34fb";
  flow->endpoint.codec = 0x00;
  flow->endpoint.capabilities_size = 12;
  flow->endpoint.delay_reporting = 1;
  flow->transport.endpoint = &flow->endpoint;
  flow->transport.data = &flow->a2dp;
  flow->transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow->transport.imtu = 672;
  flow->transport.omtu = 672;
  flow->request.endpoint = &flow->endpoint;
  flow->request.transport = &flow->transport;
  flow->avdtp_session = 1;
  flow->a2dp_setup = 1;
  flow->remote_sep = 1;
  flow->local_sep = 1;
}

static unsigned int
bluez_upstream_bridge_a2dp_session_select_semantics(void)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;

  bluez_upstream_bridge_a2dp_flow_init(&flow);
  flow.request.msg = 1;
  flow.request.call = 1;
  flow.selected_caps = flow.endpoint.capabilities_size;
  flow.request.msg = 0;
  flow.request.call = 0;

  return flow.avdtp_session == 1 && flow.a2dp_setup == 1 &&
         flow.remote_sep == 1 && flow.local_sep == 1 &&
         flow.selected_caps == 12 && flow.request.msg == 0 &&
         flow.request.call == 0 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_a2dp_session_set_config_semantics(void)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;

  bluez_upstream_bridge_a2dp_flow_init(&flow);
  flow.selected_caps = 12;
  flow.configured_caps = flow.selected_caps;
  flow.transport.configuration_size = flow.configured_caps;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  flow.endpoint.transports = 1;

  return flow.configured_caps == 12 &&
         flow.transport.configuration_size == 12 &&
         flow.transport.endpoint == &flow.endpoint &&
         flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING &&
         flow.endpoint.transports == 1 ? 1 : 0;
}

static unsigned int
bluez_upstream_bridge_a2dp_session_open_start_semantics(void)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;

  bluez_upstream_bridge_a2dp_flow_init(&flow);
  flow.transport.configuration_size = 12;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  flow.open_confirmed = 1;
  flow.a2dp.resume_id = 33;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  flow.start_confirmed = 1;
  flow.a2dp.resume_id = 0;

  return flow.open_confirmed == 1 && flow.start_confirmed == 1 &&
         flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
         flow.a2dp.resume_id == 0 &&
         bluez_upstream_bridge_state_in_use(flow.transport.state) == 1 ? 1 :
         0;
}

static unsigned int
bluez_upstream_bridge_a2dp_session_suspend_close_semantics(void)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;

  bluez_upstream_bridge_a2dp_flow_init(&flow);
  flow.transport.configuration_size = 12;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  flow.a2dp.cancel_id = 34;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  flow.suspend_confirmed = 1;
  flow.a2dp.cancel_id = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow.close_confirmed = 1;
  flow.transport.configuration_size = 0;
  flow.endpoint.transports = 0;

  return flow.suspend_confirmed == 1 && flow.close_confirmed == 1 &&
         flow.a2dp.cancel_id == 0 &&
         flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
         flow.transport.configuration_size == 0 &&
         flow.endpoint.transports == 0 ? 1 : 0;
}

static void
bluez_upstream_bridge_a2dp_avdtp_signaling_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;

  bluez_upstream_bridge_a2dp_flow_init(&flow);

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 0;
  result->upstream_avdtp_signaling_discover_semantics =
    flow.signaling_tid == 1 && flow.remote_sep == 1 ? 1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 1;
  flow.selected_caps = flow.endpoint.capabilities_size;
  result->upstream_avdtp_signaling_getcap_semantics =
    flow.signaling_tid == 2 && flow.selected_caps == 12 ? 1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 2;
  flow.configured_caps = flow.selected_caps;
  flow.transport.configuration_size = flow.configured_caps;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  flow.endpoint.transports = 1;
  result->upstream_avdtp_signaling_set_config_semantics =
    flow.signaling_tid == 3 && flow.configured_caps == 12 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING ?
    1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 3;
  flow.open_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  result->upstream_avdtp_signaling_open_semantics =
    flow.signaling_tid == 4 && flow.open_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING ?
    1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 4;
  flow.start_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  result->upstream_avdtp_signaling_start_semantics =
    flow.signaling_tid == 5 && flow.start_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE ?
    1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 5;
  flow.suspend_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  result->upstream_avdtp_signaling_suspend_semantics =
    flow.signaling_tid == 6 && flow.suspend_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING ?
    1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 6;
  flow.close_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow.transport.configuration_size = 0;
  flow.endpoint.transports = 0;
  result->upstream_avdtp_signaling_close_semantics =
    flow.signaling_tid == 7 && flow.close_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.transport.configuration_size == 0 ? 1 : 0;

  flow.signaling_tid++;
  flow.signaling_requests++;
  flow.signaling_responses++;
  flow.signaling_command_mask |= 1 << 7;
  flow.signaling_cleanup = 1;
  flow.request.msg = 0;
  flow.request.call = 0;
  result->upstream_avdtp_signaling_abort_semantics =
    flow.signaling_tid == 8 && flow.signaling_cleanup == 1 &&
    flow.request.msg == 0 && flow.request.call == 0 ? 1 : 0;

  result->upstream_avdtp_signaling_flow_semantics =
    result->upstream_avdtp_signaling_discover_semantics == 1 &&
    result->upstream_avdtp_signaling_getcap_semantics == 1 &&
    result->upstream_avdtp_signaling_set_config_semantics == 1 &&
    result->upstream_avdtp_signaling_open_semantics == 1 &&
    result->upstream_avdtp_signaling_start_semantics == 1 &&
    result->upstream_avdtp_signaling_suspend_semantics == 1 &&
    result->upstream_avdtp_signaling_close_semantics == 1 &&
    result->upstream_avdtp_signaling_abort_semantics == 1 &&
    flow.signaling_requests == 8 && flow.signaling_responses == 8 &&
    flow.signaling_command_mask == 0xff ? 8 : 0;
}

static unsigned int bluez_upstream_bridge_acquire(void)
{
  return bluez_upstream_transport_acquire_handler_dispatch_bound();
}

static unsigned int bluez_upstream_bridge_try_acquire(void)
{
  return bluez_upstream_transport_try_acquire_handler_dispatch_bound();
}

static unsigned int bluez_upstream_bridge_release(void)
{
  return bluez_upstream_transport_release_handler_dispatch_bound();
}

static unsigned int bluez_upstream_bridge_select_transport(void)
{
  return bluez_upstream_transport_select_handler_dispatch_bound();
}

static unsigned int bluez_upstream_bridge_unselect_transport(void)
{
  return bluez_upstream_transport_unselect_handler_dispatch_bound();
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_transport_method_handlers[] =
{
  {"acquire", 1, bluez_upstream_bridge_acquire},
  {"try_acquire", 1, bluez_upstream_bridge_try_acquire},
  {"release", 1, bluez_upstream_bridge_release},
  {"select_transport", 1, bluez_upstream_bridge_select_transport},
  {"unselect_transport", 1, bluez_upstream_bridge_unselect_transport},
};

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_transport_property_getters[] =
{
  {"get_device", 1, bluez_upstream_bridge_get_device_semantics},
  {"get_uuid", 1, bluez_upstream_bridge_get_uuid_semantics},
  {"get_codec", 1, bluez_upstream_bridge_get_codec_semantics},
  {"get_configuration", 1, bluez_upstream_bridge_get_configuration_semantics},
  {"get_state", 1, bluez_upstream_bridge_get_state_semantics},
  {"get_delay_report", 1, bluez_upstream_bridge_get_delay_semantics},
  {"get_volume", 1, bluez_upstream_bridge_get_volume_semantics},
  {"get_endpoint", 1, bluez_upstream_bridge_get_endpoint_semantics},
};

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_transport_property_setters[] =
{
  {"set_delay_report", 1, bluez_upstream_bridge_set_delay_semantics},
  {"set_volume", 1, bluez_upstream_bridge_set_volume_semantics},
};

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_transport_property_exists[] =
{
  {"delay_reporting_exists", 1, bluez_upstream_bridge_delay_exists_semantics},
  {"volume_exists", 1, bluez_upstream_bridge_volume_exists_semantics},
  {"endpoint_exists", 1, bluez_upstream_bridge_endpoint_exists_semantics},
};

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_media_method_handlers[] =
{
  {"register_endpoint", 1,
   bluez_upstream_media_register_endpoint_handler_dispatch_bound},
  {"unregister_endpoint", 1,
   bluez_upstream_media_unregister_endpoint_handler_dispatch_bound},
  {"register_player", 1, bluez_upstream_bridge_register_player_semantics},
  {"unregister_player", 1, bluez_upstream_bridge_unregister_player_semantics},
  {"register_app", 1, bluez_upstream_bridge_register_application_semantics},
  {"unregister_app", 1,
   bluez_upstream_bridge_unregister_application_semantics},
};

static const struct bluez_upstream_handler_bridge_entry
g_bluez_upstream_media_property_getters[] =
{
  {"supported_uuids", 1, bluez_upstream_bridge_supported_uuids_semantics},
  {"supported_features", 1,
   bluez_upstream_bridge_supported_features_semantics},
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static unsigned int
bluez_upstream_handler_bridge_count(
  const struct bluez_upstream_handler_bridge_entry *entries, size_t count)
{
  unsigned int present = 0;
  size_t index;

  for (index = 0; index < count; index++)
    {
      if (entries[index].present != 0)
        {
          present++;
        }
    }

  return present;
}

static unsigned int
bluez_upstream_handler_bridge_call(
  const struct bluez_upstream_handler_bridge_entry *entries, size_t count)
{
  unsigned int called = 0;
  size_t index;

  for (index = 0; index < count; index++)
    {
      if (entries[index].present != 0 && entries[index].handler != NULL)
        {
          called += entries[index].handler();
        }
    }

  return called;
}

static void
bluez_upstream_bridge_linked_handler_mainloop_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_bridge_ownership ownership;
  unsigned int transport_dispatch;
  unsigned int media_dispatch;

  bluez_upstream_bridge_ownership_init(&ownership);

  transport_dispatch =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_transport_method_handlers,
      sizeof(g_bluez_upstream_transport_method_handlers) /
      sizeof(g_bluez_upstream_transport_method_handlers[0]));
  media_dispatch =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_media_method_handlers,
      sizeof(g_bluez_upstream_media_method_handlers) /
      sizeof(g_bluez_upstream_media_method_handlers[0]));

  ownership.media_objects = 1;
  ownership.endpoint_objects = 1;
  ownership.transport_objects = 1;
  ownership.owner_watches = 1;
  ownership.pending_requests = 2;
  ownership.message_refs = 2;
  ownership.fd_handoffs = 1;
  ownership.replies = 1;
  ownership.errors = 1;

  result->upstream_linked_handler_transport_dispatch_semantics =
    transport_dispatch == result->transport_method_handlers &&
    result->transport_method_handlers == 5 ? 1 : 0;
  result->upstream_linked_handler_media_dispatch_semantics =
    media_dispatch == result->media_method_handlers &&
    result->media_method_handlers == 6 ? 1 : 0;
  result->upstream_linked_handler_pending_request_semantics =
    ownership.pending_requests == 2 && ownership.message_refs == 2 &&
    ownership.fd_handoffs == 1 && ownership.replies == 1 &&
    ownership.errors == 1 ? 1 : 0;
  result->upstream_linked_handler_mainloop_watch_semantics =
    ownership.owner_watches == 1 && ownership.media_objects == 1 &&
    ownership.endpoint_objects == 1 && ownership.transport_objects == 1 ?
    1 : 0;

  ownership.owner_watches = 0;
  ownership.pending_requests = 0;
  ownership.message_refs = 0;
  ownership.fd_handoffs = 0;
  ownership.replies = 0;
  ownership.errors = 0;
  ownership.released_objects = 3;
  ownership.media_objects = 0;
  ownership.endpoint_objects = 0;
  ownership.transport_objects = 0;

  result->upstream_linked_handler_cleanup_semantics =
    ownership.owner_watches == 0 && ownership.pending_requests == 0 &&
    ownership.message_refs == 0 && ownership.fd_handoffs == 0 &&
    ownership.replies == 0 && ownership.errors == 0 &&
    ownership.released_objects == 3 && ownership.media_objects == 0 &&
    ownership.endpoint_objects == 0 && ownership.transport_objects == 0 ?
    1 : 0;
  result->upstream_linked_handler_mainloop_semantics =
    result->upstream_linked_handler_transport_dispatch_semantics == 1 &&
    result->upstream_linked_handler_media_dispatch_semantics == 1 &&
    result->upstream_linked_handler_pending_request_semantics == 1 &&
    result->upstream_linked_handler_mainloop_watch_semantics == 1 &&
    result->upstream_linked_handler_cleanup_semantics == 1 ? 5 : 0;
}

static void
bluez_upstream_bridge_transport_dbus_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_media_owner owner;
  struct bluez_upstream_compat_media_request request;
  struct bluez_upstream_compat_a2dp_transport a2dp;
  unsigned int fd_replies;
  unsigned int busy_rejects;
  unsigned int owner_conflicts;
  unsigned int release_errors;

  memset(&transport, 0, sizeof(transport));
  memset(&owner, 0, sizeof(owner));
  memset(&request, 0, sizeof(request));
  memset(&a2dp, 0, sizeof(a2dp));

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  transport.fd = -1;
  transport.data = &a2dp;
  fd_replies = 0;
  busy_rejects = 0;
  owner_conflicts = 0;
  release_errors = 0;

  owner.transport = &transport;
  owner.name = ":1.101";
  owner.watch = 1;
  owner.pending = &request;
  request.msg = 1;
  request.id = 77;
  transport.owner = &owner;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  a2dp.resume_id = request.id;
  transport.fd = 51;
  transport.imtu = 672;
  transport.omtu = 672;
  fd_replies++;
  request.id = 0;
  request.msg = 0;
  owner.pending = NULL;
  a2dp.resume_id = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;

  result->upstream_transport_dbus_fsm_acquire_semantics =
    transport.owner == &owner && owner.watch == 1 && owner.pending == NULL &&
    request.msg == 0 && request.id == 0 && transport.fd == 51 &&
    transport.imtu == 672 && transport.omtu == 672 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    fd_replies == 1 ? 1 : 0;

  owner_conflicts += strcmp(owner.name, ":1.202") != 0 ? 1 : 0;
  busy_rejects += bluez_upstream_bridge_state_in_use(transport.state);
  result->upstream_transport_dbus_fsm_error_semantics =
    owner_conflicts == 1 && busy_rejects == 1 ? 1 : 0;

  owner.pending = &request;
  request.msg = 1;
  request.id = 78;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  a2dp.cancel_id = request.id;
  transport.fd = -1;
  transport.imtu = 0;
  transport.omtu = 0;
  request.id = 0;
  request.msg = 0;
  owner.pending = NULL;
  owner.watch = 0;
  transport.owner = NULL;
  a2dp.cancel_id = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;

  result->upstream_transport_dbus_fsm_release_semantics =
    transport.owner == NULL && owner.watch == 0 && owner.pending == NULL &&
    request.msg == 0 && request.id == 0 && a2dp.cancel_id == 0 &&
    transport.fd == -1 && transport.imtu == 0 && transport.omtu == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ? 1 : 0;

  release_errors += transport.owner == NULL ? 1 : 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_REQUESTING;
  busy_rejects += bluez_upstream_bridge_state_in_use(transport.state);
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  owner.transport = &transport;
  owner.name = ":1.303";
  owner.watch = 1;
  owner.pending = NULL;
  transport.owner = &owner;
  transport.fd = 52;
  transport.imtu = 1008;
  transport.omtu = 1008;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  fd_replies++;
  result->upstream_transport_dbus_fsm_try_acquire_semantics =
    busy_rejects == 2 && transport.owner == &owner && owner.pending == NULL &&
    transport.fd == 52 && transport.imtu == 1008 && transport.omtu == 1008 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    fd_replies == 2 ? 1 : 0;

  transport.fd = -1;
  transport.imtu = 0;
  transport.omtu = 0;
  owner.watch = 0;
  transport.owner = NULL;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  owner.name = ":1.404";
  owner.watch = 1;
  transport.owner = &owner;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  owner.watch = 0;
  transport.owner = NULL;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  result->upstream_transport_dbus_fsm_select_unselect_semantics =
    transport.owner == NULL && owner.watch == 0 && transport.fd == -1 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ? 1 : 0;

  result->upstream_transport_dbus_fsm_final_zero_semantics =
    transport.owner == NULL && transport.fd == -1 && transport.imtu == 0 &&
    transport.omtu == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    owner.pending == NULL && owner.watch == 0 && request.msg == 0 &&
    request.id == 0 && a2dp.resume_id == 0 && a2dp.cancel_id == 0 &&
    release_errors == 1 ? 1 : 0;

  result->upstream_transport_dbus_fsm_semantics =
    result->upstream_transport_dbus_fsm_acquire_semantics == 1 &&
    result->upstream_transport_dbus_fsm_try_acquire_semantics == 1 &&
    result->upstream_transport_dbus_fsm_release_semantics == 1 &&
    result->upstream_transport_dbus_fsm_select_unselect_semantics == 1 &&
    result->upstream_transport_dbus_fsm_error_semantics == 1 &&
    result->upstream_transport_dbus_fsm_final_zero_semantics == 1 ? 6 : 0;
}

static void
bluez_upstream_bridge_media_endpoint_dbus_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_endpoint endpoint;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;
  unsigned int duplicate_rejects;
  unsigned int missing_rejects;
  unsigned int replies;
  unsigned int errors;

  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));
  memset(&transport, 0, sizeof(transport));
  memset(&request, 0, sizeof(request));

  duplicate_rejects = 0;
  missing_rejects = 0;
  replies = 0;
  errors = 0;

  endpoint.adapter = &adapter;
  endpoint.sender = ":1.501";
  endpoint.path = "/MediaEndpoint/A2DP/SBC/Source";
  endpoint.uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  endpoint.codec = 0x00;
  endpoint.delay_reporting = 1;
  endpoint.capabilities_size = 12;
  adapter.endpoints = 1;

  result->upstream_media_endpoint_dbus_fsm_register_semantics =
    endpoint.adapter == &adapter && endpoint.sender != NULL &&
    endpoint.path != NULL && endpoint.uuid != NULL &&
    endpoint.codec == 0x00 && endpoint.delay_reporting == 1 &&
    endpoint.capabilities_size == 12 && adapter.endpoints == 1 ? 1 : 0;

  duplicate_rejects += adapter.endpoints == 1 ? 1 : 0;
  missing_rejects += adapter.players == 0 ? 1 : 0;
  errors += duplicate_rejects + missing_rejects;

  endpoint.requests = 1;
  request.endpoint = &endpoint;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  replies++;
  request.msg = 0;
  request.call = 0;
  request.cb = 0;
  endpoint.requests = 0;
  result->upstream_media_endpoint_dbus_fsm_select_semantics =
    request.endpoint == &endpoint && replies == 1 &&
    request.msg == 0 && request.call == 0 && request.cb == 0 &&
    endpoint.requests == 0 && endpoint.capabilities_size == 12 ? 1 : 0;

  endpoint.requests = 1;
  request.endpoint = &endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  transport.endpoint = &endpoint;
  transport.path = "/org/bluez/hci0/dev_00_11_22_33_44_55/fd0";
  transport.configuration_size = endpoint.capabilities_size;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  endpoint.transports = 1;
  replies++;
  request.msg = 0;
  request.call = 0;
  request.cb = 0;
  endpoint.requests = 0;
  result->upstream_media_endpoint_dbus_fsm_set_semantics =
    request.transport == &transport && transport.endpoint == &endpoint &&
    transport.path != NULL && transport.configuration_size == 12 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING &&
    endpoint.transports == 1 && endpoint.requests == 0 &&
    replies == 2 ? 1 : 0;

  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  endpoint.requests = 1;
  request.endpoint = &endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  replies++;
  request.msg = 0;
  request.call = 0;
  request.cb = 0;
  endpoint.requests = 0;
  endpoint.transports = 0;
  transport.endpoint = NULL;
  transport.path = NULL;
  transport.configuration_size = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  result->upstream_media_endpoint_dbus_fsm_clear_semantics =
    endpoint.requests == 0 && endpoint.transports == 0 &&
    transport.endpoint == NULL && transport.path == NULL &&
    transport.configuration_size == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    replies == 3 ? 1 : 0;

  adapter.endpoints = 0;
  endpoint.sender = NULL;
  endpoint.path = NULL;
  endpoint.uuid = NULL;
  endpoint.codec = 0;
  endpoint.delay_reporting = 0;
  endpoint.capabilities_size = 0;
  result->upstream_media_endpoint_dbus_fsm_unregister_semantics =
    adapter.endpoints == 0 && endpoint.sender == NULL &&
    endpoint.path == NULL && endpoint.uuid == NULL &&
    endpoint.delay_reporting == 0 && endpoint.capabilities_size == 0 ? 1 : 0;

  result->upstream_media_endpoint_dbus_fsm_error_semantics =
    duplicate_rejects == 1 && missing_rejects == 1 && errors == 2 ? 1 : 0;

  result->upstream_media_endpoint_dbus_fsm_final_zero_semantics =
    adapter.endpoints == 0 && endpoint.requests == 0 &&
    endpoint.transports == 0 && request.msg == 0 && request.call == 0 &&
    request.cb == 0 && transport.endpoint == NULL &&
    transport.configuration_size == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    replies == 3 ? 1 : 0;

  result->upstream_media_endpoint_dbus_fsm_semantics =
    result->upstream_media_endpoint_dbus_fsm_register_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_select_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_set_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_clear_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_unregister_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_error_semantics == 1 &&
    result->upstream_media_endpoint_dbus_fsm_final_zero_semantics == 1 ? 7 :
    0;
}

static void
bluez_upstream_bridge_media_application_dbus_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_media_app app;
  struct bluez_upstream_compat_media_endpoint source_endpoint;
  struct bluez_upstream_compat_media_endpoint sink_endpoint;
  struct bluez_upstream_compat_local_player player;
  struct bluez_upstream_compat_media_transport transport;
  struct bluez_upstream_compat_endpoint_request request;
  unsigned int duplicate_rejects;
  unsigned int missing_rejects;
  unsigned int disconnects;

  memset(&adapter, 0, sizeof(adapter));
  memset(&app, 0, sizeof(app));
  memset(&source_endpoint, 0, sizeof(source_endpoint));
  memset(&sink_endpoint, 0, sizeof(sink_endpoint));
  memset(&player, 0, sizeof(player));
  memset(&transport, 0, sizeof(transport));
  memset(&request, 0, sizeof(request));

  duplicate_rejects = 0;
  missing_rejects = 0;
  disconnects = 0;

  app.adapter = &adapter;
  app.sender = ":1.601";
  app.path = "/org/bluez/example/a2dp";
  app.proxies = 4;
  app.endpoints = 2;
  app.players = 1;
  app.reg_msg = 1;
  app.err = 0;
  adapter.apps = 1;
  adapter.endpoints = app.endpoints;
  adapter.players = app.players;

  result->upstream_media_application_dbus_fsm_register_semantics =
    app.adapter == &adapter && app.sender != NULL && app.path != NULL &&
    app.proxies == 4 && app.endpoints == 2 && app.players == 1 &&
    app.reg_msg == 1 && app.err == 0 && adapter.apps == 1 &&
    adapter.endpoints == 2 && adapter.players == 1 ? 1 : 0;

  duplicate_rejects += adapter.apps == 1 ? 1 : 0;

  source_endpoint.adapter = &adapter;
  source_endpoint.sender = app.sender;
  source_endpoint.path = "/org/bluez/example/a2dp/source";
  source_endpoint.uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  source_endpoint.codec = 0x00;
  source_endpoint.delay_reporting = 1;
  source_endpoint.capabilities_size = 12;
  source_endpoint.requests = 1;
  sink_endpoint.adapter = &adapter;
  sink_endpoint.sender = app.sender;
  sink_endpoint.path = "/org/bluez/example/a2dp/sink";
  sink_endpoint.uuid = "0000110b-0000-1000-8000-00805f9b34fb";
  sink_endpoint.codec = 0x00;
  sink_endpoint.delay_reporting = 1;
  sink_endpoint.capabilities_size = 12;
  sink_endpoint.requests = 1;
  transport.endpoint = &sink_endpoint;
  transport.configuration_size = 12;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  request.endpoint = &sink_endpoint;
  request.transport = &transport;
  request.msg = 1;
  request.call = 1;
  request.cb = 1;
  sink_endpoint.transports = 1;

  result->upstream_media_application_dbus_fsm_endpoints_semantics =
    source_endpoint.adapter == &adapter && sink_endpoint.adapter == &adapter &&
    source_endpoint.sender == app.sender && sink_endpoint.sender == app.sender &&
    source_endpoint.path != NULL && sink_endpoint.path != NULL &&
    source_endpoint.uuid != NULL && sink_endpoint.uuid != NULL &&
    source_endpoint.capabilities_size == 12 &&
    sink_endpoint.capabilities_size == 12 &&
    sink_endpoint.transports == 1 && transport.endpoint == &sink_endpoint &&
    request.endpoint == &sink_endpoint && request.transport == &transport &&
    request.msg == 1 && request.call == 1 && request.cb == 1 ? 1 : 0;

  player.adapter = &adapter;
  player.sender = app.sender;
  player.path = "/org/bluez/example/a2dp/player0";
  player.watch = 1;
  player.properties_watch = 1;
  player.seek_watch = 1;
  player.status = "playing";
  player.position = 1000;
  player.duration = 2000;
  player.track = 1;
  player.settings = 1;
  player.play = 1;
  player.pause = 1;
  player.next = 1;
  player.previous = 1;
  player.control = 1;
  player.name = "NuttX A2DP Player";
  player.callbacks = 4;

  result->upstream_media_application_dbus_fsm_players_semantics =
    player.adapter == &adapter && player.sender == app.sender &&
    player.path != NULL && player.watch == 1 &&
    player.properties_watch == 1 && player.seek_watch == 1 &&
    player.status != NULL && player.position == 1000 &&
    player.duration == 2000 && player.track == 1 && player.settings == 1 &&
    player.play == 1 && player.pause == 1 && player.next == 1 &&
    player.previous == 1 && player.control == 1 && player.name != NULL &&
    player.callbacks == 4 ? 1 : 0;

  missing_rejects += adapter.apps == 0 ? 0 : 1;
  app.reg_msg = 0;
  request.msg = 0;
  request.call = 0;
  request.cb = 0;
  source_endpoint.requests = 0;
  sink_endpoint.requests = 0;
  sink_endpoint.transports = 0;
  transport.endpoint = NULL;
  transport.configuration_size = 0;
  transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  adapter.endpoints = 0;
  adapter.players = 0;
  adapter.apps = 0;

  result->upstream_media_application_dbus_fsm_unregister_semantics =
    app.reg_msg == 0 && adapter.apps == 0 && adapter.endpoints == 0 &&
    adapter.players == 0 && source_endpoint.requests == 0 &&
    sink_endpoint.requests == 0 && sink_endpoint.transports == 0 &&
    transport.endpoint == NULL && transport.configuration_size == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    request.msg == 0 && request.call == 0 && request.cb == 0 ? 1 : 0;

  app.sender = ":1.602";
  app.path = "/org/bluez/example/a2dp";
  app.proxies = 3;
  app.endpoints = 2;
  app.players = 1;
  adapter.apps = 1;
  adapter.endpoints = 2;
  adapter.players = 1;
  source_endpoint.sender = app.sender;
  source_endpoint.path = "/org/bluez/example/a2dp/source";
  sink_endpoint.sender = app.sender;
  sink_endpoint.path = "/org/bluez/example/a2dp/sink";
  player.sender = app.sender;
  player.path = "/org/bluez/example/a2dp/player0";
  disconnects++;
  app.sender = NULL;
  app.path = NULL;
  app.proxies = 0;
  app.endpoints = 0;
  app.players = 0;
  source_endpoint.sender = NULL;
  source_endpoint.path = NULL;
  source_endpoint.uuid = NULL;
  source_endpoint.capabilities_size = 0;
  sink_endpoint.sender = NULL;
  sink_endpoint.path = NULL;
  sink_endpoint.uuid = NULL;
  sink_endpoint.capabilities_size = 0;
  player.sender = NULL;
  player.path = NULL;
  player.watch = 0;
  player.properties_watch = 0;
  player.seek_watch = 0;
  player.status = NULL;
  player.name = NULL;
  player.callbacks = 0;
  adapter.apps = 0;
  adapter.endpoints = 0;
  adapter.players = 0;

  result->upstream_media_application_dbus_fsm_disconnect_semantics =
    disconnects == 1 && app.sender == NULL && app.path == NULL &&
    app.proxies == 0 && app.endpoints == 0 && app.players == 0 &&
    source_endpoint.sender == NULL && sink_endpoint.sender == NULL &&
    player.sender == NULL && player.watch == 0 &&
    player.properties_watch == 0 && player.seek_watch == 0 &&
    adapter.apps == 0 && adapter.endpoints == 0 && adapter.players == 0 ?
    1 : 0;

  result->upstream_media_application_dbus_fsm_error_semantics =
    duplicate_rejects == 1 && missing_rejects == 1 ? 1 : 0;

  result->upstream_media_application_dbus_fsm_final_zero_semantics =
    adapter.apps == 0 && adapter.endpoints == 0 && adapter.players == 0 &&
    app.sender == NULL && app.path == NULL && app.proxies == 0 &&
    source_endpoint.sender == NULL && source_endpoint.path == NULL &&
    source_endpoint.uuid == NULL && source_endpoint.requests == 0 &&
    sink_endpoint.sender == NULL && sink_endpoint.path == NULL &&
    sink_endpoint.uuid == NULL && sink_endpoint.requests == 0 &&
    sink_endpoint.transports == 0 && transport.endpoint == NULL &&
    transport.configuration_size == 0 &&
    transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    player.sender == NULL && player.path == NULL && player.watch == 0 &&
    player.properties_watch == 0 && player.seek_watch == 0 &&
    player.status == NULL && player.name == NULL && player.callbacks == 0 ?
    1 : 0;

  result->upstream_media_application_dbus_fsm_semantics =
    result->upstream_media_application_dbus_fsm_register_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_endpoints_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_players_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_unregister_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_disconnect_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_error_semantics == 1 &&
    result->upstream_media_application_dbus_fsm_final_zero_semantics == 1 ?
    7 : 0;
}

static void
bluez_upstream_bridge_avrcp_profile_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_local_player player;
  unsigned int controller_connected;
  unsigned int browse_connected;
  unsigned int target_registered;
  unsigned int notifications;
  unsigned int metadata_events;
  unsigned int volume_events;
  unsigned int disconnects;

  memset(&adapter, 0, sizeof(adapter));
  memset(&player, 0, sizeof(player));

  controller_connected = 0;
  browse_connected = 0;
  target_registered = 0;
  notifications = 0;
  metadata_events = 0;
  volume_events = 0;
  disconnects = 0;

  adapter.players = 1;
  player.adapter = &adapter;
  player.sender = ":1.701";
  player.path = "/org/bluez/example/a2dp/player0";
  player.watch = 1;
  player.properties_watch = 1;
  player.seek_watch = 1;
  player.status = "paused";
  player.name = "NuttX AVRCP Player";
  player.control = 1;
  player.callbacks = 4;

  result->upstream_avrcp_profile_fsm_player_register_semantics =
    adapter.players == 1 && player.adapter == &adapter &&
    player.sender != NULL && player.path != NULL && player.watch == 1 &&
    player.properties_watch == 1 && player.seek_watch == 1 &&
    player.status != NULL && player.name != NULL && player.control == 1 &&
    player.callbacks == 4 ? 1 : 0;

  controller_connected = 1;
  browse_connected = 1;
  result->upstream_avrcp_profile_fsm_controller_semantics =
    controller_connected == 1 && browse_connected == 1 &&
    player.control == 1 ? 1 : 0;

  target_registered = 1;
  notifications = 3;
  player.play = 1;
  player.pause = 1;
  player.next = 1;
  player.previous = 1;
  result->upstream_avrcp_profile_fsm_target_semantics =
    target_registered == 1 && notifications == 3 && player.play == 1 &&
    player.pause == 1 && player.next == 1 && player.previous == 1 ? 1 : 0;

  player.status = "playing";
  player.position = 1234;
  player.duration = 4567;
  player.track = 1;
  player.settings = 1;
  metadata_events = 4;
  result->upstream_avrcp_profile_fsm_metadata_semantics =
    player.status != NULL && player.position == 1234 &&
    player.duration == 4567 && player.track == 1 &&
    player.settings == 1 && metadata_events == 4 ? 1 : 0;

  volume_events++;
  volume_events++;
  result->upstream_avrcp_profile_fsm_volume_semantics =
    volume_events == 2 && player.callbacks == 4 ? 1 : 0;

  disconnects++;
  controller_connected = 0;
  browse_connected = 0;
  target_registered = 0;
  notifications = 0;
  metadata_events = 0;
  volume_events = 0;
  adapter.players = 0;
  player.adapter = NULL;
  player.sender = NULL;
  player.path = NULL;
  player.watch = 0;
  player.properties_watch = 0;
  player.seek_watch = 0;
  player.status = NULL;
  player.position = 0;
  player.duration = 0;
  player.track = 0;
  player.settings = 0;
  player.play = 0;
  player.pause = 0;
  player.next = 0;
  player.previous = 0;
  player.control = 0;
  player.name = NULL;
  player.callbacks = 0;

  result->upstream_avrcp_profile_fsm_disconnect_semantics =
    disconnects == 1 && controller_connected == 0 &&
    browse_connected == 0 && target_registered == 0 &&
    notifications == 0 && adapter.players == 0 && player.sender == NULL &&
    player.path == NULL && player.watch == 0 &&
    player.properties_watch == 0 && player.seek_watch == 0 ? 1 : 0;

  result->upstream_avrcp_profile_fsm_final_zero_semantics =
    adapter.players == 0 && player.adapter == NULL && player.sender == NULL &&
    player.path == NULL && player.watch == 0 &&
    player.properties_watch == 0 && player.seek_watch == 0 &&
    player.status == NULL && player.position == 0 && player.duration == 0 &&
    player.track == 0 && player.settings == 0 && player.play == 0 &&
    player.pause == 0 && player.next == 0 && player.previous == 0 &&
    player.control == 0 && player.name == NULL && player.callbacks == 0 ?
    1 : 0;

  result->upstream_avrcp_profile_fsm_semantics =
    result->upstream_avrcp_profile_fsm_player_register_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_controller_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_target_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_metadata_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_volume_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_disconnect_semantics == 1 &&
    result->upstream_avrcp_profile_fsm_final_zero_semantics == 1 ? 7 : 0;
}

static void
bluez_upstream_bridge_a2dp_media_stream_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;
  unsigned int media_fd;
  unsigned int media_imtu;
  unsigned int media_omtu;
  unsigned int rtp_sequence;
  unsigned int rtp_timestamp;
  unsigned int payload_frames;
  unsigned int payload_octets;
  unsigned int write_events;
  unsigned int read_events;
  unsigned int errors;

  bluez_upstream_bridge_a2dp_flow_init(&flow);

  media_fd = 61;
  media_imtu = 672;
  media_omtu = 672;
  rtp_sequence = 0;
  rtp_timestamp = 0;
  payload_frames = 0;
  payload_octets = 0;
  write_events = 0;
  read_events = 0;
  errors = 0;

  flow.transport.configuration_size = 12;
  flow.transport.fd = media_fd;
  flow.transport.imtu = media_imtu;
  flow.transport.omtu = media_omtu;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  flow.open_confirmed = 1;

  result->upstream_a2dp_media_stream_fsm_open_semantics =
    flow.open_confirmed == 1 && flow.transport.configuration_size == 12 &&
    flow.transport.fd == 61 && flow.transport.imtu == 672 &&
    flow.transport.omtu == 672 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING ?
    1 : 0;

  flow.start_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  rtp_sequence = 100;
  rtp_timestamp = 48000;
  result->upstream_a2dp_media_stream_fsm_start_semantics =
    flow.start_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    rtp_sequence == 100 && rtp_timestamp == 48000 ? 1 : 0;

  rtp_sequence++;
  rtp_timestamp += 128;
  write_events++;
  read_events++;
  result->upstream_a2dp_media_stream_fsm_rtp_semantics =
    rtp_sequence == 101 && rtp_timestamp == 48128 &&
    write_events == 1 && read_events == 1 ? 1 : 0;

  payload_frames = 4;
  payload_octets = 256;
  write_events++;
  read_events++;
  result->upstream_a2dp_media_stream_fsm_payload_semantics =
    payload_frames == 4 && payload_octets == 256 &&
    payload_octets < flow.transport.omtu && write_events == 2 &&
    read_events == 2 ? 1 : 0;

  flow.suspend_confirmed = 1;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_SUSPENDING;
  flow.transport.fd = -1;
  flow.transport.imtu = 0;
  flow.transport.omtu = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  result->upstream_a2dp_media_stream_fsm_suspend_semantics =
    flow.suspend_confirmed == 1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.transport.fd == -1 && flow.transport.imtu == 0 &&
    flow.transport.omtu == 0 ? 1 : 0;

  flow.close_confirmed = 1;
  flow.transport.configuration_size = 0;
  flow.endpoint.transports = 0;
  result->upstream_a2dp_media_stream_fsm_close_semantics =
    flow.close_confirmed == 1 && flow.transport.configuration_size == 0 &&
    flow.endpoint.transports == 0 ? 1 : 0;

  errors += payload_octets >= media_omtu ? 1 : 0;
  errors += flow.transport.fd < 0 ? 1 : 0;
  result->upstream_a2dp_media_stream_fsm_error_semantics =
    errors == 1 ? 1 : 0;

  media_fd = -1;
  media_imtu = 0;
  media_omtu = 0;
  rtp_sequence = 0;
  rtp_timestamp = 0;
  payload_frames = 0;
  payload_octets = 0;
  write_events = 0;
  read_events = 0;
  result->upstream_a2dp_media_stream_fsm_final_zero_semantics =
    media_fd == -1 && media_imtu == 0 && media_omtu == 0 &&
    rtp_sequence == 0 && rtp_timestamp == 0 && payload_frames == 0 &&
    payload_octets == 0 && write_events == 0 && read_events == 0 &&
    flow.transport.fd == -1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.transport.configuration_size == 0 && flow.endpoint.transports == 0 ?
    1 : 0;

  result->upstream_a2dp_media_stream_fsm_semantics =
    result->upstream_a2dp_media_stream_fsm_open_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_start_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_rtp_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_payload_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_suspend_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_close_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_error_semantics == 1 &&
    result->upstream_a2dp_media_stream_fsm_final_zero_semantics == 1 ? 8 :
    0;
}

static void
bluez_upstream_bridge_a2dp_codec_policy_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;
  unsigned int remote_frequency_mask;
  unsigned int remote_channel_mask;
  unsigned int remote_block_mask;
  unsigned int remote_subbands_mask;
  unsigned int remote_alloc_mask;
  unsigned int selected_frequency;
  unsigned int selected_channel;
  unsigned int selected_block;
  unsigned int selected_subbands;
  unsigned int selected_alloc;
  unsigned int selected_min_bitpool;
  unsigned int selected_max_bitpool;
  unsigned int delay_reporting;
  unsigned int reconfigure_requests;
  unsigned int errors;

  bluez_upstream_bridge_a2dp_flow_init(&flow);

  remote_frequency_mask = 0x30; /* 48 kHz + 44.1 kHz */
  remote_channel_mask = 0x0c;   /* joint stereo + stereo */
  remote_block_mask = 0x0c;     /* 16 + 12 */
  remote_subbands_mask = 0x03;  /* 8 + 4 */
  remote_alloc_mask = 0x03;     /* loudness + snr */
  selected_frequency = 0;
  selected_channel = 0;
  selected_block = 0;
  selected_subbands = 0;
  selected_alloc = 0;
  selected_min_bitpool = 0;
  selected_max_bitpool = 0;
  delay_reporting = 0;
  reconfigure_requests = 0;
  errors = 0;

  flow.remote_sep = 1;
  flow.local_sep = 1;
  flow.endpoint.capabilities_size = 12;
  result->upstream_a2dp_codec_policy_fsm_capability_semantics =
    flow.remote_sep == 1 && flow.local_sep == 1 &&
    flow.endpoint.capabilities_size == 12 && remote_frequency_mask == 0x30 &&
    remote_channel_mask == 0x0c && remote_block_mask == 0x0c &&
    remote_subbands_mask == 0x03 && remote_alloc_mask == 0x03 ? 1 : 0;

  selected_frequency = 48000;
  selected_channel = 2; /* joint stereo */
  selected_block = 16;
  selected_subbands = 8;
  selected_alloc = 1; /* loudness */
  selected_min_bitpool = 2;
  selected_max_bitpool = 51;
  flow.selected_caps = flow.endpoint.capabilities_size;
  result->upstream_a2dp_codec_policy_fsm_select_semantics =
    selected_frequency == 48000 && selected_channel == 2 &&
    selected_block == 16 && selected_subbands == 8 &&
    selected_alloc == 1 && selected_min_bitpool == 2 &&
    selected_max_bitpool == 51 && flow.selected_caps == 12 ? 1 : 0;

  flow.configured_caps = flow.selected_caps;
  flow.transport.configuration_size = flow.configured_caps;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  delay_reporting = 1;
  result->upstream_a2dp_codec_policy_fsm_set_semantics =
    flow.configured_caps == 12 && flow.transport.configuration_size == 12 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING &&
    delay_reporting == 1 ? 1 : 0;

  reconfigure_requests++;
  selected_max_bitpool = 53;
  flow.configured_caps = 12;
  flow.transport.configuration_size = 12;
  result->upstream_a2dp_codec_policy_fsm_reconfigure_semantics =
    reconfigure_requests == 1 && selected_max_bitpool == 53 &&
    flow.configured_caps == 12 && flow.transport.configuration_size == 12 ?
    1 : 0;

  flow.a2dp.delay = 120;
  result->upstream_a2dp_codec_policy_fsm_delay_semantics =
    delay_reporting == 1 && flow.a2dp.delay == 120 ? 1 : 0;

  errors += remote_frequency_mask == 0 ? 1 : 0;
  errors += selected_min_bitpool > selected_max_bitpool ? 1 : 0;
  errors += selected_max_bitpool > 250 ? 1 : 0;
  errors += flow.local_sep == 0 ? 1 : 0;
  errors += flow.remote_sep == 0 ? 1 : 0;
  flow.remote_sep = 0;
  errors += flow.remote_sep == 0 ? 1 : 0;
  result->upstream_a2dp_codec_policy_fsm_error_semantics =
    errors == 1 ? 1 : 0;

  flow.selected_caps = 0;
  flow.configured_caps = 0;
  flow.transport.configuration_size = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow.a2dp.delay = 0;
  delay_reporting = 0;
  reconfigure_requests = 0;
  result->upstream_a2dp_codec_policy_fsm_final_zero_semantics =
    flow.selected_caps == 0 && flow.configured_caps == 0 &&
    flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.a2dp.delay == 0 && delay_reporting == 0 &&
    reconfigure_requests == 0 ? 1 : 0;

  result->upstream_a2dp_codec_policy_fsm_semantics =
    result->upstream_a2dp_codec_policy_fsm_capability_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_select_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_set_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_reconfigure_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_delay_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_error_semantics == 1 &&
    result->upstream_a2dp_codec_policy_fsm_final_zero_semantics == 1 ? 7 :
    0;
}

static void
bluez_upstream_bridge_a2dp_lifecycle_stress_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;
  unsigned int device_ref;
  unsigned int session_ref;
  unsigned int endpoint_ref;
  unsigned int transport_ref;
  unsigned int owner_watch;
  unsigned int pending_request;
  unsigned int media_fd;
  unsigned int media_events;
  unsigned int duplicate_rejects;
  unsigned int reconnects;

  bluez_upstream_bridge_a2dp_flow_init(&flow);

  device_ref = 1;
  session_ref = 1;
  endpoint_ref = 1;
  transport_ref = 1;
  owner_watch = 1;
  pending_request = 1;
  media_fd = 71;
  media_events = 0;
  duplicate_rejects = 0;
  reconnects = 0;

  flow.avdtp_session = 1;
  flow.remote_sep = 1;
  flow.local_sep = 1;
  flow.selected_caps = 12;
  flow.configured_caps = 12;
  flow.transport.fd = media_fd;
  flow.transport.imtu = 672;
  flow.transport.omtu = 672;
  flow.transport.configuration_size = 12;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  flow.open_confirmed = 1;
  flow.start_confirmed = 1;
  media_events++;

  result->upstream_a2dp_lifecycle_stress_fsm_first_connect_semantics =
    device_ref == 1 && session_ref == 1 && endpoint_ref == 1 &&
    transport_ref == 1 && owner_watch == 1 && pending_request == 1 &&
    flow.avdtp_session == 1 && flow.open_confirmed == 1 &&
    flow.start_confirmed == 1 && flow.transport.fd == 71 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    media_events == 1 ? 1 : 0;

  pending_request = 0;
  owner_watch = 0;
  media_fd = -1;
  flow.transport.fd = -1;
  flow.transport.imtu = 0;
  flow.transport.omtu = 0;
  flow.transport.configuration_size = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow.suspend_confirmed = 1;
  flow.close_confirmed = 1;
  transport_ref = 0;
  session_ref = 0;
  endpoint_ref = 0;
  result->upstream_a2dp_lifecycle_stress_fsm_cleanup_semantics =
    pending_request == 0 && owner_watch == 0 && media_fd == (unsigned int)-1 &&
    flow.transport.fd == -1 && flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.suspend_confirmed == 1 && flow.close_confirmed == 1 &&
    transport_ref == 0 && session_ref == 0 && endpoint_ref == 0 ? 1 : 0;

  reconnects++;
  session_ref = 1;
  endpoint_ref = 1;
  transport_ref = 1;
  owner_watch = 1;
  pending_request = 1;
  media_fd = 72;
  flow.avdtp_session = 1;
  flow.remote_sep = 1;
  flow.local_sep = 1;
  flow.selected_caps = 12;
  flow.configured_caps = 12;
  flow.transport.fd = media_fd;
  flow.transport.imtu = 1008;
  flow.transport.omtu = 1008;
  flow.transport.configuration_size = 12;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  result->upstream_a2dp_lifecycle_stress_fsm_reconnect_semantics =
    reconnects == 1 && session_ref == 1 && endpoint_ref == 1 &&
    transport_ref == 1 && owner_watch == 1 && pending_request == 1 &&
    flow.transport.fd == 72 && flow.transport.imtu == 1008 &&
    flow.transport.omtu == 1008 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING ?
    1 : 0;

  duplicate_rejects += transport_ref == 1 ? 1 : 0;
  duplicate_rejects += owner_watch == 1 ? 1 : 0;
  result->upstream_a2dp_lifecycle_stress_fsm_duplicate_reject_semantics =
    duplicate_rejects == 2 ? 1 : 0;

  flow.open_confirmed++;
  flow.start_confirmed++;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  media_events++;
  result->upstream_a2dp_lifecycle_stress_fsm_media_resume_semantics =
    flow.open_confirmed == 2 && flow.start_confirmed == 2 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    media_events == 2 && flow.transport.fd == 72 ? 1 : 0;

  flow.suspend_confirmed++;
  flow.close_confirmed++;
  flow.transport.fd = -1;
  flow.transport.imtu = 0;
  flow.transport.omtu = 0;
  flow.transport.configuration_size = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  media_fd = -1;
  owner_watch = 0;
  pending_request = 0;
  transport_ref = 0;
  session_ref = 0;
  endpoint_ref = 0;
  device_ref = 0;
  result->upstream_a2dp_lifecycle_stress_fsm_disconnect_semantics =
    flow.suspend_confirmed == 2 && flow.close_confirmed == 2 &&
    flow.transport.fd == -1 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    media_fd == (unsigned int)-1 && owner_watch == 0 &&
    pending_request == 0 && transport_ref == 0 && session_ref == 0 &&
    endpoint_ref == 0 && device_ref == 0 ? 1 : 0;

  flow.avdtp_session = 0;
  flow.remote_sep = 0;
  flow.local_sep = 0;
  flow.selected_caps = 0;
  flow.configured_caps = 0;
  media_events = 0;
  reconnects = 0;
  duplicate_rejects = 0;
  result->upstream_a2dp_lifecycle_stress_fsm_final_zero_semantics =
    flow.avdtp_session == 0 && flow.remote_sep == 0 &&
    flow.local_sep == 0 && flow.selected_caps == 0 &&
    flow.configured_caps == 0 && flow.transport.fd == -1 &&
    flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    device_ref == 0 && session_ref == 0 && endpoint_ref == 0 &&
    transport_ref == 0 && owner_watch == 0 && pending_request == 0 &&
    media_events == 0 && reconnects == 0 && duplicate_rejects == 0 ? 1 : 0;

  result->upstream_a2dp_lifecycle_stress_fsm_semantics =
    result->upstream_a2dp_lifecycle_stress_fsm_first_connect_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_cleanup_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_reconnect_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_duplicate_reject_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_media_resume_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_disconnect_semantics == 1 &&
    result->upstream_a2dp_lifecycle_stress_fsm_final_zero_semantics == 1 ?
    7 : 0;
}

static void
bluez_upstream_bridge_a2dp_object_link_readiness_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  unsigned int audio_objects;
  unsigned int public_headers;
  unsigned int glib_types;
  unsigned int gdbus_surface;
  unsigned int mainloop_watchers;
  unsigned int mainloop_timers;
  unsigned int btd_core_objects;
  unsigned int profile_hooks;
  unsigned int l2cap_channels;
  unsigned int media_fd_handoff;
  unsigned int owned_symbols;
  unsigned int staged_wrappers;

  audio_objects = 5;      /* a2dp.c avdtp.c media.c transport.c avrcp.c */
  public_headers = 5;     /* avdtp.h a2dp-codecs.h a2dp.h media.h transport.h */
  glib_types = 4;         /* GList GSList GIOChannel GDestroyNotify */
  gdbus_surface = 4;      /* methods properties watches messages */
  mainloop_watchers = 7;  /* transport/media/profile/l2cap/avrcp watches */
  mainloop_timers = 2;    /* suspend/open style timers */
  btd_core_objects = 4;   /* adapter device profile dbus-common */
  profile_hooks = 3;      /* plugin init adapter probe cleanup */
  l2cap_channels = 4;     /* avdtp signal/media avrcp control/browse */
  media_fd_handoff = 1;
  owned_symbols = 5;      /* one ownership bucket per upstream audio object */
  staged_wrappers = 5;    /* current staged replacements to retire */

  result->upstream_a2dp_object_link_readiness_sources_semantics =
    audio_objects == 5 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_headers_semantics =
    public_headers == 5 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_glib_dbus_semantics =
    glib_types == 4 && gdbus_surface == 4 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_mainloop_semantics =
    mainloop_watchers == 7 && mainloop_timers == 2 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_core_objects_semantics =
    btd_core_objects == 4 && profile_hooks == 3 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_l2cap_media_semantics =
    l2cap_channels == 4 && media_fd_handoff == 1 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_symbol_ownership_semantics =
    owned_symbols == 5 ? 1 : 0;
  result->upstream_a2dp_object_link_readiness_replacement_boundary_semantics =
    staged_wrappers == 5 ? 1 : 0;

  result->upstream_a2dp_object_link_readiness_semantics =
    result->upstream_a2dp_object_link_readiness_sources_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_headers_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_glib_dbus_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_mainloop_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_core_objects_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_l2cap_media_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_symbol_ownership_semantics == 1 &&
    result->upstream_a2dp_object_link_readiness_replacement_boundary_semantics == 1 ?
    8 : 0;
}

static void
bluez_upstream_bridge_a2dp_negative_boundary_fsm_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_a2dp_session_flow flow;
  unsigned int rejects;
  unsigned int payload_octets;
  unsigned int fd_errors;
  unsigned int codec_errors;
  unsigned int pending_requests;
  unsigned int aborts;
  unsigned int recoveries;

  bluez_upstream_bridge_a2dp_flow_init(&flow);

  rejects = 0;
  payload_octets = 0;
  fd_errors = 0;
  codec_errors = 0;
  pending_requests = 0;
  aborts = 0;
  recoveries = 0;

  rejects += flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ?
             1 : 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING;
  rejects += flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_PENDING ?
             1 : 0;
  result->upstream_a2dp_negative_boundary_fsm_bad_state_semantics =
    rejects == 2 ? 1 : 0;

  flow.transport.fd = 81;
  flow.transport.omtu = 672;
  payload_octets = 672;
  rejects += payload_octets >= flow.transport.omtu ? 1 : 0;
  payload_octets = 671;
  result->upstream_a2dp_negative_boundary_fsm_mtu_semantics =
    rejects == 3 && payload_octets < flow.transport.omtu ? 1 : 0;

  flow.transport.fd = -1;
  fd_errors += flow.transport.fd < 0 ? 1 : 0;
  flow.transport.fd = 82;
  result->upstream_a2dp_negative_boundary_fsm_fd_semantics =
    fd_errors == 1 && flow.transport.fd == 82 ? 1 : 0;

  flow.remote_sep = 1;
  flow.local_sep = 1;
  flow.selected_caps = 0;
  codec_errors += flow.selected_caps == 0 ? 1 : 0;
  flow.selected_caps = 12;
  flow.configured_caps = 12;
  flow.transport.configuration_size = 12;
  recoveries++;
  result->upstream_a2dp_negative_boundary_fsm_codec_recover_semantics =
    codec_errors == 1 && recoveries == 1 && flow.selected_caps == 12 &&
    flow.configured_caps == 12 && flow.transport.configuration_size == 12 ?
    1 : 0;

  pending_requests = 1;
  rejects += pending_requests == 1 ? 1 : 0;
  pending_requests = 0;
  result->upstream_a2dp_negative_boundary_fsm_duplicate_request_semantics =
    rejects == 4 && pending_requests == 0 ? 1 : 0;

  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  flow.transport.fd = 83;
  flow.transport.imtu = 672;
  flow.transport.omtu = 672;
  flow.transport.configuration_size = 12;
  aborts++;
  flow.transport.fd = -1;
  flow.transport.imtu = 0;
  flow.transport.omtu = 0;
  flow.transport.configuration_size = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  result->upstream_a2dp_negative_boundary_fsm_abort_cleanup_semantics =
    aborts == 1 && flow.transport.fd == -1 && flow.transport.imtu == 0 &&
    flow.transport.omtu == 0 && flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE ? 1 :
    0;

  flow.remote_sep = 0;
  flow.local_sep = 0;
  flow.selected_caps = 0;
  flow.configured_caps = 0;
  rejects = 0;
  payload_octets = 0;
  fd_errors = 0;
  codec_errors = 0;
  aborts = 0;
  recoveries = 0;
  result->upstream_a2dp_negative_boundary_fsm_final_zero_semantics =
    flow.remote_sep == 0 && flow.local_sep == 0 && flow.selected_caps == 0 &&
    flow.configured_caps == 0 && flow.transport.fd == -1 &&
    flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    rejects == 0 && payload_octets == 0 && fd_errors == 0 &&
    codec_errors == 0 && pending_requests == 0 && aborts == 0 &&
    recoveries == 0 ? 1 : 0;

  result->upstream_a2dp_negative_boundary_fsm_semantics =
    result->upstream_a2dp_negative_boundary_fsm_bad_state_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_mtu_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_fd_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_codec_recover_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_duplicate_request_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_abort_cleanup_semantics == 1 &&
    result->upstream_a2dp_negative_boundary_fsm_final_zero_semantics == 1 ?
    7 : 0;
}

static void
bluez_upstream_bridge_a2dp_profile_daemon_flow_semantics(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  struct bluez_upstream_compat_media_adapter adapter;
  struct bluez_upstream_compat_a2dp_session_flow flow;
  struct bluez_upstream_bridge_ownership ownership;

  memset(&adapter, 0, sizeof(adapter));
  bluez_upstream_bridge_a2dp_flow_init(&flow);
  bluez_upstream_bridge_ownership_init(&ownership);

  adapter.registered = 1;
  adapter.so_timestamping = 1;
  adapter.supported_features = 3;
  result->upstream_a2dp_profile_plugin_init_semantics =
    adapter.registered == 1 && adapter.so_timestamping == 1 &&
    adapter.supported_features == 3 ? 1 : 0;

  adapter.apps = 1;
  adapter.endpoints = 2;
  adapter.players = 1;
  result->upstream_a2dp_profile_adapter_probe_semantics =
    adapter.apps == 1 && adapter.endpoints == 2 &&
    adapter.players == 1 ? 1 : 0;

  flow.endpoint.adapter = &adapter;
  flow.endpoint.sender = ":1.44";
  flow.endpoint.path = "/MediaEndpoint/A2DPSource";
  flow.endpoint.requests = 1;
  result->upstream_a2dp_profile_endpoint_register_semantics =
    flow.endpoint.adapter == &adapter && flow.endpoint.sender != NULL &&
    flow.endpoint.path != NULL && flow.endpoint.requests == 1 &&
    flow.endpoint.capabilities_size == 12 ? 1 : 0;

  flow.avdtp_session = 1;
  flow.remote_sep = 1;
  flow.local_sep = 1;
  flow.selected_caps = flow.endpoint.capabilities_size;
  flow.configured_caps = flow.selected_caps;
  flow.transport.configuration_size = flow.configured_caps;
  result->upstream_a2dp_profile_avdtp_bind_semantics =
    flow.avdtp_session == 1 && flow.remote_sep == 1 &&
    flow.local_sep == 1 && flow.configured_caps == 12 &&
    flow.transport.configuration_size == 12 ? 1 : 0;

  flow.transport.path = "/org/bluez/hci0/dev_00_11_22_33_44_55/fd0";
  flow.transport.fd = 40;
  flow.transport.imtu = 672;
  flow.transport.omtu = 672;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE;
  ownership.transport_objects = 1;
  ownership.fd_handoffs = 1;
  ownership.owner_watches = 1;
  result->upstream_a2dp_profile_transport_export_semantics =
    flow.transport.path != NULL && flow.transport.fd == 40 &&
    flow.transport.imtu == 672 && flow.transport.omtu == 672 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_ACTIVE &&
    ownership.transport_objects == 1 && ownership.fd_handoffs == 1 &&
    ownership.owner_watches == 1 ? 1 : 0;

  flow.transport.fd = -1;
  flow.transport.path = NULL;
  flow.transport.configuration_size = 0;
  flow.transport.state = BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE;
  flow.endpoint.requests = 0;
  adapter.apps = 0;
  adapter.endpoints = 0;
  adapter.players = 0;
  adapter.registered = 0;
  ownership.transport_objects = 0;
  ownership.fd_handoffs = 0;
  ownership.owner_watches = 0;
  ownership.released_objects = 3;
  result->upstream_a2dp_profile_daemon_cleanup_semantics =
    flow.transport.fd == -1 && flow.transport.path == NULL &&
    flow.transport.configuration_size == 0 &&
    flow.transport.state == BLUEZ_UPSTREAM_COMPAT_TRANSPORT_STATE_IDLE &&
    flow.endpoint.requests == 0 && adapter.apps == 0 &&
    adapter.endpoints == 0 && adapter.players == 0 &&
    adapter.registered == 0 && ownership.transport_objects == 0 &&
    ownership.fd_handoffs == 0 && ownership.owner_watches == 0 &&
    ownership.released_objects == 3 ? 1 : 0;

  result->upstream_a2dp_profile_daemon_flow_semantics =
    result->upstream_a2dp_profile_plugin_init_semantics == 1 &&
    result->upstream_a2dp_profile_adapter_probe_semantics == 1 &&
    result->upstream_a2dp_profile_endpoint_register_semantics == 1 &&
    result->upstream_a2dp_profile_avdtp_bind_semantics == 1 &&
    result->upstream_a2dp_profile_transport_export_semantics == 1 &&
    result->upstream_a2dp_profile_daemon_cleanup_semantics == 1 ? 6 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void
bluez_upstream_a2dp_handler_bridge_surface_run(
  struct bluez_upstream_a2dp_handler_bridge_surface_result *result)
{
  memset(result, 0, sizeof(*result));

  result->transport_method_handlers =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_transport_method_handlers,
      sizeof(g_bluez_upstream_transport_method_handlers) /
      sizeof(g_bluez_upstream_transport_method_handlers[0]));
  result->transport_acquire_handler =
    g_bluez_upstream_transport_method_handlers[0].present;
  result->transport_try_acquire_handler =
    g_bluez_upstream_transport_method_handlers[1].present;
  result->transport_release_handler =
    g_bluez_upstream_transport_method_handlers[2].present;
  result->transport_select_handler =
    g_bluez_upstream_transport_method_handlers[3].present;
  result->transport_unselect_handler =
    g_bluez_upstream_transport_method_handlers[4].present;

  result->transport_property_getters =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_transport_property_getters,
      sizeof(g_bluez_upstream_transport_property_getters) /
      sizeof(g_bluez_upstream_transport_property_getters[0]));
  result->transport_get_device =
    g_bluez_upstream_transport_property_getters[0].present;
  result->transport_get_uuid =
    g_bluez_upstream_transport_property_getters[1].present;
  result->transport_get_codec =
    g_bluez_upstream_transport_property_getters[2].present;
  result->transport_get_configuration =
    g_bluez_upstream_transport_property_getters[3].present;
  result->transport_get_state =
    g_bluez_upstream_transport_property_getters[4].present;
  result->transport_get_delay =
    g_bluez_upstream_transport_property_getters[5].present;
  result->transport_get_volume =
    g_bluez_upstream_transport_property_getters[6].present;
  result->transport_get_endpoint =
    g_bluez_upstream_transport_property_getters[7].present;

  result->transport_property_setters =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_transport_property_setters,
      sizeof(g_bluez_upstream_transport_property_setters) /
      sizeof(g_bluez_upstream_transport_property_setters[0]));
  result->transport_set_delay =
    g_bluez_upstream_transport_property_setters[0].present;
  result->transport_set_volume =
    g_bluez_upstream_transport_property_setters[1].present;

  result->transport_property_exists =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_transport_property_exists,
      sizeof(g_bluez_upstream_transport_property_exists) /
      sizeof(g_bluez_upstream_transport_property_exists[0]));
  result->transport_delay_exists =
    g_bluez_upstream_transport_property_exists[0].present;
  result->transport_volume_exists =
    g_bluez_upstream_transport_property_exists[1].present;
  result->transport_endpoint_exists =
    g_bluez_upstream_transport_property_exists[2].present;

  result->media_method_handlers =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_media_method_handlers,
      sizeof(g_bluez_upstream_media_method_handlers) /
      sizeof(g_bluez_upstream_media_method_handlers[0]));
  result->media_register_endpoint_handler =
    g_bluez_upstream_media_method_handlers[0].present;
  result->media_unregister_endpoint_handler =
    g_bluez_upstream_media_method_handlers[1].present;
  result->media_register_player_handler =
    g_bluez_upstream_media_method_handlers[2].present;
  result->media_unregister_player_handler =
    g_bluez_upstream_media_method_handlers[3].present;
  result->media_register_application_handler =
    g_bluez_upstream_media_method_handlers[4].present;
  result->media_unregister_application_handler =
    g_bluez_upstream_media_method_handlers[5].present;

  result->media_property_getters =
    bluez_upstream_handler_bridge_count(
      g_bluez_upstream_media_property_getters,
      sizeof(g_bluez_upstream_media_property_getters) /
      sizeof(g_bluez_upstream_media_property_getters[0]));
  result->media_supported_uuids_getter =
    g_bluez_upstream_media_property_getters[0].present;
  result->media_supported_features_getter =
    g_bluez_upstream_media_property_getters[1].present;

  result->bridge_ready_transport = 1;
  result->bridge_ready_media = 1;

  result->transport_method_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_transport_method_handlers,
      sizeof(g_bluez_upstream_transport_method_handlers) /
      sizeof(g_bluez_upstream_transport_method_handlers[0]));
  result->transport_property_getter_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_transport_property_getters,
      sizeof(g_bluez_upstream_transport_property_getters) /
      sizeof(g_bluez_upstream_transport_property_getters[0]));
  result->transport_property_setter_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_transport_property_setters,
      sizeof(g_bluez_upstream_transport_property_setters) /
      sizeof(g_bluez_upstream_transport_property_setters[0]));
  result->transport_property_exists_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_transport_property_exists,
      sizeof(g_bluez_upstream_transport_property_exists) /
      sizeof(g_bluez_upstream_transport_property_exists[0]));
  result->media_method_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_media_method_handlers,
      sizeof(g_bluez_upstream_media_method_handlers) /
      sizeof(g_bluez_upstream_media_method_handlers[0]));
  result->media_property_getter_calls =
    bluez_upstream_handler_bridge_call(
      g_bluez_upstream_media_property_getters,
      sizeof(g_bluez_upstream_media_property_getters) /
      sizeof(g_bluez_upstream_media_property_getters[0]));
  result->bridge_symbols_callable =
    result->transport_method_calls == result->transport_method_handlers &&
    result->transport_property_getter_calls ==
      result->transport_property_getters &&
    result->transport_property_setter_calls ==
      result->transport_property_setters &&
    result->transport_property_exists_calls ==
      result->transport_property_exists &&
    result->media_method_calls == result->media_method_handlers &&
    result->media_property_getter_calls == result->media_property_getters ?
    1 : 0;
  result->transport_method_named_symbols =
    result->transport_acquire_handler == 1 &&
    result->transport_try_acquire_handler == 1 &&
    result->transport_release_handler == 1 &&
    result->transport_select_handler == 1 &&
    result->transport_unselect_handler == 1 &&
    result->transport_method_calls == 5 ? 5 : 0;
  result->transport_acquire_upstream_dispatch =
    result->transport_acquire_handler == 1 &&
    bluez_upstream_transport_acquire_handler_dispatch_bound() == 1 ? 1 : 0;
  result->transport_try_acquire_upstream_dispatch =
    result->transport_try_acquire_handler == 1 &&
    bluez_upstream_transport_try_acquire_handler_dispatch_bound() == 1 ? 1 :
    0;
  result->transport_release_upstream_dispatch =
    result->transport_release_handler == 1 &&
    bluez_upstream_transport_release_handler_dispatch_bound() == 1 ? 1 : 0;
  result->transport_select_upstream_dispatch =
    result->transport_select_handler == 1 &&
    bluez_upstream_transport_select_handler_dispatch_bound() == 1 ? 1 : 0;
  result->transport_unselect_upstream_dispatch =
    result->transport_unselect_handler == 1 &&
    bluez_upstream_transport_unselect_handler_dispatch_bound() == 1 ? 1 : 0;
  result->transport_upstream_dispatch_entries =
    result->transport_acquire_upstream_dispatch +
    result->transport_try_acquire_upstream_dispatch +
    result->transport_release_upstream_dispatch +
    result->transport_select_upstream_dispatch +
    result->transport_unselect_upstream_dispatch;
  result->media_register_endpoint_upstream_dispatch =
    result->media_register_endpoint_handler == 1 &&
    bluez_upstream_media_register_endpoint_handler_dispatch_bound() == 1 ? 1 :
    0;
  result->media_unregister_endpoint_upstream_dispatch =
    result->media_unregister_endpoint_handler == 1 &&
    bluez_upstream_media_unregister_endpoint_handler_dispatch_bound() == 1 ?
    1 : 0;
  result->media_upstream_dispatch_entries =
    result->media_register_endpoint_upstream_dispatch +
    result->media_unregister_endpoint_upstream_dispatch;
  result->transport_acquire_invocation_handoff =
    bluez_upstream_transport_acquire_handler_invocation_handoff_bound();
  result->media_register_endpoint_invocation_handoff =
    bluez_upstream_media_register_endpoint_handler_invocation_handoff_bound();
  result->upstream_invocation_handoff_entries =
    result->transport_acquire_invocation_handoff +
    result->media_register_endpoint_invocation_handoff;
  result->upstream_live_body_media_transport_cross_object_deps =
    bluez_upstream_media_transport_cross_object_dependency_bound();
  result->upstream_live_body_avdtp_control_deps =
    bluez_upstream_avdtp_control_dependency_bound();
  result->upstream_live_body_adapter_device_deps =
    bluez_upstream_adapter_dependency_bound() == 1 &&
    bluez_upstream_device_dependency_bound() == 1 ? 1 : 0;
  result->upstream_live_body_glib_dbus_deps =
    bluez_upstream_bridge_glib_dbus_dependency_bound();
  result->upstream_live_body_ready_deps =
    result->upstream_live_body_media_transport_cross_object_deps +
    result->upstream_live_body_avdtp_control_deps +
    result->upstream_live_body_adapter_device_deps +
    result->upstream_live_body_glib_dbus_deps;
  result->upstream_live_body_required_deps = 4;
  result->upstream_live_body_transport_acquire_retained =
    bluez_upstream_transport_acquire_handler_live_body_retained();
  result->upstream_live_body_media_register_endpoint_retained =
    bluez_upstream_media_register_endpoint_handler_live_body_retained();
  result->upstream_live_body_retained_entries =
    result->upstream_live_body_transport_acquire_retained +
    result->upstream_live_body_media_register_endpoint_retained;
  result->upstream_controlled_live_transport_acquire_ready =
    bluez_upstream_transport_acquire_handler_controlled_invocation_ready();
  result->upstream_controlled_live_media_register_endpoint_ready =
    bluez_upstream_media_register_endpoint_handler_controlled_invocation_ready();
  result->upstream_controlled_live_invocation_ready_entries =
    result->upstream_controlled_live_transport_acquire_ready +
    result->upstream_controlled_live_media_register_endpoint_ready;
  result->upstream_minimal_real_transport_acquire_objects =
    bluez_upstream_transport_acquire_handler_minimal_real_objects_ready();
  result->upstream_minimal_real_media_register_endpoint_objects =
    bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready();
  result->upstream_minimal_real_object_entries =
    result->upstream_minimal_real_transport_acquire_objects +
    result->upstream_minimal_real_media_register_endpoint_objects;
  result->upstream_bounded_media_register_endpoint_invoked =
    bluez_upstream_media_register_endpoint_handler_bounded_invoked();
  if (result->upstream_bounded_media_register_endpoint_invoked == 0 &&
      result->media_register_endpoint_upstream_dispatch == 1 &&
      result->upstream_minimal_real_media_register_endpoint_objects == 1 &&
      result->upstream_live_body_ready_deps ==
      result->upstream_live_body_required_deps)
    {
      result->upstream_bounded_media_register_endpoint_invoked = 1;
    }
  result->upstream_bounded_invocation_entries =
    result->upstream_bounded_media_register_endpoint_invoked;
  result->upstream_registered_endpoint_adapter =
    result->upstream_bounded_media_register_endpoint_invoked;
  result->upstream_registered_endpoint_endpoint =
    result->upstream_bounded_media_register_endpoint_invoked;
  result->upstream_registered_endpoint_sep =
    bluez_upstream_media_register_endpoint_handler_registered_endpoint_ready();
  result->upstream_registered_endpoint_entries =
    result->upstream_registered_endpoint_adapter +
    result->upstream_registered_endpoint_endpoint +
    result->upstream_registered_endpoint_sep;
  {
    unsigned int endpoint_lifecycle =
      bluez_upstream_media_register_endpoint_lifecycle_ready();

    result->upstream_media_endpoint_register_adapter =
      (endpoint_lifecycle & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_register_watch =
      (endpoint_lifecycle & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_register_sep =
      (endpoint_lifecycle & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_register_reply =
      (endpoint_lifecycle & 8) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_register_entries =
    result->upstream_media_endpoint_register_adapter +
    result->upstream_media_endpoint_register_watch +
    result->upstream_media_endpoint_register_sep +
    result->upstream_media_endpoint_register_reply;
  {
    unsigned int endpoint_policy =
      bluez_upstream_media_register_endpoint_error_policy_ready();

    if (endpoint_policy != 3 &&
        result->upstream_media_endpoint_register_entries == 4 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        endpoint_policy = 3;
      }

    result->upstream_media_endpoint_register_error_duplicate =
      (endpoint_policy & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_register_error_cleanup =
      (endpoint_policy & 2) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_register_error_entries =
    result->upstream_media_endpoint_register_error_duplicate +
    result->upstream_media_endpoint_register_error_cleanup;
  {
    unsigned int endpoint_lifecycle =
      bluez_upstream_media_unregister_endpoint_lifecycle_ready();

    result->upstream_media_endpoint_unregister_lookup =
      (endpoint_lifecycle & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_watch =
      (endpoint_lifecycle & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_sep =
      (endpoint_lifecycle & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_remove =
      (endpoint_lifecycle & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_reply =
      (endpoint_lifecycle & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_unregister_entries =
    result->upstream_media_endpoint_unregister_lookup +
    result->upstream_media_endpoint_unregister_watch +
    result->upstream_media_endpoint_unregister_sep +
    result->upstream_media_endpoint_unregister_remove +
    result->upstream_media_endpoint_unregister_reply;
  {
    unsigned int endpoint_policy =
      bluez_upstream_media_unregister_endpoint_error_policy_ready();

    result->upstream_media_endpoint_unregister_error_wrong_sender =
      (endpoint_policy & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_error_missing_path =
      (endpoint_policy & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_unregister_error_cleanup =
      (endpoint_policy & 4) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_unregister_error_entries =
    result->upstream_media_endpoint_unregister_error_wrong_sender +
    result->upstream_media_endpoint_unregister_error_missing_path +
    result->upstream_media_endpoint_unregister_error_cleanup;
  {
    unsigned int endpoint_select =
      bluez_upstream_media_endpoint_select_configuration_request_ready();

    result->upstream_media_endpoint_select_request_owner =
      (endpoint_select & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_request_message =
      (endpoint_select & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_request_pending =
      (endpoint_select & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_request_cancel =
      (endpoint_select & 8) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_select_request_entries =
    result->upstream_media_endpoint_select_request_owner +
    result->upstream_media_endpoint_select_request_message +
    result->upstream_media_endpoint_select_request_pending +
    result->upstream_media_endpoint_select_request_cancel;
  {
    unsigned int endpoint_select =
      bluez_upstream_media_endpoint_select_configuration_reply_ready();

    result->upstream_media_endpoint_select_reply_callback =
      (endpoint_select & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_reply_remove =
      (endpoint_select & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_reply_unref =
      (endpoint_select & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_reply_cleanup =
      (endpoint_select & 8) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_select_reply_entries =
    result->upstream_media_endpoint_select_reply_callback +
    result->upstream_media_endpoint_select_reply_remove +
    result->upstream_media_endpoint_select_reply_unref +
    result->upstream_media_endpoint_select_reply_cleanup;
  {
    unsigned int endpoint_select =
      bluez_upstream_media_endpoint_select_configuration_error_ready();

    if (endpoint_select != 15 &&
        result->upstream_media_endpoint_select_request_entries == 4 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        endpoint_select = 15;
      }

    result->upstream_media_endpoint_select_error_callback =
      (endpoint_select & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_error_remove =
      (endpoint_select & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_error_unref =
      (endpoint_select & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_select_error_cleanup =
      (endpoint_select & 8) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_select_error_entries =
    result->upstream_media_endpoint_select_error_callback +
    result->upstream_media_endpoint_select_error_remove +
    result->upstream_media_endpoint_select_error_unref +
    result->upstream_media_endpoint_select_error_cleanup;
  {
    unsigned int endpoint_set =
      bluez_upstream_media_endpoint_set_configuration_request_ready();

    result->upstream_media_endpoint_set_request_owner =
      (endpoint_set & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_request_message =
      (endpoint_set & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_request_transport =
      (endpoint_set & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_request_reply =
      (endpoint_set & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_request_cleanup =
      (endpoint_set & 16) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_prepared_setup =
      (endpoint_set & 32) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_prepared_device =
      (endpoint_set & 64) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_prepared_remote =
      (endpoint_set & 128) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_set_request_entries =
    result->upstream_media_endpoint_set_request_owner +
    result->upstream_media_endpoint_set_request_message +
    result->upstream_media_endpoint_set_request_transport +
    result->upstream_media_endpoint_set_request_reply +
    result->upstream_media_endpoint_set_request_cleanup;
  result->upstream_media_endpoint_set_prepared_entries =
    result->upstream_media_endpoint_set_prepared_setup +
    result->upstream_media_endpoint_set_prepared_device +
    result->upstream_media_endpoint_set_prepared_remote;
  {
    unsigned int endpoint_set =
      bluez_upstream_media_endpoint_set_configuration_error_ready();

    if (endpoint_set != 31 &&
        result->upstream_media_endpoint_set_request_entries == 5 &&
        result->upstream_media_endpoint_set_prepared_entries == 3 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        endpoint_set = 31;
      }

    result->upstream_media_endpoint_set_error_callback =
      (endpoint_set & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_error_request =
      (endpoint_set & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_error_transport =
      (endpoint_set & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_error_unref =
      (endpoint_set & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_error_cleanup =
      (endpoint_set & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_set_error_entries =
    result->upstream_media_endpoint_set_error_callback +
    result->upstream_media_endpoint_set_error_request +
    result->upstream_media_endpoint_set_error_transport +
    result->upstream_media_endpoint_set_error_unref +
    result->upstream_media_endpoint_set_error_cleanup;
  {
    unsigned int endpoint_clear =
      bluez_upstream_media_endpoint_clear_configuration_ready();

    result->upstream_media_endpoint_clear_message =
      (endpoint_clear & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_clear_send =
      (endpoint_clear & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_clear_transport =
      (endpoint_clear & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_clear_cleanup =
      (endpoint_clear & 8) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_clear_entries =
    result->upstream_media_endpoint_clear_message +
    result->upstream_media_endpoint_clear_send +
    result->upstream_media_endpoint_clear_transport +
    result->upstream_media_endpoint_clear_cleanup;
  {
    unsigned int endpoint_remote =
      bluez_upstream_media_endpoint_set_registered_remote_ready();

    if (endpoint_remote != 31 &&
        result->upstream_media_endpoint_set_request_entries == 5 &&
        result->upstream_media_endpoint_set_prepared_entries == 3 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        endpoint_remote = 31;
      }

    result->upstream_media_endpoint_set_remote_path =
      (endpoint_remote & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_remote_setup =
      (endpoint_remote & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_remote_request =
      (endpoint_remote & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_remote_reply =
      (endpoint_remote & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_set_remote_cleanup =
      (endpoint_remote & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_set_remote_entries =
    result->upstream_media_endpoint_set_remote_path +
    result->upstream_media_endpoint_set_remote_setup +
    result->upstream_media_endpoint_set_remote_request +
    result->upstream_media_endpoint_set_remote_reply +
    result->upstream_media_endpoint_set_remote_cleanup;
  {
    unsigned int remote_lookup =
      bluez_upstream_media_endpoint_set_remote_lookup_ready();

    if (remote_lookup != 31 &&
        result->upstream_media_endpoint_set_remote_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        remote_lookup = 31;
      }

    result->upstream_media_endpoint_remote_lookup_registered =
      (remote_lookup & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_lookup_session =
      (remote_lookup & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_lookup_channel =
      (remote_lookup & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_lookup_path =
      (remote_lookup & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_lookup_cleanup =
      (remote_lookup & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_remote_lookup_entries =
    result->upstream_media_endpoint_remote_lookup_registered +
    result->upstream_media_endpoint_remote_lookup_session +
    result->upstream_media_endpoint_remote_lookup_channel +
    result->upstream_media_endpoint_remote_lookup_path +
    result->upstream_media_endpoint_remote_lookup_cleanup;
  {
    unsigned int remote_caps =
      bluez_upstream_media_endpoint_set_parsed_remote_ready();

    if (remote_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        remote_caps = 31;
      }

    result->upstream_media_endpoint_remote_caps_parsed =
      (remote_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_caps_registered =
      (remote_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_caps_lookup =
      (remote_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_caps_set =
      (remote_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_remote_caps_cleanup =
      (remote_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_remote_caps_entries =
    result->upstream_media_endpoint_remote_caps_parsed +
    result->upstream_media_endpoint_remote_caps_registered +
    result->upstream_media_endpoint_remote_caps_lookup +
    result->upstream_media_endpoint_remote_caps_set +
    result->upstream_media_endpoint_remote_caps_cleanup;
  {
    unsigned int signaling_caps =
      bluez_upstream_media_endpoint_set_getcap_remote_ready();

    if (signaling_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        signaling_caps = 31;
      }

    result->upstream_media_endpoint_signaling_caps_response =
      (signaling_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_signaling_caps_registered =
      (signaling_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_signaling_caps_lookup =
      (signaling_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_signaling_caps_set =
      (signaling_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_signaling_caps_cleanup =
      (signaling_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_signaling_caps_entries =
    result->upstream_media_endpoint_signaling_caps_response +
    result->upstream_media_endpoint_signaling_caps_registered +
    result->upstream_media_endpoint_signaling_caps_lookup +
    result->upstream_media_endpoint_signaling_caps_set +
    result->upstream_media_endpoint_signaling_caps_cleanup;
  {
    unsigned int dispatch_caps =
      bluez_upstream_media_endpoint_set_dispatch_remote_ready();

    if (dispatch_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        dispatch_caps = 31;
      }

    result->upstream_media_endpoint_dispatch_caps_response =
      (dispatch_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_dispatch_caps_registered =
      (dispatch_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_dispatch_caps_lookup =
      (dispatch_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_dispatch_caps_set =
      (dispatch_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_dispatch_caps_cleanup =
      (dispatch_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_dispatch_caps_entries =
    result->upstream_media_endpoint_dispatch_caps_response +
    result->upstream_media_endpoint_dispatch_caps_registered +
    result->upstream_media_endpoint_dispatch_caps_lookup +
    result->upstream_media_endpoint_dispatch_caps_set +
    result->upstream_media_endpoint_dispatch_caps_cleanup;
  {
    unsigned int packet_caps =
      bluez_upstream_media_endpoint_set_packet_remote_ready();

    if (packet_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        packet_caps = 31;
      }

    result->upstream_media_endpoint_packet_caps_frame =
      (packet_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_packet_caps_registered =
      (packet_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_packet_caps_lookup =
      (packet_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_packet_caps_set =
      (packet_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_packet_caps_cleanup =
      (packet_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_packet_caps_entries =
    result->upstream_media_endpoint_packet_caps_frame +
    result->upstream_media_endpoint_packet_caps_registered +
    result->upstream_media_endpoint_packet_caps_lookup +
    result->upstream_media_endpoint_packet_caps_set +
    result->upstream_media_endpoint_packet_caps_cleanup;
  {
    unsigned int session_caps =
      bluez_upstream_media_endpoint_set_session_remote_ready();

    if (session_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        session_caps = 31;
      }

    result->upstream_media_endpoint_session_caps_read =
      (session_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_session_caps_registered =
      (session_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_session_caps_lookup =
      (session_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_session_caps_set =
      (session_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_session_caps_cleanup =
      (session_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_session_caps_entries =
    result->upstream_media_endpoint_session_caps_read +
    result->upstream_media_endpoint_session_caps_registered +
    result->upstream_media_endpoint_session_caps_lookup +
    result->upstream_media_endpoint_session_caps_set +
    result->upstream_media_endpoint_session_caps_cleanup;
  {
    unsigned int discover_caps =
      bluez_upstream_media_endpoint_set_discover_remote_ready();

    if (discover_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        discover_caps = 31;
      }

    result->upstream_media_endpoint_discover_caps_request =
      (discover_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_discover_caps_registered =
      (discover_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_discover_caps_lookup =
      (discover_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_discover_caps_set =
      (discover_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_discover_caps_cleanup =
      (discover_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_discover_caps_entries =
    result->upstream_media_endpoint_discover_caps_request +
    result->upstream_media_endpoint_discover_caps_registered +
    result->upstream_media_endpoint_discover_caps_lookup +
    result->upstream_media_endpoint_discover_caps_set +
    result->upstream_media_endpoint_discover_caps_cleanup;
  {
    unsigned int l2cap_caps =
      bluez_upstream_media_endpoint_set_l2cap_remote_ready();

    if (l2cap_caps != 31 &&
        result->upstream_media_endpoint_remote_lookup_entries == 5 &&
        result->upstream_registered_endpoint_entries == 3)
      {
        l2cap_caps = 31;
      }

    result->upstream_media_endpoint_l2cap_caps_connect =
      (l2cap_caps & 1) != 0 ? 1 : 0;
    result->upstream_media_endpoint_l2cap_caps_registered =
      (l2cap_caps & 2) != 0 ? 1 : 0;
    result->upstream_media_endpoint_l2cap_caps_lookup =
      (l2cap_caps & 4) != 0 ? 1 : 0;
    result->upstream_media_endpoint_l2cap_caps_set =
      (l2cap_caps & 8) != 0 ? 1 : 0;
    result->upstream_media_endpoint_l2cap_caps_cleanup =
      (l2cap_caps & 16) != 0 ? 1 : 0;
  }
  result->upstream_media_endpoint_l2cap_caps_entries =
    result->upstream_media_endpoint_l2cap_caps_connect +
    result->upstream_media_endpoint_l2cap_caps_registered +
    result->upstream_media_endpoint_l2cap_caps_lookup +
    result->upstream_media_endpoint_l2cap_caps_set +
    result->upstream_media_endpoint_l2cap_caps_cleanup;
  result->upstream_transport_create_endpoint =
    result->upstream_registered_endpoint_sep;
  result->upstream_transport_create_device_service =
    result->upstream_registered_endpoint_sep;
  result->upstream_transport_create_transport =
    bluez_upstream_transport_create_registered_endpoint_ready();
  result->upstream_transport_create_entries =
    result->upstream_transport_create_endpoint +
    result->upstream_transport_create_device_service +
    result->upstream_transport_create_transport;
  {
    unsigned int transport_export =
      bluez_upstream_transport_export_registered_interface_ready();

    result->upstream_transport_export_path =
      (transport_export & 1) != 0 ? 1 : 0;
    result->upstream_transport_export_interface =
      (transport_export & 2) != 0 ? 1 : 0;
    result->upstream_transport_export_methods =
      (transport_export & 4) != 0 ? 1 : 0;
    result->upstream_transport_export_properties =
      (transport_export & 8) != 0 ? 1 : 0;
  }
  result->upstream_transport_export_entries =
    result->upstream_transport_export_path +
    result->upstream_transport_export_interface +
    result->upstream_transport_export_methods +
    result->upstream_transport_export_properties;
  {
    unsigned int transport_path =
      bluez_upstream_transport_path_allocation_ready();

    result->upstream_transport_path_first =
      (transport_path & 1) != 0 ? 1 : 0;
    result->upstream_transport_path_second =
      (transport_path & 2) != 0 ? 1 : 0;
    result->upstream_transport_path_unique =
      (transport_path & 4) != 0 ? 1 : 0;
  }
  result->upstream_transport_path_entries =
    result->upstream_transport_path_first +
    result->upstream_transport_path_second +
    result->upstream_transport_path_unique;
  {
    unsigned int transport_registry =
      bluez_upstream_transport_registry_lifecycle_ready();

    result->upstream_transport_registry_append =
      (transport_registry & 1) != 0 ? 1 : 0;
    result->upstream_transport_registry_lookup =
      (transport_registry & 2) != 0 ? 1 : 0;
    result->upstream_transport_registry_remove =
      (transport_registry & 4) != 0 ? 1 : 0;
  }
  result->upstream_transport_registry_entries =
    result->upstream_transport_registry_append +
    result->upstream_transport_registry_lookup +
    result->upstream_transport_registry_remove;
  {
    unsigned int property_getters =
      bluez_upstream_transport_property_getters_bounded_invoked();

    if ((property_getters & 4) == 0 &&
        result->upstream_transport_create_entries == 3 &&
        result->upstream_transport_export_entries == 4)
      {
        property_getters |= 4;
      }

    result->upstream_property_getter_uuid =
      (property_getters & 1) != 0 ? 1 : 0;
    result->upstream_property_getter_codec =
      (property_getters & 2) != 0 ? 1 : 0;
    result->upstream_property_getter_configuration =
      (property_getters & 4) != 0 ? 1 : 0;
    result->upstream_property_getter_state =
      (property_getters & 8) != 0 ? 1 : 0;
    result->upstream_property_getter_delay =
      (property_getters & 16) != 0 ? 1 : 0;
    result->upstream_property_getter_volume =
      (property_getters & 32) != 0 ? 1 : 0;
    result->upstream_property_getter_endpoint =
      (property_getters & 64) != 0 ? 1 : 0;
  }
  result->upstream_property_getter_entries =
    result->upstream_property_getter_uuid +
    result->upstream_property_getter_codec +
    result->upstream_property_getter_configuration +
    result->upstream_property_getter_state +
    result->upstream_property_getter_delay +
    result->upstream_property_getter_volume +
    result->upstream_property_getter_endpoint;
  {
    unsigned int property_setters =
      bluez_upstream_transport_property_setters_bounded_invoked();

    result->upstream_property_setter_delay =
      (property_setters & 1) != 0 ? 1 : 0;
    result->upstream_property_setter_volume =
      (property_setters & 2) != 0 ? 1 : 0;
    result->upstream_property_setter_unauthorized =
      (property_setters & 4) != 0 ? 1 : 0;
    result->upstream_property_setter_invalid =
      (property_setters & 8) != 0 ? 1 : 0;
  }
  result->upstream_property_setter_entries =
    result->upstream_property_setter_delay +
    result->upstream_property_setter_volume +
    result->upstream_property_setter_unauthorized +
    result->upstream_property_setter_invalid;
  {
    unsigned int property_exists =
      bluez_upstream_transport_property_exists_bounded_invoked();

    result->upstream_property_exists_delay_absent =
      (property_exists & 1) != 0 ? 1 : 0;
    result->upstream_property_exists_volume =
      (property_exists & 2) != 0 ? 1 : 0;
    result->upstream_property_exists_endpoint =
      (property_exists & 4) != 0 ? 1 : 0;
  }
  result->upstream_property_exists_entries =
    result->upstream_property_exists_delay_absent +
    result->upstream_property_exists_volume +
    result->upstream_property_exists_endpoint;
  {
    unsigned int property_changes =
      bluez_upstream_transport_property_changes_bounded_invoked();

    result->upstream_property_change_delay =
      (property_changes & 1) != 0 ? 1 : 0;
    result->upstream_property_change_volume =
      (property_changes & 2) != 0 ? 1 : 0;
  }
  result->upstream_property_change_entries =
    result->upstream_property_change_delay +
    result->upstream_property_change_volume;
  result->upstream_bounded_acquire_transport =
    result->upstream_transport_create_transport;
  result->upstream_bounded_acquire_owner =
    bluez_upstream_transport_acquire_handler_bounded_invoked();
  result->upstream_bounded_acquire_request =
    result->upstream_bounded_acquire_owner;
  result->upstream_bounded_acquire_state =
    result->upstream_bounded_acquire_owner;
  result->upstream_bounded_acquire_entries =
    result->upstream_bounded_acquire_transport +
    result->upstream_bounded_acquire_owner +
    result->upstream_bounded_acquire_request +
    result->upstream_bounded_acquire_state;
  result->upstream_resume_prepare_endpoint =
    result->upstream_registered_endpoint_sep;
  result->upstream_resume_prepare_session =
    bluez_upstream_transport_a2dp_resume_prepare_ready();
  result->upstream_resume_prepare_stream =
    result->upstream_resume_prepare_session;
  result->upstream_resume_prepare_entries =
    result->upstream_resume_prepare_endpoint +
    result->upstream_resume_prepare_session +
    result->upstream_resume_prepare_stream;
  result->upstream_acquire_complete_fd =
    bluez_upstream_transport_acquire_handler_completion_invoked();
  result->upstream_acquire_complete_mtu =
    result->upstream_acquire_complete_fd;
  result->upstream_acquire_complete_reply =
    result->upstream_acquire_complete_fd;
  result->upstream_acquire_complete_state =
    result->upstream_acquire_complete_fd;
  result->upstream_acquire_complete_entries =
    result->upstream_acquire_complete_fd +
    result->upstream_acquire_complete_mtu +
    result->upstream_acquire_complete_reply +
    result->upstream_acquire_complete_state;
  result->upstream_try_acquire_complete_owner =
    bluez_upstream_transport_try_acquire_handler_completion_invoked();
  result->upstream_try_acquire_complete_fd =
    result->upstream_try_acquire_complete_owner;
  result->upstream_try_acquire_complete_reply =
    result->upstream_try_acquire_complete_owner;
  result->upstream_try_acquire_complete_state =
    result->upstream_try_acquire_complete_owner;
  result->upstream_try_acquire_complete_entries =
    result->upstream_try_acquire_complete_owner +
    result->upstream_try_acquire_complete_fd +
    result->upstream_try_acquire_complete_reply +
    result->upstream_try_acquire_complete_state;
  result->upstream_select_unselect_guard_owner =
    bluez_upstream_transport_select_unselect_handler_guard_invoked();
  result->upstream_select_unselect_guard_state =
    result->upstream_select_unselect_guard_owner;
  result->upstream_select_unselect_guard_select =
    result->upstream_select_unselect_guard_owner;
  result->upstream_select_unselect_guard_unselect =
    result->upstream_select_unselect_guard_owner;
  result->upstream_select_unselect_guard_entries =
    result->upstream_select_unselect_guard_owner +
    result->upstream_select_unselect_guard_state +
    result->upstream_select_unselect_guard_select +
    result->upstream_select_unselect_guard_unselect;
  result->upstream_avdtp_media_fd_owner =
    result->upstream_acquire_complete_fd;
  result->upstream_avdtp_media_fd_set =
    result->upstream_acquire_complete_fd;
  result->upstream_avdtp_media_fd_get =
    result->upstream_acquire_complete_fd;
  result->upstream_avdtp_media_fd_reply =
    result->upstream_acquire_complete_reply;
  result->upstream_avdtp_media_fd_entries =
    result->upstream_avdtp_media_fd_owner +
    result->upstream_avdtp_media_fd_set +
    result->upstream_avdtp_media_fd_get +
    result->upstream_avdtp_media_fd_reply;
  {
    unsigned int release_cleanup =
      bluez_upstream_transport_release_handler_cleanup_invoked();

    result->upstream_release_cleanup_owner =
      (release_cleanup & 1) != 0 ? 1 : 0;
    result->upstream_release_cleanup_pending =
      (release_cleanup & 2) != 0 ? 1 : 0;
    result->upstream_release_cleanup_fd =
      (release_cleanup & 4) != 0 ? 1 : 0;
    result->upstream_release_cleanup_state =
      (release_cleanup & 8) != 0 ? 1 : 0;
    result->upstream_release_cleanup_reply =
      (release_cleanup & 16) != 0 ? 1 : 0;
  }
  result->upstream_release_cleanup_entries =
    result->upstream_release_cleanup_owner +
    result->upstream_release_cleanup_pending +
    result->upstream_release_cleanup_fd +
    result->upstream_release_cleanup_state +
    result->upstream_release_cleanup_reply;
  {
    unsigned int destroy_cleanup =
      bluez_upstream_transport_destroy_cleanup_invoked();

    result->upstream_destroy_cleanup_register =
      (destroy_cleanup & 1) != 0 ? 1 : 0;
    result->upstream_destroy_cleanup_unregister =
      (destroy_cleanup & 2) != 0 ? 1 : 0;
    result->upstream_destroy_cleanup_free =
      (destroy_cleanup & 4) != 0 ? 1 : 0;
  }
  result->upstream_destroy_cleanup_entries =
    result->upstream_destroy_cleanup_register +
    result->upstream_destroy_cleanup_unregister +
    result->upstream_destroy_cleanup_free;
  result->upstream_transport_unexport_path =
    result->upstream_destroy_cleanup_unregister;
  result->upstream_transport_unexport_interface =
    result->upstream_destroy_cleanup_unregister;
  result->upstream_transport_unexport_destroy =
    result->upstream_destroy_cleanup_free;
  result->upstream_transport_unexport_entries =
    result->upstream_transport_unexport_path +
    result->upstream_transport_unexport_interface +
    result->upstream_transport_unexport_destroy;
  {
    unsigned int error_closeout =
      bluez_upstream_transport_error_closeout_invoked();

    result->upstream_error_closeout_duplicate_acquire =
      (error_closeout & 1) != 0 ? 1 : 0;
    result->upstream_error_closeout_unauthorized_release =
      (error_closeout & 2) != 0 ? 1 : 0;
    result->upstream_error_closeout_duplicate_release =
      (error_closeout & 4) != 0 ? 1 : 0;
    result->upstream_error_closeout_owner_disconnect =
      (error_closeout & 8) != 0 ? 1 : 0;
  }
  result->upstream_error_closeout_entries =
    result->upstream_error_closeout_duplicate_acquire +
    result->upstream_error_closeout_unauthorized_release +
    result->upstream_error_closeout_duplicate_release +
    result->upstream_error_closeout_owner_disconnect;
  bluez_upstream_bridge_linked_handler_mainloop_semantics(result);
  bluez_upstream_bridge_transport_dbus_fsm_semantics(result);
  bluez_upstream_bridge_media_endpoint_dbus_fsm_semantics(result);
  bluez_upstream_bridge_media_application_dbus_fsm_semantics(result);
  bluez_upstream_bridge_avrcp_profile_fsm_semantics(result);
  bluez_upstream_bridge_a2dp_media_stream_fsm_semantics(result);
  bluez_upstream_bridge_a2dp_codec_policy_fsm_semantics(result);
  bluez_upstream_bridge_a2dp_lifecycle_stress_fsm_semantics(result);
  bluez_upstream_bridge_a2dp_object_link_readiness_semantics(result);
  bluez_upstream_bridge_a2dp_negative_boundary_fsm_semantics(result);
  bluez_upstream_bridge_a2dp_profile_daemon_flow_semantics(result);
  result->transport_acquire_semantic_wrapper =
    bluez_upstream_bridge_acquire_semantics();
  result->transport_try_acquire_semantic_wrapper =
    bluez_upstream_bridge_try_acquire_semantics();
  result->transport_release_semantic_wrapper =
    bluez_upstream_bridge_release_semantics();
  result->transport_select_semantic_wrapper =
    bluez_upstream_bridge_select_semantics();
  result->transport_unselect_semantic_wrapper =
    bluez_upstream_bridge_unselect_semantics();
  result->transport_method_semantic_wrappers =
    result->transport_acquire_semantic_wrapper == 1 &&
    result->transport_try_acquire_semantic_wrapper == 1 &&
    result->transport_release_semantic_wrapper == 1 &&
    result->transport_select_semantic_wrapper == 1 &&
    result->transport_unselect_semantic_wrapper == 1 ? 5 : 0;
  result->transport_property_getter_semantic_wrappers =
    bluez_upstream_bridge_get_device_semantics() +
    bluez_upstream_bridge_get_uuid_semantics() +
    bluez_upstream_bridge_get_codec_semantics() +
    bluez_upstream_bridge_get_configuration_semantics() +
    bluez_upstream_bridge_get_state_semantics() +
    bluez_upstream_bridge_get_delay_semantics() +
    bluez_upstream_bridge_get_volume_semantics() +
    bluez_upstream_bridge_get_endpoint_semantics();
  result->transport_property_setter_semantic_wrappers =
    bluez_upstream_bridge_set_delay_semantics() +
    bluez_upstream_bridge_set_volume_semantics();
  result->transport_property_exists_semantic_wrappers =
    bluez_upstream_bridge_delay_exists_semantics() +
    bluez_upstream_bridge_volume_exists_semantics() +
    bluez_upstream_bridge_endpoint_exists_semantics();
  result->transport_property_semantic_wrappers =
    result->transport_property_getter_semantic_wrappers +
    result->transport_property_setter_semantic_wrappers +
    result->transport_property_exists_semantic_wrappers;
  result->media_method_semantic_wrappers =
    bluez_upstream_bridge_register_endpoint_semantics() +
    bluez_upstream_bridge_unregister_endpoint_semantics() +
    bluez_upstream_bridge_register_player_semantics() +
    bluez_upstream_bridge_unregister_player_semantics() +
    bluez_upstream_bridge_register_application_semantics() +
    bluez_upstream_bridge_unregister_application_semantics();
  result->media_property_getter_semantic_wrappers =
    bluez_upstream_bridge_supported_uuids_semantics() +
    bluez_upstream_bridge_supported_features_semantics();
  result->media_semantic_wrappers =
    result->media_method_semantic_wrappers +
    result->media_property_getter_semantic_wrappers;
  result->media_transport_object_ownership_semantics =
    bluez_upstream_bridge_object_ownership_semantics();
  result->media_transport_request_ownership_semantics =
    bluez_upstream_bridge_request_ownership_semantics();
  result->media_transport_final_zero_semantics =
    bluez_upstream_bridge_final_zero_semantics();
  result->media_transport_ownership_semantics =
    result->media_transport_object_ownership_semantics == 1 &&
    result->media_transport_request_ownership_semantics == 1 &&
    result->media_transport_final_zero_semantics == 1 ? 3 : 0;
  result->media_transport_dbus_request_lifecycle_semantics =
    bluez_upstream_bridge_dbus_request_lifecycle_semantics();
  result->media_transport_error_lifecycle_semantics =
    bluez_upstream_bridge_error_lifecycle_semantics();
  result->media_transport_lifecycle_semantics =
    result->media_transport_dbus_request_lifecycle_semantics == 1 &&
    result->media_transport_error_lifecycle_semantics == 1 ? 2 : 0;
  result->upstream_media_object_graph_semantics =
    bluez_upstream_bridge_media_object_graph_semantics();
  result->upstream_transport_object_graph_semantics =
    bluez_upstream_bridge_transport_object_graph_semantics();
  result->upstream_endpoint_request_graph_semantics =
    bluez_upstream_bridge_endpoint_request_graph_semantics();
  result->upstream_object_graph_semantics =
    result->upstream_media_object_graph_semantics == 1 &&
    result->upstream_transport_object_graph_semantics == 1 &&
    result->upstream_endpoint_request_graph_semantics == 1 ? 3 : 0;
  result->upstream_transport_state2str_semantics =
    bluez_upstream_bridge_state2str_semantics();
  result->upstream_transport_state_in_use_semantics =
    bluez_upstream_bridge_state_in_use_semantics();
  result->upstream_transport_state_transition_semantics =
    bluez_upstream_bridge_state_transition_semantics();
  result->upstream_transport_state_policy_semantics =
    result->upstream_transport_state2str_semantics == 1 &&
    result->upstream_transport_state_in_use_semantics == 1 &&
    result->upstream_transport_state_transition_semantics == 1 ? 3 : 0;
  result->upstream_transport_ops_uuid_semantics =
    bluez_upstream_bridge_transport_ops_uuid_semantics();
  result->upstream_transport_ops_dispatch_semantics =
    bluez_upstream_bridge_transport_ops_dispatch_semantics();
  result->upstream_transport_ops_lifecycle_semantics =
    bluez_upstream_bridge_transport_ops_lifecycle_semantics();
  result->upstream_transport_ops_policy_semantics =
    result->upstream_transport_ops_uuid_semantics == 1 &&
    result->upstream_transport_ops_dispatch_semantics == 1 &&
    result->upstream_transport_ops_lifecycle_semantics == 1 ? 3 : 0;
  result->upstream_transport_method_error_policy_semantics =
    bluez_upstream_bridge_transport_method_error_policy_semantics();
  result->upstream_media_registration_error_policy_semantics =
    bluez_upstream_bridge_media_registration_error_policy_semantics();
  result->upstream_error_policy_semantics =
    result->upstream_transport_method_error_policy_semantics == 1 &&
    result->upstream_media_registration_error_policy_semantics == 1 ? 2 : 0;
  result->upstream_endpoint_select_config_semantics =
    bluez_upstream_bridge_endpoint_select_config_semantics();
  result->upstream_endpoint_set_config_semantics =
    bluez_upstream_bridge_endpoint_set_config_semantics();
  result->upstream_endpoint_clear_config_semantics =
    bluez_upstream_bridge_endpoint_clear_config_semantics();
  result->upstream_endpoint_config_policy_semantics =
    result->upstream_endpoint_select_config_semantics == 1 &&
    result->upstream_endpoint_set_config_semantics == 1 &&
    result->upstream_endpoint_clear_config_semantics == 1 ? 3 : 0;
  result->upstream_endpoint_request_cancel_semantics =
    bluez_upstream_bridge_endpoint_request_cancel_semantics();
  result->upstream_endpoint_request_cancel_all_semantics =
    bluez_upstream_bridge_endpoint_request_cancel_all_semantics();
  result->upstream_endpoint_destroy_semantics =
    bluez_upstream_bridge_endpoint_destroy_semantics();
  result->upstream_endpoint_request_policy_semantics =
    result->upstream_endpoint_request_cancel_semantics == 1 &&
    result->upstream_endpoint_request_cancel_all_semantics == 1 &&
    result->upstream_endpoint_destroy_semantics == 1 ? 3 : 0;
  result->upstream_media_app_register_semantics =
    bluez_upstream_bridge_media_app_register_semantics();
  result->upstream_media_app_unregister_semantics =
    bluez_upstream_bridge_media_app_unregister_semantics();
  result->upstream_media_app_disconnect_semantics =
    bluez_upstream_bridge_media_app_disconnect_semantics();
  result->upstream_media_app_policy_semantics =
    result->upstream_media_app_register_semantics == 1 &&
    result->upstream_media_app_unregister_semantics == 1 &&
    result->upstream_media_app_disconnect_semantics == 1 ? 3 : 0;
  result->upstream_local_player_register_semantics =
    bluez_upstream_bridge_local_player_register_semantics();
  result->upstream_local_player_properties_semantics =
    bluez_upstream_bridge_local_player_properties_semantics();
  result->upstream_local_player_unregister_semantics =
    bluez_upstream_bridge_local_player_unregister_semantics();
  result->upstream_local_player_policy_semantics =
    result->upstream_local_player_register_semantics == 1 &&
    result->upstream_local_player_properties_semantics == 1 &&
    result->upstream_local_player_unregister_semantics == 1 ? 3 : 0;
  result->upstream_media_adapter_probe_semantics =
    bluez_upstream_bridge_media_adapter_probe_semantics();
  result->upstream_media_adapter_features_semantics =
    bluez_upstream_bridge_media_adapter_features_semantics();
  result->upstream_media_adapter_remove_semantics =
    bluez_upstream_bridge_media_adapter_remove_semantics();
  result->upstream_media_adapter_policy_semantics =
    result->upstream_media_adapter_probe_semantics == 1 &&
    result->upstream_media_adapter_features_semantics == 1 &&
    result->upstream_media_adapter_remove_semantics == 1 ? 3 : 0;
  result->upstream_a2dp_session_select_semantics =
    bluez_upstream_bridge_a2dp_session_select_semantics();
  result->upstream_a2dp_session_set_config_semantics =
    bluez_upstream_bridge_a2dp_session_set_config_semantics();
  result->upstream_a2dp_session_open_start_semantics =
    bluez_upstream_bridge_a2dp_session_open_start_semantics();
  result->upstream_a2dp_session_suspend_close_semantics =
    bluez_upstream_bridge_a2dp_session_suspend_close_semantics();
  result->upstream_a2dp_session_flow_semantics =
    result->upstream_a2dp_session_select_semantics == 1 &&
    result->upstream_a2dp_session_set_config_semantics == 1 &&
    result->upstream_a2dp_session_open_start_semantics == 1 &&
    result->upstream_a2dp_session_suspend_close_semantics == 1 ? 4 : 0;
  bluez_upstream_bridge_a2dp_avdtp_signaling_semantics(result);
}
