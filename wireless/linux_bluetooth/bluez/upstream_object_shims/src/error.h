#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_ERROR_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_ERROR_H

#include "dbus/dbus.h"

#define ERROR_INTERFACE "org.bluez.Error"

static inline DBusMessage *btd_error_new_shim(DBusMessage *msg,
                                              const char *name,
                                              const char *str)
{
  return dbus_message_new_shim_reply(msg, DBUS_MESSAGE_KIND_ERROR, name, str);
}

static inline DBusMessage *btd_error_invalid_args(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".InvalidArguments", NULL);
}

static inline DBusMessage *btd_error_invalid_args_str(DBusMessage *msg,
                                                      const char *str)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".InvalidArguments", str);
}

static inline DBusMessage *btd_error_failed(DBusMessage *msg,
                                            const char *str)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".Failed", str);
}

static inline DBusMessage *btd_error_not_supported(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".NotSupported", NULL);
}

static inline DBusMessage *btd_error_not_authorized(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".NotAuthorized", NULL);
}

static inline DBusMessage *btd_error_not_available(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".NotAvailable", NULL);
}

static inline DBusMessage *btd_error_in_progress(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".InProgress", NULL);
}

static inline DBusMessage *btd_error_already_exists(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".AlreadyExists", NULL);
}

static inline DBusMessage *btd_error_does_not_exist(DBusMessage *msg)
{
  return btd_error_new_shim(msg, ERROR_INTERFACE ".DoesNotExist", NULL);
}

#endif
