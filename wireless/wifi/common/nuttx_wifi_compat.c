/****************************************************************************
 * apps/wireless/wifi/common/nuttx_wifi_compat.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Small link-time compatibility helpers for the imported Wi-Fi userspace
 * ports.
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <net/if.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>

#include <nuttx/wireless/ieee80211_linux.h>

void *os_memdup(const void *src, size_t len)
{
  void *dst = malloc(len);

  if (dst != NULL && src != NULL)
    {
      memcpy(dst, src, len);
    }

  return dst;
}

#ifdef bswap_32
#  undef bswap_32
#endif

uint32_t bswap_32(uint32_t value)
{
  return __builtin_bswap32(value);
}

#ifdef if_nametoindex
#  undef if_nametoindex
#endif

#ifdef CONFIG_NETDEV_IFINDEX
extern unsigned int if_nametoindex(FAR const char *ifname);
#endif

unsigned int nuttx_wifi_if_nametoindex(const char *ifname)
{
  unsigned int ifindex;
  int linux_ifindex;

  linux_ifindex = ieee80211_linux_if_nametoindex(ifname);
  if (linux_ifindex > 0)
    {
      return (unsigned int)linux_ifindex;
    }

#ifdef CONFIG_NETDEV_IFINDEX
  ifindex = if_nametoindex(ifname);
  if (ifindex != 0)
    {
      return ifindex;
    }
#else
  if (ifname != NULL && strcmp(ifname, "wlan0") == 0)
    {
      return 1;
    }
#endif

  errno = ENODEV;
  return 0;
}

void *dlsym(void *handle, const char *name)
{
  (void)handle;
  (void)name;
  errno = ENOSYS;
  return NULL;
}

void *dlopen(const char *filename, int flags)
{
  (void)filename;
  (void)flags;
  errno = ENOSYS;
  return NULL;
}

int dlclose(void *handle)
{
  (void)handle;
  errno = ENOSYS;
  return -1;
}

char *dlerror(void)
{
  return "dynamic loading is not supported";
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
  (void)path;
  (void)buf;
  (void)bufsiz;

  errno = ENOENT;
  return -1;
}

int execv(const char *path, char *const argv[])
{
  (void)path;
  (void)argv;
  errno = ENOSYS;
  return -1;
}
