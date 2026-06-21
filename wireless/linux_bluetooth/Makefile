############################################################################
# apps/wireless/linux_bluetooth/Makefile
#
# SPDX-License-Identifier: Apache-2.0
############################################################################

include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LINUX_BTCTL),y)
MAINSRC += btctl_main.c
PROGNAME += $(CONFIG_LINUX_BTCTL_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BTCTL_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BTCTL_STACKSIZE)
MODULE += $(CONFIG_LINUX_BTCTL)
endif

ifeq ($(CONFIG_LINUX_BTBNEPTEST),y)
MAINSRC += btbneptest_main.c
PROGNAME += $(CONFIG_LINUX_BTBNEPTEST_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BTBNEPTEST_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BTBNEPTEST_STACKSIZE)
MODULE += $(CONFIG_LINUX_BTBNEPTEST)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_BNEPTEST),y)
MAINSRC += bluez/tools/bneptest_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_BNEPTEST_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_BNEPTEST_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_BNEPTEST_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_BNEPTEST)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_NETWORK),y)
MAINSRC += bluez/tools/network_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_NETWORK_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_NETWORK_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_NETWORK_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_NETWORK)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_IPSP),y)
MAINSRC += bluez/tools/ipsp_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_IPSP_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_IPSP_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_IPSP_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_IPSP)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_GATT),y)
MAINSRC += bluez/tools/gatt_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_GATT_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_GATT_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_GATT_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_GATT)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_HID),y)
MAINSRC += bluez/tools/hid_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_HID_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_HID_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_HID_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_HID)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_HFP),y)
MAINSRC += bluez/tools/hfp_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_HFP_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_HFP_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_HFP_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_HFP)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_OBEX),y)
MAINSRC += bluez/tools/obex_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_OBEX_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_OBEX_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_OBEX_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_OBEX)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_PROFILE),y)
CSRCS += bluez/upstream_profile_pdu_codec.c
MAINSRC += bluez/tools/profile_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_PROFILE_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_PROFILE_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_PROFILE_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_PROFILE)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_A2DP),y)
MAINSRC += bluez/tools/a2dp_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_A2DP_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_A2DP_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_A2DP_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_A2DP)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_LEAUDIO),y)
MAINSRC += bluez/tools/leaudio_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_LEAUDIO_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_LEAUDIO_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_LEAUDIO_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_LEAUDIO)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_MGMT),y)
MAINSRC += bluez/tools/mgmt_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_MGMT_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_MGMT_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_MGMT_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_MGMT)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_DAEMON),y)
CSRCS += bluez/upstream_avdtp_object_probe.c
CSRCS += bluez/upstream_a2dp_object_probe.c
CSRCS += bluez/upstream_source_object_probe.c
CSRCS += bluez/upstream_sink_object_probe.c
CSRCS += bluez/upstream_avctp_object_probe.c
CSRCS += bluez/upstream_avrcp_object_probe.c
CSRCS += bluez/upstream_avrcp_player_object_probe.c
CSRCS += bluez/upstream_player_object_probe.c
CSRCS += bluez/upstream_profile_object_probe.c
CSRCS += bluez/upstream_dbus_common_object_probe.c
CSRCS += bluez/upstream_device_object_probe.c
CSRCS += bluez/upstream_service_object_probe.c
CSRCS += bluez/upstream_adapter_object_probe.c
CSRCS += bluez/upstream_mgmt_object_probe.c
CSRCS += bluez/upstream_storage_object_probe.c
CSRCS += bluez/upstream_agent_object_probe.c
CSRCS += bluez/upstream_error_object_probe.c
CSRCS += bluez/upstream_textfile_object_probe.c
CSRCS += bluez/upstream_eir_object_probe.c
CSRCS += bluez/upstream_ad_object_probe.c
CSRCS += bluez/upstream_sdpd_database_object_probe.c
CSRCS += bluez/upstream_sdpd_service_object_probe.c
CSRCS += bluez/upstream_media_owner_object_probe.c
CSRCS += bluez/upstream_device_link_stubs.c
CSRCS += bluez/upstream_mainloop_object_probe.c
CSRCS += bluez/upstream_io_mainloop_object_probe.c
CSRCS += bluez/upstream_transport_object_probe.c
CSRCS += bluez/upstream_media_object_probe.c
CSRCS += bluez/upstream_audio_link_probe.c
CSRCS += bluez/upstream_a2dp_compat.c
CSRCS += bluez/upstream_media_transport_bridge.c
CSRCS += bluez/upstream_manifest.c
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/wireless/linux_bluetooth/bluez/upstream_object_shims
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/wireless/linux_bluetooth/bluez/upstream
MAINSRC += bluez/tools/daemon_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_DAEMON_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_DAEMON_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_DAEMON_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_DAEMON)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_AUDIO),y)
ifeq ($(CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC),y)
LINUX_BLUEZ_SBC_DIR = $(APPDIR)/../../third/sbc-2.0/sbc
CFLAGS += ${INCDIR_PREFIX}$(LINUX_BLUEZ_SBC_DIR)
CSRCS += $(LINUX_BLUEZ_SBC_DIR)/sbc.c
CSRCS += $(LINUX_BLUEZ_SBC_DIR)/sbc_primitives.c
CSRCS += $(LINUX_BLUEZ_SBC_DIR)/sbc_primitives_mmx.c
CSRCS += $(LINUX_BLUEZ_SBC_DIR)/sbc_primitives_sse.c
endif
LINUX_BLUEZ_LC3_DIR = $(APPDIR)/../../third/liblc3
CFLAGS += ${INCDIR_PREFIX}$(LINUX_BLUEZ_LC3_DIR)/include
CFLAGS += ${INCDIR_PREFIX}$(LINUX_BLUEZ_LC3_DIR)/src
CFLAGS += -ffast-math
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/attdet.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/bits.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/bwdet.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/energy.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/lc3.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/ltpf.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/mdct.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/plc.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/sns.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/spec.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/tables.c
CSRCS += $(LINUX_BLUEZ_LC3_DIR)/src/tns.c
CSRCS += bluez/codecs/sbc_backend.c
CSRCS += bluez/codecs/lc3_backend.c
MAINSRC += bluez/tools/audio_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_AUDIO_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_AUDIO_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_AUDIO_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_AUDIO)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_BTMON),y)
MAINSRC += bluez/tools/btmon_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_BTMON_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_BTMON_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_BTMON_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_BTMON)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_HCIIOCTL),y)
MAINSRC += bluez/tools/hciioctl_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_HCIIOCTL_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_HCIIOCTL_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_HCIIOCTL_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_HCIIOCTL)
endif

ifeq ($(CONFIG_LINUX_BLUEZ_HCIRAW),y)
MAINSRC += bluez/tools/hciraw_main.c
PROGNAME += $(CONFIG_LINUX_BLUEZ_HCIRAW_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BLUEZ_HCIRAW_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BLUEZ_HCIRAW_STACKSIZE)
MODULE += $(CONFIG_LINUX_BLUEZ_HCIRAW)
endif

ifeq ($(CONFIG_LINUX_BTAUDIO),y)
MAINSRC += btaudio_main.c
PROGNAME += $(CONFIG_LINUX_BTAUDIO_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BTAUDIO_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BTAUDIO_STACKSIZE)
MODULE += $(CONFIG_LINUX_BTAUDIO)
endif

ifeq ($(CONFIG_LINUX_BTVHCID),y)
MAINSRC += btvhcid_main.c
PROGNAME += $(CONFIG_LINUX_BTVHCID_PROGNAME)
PRIORITY += $(CONFIG_LINUX_BTVHCID_PRIORITY)
STACKSIZE += $(CONFIG_LINUX_BTVHCID_STACKSIZE)
MODULE += $(CONFIG_LINUX_BTVHCID)
endif

include $(APPDIR)/Application.mk
