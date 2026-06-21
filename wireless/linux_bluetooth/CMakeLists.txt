# ##############################################################################
# apps/wireless/linux_bluetooth/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
# ##############################################################################

if(CONFIG_LINUX_BTCTL)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BTCTL_PROGNAME}
    SRCS
    btctl_main.c
    STACKSIZE
    ${CONFIG_LINUX_BTCTL_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BTCTL_PRIORITY})
endif()

if(CONFIG_LINUX_BTBNEPTEST)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BTBNEPTEST_PROGNAME}
    SRCS
    btbneptest_main.c
    STACKSIZE
    ${CONFIG_LINUX_BTBNEPTEST_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BTBNEPTEST_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_BNEPTEST)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_BNEPTEST_PROGNAME}
    SRCS
    bluez/tools/bneptest_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_BNEPTEST_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_BNEPTEST_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_NETWORK)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_NETWORK_PROGNAME}
    SRCS
    bluez/tools/network_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_NETWORK_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_NETWORK_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_IPSP)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_IPSP_PROGNAME}
    SRCS
    bluez/tools/ipsp_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_IPSP_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_IPSP_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_GATT)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_GATT_PROGNAME}
    SRCS
    bluez/tools/gatt_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_GATT_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_GATT_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_HID)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_HID_PROGNAME}
    SRCS
    bluez/tools/hid_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_HID_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_HID_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_HFP)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_HFP_PROGNAME}
    SRCS
    bluez/tools/hfp_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_HFP_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_HFP_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_OBEX)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_OBEX_PROGNAME}
    SRCS
    bluez/tools/obex_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_OBEX_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_OBEX_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_PROFILE)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_PROFILE_PROGNAME}
    SRCS
    bluez/upstream_profile_pdu_codec.c
    bluez/tools/profile_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_PROFILE_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_PROFILE_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_A2DP)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_A2DP_PROGNAME}
    SRCS
    bluez/tools/a2dp_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_A2DP_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_A2DP_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_LEAUDIO)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_LEAUDIO_PROGNAME}
    SRCS
    bluez/tools/leaudio_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_LEAUDIO_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_LEAUDIO_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_MGMT)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_MGMT_PROGNAME}
    SRCS
    bluez/tools/mgmt_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_MGMT_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_MGMT_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_DAEMON)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_DAEMON_PROGNAME}
    SRCS
    bluez/upstream_avdtp_object_probe.c
    bluez/upstream_a2dp_object_probe.c
    bluez/upstream_source_object_probe.c
    bluez/upstream_sink_object_probe.c
    bluez/upstream_avctp_object_probe.c
    bluez/upstream_avrcp_object_probe.c
    bluez/upstream_avrcp_player_object_probe.c
    bluez/upstream_player_object_probe.c
    bluez/upstream_profile_object_probe.c
    bluez/upstream_dbus_common_object_probe.c
    bluez/upstream_device_object_probe.c
    bluez/upstream_service_object_probe.c
    bluez/upstream_adapter_object_probe.c
    bluez/upstream_mgmt_object_probe.c
    bluez/upstream_storage_object_probe.c
    bluez/upstream_agent_object_probe.c
    bluez/upstream_error_object_probe.c
    bluez/upstream_textfile_object_probe.c
    bluez/upstream_eir_object_probe.c
    bluez/upstream_ad_object_probe.c
    bluez/upstream_sdpd_database_object_probe.c
    bluez/upstream_sdpd_service_object_probe.c
    bluez/upstream_media_owner_object_probe.c
    bluez/upstream_device_link_stubs.c
    bluez/upstream_mainloop_object_probe.c
    bluez/upstream_io_mainloop_object_probe.c
    bluez/upstream_transport_object_probe.c
    bluez/upstream_media_object_probe.c
    bluez/upstream_audio_link_probe.c
    bluez/upstream_a2dp_compat.c
    bluez/upstream_media_transport_bridge.c
    bluez/upstream_manifest.c
    bluez/tools/daemon_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_DAEMON_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_DAEMON_PRIORITY}
    COMPILE_FLAGS
    ${INCDIR_PREFIX}${CMAKE_CURRENT_LIST_DIR}/bluez/upstream_object_shims
    ${INCDIR_PREFIX}${CMAKE_CURRENT_LIST_DIR}/bluez/upstream)
endif()

if(CONFIG_LINUX_BLUEZ_AUDIO)
  set(LINUX_BLUEZ_AUDIO_SRCS
      bluez/codecs/sbc_backend.c
      bluez/codecs/lc3_backend.c
      bluez/tools/audio_main.c)
  set(LINUX_BLUEZ_AUDIO_CFLAGS)

  if(CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC)
    set(LINUX_BLUEZ_SBC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../third/sbc-2.0/sbc)
    list(APPEND LINUX_BLUEZ_AUDIO_CFLAGS ${INCDIR_PREFIX}${LINUX_BLUEZ_SBC_DIR})
    list(APPEND LINUX_BLUEZ_AUDIO_SRCS
         ${LINUX_BLUEZ_SBC_DIR}/sbc.c
         ${LINUX_BLUEZ_SBC_DIR}/sbc_primitives.c
         ${LINUX_BLUEZ_SBC_DIR}/sbc_primitives_mmx.c
         ${LINUX_BLUEZ_SBC_DIR}/sbc_primitives_sse.c)
  endif()

  set(LINUX_BLUEZ_LC3_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../third/liblc3)
  list(APPEND LINUX_BLUEZ_AUDIO_CFLAGS
       ${INCDIR_PREFIX}${LINUX_BLUEZ_LC3_DIR}/include
       ${INCDIR_PREFIX}${LINUX_BLUEZ_LC3_DIR}/src
       -ffast-math)
  list(APPEND LINUX_BLUEZ_AUDIO_SRCS
       ${LINUX_BLUEZ_LC3_DIR}/src/attdet.c
       ${LINUX_BLUEZ_LC3_DIR}/src/bits.c
       ${LINUX_BLUEZ_LC3_DIR}/src/bwdet.c
       ${LINUX_BLUEZ_LC3_DIR}/src/energy.c
       ${LINUX_BLUEZ_LC3_DIR}/src/lc3.c
       ${LINUX_BLUEZ_LC3_DIR}/src/ltpf.c
       ${LINUX_BLUEZ_LC3_DIR}/src/mdct.c
       ${LINUX_BLUEZ_LC3_DIR}/src/plc.c
       ${LINUX_BLUEZ_LC3_DIR}/src/sns.c
       ${LINUX_BLUEZ_LC3_DIR}/src/spec.c
       ${LINUX_BLUEZ_LC3_DIR}/src/tables.c
       ${LINUX_BLUEZ_LC3_DIR}/src/tns.c)

  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_AUDIO_PROGNAME}
    SRCS
    ${LINUX_BLUEZ_AUDIO_SRCS}
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_AUDIO_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_AUDIO_PRIORITY}
    COMPILE_FLAGS
    ${LINUX_BLUEZ_AUDIO_CFLAGS})
endif()

if(CONFIG_LINUX_BLUEZ_BTMON)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_BTMON_PROGNAME}
    SRCS
    bluez/tools/btmon_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_BTMON_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_BTMON_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_HCIIOCTL)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_HCIIOCTL_PROGNAME}
    SRCS
    bluez/tools/hciioctl_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_HCIIOCTL_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_HCIIOCTL_PRIORITY})
endif()

if(CONFIG_LINUX_BLUEZ_HCIRAW)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BLUEZ_HCIRAW_PROGNAME}
    SRCS
    bluez/tools/hciraw_main.c
    STACKSIZE
    ${CONFIG_LINUX_BLUEZ_HCIRAW_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BLUEZ_HCIRAW_PRIORITY})
endif()

if(CONFIG_LINUX_BTAUDIO)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BTAUDIO_PROGNAME}
    SRCS
    btaudio_main.c
    STACKSIZE
    ${CONFIG_LINUX_BTAUDIO_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BTAUDIO_PRIORITY})
endif()

if(CONFIG_LINUX_BTVHCID)
  nuttx_add_application(
    NAME
    ${CONFIG_LINUX_BTVHCID_PROGNAME}
    SRCS
    btvhcid_main.c
    STACKSIZE
    ${CONFIG_LINUX_BTVHCID_STACKSIZE}
    PRIORITY
    ${CONFIG_LINUX_BTVHCID_PRIORITY})
endif()
