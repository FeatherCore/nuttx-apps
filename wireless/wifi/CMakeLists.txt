# ##############################################################################
# apps/wireless/wifi/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_WIRELESS_WIFI_BUILD_EXPERIMENTAL
   OR CONFIG_WIRELESS_WIFI_LIBNL
   OR CONFIG_WIRELESS_WIFI_WPA_SUPPLICANT
   OR CONFIG_WIRELESS_WIFI_HOSTAPD)
  set(
    LIBNL_SRCS
    libnl-3.2.25/lib/addr.c
    libnl-3.2.25/lib/attr.c
    libnl-3.2.25/lib/cache.c
    libnl-3.2.25/lib/cache_mngr.c
    libnl-3.2.25/lib/cache_mngt.c
    libnl-3.2.25/lib/data.c
    libnl-3.2.25/lib/error.c
    libnl-3.2.25/lib/handlers.c
    libnl-3.2.25/lib/hash.c
    libnl-3.2.25/lib/hashtable.c
    libnl-3.2.25/lib/msg.c
    libnl-3.2.25/lib/nl.c
    libnl-3.2.25/lib/object.c
    libnl-3.2.25/lib/socket.c
    libnl-3.2.25/lib/utils.c
    libnl-3.2.25/lib/version.c
    libnl-3.2.25/lib/genl/ctrl.c
    libnl-3.2.25/lib/genl/family.c
    libnl-3.2.25/lib/genl/genl.c
    libnl-3.2.25/lib/genl/mngt.c)

  target_sources(apps PRIVATE ${LIBNL_SRCS})
  target_include_directories(
    apps
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/common/include
            ${CMAKE_CURRENT_LIST_DIR}/libnl-3.2.25/include
            ${CMAKE_CURRENT_LIST_DIR}/libnl-3.2.25/lib
            ${NUTTX_DIR}/wireless/ieee80211/include
            ${NUTTX_DIR}/wireless/ieee80211/include/uapi)
  target_compile_options(apps PRIVATE -include nuttx_libnl_compat.h)
endif()
