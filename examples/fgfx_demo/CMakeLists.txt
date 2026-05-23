# ##############################################################################
# apps/examples/fgfx_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_FGFX_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_FGFX_DEMO_PROGNAME}
    SRCS
    fgfx_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_FGFX_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_FGFX_DEMO_PRIORITY})
endif()
