#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SDP_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SDP_H

#include <stdint.h>

typedef struct sdp_record
{
  uint32_t handle;
  struct sdp_list *pattern;
  struct sdp_list *svclass;
  struct sdp_list *attrlist;
} sdp_record_t;

typedef struct
{
  uint8_t type;
  union
  {
    uint16_t uuid16;
    uint32_t uuid32;
    struct { uint8_t data[16]; } uuid128;
  } value;
} uuid_t;

typedef struct
{
  uuid_t uuid;
  uint16_t version;
} sdp_profile_desc_t;

typedef struct sdp_list
{
  void *data;
  struct sdp_list *next;
} sdp_list_t;

typedef struct sdp_data
{
  uint8_t dtd;
  union
  {
    int8_t int8;
    uint16_t uint16;
    uuid_t uuid;
  } val;
} sdp_data_t;

typedef void (*sdp_list_func_t)(void *data, void *user_data);
typedef struct
{
  uint8_t major;
  uint8_t minor;
} sdp_version_t;

typedef struct
{
  uint8_t pdu_id;
  uint16_t tid;
  uint16_t plen;
} __attribute__((packed)) sdp_pdu_hdr_t;

#define SDP_UINT32 0x0a
#define SDP_UINT64 0x0b
#define SDP_UINT16 0x09
#define SDP_UUID16 0x19
#define SDP_IS_UUID(dtd) ((dtd) == SDP_UUID16)
#define SDP_SERVER_RECORD_HANDLE 0x00010000
#define SDP_SERVER_SVCLASS_ID 0x1000
#define BROWSE_GRP_DESC_SVCLASS_ID 0x1001
#define PUBLIC_BROWSE_GROUP 0x1002
#define DIALUP_NET_SVCLASS_ID 0x1103
#define CIP_SVCLASS_ID 0x1128
#define HEADSET_SVCLASS_ID 0x1108
#define AUDIO_SOURCE_SVCLASS_ID 0x110a
#define AUDIO_SINK_SVCLASS_ID 0x110b
#define IRMC_SYNC_SVCLASS_ID 0x1104
#define OBEX_OBJPUSH_SVCLASS_ID 0x1105
#define OBEX_FILETRANS_SVCLASS_ID 0x1106
#define IRMC_SYNC_CMD_SVCLASS_ID 0x1107
#define CORDLESS_TELEPHONY_SVCLASS_ID 0x1109
#define ADVANCED_AUDIO_PROFILE_ID 0x110d
#define AV_REMOTE_TARGET_SVCLASS_ID 0x110c
#define AV_REMOTE_SVCLASS_ID 0x110e
#define AV_REMOTE_CONTROLLER_SVCLASS_ID 0x110f
#define AV_REMOTE_PROFILE_ID 0x110e
#define INTERCOM_SVCLASS_ID 0x1110
#define FAX_SVCLASS_ID 0x1111
#define HEADSFREE_SVCLASS_ID 0x111e
#define HANDSFREE_SVCLASS_ID 0x111e
#define HANDSFREE_AGW_SVCLASS_ID 0x111f
#define SAP_SVCLASS_ID 0x112d
#define PANU_SVCLASS_ID 0x1115
#define NAP_SVCLASS_ID 0x1116
#define GN_SVCLASS_ID 0x1117
#define VIDEO_SOURCE_SVCLASS_ID 0x1303
#define VIDEO_SINK_SVCLASS_ID 0x1304
#define PBAP_PSE_SVCLASS_ID 0x112f
#define PBAP_PCE_SVCLASS_ID 0x1130
#define MPS_SVCLASS_ID 0x113a
#define MPS_PROFILE_ID 0x113a
#define L2CAP_UUID 0x0100
#define SDP_PSM 0x0001
#define OBEX_UUID 0x0008
#define PNP_INFO_PROFILE_ID 0x1200
#define SDP_ATTR_RECORD_HANDLE 0x0000
#define SDP_ATTR_VERSION_NUM_LIST 0x0200
#define SDP_ATTR_GROUP_ID 0x0200
#define SDP_ATTR_BROWSE_GRP_LIST 0x0005
#define SDP_ATTR_SVCDB_STATE 0x0201
#define SDP_ATTR_MPSD_SCENARIOS 0x0200
#define SDP_ATTR_MPMD_SCENARIOS 0x0201
#define SDP_ATTR_MPS_DEPENDENCIES 0x0202
#define SDP_ATTR_SUPPORTED_FEATURES 0x0311
#define SDP_DEVICE_RECORD 0x01
#define SDP_RECORD_PERSIST 0x02
#define SDP_INVALID_SYNTAX 0x0003
#define SDP_INVALID_RECORD_HANDLE 0x0002
#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_SDP_PROFILE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_SDP_PROFILE_COMPAT

typedef struct {
	uint8_t *data;
	uint32_t data_size;
	uint32_t buf_size;
} sdp_buf_t;

#ifndef SDP_BOOL
#define SDP_BOOL 0x28
#endif
#ifndef SDP_ATTR_REMOTE_AUDIO_VOLUME_CONTROL
#define SDP_ATTR_REMOTE_AUDIO_VOLUME_CONTROL 0x0302
#endif
#ifndef SDP_ATTR_GOEP_L2CAP_PSM
#define SDP_ATTR_GOEP_L2CAP_PSM 0x0200
#endif

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_SDP_DEVICE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_SDP_DEVICE_COMPAT
#define SDP_UUID32 0x1a
#define SDP_UUID128 0x1c
#define ATT_UUID 0x0007
#define GATT_PRIM_SVC_UUID 0x2800
#define PNP_INFO_SVCLASS_ID 0x1200
#define PNP_UUID "00001200-0000-1000-8000-00805f9b34fb"
#define SDP_ATTR_VENDOR_ID_SOURCE 0x0205
#define SDP_ATTR_VENDOR_ID 0x0201
#define SDP_ATTR_PRODUCT_ID 0x0202
#define SDP_ATTR_VERSION 0x0203
#define SDP_LARGE_MTU 672

typedef void (*sdp_free_func_t)(void *data);

static inline int sdp_uuid_cmp(const void *a, const void *b)
{
	const uuid_t *ua = a;
	const uuid_t *ub = b;

	if (!ua || !ub)
		return ua == ub ? 0 : (ua ? 1 : -1);

	if (ua->type != ub->type)
		return (int)ua->type - (int)ub->type;

	return (int)ua->value.uuid16 - (int)ub->value.uuid16;
}
#endif
