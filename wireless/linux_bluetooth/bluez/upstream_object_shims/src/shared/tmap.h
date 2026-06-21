#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_TMAP_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_TMAP_H

#define TMAP_ROLE_UUID "00001855-0000-1000-8000-00805f9b34fb"
#define BT_TMAP_ROLE_LIST(_func)

struct bt_tmap;

static inline struct bt_tmap *bt_tmap_find(void *db)
{
  (void)db;
  return NULL;
}

static inline void bt_tmap_set_role(struct bt_tmap *tmap, uint8_t role)
{
  (void)tmap;
  (void)role;
}

#endif
