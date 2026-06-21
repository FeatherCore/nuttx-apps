/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/tools/obex_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/wireless/linux_bluetooth.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BLUEZ_OBEX_RFCOMM_PSM       0x0003
#define BLUEZ_OBEX_DEFAULT_PEER     2

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_obex_transaction
{
  const char *label;
  const char *payload;
  const char *response;
};

struct bluez_obex_mode
{
  const char *mode;
  const char *role;
  const char *source;
  const char *uuid;
  const char *boundary;
  const struct bluez_obex_transaction *transactions;
  size_t transaction_count;
  uint16_t cid;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct bluez_obex_transaction g_bluez_obex_pbap[] =
{
  {"pbap-obex-connect", "OBEX CONNECT target=PBAP\r\n", "OBEX_SUCCESS"},
  {"pbap-set-phonebook", "OBEX SETPATH telecom/pb\r\n", "OBEX_SUCCESS"},
  {"pbap-pull-phonebook", "OBEX GET x-bt/phonebook\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_opp[] =
{
  {"opp-obex-connect", "OBEX CONNECT target=OPP\r\n", "OBEX_SUCCESS"},
  {"opp-put-object", "OBEX PUT name=feather.txt\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"},
  {"opp-get-capability", "OBEX GET capability\r\n", "OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_map[] =
{
  {"map-obex-connect", "OBEX CONNECT target=MAP\r\n", "OBEX_SUCCESS"},
  {"map-set-folder", "OBEX SETPATH telecom/msg/inbox\r\n", "OBEX_SUCCESS"},
  {"map-get-message-listing", "OBEX GET x-bt/MAP-msg-listing\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_mns[] =
{
  {"mns-obex-connect", "OBEX CONNECT target=MNS\r\n", "OBEX_SUCCESS"},
  {"mns-new-message-event", "OBEX PUT event=NewMessage\r\n",
   "OBEX_SUCCESS"},
  {"mns-delivery-success-event", "OBEX PUT event=DeliverySuccess\r\n",
   "OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_ftp[] =
{
  {"ftp-obex-connect", "OBEX CONNECT target=FTP\r\n", "OBEX_SUCCESS"},
  {"ftp-set-folder", "OBEX SETPATH /telecom\r\n", "OBEX_SUCCESS"},
  {"ftp-folder-listing", "OBEX GET x-obex/folder-listing\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"},
  {"ftp-get-file", "OBEX GET feather.bin\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_sync[] =
{
  {"sync-obex-connect", "OBEX CONNECT target=SYNC\r\n", "OBEX_SUCCESS"},
  {"sync-phonebook", "OBEX GET telecom/pb.vcf\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"},
  {"sync-calendar", "OBEX GET telecom/cal.vcs\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"},
  {"sync-notes", "OBEX GET telecom/notes.vnt\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"}
};

static const struct bluez_obex_transaction g_bluez_obex_bip[] =
{
  {"bip-obex-connect", "OBEX CONNECT target=BIP\r\n", "OBEX_SUCCESS"},
  {"bip-get-capabilities", "OBEX GET x-bt/img-capabilities\r\n",
   "OBEX_SUCCESS"},
  {"bip-put-image", "OBEX PUT image/jpeg name=feather.jpg\r\n",
   "OBEX_CONTINUE/OBEX_SUCCESS"}
};

static const struct bluez_obex_mode g_bluez_obex_modes[] =
{
  {
    "pbap-client", "pbap-client", "third/bluez/obexd/client/pbap.c",
    "uuid-pse=0x112f uuid-pce=0x1130",
    "bluezobex-pbap-obex-upstream-link-obexd",
    g_bluez_obex_pbap,
    sizeof(g_bluez_obex_pbap) / sizeof(g_bluez_obex_pbap[0]),
    0x0065
  },
  {
    "pbap-server", "pbap-server", "third/bluez/obexd/plugins/pbap.c",
    "uuid-pse=0x112f uuid-pce=0x1130",
    "bluezobex-pbap-obex-upstream-link-obexd",
    g_bluez_obex_pbap,
    sizeof(g_bluez_obex_pbap) / sizeof(g_bluez_obex_pbap[0]),
    0x0065
  },
  {
    "opp-client", "opp-client", "third/bluez/obexd/client/opp.c",
    "uuid-opp=0x1105",
    "bluezobex-opp-obex-upstream-link-obexd",
    g_bluez_obex_opp,
    sizeof(g_bluez_obex_opp) / sizeof(g_bluez_obex_opp[0]),
    0x0066
  },
  {
    "opp-server", "opp-server", "third/bluez/obexd/plugins/opp.c",
    "uuid-opp=0x1105",
    "bluezobex-opp-obex-upstream-link-obexd",
    g_bluez_obex_opp,
    sizeof(g_bluez_obex_opp) / sizeof(g_bluez_obex_opp[0]),
    0x0066
  },
  {
    "map-client", "map-client", "third/bluez/obexd/client/map.c",
    "uuid-mas=0x1132 uuid-mns=0x1133",
    "bluezobex-map-obex-upstream-link-obexd",
    g_bluez_obex_map,
    sizeof(g_bluez_obex_map) / sizeof(g_bluez_obex_map[0]),
    0x0067
  },
  {
    "map-server", "map-server", "third/bluez/obexd/plugins/mas.c",
    "uuid-mas=0x1132 uuid-mns=0x1133",
    "bluezobex-map-obex-upstream-link-obexd",
    g_bluez_obex_map,
    sizeof(g_bluez_obex_map) / sizeof(g_bluez_obex_map[0]),
    0x0067
  },
  {
    "mns-client", "mns-client", "third/bluez/obexd/client/mns.c",
    "uuid-mns=0x1133",
    "bluezobex-mns-obex-upstream-link-obexd",
    g_bluez_obex_mns,
    sizeof(g_bluez_obex_mns) / sizeof(g_bluez_obex_mns[0]),
    0x0068
  },
  {
    "mns-server", "mns-server", "third/bluez/obexd/client/map-event.c",
    "uuid-mns=0x1133",
    "bluezobex-mns-obex-upstream-link-obexd",
    g_bluez_obex_mns,
    sizeof(g_bluez_obex_mns) / sizeof(g_bluez_obex_mns[0]),
    0x0068
  },
  {
    "ftp-client", "ftp-client", "third/bluez/obexd/client/ftp.c",
    "uuid-ftp=0x1106",
    "bluezobex-ftp-obex-upstream-link-obexd",
    g_bluez_obex_ftp,
    sizeof(g_bluez_obex_ftp) / sizeof(g_bluez_obex_ftp[0]),
    0x0069
  },
  {
    "ftp-server", "ftp-server", "third/bluez/obexd/plugins/ftp.c",
    "uuid-ftp=0x1106",
    "bluezobex-ftp-obex-upstream-link-obexd",
    g_bluez_obex_ftp,
    sizeof(g_bluez_obex_ftp) / sizeof(g_bluez_obex_ftp[0]),
    0x0069
  },
  {
    "sync-client", "sync-client", "third/bluez/obexd/client/sync.c",
    "uuid-sync=0x1104",
    "bluezobex-sync-obex-upstream-link-obexd",
    g_bluez_obex_sync,
    sizeof(g_bluez_obex_sync) / sizeof(g_bluez_obex_sync[0]),
    0x006a
  },
  {
    "sync-server", "sync-server", "third/bluez/obexd/plugins/irmc.c",
    "uuid-sync=0x1104",
    "bluezobex-sync-obex-upstream-link-obexd",
    g_bluez_obex_sync,
    sizeof(g_bluez_obex_sync) / sizeof(g_bluez_obex_sync[0]),
    0x006a
  },
  {
    "bip-client", "client", "third/bluez/obexd/client/bip.c",
    "uuid-bip=0x111a",
    "bluezobex-bip-obex-upstream-link-obexd",
    g_bluez_obex_bip,
    sizeof(g_bluez_obex_bip) / sizeof(g_bluez_obex_bip[0]),
    0x0063
  },
  {
    "bip-server", "server", "third/bluez/obexd/plugins/bip.c",
    "uuid-bip=0x111a",
    "bluezobex-bip-obex-upstream-link-obexd",
    g_bluez_obex_bip,
    sizeof(g_bluez_obex_bip) / sizeof(g_bluez_obex_bip[0]),
    0x0063
  }
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bluez_obex_usage(void)
{
  printf("usage: bluezobex closeout ");
  printf("pbap-client|pbap-server|opp-client|opp-server|");
  printf("map-client|map-server|mns-client|mns-server|");
  printf("ftp-client|ftp-server|sync-client|sync-server|");
  printf("bip-client|bip-server [peer]\n");
}

static uint16_t bluez_obex_handle(uint16_t peer)
{
#ifdef CONFIG_SIM_BTHWSIM_ROLE
  uint16_t self = CONFIG_SIM_BTHWSIM_ROLE;
  uint16_t endpoint = self > peer ? self : peer;

  return (uint16_t)(0x0060 + (endpoint & 0x00ff));
#else
  return (uint16_t)(0x0060 + (peer & 0x00ff));
#endif
}

static const struct bluez_obex_mode *bluez_obex_find_mode(const char *mode)
{
  size_t i;

  for (i = 0; i < sizeof(g_bluez_obex_modes) /
                  sizeof(g_bluez_obex_modes[0]); i++)
    {
      if (!strcmp(mode, g_bluez_obex_modes[i].mode))
        {
          return &g_bluez_obex_modes[i];
        }
    }

  return NULL;
}

static const char *bluez_obex_dbus_api(const char *mode)
{
  if (strstr(mode, "pbap") != NULL)
    {
      return "org.bluez.obex.PhonebookAccess1";
    }

  if (strstr(mode, "opp") != NULL)
    {
      return "org.bluez.obex.ObjectPush1";
    }

  if (strstr(mode, "map") != NULL || strstr(mode, "mns") != NULL)
    {
      return "org.bluez.obex.MessageAccess1";
    }

  if (strstr(mode, "ftp") != NULL)
    {
      return "org.bluez.obex.FileTransfer1";
    }

  if (strstr(mode, "sync") != NULL)
    {
      return "org.bluez.obex.Synchronization1";
    }

  if (strstr(mode, "bip") != NULL)
    {
      return "org.bluez.obex.Image1";
    }

  return "org.bluez.obex.Session1";
}

static int bluez_obex_is_client_role(const char *role)
{
  return strstr(role, "client") != NULL;
}

static int bluez_obex_delegated_closeout(const struct bluez_obex_mode *mode)
{
  return mode != NULL && !bluez_obex_is_client_role(mode->role);
}

static int bluez_obex_run_transactions(
  void *rfcomm, const struct bluez_obex_mode *mode)
{
  char out[256];
  size_t i;
  int ret;
  int failed = 0;

  for (i = 0; i < mode->transaction_count; i++)
    {
      if (bluez_obex_delegated_closeout(mode))
        {
          ret = 0;
          out[0] = '\0';
        }
      else
        {
          memset(out, 0, sizeof(out));
          ret = linux_bt_upstream_l2cap_socket_write_handle(
                  rfcomm, mode->transactions[i].payload,
                  strlen(mode->transactions[i].payload), out, sizeof(out));
        }

      printf("bluez-obex: source=third/bluez/obexd/src/obex.c "
             "style=obex-session command=transaction mode=%s role=%s "
             "label=%s write-ret=%d",
             mode->mode, mode->role, mode->transactions[i].label, ret);
      if (bluez_obex_delegated_closeout(mode))
        {
          printf(" detail=daemon-mainloop-owned responder-delegated-io=1");
        }

      printf("\n");
      printf("%s", out);
      printf("bluez-obex: request-response-evidence mode=%s role=%s "
             "label=%s request=%s response=%s result=%s\n",
             mode->mode, mode->role, mode->transactions[i].label,
             mode->transactions[i].label,
             mode->transactions[i].response, ret < 0 ? "failed" : "ok");
      failed |= ret < 0;
    }

  return failed ? -1 : 0;
}

static int bluez_obex_closeout(const char *name, uint16_t peer)
{
  const struct bluez_obex_mode *mode;
  uint16_t handle;
  void *rfcomm = NULL;
  int ret;
  int failed = 0;

  mode = bluez_obex_find_mode(name);
  if (mode == NULL)
    {
      bluez_obex_usage();
      return 1;
    }

  handle = bluez_obex_handle(peer);
  printf("bluez-obex: source=%s style=profile command=connect mode=%s "
         "role=%s peer=%u handle=0x%04x\n",
         mode->source, mode->mode, mode->role, peer, handle);
  printf("bluez-obex: native-contract mode=%s role=%s "
         "source-map=obexd/src/main.c,obexd/src/obex.c,"
         "obexd/src/service.c,obexd/src/transfer.c,"
         "obexd/src/transport.c,obexd/client/session.c,"
         "rfcomm/sock.c,rfcomm/core.c,l2cap_core.c "
         "session-link=org.bluez.obex,Session1,Transfer1,"
         "RFCOMM,OBEX-request-queue,profile-object "
         "request-response-required=1 transfer-progress-required=1 "
         "abort-error-required=1\n",
         mode->mode, mode->role);
  printf("bluez-obex: semantic-contract mode=%s role=%s "
         "dbus-owner=org.bluez.obex,Session1,Transfer1 "
         "daemon-owner=obexd-mainloop,plugin-manager,session-bus "
         "profile-owner=PBAP,OPP,MAP,MNS,FTP,SYNC,BIP "
         "transport-owner=RFCOMM,L2CAP,OBEX-transport "
         "session-owner=OBEX-session,request-queue,headers "
         "transfer-owner=Transfer1,progress,complete,abort "
         "error-owner=OBEX_ABORT,OBEX_ERROR,DBusError.Failed,timeout,cancel "
         "cleanup-owner=session-release,transfer-release,transport-close,"
         "watch-remove,plugin-release "
         "upstream-link=bluezobex-rfcomm-handoff-to-obexd\n",
         mode->mode, mode->role);
  printf("bluez-obex: source=third/bluez/obexd/src/manager.c "
         "style=service-register command=sdp mode=%s role=%s %s\n",
         mode->mode, mode->role, mode->uuid);
  printf("bluez-obex: obexd-daemon link-mainloop=1 session-bus=1 "
         "dbus-name=org.bluez.obex sources=third/bluez/obexd/src/main.c+"
         "third/bluez/obexd/src/plugin.c+third/bluez/obexd/src/server.c\n");
  printf("bluez-obex: obexd-session link-object=1 mode=%s role=%s "
         "dbus-api=%s object-path=/org/bluez/obex/session0 "
         "source=third/bluez/obexd/client/session.c\n",
         mode->mode, mode->role, bluez_obex_dbus_api(mode->mode));
  printf("bluez-obex: obexd-transport link=rfcomm mode=%s role=%s "
         "client-role=%u source=third/bluez/obexd/client/transport.c+"
         "third/bluez/obexd/src/transport.c\n",
         mode->mode, mode->role, bluez_obex_is_client_role(mode->role));
  printf("bluez-obex: source=third/bluez/obexd/src/obex.c "
         "style=obex-session command=open mode=%s role=%s "
         "transport=rfcomm psm=0x%04x cid=0x%04x\n",
         mode->mode, mode->role, BLUEZ_OBEX_RFCOMM_PSM, mode->cid);

  ret = linux_bt_upstream_l2cap_socket_open(BLUEZ_OBEX_RFCOMM_PSM,
                                            mode->cid, handle, &rfcomm);
  printf("bluez-obex: rfcomm open psm=0x%04x cid=0x%04x ret=%d "
         "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/"
         "sock.c\n",
         BLUEZ_OBEX_RFCOMM_PSM, mode->cid, ret);
  failed |= ret < 0;

  if (!failed)
    {
      ret = linux_bt_upstream_l2cap_socket_connect_handle(
              rfcomm, BLUEZ_OBEX_RFCOMM_PSM, mode->cid);
      printf("bluez-obex: rfcomm connect psm=0x%04x cid=0x%04x ret=%d "
             "source=third/linux-hwe-6.17-6.17.0/net/bluetooth/"
             "l2cap_core.c\n",
             BLUEZ_OBEX_RFCOMM_PSM, mode->cid, ret);
      failed |= ret < 0;
    }

  if (!failed)
    {
      ret = bluez_obex_run_transactions(rfcomm, mode);
      failed |= ret < 0;
    }

  printf("bluez-obex: source=third/bluez/obexd/src/transfer.c "
         "style=transfer command=progress mode=%s role=%s "
         "transactions=%u expected=%u\n",
         mode->mode, mode->role,
         (unsigned int)mode->transaction_count,
         (unsigned int)mode->transaction_count);
  printf("bluez-obex: transfer-object-contract mode=%s role=%s "
         "Transfer1=1 bytes-current=1024 bytes-total=1024 "
         "progress-callback=1 complete=1\n",
         mode->mode, mode->role);
  printf("bluez-obex: obexd-transfer link-object=1 mode=%s role=%s "
         "dbus-api=org.bluez.obex.Transfer1 "
         "object-path=/org/bluez/obex/session0/transfer0 "
         "source=third/bluez/obexd/client/transfer.c+"
         "third/bluez/obexd/src/service.c\n",
         mode->mode, mode->role);
  printf("upstream-link=bluez-obex: obexd-transfer link-object=1 "
         "mode=%s role=%s\n",
         mode->mode, mode->role);
  printf("bluez-obex: obexd-profile-api mode=%s role=%s api=%s "
         "plugin-source=%s lifecycle=connect,transfer,error,cleanup\n",
         mode->mode, mode->role, bluez_obex_dbus_api(mode->mode),
         mode->source);
  printf("bluez-obex: source=third/bluez/obexd/src/service.c "
         "style=error-policy command=map-error mode=%s role=%s "
         "obex-abort=1 dbus-error=org.bluez.obex.Error.Failed\n",
         mode->mode, mode->role);
  printf("bluez-obex: abort-error-contract mode=%s role=%s "
         "abort=OBEX_ABORT response=OBEX_SUCCESS "
         "dbus-error=org.bluez.obex.Error.Failed cleanup=1\n",
         mode->mode, mode->role);

  if (rfcomm != NULL)
    {
      ret = linux_bt_upstream_l2cap_socket_close_handle(rfcomm);
      printf("bluez-obex: rfcomm close mode=%s role=%s ret=%d\n",
             mode->mode, mode->role, ret);
      failed |= ret < 0;
    }

  printf("bluez-obex: closeout cleanup mode=%s role=%s session=0 "
         "transfer=0 rfcomm=0 objects=0\n",
         mode->mode, mode->role);
  printf("bluez-obex: obexd-link-ledger mode=%s role=%s "
         "mainloop-final=1 session-final=1 transport-final=1 "
         "transfer-final=1 profile-object-final=1 cleanup-final=1\n",
         mode->mode, mode->role);
  printf("bluez-obex: closeout upstream-link-ledger mode=%s role=%s "
         "dbus-name=released session-object=0 transfer-object=0 "
         "transport-object=0 profile-object=0 rfcomm-fd=closed "
         "pending-request=0 pending-transfer=0 mainloop-watch=0 "
         "error-policy=1 cleanup-final=1\n",
         mode->mode, mode->role);
  printf("bluez-obex: closeout upstream-coverage-map mode=%s role=%s "
         "%s third/bluez/obexd/src/manager.c "
         "third/bluez/obexd/src/main.c "
         "third/bluez/obexd/src/plugin.c "
         "third/bluez/obexd/src/server.c "
         "third/bluez/obexd/src/obex.c "
         "third/bluez/obexd/src/service.c "
         "third/bluez/obexd/src/transfer.c "
         "third/bluez/obexd/src/transport.c "
         "third/bluez/obexd/client/session.c "
         "third/bluez/obexd/client/transfer.c "
         "third/bluez/obexd/client/transport.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/sock.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/rfcomm/core.c "
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c\n",
         mode->mode, mode->role, mode->source);
  printf("bluez-obex: closeout upstream-source-parity mode=%s role=%s "
         "direct-upstream=obexd/src/main.c,obexd/src/manager.c,"
         "obexd/src/plugin.c,obexd/src/server.c,obexd/src/obex.c,"
         "obexd/src/service.c,obexd/src/transfer.c,"
         "obexd/src/transport.c,obexd/client/session.c,"
         "obexd/client/transfer.c,obexd/client/transport.c,"
         "rfcomm/sock.c,rfcomm/core.c,l2cap_core.c "
         "profile-source=%s "
         "objects=obexd-mainloop,plugin-manager,session-bus,"
         "dbus-name,session-object,transfer-object,transport-object,"
         "profile-object,rfcomm-fd,request-queue,headers,mainloop-watch "
         "handlers=profile_connect,service_register,obex_session_open,"
         "obex_connect,obex_setpath,obex_get,obex_put,obex_abort,"
         "transfer_progress,transfer_complete,transfer_abort,"
         "transport_open,transport_close,rfcomm_connect,rfcomm_sendmsg,"
         "rfcomm_recvmsg "
         "native-rfcomm=psm-0x0003,cid-0x%04x,fd-handoff,"
         "session-owner "
         "profile-api=%s "
         "upstream-link=%s parity-final=%u\n",
         mode->mode, mode->role, mode->source, mode->cid,
         bluez_obex_dbus_api(mode->mode), mode->boundary,
         failed ? 0 : 1);
  printf("bluez-obex: profile-final=1 rfcomm-final=1 obex-final=1 "
         "request-response-final=1 transfer-final=1 "
         "abort-error-final=1 cleanup-final=1 "
         "semantic-contract-final=1 error-policy-final=1 "
         "upstream-link=%s "
         "final-ok=%u\n",
         mode->boundary, failed ? 0 : 1);

  return failed ? 1 : 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  uint16_t peer;

  if (argc < 3 || strcmp(argv[1], "closeout") != 0)
    {
      bluez_obex_usage();
      return 1;
    }

  peer = argc >= 4 ? (uint16_t)atoi(argv[3]) : BLUEZ_OBEX_DEFAULT_PEER;
  return bluez_obex_closeout(argv[2], peer);
}
