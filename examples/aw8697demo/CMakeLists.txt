# ##############################################################################
# apps/examples/aw8697demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_AW8697DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_AW8697DEMO_PROGNAME}
    SRCS
    aw8697demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_AW8697DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_AW8697DEMO_PRIORITY})
endif()
