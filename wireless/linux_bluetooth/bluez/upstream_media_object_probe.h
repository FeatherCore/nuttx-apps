/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_media_object_probe.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef BLUEZ_UPSTREAM_MEDIA_OBJECT_PROBE_H
#define BLUEZ_UPSTREAM_MEDIA_OBJECT_PROBE_H

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void bluez_upstream_media_object_probe_print(const char *role);
void bluez_upstream_media_handler_object_probe_print(const char *role);
unsigned int bluez_upstream_media_register_endpoint_handler_dispatch_bound(void);
unsigned int
bluez_upstream_media_unregister_endpoint_handler_dispatch_bound(void);
unsigned int
bluez_upstream_media_register_endpoint_handler_invocation_handoff_bound(void);
unsigned int bluez_upstream_media_transport_cross_object_dependency_bound(void);
unsigned int
bluez_upstream_media_register_endpoint_handler_live_body_retained(void);
unsigned int
bluez_upstream_media_register_endpoint_handler_controlled_invocation_ready(
  void);
unsigned int
bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready(
  void);
unsigned int
bluez_upstream_media_register_endpoint_handler_bounded_invoked(void);
unsigned int
bluez_upstream_media_register_endpoint_handler_registered_endpoint_ready(void);
unsigned int
bluez_upstream_media_register_endpoint_lifecycle_ready(void);
unsigned int
bluez_upstream_media_register_endpoint_error_policy_ready(void);
unsigned int
bluez_upstream_media_unregister_endpoint_lifecycle_ready(void);
unsigned int
bluez_upstream_media_unregister_endpoint_error_policy_ready(void);
unsigned int
bluez_upstream_media_endpoint_select_configuration_request_ready(void);
unsigned int
bluez_upstream_media_endpoint_select_configuration_reply_ready(void);
unsigned int
bluez_upstream_media_endpoint_select_configuration_error_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_configuration_request_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_configuration_error_ready(void);
unsigned int
bluez_upstream_media_endpoint_clear_configuration_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_registered_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_remote_lookup_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_parsed_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_getcap_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_dispatch_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_packet_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_session_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_discover_remote_ready(void);
unsigned int
bluez_upstream_media_endpoint_set_l2cap_remote_ready(void);
struct media_endpoint *
bluez_upstream_media_create_registered_a2dp_endpoint_for_transport(void);

#endif /* BLUEZ_UPSTREAM_MEDIA_OBJECT_PROBE_H */
