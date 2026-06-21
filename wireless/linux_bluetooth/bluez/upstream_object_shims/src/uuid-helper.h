#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_UUID_HELPER_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_UUID_HELPER_H

#include <stdbool.h>
#include <string.h>

static inline bool bt_uuid_strcmp(const char *uuid1, const char *uuid2)
{
  if (uuid1 == NULL || uuid2 == NULL)
    {
      return uuid1 != uuid2;
    }

  return strcasecmp(uuid1, uuid2) != 0;
}

#endif
