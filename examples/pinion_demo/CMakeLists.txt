# ##############################################################################
# apps/examples/pinion_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_PINION_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_PINION_DEMO_PROGNAME}
    PRIORITY
    ${CONFIG_EXAMPLES_PINION_DEMO_PRIORITY}
    STACKSIZE
    ${CONFIG_EXAMPLES_PINION_DEMO_STACKSIZE}
    MODULE
    ${CONFIG_EXAMPLES_PINION_DEMO}
    DEPENDS
    pinion
    SRCS
    pinion_demo_main.c)
endif()
