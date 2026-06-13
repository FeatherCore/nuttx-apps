/****************************************************************************
 * apps/wireless/linux_bluetooth/btaudio_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void btaudio_usage(void)
{
  printf("usage: btaudio <command> [args]\\n");
  printf("\\n");
  printf("commands:\\n");
  printf("  a2dp-source start|stop [peer]\\n");
  printf("  a2dp-sink start|stop\\n");
  printf("  le-broadcast-source create|start|stop [big] [bis]\\n");
  printf("  le-broadcast-sink sync|start|stop [big] [bis]\\n");
  printf("  upstream-a2dp-source start\\n");
  printf("  upstream-a2dp-sink start|read|stop [max]\\n");
  printf("  upstream-le-broadcast-source start [big] [bis]\\n");
  printf("  upstream-le-broadcast-sink sync|start|stop [big] [bis] [max]\\n");
}

static int btaudio_todo(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  printf("btaudio: internal command dispatch error\\n");
  return 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--help"))
    {
      btaudio_usage();
      return argc < 2 ? 1 : 0;
    }

  if (!strcmp(argv[1], "a2dp-source") ||
      !strcmp(argv[1], "a2dp-sink") ||
      !strcmp(argv[1], "le-broadcast-source") ||
      !strcmp(argv[1], "le-broadcast-sink") ||
      !strcmp(argv[1], "upstream-a2dp-source") ||
      !strcmp(argv[1], "upstream-a2dp-sink") ||
      !strcmp(argv[1], "upstream-le-broadcast-source") ||
      !strcmp(argv[1], "upstream-le-broadcast-sink"))
    {
      if (argc < 3)
        {
          btaudio_usage();
          return 2;
        }

      if (!strcmp(argv[1], "a2dp-source") && !strcmp(argv[2], "start"))
        {
          int ret;

          if (argc > 3)
            {
              ret = linux_bt_a2dp_source_start_peer((uint16_t)atoi(argv[3]));
            }
          else
            {
              ret = linux_bt_a2dp_source_start();
            }

          if (ret < 0)
            {
              printf("btaudio: a2dp-source failed: %d\n", ret);
              return 1;
            }

          printf("btaudio: a2dp source started\n");
          return 0;
        }

      if (!strcmp(argv[1], "a2dp-sink") && !strcmp(argv[2], "start"))
        {
          char out[1024];
          int ret = linux_bt_a2dp_sink_poll(out, sizeof(out));

          if (ret < 0)
            {
              printf("btaudio: a2dp-sink failed: %d\n", ret);
              return 1;
            }

          printf("btaudio: a2dp sink records=%d\n", ret);
          if (out[0] != '\0')
            {
              printf("%s", out);
            }

          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-source") &&
          !strcmp(argv[2], "create"))
        {
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_source_create(big, bis);

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-source create failed: %d\n",
                     ret);
              return 1;
            }

          printf("btaudio: le broadcast source created big=%u bis=%u\n",
                 big, bis);
          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-source") &&
          !strcmp(argv[2], "start"))
        {
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_source_start_path(big, bis);

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-source failed: %d\n", ret);
              return 1;
            }

          printf("btaudio: le broadcast source started\n");
          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-sink") &&
          !strcmp(argv[2], "sync"))
        {
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_sink_sync(big, bis);

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-sink sync failed: %d\n",
                     ret);
              return 1;
            }

          printf("btaudio: le broadcast sink sync requested big=%u "
                 "bis=%u\n", big, bis);
          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-source") &&
          !strcmp(argv[2], "stop"))
        {
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_source_stop(big, bis);

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-source stop failed: %d\n",
                     ret);
              return 1;
            }

          printf("btaudio: le broadcast source stopped big=%u bis=%u\n",
                 big, bis);
          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-sink") &&
          !strcmp(argv[2], "stop"))
        {
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_sink_stop(big, bis);

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-sink stop failed: %d\n",
                     ret);
              return 1;
            }

          printf("btaudio: le broadcast sink stopped big=%u bis=%u\n",
                 big, bis);
          return 0;
        }

      if (!strcmp(argv[1], "le-broadcast-sink") &&
          !strcmp(argv[2], "start"))
        {
          char out[1024];
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          int ret = linux_bt_le_broadcast_sink_poll_path(big, bis,
                                                         out, sizeof(out));

          if (ret < 0)
            {
              printf("btaudio: le-broadcast-sink failed: %d\n", ret);
              return 1;
            }

          printf("btaudio: le broadcast sink records=%d\n", ret);
          if (out[0] != '\0')
            {
              printf("%s", out);
            }

          return 0;
        }

      if (!strcmp(argv[1], "upstream-a2dp-source") &&
          !strcmp(argv[2], "start"))
        {
          static const char media[] = "A2DP:SBC:synthetic-frame";
          char out[512] = "";
          uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 0;
          uint16_t handle = peer != 0 ? linux_bt_conn_handle(peer) : 0x0040;
          int ret = linux_bt_upstream_l2cap_socket_send_probe(0x0019,
                                                              0x0041,
                                                              handle,
                                                              media,
                                                              sizeof(media) -
                                                              1,
                                                              out,
                                                              sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream a2dp-source failed: %d\n", ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream a2dp source queued l2cap socket "
                 "sample peer=%u handle=0x%04x\n", peer, handle);
          return 0;
        }

      if (!strcmp(argv[1], "upstream-a2dp-sink") &&
          !strcmp(argv[2], "start"))
        {
          char out[512] = "";
          uint16_t peer = argc > 3 ? (uint16_t)atoi(argv[3]) : 0;
          uint16_t handle = peer != 0 ? linux_bt_conn_handle(peer) : 0;
          int ret = linux_bt_upstream_l2cap_socket_bind_probe(0x0019,
                                                              0x0041,
                                                              handle,
                                                              out,
                                                              sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream a2dp-sink bind failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          ret = linux_bt_upstream_l2cap_socket_listen_probe(1,
                                                            out,
                                                            sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream a2dp-sink listen failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream a2dp sink listening on l2cap socket\n");
          return 0;
        }

      if (!strcmp(argv[1], "upstream-a2dp-sink") &&
          !strcmp(argv[2], "read"))
        {
          char out[1024] = "";
          size_t max_len = argc > 3 ? (size_t)strtoul(argv[3], NULL, 0) :
                           512;
          int polled = 0;
          int attempt;
          int ret = -EAGAIN;

          for (attempt = 0; attempt < 20; attempt++)
            {
              ret = linux_bt_upstream_vhci_poll_medium();
              if (ret < 0)
                {
                  printf("btaudio: upstream a2dp-sink poll failed: %d\n",
                         ret);
                  return 1;
                }

              polled += ret;
              ret = linux_bt_upstream_l2cap_socket_recv_probe(max_len,
                                                              out,
                                                              sizeof(out));
              if (ret >= 0)
                {
                  printf("btaudio: upstream a2dp sink polled=%d\n",
                         polled);
                  printf("%s", out);
                  return 0;
                }

              if (ret != -EAGAIN)
                {
                  printf("btaudio: upstream a2dp sink polled=%d\n",
                         polled);
                  printf("%s", out);
                  printf("btaudio: upstream a2dp-sink recv failed: %d\n",
                         ret);
                  return 1;
                }

              usleep(50000);
            }

          printf("btaudio: upstream a2dp sink polled=%d\n", polled);
          printf("%s", out);
          printf("btaudio: upstream a2dp-sink recv failed: %d\n", ret);
          return 1;
        }

      if (!strcmp(argv[1], "upstream-a2dp-sink") &&
          !strcmp(argv[2], "stop"))
        {
          char out[512] = "";
          int ret = linux_bt_upstream_l2cap_socket_close_probe(out,
                                                               sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream a2dp-sink close failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream a2dp sink stopped\n");
          return 0;
        }

      if (!strcmp(argv[1], "upstream-le-broadcast-source") &&
          !strcmp(argv[2], "start"))
        {
          static const char media[] = "LE-AUDIO:LC3:synthetic-frame";
          char out[512] = "";
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          uint16_t handle = (uint16_t)(0x0100 | ((big & 0x0f) << 4) |
                                      (bis & 0x0f));
          int ret = linux_bt_upstream_iso_socket_send_probe(0,
                                                            handle,
                                                            media,
                                                            sizeof(media) -
                                                            1,
                                                            out,
                                                            sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream le-broadcast-source failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream le broadcast source queued iso socket "
                 "sample big=%u bis=%u handle=0x%04x\n", big, bis,
                 handle);
          return 0;
        }

      if (!strcmp(argv[1], "upstream-le-broadcast-sink") &&
          !strcmp(argv[2], "sync"))
        {
          char out[512] = "";
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          uint16_t handle = (uint16_t)(0x0100 | ((big & 0x0f) << 4) |
                                      (bis & 0x0f));
          int ret = linux_bt_upstream_iso_socket_bind_probe(0,
                                                            handle,
                                                            out,
                                                            sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream le-broadcast-sink bind failed: %d\n",
                     ret);
              return 1;
            }

          printf("%s", out);
          ret = linux_bt_upstream_iso_socket_connect_probe(0,
                                                           out,
                                                           sizeof(out));
          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream le-broadcast-sink connect failed: "
                     "%d\n", ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream le broadcast sink synced big=%u bis=%u "
                 "handle=0x%04x\n", big, bis, handle);
          return 0;
        }

      if (!strcmp(argv[1], "upstream-le-broadcast-sink") &&
          !strcmp(argv[2], "start"))
        {
          char out[1024] = "";
          uint8_t big = argc > 3 ? (uint8_t)atoi(argv[3]) : 0;
          uint8_t bis = argc > 4 ? (uint8_t)atoi(argv[4]) : 1;
          size_t max_len = argc > 5 ? (size_t)strtoul(argv[5], NULL, 0) :
                           512;
          int polled = 0;
          int attempt;
          int ret = -EAGAIN;

          (void)big;
          (void)bis;

          for (attempt = 0; attempt < 20; attempt++)
            {
              ret = linux_bt_upstream_vhci_poll_medium();
              if (ret < 0)
                {
                  printf("btaudio: upstream le-broadcast-sink poll failed: "
                         "%d\n", ret);
                  return 1;
                }

              polled += ret;
              ret = linux_bt_upstream_iso_socket_recv_probe(max_len,
                                                            out,
                                                            sizeof(out));
              if (ret >= 0)
                {
                  printf("btaudio: upstream le broadcast sink polled=%d\n",
                         polled);
                  printf("%s", out);
                  return 0;
                }

              if (ret != -EAGAIN)
                {
                  printf("btaudio: upstream le broadcast sink polled=%d\n",
                         polled);
                  printf("%s", out);
                  printf("btaudio: upstream le-broadcast-sink recv failed: "
                         "%d\n", ret);
                  return 1;
                }

              usleep(50000);
            }

          printf("btaudio: upstream le broadcast sink polled=%d\n",
                 polled);
          printf("%s", out);
          printf("btaudio: upstream le-broadcast-sink recv failed: "
                 "%d\n", ret);
          return 1;
        }

      if (!strcmp(argv[1], "upstream-le-broadcast-sink") &&
          !strcmp(argv[2], "stop"))
        {
          char out[512] = "";
          int ret = linux_bt_upstream_iso_socket_close_probe(out,
                                                             sizeof(out));

          if (ret < 0)
            {
              printf("%s", out);
              printf("btaudio: upstream le-broadcast-sink close failed: "
                     "%d\n", ret);
              return 1;
            }

          printf("%s", out);
          printf("btaudio: upstream le broadcast sink stopped\n");
          return 0;
        }

      return btaudio_todo(argc - 1, &argv[1]);
    }

  printf("btaudio: unknown command: %s\\n", argv[1]);
  btaudio_usage();
  return 2;
}
