#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_GDBUS_GDBUS_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_GDBUS_GDBUS_H

#include "dbus/dbus.h"
#include "glib.h"

typedef DBusMessage *(*GDBusMethodFunction)(DBusConnection *connection,
                                            DBusMessage *message,
                                            void *user_data);
typedef gboolean (*GDBusPropertyGetter)(const struct GDBusPropertyTable *property,
                                        DBusMessageIter *iter,
                                        void *user_data);
typedef void (*GDBusPendingPropertySet)(const struct GDBusPropertyTable *property,
                                        DBusMessageIter *iter,
                                        void *user_data);
typedef void (*GDBusDestroyFunction)(void *user_data);
typedef struct GDBusClient GDBusClient;
typedef struct GDBusProxy GDBusProxy;
typedef void (*GDBusWatchFunction)(DBusConnection *connection,
                                   void *user_data);
typedef void (*GDBusSignalFunction)(DBusConnection *connection,
                                    DBusMessage *message,
                                    void *user_data);
typedef void (*GDBusProxyFunction)(GDBusProxy *proxy, void *user_data);
typedef void (*GDBusClientFunction)(GDBusClient *client, void *user_data);

typedef struct GDBusArgInfo
{
  const char *name;
  const char *signature;
} GDBusArgInfo;

typedef struct GDBusMethodTable
{
  const char *name;
  const GDBusArgInfo *in_args;
  const GDBusArgInfo *out_args;
  GDBusMethodFunction function;
  guint flags;
} GDBusMethodTable;

typedef struct GDBusPropertyTable
{
  const char *name;
  const char *type;
  GDBusPropertyGetter get;
  GDBusPendingPropertySet set;
  void *exists;
  guint flags;
} GDBusPropertyTable;

typedef struct GDBusSignalTable
{
  const char *name;
  const GDBusArgInfo *args;
} GDBusSignalTable;

#define G_DBUS_PROPERTY_FLAG_EXPERIMENTAL 0x01
#define G_DBUS_FLAG_ENABLE_EXPERIMENTAL 0x01
#define G_DBUS_PROPERTY_CHANGED_FLAG_FLUSH 0x01

#define GDBUS_METHOD(_name, _in, _out, _func) \
  (_name), (_in), (_out), (_func), 0
#define GDBUS_ASYNC_METHOD(_name, _in, _out, _func) \
  (_name), (_in), (_out), (_func), 0
#define GDBUS_SIGNAL(_name, _args) \
  (_name), (_args)
#define GDBUS_ARGS(...) ((const GDBusArgInfo[]) { __VA_ARGS__, { } })
#define GDBUS_ARG(_name, _signature) { .name = (_name), .signature = (_signature) }
#define GDBUS_PROPERTY(...) __VA_ARGS__

static inline gboolean g_dbus_register_interface(DBusConnection *connection,
                                                 const char *path,
                                                 const char *name,
                                                 const GDBusMethodTable *methods,
                                                 const void *signals,
                                                 const GDBusPropertyTable *properties,
                                                 void *user_data,
                                                 GDBusDestroyFunction destroy)
{
  (void)connection;
  (void)path;
  (void)name;
  (void)methods;
  (void)signals;
  (void)properties;
  (void)user_data;
  (void)destroy;
  return TRUE;
}

static inline gboolean g_dbus_unregister_interface(DBusConnection *connection,
                                                   const char *path,
                                                   const char *name)
{
  (void)connection;
  (void)path;
  (void)name;
  return TRUE;
}

static const char *g_dbus_shim_last_property_path;
static const char *g_dbus_shim_last_property_interface;
static const char *g_dbus_shim_last_property_name;
static unsigned int g_dbus_shim_property_changed_count;
static DBusMessage *g_dbus_shim_last_sent_message;
static unsigned int g_dbus_shim_sent_message_count;
static guint g_dbus_shim_next_watch_id = 1;
static guint g_dbus_shim_last_disconnect_watch_id;
static guint g_dbus_shim_last_removed_watch_id;
static const char *g_dbus_shim_last_disconnect_watch_name;
static GDBusWatchFunction g_dbus_shim_last_disconnect_watch_function;
static void *g_dbus_shim_last_disconnect_watch_user_data;
static unsigned int g_dbus_shim_disconnect_watch_add_count;
static unsigned int g_dbus_shim_disconnect_watch_remove_count;
static const char *g_dbus_shim_pending_property_sender = ":1.feather";
static const char *g_dbus_shim_pending_property_error_name;
static unsigned int g_dbus_shim_pending_property_success_count;
static unsigned int g_dbus_shim_pending_property_error_count;

static inline void g_dbus_emit_property_changed(DBusConnection *connection,
                                                const char *path,
                                                const char *interface,
                                                const char *name)
{
  (void)connection;
  g_dbus_shim_last_property_path = path;
  g_dbus_shim_last_property_interface = interface;
  g_dbus_shim_last_property_name = name;
  g_dbus_shim_property_changed_count++;
}

static inline guint g_dbus_add_disconnect_watch(DBusConnection *connection,
                                                const char *name,
                                                GDBusWatchFunction function,
                                                void *user_data,
                                                GDestroyNotify destroy)
{
  guint id = g_dbus_shim_next_watch_id++;

  (void)connection;
  (void)destroy;
  g_dbus_shim_last_disconnect_watch_id = id;
  g_dbus_shim_last_disconnect_watch_name = name;
  g_dbus_shim_last_disconnect_watch_function = function;
  g_dbus_shim_last_disconnect_watch_user_data = user_data;
  g_dbus_shim_disconnect_watch_add_count++;
  return id;
}

static inline guint g_dbus_add_properties_watch(DBusConnection *connection,
                                                const char *sender,
                                                const char *path,
                                                const char *interface,
                                                GDBusSignalFunction function,
                                                void *user_data,
                                                GDestroyNotify destroy)
{
  (void)connection;
  (void)sender;
  (void)path;
  (void)interface;
  (void)function;
  (void)user_data;
  (void)destroy;
  return 1;
}

static inline guint g_dbus_add_signal_watch(DBusConnection *connection,
                                            const char *sender,
                                            const char *path,
                                            const char *interface,
                                            const char *member,
                                            GDBusSignalFunction function,
                                            void *user_data,
                                            GDestroyNotify destroy)
{
  (void)connection;
  (void)sender;
  (void)path;
  (void)interface;
  (void)member;
  (void)function;
  (void)user_data;
  (void)destroy;
  return 1;
}

static inline void g_dbus_remove_watch(DBusConnection *connection, guint id)
{
  (void)connection;
  g_dbus_shim_last_removed_watch_id = id;
  g_dbus_shim_disconnect_watch_remove_count++;
}

static inline void g_dbus_shim_reset_disconnect_watches(void)
{
  g_dbus_shim_last_disconnect_watch_id = 0;
  g_dbus_shim_last_removed_watch_id = 0;
  g_dbus_shim_last_disconnect_watch_name = NULL;
  g_dbus_shim_last_disconnect_watch_function = NULL;
  g_dbus_shim_last_disconnect_watch_user_data = NULL;
  g_dbus_shim_disconnect_watch_add_count = 0;
  g_dbus_shim_disconnect_watch_remove_count = 0;
}

static inline guint g_dbus_shim_get_last_disconnect_watch_id(void)
{
  return g_dbus_shim_last_disconnect_watch_id;
}

static inline guint g_dbus_shim_get_last_removed_watch_id(void)
{
  return g_dbus_shim_last_removed_watch_id;
}

static inline const char *g_dbus_shim_get_last_disconnect_watch_name(void)
{
  return g_dbus_shim_last_disconnect_watch_name;
}

static inline GDBusWatchFunction
g_dbus_shim_get_last_disconnect_watch_function(void)
{
  return g_dbus_shim_last_disconnect_watch_function;
}

static inline void *g_dbus_shim_get_last_disconnect_watch_user_data(void)
{
  return g_dbus_shim_last_disconnect_watch_user_data;
}

static inline unsigned int g_dbus_shim_get_disconnect_watch_add_count(void)
{
  return g_dbus_shim_disconnect_watch_add_count;
}

static inline unsigned int g_dbus_shim_get_disconnect_watch_remove_count(void)
{
  return g_dbus_shim_disconnect_watch_remove_count;
}

static inline void g_dbus_shim_set_pending_property_sender(
    const char *sender)
{
  g_dbus_shim_pending_property_sender = sender;
}

static inline void g_dbus_shim_reset_pending_properties(void)
{
  g_dbus_shim_pending_property_error_name = NULL;
  g_dbus_shim_pending_property_success_count = 0;
  g_dbus_shim_pending_property_error_count = 0;
}

static inline unsigned int g_dbus_shim_get_pending_property_success_count(void)
{
  return g_dbus_shim_pending_property_success_count;
}

static inline unsigned int g_dbus_shim_get_pending_property_error_count(void)
{
  return g_dbus_shim_pending_property_error_count;
}

static inline const char *g_dbus_shim_get_pending_property_error_name(void)
{
  return g_dbus_shim_pending_property_error_name;
}

static inline void g_dbus_shim_reset_property_changes(void)
{
  g_dbus_shim_last_property_path = NULL;
  g_dbus_shim_last_property_interface = NULL;
  g_dbus_shim_last_property_name = NULL;
  g_dbus_shim_property_changed_count = 0;
}

static inline unsigned int g_dbus_shim_get_property_changed_count(void)
{
  return g_dbus_shim_property_changed_count;
}

static inline const char *g_dbus_shim_get_last_property_path(void)
{
  return g_dbus_shim_last_property_path;
}

static inline const char *g_dbus_shim_get_last_property_interface(void)
{
  return g_dbus_shim_last_property_interface;
}

static inline const char *g_dbus_shim_get_last_property_name(void)
{
  return g_dbus_shim_last_property_name;
}

static inline void g_dbus_shim_reset_sent_messages(void)
{
  g_dbus_shim_last_sent_message = NULL;
  g_dbus_shim_sent_message_count = 0;
}

static inline DBusMessage *g_dbus_shim_get_last_sent_message(void)
{
  return g_dbus_shim_last_sent_message;
}

static inline unsigned int g_dbus_shim_get_sent_message_count(void)
{
  return g_dbus_shim_sent_message_count;
}

static inline gboolean g_dbus_send_message(DBusConnection *connection,
                                           DBusMessage *message)
{
  (void)connection;
  g_dbus_shim_last_sent_message = message;
  g_dbus_shim_sent_message_count++;
  return TRUE;
}

static inline gboolean g_dbus_send_message_with_reply(
    DBusConnection *connection, DBusMessage *message,
    DBusPendingCall **call, int timeout)
{
  static DBusPendingCall pending_call;

  (void)connection;
  (void)message;
  (void)timeout;
  if (call != NULL)
    {
      memset(&pending_call, 0, sizeof(pending_call));
      *call = &pending_call;
    }

  return TRUE;
}

static inline DBusMessage *g_dbus_create_reply(DBusMessage *message,
                                               int first_arg_type, ...)
{
  (void)first_arg_type;
  return dbus_message_new_shim_reply(message,
                                    DBUS_MESSAGE_KIND_METHOD_RETURN,
                                    NULL, NULL);
}

static inline DBusMessage *g_dbus_create_error(DBusMessage *message,
                                               const char *name,
                                               const char *format, ...)
{
  (void)format;
  return dbus_message_new_shim_reply(message, DBUS_MESSAGE_KIND_ERROR,
                                    name, format);
}

static inline gboolean g_dbus_send_error(DBusConnection *connection,
                                         DBusMessage *message,
                                         const char *name,
                                         const char *format, ...)
{
  (void)connection;
  (void)message;
  (void)name;
  (void)format;
  return TRUE;
}

static inline gboolean g_dbus_send_reply(DBusConnection *connection,
                                         DBusMessage *message,
                                         int first_arg_type, ...)
{
  (void)connection;
  (void)message;
  (void)first_arg_type;
  return TRUE;
}

static inline int g_dbus_get_flags(void)
{
  return 0;
}

static inline gboolean g_dbus_get_properties(DBusConnection *connection,
                                             const char *path,
                                             const char *interface,
                                             DBusMessageIter *iter)
{
  (void)connection;
  (void)path;
  (void)interface;
  (void)iter;
  return FALSE;
}

static inline void g_dbus_dict_append_entry(DBusMessageIter *dict,
                                            const char *key, int type,
                                            const void *value)
{
  (void)dict;
  (void)key;
  (void)type;
  (void)value;
}

static inline void g_dbus_dict_append_basic_array(DBusMessageIter *dict,
                                                  int key_type,
                                                  const char **key,
                                                  int value_type,
                                                  const void *array,
                                                  int n_elements)
{
  (void)dict;
  (void)key_type;
  (void)key;
  (void)value_type;
  (void)array;
  (void)n_elements;
}


#ifndef BLUEZ_UPSTREAM_OBJECT_NO_GDBUS_DICT_SHIMS
static inline void dict_append_entry(DBusMessageIter *dict, const char *key,
                                     int type, void *value)
{
  g_dbus_dict_append_entry(dict, key, type, value);
}

static inline void dict_append_array(DBusMessageIter *dict, const char *key,
                                     int type, void *array,
                                     int n_elements)
{
  (void)dict;
  (void)key;
  (void)type;
  (void)array;
  (void)n_elements;
}
#endif

static inline void g_dbus_pending_property_success(
    GDBusPendingPropertySet id)
{
  (void)id;
  g_dbus_shim_pending_property_success_count++;
}

static inline void g_dbus_pending_property_error(
    GDBusPendingPropertySet id, const char *name, const char *format, ...)
{
  (void)id;
  (void)format;
  g_dbus_shim_pending_property_error_name = name;
  g_dbus_shim_pending_property_error_count++;
}

static inline const char *g_dbus_pending_property_get_sender(
    GDBusPendingPropertySet id)
{
  (void)id;
  return g_dbus_shim_pending_property_sender;
}

static inline GDBusClient *g_dbus_client_new_full(DBusConnection *connection,
                                                  const char *service,
                                                  const char *path,
                                                  const char *root_path)
{
  (void)connection;
  (void)service;
  (void)path;
  (void)root_path;
  return NULL;
}

static inline void g_dbus_client_unref(GDBusClient *client)
{
  (void)client;
}

static inline void g_dbus_client_set_disconnect_watch(
    GDBusClient *client, GDBusWatchFunction function, void *user_data)
{
  (void)client;
  (void)function;
  (void)user_data;
}

static inline void g_dbus_client_set_proxy_handlers(
    GDBusClient *client, GDBusProxyFunction added,
    GDBusProxyFunction removed, GDBusProxyFunction property_changed,
    void *user_data)
{
  (void)client;
  (void)added;
  (void)removed;
  (void)property_changed;
  (void)user_data;
}

static inline void g_dbus_client_set_ready_watch(
    GDBusClient *client, GDBusClientFunction ready, void *user_data)
{
  (void)client;
  (void)ready;
  (void)user_data;
}

static inline const char *g_dbus_proxy_get_interface(GDBusProxy *proxy)
{
  (void)proxy;
  return "";
}

static inline const char *g_dbus_proxy_get_path(GDBusProxy *proxy)
{
  (void)proxy;
  return "/";
}

static inline gboolean g_dbus_proxy_get_property(GDBusProxy *proxy,
                                                 const char *name,
                                                 DBusMessageIter *iter)
{
  (void)proxy;
  (void)name;
  (void)iter;
  return FALSE;
}

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_GDBUS_TESTING_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_GDBUS_TESTING_COMPAT
#ifndef G_DBUS_FLAG_ENABLE_TESTING
#define G_DBUS_FLAG_ENABLE_TESTING 0x02
#endif
#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_GDBUS_EXPERIMENTAL_METHOD_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_GDBUS_EXPERIMENTAL_METHOD_COMPAT
#ifndef GDBUS_EXPERIMENTAL_METHOD
#define GDBUS_EXPERIMENTAL_METHOD(_name, _in, _out, _func) \
  (_name), (_in), (_out), (_func), G_DBUS_FLAG_ENABLE_EXPERIMENTAL
#endif
#ifndef GDBUS_EXPERIMENTAL_ASYNC_METHOD
#define GDBUS_EXPERIMENTAL_ASYNC_METHOD(_name, _in, _out, _func) \
  GDBUS_EXPERIMENTAL_METHOD(_name, _in, _out, _func)
#endif
#endif
