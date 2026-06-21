#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_GMAP_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_GMAP_H

#define GMAP_UUID "0000184d-0000-1000-8000-00805f9b34fb"
#define BT_GMAP_ROLE_LIST(_func)
#define BT_GMAP_FEATURE_LIST(_func)

struct bt_gmap;

static inline struct bt_gmap *bt_gmap_find(void *db)
{
  (void)db;
  return NULL;
}

static inline void bt_gmap_set_role(struct bt_gmap *gmap, uint8_t role)
{
  (void)gmap;
  (void)role;
}

static inline void bt_gmap_set_features(struct bt_gmap *gmap,
                                        uint32_t features)
{
  (void)gmap;
  (void)features;
}

#endif
