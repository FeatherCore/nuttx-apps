/****************************************************************************
 * apps/wireless/wifi/common/include/nuttx_libnl_compat.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Compatibility definitions used while building the imported libnl source
 * against NuttX libc/socket headers.
 *
 ****************************************************************************/

#ifndef __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_LIBNL_COMPAT_H
#define __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_LIBNL_COMPAT_H

#include <nuttx_wifi_port.h>

#include <byteswap.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/endian.h>
#include <sys/socket.h>

unsigned int nuttx_wifi_if_nametoindex(const char *ifname);

#define if_nametoindex nuttx_wifi_if_nametoindex

#ifndef NL_CAPABILITY_VERSION_3_5_0
#  define NL_CAPABILITY_VERSION_3_5_0 1
#endif

/*
 * NuttX links builtin applications and kernel-side networking code into one
 * image.  Keep libnl helpers out of the global namespace used by the NuttX
 * netlink and generic-netlink implementations.
 */

#define genl_register_family   nuttx_libnl_genl_register_family
#define genl_unregister_family nuttx_libnl_genl_unregister_family
#define genlmsg_put            nuttx_libnl_genlmsg_put

#define nla_attr_size    nuttx_libnl_nla_attr_size
#define nla_data         nuttx_libnl_nla_data
#define nla_find         nuttx_libnl_nla_find
#define nla_get_flag     nuttx_libnl_nla_get_flag
#define nla_get_msecs    nuttx_libnl_nla_get_msecs
#define nla_get_string   nuttx_libnl_nla_get_string
#define nla_get_u16      nuttx_libnl_nla_get_u16
#define nla_get_u32      nuttx_libnl_nla_get_u32
#define nla_get_u64      nuttx_libnl_nla_get_u64
#define nla_get_u8       nuttx_libnl_nla_get_u8
#define nla_is_nested    nuttx_libnl_nla_is_nested
#define nla_len          nuttx_libnl_nla_len
#define nla_memcmp       nuttx_libnl_nla_memcmp
#define nla_memcpy       nuttx_libnl_nla_memcpy
#define nla_nest_cancel  nuttx_libnl_nla_nest_cancel
#define nla_nest_end     nuttx_libnl_nla_nest_end
#define nla_nest_start   nuttx_libnl_nla_nest_start
#define nla_next         nuttx_libnl_nla_next
#define nla_ok           nuttx_libnl_nla_ok
#define nla_padlen       nuttx_libnl_nla_padlen
#define nla_parse        nuttx_libnl_nla_parse
#define nla_parse_nested nuttx_libnl_nla_parse_nested
#define nla_put          nuttx_libnl_nla_put
#define nla_put_addr     nuttx_libnl_nla_put_addr
#define nla_put_data     nuttx_libnl_nla_put_data
#define nla_put_flag     nuttx_libnl_nla_put_flag
#define nla_put_msecs    nuttx_libnl_nla_put_msecs
#define nla_put_nested   nuttx_libnl_nla_put_nested
#define nla_put_string   nuttx_libnl_nla_put_string
#define nla_put_u16      nuttx_libnl_nla_put_u16
#define nla_put_u32      nuttx_libnl_nla_put_u32
#define nla_put_u64      nuttx_libnl_nla_put_u64
#define nla_put_u8       nuttx_libnl_nla_put_u8
#define nla_reserve      nuttx_libnl_nla_reserve
#define nla_strcmp       nuttx_libnl_nla_strcmp
#define nla_strdup       nuttx_libnl_nla_strdup
#define nla_strlcpy      nuttx_libnl_nla_strlcpy
#define nla_total_size   nuttx_libnl_nla_total_size
#define nla_type         nuttx_libnl_nla_type
#define nla_validate     nuttx_libnl_nla_validate

#define nlmsg_hdr        nuttx_libnl_nlmsg_hdr

#ifndef SYSCONFDIR
#  define SYSCONFDIR "/etc"
#endif

#ifndef SO_PASSCRED
#  define SO_PASSCRED 64
#endif

#ifndef SO_PRIORITY
#  define SO_PRIORITY 12
#endif

#ifndef SO_ATTACH_FILTER
#  define SO_ATTACH_FILTER 26
#endif

#ifndef SIOCDEVPRIVATE
#  define SIOCDEVPRIVATE 0x89f0
#endif

#ifndef __WORDSIZE
#  define __WORDSIZE (__SIZEOF_POINTER__ * 8)
#endif

#ifndef __FAVOR_BSD
#  define __FAVOR_BSD 0
#endif

#ifndef AF_MAX
#  define AF_MAX 64
#endif

#ifndef AF_AX25
#  define AF_AX25 3
#endif
#ifndef AF_IPX
#  define AF_IPX 4
#endif
#ifndef AF_APPLETALK
#  define AF_APPLETALK 5
#endif
#ifndef AF_NETROM
#  define AF_NETROM 6
#endif
#ifndef AF_BRIDGE
#  define AF_BRIDGE 7
#endif
#ifndef AF_ATMPVC
#  define AF_ATMPVC 8
#endif
#ifndef AF_X25
#  define AF_X25 9
#endif
#ifndef AF_ROSE
#  define AF_ROSE 11
#endif
#ifndef AF_DECnet
#  define AF_DECnet 12
#endif
#ifndef AF_NETBEUI
#  define AF_NETBEUI 13
#endif
#ifndef AF_SECURITY
#  define AF_SECURITY 14
#endif
#ifndef AF_KEY
#  define AF_KEY 15
#endif
#ifndef AF_ASH
#  define AF_ASH 18
#endif
#ifndef AF_ECONET
#  define AF_ECONET 19
#endif
#ifndef AF_ATMSVC
#  define AF_ATMSVC 20
#endif
#ifndef AF_SNA
#  define AF_SNA 22
#endif
#ifndef AF_IRDA
#  define AF_IRDA 23
#endif
#ifndef AF_PPPOX
#  define AF_PPPOX 24
#endif
#ifndef AF_WANPIPE
#  define AF_WANPIPE 25
#endif

#ifndef EAI_ADDRFAMILY
#  define EAI_ADDRFAMILY 1001
#endif

#ifndef EAI_NODATA
#  define EAI_NODATA 1002
#endif

#ifndef CONFIG_LIBC_NETDB
struct addrinfo;
int getaddrinfo(const char *nodename, const char *servname,
                const struct addrinfo *hints, struct addrinfo **res);
int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *node,
                socklen_t nodelen, char *service, socklen_t servicelen,
                int flags);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);
#endif

#ifndef __NUTTX_WIFI_TCP_INFO_DEFINED
#define __NUTTX_WIFI_TCP_INFO_DEFINED
struct tcp_info
{
  uint8_t tcpi_state;
};
#endif

#ifndef __NUTTX_WIFI_STRUCT_IP_DEFINED
#define __NUTTX_WIFI_STRUCT_IP_DEFINED
struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
  unsigned int ip_hl:4;
  unsigned int ip_v:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
  unsigned int ip_v:4;
  unsigned int ip_hl:4;
#else
#  error "unknown endian"
#endif
  uint8_t ip_tos;
  uint16_t ip_len;
  uint16_t ip_id;
  uint16_t ip_off;
  uint8_t ip_ttl;
  uint8_t ip_p;
  uint16_t ip_sum;
  struct in_addr ip_src;
  struct in_addr ip_dst;
};
#endif

#ifndef NLA_S8
#  define NLA_S8 1
struct nlattr;
struct nl_msg;
void *nla_data(const struct nlattr *nla);
int nla_put(struct nl_msg *msg, int attrtype, int attrlen, const void *data);

static inline int8_t nla_get_s8(struct nlattr *nla)
{
  return *(int8_t *)nla_data(nla);
}

static inline int nla_put_s8(struct nl_msg *msg, int attrtype, int8_t value)
{
  return nla_put(msg, attrtype, sizeof(value), &value);
}
#endif

#endif /* __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_LIBNL_COMPAT_H */
