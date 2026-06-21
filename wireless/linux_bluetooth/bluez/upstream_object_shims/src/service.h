#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SERVICE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SERVICE_H

#include <stdbool.h>

typedef enum
{
  BTD_SERVICE_STATE_UNAVAILABLE,
  BTD_SERVICE_STATE_DISCONNECTED,
  BTD_SERVICE_STATE_CONNECTING,
  BTD_SERVICE_STATE_CONNECTED,
  BTD_SERVICE_STATE_DISCONNECTING,
} btd_service_state_t;

struct btd_device;
struct btd_profile;

struct btd_service
{
  struct btd_device *device;
  struct btd_profile *profile;
  void *user_data;
  btd_service_state_t state;
  int error;
  bool initiator;
  bool allowed;
};

static inline struct btd_service *btd_service_ref(struct btd_service *service)
{
  return service;
}

static inline void btd_service_unref(struct btd_service *service)
{
  (void)service;
}

static inline struct btd_device *btd_service_get_device(
    const struct btd_service *service)
{
  return service == NULL ? NULL : service->device;
}

static inline struct btd_profile *btd_service_get_profile(
    const struct btd_service *service)
{
  return service == NULL ? NULL : service->profile;
}

static inline btd_service_state_t btd_service_get_state(
    const struct btd_service *service)
{
  return service == NULL ? BTD_SERVICE_STATE_UNAVAILABLE : service->state;
}

static inline int btd_service_get_error(const struct btd_service *service)
{
  return service == NULL ? 0 : service->error;
}

static inline bool btd_service_is_initiator(const struct btd_service *service)
{
  return service != NULL && service->initiator;
}

static inline void btd_service_connecting_complete(struct btd_service *service,
                                                   int err)
{
  if (service != NULL)
    {
      service->error = err;
      service->state = err == 0 ? BTD_SERVICE_STATE_CONNECTED :
                                  BTD_SERVICE_STATE_DISCONNECTED;
    }
}

static inline void btd_service_disconnecting_complete(
    struct btd_service *service, int err)
{
  if (service != NULL)
    {
      service->error = err;
      service->state = BTD_SERVICE_STATE_DISCONNECTED;
    }
}

static inline void btd_service_set_user_data(struct btd_service *service,
                                             void *user_data)
{
  if (service != NULL)
    {
      service->user_data = user_data;
    }
}

static inline void *btd_service_get_user_data(const struct btd_service *service)
{
  return service == NULL ? NULL : service->user_data;
}

#endif
