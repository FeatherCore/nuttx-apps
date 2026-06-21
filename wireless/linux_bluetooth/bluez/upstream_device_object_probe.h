/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_device_object_probe.h
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#ifndef APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_DEVICE_OBJECT_PROBE_H
#define APPS_WIRELESS_LINUX_BLUETOOTH_BLUEZ_UPSTREAM_DEVICE_OBJECT_PROBE_H

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void bluez_upstream_device_object_probe_print(const char *role);
unsigned int bluez_upstream_device_dependency_bound(void);

#endif
