# ##############################################################################
# apps/examples/wifi_hwsim_sta/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_WIFI_HWSIM_STA)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_WIFI_HWSIM_STA_PROGNAME}
    SRCS
    wifi_hwsim_sta_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_WIFI_HWSIM_STA_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_WIFI_HWSIM_STA_PRIORITY})
endif()
