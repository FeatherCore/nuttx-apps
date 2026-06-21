#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_DEVICE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_DEVICE_H

#include "bluetooth/bluetooth.h"
#include "bluetooth/sdp.h"
#include "src/service.h"

struct btd_device;

struct btd_service;

static inline struct btd_device *btd_device_ref(struct btd_device *device)
{
  return device;
}

static inline void btd_device_unref(struct btd_device *device)
{
  (void)device;
}

static inline const bdaddr_t *device_get_address(struct btd_device *device)
{
  static bdaddr_t addr;

  (void)device;
  return &addr;
}

static inline const char *device_get_path(struct btd_device *device)
{
  (void)device;
  return "/org/bluez/hci0/dev_00_00_00_00_00_00";
}

static inline struct btd_adapter *device_get_adapter(struct btd_device *device)
{
  (void)device;
  return bluez_upstream_probe_adapter();
}

static inline struct btd_service *btd_device_get_service(
    struct btd_device *device, const char *uuid)
{
  static struct btd_service service;

  service.device = device;
  service.state = BTD_SERVICE_STATE_DISCONNECTED;
  service.allowed = true;
  (void)uuid;
  return &service;
}

static inline void btd_device_add_uuid(struct btd_device *device,
                                       const char *uuid)
{
  (void)device;
  (void)uuid;
}

static inline const sdp_record_t *btd_device_get_record(
    struct btd_device *device, const char *uuid)
{
  (void)device;
  (void)uuid;
  return NULL;
}

static inline int btd_device_get_volume(struct btd_device *device)
{
  (void)device;
  return -1;
}

static inline void btd_device_set_volume(struct btd_device *device,
                                         int volume)
{
  (void)device;
  (void)volume;
}
#endif
