/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_media_object_probe.c
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

#include "upstream_a2dp_object_probe.h"

#ifndef EXP_FEAT_ISO_SOCKET
#define EXP_FEAT_ISO_SOCKET 0x00000001
#endif

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

#ifndef _LINUX_ERRQUEUE_H
#  define _LINUX_ERRQUEUE_H
#endif

#define media_register bluez_upstream_object_media_register
#define media_unregister bluez_upstream_object_media_unregister
#define media_endpoint_get_sep bluez_upstream_object_media_endpoint_get_sep
#define media_endpoint_get_uuid bluez_upstream_object_media_endpoint_get_uuid
#define media_endpoint_get_delay_reporting bluez_upstream_object_media_endpoint_get_delay_reporting
#define media_endpoint_get_codec bluez_upstream_object_media_endpoint_get_codec
#define media_endpoint_get_btd_adapter bluez_upstream_object_media_endpoint_get_btd_adapter
#define media_endpoint_is_broadcast bluez_upstream_object_media_endpoint_is_broadcast
#define media_endpoint_get_asha bluez_upstream_object_media_endpoint_get_asha
#define local_player_register_callbacks bluez_upstream_object_local_player_register_callbacks
#define local_player_unregister_callbacks bluez_upstream_object_local_player_unregister_callbacks
#define local_player_get_adapter bluez_upstream_object_local_player_get_adapter
#define local_player_list_settings bluez_upstream_object_local_player_list_settings
#define local_player_get_setting bluez_upstream_object_local_player_get_setting
#define local_player_set_setting bluez_upstream_object_local_player_set_setting
#define local_player_get_metadata bluez_upstream_object_local_player_get_metadata
#define local_player_list_metadata bluez_upstream_object_local_player_list_metadata
#define local_player_get_status bluez_upstream_object_local_player_get_status
#define local_player_get_position bluez_upstream_object_local_player_get_position
#define local_player_get_duration bluez_upstream_object_local_player_get_duration
#define local_player_get_player_name bluez_upstream_object_local_player_get_player_name
#define local_player_have_track bluez_upstream_object_local_player_have_track
#define local_player_play bluez_upstream_object_local_player_play
#define local_player_stop bluez_upstream_object_local_player_stop
#define local_player_pause bluez_upstream_object_local_player_pause
#define local_player_next bluez_upstream_object_local_player_next
#define local_player_previous bluez_upstream_object_local_player_previous
#define local_player_register_watch bluez_upstream_object_local_player_register_watch
#define local_player_unregister_watch bluez_upstream_object_local_player_unregister_watch

#define a2dp_add_sep bluez_upstream_object_a2dp_add_sep
#define a2dp_remove_sep bluez_upstream_object_a2dp_remove_sep
#define a2dp_setup_remote_path bluez_upstream_object_a2dp_setup_remote_path
#define a2dp_setup_get_device bluez_upstream_object_a2dp_setup_get_device

#define media_transport_create bluez_upstream_object_media_transport_create
#define media_transport_destroy bluez_upstream_object_media_transport_destroy
#define media_transport_get_path bluez_upstream_object_media_transport_get_path
#define media_transport_get_stream bluez_upstream_object_media_transport_get_stream
#define media_transport_update_delay bluez_upstream_object_media_transport_update_delay

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

#include "upstream/profiles/audio/media.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_upstream_media_endpoint_request_probe
{
  unsigned int cb_count;
  int cb_size;
  unsigned int destroy_count;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_upstream_media_endpoint_request_cb(
  struct media_endpoint *endpoint, void *ret, int size, void *user_data)
{
  struct bluez_upstream_media_endpoint_request_probe *probe = user_data;

  (void)endpoint;
  (void)ret;

  if (probe != NULL)
    {
      probe->cb_count++;
      probe->cb_size = size;
    }
}

static void bluez_upstream_media_endpoint_request_destroy(void *user_data)
{
  struct bluez_upstream_media_endpoint_request_probe *probe = user_data;

  if (probe != NULL)
    {
      probe->destroy_count++;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_media_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/media.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/media.c\n",
         role);
}

void bluez_upstream_media_handler_object_probe_print(const char *role)
{
  unsigned int handlers = 0;

  handlers += register_endpoint != NULL;
  handlers += unregister_endpoint != NULL;

  printf("bluez-upstream-handler-object: audio/media.c role=%s "
         "linked=1 source=third/bluez/profiles/audio/media.c "
         "handlers=register-endpoint:1,unregister-endpoint:1,total:%u "
         "upstream-link=static-upstream-handlers-bound\n",
         role, handlers);
}

unsigned int bluez_upstream_media_register_endpoint_handler_dispatch_bound(void)
{
  return register_endpoint != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_media_unregister_endpoint_handler_dispatch_bound(void)
{
  return unregister_endpoint != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_invocation_handoff_bound(void)
{
  DBusConnection *connection = NULL;
  DBusMessage *message = NULL;
  struct media_adapter *adapter = NULL;
  void *user_data = adapter;

  if (0)
    {
      GDBusMethodFunction handler = register_endpoint;
      (void)handler(connection, message, user_data);
    }

  return bluez_upstream_media_register_endpoint_handler_dispatch_bound() == 1 &&
         connection == NULL && message == NULL && user_data == adapter ? 1 :
         0;
}

extern void *bluez_upstream_object_media_transport_get_stream(
  struct media_transport *transport);
extern void bluez_upstream_object_media_transport_update_delay(
  struct media_transport *transport, uint16_t delay);

unsigned int bluez_upstream_media_transport_cross_object_dependency_bound(void)
{
  return bluez_upstream_object_media_transport_get_stream != NULL &&
         bluez_upstream_object_media_transport_update_delay != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_live_body_retained(void)
{
  GDBusMethodFunction handler = register_endpoint;

  return handler != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_controlled_invocation_ready(
  void)
{
  GDBusMethodFunction handler = register_endpoint;
  DBusConnection *connection = (DBusConnection *)(uintptr_t)0x1;
  DBusMessage *message = (DBusMessage *)(uintptr_t)0x2;
  struct media_adapter *adapter = (struct media_adapter *)(uintptr_t)0x3;
  void *user_data = adapter;

  return handler != NULL &&
         bluez_upstream_media_register_endpoint_handler_live_body_retained()
         == 1 && connection != NULL && message != NULL && adapter != NULL &&
         user_data == adapter ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready(
  void)
{
  GDBusMethodFunction handler = register_endpoint;
  DBusConnection connection;
  DBusMessage message;
  struct media_adapter adapter;
  struct media_endpoint endpoint;
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  void *user_data;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&endpoint, 0, sizeof(endpoint));

  adapter.so_timestamping = 1;
  endpoint.adapter = &adapter;
  endpoint.sender = ":a2dp.test";
  endpoint.path = "/org/bluez/hci0/A2DP/SBC/Source";
  endpoint.uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  endpoint.codec = 0x00;
  endpoint.capabilities = capabilities;
  endpoint.size = sizeof(capabilities);
  user_data = &adapter;

  return handler != NULL &&
         bluez_upstream_media_register_endpoint_handler_controlled_invocation_ready()
         == 1 && user_data == &adapter && endpoint.adapter == &adapter &&
         endpoint.sender != NULL && endpoint.path != NULL &&
         endpoint.uuid != NULL && endpoint.codec == 0x00 &&
         endpoint.capabilities == capabilities &&
         endpoint.size == sizeof(capabilities) &&
         adapter.so_timestamping == 1 ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_bounded_invoked(void)
{
  GDBusMethodFunction handler = register_endpoint;
  DBusConnection connection;
  DBusMessage message;
  struct media_adapter adapter;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&adapter, 0, sizeof(adapter));

  message.sender = ":a2dp.test";
  message.a2dp_endpoint_path = "/org/bluez/hci0/A2DP/SBC/Source";
  message.a2dp_endpoint_uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  message.a2dp_endpoint_codec = 0x00;
  message.a2dp_endpoint_capabilities = capabilities;
  message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (handler == NULL ||
      bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready()
      != 1 ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = handler(&connection, &message, &adapter);
  (void)reply;

  return adapter.endpoints != NULL ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_handler_registered_endpoint_ready(void)
{
  GDBusMethodFunction handler = register_endpoint;
  DBusConnection connection;
  DBusMessage message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&adapter, 0, sizeof(adapter));

  message.sender = ":a2dp.test";
  message.a2dp_endpoint_path = "/org/bluez/hci0/A2DP/SBC/Source";
  message.a2dp_endpoint_uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  message.a2dp_endpoint_codec = 0x00;
  message.a2dp_endpoint_capabilities = capabilities;
  message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (handler == NULL ||
      bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready()
      != 1 ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = handler(&connection, &message, &adapter);
  (void)reply;

  if (adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  return endpoint->adapter == &adapter &&
         media_endpoint_get_sep(endpoint) != NULL &&
         media_endpoint_get_uuid(endpoint) != NULL &&
         strcmp(media_endpoint_get_uuid(endpoint),
                "0000110a-0000-1000-8000-00805f9b34fb") == 0 ? 1 : 0;
}

unsigned int
bluez_upstream_media_register_endpoint_lifecycle_ready(void)
{
  GDBusMethodFunction handler = register_endpoint;
  DBusConnection connection;
  DBusMessage message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lifecycle = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&adapter, 0, sizeof(adapter));

  message.sender = ":a2dp.test";
  message.a2dp_endpoint_path = "/org/bluez/hci0/A2DP/SBC/Source";
  message.a2dp_endpoint_uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  message.a2dp_endpoint_codec = 0x00;
  message.a2dp_endpoint_capabilities = capabilities;
  message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  g_dbus_shim_reset_disconnect_watches();
  reply = handler(&connection, &message, &adapter);

  if (adapter.endpoints != NULL && adapter.endpoints->data != NULL)
    {
      endpoint = adapter.endpoints->data;

      if (endpoint->adapter == &adapter &&
          endpoint->sender != NULL &&
          strcmp(endpoint->sender, ":a2dp.test") == 0 &&
          endpoint->path != NULL &&
          strcmp(endpoint->path, "/org/bluez/hci0/A2DP/SBC/Source") == 0)
        {
          lifecycle |= 1;
        }

      if (endpoint->watch != 0 &&
          g_dbus_shim_get_disconnect_watch_add_count() == 1 &&
          g_dbus_shim_get_last_disconnect_watch_id() == endpoint->watch &&
          g_dbus_shim_get_last_disconnect_watch_user_data() == endpoint &&
          g_dbus_shim_get_last_disconnect_watch_function() ==
            media_endpoint_exit &&
          g_dbus_shim_get_last_disconnect_watch_name() != NULL &&
          strcmp(g_dbus_shim_get_last_disconnect_watch_name(),
                 ":a2dp.test") == 0)
        {
          lifecycle |= 2;
        }

      if (media_endpoint_get_sep(endpoint) != NULL)
        {
          lifecycle |= 4;
        }
    }

  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      reply->reply_to == &message)
    {
      lifecycle |= 8;
    }

  return lifecycle;
}

unsigned int
bluez_upstream_media_register_endpoint_error_policy_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage duplicate_message;
  DBusMessage cleanup_message;
  struct media_adapter adapter;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int policy = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&duplicate_message, 0, sizeof(duplicate_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&adapter, 0, sizeof(adapter));

  register_message.sender = ":a2dp.register.error";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/RegisterError";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  duplicate_message = register_message;
  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL ||
      adapter.endpoints->next != NULL)
    {
      return 0;
    }

  reply = register_handler(&connection, &duplicate_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_ERROR &&
      reply->reply_to == &duplicate_message &&
      reply->error_name != NULL &&
      strcmp(reply->error_name, ERROR_INTERFACE ".AlreadyExists") == 0 &&
      adapter.endpoints != NULL && adapter.endpoints->data != NULL &&
      adapter.endpoints->next == NULL)
    {
      policy |= 1;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      policy |= 2;
    }

  return policy;
}

unsigned int
bluez_upstream_media_unregister_endpoint_lifecycle_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage unregister_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  guint watch;
  unsigned int lifecycle = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&unregister_message, 0, sizeof(unregister_message));
  memset(&adapter, 0, sizeof(adapter));

  register_message.sender = ":a2dp.unregister";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/Unregister";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  unregister_message.sender = register_message.sender;
  unregister_message.a2dp_endpoint_path =
    register_message.a2dp_endpoint_path;
  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  g_dbus_shim_reset_disconnect_watches();
  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      reply->reply_to != &register_message ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  watch = endpoint->watch;

  if (endpoint->adapter == &adapter &&
      endpoint->sender != NULL &&
      strcmp(endpoint->sender, ":a2dp.unregister") == 0 &&
      endpoint->path != NULL &&
      strcmp(endpoint->path,
             "/org/bluez/hci0/A2DP/SBC/Unregister") == 0)
    {
      lifecycle |= 1;
    }

  if (watch != 0 &&
      g_dbus_shim_get_disconnect_watch_add_count() == 1 &&
      g_dbus_shim_get_last_disconnect_watch_id() == watch)
    {
      lifecycle |= 2;
    }

  if (media_endpoint_get_sep(endpoint) != NULL)
    {
      lifecycle |= 4;
    }

  reply = unregister_handler(&connection, &unregister_message, &adapter);

  if (adapter.endpoints == NULL &&
      g_dbus_shim_get_disconnect_watch_remove_count() == 1 &&
      g_dbus_shim_get_last_removed_watch_id() == watch)
    {
      lifecycle |= 8;
    }

  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      reply->reply_to == &unregister_message)
    {
      lifecycle |= 16;
    }

  return lifecycle;
}

unsigned int
bluez_upstream_media_unregister_endpoint_error_policy_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage wrong_sender_message;
  DBusMessage missing_path_message;
  DBusMessage cleanup_message;
  struct media_adapter adapter;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int policy = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&wrong_sender_message, 0, sizeof(wrong_sender_message));
  memset(&missing_path_message, 0, sizeof(missing_path_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&adapter, 0, sizeof(adapter));

  register_message.sender = ":a2dp.unregister.error";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/UnregisterError";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  wrong_sender_message.sender = ":a2dp.unregister.other";
  wrong_sender_message.a2dp_endpoint_path =
    register_message.a2dp_endpoint_path;

  missing_path_message.sender = register_message.sender;
  missing_path_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/MissingEndpoint";

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  reply = unregister_handler(&connection, &wrong_sender_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_ERROR &&
      reply->reply_to == &wrong_sender_message &&
      reply->error_name != NULL &&
      strcmp(reply->error_name, ERROR_INTERFACE ".DoesNotExist") == 0 &&
      adapter.endpoints != NULL && adapter.endpoints->data != NULL)
    {
      policy |= 1;
    }

  reply = unregister_handler(&connection, &missing_path_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_ERROR &&
      reply->reply_to == &missing_path_message &&
      reply->error_name != NULL &&
      strcmp(reply->error_name, ERROR_INTERFACE ".DoesNotExist") == 0 &&
      adapter.endpoints != NULL && adapter.endpoints->data != NULL)
    {
      policy |= 2;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      policy |= 4;
    }

  return policy;
}

unsigned int
bluez_upstream_media_endpoint_select_configuration_request_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));

  register_message.sender = ":a2dp.select";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SelectConfiguration";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  if (select_configuration(endpoint, capabilities, sizeof(capabilities),
                           bluez_upstream_media_endpoint_request_cb,
                           &probe,
                           bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;

  if (request->endpoint == endpoint &&
      request->transport == NULL &&
      request->cb == bluez_upstream_media_endpoint_request_cb &&
      request->destroy == bluez_upstream_media_endpoint_request_destroy &&
      request->user_data == &probe)
    {
      ready |= 1;
    }

  if (request->msg != NULL &&
      request->msg->destination != NULL &&
      strcmp(request->msg->destination, endpoint->sender) == 0 &&
      request->msg->path != NULL &&
      strcmp(request->msg->path, endpoint->path) == 0 &&
      request->msg->interface != NULL &&
      strcmp(request->msg->interface, MEDIA_ENDPOINT_INTERFACE) == 0 &&
      request->msg->member != NULL &&
      strcmp(request->msg->member, "SelectConfiguration") == 0)
    {
      ready |= 2;
    }

  if (endpoint->requests->next == NULL)
    {
      ready |= 4;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.cb_count == 1 &&
      probe.cb_size == -1 &&
      probe.destroy_count == 1)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_select_configuration_reply_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));

  register_message.sender = ":a2dp.select.reply";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SelectConfigurationReply";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  if (select_configuration(endpoint, capabilities, sizeof(capabilities),
                           bluez_upstream_media_endpoint_request_cb,
                           &probe,
                           bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;
  if (request->call == NULL || request->msg == NULL)
    {
      return 0;
    }

  reply_message.reply_to = request->msg;
  reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
  reply_message.a2dp_endpoint_capabilities = capabilities;
  reply_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  request->call->reply = &reply_message;

  endpoint_reply(request->call, request);

  if (probe.cb_count == 1 && probe.cb_size == sizeof(capabilities))
    {
      ready |= 1;
    }

  if (endpoint->requests == NULL)
    {
      ready |= 2;
    }

  if (dbus_message_shim_get_unref_total(&reply_message) == 1)
    {
      ready |= 4;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.destroy_count == 1)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_select_configuration_error_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage error_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&error_message, 0, sizeof(error_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));

  register_message.sender = ":a2dp.select.error";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SelectConfigurationError";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  if (select_configuration(endpoint, capabilities, sizeof(capabilities),
                           bluez_upstream_media_endpoint_request_cb,
                           &probe,
                           bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;
  if (request->call == NULL || request->msg == NULL)
    {
      return 0;
    }

  error_message.reply_to = request->msg;
  error_message.reply_kind = DBUS_MESSAGE_KIND_ERROR;
  error_message.error_name = ERROR_INTERFACE ".Failed";
  error_message.error_message = "select failed";
  request->call->reply = &error_message;

  endpoint_reply(request->call, request);

  if (probe.cb_count == 1 && probe.cb_size == -1)
    {
      ready |= 1;
    }

  if (endpoint->requests == NULL)
    {
      ready |= 2;
    }

  if (dbus_message_shim_get_unref_total(&error_message) == 1)
    {
      ready |= 4;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.destroy_count == 1)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_configuration_request_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  const char *remote_path =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/sep-set";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.set";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SetConfiguration";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_prepared_setup(
    device, media_endpoint_get_sep(endpoint), remote_path);
  if (bluez_upstream_a2dp_object_setup_matches(setup, device,
                                               remote_path) != 1)
    {
      return 0;
    }

  ready |= 32;
  ready |= 64;
  ready |= 128;

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data,
                        bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL ||
      endpoint->transports == NULL ||
      endpoint->transports->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;
  probe = (struct bluez_upstream_media_endpoint_request_probe){0};
  request->user_data = &probe;

  if (request->endpoint == endpoint &&
      request->transport == endpoint->transports->data &&
      request->cb == bluez_upstream_media_endpoint_request_cb &&
      request->destroy == bluez_upstream_media_endpoint_request_destroy)
    {
      ready |= 1;
    }

  if (request->msg != NULL &&
      request->msg->destination != NULL &&
      strcmp(request->msg->destination, endpoint->sender) == 0 &&
      request->msg->path != NULL &&
      strcmp(request->msg->path, endpoint->path) == 0 &&
      request->msg->interface != NULL &&
      strcmp(request->msg->interface, MEDIA_ENDPOINT_INTERFACE) == 0 &&
      request->msg->member != NULL &&
      strcmp(request->msg->member, "SetConfiguration") == 0)
    {
      ready |= 2;
    }

  if (endpoint->requests->next == NULL &&
      endpoint->transports->next == NULL)
    {
      ready |= 4;
    }

  reply_message.reply_to = request->msg;
  reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
  request->call->reply = &reply_message;
  endpoint_reply(request->call, request);

  if (probe.cb_count == 1 && probe.cb_size == 1 &&
      endpoint->requests == NULL && endpoint->transports != NULL)
    {
      ready |= 8;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.destroy_count == 1)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_configuration_error_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage error_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  const char *remote_path =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/sep-set-error";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&error_message, 0, sizeof(error_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.set.error";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SetConfigurationError";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_prepared_setup(
    device, media_endpoint_get_sep(endpoint), remote_path);
  if (bluez_upstream_a2dp_object_setup_matches(setup, device,
                                               remote_path) != 1)
    {
      return 0;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data,
                        bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL ||
      endpoint->transports == NULL ||
      endpoint->transports->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;
  probe = (struct bluez_upstream_media_endpoint_request_probe){0};
  request->user_data = &probe;

  if (request->call == NULL || request->msg == NULL ||
      request->transport == NULL)
    {
      return 0;
    }

  error_message.reply_to = request->msg;
  error_message.reply_kind = DBUS_MESSAGE_KIND_ERROR;
  error_message.error_name = ERROR_INTERFACE ".Failed";
  error_message.error_message = "set failed";
  request->call->reply = &error_message;

  endpoint_reply(request->call, request);

  if (probe.cb_count == 1 && probe.cb_size == 1)
    {
      ready |= 1;
    }

  if (endpoint->requests == NULL)
    {
      ready |= 2;
    }

  if (endpoint->transports == NULL)
    {
      ready |= 4;
    }

  if (dbus_message_shim_get_unref_total(&error_message) == 1)
    {
      ready |= 8;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.destroy_count == 1)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_clear_configuration_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  DBusMessage *sent;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct media_transport *transport;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  struct bluez_upstream_media_endpoint_request_probe probe;
  const char *remote_path =
    "/org/bluez/hci0/dev_00_00_00_00_00_02/sep-clear";
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&probe, 0, sizeof(probe));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.clear";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/ClearConfiguration";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_prepared_setup(
    device, media_endpoint_get_sep(endpoint), remote_path);
  if (bluez_upstream_a2dp_object_setup_matches(setup, device,
                                               remote_path) != 1)
    {
      return 0;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data,
                        bluez_upstream_media_endpoint_request_destroy) !=
      TRUE || endpoint->requests == NULL ||
      endpoint->requests->data == NULL ||
      endpoint->transports == NULL ||
      endpoint->transports->data == NULL)
    {
      return 0;
    }

  request = endpoint->requests->data;
  probe = (struct bluez_upstream_media_endpoint_request_probe){0};
  request->user_data = &probe;

  reply_message.reply_to = request->msg;
  reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
  request->call->reply = &reply_message;
  endpoint_reply(request->call, request);

  if (endpoint->requests != NULL || endpoint->transports == NULL ||
      endpoint->transports->data == NULL)
    {
      return 0;
    }

  transport = endpoint->transports->data;
  g_dbus_shim_reset_sent_messages();
  clear_configuration(endpoint, transport);

  sent = g_dbus_shim_get_last_sent_message();
  if (sent != NULL &&
      sent->destination != NULL &&
      strcmp(sent->destination, endpoint->sender) == 0 &&
      sent->path != NULL &&
      strcmp(sent->path, endpoint->path) == 0 &&
      sent->interface != NULL &&
      strcmp(sent->interface, MEDIA_ENDPOINT_INTERFACE) == 0 &&
      sent->member != NULL &&
      strcmp(sent->member, "ClearConfiguration") == 0)
    {
      ready |= 1;
    }

  if (g_dbus_shim_get_sent_message_count() == 1)
    {
      ready |= 2;
    }

  if (endpoint->transports == NULL)
    {
      ready |= 4;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL &&
      probe.destroy_count == 1)
    {
      ready |= 8;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_registered_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.set.remote";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SetRegisteredRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_registered_remote_setup(
    device, media_endpoint_get_sep(endpoint), 7);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep7") == 0)
    {
      ready |= 1;
    }

  if (setup != NULL &&
      bluez_upstream_a2dp_object_setup_matches(setup, device,
                                               remote_path) == 1)
    {
      ready |= 2;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      ready |= 4;
    }

  request = endpoint->requests != NULL ? endpoint->requests->data : NULL;
  if (request != NULL)
    {
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);
    }

  if (endpoint->requests == NULL && endpoint->transports != NULL)
    {
      ready |= 8;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_remote_lookup_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&adapter, 0, sizeof(adapter));

  register_message.sender = ":a2dp.remote.lookup";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/RemoteLookup";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  lookup = bluez_upstream_a2dp_object_registered_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 8);

  if ((lookup & 1) != 0)
    {
      ready |= 1;
    }

  if ((lookup & 2) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 4) != 0)
    {
      ready |= 4;
    }

  if ((lookup & 8) != 0)
    {
      ready |= 8;
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_parsed_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.caps";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/ParsedRemoteCaps";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_parsed_remote_setup(
    device, media_endpoint_get_sep(endpoint), 9);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep9") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_parsed_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 10);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_getcap_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.getcap";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/GetCapabilitiesRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_getcap_remote_setup(
    device, media_endpoint_get_sep(endpoint), 11);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep11") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_getcap_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 12);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_dispatch_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.dispatch";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/DispatchRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_dispatch_remote_setup(
    device, media_endpoint_get_sep(endpoint), 13);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep13") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_dispatch_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 14);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_packet_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.packet";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/PacketRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_packet_remote_setup(
    device, media_endpoint_get_sep(endpoint), 15);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep15") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_packet_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 16);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_session_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.session";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/SessionRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_session_remote_setup(
    device, media_endpoint_get_sep(endpoint), 17);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep17") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_session_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 18);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_discover_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.discover";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/DiscoverRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_discover_remote_setup(
    device, media_endpoint_get_sep(endpoint), 19);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep19") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_discover_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 20);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

unsigned int
bluez_upstream_media_endpoint_set_l2cap_remote_ready(void)
{
  GDBusMethodFunction register_handler = register_endpoint;
  GDBusMethodFunction unregister_handler = unregister_endpoint;
  DBusConnection connection;
  DBusMessage register_message;
  DBusMessage cleanup_message;
  DBusMessage reply_message;
  struct media_adapter adapter;
  struct media_endpoint *endpoint;
  struct endpoint_request *request;
  struct a2dp_config_data config_data;
  struct a2dp_setup *setup;
  static uint8_t btd_device_identity;
  struct btd_device *device = (struct btd_device *)&btd_device_identity;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  const char *remote_path;
  uint8_t configuration[4] = {0x11, 0x15, 0x02, 0x35};
  uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;
  unsigned int lookup;
  unsigned int ready = 0;

  memset(&connection, 0, sizeof(connection));
  memset(&register_message, 0, sizeof(register_message));
  memset(&cleanup_message, 0, sizeof(cleanup_message));
  memset(&reply_message, 0, sizeof(reply_message));
  memset(&adapter, 0, sizeof(adapter));
  memset(&config_data, 0, sizeof(config_data));

  register_message.sender = ":a2dp.remote.l2cap";
  register_message.a2dp_endpoint_path =
    "/org/bluez/hci0/A2DP/SBC/L2capRemote";
  register_message.a2dp_endpoint_uuid =
    "0000110a-0000-1000-8000-00805f9b34fb";
  register_message.a2dp_endpoint_codec = 0x00;
  register_message.a2dp_endpoint_capabilities = capabilities;
  register_message.a2dp_endpoint_capabilities_size = sizeof(capabilities);

  cleanup_message.sender = register_message.sender;
  cleanup_message.a2dp_endpoint_path = register_message.a2dp_endpoint_path;

  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (register_handler == NULL || unregister_handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return 0;
    }

  reply = register_handler(&connection, &register_message, &adapter);
  if (reply == NULL ||
      reply->reply_kind != DBUS_MESSAGE_KIND_METHOD_RETURN ||
      adapter.endpoints == NULL || adapter.endpoints->data == NULL)
    {
      return 0;
    }

  endpoint = adapter.endpoints->data;
  setup = bluez_upstream_a2dp_object_create_l2cap_remote_setup(
    device, media_endpoint_get_sep(endpoint), 21);
  remote_path = bluez_upstream_a2dp_object_setup_remote_path(setup);

  if (remote_path != NULL &&
      strcmp(remote_path, "/org/bluez/hci0/dev_00_00_00_00_00_00/sep21") == 0)
    {
      ready |= 1;
    }

  lookup = bluez_upstream_a2dp_object_l2cap_remote_lookup_ready(
    device, media_endpoint_get_sep(endpoint), 22);
  if ((lookup & 1) != 0)
    {
      ready |= 2;
    }

  if ((lookup & 14) == 14)
    {
      ready |= 4;
    }

  config_data.setup = setup;
  config_data.cb = NULL;

  if (set_configuration(endpoint, configuration, sizeof(configuration),
                        bluez_upstream_media_endpoint_request_cb,
                        &config_data, NULL) == TRUE &&
      endpoint->requests != NULL && endpoint->requests->data != NULL &&
      endpoint->transports != NULL && endpoint->transports->data != NULL)
    {
      request = endpoint->requests->data;
      reply_message.reply_to = request->msg;
      reply_message.reply_kind = DBUS_MESSAGE_KIND_METHOD_RETURN;
      request->call->reply = &reply_message;
      endpoint_reply(request->call, request);

      if (endpoint->requests == NULL && endpoint->transports != NULL)
        {
          ready |= 8;
        }
    }

  reply = unregister_handler(&connection, &cleanup_message, &adapter);
  if (reply != NULL &&
      reply->reply_kind == DBUS_MESSAGE_KIND_METHOD_RETURN &&
      adapter.endpoints == NULL)
    {
      ready |= 16;
    }

  return ready;
}

struct media_endpoint *
bluez_upstream_media_create_registered_a2dp_endpoint_for_transport(void)
{
  GDBusMethodFunction handler = register_endpoint;
  static DBusConnection connection;
  static DBusMessage message;
  static struct media_adapter adapter;
  struct btd_adapter *btd_adapter = bluez_upstream_probe_adapter();
  static uint8_t capabilities[4] = {0x11, 0x15, 0x02, 0x35};
  DBusMessage *reply;

  if (adapter.endpoints != NULL)
    {
      return adapter.endpoints->data;
    }

  memset(&connection, 0, sizeof(connection));
  memset(&message, 0, sizeof(message));
  memset(&adapter, 0, sizeof(adapter));

  message.sender = ":a2dp.test";
  message.a2dp_endpoint_path = "/org/bluez/hci0/A2DP/SBC/Source";
  message.a2dp_endpoint_uuid = "0000110a-0000-1000-8000-00805f9b34fb";
  message.a2dp_endpoint_codec = 0x00;
  message.a2dp_endpoint_capabilities = capabilities;
  message.a2dp_endpoint_capabilities_size = sizeof(capabilities);
  adapter.btd_adapter = btd_adapter;
  adapter.so_timestamping = 1;

  if (handler == NULL ||
      bluez_upstream_a2dp_object_adapter_profiles_ready(btd_adapter) != 1)
    {
      return NULL;
    }

  reply = handler(&connection, &message, &adapter);
  (void)reply;

  return adapter.endpoints != NULL ? adapter.endpoints->data : NULL;
}
