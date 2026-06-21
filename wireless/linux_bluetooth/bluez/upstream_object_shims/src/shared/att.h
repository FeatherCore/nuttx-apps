#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_ATT_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_ATT_H

struct bt_att;

static inline int bt_att_get_fd(struct bt_att *att)
{
  (void)att;
  return -1;
}

#endif
