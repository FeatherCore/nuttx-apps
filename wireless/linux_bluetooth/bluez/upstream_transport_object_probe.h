/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_transport_object_probe.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef BLUEZ_UPSTREAM_TRANSPORT_OBJECT_PROBE_H
#define BLUEZ_UPSTREAM_TRANSPORT_OBJECT_PROBE_H

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void bluez_upstream_transport_object_probe_print(const char *role);
void bluez_upstream_transport_handler_object_probe_print(const char *role);
unsigned int bluez_upstream_transport_acquire_handler_dispatch_bound(void);
unsigned int bluez_upstream_transport_try_acquire_handler_dispatch_bound(void);
unsigned int bluez_upstream_transport_release_handler_dispatch_bound(void);
unsigned int bluez_upstream_transport_select_handler_dispatch_bound(void);
unsigned int bluez_upstream_transport_unselect_handler_dispatch_bound(void);
unsigned int
bluez_upstream_transport_acquire_handler_invocation_handoff_bound(void);
unsigned int
bluez_upstream_transport_acquire_handler_live_body_retained(void);
unsigned int
bluez_upstream_transport_acquire_handler_controlled_invocation_ready(void);
unsigned int
bluez_upstream_transport_acquire_handler_minimal_real_objects_ready(void);
unsigned int
bluez_upstream_transport_create_registered_endpoint_ready(void);
unsigned int
bluez_upstream_transport_export_registered_interface_ready(void);
unsigned int
bluez_upstream_transport_path_allocation_ready(void);
unsigned int
bluez_upstream_transport_registry_lifecycle_ready(void);
unsigned int
bluez_upstream_transport_property_getters_bounded_invoked(void);
unsigned int
bluez_upstream_transport_property_setters_bounded_invoked(void);
unsigned int
bluez_upstream_transport_property_exists_bounded_invoked(void);
unsigned int
bluez_upstream_transport_property_changes_bounded_invoked(void);
unsigned int
bluez_upstream_transport_acquire_handler_bounded_invoked(void);
unsigned int
bluez_upstream_transport_acquire_handler_completion_invoked(void);
unsigned int
bluez_upstream_transport_try_acquire_handler_completion_invoked(void);
unsigned int
bluez_upstream_transport_select_unselect_handler_guard_invoked(void);
unsigned int
bluez_upstream_transport_release_handler_cleanup_invoked(void);
unsigned int
bluez_upstream_transport_destroy_cleanup_invoked(void);
unsigned int
bluez_upstream_transport_error_closeout_invoked(void);
unsigned int
bluez_upstream_transport_a2dp_resume_prepare_ready(void);

#endif /* BLUEZ_UPSTREAM_TRANSPORT_OBJECT_PROBE_H */
