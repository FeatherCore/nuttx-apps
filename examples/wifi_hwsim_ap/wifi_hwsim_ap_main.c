/****************************************************************************
 * apps/examples/wifi_hwsim_ap/wifi_hwsim_ap_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netutils/netlib.h"
#include <nuttx/wireless/wireless.h>

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_IFNAME
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_IFNAME "wlan0"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_IPADDR
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_IPADDR "192.168.201.1"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_NETMASK
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_NETMASK "255.255.255.0"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_SSID
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_SSID "nuttx-hwsim"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA1
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA1 "192.168.201.2"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA2
#  define CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA2 "192.168.201.3"
#endif

static int set_wireless_mode(FAR const char *ifname, int mode)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_ap_demo: socket() failed: %d\n", ret);
      return ret;
    }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, ifname, IFNAMSIZ);
  req.u.mode = mode;

  ret = ioctl(sockfd, SIOCSIWMODE, (unsigned long)((uintptr_t)&req));
  if (ret < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_ap_demo: SIOCSIWMODE(%s) failed: %d\n",
              ifname, ret);
    }

  close(sockfd);
  return ret;
}

static int set_wireless_essid(FAR const char *ifname, FAR const char *ssid)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_ap_demo: socket() failed: %d\n", ret);
      return ret;
    }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, ifname, IFNAMSIZ);
  req.u.essid.pointer = (FAR void *)ssid;
  req.u.essid.length = strlen(ssid);
  req.u.essid.flags = 1;

  ret = ioctl(sockfd, SIOCSIWESSID, (unsigned long)((uintptr_t)&req));
  if (ret < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_ap_demo: SIOCSIWESSID(%s) failed: %d\n",
              ifname, ret);
    }

  close(sockfd);
  return ret;
}

static int set_ipv4(FAR const char *ifname, FAR const char *addr,
                    FAR const char *mask)
{
  struct in_addr ipaddr;
  struct in_addr netmask;
  int ret;

  if (inet_aton(addr, &ipaddr) == 0)
    {
      fprintf(stderr, "wifi_ap_demo: invalid IPv4 address: %s\n", addr);
      return -EINVAL;
    }

  if (inet_aton(mask, &netmask) == 0)
    {
      fprintf(stderr, "wifi_ap_demo: invalid IPv4 netmask: %s\n", mask);
      return -EINVAL;
    }

  ret = netlib_set_ipv4addr(ifname, &ipaddr);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_ap_demo: netlib_set_ipv4addr(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  ret = netlib_set_ipv4netmask(ifname, &netmask);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_ap_demo: netlib_set_ipv4netmask(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  ret = netlib_ifup(ifname);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_ap_demo: netlib_ifup(%s) failed: %d\n",
              ifname, ret);
    }

  return ret;
}

static int set_demo_mac(FAR const char *ifname)
{
  const uint8_t mac[IFHWADDRLEN] =
  {
    0x02, 0x00, 0x00, 0x00, 0x00, 0x01
  };
  int ret;

  ret = netlib_setmacaddr(ifname, mac);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_ap_demo: netlib_setmacaddr(%s) failed: %d\n",
              ifname, ret);
    }

  return ret;
}

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_EXAMPLES_WIFI_HWSIM_AP_IFNAME;
  FAR const char *ipaddr = CONFIG_EXAMPLES_WIFI_HWSIM_AP_IPADDR;
  FAR const char *netmask = CONFIG_EXAMPLES_WIFI_HWSIM_AP_NETMASK;
  FAR const char *ssid = CONFIG_EXAMPLES_WIFI_HWSIM_AP_SSID;
  int ret;

  if (argc > 1)
    {
      ifname = argv[1];
    }

  if (argc > 2)
    {
      ipaddr = argv[2];
    }

  if (argc > 3)
    {
      netmask = argv[3];
    }

  if (argc > 4)
    {
      ssid = argv[4];
    }

  printf("wifi_ap_demo: role=ap if=%s ssid=%s ip=%s mask=%s\n",
         ifname, ssid, ipaddr, netmask);
  printf("wifi_ap_demo: expected sta peers: %s %s\n",
         CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA1,
         CONFIG_EXAMPLES_WIFI_HWSIM_AP_STA2);

  ret = set_demo_mac(ifname);
  if (ret < 0)
    {
      return ret;
    }

  ret = set_wireless_mode(ifname, IW_MODE_MASTER);
  if (ret < 0)
    {
      return ret;
    }

  ret = set_wireless_essid(ifname, ssid);
  if (ret < 0)
    {
      printf("wifi_ap_demo: continuing after AP ESSID warning: %d\n", ret);
    }

  return set_ipv4(ifname, ipaddr, netmask);
}
