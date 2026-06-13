/****************************************************************************
 * apps/wireless/linux_bluetooth/btvhcid_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void btvhcid_usage(void)
{
  printf("usage: btvhcid [--rounds N] [--max-records N] "
         "[--delay-ms N] [--h4-in PATH] [--h4-in-max N] "
         "[--h4-out PATH] "
         "[--quiet] [--no-trace]\n");
  printf("\n");
  printf("defaults: rounds=0(infinite) max-records=32 "
         "h4-in-max=16 delay-ms=20\n");
  printf("\n");
  printf("The daemon pumps the Linux upstream VHCI endpoint and the SIM "
         "hwsim public-file medium in both directions:\n");
  printf("  hwsim medium -> upstream HCI/L2CAP/ISO path\n");
  printf("  upstream VHCI H4 output -> hwsim medium\n");
  printf("  optional external H4 file input -> upstream VHCI\n");
  printf("  optional upstream VHCI H4 output tee -> external H4 file\n");
  printf("\n");
  printf("H4 PATH may be a host file path or tcp:<port>.  tcp:<port> is\n");
  printf("handled by the SIM host hwsim layer as a 127.0.0.1 listener for\n");
  printf("external BlueZ/VHCI-style H4 stream experiments.\n");
}

static unsigned long btvhcid_arg_ulong(const char *arg,
                                       unsigned long fallback)
{
  char *end = NULL;
  unsigned long value;

  if (arg == NULL)
    {
      return fallback;
    }

  value = strtoul(arg, &end, 0);
  if (end == arg)
    {
      return fallback;
    }

  return value;
}

static int btvhcid_pump_h4_file(const char *path,
                                unsigned long max_packets,
                                bool quiet)
{
  uint8_t h4[1025];
  uint32_t h4_len;
  int total = 0;

  if (path == NULL)
    {
      return 0;
    }

  while (max_packets == 0 || total < (int)max_packets)
    {
      int ret;

      ret = linux_bt_hwsim_h4_read(path, h4, sizeof(h4), &h4_len);
      if (ret < 0)
        {
          return ret;
        }

      if (ret == 0)
        {
          break;
        }

      if (h4_len < 1)
        {
          break;
        }

      ret = linux_bt_upstream_vhci_write_default(h4[0], &h4[1],
                                                 h4_len - 1);
      if (ret < 0)
        {
          return ret;
        }

      if (!quiet)
        {
          printf("btvhcid: h4-in packet=%d type=0x%02x len=%u\n",
                 total, h4[0], (unsigned int)h4_len);
        }

      total++;
    }

  return total;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  char trace[4096];
  unsigned long rounds = 0;
  unsigned long max_records = 32;
  unsigned long h4_in_max = 16;
  unsigned long delay_ms = 20;
  unsigned long i = 0;
  const char *h4_in_path = NULL;
  const char *h4_out_path = NULL;
  bool quiet = false;
  bool trace_enabled = true;
  int total_h4_in = 0;
  int total_polled = 0;
  int total_drained = 0;
  int ret;
  int argi;

  for (argi = 1; argi < argc; argi++)
    {
      if (!strcmp(argv[argi], "help") || !strcmp(argv[argi], "-h") ||
          !strcmp(argv[argi], "--help"))
        {
          btvhcid_usage();
          return 0;
        }
      else if (!strcmp(argv[argi], "--rounds") && argi + 1 < argc)
        {
          rounds = btvhcid_arg_ulong(argv[++argi], rounds);
        }
      else if (!strcmp(argv[argi], "--max-records") && argi + 1 < argc)
        {
          max_records = btvhcid_arg_ulong(argv[++argi], max_records);
        }
      else if (!strcmp(argv[argi], "--delay-ms") && argi + 1 < argc)
        {
          delay_ms = btvhcid_arg_ulong(argv[++argi], delay_ms);
        }
      else if (!strcmp(argv[argi], "--h4-in") && argi + 1 < argc)
        {
          h4_in_path = argv[++argi];
        }
      else if (!strcmp(argv[argi], "--h4-in-max") && argi + 1 < argc)
        {
          h4_in_max = btvhcid_arg_ulong(argv[++argi], h4_in_max);
        }
      else if (!strcmp(argv[argi], "--h4-out") && argi + 1 < argc)
        {
          h4_out_path = argv[++argi];
        }
      else if (!strcmp(argv[argi], "--quiet"))
        {
          quiet = true;
        }
      else if (!strcmp(argv[argi], "--no-trace"))
        {
          trace_enabled = false;
        }
      else
        {
          printf("btvhcid: unknown argument: %s\n", argv[argi]);
          btvhcid_usage();
          return 2;
        }
    }

  if (max_records == 0)
    {
      max_records = 32;
    }

  ret = linux_bt_upstream_vhci_open_default();
  if (ret < 0 && ret != -EALREADY)
    {
      printf("btvhcid: open default VHCI failed: %d\n", ret);
      return 1;
    }

  ret = linux_bt_upstream_vhci_bind_hwsim();
  if (ret < 0)
    {
      printf("btvhcid: bind hwsim failed: %d\n", ret);
      return 1;
    }

  if (!quiet)
    {
      printf("btvhcid: started rounds=%lu max-records=%lu delay-ms=%lu "
             "h4-in=%s h4-in-max=%lu h4-out=%s trace=%u\n",
             rounds, max_records, delay_ms,
             h4_in_path != NULL ? h4_in_path : "(none)",
             h4_in_max, h4_out_path != NULL ? h4_out_path : "(none)",
             trace_enabled ? 1 : 0);
    }

  while (rounds == 0 || i < rounds)
    {
      int h4_in;
      int polled;
      int drained;

      h4_in = btvhcid_pump_h4_file(h4_in_path, h4_in_max, quiet);
      if (h4_in < 0)
        {
          printf("btvhcid: h4-in failed round=%lu ret=%d\n", i, h4_in);
          return 1;
        }

      polled = linux_bt_upstream_vhci_poll_medium();
      if (polled < 0)
        {
          printf("btvhcid: poll medium failed round=%lu ret=%d\n",
                 i, polled);
          return 1;
        }

      if (trace_enabled)
        {
          drained =
            linux_bt_upstream_vhci_drain_default_trace_tee(max_records,
                                                           h4_out_path,
                                                           trace,
                                                           sizeof(trace));
        }
      else
        {
          trace[0] = '\0';
          if (h4_out_path != NULL)
            {
              drained =
                linux_bt_upstream_vhci_drain_default_trace_tee(max_records,
                                                               h4_out_path,
                                                               trace,
                                                               sizeof(trace));
              trace[0] = '\0';
            }
          else
            {
              drained = linux_bt_upstream_vhci_drain_default();
            }
        }

      if (drained < 0)
        {
          printf("btvhcid: drain VHCI failed round=%lu ret=%d\n",
                 i, drained);
          return 1;
        }

      ret = btvhcid_pump_h4_file(h4_in_path, h4_in_max, quiet);
      if (ret < 0)
        {
          printf("btvhcid: post-drain h4-in failed round=%lu ret=%d\n",
                 i, ret);
          return 1;
        }

      h4_in += ret;
      total_h4_in += h4_in;
      total_polled += polled;
      total_drained += drained;

      if (!quiet)
        {
          printf("btvhcid: round=%lu h4-in=%d polled=%d drained=%d\n",
                 i, h4_in, polled, drained);
          if (trace_enabled && trace[0] != '\0')
            {
              printf("%s", trace);
            }
        }

      i++;
      if ((rounds == 0 || i < rounds) && delay_ms > 0)
        {
          usleep(delay_ms * 1000);
        }
    }

  if (!quiet)
    {
      printf("btvhcid: stopped rounds=%lu total-h4-in=%d "
             "total-polled=%d total-drained=%d\n",
             i, total_h4_in, total_polled, total_drained);
    }

  return 0;
}
