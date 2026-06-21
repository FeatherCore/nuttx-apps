#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_BTD_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_BTD_H

struct btd_service;

typedef void (*btd_auth_cb_t)(void *derr, void *user_data);

static inline unsigned int btd_request_authorization(const void *src,
                                                     const void *dst,
                                                     const char *uuid,
                                                     btd_auth_cb_t cb,
                                                     void *user_data)
{
  (void)src;
  (void)dst;
  (void)uuid;
  (void)cb;
  (void)user_data;
  return 1;
}

static inline int btd_cancel_authorization(unsigned int id)
{
  (void)id;
  return 0;
}
#endif
