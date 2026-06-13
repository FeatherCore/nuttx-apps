/****************************************************************************
 * apps/netutils/iperf/iperf_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <strings.h>
#include <sys/time.h>

#include "argtable3.h"
#include "iperf.h"
#include "netutils/netlib.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_NETUTILS_IPERFTEST_DEVNAME
#  define DEVNAME CONFIG_NETUTILS_IPERFTEST_DEVNAME
#else
#  define DEVNAME "wlan0"
#endif

#define IPERF_DEFAULT_PORT     5001
#define IPERF_DEFAULT_INTERVAL 3
#define IPERF_DEFAULT_TIME     30

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct wifi_iperf_t
{
  FAR struct arg_str *ip;
  FAR struct arg_lit *server;
  FAR struct arg_lit *udp;
  FAR struct arg_lit *ipv6;
  FAR struct arg_str *local;
  FAR struct arg_str *rpmsg;
  FAR struct arg_str *bind;
  FAR struct arg_int *port;
  FAR struct arg_int *interval;
  FAR struct arg_int *time;
  FAR struct arg_lit *abort;
  FAR struct arg_end *end;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: iperf_showusage
 *
 * Description:
 *   Show usage of the demo program and exit
 *
 ****************************************************************************/

static void iperf_showusage(FAR const char *progname,
                            FAR struct wifi_iperf_t *args, int exitcode)
{
  printf("USAGE: %s [-suVa] [-c <ip|cpu>] [-p <port>] [-i <interval>] "
         "[-t <time>] [--local <path>] [--rpmsg <name>]\n", progname);
  printf("iperf command:\n");
  arg_print_glossary(stdout, (FAR void **)args, NULL);

  arg_freetable((FAR void **)args, 1);
  exit(exitcode);
}

/****************************************************************************
 * Name: iperf_printcfg
 *
 * Description:
 *   Print config line before start
 *
 ****************************************************************************/

static void iperf_printcfg(FAR struct iperf_cfg_t *cfg)
{
  printf("\n mode=%s%s-%s ",
         cfg->flag & IPERF_FLAG_LOCAL ? "local-":
           cfg->flag & IPERF_FLAG_RPMSG ? "rpmsg-":"",
         cfg->flag & IPERF_FLAG_TCP ? "tcp":"udp",
         cfg->flag & IPERF_FLAG_SERVER ? "server":"client");

  if (cfg->flag & IPERF_FLAG_LOCAL)
    {
      printf("path=%s, ", cfg->path);
    }
  else if (cfg->flag & IPERF_FLAG_RPMSG)
    {
      printf("cpu=%s, name=%s, ", cfg->host, cfg->path);
    }
  else
    {
      if (cfg->flag & IPERF_FLAG_IPV6)
        {
#ifdef CONFIG_NET_IPv6
          char sip[INET6_ADDRSTRLEN];
          char dip[INET6_ADDRSTRLEN];

          inet_ntop(AF_INET6, &cfg->sip6, sip, sizeof(sip));
          inet_ntop(AF_INET6, &cfg->dip6, dip, sizeof(dip));
          printf("sip=%s:%d,dip=%s:%d, ",
                 sip, cfg->sport, dip, cfg->dport);
#endif
        }
      else
        {
          printf("sip=%" PRId32 ".%" PRId32 ".%" PRId32 ".%" PRId32 ":%d,"
                 "dip=%" PRId32 ".%" PRId32 ".%" PRId32 ".%" PRId32 ":%d, ",
                 cfg->sip & 0xff, (cfg->sip >> 8) & 0xff,
                 (cfg->sip >> 16) & 0xff, (cfg->sip >> 24) & 0xff,
                 cfg->sport, cfg->dip & 0xff, (cfg->dip >> 8) & 0xff,
                 (cfg->dip >> 16) & 0xff, (cfg->dip >> 24) & 0xff,
                 cfg->dport);
        }
    }

  printf("interval=%" PRId32 ", time=%" PRId32 "\n",
         cfg->interval, cfg->time);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  struct wifi_iperf_t iperf_args;
  struct iperf_cfg_t cfg;
  struct in_addr addr;
  int nerrors;

#ifdef CONFIG_NET_IPv4
  char inetaddr[INET_ADDRSTRLEN];
#endif
#ifdef CONFIG_NET_IPv6
  struct in6_addr addr6;
  char inet6addr[INET6_ADDRSTRLEN];
#endif

  bzero(&addr, sizeof(struct in_addr));
  bzero(&cfg, sizeof(cfg));
#ifdef CONFIG_NET_IPv6
  bzero(&addr6, sizeof(addr6));
#endif

  iperf_args.ip = arg_str0("c", "client", "<ip>",
                           "run in client mode, connecting to <host>");
  iperf_args.server = arg_lit0("s", "server", "run in server mode");
  iperf_args.udp = arg_lit0("u", "udp", "use UDP rather than TCP");
  iperf_args.ipv6 = arg_lit0("V", "ipv6", "use IPv6 address family");
  iperf_args.local = arg_str0(NULL, "local", "<path>", "use local socket");
  iperf_args.rpmsg = arg_str0(NULL, "rpmsg", "<name>", "use RPMsg socket");
  iperf_args.bind = arg_str0("B", "bind", "<ip>", "ip to bind");
  iperf_args.port = arg_int0("p", "port", "<port>",
                             "server port to listen on/connect to");
  iperf_args.interval = arg_int0("i", "interval", "<interval>",
                            "seconds between periodic bandwidth reports");
  iperf_args.time = arg_int0("t", "time", "<time>",
                        "time in seconds to transmit for (default 10 secs)");
  iperf_args.abort = arg_lit0("a", "abort", "abort running iperf");
  iperf_args.end = arg_end(1);

  /* Value of local is needed for server, optional for client. */

  iperf_args.local->hdr.flag |= ARG_HASOPTVALUE;

  nerrors = arg_parse(argc, argv, (FAR void**) &iperf_args);
  if (nerrors != 0)
    {
      arg_print_errors(stderr, iperf_args.end, argv[0]);
      iperf_showusage(argv[0], &iperf_args, 0);
    }

  if (iperf_args.abort->count != 0)
    {
      iperf_stop();
      printf("ERROR: abort->count: %d\n", iperf_args.abort->count);
      iperf_showusage(argv[0], &iperf_args, 0);
    }

  if (((iperf_args.ip->count == 0) && (iperf_args.server->count == 0)) ||
         ((iperf_args.ip->count != 0) && (iperf_args.server->count != 0)))
    {
      printf("ERROR: should specific client/server mode\n");
      iperf_showusage(argv[0], &iperf_args, 0);
    }

  if (iperf_args.ip->count == 0)
    {
      cfg.host  = "";
      cfg.flag |= IPERF_FLAG_SERVER;
    }
  else
    {
      cfg.host  = iperf_args.ip->sval[0];
      cfg.flag |= IPERF_FLAG_CLIENT;
    }

  if (iperf_args.ipv6->count > 0)
    {
      cfg.flag |= IPERF_FLAG_IPV6;
    }

  if (iperf_args.local->count > 0)
    {
      cfg.flag |= IPERF_FLAG_LOCAL;
      if (strlen(iperf_args.local->sval[0]) > 0)
        {
          /* iperf -s --local <path> or iperf -c <whatever> --local <path> */

          cfg.path = iperf_args.local->sval[0];
        }
      else if (iperf_args.ip->count > 0)
        {
          /* iperf -c <path> --local */

          cfg.path = iperf_args.ip->sval[0];
        }
      else
        {
          printf("ERROR: should specific local socket path\n");
          iperf_showusage(argv[0], &iperf_args, 0);
        }
    }
  else if (iperf_args.rpmsg->count > 0)
    {
      cfg.flag |= IPERF_FLAG_RPMSG;
      cfg.path  = iperf_args.rpmsg->sval[0];
    }
  else
    {
      if (cfg.flag & IPERF_FLAG_IPV6)
        {
#ifdef CONFIG_NET_IPv6
          if (iperf_args.ip->count > 0 &&
              inet_pton(AF_INET6, iperf_args.ip->sval[0],
                        &cfg.dip6) != 1)
            {
              printf("ERROR: invalid IPv6 destination: %s\n",
                     iperf_args.ip->sval[0]);
              goto out;
            }

          if (iperf_args.bind->count > 0)
            {
              if (inet_pton(AF_INET6, iperf_args.bind->sval[0],
                            &addr6) != 1)
                {
                  printf("ERROR: invalid IPv6 bind address: %s\n",
                         iperf_args.bind->sval[0]);
                  goto out;
                }
            }
          else
            {
              netlib_get_ipv6addr(DEVNAME, &addr6);
              if (IN6_IS_ADDR_UNSPECIFIED(&addr6))
                {
                  printf("ERROR: access IPv6 is ::\n");
                  goto out;
                }
            }

          printf("     IP: %s\n",
                 inet_ntop(AF_INET6, &addr6, inet6addr,
                           sizeof(inet6addr)));
          cfg.sip6 = addr6;
#else
          printf("ERROR: IPv6 Not Enabled\n");
          goto out;
#endif
        }
      else
        {
#ifdef CONFIG_NET_IPv4
          if (iperf_args.ip->count > 0)
            {
              cfg.dip = inet_addr(iperf_args.ip->sval[0]);
              if (cfg.dip == INADDR_NONE)
                {
                  printf("ERROR: invalid IPv4 destination: %s\n",
                         iperf_args.ip->sval[0]);
                  goto out;
                }
            }

          if (iperf_args.bind->count > 0)
            {
              addr.s_addr = inet_addr(iperf_args.bind->sval[0]);
              if (addr.s_addr == INADDR_NONE)
                {
                  printf("ERROR: access IP is 0xffffffff\n");
                  goto out;
                }
            }
          else
            {
              netlib_get_ipv4addr(DEVNAME, &addr);
              if (addr.s_addr == 0)
                {
                  printf("ERROR: access IP is 0x00\n");
                  goto out;
                }
            }

          printf("     IP: %s\n",
                 inet_ntoa_r(addr, inetaddr, sizeof(inetaddr)));

          cfg.sip = addr.s_addr;
#else
          printf("ERROR: IPv4 Not Enabled\n");
          goto out;
#endif
        }
    }

  if (iperf_args.udp->count == 0)
    {
      cfg.flag |= IPERF_FLAG_TCP;
    }
  else
    {
      cfg.flag |= IPERF_FLAG_UDP;
    }

  if (iperf_args.port->count == 0)
    {
      cfg.sport = IPERF_DEFAULT_PORT;
      cfg.dport = IPERF_DEFAULT_PORT;
    }
  else
    {
      if (cfg.flag & IPERF_FLAG_SERVER)
        {
          cfg.sport = iperf_args.port->ival[0];
          cfg.dport = IPERF_DEFAULT_PORT;
        }
      else
        {
          cfg.sport = IPERF_DEFAULT_PORT;
          cfg.dport = iperf_args.port->ival[0];
        }
    }

  if (iperf_args.interval->count == 0)
    {
      cfg.interval = IPERF_DEFAULT_INTERVAL;
    }
  else
    {
      cfg.interval = iperf_args.interval->ival[0];
      if (cfg.interval <= 0)
        {
          cfg.interval = IPERF_DEFAULT_INTERVAL;
        }
    }

  if (iperf_args.time->count == 0)
    {
      if (iperf_args.server->count != 0)
        {
          /* Note: -t is a client-only option for the original iperf 2. */

          cfg.time = 0;
        }
      else
        {
          cfg.time = IPERF_DEFAULT_TIME;
        }
    }
  else
    {
      cfg.time = iperf_args.time->ival[0];
      if (cfg.time != 0 && cfg.time <= cfg.interval)
        {
          cfg.time = cfg.interval;
        }
    }

  iperf_printcfg(&cfg);
  iperf_start(&cfg);

out:
  arg_freetable((FAR void **)&iperf_args, 1);

  return 0;
}
