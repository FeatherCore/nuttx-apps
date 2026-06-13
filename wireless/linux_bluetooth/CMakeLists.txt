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
