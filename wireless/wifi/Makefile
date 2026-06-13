############################################################################
# apps/wireless/wifi/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)common$(DELIM)include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)libnl-3.2.25$(DELIM)include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)libnl-3.2.25$(DELIM)lib
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)wpa_supplicant-2.11$(DELIM)src
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)wpa_supplicant-2.11$(DELIM)src$(DELIM)utils
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)wpa_supplicant-2.11$(DELIM)wpa_supplicant
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)$(DELIM)wireless$(DELIM)wifi$(DELIM)hostapd-2.11$(DELIM)hostapd
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)$(DELIM)wireless$(DELIM)ieee80211$(DELIM)include
CFLAGS += ${INCDIR_PREFIX}$(TOPDIR)$(DELIM)wireless$(DELIM)ieee80211$(DELIM)include$(DELIM)uapi
CFLAGS += -include nuttx_libnl_compat.h

ifeq ($(CONFIG_WIRELESS_WIFI_LIBNL),y)
CSRCS += common/nuttx_wifi_compat.c
ifeq ($(CONFIG_ARCH_SIM),y)
ifneq ($(CONFIG_WIRELESS_WIFI_WPA_SUPPLICANT)$(CONFIG_WIRELESS_WIFI_HOSTAPD),)
CSRCS += common/nuttx_wpa_openssl_ec.c
CSRCS += common/nuttx_wpa_p384_ecdh.c
endif
endif
CSRCS += libnl-3.2.25/lib/addr.c
CSRCS += libnl-3.2.25/lib/attr.c
CSRCS += libnl-3.2.25/lib/cache.c
CSRCS += libnl-3.2.25/lib/cache_mngr.c
CSRCS += libnl-3.2.25/lib/cache_mngt.c
CSRCS += libnl-3.2.25/lib/data.c
CSRCS += libnl-3.2.25/lib/error.c
CSRCS += libnl-3.2.25/lib/handlers.c
CSRCS += libnl-3.2.25/lib/hash.c
CSRCS += libnl-3.2.25/lib/hashtable.c
CSRCS += libnl-3.2.25/lib/msg.c
CSRCS += libnl-3.2.25/lib/nl.c
CSRCS += libnl-3.2.25/lib/object.c
CSRCS += libnl-3.2.25/lib/socket.c
CSRCS += libnl-3.2.25/lib/utils.c
CSRCS += libnl-3.2.25/lib/version.c
CSRCS += libnl-3.2.25/lib/genl/ctrl.c
CSRCS += libnl-3.2.25/lib/genl/family.c
CSRCS += libnl-3.2.25/lib/genl/genl.c
CSRCS += libnl-3.2.25/lib/genl/mngt.c
CSRCS += libnl-3.2.25/lib/route/link.c
CSRCS += libnl-3.2.25/lib/route/link/api.c
CSRCS += libnl-3.2.25/lib/route/link/vlan.c
CSRCS += libnl-3.2.25/lib/route/rtnl.c
endif

include wpa_hostapd_sources.mk

include $(APPDIR)/Application.mk
