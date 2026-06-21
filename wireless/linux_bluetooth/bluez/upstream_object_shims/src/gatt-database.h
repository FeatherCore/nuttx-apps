#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_GATT_DATABASE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_GATT_DATABASE_H

struct btd_gatt_database;
struct gatt_db;

static inline struct btd_gatt_database *btd_adapter_get_database(
    struct btd_adapter *adapter)
{
  (void)adapter;
  return NULL;
}

static inline struct gatt_db *btd_gatt_database_get_db(
    struct btd_gatt_database *database)
{
  (void)database;
  return NULL;
}

#endif
