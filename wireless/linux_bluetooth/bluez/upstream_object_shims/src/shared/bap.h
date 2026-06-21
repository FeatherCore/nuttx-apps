#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_BAP_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SHARED_BAP_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/uio.h>

struct bt_bap_stream;
struct bt_bap_io;
struct bt_bap_pac;
struct bt_bap;

struct bt_bap_pac_qos
{
  uint8_t framing;
  uint8_t phys;
  uint8_t rtn;
  uint16_t latency;
  uint16_t pd_min;
  uint16_t pd_max;
  uint16_t ppd_min;
  uint16_t ppd_max;
  uint32_t location;
  uint16_t context;
  uint16_t supported_context;
  uint8_t target_latency;
};

struct bt_bap_io_qos
{
  uint8_t cig_id;
  uint8_t cis_id;
  uint32_t interval;
  uint16_t latency;
  uint32_t sdu;
  uint8_t phy;
  uint8_t phys;
  uint8_t rtn;
};

struct bt_bap_qos
{
  struct
  {
    uint8_t cig_id;
    uint8_t cis_id;
    uint32_t interval;
    uint16_t latency;
    uint32_t sdu;
    uint8_t phy;
    uint8_t phys;
    uint8_t rtn;
    uint8_t framing;
    uint32_t delay;
    uint8_t target_latency;
    struct bt_bap_io_qos io_qos;
  } ucast;
  struct
  {
    uint8_t big;
    uint8_t bis;
    uint32_t sync_factor;
    uint32_t packing;
    uint32_t framing;
    uint32_t encryption;
    uint32_t options;
    uint32_t skip;
    uint32_t sync_timeout;
    uint32_t sync_cte_type;
    uint32_t mse;
    uint32_t timeout;
    uint32_t interval;
    uint32_t latency;
    uint32_t sdu;
    uint32_t phy;
    uint32_t rtn;
    uint32_t delay;
    struct bt_bap_io_qos io_qos;
    struct iovec *bcode;
  } bcast;
};

typedef void (*bt_bap_stream_func_t)(struct bt_bap_stream *stream,
                                     void *user_data);
typedef void (*bt_bap_pac_select_t)(struct bt_bap_pac *pac, int err,
                                    struct iovec *caps,
                                    struct iovec *metadata,
                                    struct bt_bap_qos *qos,
                                    void *user_data);
typedef void (*bt_bap_pac_config_t)(struct bt_bap_stream *stream, int err);

struct bt_bap_pac_ops
{
  int (*select)(struct bt_bap_pac *lpac, struct bt_bap_pac *rpac,
                uint32_t location, struct bt_bap_pac_qos *qos,
                bt_bap_pac_select_t cb, void *cb_data, void *user_data);
  void (*cancel_select)(struct bt_bap_pac *lpac, bt_bap_pac_select_t cb,
                        void *cb_data, void *user_data);
  int (*config)(struct bt_bap_stream *stream, struct iovec *cfg,
                struct bt_bap_qos *qos, bt_bap_pac_config_t cb,
                void *user_data);
  void (*clear)(struct bt_bap_stream *stream, void *user_data);
};

#define BT_BAP_STREAM_STATE_IDLE       0x00
#define BT_BAP_STREAM_STATE_CONFIG     0x01
#define BT_BAP_STREAM_STATE_QOS        0x02
#define BT_BAP_STREAM_STATE_ENABLING   0x03
#define BT_BAP_STREAM_STATE_STREAMING  0x04
#define BT_BAP_STREAM_STATE_DISABLING  0x05
#define BT_BAP_STREAM_STATE_RELEASING  0x06

#define BT_BAP_SINK         0x01
#define BT_BAP_SOURCE       0x02
#define BT_BAP_BCAST_SINK   0x03
#define BT_BAP_BCAST_SOURCE 0x04

#define BT_BAP_STREAM_TYPE_UCAST 0x00
#define BT_BAP_STREAM_TYPE_BCAST 0x01
#define BT_ISO_QOS_CIG_UNSET 0xff
#define BT_ISO_QOS_CIS_UNSET 0xff

static inline void *bt_bap_stream_get_user_data(struct bt_bap_stream *stream)
{
  (void)stream;
  return NULL;
}

static inline uint8_t bt_bap_stream_io_dir(struct bt_bap_stream *stream)
{
  (void)stream;
  return BT_BAP_SINK;
}

static inline int bt_bap_stream_get_io(struct bt_bap_stream *stream)
{
  (void)stream;
  return -1;
}

static inline void bt_bap_remove_pac(struct bt_bap_pac *pac)
{
  (void)pac;
}

static inline void bt_bap_pac_get_codec(struct bt_bap_pac *pac,
                                        uint8_t *codec,
                                        struct iovec **caps,
                                        struct iovec **metadata)
{
  (void)pac;
  if (codec != NULL)
    {
      *codec = 0;
    }

  if (caps != NULL)
    {
      *caps = NULL;
    }

  if (metadata != NULL)
    {
      *metadata = NULL;
    }
}

static inline const char *bt_bap_pac_get_user_data(struct bt_bap_pac *pac)
{
  (void)pac;
  return NULL;
}

static inline uint32_t bt_bap_pac_get_locations(struct bt_bap_pac *pac)
{
  (void)pac;
  return 0;
}

static inline struct bt_bap *bt_bap_stream_get_session(
    struct bt_bap_stream *stream)
{
  (void)stream;
  return NULL;
}

static inline void *bt_bap_get_user_data(struct bt_bap *bap)
{
  (void)bap;
  return NULL;
}

static inline struct bt_att *bt_bap_get_att(struct bt_bap *bap)
{
  (void)bap;
  return NULL;
}

static inline uint8_t bt_bap_stream_get_type(struct bt_bap_stream *stream)
{
  (void)stream;
  return BT_BAP_STREAM_TYPE_UCAST;
}

static inline struct bt_bap_pac *bt_bap_add_vendor_pac_full(
    void *db, const char *name, uint8_t type, uint8_t codec,
    uint16_t cid, uint16_t vid, struct bt_bap_pac_qos *qos,
    struct iovec *caps, struct iovec *metadata,
    struct bt_bap_pac_ops *ops, void *user_data)
{
  (void)db;
  (void)name;
  (void)type;
  (void)codec;
  (void)cid;
  (void)vid;
  (void)qos;
  (void)caps;
  (void)metadata;
  (void)ops;
  (void)user_data;
  return NULL;
}

static inline int bt_bap_stream_qos(struct bt_bap_stream *stream,
                                    struct bt_bap_qos *qos,
                                    void *func, void *user_data)
{
  (void)stream;
  (void)qos;
  (void)func;
  (void)user_data;
  return 0;
}

#endif
