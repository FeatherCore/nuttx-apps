/****************************************************************************
 * apps/wireless/wifi/common/include/nuttx_wifi_config.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Shared compile-time knobs for the staged libnl/wpa_supplicant/hostapd
 * ports.  Keep NuttX-specific app-side definitions here instead of editing
 * upstream imported headers.
 *
 ****************************************************************************/

#ifndef __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_CONFIG_H
#define __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_CONFIG_H

#include <nuttx_wifi_port.h>

#define NUTTX_WIFI_USES_KERNEL_IEEE80211_UAPI 1

#endif /* __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_CONFIG_H */
