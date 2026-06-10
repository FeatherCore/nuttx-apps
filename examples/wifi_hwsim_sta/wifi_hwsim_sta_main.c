/****************************************************************************
 * apps/examples/wifi_hwsim_sta/wifi_hwsim_sta_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netutils/netlib.h"
#include <nuttx/wireless/wireless.h>

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_STA_IFNAME
#  define CONFIG_EXAMPLES_WIFI_HWSIM_STA_IFNAME "wlan0"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_STA_IPADDR
#  define CONFIG_EXAMPLES_WIFI_HWSIM_STA_IPADDR "192.168.201.2"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_STA_NETMASK
#  define CONFIG_EXAMPLES_WIFI_HWSIM_STA_NETMASK "255.255.255.0"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_STA_SSID
#  define CONFIG_EXAMPLES_WIFI_HWSIM_STA_SSID "nuttx-hwsim"
#endif

#ifndef CONFIG_EXAMPLES_WIFI_HWSIM_STA_PEER
#  define CONFIG_EXAMPLES_WIFI_HWSIM_STA_PEER "192.168.201.1"
#endif

#ifndef CONFIG_WIFI_SIM_CONFDIR
#  define CONFIG_WIFI_SIM_CONFDIR "/etc/wifi"
#endif

#define WIFI_SIM_ETCDIR "/etc"
#define WIFI_SIM_BSS_FILE CONFIG_WIFI_SIM_CONFDIR "/bss"
#define WIFI_SIM_BSSID "02:00:00:00:00:01"
#define WIFI_SIM_FREQ "2412"
#define WIFI_SIM_RSSI "-30"
#define WIFI_SIM_SECURITY "[ESS]"

static int open_iw_socket(void)
{
  int sockfd;
  int ret;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: socket() failed: %d\n", ret);
      return ret;
    }

  return sockfd;
}

static int set_wireless_mode(FAR const char *ifname, int mode)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = open_iw_socket();
  if (sockfd < 0)
    {
      return sockfd;
    }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, ifname, IFNAMSIZ);
  req.u.mode = mode;

  ret = ioctl(sockfd, SIOCSIWMODE, (unsigned long)((uintptr_t)&req));
  if (ret < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: SIOCSIWMODE(%s) failed: %d\n",
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

  sockfd = open_iw_socket();
  if (sockfd < 0)
    {
      return sockfd;
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
      fprintf(stderr, "wifi_sta_demo: SIOCSIWESSID(%s) failed: %d\n",
              ifname, ret);
    }

  close(sockfd);
  return ret;
}

static int wireless_commit(FAR const char *ifname)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = open_iw_socket();
  if (sockfd < 0)
    {
      return sockfd;
    }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, ifname, IFNAMSIZ);

  ret = ioctl(sockfd, SIOCSIWCOMMIT, (unsigned long)((uintptr_t)&req));
  if (ret < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: SIOCSIWCOMMIT(%s) failed: %d\n",
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
      fprintf(stderr, "wifi_sta_demo: invalid IPv4 address: %s\n", addr);
      return -EINVAL;
    }

  if (inet_aton(mask, &netmask) == 0)
    {
      fprintf(stderr, "wifi_sta_demo: invalid IPv4 netmask: %s\n", mask);
      return -EINVAL;
    }

  ret = netlib_set_ipv4addr(ifname, &ipaddr);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_sta_demo: netlib_set_ipv4addr(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  ret = netlib_set_ipv4netmask(ifname, &netmask);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_sta_demo: netlib_set_ipv4netmask(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  ret = netlib_ifup(ifname);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_sta_demo: netlib_ifup(%s) failed: %d\n",
              ifname, ret);
    }

  return ret;
}

static int set_demo_mac(FAR const char *ifname, FAR const char *addr)
{
  struct in_addr ipaddr;
  uint32_t hostaddr;
  uint8_t mac[IFHWADDRLEN];
  int ret;

  if (inet_aton(addr, &ipaddr) == 0)
    {
      fprintf(stderr, "wifi_sta_demo: invalid IPv4 address: %s\n", addr);
      return -EINVAL;
    }

  hostaddr = ntohl(ipaddr.s_addr);
  mac[0] = 0x02;
  mac[1] = 0x00;
  mac[2] = 0x00;
  mac[3] = 0x00;
  mac[4] = 0x00;
  mac[5] = hostaddr & 0xff;

  ret = netlib_setmacaddr(ifname, mac);
  if (ret < 0)
    {
      fprintf(stderr, "wifi_sta_demo: netlib_setmacaddr(%s) failed: %d\n",
              ifname, ret);
    }

  return ret;
}

static int ensure_directory(FAR const char *path)
{
  int ret;

  ret = mkdir(path, 0777);
  if (ret < 0 && errno != EEXIST)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: mkdir(%s) failed: %d\n", path, ret);
      return ret;
    }

  return OK;
}

static int prepare_wifi_sim_bss(FAR const char *ssid)
{
  char line[128];
  ssize_t written;
  size_t len;
  int fd;
  int ret;

  ret = ensure_directory(WIFI_SIM_ETCDIR);
  if (ret < 0)
    {
      return ret;
    }

  ret = mount(NULL, WIFI_SIM_ETCDIR, "tmpfs", 0, NULL);
  if (ret < 0 && errno != EBUSY)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: mount(tmpfs,%s) failed: %d\n",
              WIFI_SIM_ETCDIR, ret);
      return ret;
    }

  ret = ensure_directory(CONFIG_WIFI_SIM_CONFDIR);
  if (ret < 0)
    {
      return ret;
    }

  snprintf(line, sizeof(line), "%s,%s,%s,%s,%s,\n",
           WIFI_SIM_BSSID, WIFI_SIM_FREQ, WIFI_SIM_RSSI,
           WIFI_SIM_SECURITY, ssid);

  fd = open(WIFI_SIM_BSS_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: open(%s) failed: %d\n",
              WIFI_SIM_BSS_FILE, ret);
      return ret;
    }

  len = strlen(line);
  written = write(fd, line, len);
  if (written < 0)
    {
      ret = -errno;
      fprintf(stderr, "wifi_sta_demo: write(%s) failed: %d\n",
              WIFI_SIM_BSS_FILE, ret);
      close(fd);
      return ret;
    }

  close(fd);

  if ((size_t)written != len)
    {
      fprintf(stderr, "wifi_sta_demo: short write(%s): %zd/%zu\n",
              WIFI_SIM_BSS_FILE, written, len);
      return -EIO;
    }

  printf("wifi_sta_demo: prepared %s\n", WIFI_SIM_BSS_FILE);
  return OK;
}

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_EXAMPLES_WIFI_HWSIM_STA_IFNAME;
  FAR const char *ipaddr = CONFIG_EXAMPLES_WIFI_HWSIM_STA_IPADDR;
  FAR const char *netmask = CONFIG_EXAMPLES_WIFI_HWSIM_STA_NETMASK;
  FAR const char *ssid = CONFIG_EXAMPLES_WIFI_HWSIM_STA_SSID;
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

  printf("wifi_sta_demo: role=sta if=%s ssid=%s ip=%s mask=%s peer=%s\n",
         ifname, ssid, ipaddr, netmask, CONFIG_EXAMPLES_WIFI_HWSIM_STA_PEER);

  ret = set_demo_mac(ifname, ipaddr);
  if (ret < 0)
    {
      return ret;
    }

  ret = prepare_wifi_sim_bss(ssid);
  if (ret < 0)
    {
      return ret;
    }

  ret = set_wireless_mode(ifname, IW_MODE_INFRA);
  if (ret < 0)
    {
      return ret;
    }

  ret = set_wireless_essid(ifname, ssid);
  if (ret < 0)
    {
      return ret;
    }

  ret = wireless_commit(ifname);
  if (ret < 0)
    {
      printf("wifi_sta_demo: continuing after commit warning: %d\n", ret);
    }

  return set_ipv4(ifname, ipaddr, netmask);
}
