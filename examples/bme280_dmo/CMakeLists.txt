# ##############################################################################
# apps/examples/bme280_dmo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_BME280_DMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_BME280_DMO_PROGNAME}
    SRCS
    bme280_dmo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_BME280_DMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_BME280_DMO_PRIORITY})
endif()
