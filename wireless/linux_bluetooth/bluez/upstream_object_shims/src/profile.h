#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_PROFILE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_PROFILE_H

#include <stdbool.h>
#include "dbus/dbus.h"
#include "glib.h"

#define BTD_PROFILE_PRIORITY_LOW    0
#define BTD_PROFILE_PRIORITY_MEDIUM 1
#define BTD_PROFILE_PRIORITY_HIGH   2

#define BTD_PROFILE_BEARER_ANY   0
#define BTD_PROFILE_BEARER_LE    1
#define BTD_PROFILE_BEARER_BREDR 2

#define BTD_PROFILE_UUID_CB(func_, ...) \
  { \
    .func = (func_), \
    .count = 0, \
    .uuids = NULL, \
  }

struct btd_adapter;
struct btd_device;
struct btd_service;

struct btd_profile_uuid_cb
{
  void (*func)(struct btd_service *service);
  unsigned int count;
  const char **uuids;
};

struct btd_profile
{
  const char *name;
  int priority;
  int bearer;
  const char *local_uuid;
  const char *remote_uuid;
  bool auto_connect;
  bool external;
  bool experimental;
  bool testing;
  struct btd_profile_uuid_cb after_services;
  int (*device_probe)(struct btd_service *service);
  void (*device_remove)(struct btd_service *service);
  int (*connect)(struct btd_service *service);
  int (*disconnect)(struct btd_service *service);
  int (*accept)(struct btd_service *service);
  int (*adapter_probe)(struct btd_profile *p,
                       struct btd_adapter *adapter);
  void (*adapter_remove)(struct btd_profile *p,
                         struct btd_adapter *adapter);
};

typedef bool (*btd_profile_prop_exists)(const char *uuid,
                                        struct btd_device *dev,
                                        void *user_data);
typedef bool (*btd_profile_prop_get)(const char *uuid,
                                     struct btd_device *dev,
                                     DBusMessageIter *iter,
                                     void *user_data);

static inline int btd_profile_register(struct btd_profile *profile)
{
  (void)profile;
  return 0;
}

static inline void btd_profile_unregister(struct btd_profile *profile)
{
  (void)profile;
}

static inline bool btd_profile_add_custom_prop(const char *uuid,
                                               const char *type,
                                               const char *name,
                                               btd_profile_prop_exists exists,
                                               btd_profile_prop_get get,
                                               void *user_data)
{
  (void)uuid;
  (void)type;
  (void)name;
  (void)exists;
  (void)get;
  (void)user_data;
  return true;
}

static inline bool btd_profile_remove_custom_prop(const char *uuid,
                                                  const char *name)
{
  (void)uuid;
  (void)name;
  return true;
}

#endif
