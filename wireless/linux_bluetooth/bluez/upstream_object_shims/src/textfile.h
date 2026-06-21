#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_TEXTFILE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_TEXTFILE_H

static inline char *textfile_get(const char *pathname, const char *key)
{
  (void)pathname;
  (void)key;
  return NULL;
}

static inline int textfile_put(const char *pathname, const char *key,
                               const char *value)
{
  (void)pathname;
  (void)key;
  (void)value;
  return 0;
}

static inline int textfile_del(const char *pathname, const char *key)
{
  (void)pathname;
  (void)key;
  return 0;
}

#endif
