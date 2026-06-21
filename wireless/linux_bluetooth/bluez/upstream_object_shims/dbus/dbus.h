#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_DBUS_DBUS_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_DBUS_DBUS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

typedef int dbus_bool_t;
typedef int16_t dbus_int16_t;
typedef uint16_t dbus_uint16_t;
typedef uint32_t dbus_uint32_t;
typedef uint64_t dbus_uint64_t;

typedef struct DBusConnection
{
  int dummy;
} DBusConnection;

typedef struct DBusMessage
{
  const char *sender;
  const char *destination;
  const char *path;
  const char *interface;
  const char *member;
  const struct DBusMessage *reply_to;
  const char *error_name;
  const char *error_message;
  uint8_t reply_kind;
  unsigned int ref_count;
  unsigned int ref_total;
  unsigned int unref_total;
  const char *a2dp_endpoint_path;
  const char *a2dp_endpoint_uuid;
  uint8_t a2dp_endpoint_codec;
  const uint8_t *a2dp_endpoint_capabilities;
  int a2dp_endpoint_capabilities_size;
} DBusMessage;

typedef struct DBusPendingCall
{
  DBusMessage *reply;
  unsigned int cancel_count;
  unsigned int unref_count;
} DBusPendingCall;

typedef struct DBusError
{
  const char *name;
  const char *message;
} DBusError;

typedef struct DBusMessageIter
{
  DBusMessage *message;
  uint8_t kind;
  uint8_t index;
  uint8_t property;
  int last_basic_type;
  const char *last_string;
  uint8_t last_byte;
  uint16_t last_uint16;
  const uint8_t *last_fixed_array;
  int last_fixed_array_len;
  unsigned int basic_append_count;
  unsigned int fixed_array_append_count;
} DBusMessageIter;

#define DBUS_TYPE_INVALID 0
#define DBUS_TYPE_STRING  's'
#define DBUS_TYPE_BYTE    'y'
#define DBUS_TYPE_BOOLEAN 'b'
#define DBUS_TYPE_UINT16  'q'
#define DBUS_TYPE_INT32   'i'
#define DBUS_TYPE_UINT32  'u'
#define DBUS_TYPE_INT64   'x'
#define DBUS_TYPE_UINT64  't'
#define DBUS_TYPE_OBJECT_PATH 'o'
#define DBUS_TYPE_UNIX_FD 'h'
#define DBUS_TYPE_ARRAY   'a'
#define DBUS_TYPE_DICT_ENTRY 'e'
#define DBUS_TYPE_VARIANT 'v'

#define DBUS_TYPE_STRING_AS_STRING "s"
#define DBUS_TYPE_BYTE_AS_STRING "y"
#define DBUS_TYPE_BOOLEAN_AS_STRING "b"
#define DBUS_TYPE_UINT16_AS_STRING "q"
#define DBUS_TYPE_INT32_AS_STRING "i"
#define DBUS_TYPE_UINT32_AS_STRING "u"
#define DBUS_TYPE_INT64_AS_STRING "x"
#define DBUS_TYPE_UINT64_AS_STRING "t"
#define DBUS_TYPE_OBJECT_PATH_AS_STRING "o"
#define DBUS_TYPE_UNIX_FD_AS_STRING "h"
#define DBUS_TYPE_ARRAY_AS_STRING "a"
#define DBUS_TYPE_DICT_ENTRY_AS_STRING "e"
#define DBUS_TYPE_VARIANT_AS_STRING "v"
#define DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING "{"
#define DBUS_DICT_ENTRY_END_CHAR_AS_STRING "}"
#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_ERROR_NO_REPLY "org.freedesktop.DBus.Error.NoReply"

#define DBUS_MESSAGE_KIND_METHOD_RETURN 1
#define DBUS_MESSAGE_KIND_ERROR 2

static inline void dbus_error_init(DBusError *error)
{
  if (error != NULL)
    {
      error->name = NULL;
      error->message = NULL;
    }
}

static inline dbus_bool_t dbus_error_is_set(const DBusError *error)
{
  return error != NULL && error->name != NULL;
}

static inline void dbus_error_free(DBusError *error)
{
  if (error != NULL)
    {
      error->name = NULL;
      error->message = NULL;
    }
}

static inline dbus_bool_t dbus_message_iter_init(DBusMessage *message,
                                                 DBusMessageIter *iter)
{
  if (iter != NULL)
    {
      memset(iter, 0, sizeof(*iter));
      iter->message = message;
    }

  return 0;
}

static inline void dbus_message_iter_init_append(DBusMessage *message,
                                                 DBusMessageIter *iter)
{
  (void)message;
  if (iter != NULL)
    {
      memset(iter, 0, sizeof(*iter));
      iter->message = message;
    }
}

static inline int dbus_message_iter_get_arg_type(DBusMessageIter *iter)
{
  if (iter == NULL)
    {
      return DBUS_TYPE_INVALID;
    }

  if (iter->last_basic_type != 0)
    {
      return iter->last_basic_type;
    }

  if (iter->kind == 0)
    {
      return iter->index == 0 ? DBUS_TYPE_OBJECT_PATH :
             iter->index == 1 ? DBUS_TYPE_ARRAY : DBUS_TYPE_INVALID;
    }

  if (iter->kind == 1)
    {
      return iter->index < 3 ? DBUS_TYPE_DICT_ENTRY : DBUS_TYPE_INVALID;
    }

  if (iter->kind == 2)
    {
      return iter->index == 0 ? DBUS_TYPE_STRING :
             iter->index == 1 ? DBUS_TYPE_VARIANT : DBUS_TYPE_INVALID;
    }

  if (iter->kind == 3)
    {
      return iter->property == 0 ? DBUS_TYPE_STRING :
             iter->property == 1 ? DBUS_TYPE_BYTE :
             iter->property == 2 ? DBUS_TYPE_ARRAY : DBUS_TYPE_INVALID;
    }

  if (iter->kind == 4)
    {
      return DBUS_TYPE_BYTE;
    }

  return DBUS_TYPE_INVALID;
}

static inline int dbus_message_iter_get_element_type(DBusMessageIter *iter)
{
  (void)iter;
  return DBUS_TYPE_INVALID;
}

static inline void dbus_message_iter_get_basic(DBusMessageIter *iter,
                                               void *value)
{
  static const char *keys[] =
    {
      "UUID",
      "Codec",
      "Capabilities"
    };

  if (iter == NULL || value == NULL)
    {
      return;
    }

  if (iter->last_basic_type == DBUS_TYPE_UINT16)
    {
      *(uint16_t *)value = iter->last_uint16;
      return;
    }

  if (iter->kind == 0 && iter->index == 0)
    {
      *(const char **)value = iter->message != NULL &&
        iter->message->a2dp_endpoint_path != NULL ?
        iter->message->a2dp_endpoint_path : "/";
    }
  else if (iter->kind == 2 && iter->index == 0 && iter->property < 3)
    {
      *(const char **)value = keys[iter->property];
    }
  else if (iter->kind == 3 && iter->property == 0)
    {
      *(const char **)value = iter->message != NULL &&
        iter->message->a2dp_endpoint_uuid != NULL ?
        iter->message->a2dp_endpoint_uuid : "";
    }
  else if (iter->kind == 3 && iter->property == 1)
    {
      *(uint8_t *)value = iter->message != NULL ?
        iter->message->a2dp_endpoint_codec : 0;
    }
}

static inline dbus_bool_t dbus_message_iter_next(DBusMessageIter *iter)
{
  if (iter == NULL)
    {
      return 0;
    }

  if (iter->kind == 0)
    {
      iter->index++;
      return iter->index < 2;
    }

  if (iter->kind == 1)
    {
      iter->index++;
      return iter->index < 3;
    }

  if (iter->kind == 2)
    {
      iter->index++;
      return iter->index < 2;
    }

  return 0;
}

static inline void dbus_message_iter_recurse(DBusMessageIter *iter,
                                             DBusMessageIter *sub)
{
  if (iter == NULL || sub == NULL)
    {
      return;
    }

  sub->message = iter->message;
  sub->index = 0;
  sub->property = iter->property;

  if (iter->kind == 0 && iter->index == 1)
    {
      sub->kind = 1;
    }
  else if (iter->kind == 1)
    {
      sub->kind = 2;
      sub->property = iter->index;
    }
  else if (iter->kind == 2 && iter->index == 1)
    {
      sub->kind = 3;
    }
  else if (iter->kind == 3 && iter->property == 2)
    {
      sub->kind = 4;
    }
  else
    {
      sub->kind = 0xff;
    }
}

static inline void dbus_message_iter_append_basic(DBusMessageIter *iter,
                                                  int type,
                                                  const void *value)
{
  if (iter == NULL)
    {
      return;
    }

  iter->last_basic_type = type;
  iter->basic_append_count++;

  if ((type == DBUS_TYPE_STRING || type == DBUS_TYPE_OBJECT_PATH) &&
      value != NULL)
    {
      iter->last_string = *(const char * const *)value;
    }
  else if (type == DBUS_TYPE_BYTE && value != NULL)
    {
      iter->last_byte = *(const uint8_t *)value;
    }
  else if (type == DBUS_TYPE_UINT16 && value != NULL)
    {
      iter->last_uint16 = *(const uint16_t *)value;
    }
}

static inline dbus_bool_t dbus_message_iter_append_fixed_array(DBusMessageIter *iter,
                                                        int type,
                                                        const void *value,
                                                        int n_elements)
{
  if (iter != NULL && type == DBUS_TYPE_BYTE && value != NULL)
    {
      iter->last_fixed_array = *(const uint8_t * const *)value;
      iter->last_fixed_array_len = n_elements;
      iter->fixed_array_append_count++;
    }
}

static inline dbus_bool_t dbus_message_iter_open_container(
    DBusMessageIter *iter, int type, const char *contained_signature,
    DBusMessageIter *sub)
{
  (void)iter;
  (void)type;
  (void)contained_signature;
  (void)sub;
  return 1;
}

static inline dbus_bool_t dbus_message_iter_close_container(
    DBusMessageIter *iter, DBusMessageIter *sub)
{
  (void)iter;
  (void)sub;
  return 1;
}

static inline void dbus_message_iter_abandon_container(
    DBusMessageIter *iter, DBusMessageIter *sub)
{
  (void)iter;
  (void)sub;
}

static inline const char *dbus_message_get_sender(DBusMessage *message)
{
  return message != NULL && message->sender != NULL ? message->sender : "";
}

static inline const char *dbus_message_get_member(DBusMessage *message)
{
  return message != NULL && message->member != NULL ? message->member : "";
}

static inline const char *dbus_message_get_destination(DBusMessage *message)
{
  return message != NULL && message->destination != NULL ?
         message->destination : "";
}

static inline const char *dbus_message_get_path(DBusMessage *message)
{
  return message != NULL && message->path != NULL ? message->path : "/";
}

static inline void dbus_message_unref(DBusMessage *message)
{
  if (message != NULL)
    {
      if (message->ref_count > 0)
        {
          message->ref_count--;
        }

      message->unref_total++;
    }
}

static inline DBusMessage *dbus_message_ref(DBusMessage *message)
{
  if (message != NULL)
    {
      message->ref_count++;
      message->ref_total++;
    }

  return message;
}

static inline unsigned int dbus_message_shim_get_ref_count(
    const DBusMessage *message)
{
  return message != NULL ? message->ref_count : 0;
}

static inline unsigned int dbus_message_shim_get_ref_total(
    const DBusMessage *message)
{
  return message != NULL ? message->ref_total : 0;
}

static inline unsigned int dbus_message_shim_get_unref_total(
    const DBusMessage *message)
{
  return message != NULL ? message->unref_total : 0;
}

static inline void dbus_pending_call_unref(DBusPendingCall *call)
{
  if (call != NULL)
    {
      call->unref_count++;
    }
}

static inline void dbus_pending_call_cancel(DBusPendingCall *call)
{
  if (call != NULL)
    {
      call->cancel_count++;
    }
}

static inline DBusMessage *dbus_pending_call_steal_reply(DBusPendingCall *call)
{
  DBusMessage *reply;

  if (call == NULL)
    {
      return NULL;
    }

  reply = call->reply;
  call->reply = NULL;
  return reply;
}

static inline void dbus_message_iter_get_fixed_array(DBusMessageIter *iter,
                                                     void *value,
                                                     int *n_elements)
{
  if (value != NULL)
    {
      *(const uint8_t **)value = iter != NULL && iter->message != NULL ?
        iter->message->a2dp_endpoint_capabilities : NULL;
    }

  if (n_elements != NULL)
    {
      *n_elements = iter != NULL && iter->message != NULL ?
        iter->message->a2dp_endpoint_capabilities_size : 0;
    }
}

static inline DBusMessage *dbus_message_new_method_call(const char *bus_name,
                                                        const char *path,
                                                        const char *interface,
                                                        const char *method)
{
  static DBusMessage messages[4];
  static unsigned int next;
  DBusMessage *message = &messages[next++ % 4];

  memset(message, 0, sizeof(*message));
  message->destination = bus_name;
  message->path = path;
  message->interface = interface;
  message->member = method;
  return message;
}

static inline DBusMessage *dbus_message_new_shim_reply(DBusMessage *message,
                                                       uint8_t kind,
                                                       const char *error_name,
                                                       const char *error_text)
{
  static DBusMessage reply;

  memset(&reply, 0, sizeof(reply));
  reply.reply_to = message;
  reply.reply_kind = kind;
  reply.error_name = error_name;
  reply.error_message = error_text;
  return &reply;
}

static inline DBusMessage *dbus_message_new_method_return(DBusMessage *message)
{
  return dbus_message_new_shim_reply(message,
                                    DBUS_MESSAGE_KIND_METHOD_RETURN,
                                    NULL, NULL);
}

static inline dbus_bool_t dbus_message_append_args(DBusMessage *message,
                                                   int first_arg_type, ...)
{
  (void)message;
  (void)first_arg_type;
  return 1;
}

static inline dbus_bool_t dbus_message_get_args(DBusMessage *message,
                                                DBusError *error,
                                                int first_arg_type, ...)
{
  va_list ap;
  int next_arg_type;

  (void)error;

  if (message == NULL)
    {
      return 0;
    }

  va_start(ap, first_arg_type);

  if (first_arg_type == DBUS_TYPE_OBJECT_PATH)
    {
      const char **path = va_arg(ap, const char **);

      if (path != NULL)
        {
          *path = message->a2dp_endpoint_path != NULL ?
                  message->a2dp_endpoint_path : "/";
        }

      next_arg_type = va_arg(ap, int);
      va_end(ap);
      return next_arg_type == DBUS_TYPE_INVALID ? 1 : 0;
    }

  va_end(ap);
  return first_arg_type == DBUS_TYPE_INVALID ? 1 : 0;
}

static inline dbus_bool_t dbus_message_is_method_call(DBusMessage *message,
                                                      const char *interface,
                                                      const char *method)
{
  return message != NULL && interface != NULL && method != NULL &&
         message->interface != NULL && message->member != NULL &&
         strcmp(message->interface, interface) == 0 &&
         strcmp(message->member, method) == 0;
}

static inline dbus_bool_t dbus_set_error_from_message(DBusError *error,
                                                      DBusMessage *message)
{
  if (error != NULL)
    {
      if (message != NULL &&
          message->reply_kind == DBUS_MESSAGE_KIND_ERROR &&
          message->error_name != NULL)
        {
          error->name = message->error_name;
          error->message = message->error_message;
          return 1;
        }

      error->name = NULL;
      error->message = NULL;
    }

  return 0;
}

static inline dbus_bool_t dbus_error_has_name(const DBusError *error,
                                              const char *name)
{
  return error != NULL && error->name != NULL && name != NULL &&
         strcmp(error->name, name) == 0;
}

static inline dbus_bool_t dbus_pending_call_set_notify(
    DBusPendingCall *call, void (*function)(DBusPendingCall *call,
                                            void *user_data),
    void *user_data, void (*free_user_data)(void *user_data))
{
  (void)call;
  (void)function;
  (void)user_data;
  (void)free_user_data;
  return 1;
}

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_DBUS_DEVICE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_DBUS_DEVICE_COMPAT

typedef int16_t dbus_int16_t;
#define DBUS_TYPE_INT16 'n'
#define DBUS_TYPE_INT16_AS_STRING "n"

static inline DBusMessage *dbus_message_new_error(DBusMessage *message,
                                                  const char *name,
                                                  const char *msg)
{
	(void)message;
	(void)name;
	(void)msg;
	return NULL;
}

static inline void dbus_set_error_const(DBusError *error, const char *name,
                                        const char *message)
{
	if (error != NULL) {
		error->name = name;
		error->message = message;
	}
}

#endif
