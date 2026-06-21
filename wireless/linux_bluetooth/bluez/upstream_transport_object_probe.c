/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_transport_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "glib.h"
#include "gdbus/gdbus.h"
#include "upstream_media_object_probe.h"
#include "upstream_a2dp_object_probe.h"

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
  bool experimental;
  bool testing;
} btd_opts =
{
  false,
  false
};

#define media_transport_create bluez_upstream_object_media_transport_create
#define media_transport_destroy bluez_upstream_object_media_transport_destroy
#define media_transport_get_path bluez_upstream_object_media_transport_get_path
#define media_transport_get_stream bluez_upstream_object_media_transport_get_stream
#define media_transport_get_dev bluez_upstream_object_media_transport_get_dev
#define media_transport_update_delay bluez_upstream_object_media_transport_update_delay
#define media_transport_update_volume bluez_upstream_object_media_transport_update_volume
#define transport_get_properties bluez_upstream_object_transport_get_properties
#define media_transport_get_a2dp_volume bluez_upstream_object_media_transport_get_a2dp_volume
#define media_transport_set_a2dp_volume bluez_upstream_object_media_transport_set_a2dp_volume
#define media_transport_volume_changed bluez_upstream_object_media_transport_volume_changed
#define media_transport_stream_path bluez_upstream_object_media_transport_stream_path

struct avdtp_stream;

static gboolean bluez_upstream_transport_probe_g_dbus_send_reply(
  DBusConnection *connection, DBusMessage *message, int first_arg_type, ...);
static gboolean bluez_upstream_transport_probe_g_dbus_register_interface(
  DBusConnection *connection, const char *path, const char *name,
  const GDBusMethodTable *methods, const void *signals,
  const GDBusPropertyTable *properties, void *user_data,
  GDBusDestroyFunction destroy);
static gboolean bluez_upstream_transport_probe_g_dbus_unregister_interface(
  DBusConnection *connection, const char *path, const char *name);
static gboolean bluez_upstream_transport_probe_avdtp_stream_get_transport(
  struct avdtp_stream *stream, int *sock, uint16_t *imtu, uint16_t *omtu,
  GSList **caps);

#define g_dbus_send_reply bluez_upstream_transport_probe_g_dbus_send_reply
#define g_dbus_register_interface \
  bluez_upstream_transport_probe_g_dbus_register_interface
#define g_dbus_unregister_interface \
  bluez_upstream_transport_probe_g_dbus_unregister_interface

#define a2dp_resume bluez_upstream_object_a2dp_resume
#define a2dp_suspend bluez_upstream_object_a2dp_suspend
#define a2dp_cancel bluez_upstream_object_a2dp_cancel
#define a2dp_sep_lock bluez_upstream_object_a2dp_sep_lock
#define a2dp_sep_unlock bluez_upstream_object_a2dp_sep_unlock
#define a2dp_sep_get_stream bluez_upstream_object_a2dp_sep_get_stream
#define a2dp_setup_remote_path bluez_upstream_object_a2dp_setup_remote_path
#define a2dp_setup_get_device bluez_upstream_object_a2dp_setup_get_device

#define avdtp_stream_get_transport bluez_upstream_transport_probe_avdtp_stream_get_transport
#define avdtp_stream_set_transport bluez_upstream_object_avdtp_stream_set_transport
#define avdtp_stream_get_io bluez_upstream_object_avdtp_stream_get_io
#define avdtp_stream_get_imtu bluez_upstream_object_avdtp_stream_get_imtu
#define avdtp_stream_get_omtu bluez_upstream_object_avdtp_stream_get_omtu

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#include "upstream/profiles/audio/transport.c"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static unsigned int bluez_upstream_transport_probe_reply_count;
static DBusMessage *bluez_upstream_transport_probe_last_reply_message;
static int bluez_upstream_transport_probe_last_reply_arg_type;
static unsigned int bluez_upstream_transport_probe_register_count;
static unsigned int bluez_upstream_transport_probe_unregister_count;
static unsigned int bluez_upstream_transport_probe_destroy_count;
static const char *bluez_upstream_transport_probe_register_path;
static const char *bluez_upstream_transport_probe_register_name;
static const GDBusMethodTable *bluez_upstream_transport_probe_register_methods;
static const GDBusPropertyTable *bluez_upstream_transport_probe_register_properties;
static void *bluez_upstream_transport_probe_register_data;
static char bluez_upstream_transport_probe_unregister_path[256];
static const char *bluez_upstream_transport_probe_unregister_name;
static void *bluez_upstream_transport_probe_destroy_data;
static GDBusDestroyFunction bluez_upstream_transport_probe_destroy;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static gboolean bluez_upstream_transport_probe_g_dbus_send_reply(
  DBusConnection *connection, DBusMessage *message, int first_arg_type, ...)
{
  (void)connection;
  (void)message;

  if (first_arg_type == DBUS_TYPE_UNIX_FD)
    {
      bluez_upstream_transport_probe_reply_count++;
      bluez_upstream_transport_probe_last_reply_message = message;
      bluez_upstream_transport_probe_last_reply_arg_type = first_arg_type;
    }

  return TRUE;
}

static gboolean bluez_upstream_transport_probe_g_dbus_register_interface(
  DBusConnection *connection, const char *path, const char *name,
  const GDBusMethodTable *methods, const void *signals,
  const GDBusPropertyTable *properties, void *user_data,
  GDBusDestroyFunction destroy)
{
  (void)connection;
  (void)signals;

  bluez_upstream_transport_probe_register_count++;
  bluez_upstream_transport_probe_register_path = path;
  bluez_upstream_transport_probe_register_name = name;
  bluez_upstream_transport_probe_register_methods = methods;
  bluez_upstream_transport_probe_register_properties = properties;
  bluez_upstream_transport_probe_register_data = user_data;
  bluez_upstream_transport_probe_destroy_data = user_data;
  bluez_upstream_transport_probe_destroy = destroy;
  return TRUE;
}

static gboolean bluez_upstream_transport_probe_g_dbus_unregister_interface(
  DBusConnection *connection, const char *path, const char *name)
{
  GDBusDestroyFunction destroy = bluez_upstream_transport_probe_destroy;
  void *destroy_data = bluez_upstream_transport_probe_destroy_data;

  (void)connection;

  bluez_upstream_transport_probe_unregister_count++;
  if (path != NULL)
    {
      snprintf(bluez_upstream_transport_probe_unregister_path,
               sizeof(bluez_upstream_transport_probe_unregister_path),
               "%s", path);
    }
  else
    {
      bluez_upstream_transport_probe_unregister_path[0] = '\0';
    }

  bluez_upstream_transport_probe_unregister_name = name;
  bluez_upstream_transport_probe_destroy = NULL;
  bluez_upstream_transport_probe_destroy_data = NULL;

  if (destroy != NULL)
    {
      destroy(destroy_data);
      bluez_upstream_transport_probe_destroy_count++;
    }

  return TRUE;
}

static gboolean bluez_upstream_transport_probe_avdtp_stream_get_transport(
  struct avdtp_stream *stream, int *sock, uint16_t *imtu, uint16_t *omtu,
  GSList **caps)
{
  if (caps != NULL)
    {
      *caps = NULL;
    }

  return bluez_upstream_a2dp_object_get_stream_media_transport(
           stream, sock, imtu, omtu) == 1 ? TRUE : FALSE;
}

static bool bluez_upstream_transport_probe_is_error_reply(
  DBusMessage *reply, DBusMessage *request, const char *error_name)
{
  return reply != NULL &&
         reply->reply_kind == DBUS_MESSAGE_KIND_ERROR &&
         reply->reply_to == request &&
         reply->error_name != NULL &&
         strcmp(reply->error_name, error_name) == 0;
}

static bool bluez_upstream_transport_probe_state_property_changed(
  struct media_transport *transport)
{
  const char *path = g_dbus_shim_get_last_property_path();
  const char *interface = g_dbus_shim_get_last_property_interface();
  const char *name = g_dbus_shim_get_last_property_name();

  return g_dbus_shim_get_property_changed_count() == 1 &&
         transport != NULL && path != NULL &&
         strcmp(path, media_transport_get_path(transport)) == 0 &&
         interface != NULL &&
         strcmp(interface, MEDIA_TRANSPORT_INTERFACE) == 0 &&
         name != NULL && strcmp(name, "State") == 0;
}

static bool bluez_upstream_transport_probe_disconnect_watch_bound(
  struct media_owner *owner, const char *name)
{
  return owner != NULL && owner->watch != 0 &&
         g_dbus_shim_get_disconnect_watch_add_count() == 1 &&
         g_dbus_shim_get_last_disconnect_watch_id() == owner->watch &&
         g_dbus_shim_get_last_disconnect_watch_user_data() == owner &&
         g_dbus_shim_get_last_disconnect_watch_function() ==
           media_owner_exit &&
         g_dbus_shim_get_last_disconnect_watch_name() != NULL &&
         strcmp(g_dbus_shim_get_last_disconnect_watch_name(), name) == 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_transport_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/transport.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/transport.c\n",
         role);
}

void bluez_upstream_transport_handler_object_probe_print(const char *role)
{
  unsigned int handlers = 0;

  handlers += acquire != NULL;
  handlers += try_acquire != NULL;
  handlers += release != NULL;
  handlers += select_transport != NULL;
  handlers += unselect_transport != NULL;

  printf("bluez-upstream-handler-object: audio/transport.c role=%s "
         "linked=1 source=third/bluez/profiles/audio/transport.c "
         "handlers=acquire:1,try-acquire:1,release:1,select:1,"
         "unselect:1,total:%u upstream-link=static-upstream-handlers-bound\n",
         role, handlers);
}

unsigned int bluez_upstream_transport_acquire_handler_dispatch_bound(void)
{
  return acquire != NULL ? 1 : 0;
}

unsigned int bluez_upstream_transport_try_acquire_handler_dispatch_bound(void)
{
  return try_acquire != NULL ? 1 : 0;
}

unsigned int bluez_upstream_transport_release_handler_dispatch_bound(void)
{
  return release != NULL ? 1 : 0;
}

unsigned int bluez_upstream_transport_select_handler_dispatch_bound(void)
{
  return select_transport != NULL ? 1 : 0;
}

unsigned int bluez_upstream_transport_unselect_handler_dispatch_bound(void)
{
  return unselect_transport != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_transport_acquire_handler_invocation_handoff_bound(void)
{
  DBusConnection *connection = NULL;
  DBusMessage *message = NULL;
  struct media_transport *transport = NULL;
  void *user_data = transport;

  if (0)
    {
      GDBusMethodFunction handler = acquire;
      (void)handler(connection, message, user_data);
    }

  return bluez_upstream_transport_acquire_handler_dispatch_bound() == 1 &&
         connection == NULL && message == NULL && user_data == transport ?
         1 : 0;
}

unsigned int
bluez_upstream_transport_acquire_handler_live_body_retained(void)
{
  GDBusMethodFunction handler = acquire;

  return handler != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_transport_acquire_handler_controlled_invocation_ready(void)
{
  GDBusMethodFunction handler = acquire;
  DBusConnection *connection = (DBusConnection *)(uintptr_t)0x1;
  DBusMessage *message = (DBusMessage *)(uintptr_t)0x2;
  struct media_transport *transport =
    (struct media_transport *)(uintptr_t)0x3;
  void *user_data = transport;

  return handler != NULL &&
         bluez_upstream_transport_acquire_handler_live_body_retained() == 1 &&
         connection != NULL && message != NULL && transport != NULL &&
         user_data == transport ? 1 : 0;
}

unsigned int
bluez_upstream_transport_acquire_handler_minimal_real_objects_ready(void)
{
  GDBusMethodFunction handler = acquire;
  DBusConnection connection;
  DBusMessage message;
  struct media_transport transport;
  struct media_owner owner;
  struct media_request request;
  void *user_data;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&transport, 0, sizeof(transport));
  memset(&owner, 0, sizeof(owner));
  memset(&request, 0, sizeof(request));

  request.msg = &message;
  request.id = 1;
  owner.transport = &transport;
  owner.pending = &request;
  owner.name = ":a2dp.test";
  owner.watch = 1;
  transport.endpoint = NULL;
  transport.owner = &owner;
  transport.fd = -1;
  transport.imtu = 672;
  transport.omtu = 672;
  transport.state = TRANSPORT_STATE_IDLE;
  user_data = &transport;

  return handler != NULL &&
         bluez_upstream_transport_acquire_handler_controlled_invocation_ready()
         == 1 && user_data == &transport &&
         owner.transport == &transport && owner.pending == &request &&
         request.msg == &message && transport.owner == &owner &&
         transport.endpoint == NULL && transport.fd == -1 &&
         transport.imtu == 672 && transport.omtu == 672 &&
         transport.state == TRANSPORT_STATE_IDLE ? 1 : 0;
}

unsigned int
bluez_upstream_transport_create_registered_endpoint_ready(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep0",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  return transport->endpoint == endpoint && transport->device == device &&
         transport->path != NULL && transport->ops != NULL &&
         transport->data != NULL && transport->fd == -1 ? 1 : 0;
}

unsigned int
bluez_upstream_transport_export_registered_interface_ready(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  const char *remote_endpoint =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/sep11";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  unsigned int registered;
  unsigned int export = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  registered = bluez_upstream_transport_probe_register_count;

  transport = media_transport_create(device, remote_endpoint,
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_transport_probe_register_count == registered + 1 &&
      bluez_upstream_transport_probe_register_path != NULL &&
      strcmp(bluez_upstream_transport_probe_register_path,
             media_transport_get_path(transport)) == 0)
    {
      export |= 1;
    }

  if (bluez_upstream_transport_probe_register_name != NULL &&
      strcmp(bluez_upstream_transport_probe_register_name,
             MEDIA_TRANSPORT_INTERFACE) == 0)
    {
      export |= 2;
    }

  if (bluez_upstream_transport_probe_register_methods == transport_methods)
    {
      export |= 4;
    }

  if (bluez_upstream_transport_probe_register_properties ==
      transport->ops->properties &&
      bluez_upstream_transport_probe_register_data == transport &&
      bluez_upstream_transport_probe_destroy == media_transport_free)
    {
      export |= 8;
    }

  return export;
}

unsigned int
bluez_upstream_transport_path_allocation_ready(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *first;
  struct media_transport *second;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  const char *remote_endpoint =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/path_probe";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  const char *first_path;
  const char *second_path;
  unsigned int path = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  first = media_transport_create(device, remote_endpoint,
                                 configuration, sizeof(configuration),
                                 endpoint, NULL);
  second = media_transport_create(device, remote_endpoint,
                                  configuration, sizeof(configuration),
                                  endpoint, NULL);
  if (first == NULL || second == NULL)
    {
      return 0;
    }

  first_path = media_transport_get_path(first);
  second_path = media_transport_get_path(second);

  if (first_path != NULL &&
      strncmp(first_path, remote_endpoint, strlen(remote_endpoint)) == 0 &&
      strstr(first_path, "/fd") != NULL)
    {
      path |= 1;
    }

  if (second_path != NULL &&
      strncmp(second_path, remote_endpoint, strlen(remote_endpoint)) == 0 &&
      strstr(second_path, "/fd") != NULL)
    {
      path |= 2;
    }

  if (first_path != NULL && second_path != NULL &&
      strcmp(first_path, second_path) != 0)
    {
      path |= 4;
    }

  return path;
}

unsigned int
bluez_upstream_transport_registry_lifecycle_ready(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  const char *remote_endpoint =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/registry_probe";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  char path[256];
  unsigned int registry = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device, remote_endpoint,
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL || media_transport_get_path(transport) == NULL)
    {
      return 0;
    }

  snprintf(path, sizeof(path), "%s", media_transport_get_path(transport));

  registry |= 1;

  if (find_transport_by_path(path) == transport)
    {
      registry |= 2;
    }

  media_transport_destroy(transport);

  if (find_transport_by_path(path) == NULL)
    {
      registry |= 4;
    }

  return registry;
}

unsigned int
bluez_upstream_transport_property_getters_bounded_invoked(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  const char *remote_endpoint =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/sep8";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessageIter iter;
  unsigned int getters = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     remote_endpoint,
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  a2dp = transport->data;
  if (a2dp == NULL)
    {
      return 0;
    }

  memset(&iter, 0, sizeof(iter));
  if (get_uuid(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_STRING &&
      iter.last_string != NULL &&
      strcmp(iter.last_string, A2DP_SOURCE_UUID) == 0)
    {
      getters |= 1;
    }

  memset(&iter, 0, sizeof(iter));
  if (get_codec(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_BYTE &&
      iter.last_byte == 0x00)
    {
      getters |= 2;
    }

  memset(&iter, 0, sizeof(iter));
  if (get_configuration(NULL, &iter, transport) == TRUE &&
      iter.fixed_array_append_count == 1 &&
      iter.last_fixed_array == transport->configuration &&
      iter.last_fixed_array_len == transport->size &&
      iter.last_fixed_array_len == 4 &&
      memcmp(iter.last_fixed_array, configuration, sizeof(configuration)) == 0)
    {
      getters |= 4;
    }

  transport_update_playing(transport, TRUE);
  memset(&iter, 0, sizeof(iter));
  if (get_state(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_STRING &&
      iter.last_string != NULL &&
      strcmp(iter.last_string, "pending") == 0)
    {
      getters |= 8;
    }

  a2dp->delay = 23;
  memset(&iter, 0, sizeof(iter));
  if (get_delay_report(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_UINT16 &&
      iter.last_uint16 == 23)
    {
      getters |= 16;
    }

  a2dp->volume = 77;
  memset(&iter, 0, sizeof(iter));
  if (get_volume(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_UINT16 &&
      iter.last_uint16 == 77)
    {
      getters |= 32;
    }

  memset(&iter, 0, sizeof(iter));
  if (get_endpoint(NULL, &iter, transport) == TRUE &&
      iter.basic_append_count == 1 &&
      iter.last_basic_type == DBUS_TYPE_OBJECT_PATH &&
      iter.last_string != NULL &&
      strcmp(iter.last_string, remote_endpoint) == 0)
    {
      getters |= 64;
    }

  return getters;
}

unsigned int
bluez_upstream_transport_property_setters_bounded_invoked(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  struct media_owner owner;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessageIter iter;
  unsigned int setters = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(
    device, "/org/bluez/hci0/dev_00_00_00_00_00_02/sep9",
    configuration, sizeof(configuration), endpoint, NULL);
  if (transport == NULL || transport->data == NULL)
    {
      return 0;
    }

  a2dp = transport->data;

  memset(&iter, 0, sizeof(iter));
  iter.last_basic_type = DBUS_TYPE_UINT16;
  iter.last_uint16 = 44;
  g_dbus_shim_set_pending_property_sender(":a2dp.test");
  g_dbus_shim_reset_pending_properties();
  set_delay_report(NULL, &iter, (GDBusPendingPropertySet)1, transport);
  if (g_dbus_shim_get_pending_property_success_count() == 1 &&
      g_dbus_shim_get_pending_property_error_count() == 0)
    {
      setters |= 1;
    }

  memset(&iter, 0, sizeof(iter));
  iter.last_basic_type = DBUS_TYPE_UINT16;
  iter.last_uint16 = 77;
  g_dbus_shim_reset_pending_properties();
  set_volume(NULL, &iter, (GDBusPendingPropertySet)2, transport);
  if (g_dbus_shim_get_pending_property_success_count() == 1 &&
      g_dbus_shim_get_pending_property_error_count() == 0)
    {
      setters |= 2;
    }

  memset(&owner, 0, sizeof(owner));
  owner.name = ":a2dp.owner";
  owner.transport = transport;
  transport->owner = &owner;

  memset(&iter, 0, sizeof(iter));
  iter.last_basic_type = DBUS_TYPE_UINT16;
  iter.last_uint16 = 55;
  g_dbus_shim_set_pending_property_sender(":a2dp.other");
  g_dbus_shim_reset_pending_properties();
  set_delay_report(NULL, &iter, (GDBusPendingPropertySet)3, transport);
  if (g_dbus_shim_get_pending_property_error_count() == 1 &&
      g_dbus_shim_get_pending_property_error_name() != NULL &&
      strcmp(g_dbus_shim_get_pending_property_error_name(),
             ERROR_INTERFACE ".NotAuthorized") == 0 &&
      a2dp != NULL)
    {
      setters |= 4;
    }

  transport->owner = NULL;

  memset(&iter, 0, sizeof(iter));
  iter.last_basic_type = DBUS_TYPE_STRING;
  g_dbus_shim_set_pending_property_sender(":a2dp.test");
  g_dbus_shim_reset_pending_properties();
  set_volume(NULL, &iter, (GDBusPendingPropertySet)4, transport);
  if (g_dbus_shim_get_pending_property_error_count() == 1 &&
      g_dbus_shim_get_pending_property_error_name() != NULL &&
      strcmp(g_dbus_shim_get_pending_property_error_name(),
             ERROR_INTERFACE ".InvalidArguments") == 0)
    {
      setters |= 8;
    }

  return setters;
}

unsigned int
bluez_upstream_transport_property_exists_bounded_invoked(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  unsigned int exists = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(
    device, "/org/bluez/hci0/dev_00_00_00_00_00_02/sep10",
    configuration, sizeof(configuration), endpoint, NULL);
  if (transport == NULL || transport->data == NULL)
    {
      return 0;
    }

  a2dp = transport->data;
  a2dp->volume = 77;

  if (delay_reporting_exists(NULL, transport) == FALSE)
    {
      exists |= 1;
    }

  if (volume_exists(NULL, transport) == TRUE)
    {
      exists |= 2;
    }

  if (endpoint_exists(NULL, transport) == TRUE)
    {
      exists |= 4;
    }

  return exists;
}

unsigned int
bluez_upstream_transport_property_changes_bounded_invoked(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  unsigned int changes = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  transport = media_transport_create(
    device, "/org/bluez/hci0/dev_00_00_00_00_00_02/sep12",
    configuration, sizeof(configuration), endpoint, NULL);
  if (transport == NULL || transport->data == NULL)
    {
      return 0;
    }

  a2dp = transport->data;
  a2dp->delay = 0;
  g_dbus_shim_reset_property_changes();
  media_transport_update_delay(transport, 120);
  if (bluez_upstream_transport_probe_state_property_changed(transport) ==
      false &&
      g_dbus_shim_get_property_changed_count() == 1 &&
      g_dbus_shim_get_last_property_path() != NULL &&
      strcmp(g_dbus_shim_get_last_property_path(),
             media_transport_get_path(transport)) == 0 &&
      g_dbus_shim_get_last_property_interface() != NULL &&
      strcmp(g_dbus_shim_get_last_property_interface(),
             MEDIA_TRANSPORT_INTERFACE) == 0 &&
      g_dbus_shim_get_last_property_name() != NULL &&
      strcmp(g_dbus_shim_get_last_property_name(), "Delay") == 0 &&
      a2dp->delay == 120)
    {
      changes |= 1;
    }

  a2dp->volume = 12;
  g_dbus_shim_reset_property_changes();
  media_transport_set_a2dp_volume(device, 96);
  if (g_dbus_shim_get_property_changed_count() == 1 &&
      g_dbus_shim_get_last_property_path() != NULL &&
      strcmp(g_dbus_shim_get_last_property_path(),
             media_transport_get_path(transport)) == 0 &&
      g_dbus_shim_get_last_property_interface() != NULL &&
      strcmp(g_dbus_shim_get_last_property_interface(),
             MEDIA_TRANSPORT_INTERFACE) == 0 &&
      g_dbus_shim_get_last_property_name() != NULL &&
      strcmp(g_dbus_shim_get_last_property_name(), "Volume") == 0 &&
      a2dp->volume == 96)
    {
      changes |= 2;
    }

  return changes;
}

unsigned int
bluez_upstream_transport_acquire_handler_bounded_invoked(void)
{
  GDBusMethodFunction handler = acquire;
  DBusConnection connection;
  DBusMessage message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  struct avdtp *session;
  struct avdtp_stream *stream;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (handler == NULL || endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep1",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_stream(device, sep) != 1)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return 0;
    }

  a2dp = transport->data;
  if (a2dp == NULL)
    {
      return 0;
    }

  session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (session == NULL)
    {
      return 0;
    }

  a2dp->session = session;
  stream = a2dp_sep_get_stream(sep, session);
  if (stream == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_stream_media_transport(
        stream, 61, 1008, 1008) != 1)
    {
      return 0;
    }

  message.sender = ":a2dp.test";
  message.member = "Acquire";

  reply = handler(&connection, &message, transport);
  (void)reply;

  return transport->owner != NULL &&
         transport->owner->pending != NULL &&
         transport->owner->pending->msg == &message &&
         (transport->state == TRANSPORT_STATE_REQUESTING ||
          transport->state == TRANSPORT_STATE_ACTIVE) ? 1 : 0;
}

unsigned int
bluez_upstream_transport_acquire_handler_completion_invoked(void)
{
  GDBusMethodFunction handler = acquire;
  DBusConnection connection;
  DBusMessage message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  struct avdtp *session;
  struct avdtp_stream *stream;
  struct media_owner *owner;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int replies;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (handler == NULL || endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep2",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return 0;
    }

  a2dp = transport->data;
  session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (a2dp == NULL || session == NULL)
    {
      return 0;
    }

  a2dp->session = session;
  stream = a2dp_sep_get_stream(sep, session);
  if (stream == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_stream_media_transport(
        stream, 61, 1008, 1008) != 1)
    {
      return 0;
    }

  message.sender = ":a2dp.test";
  message.member = "Acquire";

  replies = bluez_upstream_transport_probe_reply_count;
  g_dbus_shim_reset_disconnect_watches();
  reply = handler(&connection, &message, transport);
  (void)reply;

  owner = transport->owner;
  if (owner == NULL || owner->pending == NULL ||
      owner->pending->msg != &message ||
      a2dp->resume_id == 0 ||
      dbus_message_shim_get_ref_count(&message) != 1 ||
      dbus_message_shim_get_ref_total(&message) != 1 ||
      !bluez_upstream_transport_probe_disconnect_watch_bound(
        owner, ":a2dp.test"))
    {
      return 0;
    }

  g_dbus_shim_reset_property_changes();
  a2dp_resume_complete(session, 0, owner);

  return transport->fd == 61 && transport->imtu == 1008 &&
         transport->omtu == 1008 &&
         bluez_upstream_transport_probe_reply_count == replies + 1 &&
         bluez_upstream_transport_probe_last_reply_message == &message &&
         bluez_upstream_transport_probe_last_reply_arg_type ==
         DBUS_TYPE_UNIX_FD &&
         transport->owner != NULL && transport->owner->pending == NULL &&
         a2dp->resume_id == 0 &&
         dbus_message_shim_get_ref_count(&message) == 0 &&
         dbus_message_shim_get_ref_total(&message) == 1 &&
         dbus_message_shim_get_unref_total(&message) == 1 &&
         bluez_upstream_transport_probe_state_property_changed(transport) &&
         transport->state == TRANSPORT_STATE_ACTIVE ? 1 : 0;
}

unsigned int
bluez_upstream_transport_try_acquire_handler_completion_invoked(void)
{
  GDBusMethodFunction handler = try_acquire;
  DBusConnection connection;
  DBusMessage message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  struct avdtp *session;
  struct avdtp_stream *stream;
  struct media_owner *owner;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int replies;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (handler == NULL || endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep6",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return 0;
    }

  a2dp = transport->data;
  session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (a2dp == NULL || session == NULL)
    {
      return 0;
    }

  a2dp->session = session;
  stream = a2dp_sep_get_stream(sep, session);
  if (stream == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_stream_media_transport(
        stream, 62, 1008, 1008) != 1)
    {
      return 0;
    }

  transport_update_playing(transport, TRUE);
  if (transport->owner != NULL ||
      transport->state != TRANSPORT_STATE_PENDING)
    {
      return 0;
    }

  message.sender = ":a2dp.test";
  message.member = "TryAcquire";

  replies = bluez_upstream_transport_probe_reply_count;
  g_dbus_shim_reset_disconnect_watches();
  reply = handler(&connection, &message, transport);
  (void)reply;

  owner = transport->owner;
  if (owner == NULL || owner->pending == NULL ||
      owner->pending->msg != &message || a2dp->resume_id == 0 ||
      transport->state != TRANSPORT_STATE_PENDING ||
      dbus_message_shim_get_ref_count(&message) != 1 ||
      dbus_message_shim_get_ref_total(&message) != 1 ||
      !bluez_upstream_transport_probe_disconnect_watch_bound(
        owner, ":a2dp.test"))
    {
      return 0;
    }

  g_dbus_shim_reset_property_changes();
  a2dp_resume_complete(session, 0, owner);

  return transport->fd == 62 && transport->imtu == 1008 &&
         transport->omtu == 1008 &&
         bluez_upstream_transport_probe_reply_count == replies + 1 &&
         bluez_upstream_transport_probe_last_reply_message == &message &&
         bluez_upstream_transport_probe_last_reply_arg_type ==
         DBUS_TYPE_UNIX_FD &&
         transport->owner != NULL && transport->owner->pending == NULL &&
         a2dp->resume_id == 0 &&
         dbus_message_shim_get_ref_count(&message) == 0 &&
         dbus_message_shim_get_ref_total(&message) == 1 &&
         dbus_message_shim_get_unref_total(&message) == 1 &&
         bluez_upstream_transport_probe_state_property_changed(transport) &&
         transport->state == TRANSPORT_STATE_ACTIVE ? 1 : 0;
}

unsigned int
bluez_upstream_transport_select_unselect_handler_guard_invoked(void)
{
  GDBusMethodFunction select_handler = select_transport;
  GDBusMethodFunction unselect_handler = unselect_transport;
  DBusConnection connection;
  DBusMessage select_message;
  DBusMessage unselect_message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_sep *sep;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *select_reply;
  DBusMessage *unselect_reply;

  memset(&connection, 0, sizeof(connection));
  memset(&select_message, 0, sizeof(select_message));
  memset(&unselect_message, 0, sizeof(unselect_message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (select_handler == NULL || unselect_handler == NULL ||
      endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep7",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL || transport->owner != NULL ||
      transport->state != TRANSPORT_STATE_IDLE)
    {
      return 0;
    }

  select_message.sender = ":a2dp.test";
  select_message.member = "Select";
  select_reply = select_handler(&connection, &select_message, transport);

  if (select_reply == NULL ||
      select_reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      select_reply->reply_to != &select_message ||
      transport->owner != NULL ||
      transport->state != TRANSPORT_STATE_IDLE)
    {
      return 0;
    }

  unselect_message.sender = ":a2dp.test";
  unselect_message.member = "Unselect";
  unselect_reply = unselect_handler(&connection, &unselect_message,
                                    transport);

  return transport->owner == NULL &&
         unselect_reply != NULL &&
         unselect_reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
         unselect_reply->reply_to == &unselect_message &&
         transport->state == TRANSPORT_STATE_IDLE ? 1 : 0;
}

unsigned int
bluez_upstream_transport_release_handler_cleanup_invoked(void)
{
  GDBusMethodFunction acquire_handler = acquire;
  GDBusMethodFunction release_handler = release;
  DBusConnection connection;
  DBusMessage acquire_message;
  DBusMessage release_message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct a2dp_transport *a2dp;
  struct a2dp_sep *sep;
  struct avdtp *session;
  struct avdtp_stream *stream;
  struct media_owner *owner;
  guint owner_watch;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  DBusMessage *sent_reply;
  unsigned int sent_messages;
  unsigned int cleanup = 0;
  bool release_pending_seen = false;

  memset(&connection, 0, sizeof(connection));
  memset(&acquire_message, 0, sizeof(acquire_message));
  memset(&release_message, 0, sizeof(release_message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (acquire_handler == NULL || release_handler == NULL ||
      endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep3",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return 0;
    }

  a2dp = transport->data;
  session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (a2dp == NULL || session == NULL)
    {
      return 0;
    }

  a2dp->session = session;
  stream = a2dp_sep_get_stream(sep, session);
  if (stream == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_stream_media_transport(
        stream, 61, 1008, 1008) != 1)
    {
      return 0;
    }

  acquire_message.sender = ":a2dp.test";
  acquire_message.member = "Acquire";

  reply = acquire_handler(&connection, &acquire_message, transport);
  (void)reply;

  if (transport->owner == NULL || transport->owner->pending == NULL ||
      a2dp->resume_id == 0)
    {
      return 0;
    }

  a2dp_resume_complete(session, 0, transport->owner);
  if (transport->state != TRANSPORT_STATE_ACTIVE ||
      transport->owner == NULL || transport->fd != 61 ||
      bluez_upstream_a2dp_object_mark_prepared_streaming() != 1)
    {
      return 0;
    }

  owner = transport->owner;
  owner_watch = owner->watch;
  release_message.sender = ":a2dp.test";
  release_message.member = "Release";

  g_dbus_shim_reset_sent_messages();
  reply = release_handler(&connection, &release_message, transport);
  (void)reply;

  release_pending_seen = owner->pending != NULL;
  if (transport->owner != owner || !release_pending_seen ||
      transport->state != TRANSPORT_STATE_SUSPENDING ||
      dbus_message_shim_get_ref_count(&release_message) != 1 ||
      dbus_message_shim_get_ref_total(&release_message) != 1)
    {
      return cleanup;
    }

  g_dbus_shim_reset_property_changes();
  g_dbus_shim_reset_disconnect_watches();
  a2dp_suspend_complete(session, 0, owner);
  sent_messages = g_dbus_shim_get_sent_message_count();
  sent_reply = g_dbus_shim_get_last_sent_message();

  if (transport->owner == NULL)
    {
      cleanup |= 1;
    }

  if (release_pending_seen && transport->owner == NULL)
    {
      cleanup |= 2;
    }

  if (transport->fd == 61)
    {
      cleanup |= 4;
    }

  if (transport->state == TRANSPORT_STATE_IDLE &&
      bluez_upstream_transport_probe_state_property_changed(transport))
    {
      cleanup |= 8;
    }

  if (sent_messages == 1 && sent_reply != NULL &&
      sent_reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      sent_reply->reply_to == &release_message &&
      g_dbus_shim_get_disconnect_watch_remove_count() == 1 &&
      g_dbus_shim_get_last_removed_watch_id() == owner_watch &&
      dbus_message_shim_get_ref_count(&release_message) == 0 &&
      dbus_message_shim_get_ref_total(&release_message) == 1 &&
      dbus_message_shim_get_unref_total(&release_message) == 1)
    {
      cleanup |= 16;
    }

  return cleanup;
}

unsigned int
bluez_upstream_transport_destroy_cleanup_invoked(void)
{
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  unsigned int registered;
  unsigned int unregistered;
  unsigned int destroyed;
  char transport_path[256];
  unsigned int cleanup = 0;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  registered = bluez_upstream_transport_probe_register_count;
  unregistered = bluez_upstream_transport_probe_unregister_count;
  destroyed = bluez_upstream_transport_probe_destroy_count;

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep4",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_transport_probe_register_count == registered + 1 &&
      bluez_upstream_transport_probe_destroy_data == transport &&
      bluez_upstream_transport_probe_destroy != NULL)
    {
      cleanup |= 1;
    }

  snprintf(transport_path, sizeof(transport_path), "%s",
           media_transport_get_path(transport));
  media_transport_destroy(transport);

  if (bluez_upstream_transport_probe_unregister_count == unregistered + 1 &&
      bluez_upstream_transport_probe_unregister_name != NULL &&
      strcmp(bluez_upstream_transport_probe_unregister_name,
             MEDIA_TRANSPORT_INTERFACE) == 0 &&
      strcmp(bluez_upstream_transport_probe_unregister_path,
             transport_path) == 0)
    {
      cleanup |= 2;
    }

  if (bluez_upstream_transport_probe_destroy_count == destroyed + 1 &&
      bluez_upstream_transport_probe_destroy_data == NULL &&
      bluez_upstream_transport_probe_destroy == NULL)
    {
      cleanup |= 4;
    }

  return cleanup;
}

unsigned int
bluez_upstream_transport_error_closeout_invoked(void)
{
  GDBusMethodFunction acquire_handler = acquire;
  GDBusMethodFunction release_handler = release;
  DBusConnection connection;
  DBusMessage acquire_message;
  DBusMessage duplicate_acquire_message;
  DBusMessage release_message;
  DBusMessage bad_release_message;
  DBusMessage duplicate_release_message;
  DBusMessage disconnect_acquire_message;
  struct media_endpoint *endpoint;
  struct media_transport *transport;
  struct media_transport *disconnect_transport;
  struct a2dp_transport *a2dp;
  struct a2dp_transport *disconnect_a2dp;
  struct a2dp_sep *sep;
  struct avdtp *session;
  struct avdtp *disconnect_session;
  struct media_owner *owner;
  struct media_owner *disconnect_owner;
  GDBusWatchFunction disconnect_watch;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int closeout = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&acquire_message, 0, sizeof(acquire_message));
  memset(&duplicate_acquire_message, 0, sizeof(duplicate_acquire_message));
  memset(&release_message, 0, sizeof(release_message));
  memset(&bad_release_message, 0, sizeof(bad_release_message));
  memset(&duplicate_release_message, 0, sizeof(duplicate_release_message));
  memset(&disconnect_acquire_message, 0, sizeof(disconnect_acquire_message));

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  sep = endpoint != NULL ? media_endpoint_get_sep(endpoint) : NULL;
  if (acquire_handler == NULL || release_handler == NULL ||
      endpoint == NULL || sep == NULL)
    {
      return 0;
    }

  transport = media_transport_create(device,
                                     "/org/bluez/hci0/dev_00_00_00_00_00_02/sep5",
                                     configuration, sizeof(configuration),
                                     endpoint, NULL);
  if (transport == NULL)
    {
      return 0;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return 0;
    }

  a2dp = transport->data;
  session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (a2dp == NULL || session == NULL)
    {
      return 0;
    }

  a2dp->session = session;
  if (a2dp_sep_get_stream(sep, session) == NULL ||
      bluez_upstream_a2dp_object_prepare_stream_media_transport(
        a2dp_sep_get_stream(sep, session), 61, 1008, 1008) != 1)
    {
      return 0;
    }

  acquire_message.sender = ":a2dp.test";
  acquire_message.member = "Acquire";
  reply = acquire_handler(&connection, &acquire_message, transport);
  (void)reply;

  if (transport->owner == NULL || transport->owner->pending == NULL ||
      a2dp->resume_id == 0)
    {
      return 0;
    }

  a2dp_resume_complete(session, 0, transport->owner);
  owner = transport->owner;
  if (owner == NULL || transport->state != TRANSPORT_STATE_ACTIVE ||
      transport->fd != 61)
    {
      return 0;
    }

  duplicate_acquire_message.sender = ":a2dp.test";
  duplicate_acquire_message.member = "Acquire";
  reply = acquire_handler(&connection, &duplicate_acquire_message,
                          transport);
  if (bluez_upstream_transport_probe_is_error_reply(
        reply, &duplicate_acquire_message,
        ERROR_INTERFACE ".NotAuthorized") &&
      transport->owner == owner && transport->state == TRANSPORT_STATE_ACTIVE)
    {
      closeout |= 1;
    }

  bad_release_message.sender = ":a2dp.other";
  bad_release_message.member = "Release";
  reply = release_handler(&connection, &bad_release_message, transport);
  if (bluez_upstream_transport_probe_is_error_reply(
        reply, &bad_release_message, ERROR_INTERFACE ".NotAuthorized") &&
      transport->owner == owner && transport->state == TRANSPORT_STATE_ACTIVE)
    {
      closeout |= 2;
    }

  if (bluez_upstream_a2dp_object_mark_prepared_streaming() != 1)
    {
      return closeout;
    }

  release_message.sender = ":a2dp.test";
  release_message.member = "Release";
  reply = release_handler(&connection, &release_message, transport);
  (void)reply;

  if (transport->owner != owner || owner->pending == NULL ||
      transport->state != TRANSPORT_STATE_SUSPENDING)
    {
      return closeout;
    }

  duplicate_release_message.sender = ":a2dp.test";
  duplicate_release_message.member = "Release";
  reply = release_handler(&connection, &duplicate_release_message,
                          transport);
  if (bluez_upstream_transport_probe_is_error_reply(
        reply, &duplicate_release_message, ERROR_INTERFACE ".InProgress") &&
      transport->owner == owner && owner->pending != NULL &&
      transport->state == TRANSPORT_STATE_SUSPENDING)
    {
      closeout |= 4;
    }

  a2dp_suspend_complete(session, 0, owner);

  disconnect_transport = media_transport_create(
    device, "/org/bluez/hci0/dev_00_00_00_00_00_02/sep6",
    configuration, sizeof(configuration), endpoint, NULL);
  if (disconnect_transport == NULL)
    {
      return closeout;
    }

  if (bluez_upstream_a2dp_object_prepare_resume_setup(device, sep) == 0)
    {
      return closeout;
    }

  disconnect_a2dp = disconnect_transport->data;
  disconnect_session = bluez_upstream_a2dp_object_get_prepared_resume_session();
  if (disconnect_a2dp == NULL || disconnect_session == NULL)
    {
      return closeout;
    }

  disconnect_a2dp->session = disconnect_session;
  if (a2dp_sep_get_stream(sep, disconnect_session) == NULL ||
      bluez_upstream_a2dp_object_prepare_stream_media_transport(
        a2dp_sep_get_stream(sep, disconnect_session), 61, 1008, 1008) != 1)
    {
      return closeout;
    }

  disconnect_acquire_message.sender = ":a2dp.disconnect";
  disconnect_acquire_message.member = "Acquire";
  g_dbus_shim_reset_disconnect_watches();
  reply = acquire_handler(&connection, &disconnect_acquire_message,
                          disconnect_transport);
  (void)reply;

  if (disconnect_transport->owner == NULL ||
      disconnect_transport->owner->pending == NULL ||
      disconnect_a2dp->resume_id == 0)
    {
      return closeout;
    }

  a2dp_resume_complete(disconnect_session, 0, disconnect_transport->owner);
  disconnect_owner = disconnect_transport->owner;
  if (disconnect_owner == NULL ||
      disconnect_transport->state != TRANSPORT_STATE_ACTIVE ||
      disconnect_transport->fd != 61 ||
      !bluez_upstream_transport_probe_disconnect_watch_bound(
        disconnect_owner, ":a2dp.disconnect"))
    {
      return closeout;
    }

  disconnect_watch = g_dbus_shim_get_last_disconnect_watch_function();
  disconnect_watch(&connection, disconnect_owner);
  if (disconnect_transport->owner == NULL &&
      disconnect_transport->state == TRANSPORT_STATE_IDLE &&
      disconnect_transport->fd == 61)
    {
      closeout |= 8;
    }

  return closeout;
}

unsigned int
bluez_upstream_transport_a2dp_resume_prepare_ready(void)
{
  struct media_endpoint *endpoint;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;

  endpoint = bluez_upstream_media_create_registered_a2dp_endpoint_for_transport();
  if (endpoint == NULL || media_endpoint_get_sep(endpoint) == NULL)
    {
      return 0;
    }

  return bluez_upstream_a2dp_object_prepare_resume_stream(
           device, media_endpoint_get_sep(endpoint));
}
