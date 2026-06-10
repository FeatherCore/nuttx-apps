# ##############################################################################
# apps/examples/wifi_hwsim_ap/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_WIFI_HWSIM_AP)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_WIFI_HWSIM_AP_PROGNAME}
    SRCS
    wifi_hwsim_ap_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_WIFI_HWSIM_AP_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_WIFI_HWSIM_AP_PRIORITY})
endif()
