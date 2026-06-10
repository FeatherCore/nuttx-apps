/****************************************************************************
 * apps/wireless/wifi/common/include/nuttx_wifi_port.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_PORT_H
#define __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_PORT_H

#include <nuttx/config.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NUTTX_WIFI_PORT_LIBNL_VERSION "3.2.25"
#define NUTTX_WIFI_PORT_WPA_VERSION   "2.11"
#define NUTTX_WIFI_PORT_HOSTAPD_VERSION "2.11"
#define NUTTX_WIFI_USERSPACE_PORT 1

#define NUTTX_WIFI_UNUSED(x) ((void)(x))

#endif /* __APPS_WIRELESS_WIFI_COMMON_INCLUDE_NUTTX_WIFI_PORT_H */
