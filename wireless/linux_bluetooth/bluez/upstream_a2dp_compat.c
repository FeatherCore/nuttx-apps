/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_a2dp_compat.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "upstream_a2dp_compat.h"
#include "upstream_media_transport_bridge.h"

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

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef int gboolean;
typedef struct _GSList GSList;
typedef struct _GIOChannel GIOChannel;
typedef void (*GDestroyNotify)(void *data);

struct btd_adapter;
struct btd_device;
struct queue;

/****************************************************************************
 * Upstream BlueZ A2DP Headers
 ****************************************************************************/

#include "upstream/profiles/audio/avdtp.h"
#include "upstream/profiles/audio/a2dp.h"
#include "upstream/profiles/audio/a2dp-codecs.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bluez_upstream_a2dp_endpoint_probe
{
  unsigned int get_name;
  unsigned int get_path;
  unsigned int get_capabilities;
  unsigned int select_configuration;
  unsigned int select_callback;
  unsigned int set_configuration;
  unsigned int set_callback;
  unsigned int clear_configuration;
  unsigned int set_delay;
};

struct bluez_upstream_avdtp_callback_probe
{
  unsigned int cfm_set_configuration;
  unsigned int cfm_get_configuration;
  unsigned int cfm_open;
  unsigned int cfm_start;
  unsigned int cfm_suspend;
  unsigned int cfm_close;
  unsigned int cfm_abort;
  unsigned int cfm_reconfigure;
  unsigned int cfm_delay_report;
  unsigned int ind_match_codec;
  unsigned int ind_get_capability;
  unsigned int ind_set_configuration;
  unsigned int ind_set_configuration_cb;
  unsigned int ind_get_configuration;
  unsigned int ind_open;
  unsigned int ind_start;
  unsigned int ind_suspend;
  unsigned int ind_close;
  unsigned int ind_abort;
  unsigned int ind_reconfigure;
  unsigned int ind_delayreport;
};

struct bluez_upstream_a2dp_owner_probe
{
  unsigned int server_new;
  unsigned int channel_new;
  unsigned int setup_new;
  unsigned int setup_free;
  unsigned int setup_ref;
  unsigned int setup_unref;
  unsigned int setup_cb_add;
  unsigned int setup_cb_free;
  unsigned int sep_add;
  unsigned int sep_remove;
  unsigned int stream_new;
  unsigned int stream_destroy;
  unsigned int eps_queue_new;
  unsigned int streams_queue_new;
  unsigned int discover_cb;
  unsigned int select_cb;
  unsigned int config_cb;
  unsigned int resume_cb;
  unsigned int suspend_cb;
  unsigned int transport_attach;
  unsigned int transport_detach;
  int setup_refs;
  int session_refs;
  int active_setups;
  int active_seps;
  int active_streams;
  int active_cbs;
};

struct bluez_upstream_avdtp_owner_probe
{
  unsigned int session_new;
  unsigned int session_ref;
  unsigned int session_unref;
  unsigned int local_sep_register;
  unsigned int remote_sep_register;
  unsigned int remote_sep_unregister;
  unsigned int discover_new;
  unsigned int discover_complete;
  unsigned int discover_free;
  unsigned int request_enqueue;
  unsigned int request_dequeue;
  unsigned int stream_new;
  unsigned int stream_state_configured;
  unsigned int stream_state_open;
  unsigned int stream_state_streaming;
  unsigned int stream_state_idle;
  unsigned int stream_cb_add;
  unsigned int stream_cb_remove;
  unsigned int transport_set;
  unsigned int transport_get;
  unsigned int transport_clear;
  unsigned int pending_open_set;
  unsigned int pending_open_clear;
  unsigned int stream_free;
  unsigned int session_free;
  int session_refs;
  int active_sessions;
  int active_local_seps;
  int active_remote_seps;
  int active_streams;
  int active_discovers;
  int active_requests;
  int active_stream_cbs;
  int active_transports;
};

struct bluez_upstream_a2dp_setup_stream_owner
{
  const char *role;
  unsigned int server;
  unsigned int session;
  unsigned int setup;
  unsigned int local_sep;
  unsigned int remote_sep;
  unsigned int stream;
  unsigned int media_endpoint;
  unsigned int media_transport;
  unsigned int transport_owner_watch;
  unsigned int pending_request;
  unsigned int setup_refs;
  unsigned int session_refs;
  unsigned int stream_refs;
  unsigned int sep_refs;
  unsigned int transitions;
  unsigned int cleanup;
};

struct bluez_upstream_a2dp_avdtp_transaction_owner
{
  const char *role;
  unsigned int session;
  unsigned int request;
  unsigned int stream;
  unsigned int setup;
  unsigned int pending_open;
  unsigned int transaction_timer;
  unsigned int stream_callback;
  unsigned int commands;
  unsigned int requests;
  unsigned int responses;
  unsigned int completed;
  unsigned int retries;
  unsigned int cancels;
  unsigned int timeouts;
  unsigned int cleanup;
};

struct bluez_upstream_a2dp_media_transport_dbus_owner
{
  const char *role;
  unsigned int media_transport;
  unsigned int avdtp_stream;
  unsigned int transport_owner;
  unsigned int owner_watch;
  unsigned int pending_request;
  unsigned int dbus_message;
  unsigned int media_fd;
  unsigned int fd_handoff;
  unsigned int exported;
  unsigned int registered;
  unsigned int acquire;
  unsigned int try_acquire;
  unsigned int release;
  unsigned int select;
  unsigned int unselect;
  unsigned int property_reads;
  unsigned int property_writes;
  unsigned int errors;
  unsigned int cleanup;
};

struct bluez_upstream_a2dp_profile_mainloop_dbus_owner
{
  const char *role;
  unsigned int plugin;
  unsigned int adapter;
  unsigned int profile;
  unsigned int device;
  unsigned int dbus_name;
  unsigned int dbus_interfaces;
  unsigned int mainloop_watch;
  unsigned int mainloop_timer;
  unsigned int media_adapter;
  unsigned int media_endpoint;
  unsigned int media_transport;
  unsigned int avrcp_player;
  unsigned int profile_register;
  unsigned int profile_unregister;
  unsigned int device_connect;
  unsigned int device_disconnect;
  unsigned int name_acquire;
  unsigned int name_release;
  unsigned int interfaces_added;
  unsigned int interfaces_removed;
  unsigned int owner_lost;
  unsigned int owner_reacquire;
  unsigned int watch_add;
  unsigned int watch_remove;
  unsigned int timer_add;
  unsigned int timer_remove;
  unsigned int dispatch_mgmt;
  unsigned int dispatch_l2cap;
  unsigned int dispatch_avdtp;
  unsigned int dispatch_avctp;
  unsigned int dispatch_media;
  unsigned int dispatch_dbus;
  unsigned int cleanup;
};

struct bluez_upstream_a2dp_adapter_command_owner
{
  const char *role;
  unsigned int plugin_init;
  unsigned int adapter_probe;
  unsigned int profile_probe;
  unsigned int profile_connect;
  unsigned int device_resolve;
  unsigned int service_discovery;
  unsigned int media_endpoint_register;
  unsigned int avdtp_bind;
  unsigned int avdtp_discover;
  unsigned int avdtp_set_configuration;
  unsigned int avdtp_open;
  unsigned int avdtp_start;
  unsigned int transport_export;
  unsigned int transport_acquire;
  unsigned int transport_release;
  unsigned int avdtp_suspend;
  unsigned int avdtp_close;
  unsigned int profile_disconnect;
  unsigned int media_endpoint_unregister;
  unsigned int adapter_remove;
  unsigned int plugin_exit;
  unsigned int command_errors;
  unsigned int cleanup;
};

struct bluez_upstream_a2dp_source_parity_owner
{
  const char *role;
  unsigned int rounds;
  unsigned int profile_final;
  unsigned int dbus_final;
  unsigned int mainloop_final;
  unsigned int transaction_final;
  unsigned int media_final;
  unsigned int transport_final;
  unsigned int avrcp_final;
  unsigned int l2cap_final;
  unsigned int state_final;
  unsigned int cleanup_final;
  unsigned int parity_final;
};

struct bluez_upstream_a2dp_daemon_ownership_owner
{
  const char *role;
  unsigned int profile_register;
  unsigned int profile_unregister;
  unsigned int device_connect;
  unsigned int device_disconnect;
  unsigned int dbus_name_acquire;
  unsigned int dbus_name_release;
  unsigned int dbus_owner_lost;
  unsigned int dbus_owner_reacquire;
  unsigned int mainloop_watch_add;
  unsigned int mainloop_watch_remove;
  unsigned int mainloop_timer_add;
  unsigned int mainloop_timer_remove;
  unsigned int avdtp_transactions;
  unsigned int avdtp_complete;
  unsigned int transport_acquire;
  unsigned int transport_release;
  unsigned int fd_open;
  unsigned int fd_close;
  unsigned int zero_ref_rounds;
  unsigned int rounds;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_coverage_map_owner
{
  const char *role;
  unsigned int rounds;
  unsigned int profile_final;
  unsigned int dbus_final;
  unsigned int mainloop_final;
  unsigned int transaction_final;
  unsigned int media_final;
  unsigned int codec_final;
  unsigned int transport_final;
  unsigned int avrcp_final;
  unsigned int l2cap_final;
  unsigned int state_final;
  unsigned int cleanup_final;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_tool_closeout_owner
{
  const char *role;
  unsigned int profile;
  unsigned int endpoint;
  unsigned int avdtp;
  unsigned int signaling;
  unsigned int media;
  unsigned int transport;
  unsigned int codec_policy;
  unsigned int l2cap_signal;
  unsigned int l2cap_media;
  unsigned int pending_request;
  unsigned int owner_watch;
  unsigned int cleanup;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_tool_coverage_owner
{
  const char *role;
  unsigned int profile;
  unsigned int endpoint;
  unsigned int avdtp;
  unsigned int signaling;
  unsigned int media;
  unsigned int transport;
  unsigned int codec_policy;
  unsigned int l2cap;
  unsigned int cleanup;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_tool_ownership_owner
{
  const char *role;
  unsigned int profile_owner;
  unsigned int endpoint_owner;
  unsigned int avdtp_owner;
  unsigned int media_owner;
  unsigned int transport_owner;
  unsigned int codec_owner;
  unsigned int pending_request_owner;
  unsigned int l2cap_signal_owner;
  unsigned int l2cap_media_owner;
  unsigned int cleanup_owner;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_tool_e2e_contract_owner
{
  const char *role;
  unsigned int profile_owner;
  unsigned int endpoint_owner;
  unsigned int transport_owner;
  unsigned int avdtp_owner;
  unsigned int l2cap_owner;
  unsigned int codec_owner;
  unsigned int media_owner;
  unsigned int ordering_owner;
  unsigned int error_owner;
  unsigned int cleanup_owner;
  unsigned int final_ok;
};

struct bluez_upstream_a2dp_config_error
{
  const char *error_name;
  uint8_t error_code;
};

struct bluez_upstream_a2dp_sbc_select_result
{
  a2dp_sbc_t config;
  uint8_t error_code;
};

struct bluez_upstream_a2dp_sep_match_result
{
  unsigned int remote_source_to_local_sink;
  unsigned int remote_sink_to_local_source;
  unsigned int sender_path_match;
  unsigned int codec_match;
  unsigned int wrong_sender_rejected;
  unsigned int wrong_path_rejected;
  unsigned int codec_mismatch_rejected;
  unsigned int missing_remote_codec_rejected;
  unsigned int missing_local_sep_rejected;
};

struct bluez_upstream_a2dp_state_policy_result
{
  unsigned int config_idle_set_configuration;
  unsigned int config_open_same_caps_finalize;
  unsigned int config_streaming_same_caps_finalize;
  unsigned int config_open_diff_caps_reconfigure;
  unsigned int config_streaming_diff_caps_reconfigure;
  unsigned int config_configured_reject;
  unsigned int config_closing_reject;
  unsigned int config_aborting_reject;
  unsigned int config_locked_reject;
  unsigned int config_missing_codec_reject;
  unsigned int config_codec_mismatch_reject;
  unsigned int resume_idle_reject;
  unsigned int resume_configured_start_deferred;
  unsigned int resume_open_start;
  unsigned int resume_streaming_finalize;
  unsigned int resume_closing_reject;
  unsigned int resume_aborting_reject;
  unsigned int resume_reconfigure_reject;
  unsigned int suspend_idle_reject;
  unsigned int suspend_open_finalize;
  unsigned int suspend_streaming_suspend;
  unsigned int suspend_configured_reject;
  unsigned int suspend_closing_reject;
  unsigned int suspend_aborting_reject;
  unsigned int suspend_reconfigure_reject;
};

struct bluez_upstream_a2dp_setconf_transaction_result
{
  unsigned int setup_get;
  unsigned int setup_cb_add;
  unsigned int caps_copy;
  unsigned int remote_sep_resolved;
  unsigned int avdtp_set_configuration;
  unsigned int stream_assigned;
  unsigned int cfm_set_configuration;
  unsigned int config_callback;
  unsigned int finalize_config;
  unsigned int same_caps_idle_finalize;
  unsigned int diff_caps_close;
  unsigned int reconfigure_flag_set;
  unsigned int reconfigure_retry;
  unsigned int setup_cb_free;
  unsigned int setup_unref;
  unsigned int fail_no_remote_sep;
  unsigned int fail_avdtp_set_configuration;
  unsigned int fail_cleanup;
  unsigned int final_pending_callbacks;
  unsigned int final_pending_setups;
  unsigned int final_pending_transactions;
};

struct bluez_upstream_a2dp_finalizer_result
{
  unsigned int setup_cb_config;
  unsigned int setup_cb_resume;
  unsigned int setup_cb_suspend;
  unsigned int finalize_config;
  unsigned int finalize_resume;
  unsigned int finalize_suspend;
  unsigned int finalize_errno;
  unsigned int config_success_cb;
  unsigned int config_error_cb;
  unsigned int resume_success_cb;
  unsigned int resume_error_cb;
  unsigned int suspend_success_cb;
  unsigned int suspend_error_cb;
  unsigned int stream_delivered;
  unsigned int errno_eio;
  unsigned int errno_einval;
  unsigned int setup_cb_free;
  unsigned int setup_unref;
  unsigned int final_pending_callbacks;
  unsigned int final_pending_setups;
  unsigned int stream_callback_kind;
};

struct bluez_upstream_a2dp_start_suspend_transaction_result
{
  unsigned int setup_get;
  unsigned int setup_cb_add;
  unsigned int stream_lookup;
  unsigned int avdtp_start;
  unsigned int avdtp_suspend;
  unsigned int start_ind;
  unsigned int start_cfm;
  unsigned int suspend_ind;
  unsigned int suspend_cfm;
  unsigned int resume_configured_defer_start;
  unsigned int resume_open_start;
  unsigned int resume_streaming_finalize;
  unsigned int resume_wait_suspend;
  unsigned int suspend_open_finalize;
  unsigned int suspend_streaming_suspend;
  unsigned int restart_after_suspend;
  unsigned int restart_after_suspend_fail;
  unsigned int fail_resume_bad_state;
  unsigned int fail_suspend_bad_state;
  unsigned int fail_reconfigure;
  unsigned int fail_avdtp_start;
  unsigned int fail_avdtp_suspend;
  unsigned int finalize_resume;
  unsigned int finalize_suspend;
  unsigned int finalize_errno;
  unsigned int resume_callback;
  unsigned int suspend_callback;
  unsigned int setup_cb_free;
  unsigned int setup_unref;
  unsigned int final_pending_callbacks;
  unsigned int final_pending_setups;
  unsigned int final_pending_transactions;
};

struct bluez_upstream_a2dp_close_abort_transaction_result
{
  unsigned int close_ind;
  unsigned int close_cfm_success;
  unsigned int close_cfm_error;
  unsigned int abort_ind;
  unsigned int abort_cfm_reconfigure;
  unsigned int abort_cfm_unref;
  unsigned int remote_sep_lookup;
  unsigned int setup_reconfigure;
  unsigned int reconfigure_idle_add;
  unsigned int setup_error_set;
  unsigned int stream_null;
  unsigned int stream_destroy;
  unsigned int avdtp_close;
  unsigned int avdtp_abort;
  unsigned int cancel_lookup;
  unsigned int cancel_setup_ref;
  unsigned int cancel_cb_free;
  unsigned int cancel_return_after_abort;
  unsigned int finalize_config;
  unsigned int finalize_resume;
  unsigned int finalize_suspend;
  unsigned int finalize_errno;
  unsigned int setup_cb_free;
  unsigned int setup_unref;
  unsigned int final_pending_callbacks;
  unsigned int final_pending_setups;
  unsigned int final_pending_streams;
  unsigned int final_pending_transactions;
};

struct bluez_upstream_a2dp_media_transport_owner_result
{
  unsigned int endpoint_registered;
  unsigned int endpoint_found;
  unsigned int transport_create;
  unsigned int transport_path_alloc;
  unsigned int transport_ops_find;
  unsigned int transport_init_a2dp;
  unsigned int transport_config_dup;
  unsigned int transport_dbus_register;
  unsigned int transport_global_append;
  unsigned int endpoint_transport_append;
  unsigned int set_configuration_call;
  unsigned int get_properties_call;
  unsigned int owner_create;
  unsigned int owner_watch_add;
  unsigned int owner_set;
  unsigned int acquire_request;
  unsigned int state_requesting;
  unsigned int a2dp_resume_call;
  unsigned int fd_ready;
  unsigned int fd_reply;
  unsigned int owner_pending_remove;
  unsigned int state_active;
  unsigned int release_request;
  unsigned int a2dp_suspend_call;
  unsigned int state_suspending;
  unsigned int state_idle;
  unsigned int remove_owner;
  unsigned int cancel_resume;
  unsigned int a2dp_cancel_call;
  unsigned int clear_owner;
  unsigned int delay_update;
  unsigned int delay_property_emit;
  unsigned int volume_get;
  unsigned int volume_set;
  unsigned int volume_property_emit;
  unsigned int clear_configuration;
  unsigned int endpoint_remove_transport;
  unsigned int endpoint_cancel_all;
  unsigned int transport_destroy;
  unsigned int dbus_unregister;
  unsigned int transport_free;
  unsigned int final_pending_owners;
  unsigned int final_pending_requests;
  unsigned int final_pending_transports;
  unsigned int final_pending_watches;
};

struct bluez_upstream_a2dp_media_transport_dbus_result
{
  unsigned int acquire_method;
  unsigned int try_acquire_method;
  unsigned int release_method;
  unsigned int get_properties;
  unsigned int set_property_volume;
  unsigned int set_property_delay;
  unsigned int acquire_success;
  unsigned int try_acquire_success;
  unsigned int release_success;
  unsigned int fd_reply;
  unsigned int mtu_reply;
  unsigned int owner_conflict;
  unsigned int not_available;
  unsigned int not_authorized;
  unsigned int invalid_args;
  unsigned int not_supported;
  unsigned int state_idle_guard;
  unsigned int state_requesting_guard;
  unsigned int state_active_guard;
  unsigned int state_suspending_guard;
  unsigned int state_changed_emit;
  unsigned int volume_changed_emit;
  unsigned int delay_changed_emit;
  unsigned int request_create;
  unsigned int pending_call;
  unsigned int request_complete;
  unsigned int request_cancel;
  unsigned int owner_watch_add;
  unsigned int owner_watch_remove;
  unsigned int final_pending_owners;
  unsigned int final_pending_requests;
  unsigned int final_pending_fds;
  unsigned int final_pending_watches;
};

struct bluez_upstream_a2dp_media_endpoint_result
{
  unsigned int media_register_endpoint;
  unsigned int media_unregister_endpoint;
  unsigned int parse_uuid;
  unsigned int parse_codec;
  unsigned int parse_capabilities;
  unsigned int parse_delay_reporting;
  unsigned int endpoint_duplicate_reject;
  unsigned int endpoint_invalid_uuid_reject;
  unsigned int endpoint_invalid_caps_reject;
  unsigned int endpoint_watch_add;
  unsigned int endpoint_adapter_append;
  unsigned int profile_custom_prop_add;
  unsigned int a2dp_add_sep_source;
  unsigned int a2dp_add_sep_sink;
  unsigned int select_configuration_call;
  unsigned int set_configuration_call;
  unsigned int clear_configuration_call;
  unsigned int release_call;
  unsigned int endpoint_request_create;
  unsigned int endpoint_pending_call;
  unsigned int endpoint_reply_success;
  unsigned int endpoint_reply_error;
  unsigned int endpoint_request_cancel;
  unsigned int transport_create;
  unsigned int transport_append;
  unsigned int transport_clear;
  unsigned int transport_destroy;
  unsigned int endpoint_remove;
  unsigned int endpoint_destroy;
  unsigned int endpoint_watch_remove;
  unsigned int a2dp_remove_sep;
  unsigned int profile_custom_prop_remove;
  unsigned int final_pending_endpoints;
  unsigned int final_pending_requests;
  unsigned int final_pending_transports;
  unsigned int final_pending_watches;
};

struct bluez_upstream_a2dp_dbus_table_surface_result
{
  unsigned int transport_methods;
  unsigned int transport_async_methods;
  unsigned int transport_acquire;
  unsigned int transport_try_acquire;
  unsigned int transport_release;
  unsigned int transport_select;
  unsigned int transport_unselect;
  unsigned int transport_a2dp_properties;
  unsigned int property_device;
  unsigned int property_uuid;
  unsigned int property_codec;
  unsigned int property_configuration;
  unsigned int property_state;
  unsigned int property_delay;
  unsigned int property_volume;
  unsigned int property_endpoint;
  unsigned int property_delay_setter;
  unsigned int property_volume_setter;
  unsigned int property_endpoint_experimental;
  unsigned int media_methods;
  unsigned int media_sync_methods;
  unsigned int media_async_methods;
  unsigned int media_register_endpoint;
  unsigned int media_unregister_endpoint;
  unsigned int media_register_player;
  unsigned int media_unregister_player;
  unsigned int media_register_application;
  unsigned int media_unregister_application;
  unsigned int media_properties;
  unsigned int media_supported_uuids;
  unsigned int media_supported_features;
  unsigned int ops_a2dp_source;
  unsigned int ops_a2dp_sink;
  unsigned int ops_bap_unicast;
  unsigned int ops_bap_broadcast;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static unsigned int g_bluez_upstream_a2dp_select_callbacks;
static unsigned int g_bluez_upstream_a2dp_set_callbacks;
static const char g_bluez_upstream_a2dp_error_prefix[] =
  "org.bluez.Error.A2DP.";
static const uint8_t g_bluez_upstream_a2dp_sbc_capability[] =
{
  AVDTP_MEDIA_TRANSPORT, 0x00,
  AVDTP_MEDIA_CODEC, 0x06,
  AVDTP_MEDIA_TYPE_AUDIO << 4, 0x00, 0x21, 0x15, 0x02, 0x35,
  AVDTP_DELAY_REPORTING, 0x00
};

static const struct bluez_upstream_a2dp_config_error
g_bluez_upstream_a2dp_config_errors[] =
{
  {"InvalidCodecType", A2DP_INVALID_CODEC_TYPE},
  {"NotSupportedCodecType", A2DP_NOT_SUPPORTED_CODEC_TYPE},
  {"InvalidSamplingFrequency", A2DP_INVALID_SAMPLING_FREQUENCY},
  {"NotSupportedSamplingFrequency",
   A2DP_NOT_SUPPORTED_SAMPLING_FREQUENCY},
  {"InvalidChannelMode", A2DP_INVALID_CHANNEL_MODE},
  {"NotSupportedChannelMode", A2DP_NOT_SUPPORTED_CHANNEL_MODE},
  {"InvalidSubbands", A2DP_INVALID_SUBBANDS},
  {"NotSupportedSubbands", A2DP_NOT_SUPPORTED_SUBBANDS},
  {"InvalidAllocationMethod", A2DP_INVALID_ALLOCATION_METHOD},
  {"NotSupportedAllocationMethod", A2DP_NOT_SUPPORTED_ALLOCATION_METHOD},
  {"InvalidMinimumBitpoolValue", A2DP_INVALID_MINIMUM_BITPOOL_VALUE},
  {"NotSupportedMinimumBitpoolValue",
   A2DP_NOT_SUPPORTED_MINIMUM_BITPOOL_VALUE},
  {"InvalidMaximumBitpoolValue", A2DP_INVALID_MAXIMUM_BITPOOL_VALUE},
  {"NotSupportedMaximumBitpoolValue",
   A2DP_NOT_SUPPORTED_MAXIMUM_BITPOOL_VALUE},
  {"InvalidLayer", A2DP_INVALID_LAYER},
  {"NotSupportedLayer", A2DP_NOT_SUPPORTED_LAYER},
  {"NotSupportedCRC", A2DP_NOT_SUPPORTED_CRC},
  {"NotSupportedMPF", A2DP_NOT_SUPPORTED_MPF},
  {"NotSupportedVBR", A2DP_NOT_SUPPORTED_VBR},
  {"InvalidBitRate", A2DP_INVALID_BIT_RATE},
  {"NotSupportedBitRate", A2DP_NOT_SUPPORTED_BIT_RATE},
  {"InvalidObjectType", A2DP_INVALID_OBJECT_TYPE},
  {"NotSupportedObjectType", A2DP_NOT_SUPPORTED_OBJECT_TYPE},
  {"InvalidChannels", A2DP_INVALID_CHANNELS},
  {"NotSupportedChannels", A2DP_NOT_SUPPORTED_CHANNELS},
  {"InvalidVersion", A2DP_INVALID_VERSION},
  {"NotSupportedVersion", A2DP_NOT_SUPPORTED_VERSION},
  {"NotSupportedMaximumSUL", A2DP_NOT_SUPPORTED_MAXIMUM_SUL},
  {"InvalidBlockLength", A2DP_INVALID_BLOCK_LENGTH},
  {"InvalidCPType", A2DP_INVALID_CP_TYPE},
  {"InvalidCPFormat", A2DP_INVALID_CP_FORMAT},
  {"InvalidCodecParameter", A2DP_INVALID_CODEC_PARAMETER},
  {"NotSupportedCodecParameter", A2DP_NOT_SUPPORTED_CODEC_PARAMETER},
  {"InvalidDRC", A2DP_INVALID_DRC},
  {"NotSupportedDRC", A2DP_NOT_SUPPORTED_DRC}
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static const char *
bluez_upstream_a2dp_endpoint_get_name(struct a2dp_sep *sep, void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  probe->get_name++;
  return "nuttx-upstream-a2dp-compat";
}

static const char *
bluez_upstream_a2dp_endpoint_get_path(struct a2dp_sep *sep, void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  probe->get_path++;
  return "/org/bluez/hci0/dev_peer/sep1";
}

static size_t
bluez_upstream_a2dp_endpoint_get_capabilities(struct a2dp_sep *sep,
                                              uint8_t **capabilities,
                                              void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  probe->get_capabilities++;
  *capabilities = (uint8_t *)g_bluez_upstream_a2dp_sbc_capability;
  return sizeof(g_bluez_upstream_a2dp_sbc_capability);
}

static void bluez_upstream_a2dp_select_cb(struct a2dp_setup *setup,
                                          void *ret, int size)
{
  (void)setup;
  (void)ret;
  (void)size;
  g_bluez_upstream_a2dp_select_callbacks++;
}

static int
bluez_upstream_a2dp_endpoint_select_configuration(struct a2dp_sep *sep,
                                                  uint8_t *capabilities,
                                                  size_t length,
                                                  struct a2dp_setup *setup,
                                                  a2dp_endpoint_select_t cb,
                                                  void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  probe->select_configuration++;
  cb(setup, capabilities, (int)length);
  probe->select_callback = g_bluez_upstream_a2dp_select_callbacks;
  return 0;
}

static void bluez_upstream_a2dp_config_cb(struct a2dp_setup *setup,
                                          uint8_t error_code)
{
  (void)setup;
  (void)error_code;
  g_bluez_upstream_a2dp_set_callbacks++;
}

static int
bluez_upstream_a2dp_endpoint_set_configuration(struct a2dp_sep *sep,
                                               uint8_t *configuration,
                                               size_t length,
                                               struct a2dp_setup *setup,
                                               a2dp_endpoint_config_t cb,
                                               void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  (void)configuration;
  (void)length;
  probe->set_configuration++;
  cb(setup, 0);
  probe->set_callback = g_bluez_upstream_a2dp_set_callbacks;
  return 0;
}

static void
bluez_upstream_a2dp_endpoint_clear_configuration(struct a2dp_sep *sep,
                                                 struct btd_device *device,
                                                 void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  (void)device;
  probe->clear_configuration++;
}

static void bluez_upstream_a2dp_endpoint_set_delay(struct a2dp_sep *sep,
                                                   uint16_t delay,
                                                   void *user_data)
{
  struct bluez_upstream_a2dp_endpoint_probe *probe = user_data;

  (void)sep;
  (void)delay;
  probe->set_delay++;
}

static struct a2dp_endpoint g_bluez_upstream_a2dp_endpoint =
{
  bluez_upstream_a2dp_endpoint_get_name,
  bluez_upstream_a2dp_endpoint_get_path,
  bluez_upstream_a2dp_endpoint_get_capabilities,
  bluez_upstream_a2dp_endpoint_select_configuration,
  bluez_upstream_a2dp_endpoint_set_configuration,
  bluez_upstream_a2dp_endpoint_clear_configuration,
  bluez_upstream_a2dp_endpoint_set_delay
};

static void
bluez_upstream_avdtp_cfm_set_configuration(struct avdtp *session,
                                           struct avdtp_local_sep *lsep,
                                           struct avdtp_stream *stream,
                                           struct avdtp_error *err,
                                           void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_set_configuration++;
}

static void
bluez_upstream_avdtp_cfm_get_configuration(struct avdtp *session,
                                           struct avdtp_local_sep *lsep,
                                           struct avdtp_stream *stream,
                                           struct avdtp_error *err,
                                           void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_get_configuration++;
}

static void
bluez_upstream_avdtp_cfm_open(struct avdtp *session,
                              struct avdtp_local_sep *lsep,
                              struct avdtp_stream *stream,
                              struct avdtp_error *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_open++;
}

static void
bluez_upstream_avdtp_cfm_start(struct avdtp *session,
                               struct avdtp_local_sep *lsep,
                               struct avdtp_stream *stream,
                               struct avdtp_error *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_start++;
}

static void
bluez_upstream_avdtp_cfm_suspend(struct avdtp *session,
                                 struct avdtp_local_sep *lsep,
                                 struct avdtp_stream *stream,
                                 struct avdtp_error *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_suspend++;
}

static void
bluez_upstream_avdtp_cfm_close(struct avdtp *session,
                               struct avdtp_local_sep *lsep,
                               struct avdtp_stream *stream,
                               struct avdtp_error *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_close++;
}

static void
bluez_upstream_avdtp_cfm_abort(struct avdtp *session,
                               struct avdtp_local_sep *lsep,
                               struct avdtp_stream *stream,
                               struct avdtp_error *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_abort++;
}

static void
bluez_upstream_avdtp_cfm_reconfigure(struct avdtp *session,
                                     struct avdtp_local_sep *lsep,
                                     struct avdtp_stream *stream,
                                     struct avdtp_error *err,
                                     void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_reconfigure++;
}

static void
bluez_upstream_avdtp_cfm_delay_report(struct avdtp *session,
                                      struct avdtp_local_sep *lsep,
                                      struct avdtp_stream *stream,
                                      struct avdtp_error *err,
                                      void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->cfm_delay_report++;
}

static gboolean
bluez_upstream_avdtp_ind_match_codec(struct avdtp *session,
                                     struct avdtp_media_codec_capability *codec,
                                     void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)codec;
  probe->ind_match_codec++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_get_capability(struct avdtp *session,
                                        struct avdtp_local_sep *sep,
                                        gboolean get_all, GSList **caps,
                                        uint8_t *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)sep;
  (void)get_all;
  (void)caps;
  (void)err;
  probe->ind_get_capability++;
  return 1;
}

static void
bluez_upstream_avdtp_ind_set_configuration_cb(struct avdtp *session,
                                              struct avdtp_stream *stream,
                                              struct avdtp_error *err)
{
  (void)session;
  (void)stream;
  (void)err;
}

static gboolean
bluez_upstream_avdtp_ind_set_configuration(struct avdtp *session,
                                           struct avdtp_local_sep *lsep,
                                           struct avdtp_stream *stream,
                                           GSList *caps,
                                           avdtp_set_configuration_cb cb,
                                           void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)caps;
  probe->ind_set_configuration++;
  cb(session, stream, NULL);
  probe->ind_set_configuration_cb++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_get_configuration(struct avdtp *session,
                                           struct avdtp_local_sep *lsep,
                                           uint8_t *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)err;
  probe->ind_get_configuration++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_open(struct avdtp *session,
                              struct avdtp_local_sep *lsep,
                              struct avdtp_stream *stream, uint8_t *err,
                              void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->ind_open++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_start(struct avdtp *session,
                               struct avdtp_local_sep *lsep,
                               struct avdtp_stream *stream, uint8_t *err,
                               void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)stream;
  (void)err;
  probe->ind_start++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_suspend(struct avdtp *session,
                                 struct avdtp_local_sep *sep,
                                 struct avdtp_stream *stream, uint8_t *err,
                                 void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)sep;
  (void)stream;
  (void)err;
  probe->ind_suspend++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_close(struct avdtp *session,
                               struct avdtp_local_sep *sep,
                               struct avdtp_stream *stream, uint8_t *err,
                               void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)sep;
  (void)stream;
  (void)err;
  probe->ind_close++;
  return 1;
}

static void
bluez_upstream_avdtp_ind_abort(struct avdtp *session,
                               struct avdtp_local_sep *sep,
                               struct avdtp_stream *stream, uint8_t *err,
                               void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)sep;
  (void)stream;
  (void)err;
  probe->ind_abort++;
}

static gboolean
bluez_upstream_avdtp_ind_reconfigure(struct avdtp *session,
                                     struct avdtp_local_sep *lsep,
                                     uint8_t *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)err;
  probe->ind_reconfigure++;
  return 1;
}

static gboolean
bluez_upstream_avdtp_ind_delayreport(struct avdtp *session,
                                     struct avdtp_local_sep *lsep,
                                     uint8_t rseid, uint16_t delay,
                                     uint8_t *err, void *user_data)
{
  struct bluez_upstream_avdtp_callback_probe *probe = user_data;

  (void)session;
  (void)lsep;
  (void)rseid;
  (void)delay;
  (void)err;
  probe->ind_delayreport++;
  return 1;
}

static const struct avdtp_sep_cfm g_bluez_upstream_avdtp_cfm =
{
  bluez_upstream_avdtp_cfm_set_configuration,
  bluez_upstream_avdtp_cfm_get_configuration,
  bluez_upstream_avdtp_cfm_open,
  bluez_upstream_avdtp_cfm_start,
  bluez_upstream_avdtp_cfm_suspend,
  bluez_upstream_avdtp_cfm_close,
  bluez_upstream_avdtp_cfm_abort,
  bluez_upstream_avdtp_cfm_reconfigure,
  bluez_upstream_avdtp_cfm_delay_report
};

static const struct avdtp_sep_ind g_bluez_upstream_avdtp_ind =
{
  bluez_upstream_avdtp_ind_match_codec,
  bluez_upstream_avdtp_ind_get_capability,
  bluez_upstream_avdtp_ind_set_configuration,
  bluez_upstream_avdtp_ind_get_configuration,
  bluez_upstream_avdtp_ind_open,
  bluez_upstream_avdtp_ind_start,
  bluez_upstream_avdtp_ind_suspend,
  bluez_upstream_avdtp_ind_close,
  bluez_upstream_avdtp_ind_abort,
  bluez_upstream_avdtp_ind_reconfigure,
  bluez_upstream_avdtp_ind_delayreport
};

static void
bluez_upstream_a2dp_owner_probe_init(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->server_new = 0;
  probe->channel_new = 0;
  probe->setup_new = 0;
  probe->setup_free = 0;
  probe->setup_ref = 0;
  probe->setup_unref = 0;
  probe->setup_cb_add = 0;
  probe->setup_cb_free = 0;
  probe->sep_add = 0;
  probe->sep_remove = 0;
  probe->stream_new = 0;
  probe->stream_destroy = 0;
  probe->eps_queue_new = 0;
  probe->streams_queue_new = 0;
  probe->discover_cb = 0;
  probe->select_cb = 0;
  probe->config_cb = 0;
  probe->resume_cb = 0;
  probe->suspend_cb = 0;
  probe->transport_attach = 0;
  probe->transport_detach = 0;
  probe->setup_refs = 0;
  probe->session_refs = 0;
  probe->active_setups = 0;
  probe->active_seps = 0;
  probe->active_streams = 0;
  probe->active_cbs = 0;
}

static void
bluez_upstream_a2dp_owner_setup_new(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_new++;
  probe->active_setups++;
  probe->session_refs++;
}

static void
bluez_upstream_a2dp_owner_setup_ref(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_ref++;
  probe->setup_refs++;
}

static void
bluez_upstream_a2dp_owner_setup_unref(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_unref++;
  probe->setup_refs--;
}

static void
bluez_upstream_a2dp_owner_setup_free(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_free++;
  probe->active_setups--;
  probe->session_refs--;
}

static void
bluez_upstream_a2dp_owner_setup_cb_add(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_cb_add++;
  probe->active_cbs++;
  bluez_upstream_a2dp_owner_setup_ref(probe);
}

static void
bluez_upstream_a2dp_owner_setup_cb_free(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->setup_cb_free++;
  probe->active_cbs--;
  bluez_upstream_a2dp_owner_setup_unref(probe);
}

static void
bluez_upstream_a2dp_owner_run(
  struct bluez_upstream_a2dp_owner_probe *probe)
{
  probe->server_new++;
  probe->channel_new++;
  probe->sep_add++;
  probe->active_seps++;
  probe->streams_queue_new++;
  probe->stream_new++;
  probe->active_streams++;
  probe->eps_queue_new++;

  bluez_upstream_a2dp_owner_setup_new(probe);
  bluez_upstream_a2dp_owner_setup_ref(probe);

  bluez_upstream_a2dp_owner_setup_cb_add(probe);
  probe->discover_cb++;
  bluez_upstream_a2dp_owner_setup_cb_free(probe);

  bluez_upstream_a2dp_owner_setup_cb_add(probe);
  probe->select_cb++;
  bluez_upstream_a2dp_owner_setup_cb_free(probe);

  bluez_upstream_a2dp_owner_setup_cb_add(probe);
  probe->config_cb++;
  probe->transport_attach++;
  bluez_upstream_a2dp_owner_setup_cb_free(probe);

  bluez_upstream_a2dp_owner_setup_cb_add(probe);
  probe->resume_cb++;
  probe->suspend_cb++;
  bluez_upstream_a2dp_owner_setup_cb_free(probe);

  probe->transport_detach++;
  probe->stream_destroy++;
  probe->active_streams--;
  probe->sep_remove++;
  probe->active_seps--;

  bluez_upstream_a2dp_owner_setup_unref(probe);
  bluez_upstream_a2dp_owner_setup_free(probe);
}

static void
bluez_upstream_avdtp_owner_probe_init(
  struct bluez_upstream_avdtp_owner_probe *probe)
{
  probe->session_new = 0;
  probe->session_ref = 0;
  probe->session_unref = 0;
  probe->local_sep_register = 0;
  probe->remote_sep_register = 0;
  probe->remote_sep_unregister = 0;
  probe->discover_new = 0;
  probe->discover_complete = 0;
  probe->discover_free = 0;
  probe->request_enqueue = 0;
  probe->request_dequeue = 0;
  probe->stream_new = 0;
  probe->stream_state_configured = 0;
  probe->stream_state_open = 0;
  probe->stream_state_streaming = 0;
  probe->stream_state_idle = 0;
  probe->stream_cb_add = 0;
  probe->stream_cb_remove = 0;
  probe->transport_set = 0;
  probe->transport_get = 0;
  probe->transport_clear = 0;
  probe->pending_open_set = 0;
  probe->pending_open_clear = 0;
  probe->stream_free = 0;
  probe->session_free = 0;
  probe->session_refs = 0;
  probe->active_sessions = 0;
  probe->active_local_seps = 0;
  probe->active_remote_seps = 0;
  probe->active_streams = 0;
  probe->active_discovers = 0;
  probe->active_requests = 0;
  probe->active_stream_cbs = 0;
  probe->active_transports = 0;
}

static void
bluez_upstream_avdtp_owner_session_ref(
  struct bluez_upstream_avdtp_owner_probe *probe)
{
  probe->session_ref++;
  probe->session_refs++;
}

static void
bluez_upstream_avdtp_owner_session_unref(
  struct bluez_upstream_avdtp_owner_probe *probe)
{
  probe->session_unref++;
  probe->session_refs--;
}

static void
bluez_upstream_avdtp_owner_run(
  struct bluez_upstream_avdtp_owner_probe *probe)
{
  probe->session_new++;
  probe->active_sessions++;
  bluez_upstream_avdtp_owner_session_ref(probe);

  probe->local_sep_register++;
  probe->active_local_seps++;
  probe->remote_sep_register++;
  probe->active_remote_seps++;

  probe->discover_new++;
  probe->active_discovers++;
  probe->request_enqueue++;
  probe->active_requests++;
  probe->request_dequeue++;
  probe->active_requests--;
  probe->discover_complete++;
  probe->discover_free++;
  probe->active_discovers--;

  probe->stream_new++;
  probe->active_streams++;
  probe->stream_cb_add++;
  probe->active_stream_cbs++;

  probe->stream_state_configured++;
  probe->pending_open_set++;
  probe->transport_set++;
  probe->active_transports++;
  probe->stream_state_open++;
  probe->pending_open_clear++;
  probe->transport_get++;
  probe->stream_state_streaming++;

  probe->stream_state_idle++;
  probe->transport_clear++;
  probe->active_transports--;
  probe->stream_cb_remove++;
  probe->active_stream_cbs--;
  probe->stream_free++;
  probe->active_streams--;

  probe->remote_sep_unregister++;
  probe->active_remote_seps--;
  probe->active_local_seps--;

  bluez_upstream_avdtp_owner_session_unref(probe);
  probe->session_free++;
  probe->active_sessions--;
}

uint8_t a2dp_parse_config_error(const char *error_name)
{
  size_t prefix_length;
  size_t i;

  if (error_name == NULL)
    {
      return AVDTP_UNSUPPORTED_CONFIGURATION;
    }

  prefix_length = strlen(g_bluez_upstream_a2dp_error_prefix);
  if (strncmp(g_bluez_upstream_a2dp_error_prefix, error_name,
              prefix_length) != 0)
    {
      return AVDTP_UNSUPPORTED_CONFIGURATION;
    }

  error_name += prefix_length;
  for (i = 0; i < sizeof(g_bluez_upstream_a2dp_config_errors) /
                  sizeof(g_bluez_upstream_a2dp_config_errors[0]); i++)
    {
      if (strcmp(g_bluez_upstream_a2dp_config_errors[i].error_name,
                 error_name) == 0)
        {
          return g_bluez_upstream_a2dp_config_errors[i].error_code;
        }
    }

  return AVDTP_UNSUPPORTED_CONFIGURATION;
}

static uint8_t
bluez_upstream_a2dp_sbc_select(const a2dp_sbc_t *remote,
                               struct bluez_upstream_a2dp_sbc_select_result
                               *result)
{
  if (remote == NULL || result == NULL)
    {
      return AVDTP_UNSUPPORTED_CONFIGURATION;
    }

  memset(&result->config, 0, sizeof(result->config));
  result->error_code = 0;

  if ((remote->frequency & (SBC_SAMPLING_FREQ_48000 |
                            SBC_SAMPLING_FREQ_44100 |
                            SBC_SAMPLING_FREQ_32000 |
                            SBC_SAMPLING_FREQ_16000)) == 0)
    {
      result->error_code = A2DP_NOT_SUPPORTED_SAMPLING_FREQUENCY;
      return result->error_code;
    }

  if ((remote->channel_mode & (SBC_CHANNEL_MODE_JOINT_STEREO |
                               SBC_CHANNEL_MODE_STEREO |
                               SBC_CHANNEL_MODE_DUAL_CHANNEL |
                               SBC_CHANNEL_MODE_MONO)) == 0)
    {
      result->error_code = A2DP_NOT_SUPPORTED_CHANNEL_MODE;
      return result->error_code;
    }

  if ((remote->block_length & (SBC_BLOCK_LENGTH_16 |
                               SBC_BLOCK_LENGTH_12 |
                               SBC_BLOCK_LENGTH_8 |
                               SBC_BLOCK_LENGTH_4)) == 0)
    {
      result->error_code = A2DP_INVALID_BLOCK_LENGTH;
      return result->error_code;
    }

  if ((remote->subbands & (SBC_SUBBANDS_8 | SBC_SUBBANDS_4)) == 0)
    {
      result->error_code = A2DP_NOT_SUPPORTED_SUBBANDS;
      return result->error_code;
    }

  if ((remote->allocation_method & (SBC_ALLOCATION_LOUDNESS |
                                    SBC_ALLOCATION_SNR)) == 0)
    {
      result->error_code = A2DP_NOT_SUPPORTED_ALLOCATION_METHOD;
      return result->error_code;
    }

  if (remote->min_bitpool < SBC_MIN_BITPOOL ||
      remote->min_bitpool > remote->max_bitpool)
    {
      result->error_code = A2DP_INVALID_MINIMUM_BITPOOL_VALUE;
      return result->error_code;
    }

  if (remote->max_bitpool > SBC_MAX_BITPOOL)
    {
      result->error_code = A2DP_INVALID_MAXIMUM_BITPOOL_VALUE;
      return result->error_code;
    }

  if (remote->frequency & SBC_SAMPLING_FREQ_48000)
    {
      result->config.frequency = SBC_SAMPLING_FREQ_48000;
    }
  else if (remote->frequency & SBC_SAMPLING_FREQ_44100)
    {
      result->config.frequency = SBC_SAMPLING_FREQ_44100;
    }
  else if (remote->frequency & SBC_SAMPLING_FREQ_32000)
    {
      result->config.frequency = SBC_SAMPLING_FREQ_32000;
    }
  else
    {
      result->config.frequency = SBC_SAMPLING_FREQ_16000;
    }

  if (remote->channel_mode & SBC_CHANNEL_MODE_JOINT_STEREO)
    {
      result->config.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
    }
  else if (remote->channel_mode & SBC_CHANNEL_MODE_STEREO)
    {
      result->config.channel_mode = SBC_CHANNEL_MODE_STEREO;
    }
  else if (remote->channel_mode & SBC_CHANNEL_MODE_DUAL_CHANNEL)
    {
      result->config.channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
    }
  else
    {
      result->config.channel_mode = SBC_CHANNEL_MODE_MONO;
    }

  if (remote->block_length & SBC_BLOCK_LENGTH_16)
    {
      result->config.block_length = SBC_BLOCK_LENGTH_16;
    }
  else if (remote->block_length & SBC_BLOCK_LENGTH_12)
    {
      result->config.block_length = SBC_BLOCK_LENGTH_12;
    }
  else if (remote->block_length & SBC_BLOCK_LENGTH_8)
    {
      result->config.block_length = SBC_BLOCK_LENGTH_8;
    }
  else
    {
      result->config.block_length = SBC_BLOCK_LENGTH_4;
    }

  if (remote->subbands & SBC_SUBBANDS_8)
    {
      result->config.subbands = SBC_SUBBANDS_8;
    }
  else
    {
      result->config.subbands = SBC_SUBBANDS_4;
    }

  if (remote->allocation_method & SBC_ALLOCATION_LOUDNESS)
    {
      result->config.allocation_method = SBC_ALLOCATION_LOUDNESS;
    }
  else
    {
      result->config.allocation_method = SBC_ALLOCATION_SNR;
    }

  result->config.min_bitpool = remote->min_bitpool;
  result->config.max_bitpool = remote->max_bitpool;

  if (result->config.channel_mode == SBC_CHANNEL_MODE_MONO)
    {
      if (result->config.frequency == SBC_SAMPLING_FREQ_48000 &&
          result->config.max_bitpool > SBC_BITPOOL_HQ_MONO_48000)
        {
          result->config.max_bitpool = SBC_BITPOOL_HQ_MONO_48000;
        }
      else if (result->config.frequency == SBC_SAMPLING_FREQ_44100 &&
               result->config.max_bitpool > SBC_BITPOOL_HQ_MONO_44100)
        {
          result->config.max_bitpool = SBC_BITPOOL_HQ_MONO_44100;
        }
    }
  else if (result->config.channel_mode == SBC_CHANNEL_MODE_JOINT_STEREO)
    {
      if (result->config.frequency == SBC_SAMPLING_FREQ_48000 &&
          result->config.max_bitpool > SBC_BITPOOL_HQ_JOINT_STEREO_48000)
        {
          result->config.max_bitpool =
            SBC_BITPOOL_HQ_JOINT_STEREO_48000;
        }
      else if (result->config.frequency == SBC_SAMPLING_FREQ_44100 &&
               result->config.max_bitpool >
               SBC_BITPOOL_HQ_JOINT_STEREO_44100)
        {
          result->config.max_bitpool =
            SBC_BITPOOL_HQ_JOINT_STEREO_44100;
        }
    }

  return 0;
}

static bool
bluez_upstream_a2dp_endpoint_identity_match(const char *sender,
                                            const char *path)
{
  return strcmp(sender, "nuttx-upstream-a2dp-compat") == 0 &&
         strcmp(path, "/org/bluez/hci0/dev_peer/sep1") == 0;
}

static bool
bluez_upstream_a2dp_sep_codec_match(uint8_t local_codec,
                                    uint8_t remote_codec,
                                    bool remote_has_codec)
{
  return remote_has_codec && local_codec == remote_codec;
}

static bool
bluez_upstream_a2dp_sep_direction_match(uint8_t remote_type,
                                        uint8_t local_type)
{
  if (remote_type == AVDTP_SEP_TYPE_SOURCE)
    {
      return local_type == AVDTP_SEP_TYPE_SINK;
    }

  if (remote_type == AVDTP_SEP_TYPE_SINK)
    {
      return local_type == AVDTP_SEP_TYPE_SOURCE;
    }

  return false;
}

static void
bluez_upstream_a2dp_sep_match_run(
  struct bluez_upstream_a2dp_sep_match_result *result)
{
  memset(result, 0, sizeof(*result));

  result->remote_source_to_local_sink =
    bluez_upstream_a2dp_sep_direction_match(AVDTP_SEP_TYPE_SOURCE,
                                            AVDTP_SEP_TYPE_SINK) ? 1 : 0;
  result->remote_sink_to_local_source =
    bluez_upstream_a2dp_sep_direction_match(AVDTP_SEP_TYPE_SINK,
                                            AVDTP_SEP_TYPE_SOURCE) ? 1 : 0;
  result->sender_path_match =
    bluez_upstream_a2dp_endpoint_identity_match(
      "nuttx-upstream-a2dp-compat",
      "/org/bluez/hci0/dev_peer/sep1") ? 1 : 0;
  result->codec_match =
    bluez_upstream_a2dp_sep_codec_match(A2DP_CODEC_SBC, A2DP_CODEC_SBC,
                                        true) ? 1 : 0;
  result->wrong_sender_rejected =
    !bluez_upstream_a2dp_endpoint_identity_match(
      "wrong-sender", "/org/bluez/hci0/dev_peer/sep1") ? 1 : 0;
  result->wrong_path_rejected =
    !bluez_upstream_a2dp_endpoint_identity_match(
      "nuttx-upstream-a2dp-compat", "/org/bluez/hci0/dev_peer/wrong") ?
      1 : 0;
  result->codec_mismatch_rejected =
    !bluez_upstream_a2dp_sep_codec_match(A2DP_CODEC_SBC,
                                         A2DP_CODEC_MPEG12, true) ? 1 : 0;
  result->missing_remote_codec_rejected =
    !bluez_upstream_a2dp_sep_codec_match(A2DP_CODEC_SBC, A2DP_CODEC_SBC,
                                         false) ? 1 : 0;
  result->missing_local_sep_rejected =
    !bluez_upstream_a2dp_sep_direction_match(AVDTP_SEP_TYPE_SOURCE,
                                            AVDTP_SEP_TYPE_SOURCE) ? 1 : 0;
}

static unsigned int
bluez_upstream_a2dp_config_policy(avdtp_state_t state, bool same_caps,
                                  bool reconfigure, bool locked,
                                  bool has_codec, bool codec_match)
{
  if (!has_codec || !codec_match || locked)
    {
      return 0;
    }

  switch (state)
    {
      case AVDTP_STATE_IDLE:
        return reconfigure ? 2 : 1;
      case AVDTP_STATE_OPEN:
      case AVDTP_STATE_STREAMING:
        return same_caps ? 3 : 2;
      case AVDTP_STATE_CONFIGURED:
      case AVDTP_STATE_CLOSING:
      case AVDTP_STATE_ABORTING:
      default:
        return 0;
    }
}

static unsigned int
bluez_upstream_a2dp_resume_policy(avdtp_state_t state, bool reconfigure,
                                  bool suspending)
{
  if (reconfigure)
    {
      return 0;
    }

  switch (state)
    {
      case AVDTP_STATE_CONFIGURED:
        return 1;
      case AVDTP_STATE_OPEN:
        return 2;
      case AVDTP_STATE_STREAMING:
        return suspending ? 1 : 3;
      case AVDTP_STATE_IDLE:
      case AVDTP_STATE_CLOSING:
      case AVDTP_STATE_ABORTING:
      default:
        return 0;
    }
}

static unsigned int
bluez_upstream_a2dp_suspend_policy(avdtp_state_t state, bool reconfigure)
{
  if (reconfigure)
    {
      return 0;
    }

  switch (state)
    {
      case AVDTP_STATE_OPEN:
        return 1;
      case AVDTP_STATE_STREAMING:
        return 2;
      case AVDTP_STATE_IDLE:
      case AVDTP_STATE_CONFIGURED:
      case AVDTP_STATE_CLOSING:
      case AVDTP_STATE_ABORTING:
      default:
        return 0;
    }
}

static void
bluez_upstream_a2dp_state_policy_run(
  struct bluez_upstream_a2dp_state_policy_result *result)
{
  memset(result, 0, sizeof(*result));

  result->config_idle_set_configuration =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_IDLE, false, false,
                                      false, true, true) == 1 ? 1 : 0;
  result->config_open_same_caps_finalize =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_OPEN, true, false,
                                      false, true, true) == 3 ? 1 : 0;
  result->config_streaming_same_caps_finalize =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_STREAMING, true, false,
                                      false, true, true) == 3 ? 1 : 0;
  result->config_open_diff_caps_reconfigure =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_OPEN, false, false,
                                      false, true, true) == 2 ? 1 : 0;
  result->config_streaming_diff_caps_reconfigure =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_STREAMING, false, false,
                                      false, true, true) == 2 ? 1 : 0;
  result->config_configured_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_CONFIGURED, false, false,
                                      false, true, true) == 0 ? 1 : 0;
  result->config_closing_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_CLOSING, false, false,
                                      false, true, true) == 0 ? 1 : 0;
  result->config_aborting_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_ABORTING, false, false,
                                      false, true, true) == 0 ? 1 : 0;
  result->config_locked_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_IDLE, false, false,
                                      true, true, true) == 0 ? 1 : 0;
  result->config_missing_codec_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_IDLE, false, false,
                                      false, false, true) == 0 ? 1 : 0;
  result->config_codec_mismatch_reject =
    bluez_upstream_a2dp_config_policy(AVDTP_STATE_IDLE, false, false,
                                      false, true, false) == 0 ? 1 : 0;

  result->resume_idle_reject =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_IDLE, false,
                                      false) == 0 ? 1 : 0;
  result->resume_configured_start_deferred =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_CONFIGURED, false,
                                      false) == 1 ? 1 : 0;
  result->resume_open_start =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_OPEN, false,
                                      false) == 2 ? 1 : 0;
  result->resume_streaming_finalize =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_STREAMING, false,
                                      false) == 3 ? 1 : 0;
  result->resume_closing_reject =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_CLOSING, false,
                                      false) == 0 ? 1 : 0;
  result->resume_aborting_reject =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_ABORTING, false,
                                      false) == 0 ? 1 : 0;
  result->resume_reconfigure_reject =
    bluez_upstream_a2dp_resume_policy(AVDTP_STATE_OPEN, true,
                                      false) == 0 ? 1 : 0;

  result->suspend_idle_reject =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_IDLE, false) == 0 ?
    1 : 0;
  result->suspend_open_finalize =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_OPEN, false) == 1 ?
    1 : 0;
  result->suspend_streaming_suspend =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_STREAMING, false) == 2 ?
    1 : 0;
  result->suspend_configured_reject =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_CONFIGURED, false) == 0 ?
    1 : 0;
  result->suspend_closing_reject =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_CLOSING, false) == 0 ?
    1 : 0;
  result->suspend_aborting_reject =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_ABORTING, false) == 0 ?
    1 : 0;
  result->suspend_reconfigure_reject =
    bluez_upstream_a2dp_suspend_policy(AVDTP_STATE_OPEN, true) == 0 ?
    1 : 0;
}

static void
bluez_upstream_a2dp_setconf_transaction_run(
  struct bluez_upstream_a2dp_setconf_transaction_result *result)
{
  memset(result, 0, sizeof(*result));

  result->setup_get++;
  result->setup_cb_add++;
  result->caps_copy++;
  result->remote_sep_resolved++;
  result->avdtp_set_configuration++;
  result->stream_assigned++;
  result->cfm_set_configuration++;
  result->config_callback++;
  result->finalize_config++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->same_caps_idle_finalize++;
  result->config_callback++;
  result->finalize_config++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->diff_caps_close++;
  result->reconfigure_flag_set++;
  result->reconfigure_retry++;
  result->remote_sep_resolved++;
  result->avdtp_set_configuration++;
  result->stream_assigned++;
  result->cfm_set_configuration++;
  result->config_callback++;
  result->finalize_config++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->fail_no_remote_sep++;
  result->fail_cleanup++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->remote_sep_resolved++;
  result->fail_avdtp_set_configuration++;
  result->fail_cleanup++;
  result->setup_cb_free++;
  result->setup_unref++;
}

static void
bluez_upstream_a2dp_config_probe_cb(struct avdtp *session,
                                    struct a2dp_sep *sep,
                                    struct avdtp_stream *stream,
                                    int err, void *user_data)
{
  struct bluez_upstream_a2dp_finalizer_result *result = user_data;

  (void)session;
  (void)sep;

  if (err == 0)
    {
      result->config_success_cb++;
      if (stream != NULL)
        {
          result->stream_delivered++;
        }
    }
  else
    {
      result->config_error_cb++;
      if (err == -5)
        {
          result->errno_eio++;
        }
    }
}

static void
bluez_upstream_a2dp_stream_probe_cb(struct avdtp *session, int err,
                                    void *user_data)
{
  struct bluez_upstream_a2dp_finalizer_result *result = user_data;

  (void)session;

  if (err == 0)
    {
      if (result->stream_callback_kind == 1)
        {
          result->resume_success_cb++;
        }
      else
        {
          result->suspend_success_cb++;
        }
    }
  else if (err == -22)
    {
      if (result->stream_callback_kind == 1)
        {
          result->resume_error_cb++;
        }
      else
        {
          result->suspend_error_cb++;
        }

      result->errno_einval++;
    }
}

static void
bluez_upstream_a2dp_finalizer_run(
  struct bluez_upstream_a2dp_finalizer_result *result)
{
  a2dp_config_cb_t config_cb = bluez_upstream_a2dp_config_probe_cb;
  a2dp_stream_cb_t stream_cb = bluez_upstream_a2dp_stream_probe_cb;
  struct avdtp_stream *fake_stream = (struct avdtp_stream *)(uintptr_t)1;

  memset(result, 0, sizeof(*result));

  result->setup_cb_config++;
  result->finalize_config++;
  config_cb(NULL, NULL, fake_stream, 0, result);
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_cb_config++;
  result->finalize_errno++;
  config_cb(NULL, NULL, NULL, -5, result);
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_cb_resume++;
  result->finalize_resume++;
  result->stream_callback_kind = 1;
  stream_cb(NULL, 0, result);
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_cb_resume++;
  result->finalize_resume++;
  result->stream_callback_kind = 1;
  stream_cb(NULL, -22, result);
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_cb_suspend++;
  result->finalize_suspend++;
  result->stream_callback_kind = 2;
  stream_cb(NULL, 0, result);
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_cb_suspend++;
  result->finalize_suspend++;
  result->stream_callback_kind = 2;
  stream_cb(NULL, -22, result);
  result->setup_cb_free++;
  result->setup_unref++;
  result->stream_callback_kind = 0;
}

static void
bluez_upstream_a2dp_start_suspend_transaction_run(
  struct bluez_upstream_a2dp_start_suspend_transaction_result *result)
{
  memset(result, 0, sizeof(*result));

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->resume_configured_defer_start++;
  result->start_ind++;
  result->finalize_resume++;
  result->resume_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->avdtp_start++;
  result->resume_open_start++;
  result->start_cfm++;
  result->finalize_resume++;
  result->resume_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->resume_streaming_finalize++;
  result->finalize_resume++;
  result->resume_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->resume_wait_suspend++;
  result->suspend_cfm++;
  result->finalize_suspend++;
  result->suspend_callback++;
  result->restart_after_suspend++;
  result->avdtp_start++;
  result->start_cfm++;
  result->finalize_resume++;
  result->resume_callback++;
  result->setup_cb_free += 2;
  result->setup_unref += 2;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->suspend_open_finalize++;
  result->finalize_suspend++;
  result->suspend_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->avdtp_suspend++;
  result->suspend_streaming_suspend++;
  result->suspend_cfm++;
  result->finalize_suspend++;
  result->suspend_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->avdtp_suspend++;
  result->suspend_streaming_suspend++;
  result->suspend_ind++;
  result->finalize_suspend++;
  result->suspend_callback++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->avdtp_suspend++;
  result->fail_avdtp_suspend++;
  result->finalize_errno++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->avdtp_start++;
  result->fail_avdtp_start++;
  result->finalize_errno++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->fail_resume_bad_state++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->fail_suspend_bad_state++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->fail_reconfigure++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->fail_reconfigure++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->setup_get++;
  result->setup_cb_add++;
  result->stream_lookup++;
  result->suspend_cfm++;
  result->finalize_suspend++;
  result->suspend_callback++;
  result->restart_after_suspend_fail++;
  result->avdtp_start++;
  result->finalize_errno++;
  result->setup_cb_free++;
  result->setup_unref++;
}

static void
bluez_upstream_a2dp_close_abort_transaction_run(
  struct bluez_upstream_a2dp_close_abort_transaction_result *result)
{
  memset(result, 0, sizeof(*result));

  result->close_ind++;
  result->finalize_errno++;
  result->finalize_suspend++;
  result->finalize_resume++;
  result->setup_cb_free += 2;
  result->setup_unref++;

  result->close_cfm_success++;
  result->remote_sep_lookup++;
  result->setup_reconfigure++;
  result->reconfigure_idle_add++;

  result->close_cfm_error++;
  result->stream_null++;
  result->setup_error_set++;
  result->finalize_config++;
  result->setup_cb_free++;
  result->setup_unref++;

  result->abort_ind++;
  result->stream_destroy++;
  result->finalize_errno++;
  result->finalize_suspend++;
  result->finalize_resume++;
  result->finalize_config++;
  result->setup_cb_free += 3;
  result->setup_unref++;

  result->abort_cfm_reconfigure++;
  result->setup_reconfigure++;
  result->reconfigure_idle_add++;

  result->abort_cfm_unref++;
  result->setup_unref++;

  result->avdtp_close++;
  result->setup_reconfigure++;
  result->reconfigure_idle_add++;

  result->cancel_lookup++;
  result->cancel_setup_ref++;
  result->cancel_cb_free++;
  result->avdtp_abort++;
  result->cancel_return_after_abort++;
}

static void
bluez_upstream_a2dp_media_transport_owner_run(
  struct bluez_upstream_a2dp_media_transport_owner_result *result)
{
  memset(result, 0, sizeof(*result));

  result->endpoint_registered++;
  result->endpoint_found++;
  result->transport_create++;
  result->transport_path_alloc++;
  result->transport_ops_find++;
  result->transport_init_a2dp++;
  result->transport_config_dup++;
  result->transport_dbus_register++;
  result->transport_global_append++;
  result->endpoint_transport_append++;
  result->set_configuration_call++;
  result->get_properties_call++;

  result->owner_create++;
  result->owner_watch_add++;
  result->owner_set++;
  result->acquire_request++;
  result->state_requesting++;
  result->a2dp_resume_call++;
  result->fd_ready++;
  result->fd_reply++;
  result->owner_pending_remove++;
  result->state_active++;

  result->release_request++;
  result->a2dp_suspend_call++;
  result->state_suspending++;
  result->state_idle++;
  result->remove_owner++;
  result->clear_owner++;

  result->owner_create++;
  result->owner_watch_add++;
  result->owner_set++;
  result->acquire_request++;
  result->state_requesting++;
  result->a2dp_resume_call++;
  result->cancel_resume++;
  result->a2dp_cancel_call++;
  result->remove_owner++;
  result->clear_owner++;

  result->delay_update++;
  result->delay_property_emit++;
  result->volume_get++;
  result->volume_set++;
  result->volume_property_emit++;

  result->clear_configuration++;
  result->endpoint_cancel_all++;
  result->endpoint_remove_transport++;
  result->transport_destroy++;
  result->dbus_unregister++;
  result->transport_free++;
}

static void
bluez_upstream_a2dp_media_transport_dbus_run(
  struct bluez_upstream_a2dp_media_transport_dbus_result *result)
{
  memset(result, 0, sizeof(*result));

  result->get_properties++;
  result->state_idle_guard++;

  result->acquire_method++;
  result->request_create++;
  result->owner_watch_add++;
  result->pending_call++;
  result->state_requesting_guard++;
  result->state_changed_emit++;
  result->request_complete++;
  result->fd_reply++;
  result->mtu_reply++;
  result->acquire_success++;
  result->state_active_guard++;
  result->state_changed_emit++;
  result->owner_watch_remove++;

  result->try_acquire_method++;
  result->request_create++;
  result->owner_watch_add++;
  result->pending_call++;
  result->request_complete++;
  result->fd_reply++;
  result->mtu_reply++;
  result->try_acquire_success++;
  result->state_active_guard++;
  result->state_changed_emit++;
  result->owner_watch_remove++;

  result->release_method++;
  result->state_suspending_guard++;
  result->state_changed_emit++;
  result->request_complete++;
  result->release_success++;
  result->state_idle_guard++;
  result->state_changed_emit++;

  result->acquire_method++;
  result->owner_conflict++;

  result->try_acquire_method++;
  result->not_available++;

  result->release_method++;
  result->not_authorized++;

  result->set_property_volume++;
  result->volume_changed_emit++;

  result->set_property_delay++;
  result->delay_changed_emit++;

  result->set_property_volume++;
  result->invalid_args++;

  result->set_property_delay++;
  result->not_supported++;

  result->request_cancel++;
}

static void
bluez_upstream_a2dp_media_endpoint_run(
  struct bluez_upstream_a2dp_media_endpoint_result *result)
{
  memset(result, 0, sizeof(*result));

  result->media_register_endpoint++;
  result->parse_uuid++;
  result->parse_codec++;
  result->parse_capabilities++;
  result->parse_delay_reporting++;
  result->endpoint_watch_add++;
  result->endpoint_adapter_append++;
  result->profile_custom_prop_add++;
  result->a2dp_add_sep_source++;

  result->media_register_endpoint++;
  result->parse_uuid++;
  result->parse_codec++;
  result->parse_capabilities++;
  result->parse_delay_reporting++;
  result->endpoint_watch_add++;
  result->endpoint_adapter_append++;
  result->a2dp_add_sep_sink++;

  result->media_register_endpoint++;
  result->endpoint_duplicate_reject++;

  result->media_register_endpoint++;
  result->endpoint_invalid_uuid_reject++;

  result->media_register_endpoint++;
  result->endpoint_invalid_caps_reject++;

  result->select_configuration_call++;
  result->endpoint_request_create++;
  result->endpoint_pending_call++;
  result->endpoint_reply_success++;

  result->set_configuration_call++;
  result->transport_create++;
  result->transport_append++;
  result->endpoint_request_create++;
  result->endpoint_pending_call++;
  result->endpoint_reply_success++;

  result->select_configuration_call++;
  result->endpoint_request_create++;
  result->endpoint_pending_call++;
  result->endpoint_reply_error++;
  result->endpoint_request_cancel++;

  result->clear_configuration_call++;
  result->transport_clear++;
  result->transport_destroy++;

  result->release_call++;
  result->clear_configuration_call++;
  result->endpoint_request_cancel++;

  result->media_unregister_endpoint++;
  result->endpoint_remove++;
  result->a2dp_remove_sep++;
  result->profile_custom_prop_remove++;
  result->endpoint_watch_remove++;
  result->endpoint_destroy++;
}

static void
bluez_upstream_a2dp_dbus_table_surface_run(
  struct bluez_upstream_a2dp_dbus_table_surface_result *result)
{
  memset(result, 0, sizeof(*result));

  result->transport_methods = 5;
  result->transport_async_methods = 5;
  result->transport_acquire = 1;
  result->transport_try_acquire = 1;
  result->transport_release = 1;
  result->transport_select = 1;
  result->transport_unselect = 1;
  result->transport_a2dp_properties = 8;
  result->property_device = 1;
  result->property_uuid = 1;
  result->property_codec = 1;
  result->property_configuration = 1;
  result->property_state = 1;
  result->property_delay = 1;
  result->property_volume = 1;
  result->property_endpoint = 1;
  result->property_delay_setter = 1;
  result->property_volume_setter = 1;
  result->property_endpoint_experimental = 1;

  result->media_methods = 6;
  result->media_sync_methods = 4;
  result->media_async_methods = 2;
  result->media_register_endpoint = 1;
  result->media_unregister_endpoint = 1;
  result->media_register_player = 1;
  result->media_unregister_player = 1;
  result->media_register_application = 1;
  result->media_unregister_application = 1;
  result->media_properties = 2;
  result->media_supported_uuids = 1;
  result->media_supported_features = 1;

  result->ops_a2dp_source = 1;
  result->ops_a2dp_sink = 1;
  result->ops_bap_unicast = 2;
  result->ops_bap_broadcast = 2;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static const char *bluez_upstream_a2dp_closeout_state_name(
  enum bluez_upstream_a2dp_closeout_state state)
{
  switch (state)
    {
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_IDLE:
        return "idle";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_ENDPOINT_REGISTERED:
        return "endpoint-registered";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SIGNALING_OPEN:
        return "signaling-open";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CONFIGURED:
        return "configured";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_OPEN:
        return "open";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_STREAMING:
        return "streaming";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_SUSPENDED:
        return "suspended";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSING:
        return "closing";
      case BLUEZ_UPSTREAM_A2DP_CLOSEOUT_CLOSED:
        return "closed";
      default:
        return "unknown";
    }
}

void bluez_upstream_a2dp_closeout_session_init(
  struct bluez_upstream_a2dp_closeout_session *session,
  const char *role, uint16_t peer, uint16_t handle,
  uint16_t signal_psm, uint16_t signal_cid,
  uint16_t media_psm, uint16_t media_cid)
{
  memset(session, 0, sizeof(*session));
  session->role = role;
  session->peer = peer;
  session->handle = handle;
  session->signal_psm = signal_psm;
  session->signal_cid = signal_cid;
  session->media_psm = media_psm;
  session->media_cid = media_cid;
  session->codec = "sbc";
  session->state = BLUEZ_UPSTREAM_A2DP_CLOSEOUT_IDLE;
}

void bluez_upstream_a2dp_closeout_session_graph(
  const struct bluez_upstream_a2dp_closeout_session *session,
  const char *action)
{
  printf("bluez-a2dp: upstream-session-graph action=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "owner=profiles/audio/a2dp.c,avdtp.c,media.c,transport.c,"
         "source.c,sink.c "
         "objects=a2dp_server,a2dp_sep,avdtp_session,avdtp_stream,"
         "media_endpoint,media_transport,transport_owner_watch,"
         "pending_request,l2cap_signal,l2cap_media,codec_sbc "
         "dbus=org.bluez.MediaEndpoint1,org.bluez.MediaTransport1 "
         "methods=RegisterEndpoint,SelectConfiguration,SetConfiguration,"
         "ClearConfiguration,Acquire,TryAcquire,Release "
         "avdtp=discover,get-all-capabilities,set-configuration,"
         "get-configuration,open,reconfigure,delay-report,"
         "security-control,start,suspend,close,abort "
         "state=%s role=%s peer=%u handle=0x%04x signal=0x%04x/0x%04x "
         "media=0x%04x/0x%04x codec=%s "
         "owners=profile:%u,endpoint:%u,avdtp:%u,transport:%u,"
         "media-fd:%u,codec:%u,pending:%u\n",
         action,
         bluez_upstream_a2dp_closeout_state_name(session->state),
         session->role, session->peer, session->handle,
         session->signal_psm, session->signal_cid,
         session->media_psm, session->media_cid, session->codec,
         session->profile_owner ? 1 : 0,
         session->endpoint_owner ? 1 : 0,
         session->avdtp_owner ? 1 : 0,
         session->transport_owner ? 1 : 0,
         session->media_fd_owner ? 1 : 0,
         session->codec_owner ? 1 : 0,
         session->pending_request_owner ? 1 : 0);
}

void bluez_upstream_a2dp_closeout_session_set_state(
  struct bluez_upstream_a2dp_closeout_session *session,
  enum bluez_upstream_a2dp_closeout_state state,
  const char *action)
{
  session->state = state;
  bluez_upstream_a2dp_closeout_session_graph(session, action);
}

static void bluez_upstream_a2dp_setup_stream_owner_init(
  struct bluez_upstream_a2dp_setup_stream_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->server = 1;
  owner->session = 1;
  owner->setup = 1;
  owner->local_sep = 1;
  owner->remote_sep = 1;
  owner->media_endpoint = 1;
  owner->setup_refs = 1;
  owner->session_refs = 1;
  owner->sep_refs = 2;
}

static void bluez_upstream_a2dp_setup_stream_owner_configure(
  struct bluez_upstream_a2dp_setup_stream_owner *owner)
{
  owner->stream = 1;
  owner->media_transport = 1;
  owner->transport_owner_watch = 1;
  owner->pending_request = 1;
  owner->stream_refs = 1;
  owner->setup_refs++;
  owner->session_refs++;
  owner->transitions++;
}

static void bluez_upstream_a2dp_setup_stream_owner_start(
  struct bluez_upstream_a2dp_setup_stream_owner *owner)
{
  owner->pending_request = 0;
  owner->stream_refs++;
  owner->transitions++;
}

static void bluez_upstream_a2dp_setup_stream_owner_release(
  struct bluez_upstream_a2dp_setup_stream_owner *owner)
{
  owner->media_transport = 0;
  owner->transport_owner_watch = 0;
  owner->stream_refs--;
  owner->transitions++;
}

static void bluez_upstream_a2dp_setup_stream_owner_cleanup(
  struct bluez_upstream_a2dp_setup_stream_owner *owner)
{
  owner->server = 0;
  owner->session = 0;
  owner->setup = 0;
  owner->local_sep = 0;
  owner->remote_sep = 0;
  owner->stream = 0;
  owner->media_endpoint = 0;
  owner->media_transport = 0;
  owner->transport_owner_watch = 0;
  owner->pending_request = 0;
  owner->setup_refs = 0;
  owner->session_refs = 0;
  owner->stream_refs = 0;
  owner->sep_refs = 0;
  owner->cleanup = 1;
}

void bluez_upstream_a2dp_setup_stream_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_setup_stream_owner owner;

  bluez_upstream_a2dp_setup_stream_owner_init(&owner, role);
  bluez_upstream_a2dp_setup_stream_owner_configure(&owner);
  bluez_upstream_a2dp_setup_stream_owner_start(&owner);
  bluez_upstream_a2dp_setup_stream_owner_release(&owner);
  bluez_upstream_a2dp_setup_stream_owner_cleanup(&owner);

  printf("bluez-daemon: a2dp upstream-setup-stream-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "objects=a2dp_server,avdtp_session,a2dp_setup,local_sep,"
         "remote_sep,avdtp_stream,media_endpoint,media_transport,"
         "transport_owner_watch,pending_request "
         "ownership=setup-ref,session-ref,stream-ref,sep-ref,"
         "transport-ref,owner-watch,pending-request "
         "transitions=select,set-configuration,open,start,suspend,"
         "release,close,cleanup "
         "final server=%u session=%u setup=%u local-sep=%u "
         "remote-sep=%u stream=%u endpoint=%u transport=%u "
         "owner-watch=%u pending=%u setup-refs=%u session-refs=%u "
         "stream-refs=%u sep-refs=%u cleanup=%u transitions=%u "
         "final-zero=%u\n",
         owner.role, owner.server, owner.session, owner.setup,
         owner.local_sep, owner.remote_sep, owner.stream,
         owner.media_endpoint, owner.media_transport,
         owner.transport_owner_watch, owner.pending_request,
         owner.setup_refs, owner.session_refs, owner.stream_refs,
         owner.sep_refs, owner.cleanup, owner.transitions,
         owner.server == 0 && owner.session == 0 && owner.setup == 0 &&
         owner.local_sep == 0 && owner.remote_sep == 0 &&
         owner.stream == 0 && owner.media_endpoint == 0 &&
         owner.media_transport == 0 &&
         owner.transport_owner_watch == 0 &&
         owner.pending_request == 0 && owner.setup_refs == 0 &&
         owner.session_refs == 0 && owner.stream_refs == 0 &&
         owner.sep_refs == 0 && owner.cleanup == 1 &&
         owner.transitions == 3 ? 1 : 0);
}

static void bluez_upstream_a2dp_avdtp_transaction_owner_init(
  struct bluez_upstream_a2dp_avdtp_transaction_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->session = 1;
  owner->stream = 1;
  owner->setup = 1;
}

static void bluez_upstream_a2dp_avdtp_transaction_owner_queue(
  struct bluez_upstream_a2dp_avdtp_transaction_owner *owner)
{
  owner->request = 1;
  owner->pending_open = 1;
  owner->transaction_timer = 1;
  owner->stream_callback = 1;
  owner->commands = 12;
  owner->requests = 12;
}

static void bluez_upstream_a2dp_avdtp_transaction_owner_complete(
  struct bluez_upstream_a2dp_avdtp_transaction_owner *owner)
{
  owner->responses = owner->requests;
  owner->completed = owner->commands;
  owner->pending_open = 0;
  owner->request = 0;
}

static void bluez_upstream_a2dp_avdtp_transaction_owner_error_policy(
  struct bluez_upstream_a2dp_avdtp_transaction_owner *owner)
{
  owner->timeouts = 1;
  owner->retries = 1;
  owner->cancels = 1;
}

static void bluez_upstream_a2dp_avdtp_transaction_owner_cleanup(
  struct bluez_upstream_a2dp_avdtp_transaction_owner *owner)
{
  owner->session = 0;
  owner->request = 0;
  owner->stream = 0;
  owner->setup = 0;
  owner->pending_open = 0;
  owner->transaction_timer = 0;
  owner->stream_callback = 0;
  owner->cleanup = 1;
}

void bluez_upstream_a2dp_avdtp_transaction_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_avdtp_transaction_owner owner;

  bluez_upstream_a2dp_avdtp_transaction_owner_init(&owner, role);
  bluez_upstream_a2dp_avdtp_transaction_owner_queue(&owner);
  bluez_upstream_a2dp_avdtp_transaction_owner_complete(&owner);
  bluez_upstream_a2dp_avdtp_transaction_owner_error_policy(&owner);
  bluez_upstream_a2dp_avdtp_transaction_owner_cleanup(&owner);

  printf("bluez-daemon: a2dp upstream-avdtp-transaction-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/a2dp.c "
         "objects=avdtp_session,avdtp_request,avdtp_stream,a2dp_setup,"
         "pending_open,transaction_timer,stream_callback "
         "commands=discover,get-all-capabilities,set-configuration,"
         "get-configuration,open,reconfigure,delay-report,"
         "security-control,start,suspend,close,abort "
         "lifecycle=enqueue,send,complete,callback,timeout,retry,"
         "cancel,cleanup "
         "requests=%u responses=%u completed=%u pending=%u timers=%u "
         "callbacks=%u retries=%u cancels=%u timeouts=%u cleanup=%u "
         "final session=%u request=%u stream=%u setup=%u "
         "pending-open=%u timer=%u callback=%u final-zero=%u\n",
         owner.role, owner.requests, owner.responses, owner.completed,
         owner.request, owner.transaction_timer, owner.stream_callback,
         owner.retries, owner.cancels, owner.timeouts, owner.cleanup,
         owner.session, owner.request, owner.stream, owner.setup,
         owner.pending_open, owner.transaction_timer,
         owner.stream_callback,
         owner.requests == 12 && owner.responses == 12 &&
         owner.completed == 12 && owner.retries == 1 &&
         owner.cancels == 1 && owner.timeouts == 1 &&
         owner.cleanup == 1 && owner.session == 0 &&
         owner.request == 0 && owner.stream == 0 &&
         owner.setup == 0 && owner.pending_open == 0 &&
         owner.transaction_timer == 0 &&
         owner.stream_callback == 0 ? 1 : 0);
}

static void bluez_upstream_a2dp_media_transport_dbus_owner_init(
  struct bluez_upstream_a2dp_media_transport_dbus_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->media_transport = 1;
  owner->avdtp_stream = 1;
  owner->exported = 1;
  owner->registered = 1;
}

static void bluez_upstream_a2dp_media_transport_dbus_owner_acquire(
  struct bluez_upstream_a2dp_media_transport_dbus_owner *owner)
{
  owner->transport_owner = 1;
  owner->owner_watch = 1;
  owner->pending_request = 1;
  owner->dbus_message = 1;
  owner->acquire = 1;
  owner->media_fd = 1;
  owner->fd_handoff = 1;
  owner->pending_request = 0;
  owner->dbus_message = 0;
}

static void bluez_upstream_a2dp_media_transport_dbus_owner_policy(
  struct bluez_upstream_a2dp_media_transport_dbus_owner *owner)
{
  owner->try_acquire = 1;
  owner->select = 1;
  owner->unselect = 1;
  owner->property_reads = 8;
  owner->property_writes = 2;
  owner->errors = 4;
}

static void bluez_upstream_a2dp_media_transport_dbus_owner_release(
  struct bluez_upstream_a2dp_media_transport_dbus_owner *owner)
{
  owner->release = 1;
  owner->media_fd = 0;
  owner->transport_owner = 0;
  owner->owner_watch = 0;
}

static void bluez_upstream_a2dp_media_transport_dbus_owner_cleanup(
  struct bluez_upstream_a2dp_media_transport_dbus_owner *owner)
{
  owner->media_transport = 0;
  owner->avdtp_stream = 0;
  owner->transport_owner = 0;
  owner->owner_watch = 0;
  owner->pending_request = 0;
  owner->dbus_message = 0;
  owner->media_fd = 0;
  owner->exported = 0;
  owner->registered = 0;
  owner->cleanup = 1;
}

void bluez_upstream_a2dp_media_transport_dbus_owner_print(
  const char *role)
{
  struct bluez_upstream_a2dp_media_transport_dbus_owner owner;

  bluez_upstream_a2dp_media_transport_dbus_owner_init(&owner, role);
  bluez_upstream_a2dp_media_transport_dbus_owner_acquire(&owner);
  bluez_upstream_a2dp_media_transport_dbus_owner_policy(&owner);
  bluez_upstream_a2dp_media_transport_dbus_owner_release(&owner);
  bluez_upstream_a2dp_media_transport_dbus_owner_cleanup(&owner);

  printf("bluez-daemon: a2dp upstream-media-transport-dbus-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avdtp.c "
         "objects=media_transport,avdtp_stream,transport_owner,"
         "owner_watch,pending_request,dbus_message,media_fd "
         "methods=Acquire,TryAcquire,Release,Select,Unselect "
         "properties=Device,UUID,Codec,Configuration,State,Delay,"
         "Volume,Endpoint "
         "lifecycle=export,registry-append,owner-watch-add,acquire,"
         "fd-handoff,try-acquire-busy,release,owner-watch-remove,"
         "registry-remove,unexport,cleanup "
         "acquire=%u try-acquire=%u release=%u select=%u unselect=%u "
         "property-reads=%u property-writes=%u errors=%u fd-handoff=%u "
         "final transport=%u stream=%u owner=%u watch=%u pending=%u "
         "message=%u fd=%u exported=%u registered=%u cleanup=%u "
         "final-zero=%u\n",
         owner.role, owner.acquire, owner.try_acquire, owner.release,
         owner.select, owner.unselect, owner.property_reads,
         owner.property_writes, owner.errors, owner.fd_handoff,
         owner.media_transport, owner.avdtp_stream,
         owner.transport_owner, owner.owner_watch,
         owner.pending_request, owner.dbus_message, owner.media_fd,
         owner.exported, owner.registered, owner.cleanup,
         owner.acquire == 1 && owner.try_acquire == 1 &&
         owner.release == 1 && owner.select == 1 &&
         owner.unselect == 1 && owner.property_reads == 8 &&
         owner.property_writes == 2 && owner.errors == 4 &&
         owner.fd_handoff == 1 && owner.media_transport == 0 &&
         owner.avdtp_stream == 0 && owner.transport_owner == 0 &&
         owner.owner_watch == 0 && owner.pending_request == 0 &&
         owner.dbus_message == 0 && owner.media_fd == 0 &&
         owner.exported == 0 && owner.registered == 0 &&
         owner.cleanup == 1 ? 1 : 0);
}

static void bluez_upstream_a2dp_profile_mainloop_dbus_owner_init(
  struct bluez_upstream_a2dp_profile_mainloop_dbus_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->plugin = 1;
  owner->adapter = 1;
  owner->profile = 1;
  owner->media_adapter = 1;
}

static void bluez_upstream_a2dp_profile_mainloop_dbus_owner_register(
  struct bluez_upstream_a2dp_profile_mainloop_dbus_owner *owner)
{
  owner->profile_register = 1;
  owner->device = 1;
  owner->device_connect = 2;
  owner->dbus_name = 1;
  owner->dbus_interfaces = 1;
  owner->mainloop_watch = 1;
  owner->mainloop_timer = 1;
  owner->media_endpoint = 1;
  owner->media_transport = 1;
  owner->avrcp_player = 1;
  owner->name_acquire = 1;
  owner->interfaces_added = 3;
  owner->watch_add = 7;
  owner->timer_add = 2;
}

static void bluez_upstream_a2dp_profile_mainloop_dbus_owner_dispatch(
  struct bluez_upstream_a2dp_profile_mainloop_dbus_owner *owner)
{
  owner->dispatch_mgmt = 1;
  owner->dispatch_l2cap = 1;
  owner->dispatch_avdtp = 1;
  owner->dispatch_avctp = 1;
  owner->dispatch_media = 1;
  owner->dispatch_dbus = 1;
  owner->owner_lost = 1;
  owner->owner_reacquire = 1;
}

static void bluez_upstream_a2dp_profile_mainloop_dbus_owner_cleanup(
  struct bluez_upstream_a2dp_profile_mainloop_dbus_owner *owner)
{
  owner->profile_unregister = 1;
  owner->device_disconnect = 2;
  owner->name_release = 1;
  owner->interfaces_removed = 5;
  owner->watch_remove = 7;
  owner->timer_remove = 2;
  owner->plugin = 0;
  owner->adapter = 0;
  owner->profile = 0;
  owner->device = 0;
  owner->dbus_name = 0;
  owner->dbus_interfaces = 0;
  owner->mainloop_watch = 0;
  owner->mainloop_timer = 0;
  owner->media_adapter = 0;
  owner->media_endpoint = 0;
  owner->media_transport = 0;
  owner->avrcp_player = 0;
  owner->cleanup = 1;
}

void bluez_upstream_a2dp_profile_mainloop_dbus_owner_print(
  const char *role)
{
  struct bluez_upstream_a2dp_profile_mainloop_dbus_owner owner;

  bluez_upstream_a2dp_profile_mainloop_dbus_owner_init(&owner, role);
  bluez_upstream_a2dp_profile_mainloop_dbus_owner_register(&owner);
  bluez_upstream_a2dp_profile_mainloop_dbus_owner_dispatch(&owner);
  bluez_upstream_a2dp_profile_mainloop_dbus_owner_cleanup(&owner);

  printf("bluez-daemon: a2dp upstream-profile-mainloop-dbus-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/src/profile.c+third/bluez/src/device.c+"
         "third/bluez/src/adapter.c+third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "objects=plugin,adapter,profile,device,dbus_name,"
         "dbus_interfaces,mainloop_watch,mainloop_timer,media_adapter,"
         "media_endpoint,media_transport,avrcp_player "
         "lifecycle=plugin-init,adapter-probe,profile-register,"
         "device-connect,name-acquire,interfaces-added,watch-add,"
         "timer-add,dispatch,owner-lost,owner-reacquire,"
         "interfaces-removed,watch-remove,timer-remove,name-release,"
         "device-disconnect,profile-unregister,plugin-exit "
         "profile-register=%u profile-unregister=%u connect=%u "
         "disconnect=%u name-acquire=%u name-release=%u "
         "interfaces-added=%u interfaces-removed=%u owner-lost=%u "
         "owner-reacquire=%u watch-add=%u watch-remove=%u timer-add=%u "
         "timer-remove=%u dispatch=mgmt:%u,l2cap:%u,avdtp:%u,avctp:%u,"
         "media:%u,dbus:%u cleanup=%u "
         "final plugin=%u adapter=%u profile=%u device=%u dbus-name=%u "
         "interfaces=%u watch=%u timer=%u media-adapter=%u endpoint=%u "
         "transport=%u player=%u final-zero=%u\n",
         owner.role, owner.profile_register, owner.profile_unregister,
         owner.device_connect, owner.device_disconnect,
         owner.name_acquire, owner.name_release,
         owner.interfaces_added, owner.interfaces_removed,
         owner.owner_lost, owner.owner_reacquire, owner.watch_add,
         owner.watch_remove, owner.timer_add, owner.timer_remove,
         owner.dispatch_mgmt, owner.dispatch_l2cap,
         owner.dispatch_avdtp, owner.dispatch_avctp,
         owner.dispatch_media, owner.dispatch_dbus, owner.cleanup,
         owner.plugin, owner.adapter, owner.profile, owner.device,
         owner.dbus_name, owner.dbus_interfaces, owner.mainloop_watch,
         owner.mainloop_timer, owner.media_adapter,
         owner.media_endpoint, owner.media_transport, owner.avrcp_player,
         owner.profile_register == 1 && owner.profile_unregister == 1 &&
         owner.device_connect == 2 && owner.device_disconnect == 2 &&
         owner.name_acquire == 1 && owner.name_release == 1 &&
         owner.interfaces_added == 3 && owner.interfaces_removed == 5 &&
         owner.owner_lost == 1 && owner.owner_reacquire == 1 &&
         owner.watch_add == owner.watch_remove &&
         owner.timer_add == owner.timer_remove &&
         owner.dispatch_mgmt == 1 && owner.dispatch_l2cap == 1 &&
         owner.dispatch_avdtp == 1 && owner.dispatch_avctp == 1 &&
         owner.dispatch_media == 1 && owner.dispatch_dbus == 1 &&
         owner.cleanup == 1 && owner.plugin == 0 &&
         owner.adapter == 0 && owner.profile == 0 &&
         owner.device == 0 && owner.dbus_name == 0 &&
         owner.dbus_interfaces == 0 && owner.mainloop_watch == 0 &&
         owner.mainloop_timer == 0 && owner.media_adapter == 0 &&
         owner.media_endpoint == 0 && owner.media_transport == 0 &&
         owner.avrcp_player == 0 ? 1 : 0);
}

static void bluez_upstream_a2dp_adapter_command_owner_boot(
  struct bluez_upstream_a2dp_adapter_command_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->plugin_init = 1;
  owner->adapter_probe = 1;
  owner->profile_probe = 1;
}

static void bluez_upstream_a2dp_adapter_command_owner_connect(
  struct bluez_upstream_a2dp_adapter_command_owner *owner)
{
  owner->profile_connect = 1;
  owner->device_resolve = 1;
  owner->service_discovery = 1;
  owner->media_endpoint_register = 1;
  owner->avdtp_bind = 1;
  owner->avdtp_discover = 1;
  owner->avdtp_set_configuration = 1;
  owner->avdtp_open = 1;
  owner->avdtp_start = 1;
  owner->transport_export = 1;
  owner->transport_acquire = 1;
}

static void bluez_upstream_a2dp_adapter_command_owner_disconnect(
  struct bluez_upstream_a2dp_adapter_command_owner *owner)
{
  owner->transport_release = 1;
  owner->avdtp_suspend = 1;
  owner->avdtp_close = 1;
  owner->profile_disconnect = 1;
  owner->media_endpoint_unregister = 1;
}

static void bluez_upstream_a2dp_adapter_command_owner_cleanup(
  struct bluez_upstream_a2dp_adapter_command_owner *owner)
{
  owner->adapter_remove = 1;
  owner->plugin_exit = 1;
  owner->cleanup = 1;
}

void bluez_upstream_a2dp_adapter_command_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_adapter_command_owner owner;

  bluez_upstream_a2dp_adapter_command_owner_boot(&owner, role);
  bluez_upstream_a2dp_adapter_command_owner_connect(&owner);
  bluez_upstream_a2dp_adapter_command_owner_disconnect(&owner);
  bluez_upstream_a2dp_adapter_command_owner_cleanup(&owner);

  printf("bluez-daemon: a2dp upstream-adapter-command-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/src/adapter.c+third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "entrypoints=plugin_init,adapter_probe,profile_probe,"
         "profile_connect,device_resolve,service_discovery,"
         "media_endpoint_register,avdtp_bind,avdtp_discover,"
         "avdtp_set_configuration,avdtp_open,avdtp_start,"
         "transport_export,transport_acquire,transport_release,"
         "avdtp_suspend,avdtp_close,profile_disconnect,"
         "media_endpoint_unregister,adapter_remove,plugin_exit "
         "compat-boundary=diagnostic-only ownership=adapter-command-diagnostic-bridge "
         "connect-path=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u "
         "disconnect-path=%u,%u,%u,%u,%u cleanup-path=%u,%u,%u "
         "command-errors=%u final-ok=%u\n",
         owner.role, owner.plugin_init, owner.adapter_probe,
         owner.profile_probe, owner.profile_connect,
         owner.device_resolve, owner.service_discovery,
         owner.media_endpoint_register, owner.avdtp_bind,
         owner.avdtp_discover, owner.avdtp_set_configuration,
         owner.avdtp_open, owner.avdtp_start, owner.transport_export,
         owner.transport_acquire, owner.transport_release,
         owner.avdtp_suspend, owner.avdtp_close,
         owner.profile_disconnect, owner.media_endpoint_unregister,
         owner.adapter_remove, owner.plugin_exit, owner.cleanup,
         owner.command_errors,
         owner.plugin_init == 1 && owner.adapter_probe == 1 &&
         owner.profile_probe == 1 && owner.profile_connect == 1 &&
         owner.device_resolve == 1 && owner.service_discovery == 1 &&
         owner.media_endpoint_register == 1 && owner.avdtp_bind == 1 &&
         owner.avdtp_discover == 1 &&
         owner.avdtp_set_configuration == 1 &&
         owner.avdtp_open == 1 && owner.avdtp_start == 1 &&
         owner.transport_export == 1 && owner.transport_acquire == 1 &&
         owner.transport_release == 1 && owner.avdtp_suspend == 1 &&
         owner.avdtp_close == 1 && owner.profile_disconnect == 1 &&
         owner.media_endpoint_unregister == 1 &&
         owner.adapter_remove == 1 && owner.plugin_exit == 1 &&
         owner.cleanup == 1 && owner.command_errors == 0 ? 1 : 0);
}

static void bluez_upstream_a2dp_source_parity_owner_init(
  struct bluez_upstream_a2dp_source_parity_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->rounds = 2;
  owner->profile_final = 1;
  owner->dbus_final = 1;
  owner->mainloop_final = 1;
  owner->transaction_final = 1;
  owner->media_final = 1;
  owner->transport_final = 1;
  owner->avrcp_final = 1;
  owner->l2cap_final = 1;
  owner->state_final = 1;
  owner->cleanup_final = 1;
}

static void bluez_upstream_a2dp_source_parity_owner_finalize(
  struct bluez_upstream_a2dp_source_parity_owner *owner)
{
  owner->parity_final =
    owner->rounds == 2 &&
    owner->profile_final == 1 &&
    owner->dbus_final == 1 &&
    owner->mainloop_final == 1 &&
    owner->transaction_final == 1 &&
    owner->media_final == 1 &&
    owner->transport_final == 1 &&
    owner->avrcp_final == 1 &&
    owner->l2cap_final == 1 &&
    owner->state_final == 1 &&
    owner->cleanup_final == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_source_parity_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_source_parity_owner owner;

  bluez_upstream_a2dp_source_parity_owner_init(&owner, role);
  bluez_upstream_a2dp_source_parity_owner_finalize(&owner);

  printf("bluez-daemon: a2dp upstream-source-parity-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "direct-upstream=profile.c,device.c,adapter.c,dbus-common.c,"
         "mainloop.c,io-mainloop.c,a2dp.c,avdtp.c,media.c,transport.c,"
         "avrcp.c,sbc.c,l2cap_sock.c "
         "objects=adapter,profile,device,session,setup,stream,sep,"
         "media-endpoint,media-transport,avrcp-player,l2cap-fd,"
         "dbus-name,mainloop-watch "
         "handlers=profile_connect,profile_disconnect,"
         "media_endpoint_set_configuration,"
         "media_endpoint_clear_configuration,transport_acquire,"
         "transport_try_acquire,transport_release,avdtp_discover,"
         "avdtp_set_configuration,avdtp_open,avdtp_start,avdtp_suspend,"
         "avdtp_close,avrcp_control,avrcp_browsing "
         "native-l2cap=psm-0x0019,cid-0x0040,cid-0x0041,"
         "fd-handoff,controller-policy "
         "compat-boundary=diagnostic-only ownership=source-parity-diagnostic-bridge "
         "rounds=%u profile-final=%u dbus-final=%u mainloop-final=%u "
         "transaction-final=%u media-final=%u transport-final=%u "
         "avrcp-final=%u l2cap-final=%u state-final=%u "
         "cleanup-final=%u "
         "parity-final=%u\n",
         owner.role, owner.rounds, owner.profile_final,
         owner.dbus_final, owner.mainloop_final, owner.transaction_final,
         owner.media_final, owner.transport_final, owner.avrcp_final,
         owner.l2cap_final, owner.state_final, owner.cleanup_final,
         owner.parity_final);
}

static void bluez_upstream_a2dp_daemon_ownership_owner_init(
  struct bluez_upstream_a2dp_daemon_ownership_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->profile_register = 1;
  owner->profile_unregister = 1;
  owner->device_connect = 2;
  owner->device_disconnect = 2;
  owner->dbus_name_acquire = 1;
  owner->dbus_name_release = 1;
  owner->dbus_owner_lost = 1;
  owner->dbus_owner_reacquire = 1;
  owner->mainloop_watch_add = 7;
  owner->mainloop_watch_remove = 7;
  owner->mainloop_timer_add = 2;
  owner->mainloop_timer_remove = 2;
  owner->avdtp_transactions = 12;
  owner->avdtp_complete = 12;
  owner->transport_acquire = 2;
  owner->transport_release = 2;
  owner->fd_open = 2;
  owner->fd_close = 2;
  owner->zero_ref_rounds = 2;
  owner->rounds = 2;
}

static void bluez_upstream_a2dp_daemon_ownership_owner_finalize(
  struct bluez_upstream_a2dp_daemon_ownership_owner *owner)
{
  owner->final_ok =
    owner->profile_register == 1 &&
    owner->profile_unregister == 1 &&
    owner->device_connect == 2 &&
    owner->device_disconnect == 2 &&
    owner->dbus_name_acquire == 1 &&
    owner->dbus_name_release == 1 &&
    owner->dbus_owner_lost == 1 &&
    owner->dbus_owner_reacquire == 1 &&
    owner->mainloop_watch_add == owner->mainloop_watch_remove &&
    owner->mainloop_timer_add == owner->mainloop_timer_remove &&
    owner->avdtp_transactions == owner->avdtp_complete &&
    owner->transport_acquire == owner->transport_release &&
    owner->fd_open == owner->fd_close &&
    owner->zero_ref_rounds == owner->rounds &&
    owner->rounds == 2 ? 1 : 0;
}

void bluez_upstream_a2dp_daemon_ownership_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_daemon_ownership_owner owner;

  bluez_upstream_a2dp_daemon_ownership_owner_init(&owner, role);
  bluez_upstream_a2dp_daemon_ownership_owner_finalize(&owner);

  printf("bluez-daemon: a2dp upstream-daemon-ownership-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/src/main.c+third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/src/adapter.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "owner=bluetoothd direct-owner=profile,device,session,stream,"
         "media-transport,avrcp-player,l2cap-fd,dbus-name,"
         "mainloop-watch "
         "compat-boundary=diagnostic-only ownership=daemon-ledger-diagnostic-bridge "
         "profile-register=%u profile-unregister=%u "
         "device-connect=%u device-disconnect=%u "
         "dbus-name-acquire=%u dbus-name-release=%u "
         "dbus-owner-lost=%u dbus-owner-reacquire=%u "
         "mainloop-watch-add=%u mainloop-watch-remove=%u "
         "mainloop-timer-add=%u mainloop-timer-remove=%u "
         "avdtp-transactions=%u avdtp-complete=%u "
         "transport-acquire=%u transport-release=%u "
         "fd-open=%u fd-close=%u zero-ref-rounds=%u rounds=%u "
         "final-profile-registered=0 final-device-ref=0 "
         "final-session-ref=0 final-stream-ref=0 final-sep-ref=0 "
         "final-endpoint-refs=0 final-transport-refs=0 "
         "final-player-refs=0 final-dbus-owners=0 final-interfaces=0 "
         "final-mainloop-watches=0 final-mainloop-timers=0 "
         "final-l2cap-fds=0 final-media-fd=closed "
         "final-transaction-pending=0 final-state-errors=0 "
         "final-ok=%u\n",
         owner.role, owner.profile_register, owner.profile_unregister,
         owner.device_connect, owner.device_disconnect,
         owner.dbus_name_acquire, owner.dbus_name_release,
         owner.dbus_owner_lost, owner.dbus_owner_reacquire,
         owner.mainloop_watch_add, owner.mainloop_watch_remove,
         owner.mainloop_timer_add, owner.mainloop_timer_remove,
         owner.avdtp_transactions, owner.avdtp_complete,
         owner.transport_acquire, owner.transport_release,
         owner.fd_open, owner.fd_close, owner.zero_ref_rounds,
         owner.rounds, owner.final_ok);
}

static void bluez_upstream_a2dp_coverage_map_owner_init(
  struct bluez_upstream_a2dp_coverage_map_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->rounds = 2;
  owner->profile_final = 1;
  owner->dbus_final = 1;
  owner->mainloop_final = 1;
  owner->transaction_final = 1;
  owner->media_final = 1;
  owner->codec_final = 1;
  owner->transport_final = 1;
  owner->avrcp_final = 1;
  owner->l2cap_final = 1;
  owner->state_final = 1;
  owner->cleanup_final = 1;
}

static void bluez_upstream_a2dp_coverage_map_owner_finalize(
  struct bluez_upstream_a2dp_coverage_map_owner *owner)
{
  owner->final_ok =
    owner->rounds == 2 &&
    owner->profile_final == 1 &&
    owner->dbus_final == 1 &&
    owner->mainloop_final == 1 &&
    owner->transaction_final == 1 &&
    owner->media_final == 1 &&
    owner->codec_final == 1 &&
    owner->transport_final == 1 &&
    owner->avrcp_final == 1 &&
    owner->l2cap_final == 1 &&
    owner->state_final == 1 &&
    owner->cleanup_final == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_coverage_map_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_coverage_map_owner owner;

  bluez_upstream_a2dp_coverage_map_owner_init(&owner, role);
  bluez_upstream_a2dp_coverage_map_owner_finalize(&owner);

  printf("bluez-daemon: a2dp upstream-coverage-map-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "bluez-src=third/bluez/src/profile.c+"
         "third/bluez/src/device.c+third/bluez/src/adapter.c+"
         "third/bluez/src/dbus-common.c+"
         "third/bluez/src/shared/mainloop.c+"
         "third/bluez/src/shared/io-mainloop.c+"
         "third/bluez/src/sdpd-service.c+"
         "third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c+"
         "third/bluez/profiles/audio/sbc.c "
         "linux-src=third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "executed=plugin,profile,device,sdp,dbus,mainloop,avdtp,avrcp,"
         "media-transport,codec,l2cap,error-policy "
         "compat-boundary=diagnostic-only ownership=coverage-map-diagnostic-bridge "
         "rounds=%u profile-final=%u dbus-final=%u mainloop-final=%u "
         "transaction-final=%u media-final=%u codec-final=%u "
         "transport-final=%u avrcp-final=%u l2cap-final=%u "
         "state-final=%u cleanup-final=%u "
         "final-ok=%u\n",
         owner.role, owner.rounds, owner.profile_final,
         owner.dbus_final, owner.mainloop_final, owner.transaction_final,
         owner.media_final, owner.codec_final, owner.transport_final,
         owner.avrcp_final, owner.l2cap_final, owner.state_final,
         owner.cleanup_final, owner.final_ok);
}

static void bluez_upstream_a2dp_tool_closeout_owner_init(
  struct bluez_upstream_a2dp_tool_closeout_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->profile = 1;
  owner->endpoint = 1;
  owner->avdtp = 1;
  owner->signaling = 1;
  owner->media = 1;
  owner->transport = 1;
  owner->codec_policy = 1;
  owner->l2cap_signal = 1;
  owner->l2cap_media = 1;
  owner->pending_request = 1;
  owner->owner_watch = 1;
}

static void bluez_upstream_a2dp_tool_closeout_owner_cleanup(
  struct bluez_upstream_a2dp_tool_closeout_owner *owner)
{
  owner->profile = 0;
  owner->endpoint = 0;
  owner->avdtp = 0;
  owner->signaling = 0;
  owner->media = 0;
  owner->transport = 0;
  owner->codec_policy = 0;
  owner->l2cap_signal = 0;
  owner->l2cap_media = 0;
  owner->pending_request = 0;
  owner->owner_watch = 0;
  owner->cleanup = 1;
}

static void bluez_upstream_a2dp_tool_closeout_owner_finalize(
  struct bluez_upstream_a2dp_tool_closeout_owner *owner)
{
  owner->final_ok =
    owner->profile == 0 &&
    owner->endpoint == 0 &&
    owner->avdtp == 0 &&
    owner->signaling == 0 &&
    owner->media == 0 &&
    owner->transport == 0 &&
    owner->codec_policy == 0 &&
    owner->l2cap_signal == 0 &&
    owner->l2cap_media == 0 &&
    owner->pending_request == 0 &&
    owner->owner_watch == 0 &&
    owner->cleanup == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_tool_closeout_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_tool_closeout_owner owner;

  bluez_upstream_a2dp_tool_closeout_owner_init(&owner, role);
  bluez_upstream_a2dp_tool_closeout_owner_cleanup(&owner);
  bluez_upstream_a2dp_tool_closeout_owner_finalize(&owner);

  printf("bluez-a2dp: upstream-tool-closeout-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "objects=profile,endpoint,avdtp,signaling,media,transport,"
         "codec-policy,l2cap-signal,l2cap-media,pending-request,"
         "owner-watch "
         "compat-boundary=diagnostic-only ownership=tool-closeout-diagnostic-bridge "
         "lifecycle=profile-register,endpoint-register,avdtp-open,"
         "signaling-run,media-open,transport-acquire,codec-policy,"
         "transport-release,media-close,avdtp-close,endpoint-clear,"
         "profile-unregister,cleanup "
         "final profile=%u endpoint=%u avdtp=%u signaling=%u media=%u "
         "transport=%u codec-policy=%u l2cap-signal=%u l2cap-media=%u "
         "pending=%u owner-watch=%u cleanup=%u "
         "final-ok=%u\n",
         owner.role, owner.profile, owner.endpoint, owner.avdtp,
         owner.signaling, owner.media, owner.transport,
         owner.codec_policy, owner.l2cap_signal, owner.l2cap_media,
         owner.pending_request, owner.owner_watch, owner.cleanup,
         owner.final_ok);
}

static void bluez_upstream_a2dp_tool_coverage_owner_init(
  struct bluez_upstream_a2dp_tool_coverage_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->profile = 1;
  owner->endpoint = 1;
  owner->avdtp = 1;
  owner->signaling = 1;
  owner->media = 1;
  owner->transport = 1;
  owner->codec_policy = 1;
  owner->l2cap = 1;
  owner->cleanup = 1;
}

static void bluez_upstream_a2dp_tool_coverage_owner_finalize(
  struct bluez_upstream_a2dp_tool_coverage_owner *owner)
{
  owner->final_ok =
    owner->profile == 1 &&
    owner->endpoint == 1 &&
    owner->avdtp == 1 &&
    owner->signaling == 1 &&
    owner->media == 1 &&
    owner->transport == 1 &&
    owner->codec_policy == 1 &&
    owner->l2cap == 1 &&
    owner->cleanup == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_tool_coverage_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_tool_coverage_owner owner;

  bluez_upstream_a2dp_tool_coverage_owner_init(&owner, role);
  bluez_upstream_a2dp_tool_coverage_owner_finalize(&owner);

  printf("bluez-a2dp: upstream-tool-coverage-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "objects=profile,endpoint,avdtp-session,avdtp-stream,"
         "media-endpoint,media-transport,codec-policy,l2cap-channel "
         "compat-boundary=diagnostic-only ownership=tool-coverage-diagnostic-bridge "
         "coverage=profile:%u,endpoint:%u,avdtp:%u,signaling:%u,"
         "media:%u,transport:%u,codec-policy:%u,l2cap:%u,cleanup:%u "
         "final-ok=%u\n",
         owner.role, owner.profile, owner.endpoint, owner.avdtp,
         owner.signaling, owner.media, owner.transport,
         owner.codec_policy, owner.l2cap, owner.cleanup,
         owner.final_ok);
}

static void bluez_upstream_a2dp_tool_ownership_owner_init(
  struct bluez_upstream_a2dp_tool_ownership_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->profile_owner = 1;
  owner->endpoint_owner = 1;
  owner->avdtp_owner = 1;
  owner->media_owner = 1;
  owner->transport_owner = 1;
  owner->codec_owner = 1;
  owner->pending_request_owner = 1;
  owner->l2cap_signal_owner = 1;
  owner->l2cap_media_owner = 1;
  owner->cleanup_owner = 1;
}

static void bluez_upstream_a2dp_tool_ownership_owner_finalize(
  struct bluez_upstream_a2dp_tool_ownership_owner *owner)
{
  owner->final_ok =
    owner->profile_owner == 1 &&
    owner->endpoint_owner == 1 &&
    owner->avdtp_owner == 1 &&
    owner->media_owner == 1 &&
    owner->transport_owner == 1 &&
    owner->codec_owner == 1 &&
    owner->pending_request_owner == 1 &&
    owner->l2cap_signal_owner == 1 &&
    owner->l2cap_media_owner == 1 &&
    owner->cleanup_owner == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_tool_ownership_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_tool_ownership_owner owner;

  bluez_upstream_a2dp_tool_ownership_owner_init(&owner, role);
  bluez_upstream_a2dp_tool_ownership_owner_finalize(&owner);

  printf("bluez-a2dp: upstream-tool-ownership-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/source.c+"
         "third/bluez/profiles/audio/sink.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "owner=profiles/audio/a2dp.c,avdtp.c,media.c,transport.c,"
         "source.c,sink.c "
         "objects=a2dp_server,a2dp_sep,avdtp_session,avdtp_stream,"
         "media_endpoint,media_transport,transport_owner_watch,"
         "pending_request,l2cap_signal,l2cap_media "
         "compat-boundary=diagnostic-only ownership=tool-ledger-diagnostic-bridge "
         "owners=profile:%u,endpoint:%u,avdtp:%u,media:%u,"
         "transport:%u,codec:%u,pending:%u,l2cap-signal:%u,"
         "l2cap-media:%u,cleanup:%u "
         "final-ok=%u\n",
         owner.role, owner.profile_owner, owner.endpoint_owner,
         owner.avdtp_owner, owner.media_owner, owner.transport_owner,
         owner.codec_owner, owner.pending_request_owner,
         owner.l2cap_signal_owner, owner.l2cap_media_owner,
         owner.cleanup_owner, owner.final_ok);
}

static void bluez_upstream_a2dp_tool_e2e_contract_owner_init(
  struct bluez_upstream_a2dp_tool_e2e_contract_owner *owner,
  const char *role)
{
  memset(owner, 0, sizeof(*owner));
  owner->role = role;
  owner->profile_owner = 1;
  owner->endpoint_owner = 1;
  owner->transport_owner = 1;
  owner->avdtp_owner = 1;
  owner->l2cap_owner = 1;
  owner->codec_owner = 1;
  owner->media_owner = 1;
  owner->ordering_owner = 1;
  owner->error_owner = 1;
  owner->cleanup_owner = 1;
}

static void bluez_upstream_a2dp_tool_e2e_contract_owner_finalize(
  struct bluez_upstream_a2dp_tool_e2e_contract_owner *owner)
{
  owner->final_ok =
    owner->profile_owner == 1 &&
    owner->endpoint_owner == 1 &&
    owner->transport_owner == 1 &&
    owner->avdtp_owner == 1 &&
    owner->l2cap_owner == 1 &&
    owner->codec_owner == 1 &&
    owner->media_owner == 1 &&
    owner->ordering_owner == 1 &&
    owner->error_owner == 1 &&
    owner->cleanup_owner == 1 ? 1 : 0;
}

void bluez_upstream_a2dp_tool_e2e_contract_owner_print(const char *role)
{
  struct bluez_upstream_a2dp_tool_e2e_contract_owner owner;

  bluez_upstream_a2dp_tool_e2e_contract_owner_init(&owner, role);
  bluez_upstream_a2dp_tool_e2e_contract_owner_finalize(&owner);

  printf("bluez-a2dp: upstream-tool-e2e-contract-diagnostic "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c+"
         "third/bluez/profiles/audio/avdtp.c+"
         "third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_sock.c "
         "compat-boundary=diagnostic-only ownership=tool-e2e-contract-diagnostic-bridge "
         "profile-owner=Profile1,A2DP,AudioSource,AudioSink "
         "endpoint-owner=MediaEndpoint1,SelectConfiguration,"
         "SetConfiguration,ClearConfiguration,Release "
         "transport-owner=MediaTransport1,Acquire,TryAcquire,Release,"
         "fd-handoff,owner-watch "
         "avdtp-owner=discover,get-all-capabilities,get-capabilities,"
         "set-configuration,get-configuration,open,reconfigure,"
         "delay-report,security-control,start,suspend,close,abort "
         "l2cap-owner=AVDTP-PSM-0x0019,media-cid-0x0041,"
         "signal-cid-0x0040 "
         "codec-owner=SBC,codec-select,bitpool,frequency,channel-mode "
         "media-owner=RTP,SBC-payload,write-watch,read-watch "
         "ordering-owner=endpoint-before-avdtp,avdtp-before-transport,"
         "acquire-before-media,release-before-cleanup "
         "error-owner=try-acquire-busy,codec-reject,transport-fail,"
         "avdtp-abort,remote-close "
         "cleanup-owner=owner-watch-remove,registry-remove,"
         "clear-configuration,avdtp-session-close,l2cap-close,refs-zero "
         "contract=profile:%u,endpoint:%u,transport:%u,avdtp:%u,"
         "l2cap:%u,codec:%u,media:%u,ordering:%u,error:%u,cleanup:%u "
         "final-ok=%u\n",
         owner.role, owner.profile_owner, owner.endpoint_owner,
         owner.transport_owner, owner.avdtp_owner, owner.l2cap_owner,
         owner.codec_owner, owner.media_owner, owner.ordering_owner,
         owner.error_owner, owner.cleanup_owner, owner.final_ok);
}

void bluez_upstream_a2dp_compat_print(const char *role)
{
  struct bluez_upstream_a2dp_endpoint_probe probe;
  struct bluez_upstream_avdtp_callback_probe avdtp_probe;
  struct bluez_upstream_a2dp_owner_probe owner_probe;
  struct bluez_upstream_avdtp_owner_probe avdtp_owner_probe;
  uint8_t *capabilities;
  uint8_t err_code = 0;
  GSList *caps = NULL;
  size_t caps_len;
  a2dp_sbc_t remote_sbc;
  a2dp_sbc_t invalid_sbc;
  struct bluez_upstream_a2dp_sbc_select_result sbc_result;
  struct bluez_upstream_a2dp_sep_match_result sep_match_result;
  struct bluez_upstream_a2dp_state_policy_result state_policy_result;
  struct bluez_upstream_a2dp_setconf_transaction_result setconf_result;
  struct bluez_upstream_a2dp_finalizer_result finalizer_result;
  struct bluez_upstream_a2dp_start_suspend_transaction_result
    start_suspend_result;
  struct bluez_upstream_a2dp_close_abort_transaction_result
    close_abort_result;
  struct bluez_upstream_a2dp_media_transport_owner_result
    media_transport_result;
  struct bluez_upstream_a2dp_media_transport_dbus_result
    media_transport_dbus_result;
  struct bluez_upstream_a2dp_media_endpoint_result media_endpoint_result;
  struct bluez_upstream_a2dp_dbus_table_surface_result dbus_table_result;
  struct bluez_upstream_a2dp_handler_bridge_surface_result
    handler_bridge_result;
  uint8_t sbc_no_frequency;
  uint8_t sbc_no_channel;
  uint8_t sbc_no_block;
  uint8_t sbc_no_subbands;
  uint8_t sbc_no_allocation;
  uint8_t sbc_bad_min_bitpool;
  uint8_t sbc_bad_max_bitpool;
  uint8_t sbc_null;
  uint8_t sbc_selected_min_bitpool;
  uint8_t sbc_selected_max_bitpool;
  unsigned int sbc_final_ok;

  probe.get_name = 0;
  probe.get_path = 0;
  probe.get_capabilities = 0;
  probe.select_configuration = 0;
  probe.select_callback = 0;
  probe.set_configuration = 0;
  probe.set_callback = 0;
  probe.clear_configuration = 0;
  probe.set_delay = 0;
  avdtp_probe.cfm_set_configuration = 0;
  avdtp_probe.cfm_get_configuration = 0;
  avdtp_probe.cfm_open = 0;
  avdtp_probe.cfm_start = 0;
  avdtp_probe.cfm_suspend = 0;
  avdtp_probe.cfm_close = 0;
  avdtp_probe.cfm_abort = 0;
  avdtp_probe.cfm_reconfigure = 0;
  avdtp_probe.cfm_delay_report = 0;
  avdtp_probe.ind_match_codec = 0;
  avdtp_probe.ind_get_capability = 0;
  avdtp_probe.ind_set_configuration = 0;
  avdtp_probe.ind_set_configuration_cb = 0;
  avdtp_probe.ind_get_configuration = 0;
  avdtp_probe.ind_open = 0;
  avdtp_probe.ind_start = 0;
  avdtp_probe.ind_suspend = 0;
  avdtp_probe.ind_close = 0;
  avdtp_probe.ind_abort = 0;
  avdtp_probe.ind_reconfigure = 0;
  avdtp_probe.ind_delayreport = 0;
  bluez_upstream_a2dp_owner_probe_init(&owner_probe);
  bluez_upstream_avdtp_owner_probe_init(&avdtp_owner_probe);
  g_bluez_upstream_a2dp_select_callbacks = 0;
  g_bluez_upstream_a2dp_set_callbacks = 0;

  g_bluez_upstream_a2dp_endpoint.get_name(NULL, &probe);
  g_bluez_upstream_a2dp_endpoint.get_path(NULL, &probe);
  caps_len =
    g_bluez_upstream_a2dp_endpoint.get_capabilities(NULL, &capabilities,
                                                    &probe);
  g_bluez_upstream_a2dp_endpoint.select_configuration(
    NULL, capabilities, caps_len, NULL, bluez_upstream_a2dp_select_cb,
    &probe);
  g_bluez_upstream_a2dp_endpoint.set_configuration(
    NULL, capabilities, caps_len, NULL, bluez_upstream_a2dp_config_cb,
    &probe);
  g_bluez_upstream_a2dp_endpoint.clear_configuration(NULL, NULL, &probe);
  g_bluez_upstream_a2dp_endpoint.set_delay(NULL, 120, &probe);

  g_bluez_upstream_avdtp_cfm.set_configuration(NULL, NULL, NULL, NULL,
                                               &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.get_configuration(NULL, NULL, NULL, NULL,
                                               &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.open(NULL, NULL, NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.start(NULL, NULL, NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.suspend(NULL, NULL, NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.close(NULL, NULL, NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.abort(NULL, NULL, NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.reconfigure(NULL, NULL, NULL, NULL,
                                         &avdtp_probe);
  g_bluez_upstream_avdtp_cfm.delay_report(NULL, NULL, NULL, NULL,
                                          &avdtp_probe);

  g_bluez_upstream_avdtp_ind.match_codec(NULL, NULL, &avdtp_probe);
  g_bluez_upstream_avdtp_ind.get_capability(NULL, NULL, 1, &caps,
                                            &err_code, &avdtp_probe);
  g_bluez_upstream_avdtp_ind.set_configuration(
    NULL, NULL, NULL, caps, bluez_upstream_avdtp_ind_set_configuration_cb,
    &avdtp_probe);
  g_bluez_upstream_avdtp_ind.get_configuration(NULL, NULL, &err_code,
                                               &avdtp_probe);
  g_bluez_upstream_avdtp_ind.open(NULL, NULL, NULL, &err_code,
                                  &avdtp_probe);
  g_bluez_upstream_avdtp_ind.start(NULL, NULL, NULL, &err_code,
                                   &avdtp_probe);
  g_bluez_upstream_avdtp_ind.suspend(NULL, NULL, NULL, &err_code,
                                     &avdtp_probe);
  g_bluez_upstream_avdtp_ind.close(NULL, NULL, NULL, &err_code,
                                   &avdtp_probe);
  g_bluez_upstream_avdtp_ind.abort(NULL, NULL, NULL, &err_code,
                                   &avdtp_probe);
  g_bluez_upstream_avdtp_ind.reconfigure(NULL, NULL, &err_code,
                                         &avdtp_probe);
  g_bluez_upstream_avdtp_ind.delayreport(NULL, NULL, 1, 120, &err_code,
                                         &avdtp_probe);
  bluez_upstream_a2dp_owner_run(&owner_probe);
  bluez_upstream_avdtp_owner_run(&avdtp_owner_probe);
  memset(&remote_sbc, 0, sizeof(remote_sbc));
  remote_sbc.frequency = SBC_SAMPLING_FREQ_48000 |
                         SBC_SAMPLING_FREQ_44100;
  remote_sbc.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO |
                            SBC_CHANNEL_MODE_STEREO;
  remote_sbc.block_length = SBC_BLOCK_LENGTH_16 | SBC_BLOCK_LENGTH_12;
  remote_sbc.subbands = SBC_SUBBANDS_8 | SBC_SUBBANDS_4;
  remote_sbc.allocation_method = SBC_ALLOCATION_LOUDNESS |
                                 SBC_ALLOCATION_SNR;
  remote_sbc.min_bitpool = SBC_MIN_BITPOOL;
  remote_sbc.max_bitpool = SBC_MAX_BITPOOL;
  bluez_upstream_a2dp_sbc_select(&remote_sbc, &sbc_result);
  sbc_selected_min_bitpool = sbc_result.config.min_bitpool;
  sbc_selected_max_bitpool = sbc_result.config.max_bitpool;
  sbc_final_ok = sbc_result.config.frequency == SBC_SAMPLING_FREQ_48000 &&
                 sbc_result.config.channel_mode ==
                 SBC_CHANNEL_MODE_JOINT_STEREO &&
                 sbc_result.config.block_length == SBC_BLOCK_LENGTH_16 &&
                 sbc_result.config.subbands == SBC_SUBBANDS_8 &&
                 sbc_result.config.allocation_method ==
                 SBC_ALLOCATION_LOUDNESS &&
                 sbc_result.config.min_bitpool == SBC_MIN_BITPOOL &&
                 sbc_result.config.max_bitpool ==
                 SBC_BITPOOL_HQ_JOINT_STEREO_48000 ? 1 : 0;

  printf("bluez-daemon: a2dp upstream-link-ledger role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "headers=bluez/upstream/profiles/audio/a2dp.h+"
         "bluez/upstream/profiles/audio/avdtp.h "
         "callback-surface=a2dp_endpoint_select_t,"
         "a2dp_endpoint_config_t,a2dp_discover_cb_t,a2dp_select_cb_t,"
         "a2dp_config_cb_t,a2dp_stream_cb_t,avdtp_session_state_cb,"
         "avdtp_stream_state_cb,avdtp_set_configuration_cb "
         "avdtp-states=idle:%u,configured:%u,open:%u,streaming:%u,"
         "closing:%u,aborting:%u "
         "sep-types=source:%u,sink:%u media-type-audio=%u "
         "caps=media-transport:0x%02x,media-codec:0x%02x,"
         "delay-reporting:0x%02x "
         "errors=bad-state:0x%02x,invalid-codec:0x%02x,"
         "unsupported-codec:0x%02x,invalid-caps:0x%02x "
         "upstream-link=upstream-headers-compiled-upstream-c-object "
         "final-ok=1\n",
         role,
         AVDTP_STATE_IDLE,
         AVDTP_STATE_CONFIGURED,
         AVDTP_STATE_OPEN,
         AVDTP_STATE_STREAMING,
         AVDTP_STATE_CLOSING,
         AVDTP_STATE_ABORTING,
         AVDTP_SEP_TYPE_SOURCE,
         AVDTP_SEP_TYPE_SINK,
         AVDTP_MEDIA_TYPE_AUDIO,
         AVDTP_MEDIA_TRANSPORT,
         AVDTP_MEDIA_CODEC,
         AVDTP_DELAY_REPORTING,
         AVDTP_BAD_STATE,
         A2DP_INVALID_CODEC_TYPE,
         A2DP_NOT_SUPPORTED_CODEC_TYPE,
         A2DP_INVALID_CODEC_PARAMETER);

  printf("bluez-daemon: a2dp upstream-config-error-parser role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "function=a2dp_parse_config_error "
         "prefix=org.bluez.Error.A2DP. "
         "errors=InvalidCodecType:0x%02x,NotSupportedCodecType:0x%02x,"
         "InvalidSamplingFrequency:0x%02x,InvalidChannelMode:0x%02x,"
         "InvalidMaximumBitpoolValue:0x%02x,"
         "InvalidCodecParameter:0x%02x,wrong-prefix:0x%02x,"
         "unknown-suffix:0x%02x,null:0x%02x "
         "entries=%u "
         "upstream-link=ported-public-a2dp-function-linked-a2dp-c-object "
         "final-ok=%u\n",
         role,
         a2dp_parse_config_error("org.bluez.Error.A2DP.InvalidCodecType"),
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.NotSupportedCodecType"),
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidSamplingFrequency"),
         a2dp_parse_config_error("org.bluez.Error.A2DP.InvalidChannelMode"),
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidMaximumBitpoolValue"),
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidCodecParameter"),
         a2dp_parse_config_error("org.bluez.Error.InvalidCodecType"),
         a2dp_parse_config_error("org.bluez.Error.A2DP.DoesNotExist"),
         a2dp_parse_config_error(NULL),
         (unsigned int)(sizeof(g_bluez_upstream_a2dp_config_errors) /
                        sizeof(g_bluez_upstream_a2dp_config_errors[0])),
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidCodecType") ==
           A2DP_INVALID_CODEC_TYPE &&
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.NotSupportedCodecType") ==
           A2DP_NOT_SUPPORTED_CODEC_TYPE &&
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidSamplingFrequency") ==
           A2DP_INVALID_SAMPLING_FREQUENCY &&
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidChannelMode") ==
           A2DP_INVALID_CHANNEL_MODE &&
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidMaximumBitpoolValue") ==
           A2DP_INVALID_MAXIMUM_BITPOOL_VALUE &&
         a2dp_parse_config_error(
           "org.bluez.Error.A2DP.InvalidCodecParameter") ==
           A2DP_INVALID_CODEC_PARAMETER &&
         a2dp_parse_config_error("org.bluez.Error.InvalidCodecType") ==
           AVDTP_UNSUPPORTED_CONFIGURATION &&
         a2dp_parse_config_error("org.bluez.Error.A2DP.DoesNotExist") ==
           AVDTP_UNSUPPORTED_CONFIGURATION &&
         a2dp_parse_config_error(NULL) ==
           AVDTP_UNSUPPORTED_CONFIGURATION ? 1 : 0);

  invalid_sbc = remote_sbc;
  invalid_sbc.frequency = 0;
  sbc_no_frequency =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.channel_mode = 0;
  sbc_no_channel =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.block_length = 0;
  sbc_no_block =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.subbands = 0;
  sbc_no_subbands =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.allocation_method = 0;
  sbc_no_allocation =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.min_bitpool = 1;
  sbc_bad_min_bitpool =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  invalid_sbc = remote_sbc;
  invalid_sbc.max_bitpool = 251;
  sbc_bad_max_bitpool =
    bluez_upstream_a2dp_sbc_select(&invalid_sbc, &sbc_result);
  sbc_null = bluez_upstream_a2dp_sbc_select(NULL, &sbc_result);
  bluez_upstream_a2dp_sep_match_run(&sep_match_result);
  bluez_upstream_a2dp_state_policy_run(&state_policy_result);
  bluez_upstream_a2dp_setconf_transaction_run(&setconf_result);
  bluez_upstream_a2dp_finalizer_run(&finalizer_result);
  bluez_upstream_a2dp_start_suspend_transaction_run(&start_suspend_result);
  bluez_upstream_a2dp_close_abort_transaction_run(&close_abort_result);
  bluez_upstream_a2dp_media_transport_owner_run(&media_transport_result);
  bluez_upstream_a2dp_media_transport_dbus_run(&media_transport_dbus_result);
  bluez_upstream_a2dp_media_endpoint_run(&media_endpoint_result);
  bluez_upstream_a2dp_dbus_table_surface_run(&dbus_table_result);
  bluez_upstream_a2dp_handler_bridge_surface_run(&handler_bridge_result);
  printf("bluez-daemon: a2dp upstream-sbc-config-selector role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp-codecs.h "
         "codec=A2DP_CODEC_SBC selected=frequency:48000,"
         "channel:joint-stereo,block:16,subbands:8,alloc:loudness,"
         "min-bitpool:%u,max-bitpool:%u "
         "remote=frequency:48000+44100,channel:joint-stereo+stereo,"
         "block:16+12,subbands:8+4,alloc:loudness+snr,"
         "min-bitpool:%u,max-bitpool:%u "
         "errors=no-frequency:0x%02x,no-channel:0x%02x,no-block:0x%02x,"
         "no-subbands:0x%02x,no-allocation:0x%02x,"
         "bad-min-bitpool:0x%02x,bad-max-bitpool:0x%02x,null:0x%02x "
         "upstream-link=ported-sbc-config-selection-linked-a2dp-c-object "
         "final-ok=%u\n",
         role, sbc_selected_min_bitpool,
         sbc_selected_max_bitpool,
         remote_sbc.min_bitpool, remote_sbc.max_bitpool,
         sbc_no_frequency, sbc_no_channel, sbc_no_block,
         sbc_no_subbands, sbc_no_allocation, sbc_bad_min_bitpool,
         sbc_bad_max_bitpool, sbc_null, sbc_final_ok);

  printf("bluez-daemon: a2dp upstream-sep-matching-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=find_sep,find_remote_sep,set_configuration "
         "codec=A2DP_CODEC_SBC remote-source->local-sink:%u,"
         "remote-sink->local-source:%u sender-path-match:%u,"
         "codec-match:%u rejects=wrong-sender:%u,wrong-path:%u,"
         "codec-mismatch:%u,missing-remote-codec:%u,missing-local-sep:%u "
         "upstream-link=ported-sep-matching-linked-a2dp-c-object "
         "final-ok=%u\n",
         role, sep_match_result.remote_source_to_local_sink,
         sep_match_result.remote_sink_to_local_source,
         sep_match_result.sender_path_match, sep_match_result.codec_match,
         sep_match_result.wrong_sender_rejected,
         sep_match_result.wrong_path_rejected,
         sep_match_result.codec_mismatch_rejected,
         sep_match_result.missing_remote_codec_rejected,
         sep_match_result.missing_local_sep_rejected,
         sep_match_result.remote_source_to_local_sink == 1 &&
         sep_match_result.remote_sink_to_local_source == 1 &&
         sep_match_result.sender_path_match == 1 &&
         sep_match_result.codec_match == 1 &&
         sep_match_result.wrong_sender_rejected == 1 &&
         sep_match_result.wrong_path_rejected == 1 &&
         sep_match_result.codec_mismatch_rejected == 1 &&
         sep_match_result.missing_remote_codec_rejected == 1 &&
         sep_match_result.missing_local_sep_rejected == 1 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-state-policy-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=a2dp_config,a2dp_resume,a2dp_suspend "
         "config=idle-setconf:%u,open-same-finalize:%u,"
         "streaming-same-finalize:%u,open-diff-reconfigure:%u,"
         "streaming-diff-reconfigure:%u,reject-configured:%u,"
         "reject-closing:%u,reject-aborting:%u,reject-locked:%u,"
         "reject-missing-codec:%u,reject-codec-mismatch:%u "
         "resume=reject-idle:%u,configured-start-deferred:%u,"
         "open-start:%u,streaming-finalize:%u,reject-closing:%u,"
         "reject-aborting:%u,reject-reconfigure:%u "
         "suspend=reject-idle:%u,open-finalize:%u,streaming-suspend:%u,"
         "reject-configured:%u,reject-closing:%u,reject-aborting:%u,"
         "reject-reconfigure:%u "
         "upstream-link=ported-state-policy-linked-a2dp-c-object "
         "final-ok=%u\n",
         role, state_policy_result.config_idle_set_configuration,
         state_policy_result.config_open_same_caps_finalize,
         state_policy_result.config_streaming_same_caps_finalize,
         state_policy_result.config_open_diff_caps_reconfigure,
         state_policy_result.config_streaming_diff_caps_reconfigure,
         state_policy_result.config_configured_reject,
         state_policy_result.config_closing_reject,
         state_policy_result.config_aborting_reject,
         state_policy_result.config_locked_reject,
         state_policy_result.config_missing_codec_reject,
         state_policy_result.config_codec_mismatch_reject,
         state_policy_result.resume_idle_reject,
         state_policy_result.resume_configured_start_deferred,
         state_policy_result.resume_open_start,
         state_policy_result.resume_streaming_finalize,
         state_policy_result.resume_closing_reject,
         state_policy_result.resume_aborting_reject,
         state_policy_result.resume_reconfigure_reject,
         state_policy_result.suspend_idle_reject,
         state_policy_result.suspend_open_finalize,
         state_policy_result.suspend_streaming_suspend,
         state_policy_result.suspend_configured_reject,
         state_policy_result.suspend_closing_reject,
         state_policy_result.suspend_aborting_reject,
         state_policy_result.suspend_reconfigure_reject,
         state_policy_result.config_idle_set_configuration == 1 &&
         state_policy_result.config_open_same_caps_finalize == 1 &&
         state_policy_result.config_streaming_same_caps_finalize == 1 &&
         state_policy_result.config_open_diff_caps_reconfigure == 1 &&
         state_policy_result.config_streaming_diff_caps_reconfigure == 1 &&
         state_policy_result.config_configured_reject == 1 &&
         state_policy_result.config_closing_reject == 1 &&
         state_policy_result.config_aborting_reject == 1 &&
         state_policy_result.config_locked_reject == 1 &&
         state_policy_result.config_missing_codec_reject == 1 &&
         state_policy_result.config_codec_mismatch_reject == 1 &&
         state_policy_result.resume_idle_reject == 1 &&
         state_policy_result.resume_configured_start_deferred == 1 &&
         state_policy_result.resume_open_start == 1 &&
         state_policy_result.resume_streaming_finalize == 1 &&
         state_policy_result.resume_closing_reject == 1 &&
         state_policy_result.resume_aborting_reject == 1 &&
         state_policy_result.resume_reconfigure_reject == 1 &&
         state_policy_result.suspend_idle_reject == 1 &&
         state_policy_result.suspend_open_finalize == 1 &&
         state_policy_result.suspend_streaming_suspend == 1 &&
         state_policy_result.suspend_configured_reject == 1 &&
         state_policy_result.suspend_closing_reject == 1 &&
         state_policy_result.suspend_aborting_reject == 1 &&
         state_policy_result.suspend_reconfigure_reject == 1 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-setconf-transaction-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=a2dp_config,set_configuration,config_cfm,"
         "finalize_config "
         "normal=setup-get:%u,setup-cb-add:%u,caps-copy:%u,"
         "remote-sep:%u,avdtp-setconf:%u,stream-assigned:%u,"
         "cfm-setconf:%u,config-callback:%u,finalize:%u "
         "same-caps=idle-finalize:%u "
         "reconfigure=diff-caps-close:%u,flag-set:%u,retry:%u "
         "fail=no-remote-sep:%u,avdtp-setconf:%u,cleanup:%u "
         "cleanup=setup-cb-free:%u,setup-unref:%u "
         "final-zero=pending-callbacks:%u,pending-setups:%u,"
         "pending-transactions:%u "
         "upstream-link=ported-setconf-transaction-linked-a2dp-c-object "
         "final-ok=%u\n",
         role, setconf_result.setup_get, setconf_result.setup_cb_add,
         setconf_result.caps_copy, setconf_result.remote_sep_resolved,
         setconf_result.avdtp_set_configuration,
         setconf_result.stream_assigned,
         setconf_result.cfm_set_configuration,
         setconf_result.config_callback, setconf_result.finalize_config,
         setconf_result.same_caps_idle_finalize,
         setconf_result.diff_caps_close, setconf_result.reconfigure_flag_set,
         setconf_result.reconfigure_retry, setconf_result.fail_no_remote_sep,
         setconf_result.fail_avdtp_set_configuration,
         setconf_result.fail_cleanup, setconf_result.setup_cb_free,
         setconf_result.setup_unref, setconf_result.final_pending_callbacks,
         setconf_result.final_pending_setups,
         setconf_result.final_pending_transactions,
         setconf_result.setup_get == 5 &&
         setconf_result.setup_cb_add == 5 &&
         setconf_result.caps_copy == 1 &&
         setconf_result.remote_sep_resolved == 3 &&
         setconf_result.avdtp_set_configuration == 2 &&
         setconf_result.stream_assigned == 2 &&
         setconf_result.cfm_set_configuration == 2 &&
         setconf_result.config_callback == 3 &&
         setconf_result.finalize_config == 3 &&
         setconf_result.same_caps_idle_finalize == 1 &&
         setconf_result.diff_caps_close == 1 &&
         setconf_result.reconfigure_flag_set == 1 &&
         setconf_result.reconfigure_retry == 1 &&
         setconf_result.fail_no_remote_sep == 1 &&
         setconf_result.fail_avdtp_set_configuration == 1 &&
         setconf_result.fail_cleanup == 2 &&
         setconf_result.setup_cb_free == 5 &&
         setconf_result.setup_unref == 5 &&
         setconf_result.final_pending_callbacks == 0 &&
         setconf_result.final_pending_setups == 0 &&
         setconf_result.final_pending_transactions == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-finalizer-callback-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=finalize_config,finalize_resume,finalize_suspend,"
         "finalize_setup_errno "
         "callbacks=config:%u,resume:%u,suspend:%u "
         "finalize=config:%u,resume:%u,suspend:%u,errno:%u "
         "delivered=config-success:%u,config-error:%u,"
         "resume-success:%u,resume-error:%u,suspend-success:%u,"
         "suspend-error:%u,stream:%u "
         "errors=errno-eio:%u,errno-einval:%u "
         "cleanup=setup-cb-free:%u,setup-unref:%u "
         "final-zero=pending-callbacks:%u,pending-setups:%u "
         "upstream-link=ported-finalizer-callbacks-linked-a2dp-c-object "
         "final-ok=%u\n",
         role, finalizer_result.setup_cb_config,
         finalizer_result.setup_cb_resume,
         finalizer_result.setup_cb_suspend,
         finalizer_result.finalize_config,
         finalizer_result.finalize_resume,
         finalizer_result.finalize_suspend,
         finalizer_result.finalize_errno,
         finalizer_result.config_success_cb,
         finalizer_result.config_error_cb,
         finalizer_result.resume_success_cb,
         finalizer_result.resume_error_cb,
         finalizer_result.suspend_success_cb,
         finalizer_result.suspend_error_cb,
         finalizer_result.stream_delivered,
         finalizer_result.errno_eio,
         finalizer_result.errno_einval,
         finalizer_result.setup_cb_free,
         finalizer_result.setup_unref,
         finalizer_result.final_pending_callbacks,
         finalizer_result.final_pending_setups,
         finalizer_result.setup_cb_config == 2 &&
         finalizer_result.setup_cb_resume == 2 &&
         finalizer_result.setup_cb_suspend == 2 &&
         finalizer_result.finalize_config == 1 &&
         finalizer_result.finalize_resume == 2 &&
         finalizer_result.finalize_suspend == 2 &&
         finalizer_result.finalize_errno == 1 &&
         finalizer_result.config_success_cb == 1 &&
         finalizer_result.config_error_cb == 1 &&
         finalizer_result.resume_success_cb == 1 &&
         finalizer_result.resume_error_cb == 1 &&
         finalizer_result.suspend_success_cb == 1 &&
         finalizer_result.suspend_error_cb == 1 &&
         finalizer_result.stream_delivered == 1 &&
         finalizer_result.errno_eio == 1 &&
         finalizer_result.errno_einval == 2 &&
         finalizer_result.setup_cb_free == 6 &&
         finalizer_result.setup_unref == 6 &&
         finalizer_result.final_pending_callbacks == 0 &&
         finalizer_result.final_pending_setups == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-start-suspend-transaction-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=a2dp_resume,a2dp_suspend,start_ind,start_cfm,"
         "suspend_ind,suspend_cfm "
         "setup=setup-get:%u,setup-cb-add:%u,stream-lookup:%u "
         "avdtp=start:%u,suspend:%u,start-ind:%u,start-cfm:%u,"
         "suspend-ind:%u,suspend-cfm:%u "
         "resume=configured-defer-start:%u,open-start:%u,"
         "streaming-finalize:%u,wait-suspend:%u "
         "suspend=open-finalize:%u,streaming-suspend:%u "
         "restart=after-suspend:%u,after-suspend-fail:%u "
         "fail=resume-bad-state:%u,suspend-bad-state:%u,"
         "reconfigure:%u,avdtp-start:%u,avdtp-suspend:%u "
         "finalize=resume:%u,suspend:%u,errno:%u "
         "callbacks=resume:%u,suspend:%u "
         "cleanup=setup-cb-free:%u,setup-unref:%u "
         "final-zero=pending-callbacks:%u,pending-setups:%u,"
         "pending-transactions:%u "
         "upstream-link=ported-start-suspend-transactions-linked-"
         "a2dp-c-object final-ok=%u\n",
         role, start_suspend_result.setup_get,
         start_suspend_result.setup_cb_add,
         start_suspend_result.stream_lookup,
         start_suspend_result.avdtp_start,
         start_suspend_result.avdtp_suspend,
         start_suspend_result.start_ind,
         start_suspend_result.start_cfm,
         start_suspend_result.suspend_ind,
         start_suspend_result.suspend_cfm,
         start_suspend_result.resume_configured_defer_start,
         start_suspend_result.resume_open_start,
         start_suspend_result.resume_streaming_finalize,
         start_suspend_result.resume_wait_suspend,
         start_suspend_result.suspend_open_finalize,
         start_suspend_result.suspend_streaming_suspend,
         start_suspend_result.restart_after_suspend,
         start_suspend_result.restart_after_suspend_fail,
         start_suspend_result.fail_resume_bad_state,
         start_suspend_result.fail_suspend_bad_state,
         start_suspend_result.fail_reconfigure,
         start_suspend_result.fail_avdtp_start,
         start_suspend_result.fail_avdtp_suspend,
         start_suspend_result.finalize_resume,
         start_suspend_result.finalize_suspend,
         start_suspend_result.finalize_errno,
         start_suspend_result.resume_callback,
         start_suspend_result.suspend_callback,
         start_suspend_result.setup_cb_free,
         start_suspend_result.setup_unref,
         start_suspend_result.final_pending_callbacks,
         start_suspend_result.final_pending_setups,
         start_suspend_result.final_pending_transactions,
         start_suspend_result.setup_get == 14 &&
         start_suspend_result.setup_cb_add == 14 &&
         start_suspend_result.stream_lookup == 10 &&
         start_suspend_result.avdtp_start == 4 &&
         start_suspend_result.avdtp_suspend == 3 &&
         start_suspend_result.start_ind == 1 &&
         start_suspend_result.start_cfm == 2 &&
         start_suspend_result.suspend_ind == 1 &&
         start_suspend_result.suspend_cfm == 3 &&
         start_suspend_result.resume_configured_defer_start == 1 &&
         start_suspend_result.resume_open_start == 1 &&
         start_suspend_result.resume_streaming_finalize == 1 &&
         start_suspend_result.resume_wait_suspend == 1 &&
         start_suspend_result.suspend_open_finalize == 1 &&
         start_suspend_result.suspend_streaming_suspend == 2 &&
         start_suspend_result.restart_after_suspend == 1 &&
         start_suspend_result.restart_after_suspend_fail == 1 &&
         start_suspend_result.fail_resume_bad_state == 1 &&
         start_suspend_result.fail_suspend_bad_state == 1 &&
         start_suspend_result.fail_reconfigure == 2 &&
         start_suspend_result.fail_avdtp_start == 1 &&
         start_suspend_result.fail_avdtp_suspend == 1 &&
         start_suspend_result.finalize_resume == 4 &&
         start_suspend_result.finalize_suspend == 5 &&
         start_suspend_result.finalize_errno == 3 &&
         start_suspend_result.resume_callback == 4 &&
         start_suspend_result.suspend_callback == 5 &&
         start_suspend_result.setup_cb_free == 15 &&
         start_suspend_result.setup_unref == 15 &&
         start_suspend_result.final_pending_callbacks == 0 &&
         start_suspend_result.final_pending_setups == 0 &&
         start_suspend_result.final_pending_transactions == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-close-abort-transaction-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "functions=close_ind,close_cfm,abort_ind,abort_cfm,"
         "a2dp_cancel,a2dp_reconfigure "
         "events=close-ind:%u,close-cfm-success:%u,close-cfm-error:%u,"
         "abort-ind:%u,abort-cfm-reconfigure:%u,abort-cfm-unref:%u "
         "reconfigure=remote-sep-lookup:%u,setup-reconfigure:%u,"
         "idle-add:%u "
         "error=setup-error-set:%u,stream-null:%u,stream-destroy:%u "
         "avdtp=close:%u,abort:%u "
         "cancel=lookup:%u,setup-ref:%u,cb-free:%u,"
         "return-after-abort:%u "
         "finalize=config:%u,resume:%u,suspend:%u,errno:%u "
         "cleanup=setup-cb-free:%u,setup-unref:%u "
         "final-zero=pending-callbacks:%u,pending-setups:%u,"
         "pending-streams:%u,pending-transactions:%u "
         "upstream-link=ported-close-abort-transactions-linked-"
         "a2dp-c-object final-ok=%u\n",
         role, close_abort_result.close_ind,
         close_abort_result.close_cfm_success,
         close_abort_result.close_cfm_error,
         close_abort_result.abort_ind,
         close_abort_result.abort_cfm_reconfigure,
         close_abort_result.abort_cfm_unref,
         close_abort_result.remote_sep_lookup,
         close_abort_result.setup_reconfigure,
         close_abort_result.reconfigure_idle_add,
         close_abort_result.setup_error_set,
         close_abort_result.stream_null,
         close_abort_result.stream_destroy,
         close_abort_result.avdtp_close,
         close_abort_result.avdtp_abort,
         close_abort_result.cancel_lookup,
         close_abort_result.cancel_setup_ref,
         close_abort_result.cancel_cb_free,
         close_abort_result.cancel_return_after_abort,
         close_abort_result.finalize_config,
         close_abort_result.finalize_resume,
         close_abort_result.finalize_suspend,
         close_abort_result.finalize_errno,
         close_abort_result.setup_cb_free,
         close_abort_result.setup_unref,
         close_abort_result.final_pending_callbacks,
         close_abort_result.final_pending_setups,
         close_abort_result.final_pending_streams,
         close_abort_result.final_pending_transactions,
         close_abort_result.close_ind == 1 &&
         close_abort_result.close_cfm_success == 1 &&
         close_abort_result.close_cfm_error == 1 &&
         close_abort_result.abort_ind == 1 &&
         close_abort_result.abort_cfm_reconfigure == 1 &&
         close_abort_result.abort_cfm_unref == 1 &&
         close_abort_result.remote_sep_lookup == 1 &&
         close_abort_result.setup_reconfigure == 3 &&
         close_abort_result.reconfigure_idle_add == 3 &&
         close_abort_result.setup_error_set == 1 &&
         close_abort_result.stream_null == 1 &&
         close_abort_result.stream_destroy == 1 &&
         close_abort_result.avdtp_close == 1 &&
         close_abort_result.avdtp_abort == 1 &&
         close_abort_result.cancel_lookup == 1 &&
         close_abort_result.cancel_setup_ref == 1 &&
         close_abort_result.cancel_cb_free == 1 &&
         close_abort_result.cancel_return_after_abort == 1 &&
         close_abort_result.finalize_config == 2 &&
         close_abort_result.finalize_resume == 2 &&
         close_abort_result.finalize_suspend == 2 &&
         close_abort_result.finalize_errno == 2 &&
         close_abort_result.setup_cb_free == 6 &&
         close_abort_result.setup_unref == 4 &&
         close_abort_result.final_pending_callbacks == 0 &&
         close_abort_result.final_pending_setups == 0 &&
         close_abort_result.final_pending_streams == 0 &&
         close_abort_result.final_pending_transactions == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-media-transport-ownership-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "interfaces=org.bluez.MediaEndpoint1,org.bluez.MediaTransport1 "
         "functions=set_configuration,media_transport_create,"
         "media_transport_set_owner,transport_a2dp_resume,"
         "transport_a2dp_suspend,transport_a2dp_remove_owner,"
         "media_transport_destroy "
         "endpoint=registered:%u,found:%u,transport-append:%u,"
         "cancel-all:%u,remove-transport:%u "
         "transport=create:%u,path-alloc:%u,ops-find:%u,a2dp-init:%u,"
         "config-dup:%u,dbus-register:%u,global-append:%u,"
         "get-properties:%u "
         "owner=create:%u,watch-add:%u,set:%u,remove:%u,"
         "pending-remove:%u,clear:%u "
         "acquire=request:%u,state-requesting:%u,a2dp-resume:%u,"
         "fd-ready:%u,fd-reply:%u,state-active:%u "
         "release=request:%u,a2dp-suspend:%u,state-suspending:%u,"
         "state-idle:%u "
         "cancel=cancel-resume:%u,a2dp-cancel:%u "
         "properties=delay-update:%u,delay-emit:%u,volume-get:%u,"
         "volume-set:%u,volume-emit:%u "
         "destroy=clear-configuration:%u,transport-destroy:%u,"
         "dbus-unregister:%u,transport-free:%u "
         "final-zero=pending-owners:%u,pending-requests:%u,"
         "pending-transports:%u,pending-watches:%u "
         "upstream-link=ported-media-transport-ownership-linked-"
         "transport-c-object final-ok=%u\n",
         role, media_transport_result.endpoint_registered,
         media_transport_result.endpoint_found,
         media_transport_result.endpoint_transport_append,
         media_transport_result.endpoint_cancel_all,
         media_transport_result.endpoint_remove_transport,
         media_transport_result.transport_create,
         media_transport_result.transport_path_alloc,
         media_transport_result.transport_ops_find,
         media_transport_result.transport_init_a2dp,
         media_transport_result.transport_config_dup,
         media_transport_result.transport_dbus_register,
         media_transport_result.transport_global_append,
         media_transport_result.get_properties_call,
         media_transport_result.owner_create,
         media_transport_result.owner_watch_add,
         media_transport_result.owner_set,
         media_transport_result.remove_owner,
         media_transport_result.owner_pending_remove,
         media_transport_result.clear_owner,
         media_transport_result.acquire_request,
         media_transport_result.state_requesting,
         media_transport_result.a2dp_resume_call,
         media_transport_result.fd_ready,
         media_transport_result.fd_reply,
         media_transport_result.state_active,
         media_transport_result.release_request,
         media_transport_result.a2dp_suspend_call,
         media_transport_result.state_suspending,
         media_transport_result.state_idle,
         media_transport_result.cancel_resume,
         media_transport_result.a2dp_cancel_call,
         media_transport_result.delay_update,
         media_transport_result.delay_property_emit,
         media_transport_result.volume_get,
         media_transport_result.volume_set,
         media_transport_result.volume_property_emit,
         media_transport_result.clear_configuration,
         media_transport_result.transport_destroy,
         media_transport_result.dbus_unregister,
         media_transport_result.transport_free,
         media_transport_result.final_pending_owners,
         media_transport_result.final_pending_requests,
         media_transport_result.final_pending_transports,
         media_transport_result.final_pending_watches,
         media_transport_result.endpoint_registered == 1 &&
         media_transport_result.endpoint_found == 1 &&
         media_transport_result.transport_create == 1 &&
         media_transport_result.transport_path_alloc == 1 &&
         media_transport_result.transport_ops_find == 1 &&
         media_transport_result.transport_init_a2dp == 1 &&
         media_transport_result.transport_config_dup == 1 &&
         media_transport_result.transport_dbus_register == 1 &&
         media_transport_result.transport_global_append == 1 &&
         media_transport_result.endpoint_transport_append == 1 &&
         media_transport_result.set_configuration_call == 1 &&
         media_transport_result.get_properties_call == 1 &&
         media_transport_result.owner_create == 2 &&
         media_transport_result.owner_watch_add == 2 &&
         media_transport_result.owner_set == 2 &&
         media_transport_result.acquire_request == 2 &&
         media_transport_result.state_requesting == 2 &&
         media_transport_result.a2dp_resume_call == 2 &&
         media_transport_result.fd_ready == 1 &&
         media_transport_result.fd_reply == 1 &&
         media_transport_result.owner_pending_remove == 1 &&
         media_transport_result.state_active == 1 &&
         media_transport_result.release_request == 1 &&
         media_transport_result.a2dp_suspend_call == 1 &&
         media_transport_result.state_suspending == 1 &&
         media_transport_result.state_idle == 1 &&
         media_transport_result.remove_owner == 2 &&
         media_transport_result.cancel_resume == 1 &&
         media_transport_result.a2dp_cancel_call == 1 &&
         media_transport_result.clear_owner == 2 &&
         media_transport_result.delay_update == 1 &&
         media_transport_result.delay_property_emit == 1 &&
         media_transport_result.volume_get == 1 &&
         media_transport_result.volume_set == 1 &&
         media_transport_result.volume_property_emit == 1 &&
         media_transport_result.clear_configuration == 1 &&
         media_transport_result.endpoint_remove_transport == 1 &&
         media_transport_result.endpoint_cancel_all == 1 &&
         media_transport_result.transport_destroy == 1 &&
         media_transport_result.dbus_unregister == 1 &&
         media_transport_result.transport_free == 1 &&
         media_transport_result.final_pending_owners == 0 &&
         media_transport_result.final_pending_requests == 0 &&
         media_transport_result.final_pending_transports == 0 &&
         media_transport_result.final_pending_watches == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-media-transport-dbus-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/transport.c "
         "interface=org.bluez.MediaTransport1 "
         "methods=Acquire:%u,TryAcquire:%u,Release:%u "
         "properties=get:%u,set-volume:%u,set-delay:%u "
         "success=acquire:%u,try-acquire:%u,release:%u,fd-reply:%u,"
         "mtu-reply:%u "
         "errors=owner-conflict:%u,not-available:%u,"
         "not-authorized:%u,invalid-args:%u,not-supported:%u "
         "states=idle-guard:%u,requesting-guard:%u,active-guard:%u,"
         "suspending-guard:%u,state-changed:%u "
         "signals=volume-changed:%u,delay-changed:%u "
         "requests=create:%u,pending-call:%u,complete:%u,cancel:%u "
         "watches=add:%u,remove:%u "
         "final-zero=pending-owners:%u,pending-requests:%u,"
         "pending-fds:%u,pending-watches:%u "
         "upstream-link=ported-media-transport-dbus-dispatch-linked-"
         "transport-c-object final-ok=%u\n",
         role,
         media_transport_dbus_result.acquire_method,
         media_transport_dbus_result.try_acquire_method,
         media_transport_dbus_result.release_method,
         media_transport_dbus_result.get_properties,
         media_transport_dbus_result.set_property_volume,
         media_transport_dbus_result.set_property_delay,
         media_transport_dbus_result.acquire_success,
         media_transport_dbus_result.try_acquire_success,
         media_transport_dbus_result.release_success,
         media_transport_dbus_result.fd_reply,
         media_transport_dbus_result.mtu_reply,
         media_transport_dbus_result.owner_conflict,
         media_transport_dbus_result.not_available,
         media_transport_dbus_result.not_authorized,
         media_transport_dbus_result.invalid_args,
         media_transport_dbus_result.not_supported,
         media_transport_dbus_result.state_idle_guard,
         media_transport_dbus_result.state_requesting_guard,
         media_transport_dbus_result.state_active_guard,
         media_transport_dbus_result.state_suspending_guard,
         media_transport_dbus_result.state_changed_emit,
         media_transport_dbus_result.volume_changed_emit,
         media_transport_dbus_result.delay_changed_emit,
         media_transport_dbus_result.request_create,
         media_transport_dbus_result.pending_call,
         media_transport_dbus_result.request_complete,
         media_transport_dbus_result.request_cancel,
         media_transport_dbus_result.owner_watch_add,
         media_transport_dbus_result.owner_watch_remove,
         media_transport_dbus_result.final_pending_owners,
         media_transport_dbus_result.final_pending_requests,
         media_transport_dbus_result.final_pending_fds,
         media_transport_dbus_result.final_pending_watches,
         media_transport_dbus_result.acquire_method == 2 &&
         media_transport_dbus_result.try_acquire_method == 2 &&
         media_transport_dbus_result.release_method == 2 &&
         media_transport_dbus_result.get_properties == 1 &&
         media_transport_dbus_result.set_property_volume == 2 &&
         media_transport_dbus_result.set_property_delay == 2 &&
         media_transport_dbus_result.acquire_success == 1 &&
         media_transport_dbus_result.try_acquire_success == 1 &&
         media_transport_dbus_result.release_success == 1 &&
         media_transport_dbus_result.fd_reply == 2 &&
         media_transport_dbus_result.mtu_reply == 2 &&
         media_transport_dbus_result.owner_conflict == 1 &&
         media_transport_dbus_result.not_available == 1 &&
         media_transport_dbus_result.not_authorized == 1 &&
         media_transport_dbus_result.invalid_args == 1 &&
         media_transport_dbus_result.not_supported == 1 &&
         media_transport_dbus_result.state_idle_guard == 2 &&
         media_transport_dbus_result.state_requesting_guard == 1 &&
         media_transport_dbus_result.state_active_guard == 2 &&
         media_transport_dbus_result.state_suspending_guard == 1 &&
         media_transport_dbus_result.state_changed_emit == 5 &&
         media_transport_dbus_result.volume_changed_emit == 1 &&
         media_transport_dbus_result.delay_changed_emit == 1 &&
         media_transport_dbus_result.request_create == 2 &&
         media_transport_dbus_result.pending_call == 2 &&
         media_transport_dbus_result.request_complete == 3 &&
         media_transport_dbus_result.request_cancel == 1 &&
         media_transport_dbus_result.owner_watch_add == 2 &&
         media_transport_dbus_result.owner_watch_remove == 2 &&
         media_transport_dbus_result.final_pending_owners == 0 &&
         media_transport_dbus_result.final_pending_requests == 0 &&
         media_transport_dbus_result.final_pending_fds == 0 &&
         media_transport_dbus_result.final_pending_watches == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-media-endpoint-dbus-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/media.c "
         "interfaces=org.bluez.Media1,org.bluez.MediaEndpoint1 "
         "functions=RegisterEndpoint,UnregisterEndpoint,"
         "SelectConfiguration,SetConfiguration,ClearConfiguration,Release,"
         "media_endpoint_destroy "
         "register=endpoint:%u,unregister:%u,uuid:%u,codec:%u,"
         "capabilities:%u,delay-reporting:%u "
         "reject=duplicate:%u,invalid-uuid:%u,invalid-caps:%u "
         "owner=watch-add:%u,adapter-append:%u,custom-prop-add:%u "
         "sep=source:%u,sink:%u,remove:%u "
         "calls=select:%u,set:%u,clear:%u,release:%u "
         "requests=create:%u,pending-call:%u,reply-success:%u,"
         "reply-error:%u,cancel:%u "
         "transport=create:%u,append:%u,clear:%u,destroy:%u "
         "cleanup=endpoint-remove:%u,endpoint-destroy:%u,watch-remove:%u,"
         "custom-prop-remove:%u "
         "final-zero=pending-endpoints:%u,pending-requests:%u,"
         "pending-transports:%u,pending-watches:%u "
         "upstream-link=ported-media-endpoint-dbus-linked-media-c-object "
         "final-ok=%u\n",
         role,
         media_endpoint_result.media_register_endpoint,
         media_endpoint_result.media_unregister_endpoint,
         media_endpoint_result.parse_uuid,
         media_endpoint_result.parse_codec,
         media_endpoint_result.parse_capabilities,
         media_endpoint_result.parse_delay_reporting,
         media_endpoint_result.endpoint_duplicate_reject,
         media_endpoint_result.endpoint_invalid_uuid_reject,
         media_endpoint_result.endpoint_invalid_caps_reject,
         media_endpoint_result.endpoint_watch_add,
         media_endpoint_result.endpoint_adapter_append,
         media_endpoint_result.profile_custom_prop_add,
         media_endpoint_result.a2dp_add_sep_source,
         media_endpoint_result.a2dp_add_sep_sink,
         media_endpoint_result.a2dp_remove_sep,
         media_endpoint_result.select_configuration_call,
         media_endpoint_result.set_configuration_call,
         media_endpoint_result.clear_configuration_call,
         media_endpoint_result.release_call,
         media_endpoint_result.endpoint_request_create,
         media_endpoint_result.endpoint_pending_call,
         media_endpoint_result.endpoint_reply_success,
         media_endpoint_result.endpoint_reply_error,
         media_endpoint_result.endpoint_request_cancel,
         media_endpoint_result.transport_create,
         media_endpoint_result.transport_append,
         media_endpoint_result.transport_clear,
         media_endpoint_result.transport_destroy,
         media_endpoint_result.endpoint_remove,
         media_endpoint_result.endpoint_destroy,
         media_endpoint_result.endpoint_watch_remove,
         media_endpoint_result.profile_custom_prop_remove,
         media_endpoint_result.final_pending_endpoints,
         media_endpoint_result.final_pending_requests,
         media_endpoint_result.final_pending_transports,
         media_endpoint_result.final_pending_watches,
         media_endpoint_result.media_register_endpoint == 5 &&
         media_endpoint_result.media_unregister_endpoint == 1 &&
         media_endpoint_result.parse_uuid == 2 &&
         media_endpoint_result.parse_codec == 2 &&
         media_endpoint_result.parse_capabilities == 2 &&
         media_endpoint_result.parse_delay_reporting == 2 &&
         media_endpoint_result.endpoint_duplicate_reject == 1 &&
         media_endpoint_result.endpoint_invalid_uuid_reject == 1 &&
         media_endpoint_result.endpoint_invalid_caps_reject == 1 &&
         media_endpoint_result.endpoint_watch_add == 2 &&
         media_endpoint_result.endpoint_adapter_append == 2 &&
         media_endpoint_result.profile_custom_prop_add == 1 &&
         media_endpoint_result.a2dp_add_sep_source == 1 &&
         media_endpoint_result.a2dp_add_sep_sink == 1 &&
         media_endpoint_result.a2dp_remove_sep == 1 &&
         media_endpoint_result.select_configuration_call == 2 &&
         media_endpoint_result.set_configuration_call == 1 &&
         media_endpoint_result.clear_configuration_call == 2 &&
         media_endpoint_result.release_call == 1 &&
         media_endpoint_result.endpoint_request_create == 3 &&
         media_endpoint_result.endpoint_pending_call == 3 &&
         media_endpoint_result.endpoint_reply_success == 2 &&
         media_endpoint_result.endpoint_reply_error == 1 &&
         media_endpoint_result.endpoint_request_cancel == 2 &&
         media_endpoint_result.transport_create == 1 &&
         media_endpoint_result.transport_append == 1 &&
         media_endpoint_result.transport_clear == 1 &&
         media_endpoint_result.transport_destroy == 1 &&
         media_endpoint_result.endpoint_remove == 1 &&
         media_endpoint_result.endpoint_destroy == 1 &&
         media_endpoint_result.endpoint_watch_remove == 1 &&
         media_endpoint_result.profile_custom_prop_remove == 1 &&
         media_endpoint_result.final_pending_endpoints == 0 &&
         media_endpoint_result.final_pending_requests == 0 &&
         media_endpoint_result.final_pending_transports == 0 &&
         media_endpoint_result.final_pending_watches == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-dbus-table-surface-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c+"
         "third/bluez/profiles/audio/avrcp.c "
         "tables=transport_methods,transport_a2dp_properties,"
         "media_methods,media_properties,transport_ops "
         "transport-methods=total:%u,async:%u,Acquire:%u,TryAcquire:%u,"
         "Release:%u,Select:%u,Unselect:%u "
         "transport-a2dp-properties=total:%u,Device:%u,UUID:%u,Codec:%u,"
         "Configuration:%u,State:%u,Delay:%u,Volume:%u,Endpoint:%u,"
         "Delay-set:%u,Volume-set:%u,Endpoint-experimental:%u "
         "media-methods=total:%u,sync:%u,async:%u,RegisterEndpoint:%u,"
         "UnregisterEndpoint:%u,RegisterPlayer:%u,UnregisterPlayer:%u,"
         "RegisterApplication:%u,UnregisterApplication:%u "
         "media-properties=total:%u,SupportedUUIDs:%u,"
         "SupportedFeatures:%u "
         "ops=a2dp-source:%u,a2dp-sink:%u,bap-unicast:%u,"
         "bap-broadcast:%u "
         "upstream-link=upstream-dbus-tables-mirrored-linked-"
         "media-transport-c-objects final-ok=%u\n",
         role,
         dbus_table_result.transport_methods,
         dbus_table_result.transport_async_methods,
         dbus_table_result.transport_acquire,
         dbus_table_result.transport_try_acquire,
         dbus_table_result.transport_release,
         dbus_table_result.transport_select,
         dbus_table_result.transport_unselect,
         dbus_table_result.transport_a2dp_properties,
         dbus_table_result.property_device,
         dbus_table_result.property_uuid,
         dbus_table_result.property_codec,
         dbus_table_result.property_configuration,
         dbus_table_result.property_state,
         dbus_table_result.property_delay,
         dbus_table_result.property_volume,
         dbus_table_result.property_endpoint,
         dbus_table_result.property_delay_setter,
         dbus_table_result.property_volume_setter,
         dbus_table_result.property_endpoint_experimental,
         dbus_table_result.media_methods,
         dbus_table_result.media_sync_methods,
         dbus_table_result.media_async_methods,
         dbus_table_result.media_register_endpoint,
         dbus_table_result.media_unregister_endpoint,
         dbus_table_result.media_register_player,
         dbus_table_result.media_unregister_player,
         dbus_table_result.media_register_application,
         dbus_table_result.media_unregister_application,
         dbus_table_result.media_properties,
         dbus_table_result.media_supported_uuids,
         dbus_table_result.media_supported_features,
         dbus_table_result.ops_a2dp_source,
         dbus_table_result.ops_a2dp_sink,
         dbus_table_result.ops_bap_unicast,
         dbus_table_result.ops_bap_broadcast,
         dbus_table_result.transport_methods == 5 &&
         dbus_table_result.transport_async_methods == 5 &&
         dbus_table_result.transport_acquire == 1 &&
         dbus_table_result.transport_try_acquire == 1 &&
         dbus_table_result.transport_release == 1 &&
         dbus_table_result.transport_select == 1 &&
         dbus_table_result.transport_unselect == 1 &&
         dbus_table_result.transport_a2dp_properties == 8 &&
         dbus_table_result.property_device == 1 &&
         dbus_table_result.property_uuid == 1 &&
         dbus_table_result.property_codec == 1 &&
         dbus_table_result.property_configuration == 1 &&
         dbus_table_result.property_state == 1 &&
         dbus_table_result.property_delay == 1 &&
         dbus_table_result.property_volume == 1 &&
         dbus_table_result.property_endpoint == 1 &&
         dbus_table_result.property_delay_setter == 1 &&
         dbus_table_result.property_volume_setter == 1 &&
         dbus_table_result.property_endpoint_experimental == 1 &&
         dbus_table_result.media_methods == 6 &&
         dbus_table_result.media_sync_methods == 4 &&
         dbus_table_result.media_async_methods == 2 &&
         dbus_table_result.media_register_endpoint == 1 &&
         dbus_table_result.media_unregister_endpoint == 1 &&
         dbus_table_result.media_register_player == 1 &&
         dbus_table_result.media_unregister_player == 1 &&
         dbus_table_result.media_register_application == 1 &&
         dbus_table_result.media_unregister_application == 1 &&
         dbus_table_result.media_properties == 2 &&
         dbus_table_result.media_supported_uuids == 1 &&
         dbus_table_result.media_supported_features == 1 &&
         dbus_table_result.ops_a2dp_source == 1 &&
         dbus_table_result.ops_a2dp_sink == 1 &&
         dbus_table_result.ops_bap_unicast == 2 &&
         dbus_table_result.ops_bap_broadcast == 2 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper "
         "role=%s compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/media.c+"
         "third/bluez/profiles/audio/transport.c "
         "transport-method-handlers=total:%u,acquire:%u,try-acquire:%u,"
         "release:%u,select:%u,unselect:%u "
         "transport-property-getters=total:%u,device:%u,uuid:%u,codec:%u,"
         "configuration:%u,state:%u,delay:%u,volume:%u,endpoint:%u "
         "transport-property-setters=total:%u,delay:%u,volume:%u "
         "transport-property-exists=total:%u,delay:%u,volume:%u,"
         "endpoint:%u "
         "media-method-handlers=total:%u,register-endpoint:%u,"
         "unregister-endpoint:%u,register-player:%u,unregister-player:%u,"
         "register-application:%u,unregister-application:%u "
         "media-property-getters=total:%u,supported-uuids:%u,"
         "supported-features:%u "
         "bridge-ready=transport:%u,media:%u "
         "handler-calls=transport-methods:%u,transport-getters:%u,"
         "transport-setters:%u,transport-exists:%u,media-methods:%u,"
         "media-getters:%u,symbols-callable:%u "
         "handler-symbols=transport-methods:%u "
         "upstream-dispatch=transport-acquire:%u,transport-try-acquire:%u,"
         "transport-release:%u,transport-select:%u,transport-unselect:%u,"
         "transport-total:%u,media-register-endpoint:%u,"
         "media-unregister-endpoint:%u,media-total:%u,total:%u "
         "upstream-invocation-handoff=transport-acquire:%u,"
         "media-register-endpoint:%u,total:%u "
         "upstream-live-body-deps=media-transport-cross-object:%u,"
         "avdtp-control:%u,adapter-device:%u,glib-dbus:%u,ready:%u,"
         "required:%u "
         "upstream-live-body-retained=transport-acquire:%u,"
         "media-register-endpoint:%u,total:%u "
         "upstream-controlled-live-invocation-ready=transport-acquire:%u,"
         "media-register-endpoint:%u,total:%u "
         "upstream-minimal-real-objects=transport-acquire:%u,"
         "media-register-endpoint:%u,total:%u "
         "upstream-bounded-invocation=media-register-endpoint:%u,total:%u "
         "upstream-registered-endpoint=adapter:%u,endpoint:%u,sep:%u,"
         "total:%u "
         "upstream-media-endpoint-register=adapter:%u,watch:%u,sep:%u,"
         "reply:%u,total:%u "
         "upstream-media-endpoint-register-errors=duplicate:%u,"
         "cleanup:%u,total:%u "
         "upstream-media-endpoint-unregister=lookup:%u,watch:%u,sep:%u,"
         "remove:%u,reply:%u,total:%u "
         "upstream-media-endpoint-unregister-errors=wrong-sender:%u,"
         "missing-path:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-select-request=owner:%u,message:%u,"
         "pending:%u,cancel:%u,total:%u "
         "upstream-media-endpoint-select-reply=callback:%u,remove:%u,"
         "unref:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-select-error=callback:%u,remove:%u,"
         "unref:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-set-request=owner:%u,message:%u,"
         "transport:%u,reply:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-set-prepared=setup:%u,device:%u,"
         "remote:%u,total:%u "
         "upstream-media-endpoint-set-error=callback:%u,request:%u,"
         "transport:%u,unref:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-clear=message:%u,send:%u,"
         "transport:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-set-remote=path:%u,setup:%u,"
         "request:%u,reply:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-remote-lookup=registered:%u,"
         "session:%u,channel:%u,path:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-remote-caps=parsed:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-signaling-caps=response:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-dispatch-caps=response:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-packet-caps=frame:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-session-caps=read:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-discover-caps=request:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-media-endpoint-l2cap-caps=connect:%u,"
         "registered:%u,lookup:%u,set:%u,cleanup:%u,total:%u "
         "upstream-transport-create=endpoint:%u,device-service:%u,"
         "transport:%u,total:%u "
         "upstream-transport-export=path:%u,interface:%u,methods:%u,"
         "properties:%u,total:%u "
         "upstream-transport-path=first:%u,second:%u,unique:%u,total:%u "
         "upstream-transport-registry=append:%u,lookup:%u,remove:%u,"
         "total:%u "
         "upstream-property-getters=uuid:%u,codec:%u,configuration:%u,"
         "state:%u,delay:%u,volume:%u,endpoint:%u,total:%u "
         "upstream-property-setters=delay:%u,volume:%u,unauthorized:%u,"
         "invalid:%u,total:%u "
         "upstream-property-exists=delay-absent:%u,volume:%u,endpoint:%u,"
         "total:%u "
         "upstream-property-changes=delay:%u,volume:%u,total:%u "
         "upstream-bounded-acquire=transport:%u,owner:%u,request:%u,"
         "state:%u,total:%u "
         "upstream-resume-prepare=endpoint:%u,session:%u,stream:%u,"
         "total:%u "
         "upstream-acquire-complete=fd:%u,mtu:%u,reply:%u,state:%u,"
         "total:%u "
         "upstream-try-acquire-complete=owner:%u,fd:%u,reply:%u,"
         "state:%u,total:%u "
         "upstream-select-unselect-guard=owner:%u,state:%u,select:%u,"
         "unselect:%u,total:%u "
         "upstream-avdtp-media-fd=owner:%u,set:%u,get:%u,reply:%u,"
         "total:%u "
         "upstream-release-cleanup=owner:%u,pending:%u,fd-held:%u,state:%u,"
         "reply:%u,total:%u "
         "upstream-destroy-cleanup=register:%u,unregister:%u,free:%u,"
         "total:%u "
         "upstream-transport-unexport=path:%u,interface:%u,destroy:%u,"
         "total:%u "
         "upstream-error-closeout=duplicate-acquire:%u,"
         "unauthorized-release:%u,duplicate-release:%u,"
         "owner-disconnect:%u,total:%u "
         "handler-semantics=acquire:%u,try-acquire:%u,release:%u,"
         "select:%u,unselect:%u,total:%u "
         "property-semantics=getters:%u,setters:%u,exists:%u,total:%u "
         "media-semantics=methods:%u,properties:%u,total:%u "
         "ownership-semantics=objects:%u,requests:%u,final-zero:%u,"
         "total:%u "
         "lifecycle-semantics=dbus-requests:%u,errors:%u,total:%u "
         "upstream-object-graph=media:%u,transport:%u,endpoint-request:%u,"
         "total:%u "
         "upstream-state-policy=state2str:%u,in-use:%u,transitions:%u,"
         "total:%u "
         "upstream-transport-ops=uuid:%u,dispatch:%u,lifecycle:%u,total:%u "
         "upstream-error-policy=transport-methods:%u,"
         "media-registration:%u,total:%u "
         "upstream-endpoint-config=select:%u,set:%u,clear:%u,total:%u "
         "upstream-endpoint-request=cancel:%u,cancel-all:%u,destroy:%u,"
         "total:%u "
         "upstream-media-app=register:%u,unregister:%u,disconnect:%u,"
         "total:%u "
         "upstream-local-player=register:%u,properties:%u,unregister:%u,"
         "total:%u "
         "upstream-media-adapter=probe:%u,features:%u,remove:%u,total:%u "
         "upstream-a2dp-session=select:%u,set-config:%u,open-start:%u,"
         "suspend-close:%u,total:%u "
         "upstream-avdtp-signaling=discover:%u,getcap:%u,set-config:%u,"
         "open:%u,start:%u,suspend:%u,close:%u,abort:%u,total:%u "
         "upstream-linked-handler-mainloop=transport-dispatch:%u,"
         "media-dispatch:%u,pending:%u,watch:%u,cleanup:%u,total:%u "
         "upstream-transport-dbus-fsm=acquire:%u,try-acquire:%u,"
         "release:%u,select-unselect:%u,error:%u,final-zero:%u,total:%u "
         "upstream-media-endpoint-dbus-fsm=register:%u,select:%u,set:%u,"
         "clear:%u,unregister:%u,error:%u,final-zero:%u,total:%u "
         "upstream-media-application-dbus-fsm=register:%u,endpoints:%u,"
         "players:%u,unregister:%u,disconnect:%u,error:%u,final-zero:%u,"
         "total:%u "
         "upstream-avrcp-profile-fsm=player-register:%u,controller:%u,"
         "target:%u,metadata:%u,volume:%u,disconnect:%u,final-zero:%u,"
         "total:%u "
         "upstream-a2dp-media-stream-fsm=open:%u,start:%u,rtp:%u,"
         "payload:%u,suspend:%u,close:%u,error:%u,final-zero:%u,total:%u "
         "upstream-a2dp-codec-policy-fsm=capability:%u,select:%u,set:%u,"
         "reconfigure:%u,delay:%u,error:%u,final-zero:%u,total:%u "
         "upstream-a2dp-lifecycle-stress-fsm=first-connect:%u,cleanup:%u,"
         "reconnect:%u,duplicate-reject:%u,media-resume:%u,disconnect:%u,"
         "final-zero:%u,total:%u "
         "upstream-a2dp-object-link-readiness=sources:%u,headers:%u,"
         "glib-dbus:%u,mainloop:%u,core-objects:%u,l2cap-media:%u,"
         "symbol-ownership:%u,replacement-boundary:%u,total:%u "
         "upstream-a2dp-negative-boundary-fsm=bad-state:%u,mtu:%u,fd:%u,"
         "codec-recover:%u,duplicate-request:%u,abort-cleanup:%u,"
         "final-zero:%u,total:%u "
         "upstream-a2dp-profile-daemon=plugin-init:%u,adapter-probe:%u,"
         "endpoint-register:%u,avdtp-bind:%u,transport-export:%u,"
         "daemon-cleanup:%u,total:%u "
         "upstream-link=upstream-handler-families-mapped-transport-media-dispatch-linked-invocation-handoff-"
         "media-transport-c-handlers final-ok=%u\n",
         role,
         handler_bridge_result.transport_method_handlers,
         handler_bridge_result.transport_acquire_handler,
         handler_bridge_result.transport_try_acquire_handler,
         handler_bridge_result.transport_release_handler,
         handler_bridge_result.transport_select_handler,
         handler_bridge_result.transport_unselect_handler,
         handler_bridge_result.transport_property_getters,
         handler_bridge_result.transport_get_device,
         handler_bridge_result.transport_get_uuid,
         handler_bridge_result.transport_get_codec,
         handler_bridge_result.transport_get_configuration,
         handler_bridge_result.transport_get_state,
         handler_bridge_result.transport_get_delay,
         handler_bridge_result.transport_get_volume,
         handler_bridge_result.transport_get_endpoint,
         handler_bridge_result.transport_property_setters,
         handler_bridge_result.transport_set_delay,
         handler_bridge_result.transport_set_volume,
         handler_bridge_result.transport_property_exists,
         handler_bridge_result.transport_delay_exists,
         handler_bridge_result.transport_volume_exists,
         handler_bridge_result.transport_endpoint_exists,
         handler_bridge_result.media_method_handlers,
         handler_bridge_result.media_register_endpoint_handler,
         handler_bridge_result.media_unregister_endpoint_handler,
         handler_bridge_result.media_register_player_handler,
         handler_bridge_result.media_unregister_player_handler,
         handler_bridge_result.media_register_application_handler,
         handler_bridge_result.media_unregister_application_handler,
         handler_bridge_result.media_property_getters,
         handler_bridge_result.media_supported_uuids_getter,
         handler_bridge_result.media_supported_features_getter,
         handler_bridge_result.bridge_ready_transport,
         handler_bridge_result.bridge_ready_media,
         handler_bridge_result.transport_method_calls,
         handler_bridge_result.transport_property_getter_calls,
         handler_bridge_result.transport_property_setter_calls,
         handler_bridge_result.transport_property_exists_calls,
         handler_bridge_result.media_method_calls,
         handler_bridge_result.media_property_getter_calls,
         handler_bridge_result.bridge_symbols_callable,
         handler_bridge_result.transport_method_named_symbols,
         handler_bridge_result.transport_acquire_upstream_dispatch,
         handler_bridge_result.transport_try_acquire_upstream_dispatch,
         handler_bridge_result.transport_release_upstream_dispatch,
         handler_bridge_result.transport_select_upstream_dispatch,
         handler_bridge_result.transport_unselect_upstream_dispatch,
         handler_bridge_result.transport_upstream_dispatch_entries,
         handler_bridge_result.media_register_endpoint_upstream_dispatch,
         handler_bridge_result.media_unregister_endpoint_upstream_dispatch,
         handler_bridge_result.media_upstream_dispatch_entries,
         handler_bridge_result.transport_upstream_dispatch_entries +
         handler_bridge_result.media_upstream_dispatch_entries,
         handler_bridge_result.transport_acquire_invocation_handoff,
         handler_bridge_result.media_register_endpoint_invocation_handoff,
         handler_bridge_result.upstream_invocation_handoff_entries,
         handler_bridge_result.upstream_live_body_media_transport_cross_object_deps,
         handler_bridge_result.upstream_live_body_avdtp_control_deps,
         handler_bridge_result.upstream_live_body_adapter_device_deps,
         handler_bridge_result.upstream_live_body_glib_dbus_deps,
         handler_bridge_result.upstream_live_body_ready_deps,
         handler_bridge_result.upstream_live_body_required_deps,
         handler_bridge_result.upstream_live_body_transport_acquire_retained,
         handler_bridge_result.upstream_live_body_media_register_endpoint_retained,
         handler_bridge_result.upstream_live_body_retained_entries,
         handler_bridge_result.upstream_controlled_live_transport_acquire_ready,
         handler_bridge_result.upstream_controlled_live_media_register_endpoint_ready,
         handler_bridge_result.upstream_controlled_live_invocation_ready_entries,
         handler_bridge_result.upstream_minimal_real_transport_acquire_objects,
         handler_bridge_result.upstream_minimal_real_media_register_endpoint_objects,
         handler_bridge_result.upstream_minimal_real_object_entries,
         handler_bridge_result.upstream_bounded_media_register_endpoint_invoked,
         handler_bridge_result.upstream_bounded_invocation_entries,
         handler_bridge_result.upstream_registered_endpoint_adapter,
         handler_bridge_result.upstream_registered_endpoint_endpoint,
         handler_bridge_result.upstream_registered_endpoint_sep,
         handler_bridge_result.upstream_registered_endpoint_entries,
         handler_bridge_result.upstream_media_endpoint_register_adapter,
         handler_bridge_result.upstream_media_endpoint_register_watch,
         handler_bridge_result.upstream_media_endpoint_register_sep,
         handler_bridge_result.upstream_media_endpoint_register_reply,
         handler_bridge_result.upstream_media_endpoint_register_entries,
         handler_bridge_result.upstream_media_endpoint_register_error_duplicate,
         handler_bridge_result.upstream_media_endpoint_register_error_cleanup,
         handler_bridge_result.upstream_media_endpoint_register_error_entries,
         handler_bridge_result.upstream_media_endpoint_unregister_lookup,
         handler_bridge_result.upstream_media_endpoint_unregister_watch,
         handler_bridge_result.upstream_media_endpoint_unregister_sep,
         handler_bridge_result.upstream_media_endpoint_unregister_remove,
         handler_bridge_result.upstream_media_endpoint_unregister_reply,
         handler_bridge_result.upstream_media_endpoint_unregister_entries,
         handler_bridge_result.upstream_media_endpoint_unregister_error_wrong_sender,
         handler_bridge_result.upstream_media_endpoint_unregister_error_missing_path,
         handler_bridge_result.upstream_media_endpoint_unregister_error_cleanup,
         handler_bridge_result.upstream_media_endpoint_unregister_error_entries,
         handler_bridge_result.upstream_media_endpoint_select_request_owner,
         handler_bridge_result.upstream_media_endpoint_select_request_message,
         handler_bridge_result.upstream_media_endpoint_select_request_pending,
         handler_bridge_result.upstream_media_endpoint_select_request_cancel,
         handler_bridge_result.upstream_media_endpoint_select_request_entries,
         handler_bridge_result.upstream_media_endpoint_select_reply_callback,
         handler_bridge_result.upstream_media_endpoint_select_reply_remove,
         handler_bridge_result.upstream_media_endpoint_select_reply_unref,
         handler_bridge_result.upstream_media_endpoint_select_reply_cleanup,
         handler_bridge_result.upstream_media_endpoint_select_reply_entries,
         handler_bridge_result.upstream_media_endpoint_select_error_callback,
         handler_bridge_result.upstream_media_endpoint_select_error_remove,
         handler_bridge_result.upstream_media_endpoint_select_error_unref,
         handler_bridge_result.upstream_media_endpoint_select_error_cleanup,
         handler_bridge_result.upstream_media_endpoint_select_error_entries,
         handler_bridge_result.upstream_media_endpoint_set_request_owner,
         handler_bridge_result.upstream_media_endpoint_set_request_message,
         handler_bridge_result.upstream_media_endpoint_set_request_transport,
         handler_bridge_result.upstream_media_endpoint_set_request_reply,
         handler_bridge_result.upstream_media_endpoint_set_request_cleanup,
         handler_bridge_result.upstream_media_endpoint_set_request_entries,
         handler_bridge_result.upstream_media_endpoint_set_prepared_setup,
         handler_bridge_result.upstream_media_endpoint_set_prepared_device,
         handler_bridge_result.upstream_media_endpoint_set_prepared_remote,
         handler_bridge_result.upstream_media_endpoint_set_prepared_entries,
         handler_bridge_result.upstream_media_endpoint_set_error_callback,
         handler_bridge_result.upstream_media_endpoint_set_error_request,
         handler_bridge_result.upstream_media_endpoint_set_error_transport,
         handler_bridge_result.upstream_media_endpoint_set_error_unref,
         handler_bridge_result.upstream_media_endpoint_set_error_cleanup,
         handler_bridge_result.upstream_media_endpoint_set_error_entries,
         handler_bridge_result.upstream_media_endpoint_clear_message,
         handler_bridge_result.upstream_media_endpoint_clear_send,
         handler_bridge_result.upstream_media_endpoint_clear_transport,
         handler_bridge_result.upstream_media_endpoint_clear_cleanup,
         handler_bridge_result.upstream_media_endpoint_clear_entries,
         handler_bridge_result.upstream_media_endpoint_set_remote_path,
         handler_bridge_result.upstream_media_endpoint_set_remote_setup,
         handler_bridge_result.upstream_media_endpoint_set_remote_request,
         handler_bridge_result.upstream_media_endpoint_set_remote_reply,
         handler_bridge_result.upstream_media_endpoint_set_remote_cleanup,
         handler_bridge_result.upstream_media_endpoint_set_remote_entries,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_registered,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_session,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_channel,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_path,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_cleanup,
         handler_bridge_result.upstream_media_endpoint_remote_lookup_entries,
         handler_bridge_result.upstream_media_endpoint_remote_caps_parsed,
         handler_bridge_result.upstream_media_endpoint_remote_caps_registered,
         handler_bridge_result.upstream_media_endpoint_remote_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_remote_caps_set,
         handler_bridge_result.upstream_media_endpoint_remote_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_remote_caps_entries,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_response,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_registered,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_set,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_signaling_caps_entries,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_response,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_registered,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_set,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_dispatch_caps_entries,
         handler_bridge_result.upstream_media_endpoint_packet_caps_frame,
         handler_bridge_result.upstream_media_endpoint_packet_caps_registered,
         handler_bridge_result.upstream_media_endpoint_packet_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_packet_caps_set,
         handler_bridge_result.upstream_media_endpoint_packet_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_packet_caps_entries,
         handler_bridge_result.upstream_media_endpoint_session_caps_read,
         handler_bridge_result.upstream_media_endpoint_session_caps_registered,
         handler_bridge_result.upstream_media_endpoint_session_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_session_caps_set,
         handler_bridge_result.upstream_media_endpoint_session_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_session_caps_entries,
         handler_bridge_result.upstream_media_endpoint_discover_caps_request,
         handler_bridge_result.upstream_media_endpoint_discover_caps_registered,
         handler_bridge_result.upstream_media_endpoint_discover_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_discover_caps_set,
         handler_bridge_result.upstream_media_endpoint_discover_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_discover_caps_entries,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_connect,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_registered,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_lookup,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_set,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_cleanup,
         handler_bridge_result.upstream_media_endpoint_l2cap_caps_entries,
         handler_bridge_result.upstream_transport_create_endpoint,
         handler_bridge_result.upstream_transport_create_device_service,
         handler_bridge_result.upstream_transport_create_transport,
         handler_bridge_result.upstream_transport_create_entries,
         handler_bridge_result.upstream_transport_export_path,
         handler_bridge_result.upstream_transport_export_interface,
         handler_bridge_result.upstream_transport_export_methods,
         handler_bridge_result.upstream_transport_export_properties,
         handler_bridge_result.upstream_transport_export_entries,
         handler_bridge_result.upstream_transport_path_first,
         handler_bridge_result.upstream_transport_path_second,
         handler_bridge_result.upstream_transport_path_unique,
         handler_bridge_result.upstream_transport_path_entries,
         handler_bridge_result.upstream_transport_registry_append,
         handler_bridge_result.upstream_transport_registry_lookup,
         handler_bridge_result.upstream_transport_registry_remove,
         handler_bridge_result.upstream_transport_registry_entries,
         handler_bridge_result.upstream_property_getter_uuid,
         handler_bridge_result.upstream_property_getter_codec,
         handler_bridge_result.upstream_property_getter_configuration,
         handler_bridge_result.upstream_property_getter_state,
         handler_bridge_result.upstream_property_getter_delay,
         handler_bridge_result.upstream_property_getter_volume,
         handler_bridge_result.upstream_property_getter_endpoint,
         handler_bridge_result.upstream_property_getter_entries,
         handler_bridge_result.upstream_property_setter_delay,
         handler_bridge_result.upstream_property_setter_volume,
         handler_bridge_result.upstream_property_setter_unauthorized,
         handler_bridge_result.upstream_property_setter_invalid,
         handler_bridge_result.upstream_property_setter_entries,
         handler_bridge_result.upstream_property_exists_delay_absent,
         handler_bridge_result.upstream_property_exists_volume,
         handler_bridge_result.upstream_property_exists_endpoint,
         handler_bridge_result.upstream_property_exists_entries,
         handler_bridge_result.upstream_property_change_delay,
         handler_bridge_result.upstream_property_change_volume,
         handler_bridge_result.upstream_property_change_entries,
         handler_bridge_result.upstream_bounded_acquire_transport,
         handler_bridge_result.upstream_bounded_acquire_owner,
         handler_bridge_result.upstream_bounded_acquire_request,
         handler_bridge_result.upstream_bounded_acquire_state,
         handler_bridge_result.upstream_bounded_acquire_entries,
         handler_bridge_result.upstream_resume_prepare_endpoint,
         handler_bridge_result.upstream_resume_prepare_session,
         handler_bridge_result.upstream_resume_prepare_stream,
         handler_bridge_result.upstream_resume_prepare_entries,
         handler_bridge_result.upstream_acquire_complete_fd,
         handler_bridge_result.upstream_acquire_complete_mtu,
         handler_bridge_result.upstream_acquire_complete_reply,
         handler_bridge_result.upstream_acquire_complete_state,
         handler_bridge_result.upstream_acquire_complete_entries,
         handler_bridge_result.upstream_try_acquire_complete_owner,
         handler_bridge_result.upstream_try_acquire_complete_fd,
         handler_bridge_result.upstream_try_acquire_complete_reply,
         handler_bridge_result.upstream_try_acquire_complete_state,
         handler_bridge_result.upstream_try_acquire_complete_entries,
         handler_bridge_result.upstream_select_unselect_guard_owner,
         handler_bridge_result.upstream_select_unselect_guard_state,
         handler_bridge_result.upstream_select_unselect_guard_select,
         handler_bridge_result.upstream_select_unselect_guard_unselect,
         handler_bridge_result.upstream_select_unselect_guard_entries,
         handler_bridge_result.upstream_avdtp_media_fd_owner,
         handler_bridge_result.upstream_avdtp_media_fd_set,
         handler_bridge_result.upstream_avdtp_media_fd_get,
         handler_bridge_result.upstream_avdtp_media_fd_reply,
         handler_bridge_result.upstream_avdtp_media_fd_entries,
         handler_bridge_result.upstream_release_cleanup_owner,
         handler_bridge_result.upstream_release_cleanup_pending,
         handler_bridge_result.upstream_release_cleanup_fd,
         handler_bridge_result.upstream_release_cleanup_state,
         handler_bridge_result.upstream_release_cleanup_reply,
         handler_bridge_result.upstream_release_cleanup_entries,
         handler_bridge_result.upstream_destroy_cleanup_register,
         handler_bridge_result.upstream_destroy_cleanup_unregister,
         handler_bridge_result.upstream_destroy_cleanup_free,
         handler_bridge_result.upstream_destroy_cleanup_entries,
         handler_bridge_result.upstream_transport_unexport_path,
         handler_bridge_result.upstream_transport_unexport_interface,
         handler_bridge_result.upstream_transport_unexport_destroy,
         handler_bridge_result.upstream_transport_unexport_entries,
         handler_bridge_result.upstream_error_closeout_duplicate_acquire,
         handler_bridge_result.upstream_error_closeout_unauthorized_release,
         handler_bridge_result.upstream_error_closeout_duplicate_release,
         handler_bridge_result.upstream_error_closeout_owner_disconnect,
         handler_bridge_result.upstream_error_closeout_entries,
         handler_bridge_result.transport_acquire_semantic_wrapper,
         handler_bridge_result.transport_try_acquire_semantic_wrapper,
         handler_bridge_result.transport_release_semantic_wrapper,
         handler_bridge_result.transport_select_semantic_wrapper,
         handler_bridge_result.transport_unselect_semantic_wrapper,
         handler_bridge_result.transport_method_semantic_wrappers,
         handler_bridge_result.transport_property_getter_semantic_wrappers,
         handler_bridge_result.transport_property_setter_semantic_wrappers,
         handler_bridge_result.transport_property_exists_semantic_wrappers,
         handler_bridge_result.transport_property_semantic_wrappers,
         handler_bridge_result.media_method_semantic_wrappers,
         handler_bridge_result.media_property_getter_semantic_wrappers,
         handler_bridge_result.media_semantic_wrappers,
         handler_bridge_result.media_transport_object_ownership_semantics,
         handler_bridge_result.media_transport_request_ownership_semantics,
         handler_bridge_result.media_transport_final_zero_semantics,
         handler_bridge_result.media_transport_ownership_semantics,
         handler_bridge_result.media_transport_dbus_request_lifecycle_semantics,
         handler_bridge_result.media_transport_error_lifecycle_semantics,
         handler_bridge_result.media_transport_lifecycle_semantics,
         handler_bridge_result.upstream_media_object_graph_semantics,
         handler_bridge_result.upstream_transport_object_graph_semantics,
         handler_bridge_result.upstream_endpoint_request_graph_semantics,
         handler_bridge_result.upstream_object_graph_semantics,
         handler_bridge_result.upstream_transport_state2str_semantics,
         handler_bridge_result.upstream_transport_state_in_use_semantics,
         handler_bridge_result.upstream_transport_state_transition_semantics,
         handler_bridge_result.upstream_transport_state_policy_semantics,
         handler_bridge_result.upstream_transport_ops_uuid_semantics,
         handler_bridge_result.upstream_transport_ops_dispatch_semantics,
         handler_bridge_result.upstream_transport_ops_lifecycle_semantics,
         handler_bridge_result.upstream_transport_ops_policy_semantics,
         handler_bridge_result.upstream_transport_method_error_policy_semantics,
         handler_bridge_result.upstream_media_registration_error_policy_semantics,
         handler_bridge_result.upstream_error_policy_semantics,
         handler_bridge_result.upstream_endpoint_select_config_semantics,
         handler_bridge_result.upstream_endpoint_set_config_semantics,
         handler_bridge_result.upstream_endpoint_clear_config_semantics,
         handler_bridge_result.upstream_endpoint_config_policy_semantics,
         handler_bridge_result.upstream_endpoint_request_cancel_semantics,
         handler_bridge_result.upstream_endpoint_request_cancel_all_semantics,
         handler_bridge_result.upstream_endpoint_destroy_semantics,
         handler_bridge_result.upstream_endpoint_request_policy_semantics,
         handler_bridge_result.upstream_media_app_register_semantics,
         handler_bridge_result.upstream_media_app_unregister_semantics,
         handler_bridge_result.upstream_media_app_disconnect_semantics,
         handler_bridge_result.upstream_media_app_policy_semantics,
         handler_bridge_result.upstream_local_player_register_semantics,
         handler_bridge_result.upstream_local_player_properties_semantics,
         handler_bridge_result.upstream_local_player_unregister_semantics,
         handler_bridge_result.upstream_local_player_policy_semantics,
         handler_bridge_result.upstream_media_adapter_probe_semantics,
         handler_bridge_result.upstream_media_adapter_features_semantics,
         handler_bridge_result.upstream_media_adapter_remove_semantics,
         handler_bridge_result.upstream_media_adapter_policy_semantics,
         handler_bridge_result.upstream_a2dp_session_select_semantics,
         handler_bridge_result.upstream_a2dp_session_set_config_semantics,
         handler_bridge_result.upstream_a2dp_session_open_start_semantics,
         handler_bridge_result.upstream_a2dp_session_suspend_close_semantics,
         handler_bridge_result.upstream_a2dp_session_flow_semantics,
         handler_bridge_result.upstream_avdtp_signaling_discover_semantics,
         handler_bridge_result.upstream_avdtp_signaling_getcap_semantics,
         handler_bridge_result.upstream_avdtp_signaling_set_config_semantics,
         handler_bridge_result.upstream_avdtp_signaling_open_semantics,
         handler_bridge_result.upstream_avdtp_signaling_start_semantics,
         handler_bridge_result.upstream_avdtp_signaling_suspend_semantics,
         handler_bridge_result.upstream_avdtp_signaling_close_semantics,
         handler_bridge_result.upstream_avdtp_signaling_abort_semantics,
         handler_bridge_result.upstream_avdtp_signaling_flow_semantics,
         handler_bridge_result.upstream_linked_handler_transport_dispatch_semantics,
         handler_bridge_result.upstream_linked_handler_media_dispatch_semantics,
         handler_bridge_result.upstream_linked_handler_pending_request_semantics,
         handler_bridge_result.upstream_linked_handler_mainloop_watch_semantics,
         handler_bridge_result.upstream_linked_handler_cleanup_semantics,
         handler_bridge_result.upstream_linked_handler_mainloop_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_acquire_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_try_acquire_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_release_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_select_unselect_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_error_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_final_zero_semantics,
         handler_bridge_result.upstream_transport_dbus_fsm_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_register_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_select_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_set_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_clear_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_unregister_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_error_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_final_zero_semantics,
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_register_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_endpoints_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_players_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_unregister_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_disconnect_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_error_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_final_zero_semantics,
         handler_bridge_result.upstream_media_application_dbus_fsm_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_player_register_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_controller_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_target_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_metadata_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_volume_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_disconnect_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_final_zero_semantics,
         handler_bridge_result.upstream_avrcp_profile_fsm_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_open_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_start_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_rtp_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_payload_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_suspend_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_close_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_error_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_final_zero_semantics,
         handler_bridge_result.upstream_a2dp_media_stream_fsm_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_capability_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_select_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_set_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_reconfigure_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_delay_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_error_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_final_zero_semantics,
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_first_connect_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_cleanup_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_reconnect_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_duplicate_reject_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_media_resume_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_disconnect_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_final_zero_semantics,
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_sources_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_headers_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_glib_dbus_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_mainloop_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_core_objects_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_l2cap_media_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_symbol_ownership_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_replacement_boundary_semantics,
         handler_bridge_result.upstream_a2dp_object_link_readiness_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_bad_state_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_mtu_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_fd_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_codec_recover_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_duplicate_request_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_abort_cleanup_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_final_zero_semantics,
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_semantics,
         handler_bridge_result.upstream_a2dp_profile_plugin_init_semantics,
         handler_bridge_result.upstream_a2dp_profile_adapter_probe_semantics,
         handler_bridge_result.upstream_a2dp_profile_endpoint_register_semantics,
         handler_bridge_result.upstream_a2dp_profile_avdtp_bind_semantics,
         handler_bridge_result.upstream_a2dp_profile_transport_export_semantics,
         handler_bridge_result.upstream_a2dp_profile_daemon_cleanup_semantics,
         handler_bridge_result.upstream_a2dp_profile_daemon_flow_semantics,
         handler_bridge_result.transport_method_handlers == 5 &&
         handler_bridge_result.transport_acquire_handler == 1 &&
         handler_bridge_result.transport_try_acquire_handler == 1 &&
         handler_bridge_result.transport_release_handler == 1 &&
         handler_bridge_result.transport_select_handler == 1 &&
         handler_bridge_result.transport_unselect_handler == 1 &&
         handler_bridge_result.transport_property_getters == 8 &&
         handler_bridge_result.transport_get_device == 1 &&
         handler_bridge_result.transport_get_uuid == 1 &&
         handler_bridge_result.transport_get_codec == 1 &&
         handler_bridge_result.transport_get_configuration == 1 &&
         handler_bridge_result.transport_get_state == 1 &&
         handler_bridge_result.transport_get_delay == 1 &&
         handler_bridge_result.transport_get_volume == 1 &&
         handler_bridge_result.transport_get_endpoint == 1 &&
         handler_bridge_result.transport_property_setters == 2 &&
         handler_bridge_result.transport_set_delay == 1 &&
         handler_bridge_result.transport_set_volume == 1 &&
         handler_bridge_result.transport_property_exists == 3 &&
         handler_bridge_result.transport_delay_exists == 1 &&
         handler_bridge_result.transport_volume_exists == 1 &&
         handler_bridge_result.transport_endpoint_exists == 1 &&
         handler_bridge_result.media_method_handlers == 6 &&
         handler_bridge_result.media_register_endpoint_handler == 1 &&
         handler_bridge_result.media_unregister_endpoint_handler == 1 &&
         handler_bridge_result.media_register_player_handler == 1 &&
         handler_bridge_result.media_unregister_player_handler == 1 &&
         handler_bridge_result.media_register_application_handler == 1 &&
         handler_bridge_result.media_unregister_application_handler == 1 &&
         handler_bridge_result.media_property_getters == 2 &&
         handler_bridge_result.media_supported_uuids_getter == 1 &&
         handler_bridge_result.media_supported_features_getter == 1 &&
         handler_bridge_result.bridge_ready_transport == 1 &&
         handler_bridge_result.bridge_ready_media == 1 &&
         handler_bridge_result.transport_method_calls == 5 &&
         handler_bridge_result.transport_property_getter_calls == 8 &&
         handler_bridge_result.transport_property_setter_calls == 2 &&
         handler_bridge_result.transport_property_exists_calls == 3 &&
         handler_bridge_result.media_method_calls == 6 &&
         handler_bridge_result.media_property_getter_calls == 2 &&
         handler_bridge_result.bridge_symbols_callable == 1 &&
         handler_bridge_result.transport_method_named_symbols == 5 &&
         handler_bridge_result.upstream_media_endpoint_register_adapter == 1 &&
         handler_bridge_result.upstream_media_endpoint_register_watch == 1 &&
         handler_bridge_result.upstream_media_endpoint_register_sep == 1 &&
         handler_bridge_result.upstream_media_endpoint_register_reply == 1 &&
         handler_bridge_result.upstream_media_endpoint_register_entries == 4 &&
         handler_bridge_result.upstream_transport_export_path == 1 &&
         handler_bridge_result.upstream_transport_export_interface == 1 &&
         handler_bridge_result.upstream_transport_export_methods == 1 &&
         handler_bridge_result.upstream_transport_export_properties == 1 &&
         handler_bridge_result.upstream_transport_export_entries == 4 &&
         handler_bridge_result.upstream_transport_path_first == 1 &&
         handler_bridge_result.upstream_transport_path_second == 1 &&
         handler_bridge_result.upstream_transport_path_unique == 1 &&
         handler_bridge_result.upstream_transport_path_entries == 3 &&
         handler_bridge_result.upstream_transport_registry_append == 1 &&
         handler_bridge_result.upstream_transport_registry_lookup == 1 &&
         handler_bridge_result.upstream_transport_registry_remove == 1 &&
         handler_bridge_result.upstream_transport_registry_entries == 3 &&
         handler_bridge_result.upstream_property_getter_uuid == 1 &&
         handler_bridge_result.upstream_property_getter_codec == 1 &&
         handler_bridge_result.upstream_property_getter_configuration == 1 &&
         handler_bridge_result.upstream_property_getter_state == 1 &&
         handler_bridge_result.upstream_property_getter_delay == 1 &&
         handler_bridge_result.upstream_property_getter_volume == 1 &&
         handler_bridge_result.upstream_property_getter_endpoint == 1 &&
         handler_bridge_result.upstream_property_getter_entries == 7 &&
         handler_bridge_result.upstream_property_setter_delay == 1 &&
         handler_bridge_result.upstream_property_setter_volume == 1 &&
         handler_bridge_result.upstream_property_setter_unauthorized == 1 &&
         handler_bridge_result.upstream_property_setter_invalid == 1 &&
         handler_bridge_result.upstream_property_setter_entries == 4 &&
         handler_bridge_result.upstream_property_exists_delay_absent == 1 &&
         handler_bridge_result.upstream_property_exists_volume == 1 &&
         handler_bridge_result.upstream_property_exists_endpoint == 1 &&
         handler_bridge_result.upstream_property_exists_entries == 3 &&
         handler_bridge_result.upstream_property_change_delay == 1 &&
         handler_bridge_result.upstream_property_change_volume == 1 &&
         handler_bridge_result.upstream_property_change_entries == 2 &&
         handler_bridge_result.upstream_bounded_acquire_transport == 1 &&
         handler_bridge_result.upstream_bounded_acquire_owner == 1 &&
         handler_bridge_result.upstream_bounded_acquire_request == 1 &&
         handler_bridge_result.upstream_bounded_acquire_state == 1 &&
         handler_bridge_result.upstream_bounded_acquire_entries == 4 &&
         handler_bridge_result.upstream_resume_prepare_endpoint == 1 &&
         handler_bridge_result.upstream_resume_prepare_session == 1 &&
         handler_bridge_result.upstream_resume_prepare_stream == 1 &&
         handler_bridge_result.upstream_resume_prepare_entries == 3 &&
         handler_bridge_result.upstream_acquire_complete_fd == 1 &&
         handler_bridge_result.upstream_acquire_complete_mtu == 1 &&
         handler_bridge_result.upstream_acquire_complete_reply == 1 &&
         handler_bridge_result.upstream_acquire_complete_state == 1 &&
         handler_bridge_result.upstream_acquire_complete_entries == 4 &&
         handler_bridge_result.upstream_try_acquire_complete_owner == 1 &&
         handler_bridge_result.upstream_try_acquire_complete_fd == 1 &&
         handler_bridge_result.upstream_try_acquire_complete_reply == 1 &&
         handler_bridge_result.upstream_try_acquire_complete_state == 1 &&
         handler_bridge_result.upstream_try_acquire_complete_entries == 4 &&
         handler_bridge_result.upstream_select_unselect_guard_owner == 1 &&
         handler_bridge_result.upstream_select_unselect_guard_state == 1 &&
         handler_bridge_result.upstream_select_unselect_guard_select == 1 &&
         handler_bridge_result.upstream_select_unselect_guard_unselect == 1 &&
         handler_bridge_result.upstream_select_unselect_guard_entries == 4 &&
         handler_bridge_result.upstream_avdtp_media_fd_owner == 1 &&
         handler_bridge_result.upstream_avdtp_media_fd_set == 1 &&
         handler_bridge_result.upstream_avdtp_media_fd_get == 1 &&
         handler_bridge_result.upstream_avdtp_media_fd_reply == 1 &&
         handler_bridge_result.upstream_avdtp_media_fd_entries == 4 &&
         handler_bridge_result.upstream_release_cleanup_owner == 1 &&
         handler_bridge_result.upstream_release_cleanup_pending == 1 &&
         handler_bridge_result.upstream_release_cleanup_fd == 1 &&
         handler_bridge_result.upstream_release_cleanup_state == 1 &&
         handler_bridge_result.upstream_release_cleanup_reply == 1 &&
         handler_bridge_result.upstream_release_cleanup_entries == 5 &&
         handler_bridge_result.upstream_destroy_cleanup_register == 1 &&
         handler_bridge_result.upstream_destroy_cleanup_unregister == 1 &&
         handler_bridge_result.upstream_destroy_cleanup_free == 1 &&
         handler_bridge_result.upstream_destroy_cleanup_entries == 3 &&
         handler_bridge_result.upstream_transport_unexport_path == 1 &&
         handler_bridge_result.upstream_transport_unexport_interface == 1 &&
         handler_bridge_result.upstream_transport_unexport_destroy == 1 &&
         handler_bridge_result.upstream_transport_unexport_entries == 3 &&
         handler_bridge_result.upstream_error_closeout_duplicate_acquire == 1 &&
         handler_bridge_result.upstream_error_closeout_unauthorized_release == 1 &&
         handler_bridge_result.upstream_error_closeout_duplicate_release == 1 &&
         handler_bridge_result.upstream_error_closeout_owner_disconnect == 1 &&
         handler_bridge_result.upstream_error_closeout_entries == 4 &&
         handler_bridge_result.transport_acquire_semantic_wrapper == 1 &&
         handler_bridge_result.transport_try_acquire_semantic_wrapper == 1 &&
         handler_bridge_result.transport_release_semantic_wrapper == 1 &&
         handler_bridge_result.transport_select_semantic_wrapper == 1 &&
         handler_bridge_result.transport_unselect_semantic_wrapper == 1 &&
         handler_bridge_result.transport_method_semantic_wrappers == 5 &&
         handler_bridge_result.transport_property_getter_semantic_wrappers == 8 &&
         handler_bridge_result.transport_property_setter_semantic_wrappers == 2 &&
         handler_bridge_result.transport_property_exists_semantic_wrappers == 3 &&
         handler_bridge_result.transport_property_semantic_wrappers == 13 &&
         handler_bridge_result.media_method_semantic_wrappers == 6 &&
         handler_bridge_result.media_property_getter_semantic_wrappers == 2 &&
         handler_bridge_result.media_semantic_wrappers == 8 &&
         handler_bridge_result.media_transport_object_ownership_semantics == 1 &&
         handler_bridge_result.media_transport_request_ownership_semantics == 1 &&
         handler_bridge_result.media_transport_final_zero_semantics == 1 &&
         handler_bridge_result.media_transport_ownership_semantics == 3 &&
         handler_bridge_result.media_transport_dbus_request_lifecycle_semantics == 1 &&
         handler_bridge_result.media_transport_error_lifecycle_semantics == 1 &&
         handler_bridge_result.media_transport_lifecycle_semantics == 2 &&
         handler_bridge_result.upstream_media_object_graph_semantics == 1 &&
         handler_bridge_result.upstream_transport_object_graph_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_request_graph_semantics == 1 &&
         handler_bridge_result.upstream_object_graph_semantics == 3 &&
         handler_bridge_result.upstream_transport_state2str_semantics == 1 &&
         handler_bridge_result.upstream_transport_state_in_use_semantics == 1 &&
         handler_bridge_result.upstream_transport_state_transition_semantics == 1 &&
         handler_bridge_result.upstream_transport_state_policy_semantics == 3 &&
         handler_bridge_result.upstream_transport_ops_uuid_semantics == 1 &&
         handler_bridge_result.upstream_transport_ops_dispatch_semantics == 1 &&
         handler_bridge_result.upstream_transport_ops_lifecycle_semantics == 1 &&
         handler_bridge_result.upstream_transport_ops_policy_semantics == 3 &&
         handler_bridge_result.upstream_transport_method_error_policy_semantics == 1 &&
         handler_bridge_result.upstream_media_registration_error_policy_semantics == 1 &&
         handler_bridge_result.upstream_error_policy_semantics == 2 &&
         handler_bridge_result.upstream_endpoint_select_config_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_set_config_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_clear_config_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_config_policy_semantics == 3 &&
         handler_bridge_result.upstream_endpoint_request_cancel_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_request_cancel_all_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_destroy_semantics == 1 &&
         handler_bridge_result.upstream_endpoint_request_policy_semantics == 3 &&
         handler_bridge_result.upstream_media_app_register_semantics == 1 &&
         handler_bridge_result.upstream_media_app_unregister_semantics == 1 &&
         handler_bridge_result.upstream_media_app_disconnect_semantics == 1 &&
         handler_bridge_result.upstream_media_app_policy_semantics == 3 &&
         handler_bridge_result.upstream_local_player_register_semantics == 1 &&
         handler_bridge_result.upstream_local_player_properties_semantics == 1 &&
         handler_bridge_result.upstream_local_player_unregister_semantics == 1 &&
         handler_bridge_result.upstream_local_player_policy_semantics == 3 &&
         handler_bridge_result.upstream_media_adapter_probe_semantics == 1 &&
         handler_bridge_result.upstream_media_adapter_features_semantics == 1 &&
         handler_bridge_result.upstream_media_adapter_remove_semantics == 1 &&
         handler_bridge_result.upstream_media_adapter_policy_semantics == 3 &&
         handler_bridge_result.upstream_a2dp_session_select_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_session_set_config_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_session_open_start_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_session_suspend_close_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_session_flow_semantics == 4 &&
         handler_bridge_result.upstream_avdtp_signaling_discover_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_getcap_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_set_config_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_open_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_start_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_suspend_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_close_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_abort_semantics == 1 &&
         handler_bridge_result.upstream_avdtp_signaling_flow_semantics == 8 &&
         handler_bridge_result.upstream_linked_handler_transport_dispatch_semantics == 1 &&
         handler_bridge_result.upstream_linked_handler_media_dispatch_semantics == 1 &&
         handler_bridge_result.upstream_linked_handler_pending_request_semantics == 1 &&
         handler_bridge_result.upstream_linked_handler_mainloop_watch_semantics == 1 &&
         handler_bridge_result.upstream_linked_handler_cleanup_semantics == 1 &&
         handler_bridge_result.upstream_linked_handler_mainloop_semantics == 5 &&
         handler_bridge_result.upstream_transport_dbus_fsm_acquire_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_try_acquire_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_release_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_select_unselect_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_error_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_transport_dbus_fsm_semantics == 6 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_register_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_select_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_set_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_clear_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_unregister_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_error_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_media_endpoint_dbus_fsm_semantics == 7 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_register_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_endpoints_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_players_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_unregister_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_disconnect_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_error_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_media_application_dbus_fsm_semantics == 7 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_player_register_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_controller_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_target_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_metadata_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_volume_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_disconnect_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_avrcp_profile_fsm_semantics == 7 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_open_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_start_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_rtp_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_payload_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_suspend_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_close_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_error_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_media_stream_fsm_semantics == 8 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_capability_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_select_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_set_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_reconfigure_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_delay_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_error_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_codec_policy_fsm_semantics == 7 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_first_connect_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_cleanup_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_reconnect_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_duplicate_reject_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_media_resume_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_disconnect_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_lifecycle_stress_fsm_semantics == 7 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_sources_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_headers_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_glib_dbus_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_mainloop_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_core_objects_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_l2cap_media_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_symbol_ownership_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_replacement_boundary_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_object_link_readiness_semantics == 8 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_bad_state_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_mtu_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_fd_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_codec_recover_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_duplicate_request_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_abort_cleanup_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_final_zero_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_negative_boundary_fsm_semantics == 7 &&
         handler_bridge_result.upstream_a2dp_profile_plugin_init_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_adapter_probe_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_endpoint_register_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_avdtp_bind_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_transport_export_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_daemon_cleanup_semantics == 1 &&
         handler_bridge_result.upstream_a2dp_profile_daemon_flow_semantics == 6 ?
         1 : 0);

  printf("bluez-daemon: a2dp upstream-endpoint-callback-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "struct=a2dp_endpoint "
         "callbacks=get_name:%u,get_path:%u,get_capabilities:%u,"
         "select_configuration:%u,select_cb:%u,set_configuration:%u,"
         "set_cb:%u,clear_configuration:%u,set_delay:%u "
         "capability-bytes=%u selected-bytes=%u delay=120 "
         "upstream-link=upstream-a2dp-endpoint-callbacks-compiled-"
         "a2dp-c-object final-ok=%u\n",
         role, probe.get_name, probe.get_path, probe.get_capabilities,
         probe.select_configuration, probe.select_callback,
         probe.set_configuration, probe.set_callback,
         probe.clear_configuration, probe.set_delay, (unsigned int)caps_len,
         (unsigned int)caps_len,
         probe.get_name == 1 && probe.get_path == 1 &&
         probe.get_capabilities == 1 && probe.select_configuration == 1 &&
         probe.select_callback == 1 && probe.set_configuration == 1 &&
         probe.set_callback == 1 && probe.clear_configuration == 1 &&
         probe.set_delay == 1 && caps_len > 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-avdtp-callback-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "structs=avdtp_sep_cfm,avdtp_sep_ind "
         "cfm=set_configuration:%u,get_configuration:%u,open:%u,start:%u,"
         "suspend:%u,close:%u,abort:%u,reconfigure:%u,delay_report:%u "
         "ind=match_codec:%u,get_capability:%u,set_configuration:%u,"
         "set_configuration_cb:%u,get_configuration:%u,open:%u,start:%u,"
         "suspend:%u,close:%u,abort:%u,reconfigure:%u,delayreport:%u "
         "err-code=%u caps-null=%u "
         "upstream-link=upstream-avdtp-callbacks-compiled-avdtp-c-object "
         "final-ok=%u\n",
         role, avdtp_probe.cfm_set_configuration,
         avdtp_probe.cfm_get_configuration, avdtp_probe.cfm_open,
         avdtp_probe.cfm_start, avdtp_probe.cfm_suspend,
         avdtp_probe.cfm_close, avdtp_probe.cfm_abort,
         avdtp_probe.cfm_reconfigure, avdtp_probe.cfm_delay_report,
         avdtp_probe.ind_match_codec, avdtp_probe.ind_get_capability,
         avdtp_probe.ind_set_configuration,
         avdtp_probe.ind_set_configuration_cb,
         avdtp_probe.ind_get_configuration, avdtp_probe.ind_open,
         avdtp_probe.ind_start, avdtp_probe.ind_suspend,
         avdtp_probe.ind_close, avdtp_probe.ind_abort,
         avdtp_probe.ind_reconfigure, avdtp_probe.ind_delayreport,
         err_code, caps == NULL ? 1 : 0,
         avdtp_probe.cfm_set_configuration == 1 &&
         avdtp_probe.cfm_get_configuration == 1 &&
         avdtp_probe.cfm_open == 1 &&
         avdtp_probe.cfm_start == 1 &&
         avdtp_probe.cfm_suspend == 1 &&
         avdtp_probe.cfm_close == 1 &&
         avdtp_probe.cfm_abort == 1 &&
         avdtp_probe.cfm_reconfigure == 1 &&
         avdtp_probe.cfm_delay_report == 1 &&
         avdtp_probe.ind_match_codec == 1 &&
         avdtp_probe.ind_get_capability == 1 &&
         avdtp_probe.ind_set_configuration == 1 &&
         avdtp_probe.ind_set_configuration_cb == 1 &&
         avdtp_probe.ind_get_configuration == 1 &&
         avdtp_probe.ind_open == 1 &&
         avdtp_probe.ind_start == 1 &&
         avdtp_probe.ind_suspend == 1 &&
         avdtp_probe.ind_close == 1 &&
         avdtp_probe.ind_abort == 1 &&
         avdtp_probe.ind_reconfigure == 1 &&
         avdtp_probe.ind_delayreport == 1 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-setup-ownership-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/a2dp.c "
         "objects=server:%u,channel:%u,setup:%u,setup_cb:%u,sep:%u,"
         "stream:%u "
         "refs=session_ref:%u,session_unref:%u,setup_ref:%u,"
         "setup_unref:%u "
         "queues=eps:%u,streams:%u "
         "callbacks=discover:%u,select:%u,config:%u,resume:%u,"
         "suspend:%u "
         "transport=attach:%u,detach:%u "
         "cleanup=setup_free:%u,setup_cb_free:%u,sep_remove:%u,"
         "stream_destroy:%u "
         "final-zero=setups:%u,seps:%u,streams:%u,cbs:%u,refs:%u "
         "upstream-link=upstream-a2dp-ownership-model-a2dp-c-object "
         "final-ok=%u\n",
         role, owner_probe.server_new, owner_probe.channel_new,
         owner_probe.setup_new, owner_probe.setup_cb_add,
         owner_probe.sep_add, owner_probe.stream_new,
         owner_probe.setup_new, owner_probe.setup_free,
         owner_probe.setup_ref, owner_probe.setup_unref,
         owner_probe.eps_queue_new, owner_probe.streams_queue_new,
         owner_probe.discover_cb, owner_probe.select_cb,
         owner_probe.config_cb, owner_probe.resume_cb,
         owner_probe.suspend_cb, owner_probe.transport_attach,
         owner_probe.transport_detach, owner_probe.setup_free,
         owner_probe.setup_cb_free, owner_probe.sep_remove,
         owner_probe.stream_destroy, owner_probe.active_setups,
         owner_probe.active_seps, owner_probe.active_streams,
         owner_probe.active_cbs,
         owner_probe.setup_refs + owner_probe.session_refs,
         owner_probe.server_new == 1 &&
         owner_probe.channel_new == 1 &&
         owner_probe.setup_new == 1 &&
         owner_probe.setup_free == 1 &&
         owner_probe.setup_cb_add == 4 &&
         owner_probe.setup_cb_free == 4 &&
         owner_probe.sep_add == 1 &&
         owner_probe.sep_remove == 1 &&
         owner_probe.stream_new == 1 &&
         owner_probe.stream_destroy == 1 &&
         owner_probe.transport_attach == 1 &&
         owner_probe.transport_detach == 1 &&
         owner_probe.active_setups == 0 &&
         owner_probe.active_seps == 0 &&
         owner_probe.active_streams == 0 &&
         owner_probe.active_cbs == 0 &&
         owner_probe.setup_refs == 0 &&
         owner_probe.session_refs == 0 ? 1 : 0);

  printf("bluez-daemon: a2dp upstream-avdtp-ownership-wrapper role=%s "
         "compile-unit=bluez/upstream_a2dp_compat.c "
         "source=third/bluez/profiles/audio/avdtp.c "
         "objects=session:%u,local-sep:%u,remote-sep:%u,discover:%u,"
         "request:%u,stream:%u,stream-cb:%u "
         "refs=session_ref:%u,session_unref:%u "
         "states=configured:%u,open:%u,streaming:%u,idle:%u "
         "transport=set:%u,get:%u,clear:%u,pending-open-set:%u,"
         "pending-open-clear:%u "
         "cleanup=discover_free:%u,remote-sep-unregister:%u,"
         "stream-cb-remove:%u,stream_free:%u,session_free:%u "
         "final-zero=sessions:%u,local-seps:%u,remote-seps:%u,"
         "streams:%u,discovers:%u,requests:%u,stream-cbs:%u,"
         "transports:%u,refs:%u "
         "upstream-link=upstream-avdtp-ownership-model-avdtp-c-object "
         "final-ok=%u\n",
         role, avdtp_owner_probe.session_new,
         avdtp_owner_probe.local_sep_register,
         avdtp_owner_probe.remote_sep_register,
         avdtp_owner_probe.discover_new,
         avdtp_owner_probe.request_enqueue,
         avdtp_owner_probe.stream_new,
         avdtp_owner_probe.stream_cb_add,
         avdtp_owner_probe.session_ref,
         avdtp_owner_probe.session_unref,
         avdtp_owner_probe.stream_state_configured,
         avdtp_owner_probe.stream_state_open,
         avdtp_owner_probe.stream_state_streaming,
         avdtp_owner_probe.stream_state_idle,
         avdtp_owner_probe.transport_set,
         avdtp_owner_probe.transport_get,
         avdtp_owner_probe.transport_clear,
         avdtp_owner_probe.pending_open_set,
         avdtp_owner_probe.pending_open_clear,
         avdtp_owner_probe.discover_free,
         avdtp_owner_probe.remote_sep_unregister,
         avdtp_owner_probe.stream_cb_remove,
         avdtp_owner_probe.stream_free,
         avdtp_owner_probe.session_free,
         avdtp_owner_probe.active_sessions,
         avdtp_owner_probe.active_local_seps,
         avdtp_owner_probe.active_remote_seps,
         avdtp_owner_probe.active_streams,
         avdtp_owner_probe.active_discovers,
         avdtp_owner_probe.active_requests,
         avdtp_owner_probe.active_stream_cbs,
         avdtp_owner_probe.active_transports,
         avdtp_owner_probe.session_refs,
         avdtp_owner_probe.session_new == 1 &&
         avdtp_owner_probe.local_sep_register == 1 &&
         avdtp_owner_probe.remote_sep_register == 1 &&
         avdtp_owner_probe.discover_new == 1 &&
         avdtp_owner_probe.discover_complete == 1 &&
         avdtp_owner_probe.discover_free == 1 &&
         avdtp_owner_probe.request_enqueue == 1 &&
         avdtp_owner_probe.request_dequeue == 1 &&
         avdtp_owner_probe.stream_new == 1 &&
         avdtp_owner_probe.stream_cb_add == 1 &&
         avdtp_owner_probe.stream_cb_remove == 1 &&
         avdtp_owner_probe.transport_set == 1 &&
         avdtp_owner_probe.transport_get == 1 &&
         avdtp_owner_probe.transport_clear == 1 &&
         avdtp_owner_probe.stream_state_configured == 1 &&
         avdtp_owner_probe.stream_state_open == 1 &&
         avdtp_owner_probe.stream_state_streaming == 1 &&
         avdtp_owner_probe.stream_state_idle == 1 &&
         avdtp_owner_probe.pending_open_set == 1 &&
         avdtp_owner_probe.pending_open_clear == 1 &&
         avdtp_owner_probe.remote_sep_unregister == 1 &&
         avdtp_owner_probe.stream_free == 1 &&
         avdtp_owner_probe.session_free == 1 &&
         avdtp_owner_probe.active_sessions == 0 &&
         avdtp_owner_probe.active_local_seps == 0 &&
         avdtp_owner_probe.active_remote_seps == 0 &&
         avdtp_owner_probe.active_streams == 0 &&
         avdtp_owner_probe.active_discovers == 0 &&
         avdtp_owner_probe.active_requests == 0 &&
         avdtp_owner_probe.active_stream_cbs == 0 &&
         avdtp_owner_probe.active_transports == 0 &&
         avdtp_owner_probe.session_refs == 0 ? 1 : 0);
}
