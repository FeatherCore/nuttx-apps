/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_avdtp_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "btio/btio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef __LITTLE_ENDIAN
#  define __LITTLE_ENDIAN 1234
#endif

#ifndef __BIG_ENDIAN
#  define __BIG_ENDIAN 4321
#endif

#ifndef __BYTE_ORDER
#  define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#define avdtp_ref bluez_upstream_object_avdtp_ref
#define avdtp_unref bluez_upstream_object_avdtp_unref
#define avdtp_service_cap_new bluez_upstream_object_avdtp_service_cap_new
#define avdtp_register_remote_sep bluez_upstream_object_avdtp_register_remote_sep
#define avdtp_unregister_remote_sep bluez_upstream_object_avdtp_unregister_remote_sep
#define avdtp_stream_add_cb bluez_upstream_object_avdtp_stream_add_cb
#define avdtp_open bluez_upstream_object_avdtp_open
#define avdtp_unregister_sep bluez_upstream_object_avdtp_unregister_sep
#define avdtp_register_sep bluez_upstream_object_avdtp_register_sep
#define avdtp_get_version bluez_upstream_object_avdtp_get_version
#define avdtp_discover bluez_upstream_object_avdtp_discover

#ifndef SHUT_WR
#  define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#  define SHUT_RDWR 2
#endif

static struct
{
  struct
  {
    BtIOMode session_mode;
    BtIOMode stream_mode;
  } avdtp;
} btd_opts =
{
  {
    BT_IO_MODE_BASIC,
    BT_IO_MODE_BASIC
  }
};

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#include "upstream/profiles/audio/avdtp.c"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

gboolean sink_setup_stream(struct btd_service *service,
                           struct avdtp *session)
{
  (void)service;
  (void)session;
  return TRUE;
}

gboolean source_setup_stream(struct btd_service *service,
                             struct avdtp *session)
{
  (void)service;
  (void)session;
  return TRUE;
}

static size_t bluez_upstream_avdtp_probe_make_accept_frame(
  uint8_t *frame, size_t frame_size, uint8_t transaction,
  uint8_t signal_id, const void *payload, size_t payload_size)
{
  struct avdtp_single_header *header =
    (struct avdtp_single_header *)frame;

  if (frame == NULL || frame_size < sizeof(*header) + payload_size)
    {
      return 0;
    }

  memset(frame, 0, frame_size);
  header->transaction = transaction;
  header->packet_type = AVDTP_PKT_TYPE_SINGLE;
  header->message_type = AVDTP_MSG_TYPE_ACCEPT;
  header->signal_id = signal_id;

  if (payload != NULL && payload_size != 0)
    {
      memcpy(frame + sizeof(*header), payload, payload_size);
    }

  return sizeof(*header) + payload_size;
}

static gboolean bluez_upstream_avdtp_probe_feed_session_frame(
  struct avdtp *session, int peer_fd, const uint8_t *frame,
  size_t frame_size)
{
  if (session == NULL || frame == NULL || frame_size == 0)
    {
      return FALSE;
    }

  if (write(peer_fd, frame, frame_size) != (ssize_t)frame_size)
    {
      return FALSE;
    }

  return session_cb(session->io, 0, session);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_avdtp_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: audio/avdtp.c role=%s linked=1 "
         "source=third/bluez/profiles/audio/avdtp.c\n",
         role);
}

unsigned int bluez_upstream_avdtp_control_dependency_bound(void)
{
  unsigned int symbols = 0;

  symbols += sizeof(&bluez_upstream_object_avdtp_stream_add_cb) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_avdtp_open) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_avdtp_unregister_sep) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_avdtp_register_sep) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_avdtp_get_version) > 0 ? 1 : 0;
  symbols += sizeof(&bluez_upstream_object_avdtp_discover) > 0 ? 1 : 0;

  return symbols == 6 ? 1 : 0;
}

struct avdtp *
bluez_upstream_avdtp_object_create_synthetic_session(
  struct btd_device *device)
{
  static struct avdtp session;

  memset(&session, 0, sizeof(session));
  session.device = device;
  session.ref = 1;
  return &session;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_raw_caps(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_service_capability *codec = NULL;
  gboolean delay_reporting = FALSE;
  GSList *caps;
  uint8_t err = 0;

  if (session == NULL || data == NULL || size == 0)
    {
      return NULL;
    }

  caps = caps_to_list(data, size, &codec, &delay_reporting, &err);
  if (caps == NULL || codec == NULL || err != 0)
    {
      return NULL;
    }

  return avdtp_register_remote_sep(session, seid, type, caps,
                                   delay_reporting);
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_getcap_response(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep = NULL;
  struct pending_req req;
  struct seid_req sreq;
  gboolean parsed;

  if (session == NULL || data == NULL || size == 0)
    {
      return NULL;
    }

  sep = avdtp_register_remote_sep(session, seid, type, NULL, false);
  if (sep == NULL)
    {
      return NULL;
    }

  memset(&req, 0, sizeof(req));
  memset(&sreq, 0, sizeof(sreq));
  sreq.acp_seid = seid;
  req.signal_id = AVDTP_GET_CAPABILITIES;
  req.data = &sreq;
  session->req = &req;

  parsed = avdtp_get_capabilities_resp(session,
                                       (struct getcap_resp *)data, size);
  session->req = NULL;

  return parsed == TRUE ? sep : NULL;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_parse_response(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep;
  struct pending_req req;
  struct seid_req sreq;
  gboolean parsed;

  if (session == NULL || data == NULL || size == 0)
    {
      return NULL;
    }

  sep = avdtp_register_remote_sep(session, seid, type, NULL, false);
  if (sep == NULL)
    {
      return NULL;
    }

  memset(&req, 0, sizeof(req));
  memset(&sreq, 0, sizeof(sreq));
  sreq.acp_seid = seid;
  req.signal_id = AVDTP_GET_CAPABILITIES;
  req.data = &sreq;
  session->req = &req;

  parsed = avdtp_parse_resp(session, NULL, 0, AVDTP_GET_CAPABILITIES,
                            data, size);
  session->req = NULL;

  return parsed == TRUE ? sep : NULL;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_parse_data(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep;
  struct pending_req req;
  struct seid_req sreq;
  uint8_t frame[sizeof(struct avdtp_single_header) + 64];
  struct avdtp_single_header *header =
    (struct avdtp_single_header *)frame;
  char *saved_buf;
  gboolean parsed = FALSE;

  if (session == NULL || data == NULL || size == 0 ||
      size > sizeof(frame) - sizeof(*header))
    {
      return NULL;
    }

  sep = avdtp_register_remote_sep(session, seid, type, NULL, false);
  if (sep == NULL)
    {
      return NULL;
    }

  memset(&req, 0, sizeof(req));
  memset(&sreq, 0, sizeof(sreq));
  memset(frame, 0, sizeof(frame));
  sreq.acp_seid = seid;
  req.transaction = 1;
  req.signal_id = AVDTP_GET_CAPABILITIES;
  req.data = &sreq;
  session->req = &req;

  header->transaction = req.transaction;
  header->packet_type = AVDTP_PKT_TYPE_SINGLE;
  header->message_type = AVDTP_MSG_TYPE_ACCEPT;
  header->signal_id = AVDTP_GET_CAPABILITIES;
  memcpy(frame + sizeof(*header), data, size);

  saved_buf = session->buf;
  session->buf = (char *)frame;

  if (avdtp_parse_data(session, frame, sizeof(*header) + size) ==
      PARSE_SUCCESS &&
      session->in_resp.transaction == req.transaction &&
      session->in_resp.signal_id == req.signal_id)
    {
      parsed = avdtp_parse_resp(session, NULL,
                                session->in_resp.transaction,
                                session->in_resp.signal_id,
                                session->in_resp.buf,
                                session->in_resp.data_size);
    }

  session->buf = saved_buf;
  session->req = NULL;

  return parsed == TRUE ? sep : NULL;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_session_cb(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep;
  struct pending_req *req;
  struct seid_req *sreq;
  uint8_t frame[sizeof(struct avdtp_single_header) + 64];
  struct avdtp_single_header *header =
    (struct avdtp_single_header *)frame;
  char packet_buf[sizeof(frame)];
  char *saved_buf;
  GIOChannel *saved_io;
  uint16_t saved_imtu;
  int fds[2];
  gboolean handled = FALSE;

  if (session == NULL || data == NULL || size == 0 ||
      size > sizeof(frame) - sizeof(*header))
    {
      return NULL;
    }

  sep = avdtp_register_remote_sep(session, seid, type, NULL, false);
  if (sep == NULL)
    {
      return NULL;
    }

  req = g_new0(struct pending_req, 1);
  sreq = g_new0(struct seid_req, 1);
  if (req == NULL || sreq == NULL)
    {
      free(sreq);
      g_free(req);
      return NULL;
    }

  memset(frame, 0, sizeof(frame));
  sreq->acp_seid = seid;
  req->transaction = 1;
  req->signal_id = AVDTP_GET_CAPABILITIES;
  req->data = sreq;
  req->data_size = sizeof(*sreq);

  header->transaction = req->transaction;
  header->packet_type = AVDTP_PKT_TYPE_SINGLE;
  header->message_type = AVDTP_MSG_TYPE_ACCEPT;
  header->signal_id = AVDTP_GET_CAPABILITIES;
  memcpy(frame + sizeof(*header), data, size);

  if (pipe(fds) != 0)
    {
      pending_req_free(req);
      return NULL;
    }

  if (write(fds[1], frame, sizeof(*header) + size) !=
      (ssize_t)(sizeof(*header) + size))
    {
      close(fds[0]);
      close(fds[1]);
      pending_req_free(req);
      return NULL;
    }

  close(fds[1]);

  saved_buf = session->buf;
  saved_io = session->io;
  saved_imtu = session->imtu;
  session->buf = packet_buf;
  session->io = g_io_channel_unix_new(fds[0]);
  session->imtu = sizeof(packet_buf);
  session->req = req;

  handled = session_cb(session->io, 0, session);

  close(fds[0]);
  session->buf = saved_buf;
  session->io = saved_io;
  session->imtu = saved_imtu;

  if (session->req != NULL)
    {
      pending_req_free(session->req);
      session->req = NULL;
    }

  return handled == TRUE && avdtp_get_codec(sep) != NULL ? sep : NULL;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_discover(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep;
  struct seid_info info;
  uint8_t frame[sizeof(struct avdtp_single_header) + 64];
  char packet_buf[sizeof(frame)];
  char *saved_buf;
  GIOChannel *saved_io;
  avdtp_session_state_t saved_state;
  uint16_t saved_imtu;
  uint16_t saved_omtu;
  int fds[2];
  uint8_t command[64];
  size_t frame_size;
  ssize_t ignored;
  gboolean ok = FALSE;

  if (session == NULL || data == NULL || size == 0 ||
      size > sizeof(frame) - sizeof(struct avdtp_single_header))
    {
      return NULL;
    }

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
    {
      return NULL;
    }

  saved_buf = session->buf;
  saved_io = session->io;
  saved_state = session->state;
  saved_imtu = session->imtu;
  saved_omtu = session->omtu;

  session->buf = packet_buf;
  session->io = g_io_channel_unix_new(fds[0]);
  session->state = AVDTP_SESSION_STATE_CONNECTED;
  session->imtu = sizeof(packet_buf);
  session->omtu = sizeof(packet_buf);

  if (avdtp_discover(session, NULL, NULL) != 0 || session->req == NULL ||
      session->req->signal_id != AVDTP_DISCOVER)
    {
      goto out;
    }

  ignored = read(fds[1], command, sizeof(command));
  (void)ignored;

  memset(&info, 0, sizeof(info));
  info.seid = seid;
  info.type = type;
  info.media_type = AVDTP_MEDIA_TYPE_AUDIO;
  frame_size = bluez_upstream_avdtp_probe_make_accept_frame(
    frame, sizeof(frame), session->req->transaction, AVDTP_DISCOVER,
    &info, sizeof(info));

  if (frame_size == 0 ||
      bluez_upstream_avdtp_probe_feed_session_frame(session, fds[1],
                                                    frame, frame_size) != TRUE ||
      session->req == NULL ||
      session->req->signal_id != AVDTP_GET_CAPABILITIES)
    {
      goto out;
    }

  ignored = read(fds[1], command, sizeof(command));
  (void)ignored;

  frame_size = bluez_upstream_avdtp_probe_make_accept_frame(
    frame, sizeof(frame), session->req->transaction,
    AVDTP_GET_CAPABILITIES, data, size);
  if (frame_size == 0 ||
      bluez_upstream_avdtp_probe_feed_session_frame(session, fds[1],
                                                    frame, frame_size) != TRUE)
    {
      goto out;
    }

  sep = find_remote_sep(session->seps, seid);
  ok = sep != NULL && sep->discovered && avdtp_get_codec(sep) != NULL;

out:
  if (session->req != NULL)
    {
      pending_req_free(session->req);
      session->req = NULL;
    }

  close(fds[0]);
  close(fds[1]);
  session->buf = saved_buf;
  session->io = saved_io;
  session->state = saved_state;
  session->imtu = saved_imtu;
  session->omtu = saved_omtu;

  return ok ? sep : NULL;
}

struct avdtp_remote_sep *
bluez_upstream_avdtp_object_register_remote_sep_from_l2cap_connect(
  struct avdtp *session, uint8_t seid, uint8_t type,
  uint8_t *data, size_t size)
{
  struct avdtp_remote_sep *sep = NULL;
  struct seid_info info;
  uint8_t frame[sizeof(struct avdtp_single_header) + 64];
  avdtp_session_state_t saved_state;
  GIOChannel *saved_io;
  char *saved_buf;
  guint saved_io_id;
  uint16_t saved_imtu;
  uint16_t saved_omtu;
  int peer_fd = -1;
  uint8_t command[64];
  size_t frame_size;
  ssize_t ignored;
  gboolean ok = FALSE;

  if (session == NULL || data == NULL || size == 0 ||
      size > sizeof(frame) - sizeof(struct avdtp_single_header))
    {
      return NULL;
    }

  saved_state = session->state;
  saved_io = session->io;
  saved_buf = session->buf;
  saved_io_id = session->io_id;
  saved_imtu = session->imtu;
  saved_omtu = session->omtu;

  session->state = AVDTP_SESSION_STATE_DISCONNECTED;
  session->io = NULL;
  session->buf = NULL;
  session->imtu = 0;
  session->omtu = 0;

  if (avdtp_discover(session, NULL, NULL) != 0 ||
      session->req_queue == NULL)
    {
      goto out;
    }

  peer_fd = bt_io_shim_take_last_peer_fd();
  if (peer_fd < 0 || session->io == NULL ||
      session->state != AVDTP_SESSION_STATE_CONNECTING)
    {
      goto out;
    }

  avdtp_connect_cb(session->io, NULL, session);
  if (session->state != AVDTP_SESSION_STATE_CONNECTED ||
      session->req == NULL ||
      session->req->signal_id != AVDTP_DISCOVER)
    {
      goto out;
    }

  ignored = read(peer_fd, command, sizeof(command));
  (void)ignored;

  memset(&info, 0, sizeof(info));
  info.seid = seid;
  info.type = type;
  info.media_type = AVDTP_MEDIA_TYPE_AUDIO;
  frame_size = bluez_upstream_avdtp_probe_make_accept_frame(
    frame, sizeof(frame), session->req->transaction, AVDTP_DISCOVER,
    &info, sizeof(info));

  if (frame_size == 0 ||
      bluez_upstream_avdtp_probe_feed_session_frame(session, peer_fd,
                                                    frame, frame_size) != TRUE ||
      session->req == NULL ||
      session->req->signal_id != AVDTP_GET_CAPABILITIES)
    {
      goto out;
    }

  ignored = read(peer_fd, command, sizeof(command));
  (void)ignored;

  frame_size = bluez_upstream_avdtp_probe_make_accept_frame(
    frame, sizeof(frame), session->req->transaction,
    AVDTP_GET_CAPABILITIES, data, size);
  if (frame_size == 0 ||
      bluez_upstream_avdtp_probe_feed_session_frame(session, peer_fd,
                                                    frame, frame_size) != TRUE)
    {
      goto out;
    }

  sep = find_remote_sep(session->seps, seid);
  ok = sep != NULL && sep->discovered && avdtp_get_codec(sep) != NULL;

out:
  if (session->req != NULL)
    {
      pending_req_free(session->req);
      session->req = NULL;
    }

  if (session->req_queue != NULL)
    {
      g_slist_free_full(session->req_queue, pending_req_free);
      session->req_queue = NULL;
    }

  if (session->prio_queue != NULL)
    {
      g_slist_free_full(session->prio_queue, pending_req_free);
      session->prio_queue = NULL;
    }

  if (session->discover != NULL)
    {
      g_free(session->discover);
      session->discover = NULL;
    }

  if (peer_fd >= 0)
    {
      close(peer_fd);
    }

  if (session->io != NULL && session->io != saved_io)
    {
      int fd = g_io_channel_unix_get_fd(session->io);

      if (fd >= 0)
        {
          close(fd);
        }
    }

  if (session->buf != NULL && session->buf != saved_buf)
    {
      g_free(session->buf);
    }

  if (session->io_id != 0 && session->io_id != saved_io_id)
    {
      g_source_remove(session->io_id);
    }

  session->state = saved_state;
  session->io = saved_io;
  session->buf = saved_buf;
  session->io_id = saved_io_id;
  session->imtu = saved_imtu;
  session->omtu = saved_omtu;

  return ok ? sep : NULL;
}
