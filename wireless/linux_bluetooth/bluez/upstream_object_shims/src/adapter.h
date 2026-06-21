#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_ADAPTER_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_ADAPTER_H

#include "bluetooth/bluetooth.h"
#include "bluetooth/mgmt.h"

struct btd_adapter;
extern unsigned char bluez_upstream_probe_adapter_identity;

static inline struct btd_adapter *bluez_upstream_probe_adapter(void)
{
  return (struct btd_adapter *)&bluez_upstream_probe_adapter_identity;
}

struct btd_adapter_driver
{
  const char *name;
  int (*probe)(struct btd_adapter *adapter);
  void (*remove)(struct btd_adapter *adapter);
};

static inline const bdaddr_t *btd_adapter_get_address(
    struct btd_adapter *adapter)
{
  static bdaddr_t addr;

  (void)adapter;
  return &addr;
}

static inline struct btd_adapter *btd_adapter_ref(struct btd_adapter *adapter)
{
  return adapter;
}

static inline void btd_adapter_unref(struct btd_adapter *adapter)
{
  (void)adapter;
}

static inline const char *adapter_get_path(struct btd_adapter *adapter)
{
  (void)adapter;
  return "/org/bluez/hci0";
}

static inline struct btd_adapter *adapter_find(const bdaddr_t *addr)
{
  (void)addr;
  return NULL;
}

static inline struct btd_device *btd_adapter_find_device(
    struct btd_adapter *adapter, const bdaddr_t *addr, uint8_t bdaddr_type)
{
  (void)adapter;
  (void)addr;
  (void)bdaddr_type;
  return NULL;
}

static inline struct btd_device *btd_adapter_find_device_by_fd(int fd)
{
  (void)fd;
  return NULL;
}

static inline void btd_register_adapter_driver(
    struct btd_adapter_driver *driver)
{
  (void)driver;
}

static inline void btd_unregister_adapter_driver(
    struct btd_adapter_driver *driver)
{
  (void)driver;
}

static inline uint16_t btd_adapter_get_index(struct btd_adapter *adapter)
{
  (void)adapter;
  return 0;
}

static inline uint32_t btd_adapter_get_supported_settings(
    struct btd_adapter *adapter)
{
  (void)adapter;
  return MGMT_SETTING_BREDR;
}

static inline uint32_t btd_adapter_get_exp_features(
    struct btd_adapter *adapter)
{
  (void)adapter;
  return 0x00000001u;
}

static inline bool btd_adapter_has_settings(struct btd_adapter *adapter,
                                            uint32_t settings)
{
  return (btd_adapter_get_supported_settings(adapter) & settings) == settings;
}

static inline bool btd_adapter_has_exp_feature(struct btd_adapter *adapter,
                                               uint32_t feature)
{
  return (btd_adapter_get_exp_features(adapter) & feature) == feature;
}
#endif
