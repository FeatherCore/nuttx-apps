#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_BAP_DEBUG_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_BAP_DEBUG_H

static inline bool bt_bap_debug_caps(void *caps, size_t size,
                                     void (*func)(const char *str,
                                                  void *user_data),
                                     void *user_data)
{
  (void)caps;
  (void)size;
  (void)func;
  (void)user_data;
  return true;
}

static inline bool bt_bap_debug_metadata(void *metadata, size_t size,
                                         void (*func)(const char *str,
                                                      void *user_data),
                                         void *user_data)
{
  (void)metadata;
  (void)size;
  (void)func;
  (void)user_data;
  return true;
}

#endif
