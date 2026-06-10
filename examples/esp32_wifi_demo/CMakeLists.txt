# ##############################################################################
# apps/examples/esp32_wifi_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_ESP32_WIFI_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_ESP32_WIFI_DEMO_PROGNAME}
    SRCS
    esp32_wifi_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_ESP32_WIFI_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_ESP32_WIFI_DEMO_PRIORITY})
endif()
