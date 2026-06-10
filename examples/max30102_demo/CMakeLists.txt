# ##############################################################################
# apps/examples/max30102_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_MAX30102_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_MAX30102_DEMO_PROGNAME}
    SRCS
    max30102_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_MAX30102_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_MAX30102_DEMO_PRIORITY})
endif()
