/****************************************************************************
 * apps/examples/esp32_wifi_demo/esp32_wifi_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <nuttx/wireless/wireless.h>

#include "netutils/netlib.h"

#ifndef CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IFNAME
#  define CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IFNAME "wlan0"
#endif

#ifndef CONFIG_EXAMPLES_ESP32_WIFI_DEMO_SSID
#  define CONFIG_EXAMPLES_ESP32_WIFI_DEMO_SSID "nuttx-esp32"
#endif

#ifndef CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IPADDR
#  define CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IPADDR "192.168.50.20"
#endif

#ifndef CONFIG_EXAMPLES_ESP32_WIFI_DEMO_NETMASK
#  define CONFIG_EXAMPLES_ESP32_WIFI_DEMO_NETMASK "255.255.255.0"
#endif

#ifndef CONFIG_EXAMPLES_ESP32_WIFI_DEMO_GATEWAY
#  define CONFIG_EXAMPLES_ESP32_WIFI_DEMO_GATEWAY "192.168.50.1"
#endif

#define ESP32_WIFI_DEMO_MAX_PASSPHRASE 64

static void usage(FAR const char *progname)
{
  printf("Usage: %s [status|up|connect|ip|all] [ssid] [passphrase]\n",
         progname);
  printf("  status   show %s MAC and IPv4 state\n",
         CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IFNAME);
  printf("  up       bring %s up\n", CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IFNAME);
  printf("  connect  set STA mode, SSID and optional passphrase\n");
  printf("  ip       apply static IPv4 address/netmask/gateway\n");
  printf("  all      run up, connect, ip, status\n");
}

static int iw_socket(void)
{
  int sockfd;
  int ret;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      ret = -errno;
      fprintf(stderr, "esp32_wifi_demo: socket failed: %d\n", ret);
      return ret;
    }

  return sockfd;
}

static int set_wireless_mode(FAR const char *ifname, int mode)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = iw_socket();
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
      fprintf(stderr, "esp32_wifi_demo: SIOCSIWMODE failed: %d\n", ret);
    }

  close(sockfd);
  return ret;
}

static int set_wireless_essid(FAR const char *ifname, FAR const char *ssid)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = iw_socket();
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
      fprintf(stderr, "esp32_wifi_demo: SIOCSIWESSID failed: %d\n", ret);
    }

  close(sockfd);
  return ret;
}

static int set_wireless_passphrase(FAR const char *ifname,
                                   FAR const char *passphrase)
{
  struct iwreq req;
  size_t len;
  int sockfd;
  int ret;

  if (passphrase == NULL)
    {
      passphrase = "";
    }

  len = strlen(passphrase);
  if (len > ESP32_WIFI_DEMO_MAX_PASSPHRASE)
    {
      fprintf(stderr, "esp32_wifi_demo: passphrase too long: %u\n",
              (unsigned int)len);
      return -EINVAL;
    }

  sockfd = iw_socket();
  if (sockfd < 0)
    {
      return sockfd;
    }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, ifname, IFNAMSIZ);
  req.u.encoding.pointer = (FAR void *)passphrase;
  req.u.encoding.length = len;

  ret = ioctl(sockfd, SIOCSIWENCODEEXT, (unsigned long)((uintptr_t)&req));
  if (ret < 0)
    {
      ret = -errno;
      fprintf(stderr, "esp32_wifi_demo: SIOCSIWENCODEEXT failed: %d\n",
              ret);
    }

  close(sockfd);
  return ret;
}

static int wireless_commit(FAR const char *ifname)
{
  struct iwreq req;
  int sockfd;
  int ret;

  sockfd = iw_socket();
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
      fprintf(stderr, "esp32_wifi_demo: SIOCSIWCOMMIT failed: %d\n", ret);
    }

  close(sockfd);
  return ret;
}

static int cmd_up(FAR const char *ifname)
{
  int ret;

  ret = netlib_ifup(ifname);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: netlib_ifup(%s) failed: %d\n",
              ifname, ret);
      return ret;
    }

  printf("esp32_wifi_demo: %s is up\n", ifname);
  return OK;
}

static int parse_ipv4(FAR const char *text, FAR struct in_addr *addr)
{
  if (inet_aton(text, addr) == 0)
    {
      fprintf(stderr, "esp32_wifi_demo: invalid IPv4 address: %s\n", text);
      return -EINVAL;
    }

  return OK;
}

static int cmd_ip(FAR const char *ifname)
{
  struct in_addr addr;
  struct in_addr mask;
  struct in_addr gateway;
  int ret;

  ret = parse_ipv4(CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IPADDR, &addr);
  if (ret < 0)
    {
      return ret;
    }

  ret = parse_ipv4(CONFIG_EXAMPLES_ESP32_WIFI_DEMO_NETMASK, &mask);
  if (ret < 0)
    {
      return ret;
    }

  ret = parse_ipv4(CONFIG_EXAMPLES_ESP32_WIFI_DEMO_GATEWAY, &gateway);
  if (ret < 0)
    {
      return ret;
    }

  ret = netlib_set_ipv4addr(ifname, &addr);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: set IPv4 failed: %d\n", ret);
      return ret;
    }

  ret = netlib_set_ipv4netmask(ifname, &mask);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: set netmask failed: %d\n", ret);
      return ret;
    }

  ret = netlib_set_dripv4addr(ifname, &gateway);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: set gateway failed: %d\n", ret);
      return ret;
    }

  printf("esp32_wifi_demo: %s IPv4=%s netmask=%s gateway=%s\n",
         ifname,
         CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IPADDR,
         CONFIG_EXAMPLES_ESP32_WIFI_DEMO_NETMASK,
         CONFIG_EXAMPLES_ESP32_WIFI_DEMO_GATEWAY);
  return OK;
}

static int cmd_connect(FAR const char *ifname, FAR const char *ssid,
                       FAR const char *passphrase)
{
  int ret;

  ret = set_wireless_mode(ifname, IW_MODE_INFRA);
  if (ret < 0)
    {
      return ret;
    }

  ret = set_wireless_passphrase(ifname, passphrase);
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
      return ret;
    }

  printf("esp32_wifi_demo: requested STA connection to SSID '%s' (%s)\n",
         ssid, passphrase != NULL && passphrase[0] != '\0' ?
         "secured" : "open");
  return OK;
}

static int cmd_status(FAR const char *ifname)
{
  struct in_addr addr;
  uint8_t mac[IFHWADDRLEN];
  int ret;

  ret = netlib_getmacaddr(ifname, mac);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: get MAC failed: %d\n", ret);
    }
  else
    {
      printf("esp32_wifi_demo: %s MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
             ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

  ret = netlib_get_ipv4addr(ifname, &addr);
  if (ret < 0)
    {
      fprintf(stderr, "esp32_wifi_demo: get IPv4 failed: %d\n", ret);
      return ret;
    }

  printf("esp32_wifi_demo: %s IPv4=%s\n", ifname, inet_ntoa(addr));
  return OK;
}

int main(int argc, FAR char *argv[])
{
  FAR const char *ifname = CONFIG_EXAMPLES_ESP32_WIFI_DEMO_IFNAME;
  FAR const char *cmd = argc > 1 ? argv[1] : "all";
  FAR const char *ssid = argc > 2 ? argv[2] :
                         CONFIG_EXAMPLES_ESP32_WIFI_DEMO_SSID;
  FAR const char *passphrase = argc > 3 ? argv[3] : "";
  int ret = OK;

  if (strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0)
    {
      usage(argv[0]);
      return OK;
    }

  if (strcmp(cmd, "status") == 0)
    {
      ret = cmd_status(ifname);
    }
  else if (strcmp(cmd, "up") == 0)
    {
      ret = cmd_up(ifname);
    }
  else if (strcmp(cmd, "connect") == 0)
    {
      ret = cmd_connect(ifname, ssid, passphrase);
    }
  else if (strcmp(cmd, "ip") == 0)
    {
      ret = cmd_ip(ifname);
    }
  else if (strcmp(cmd, "all") == 0)
    {
      ret = cmd_up(ifname);
      if (ret >= 0)
        {
          ret = cmd_connect(ifname, ssid, passphrase);
        }

      if (ret >= 0)
        {
          ret = cmd_ip(ifname);
        }

      if (ret >= 0)
        {
          ret = cmd_status(ifname);
        }
    }
  else
    {
      usage(argv[0]);
      ret = -EINVAL;
    }

  return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
