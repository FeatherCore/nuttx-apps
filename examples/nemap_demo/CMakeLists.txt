############################################################################
# apps/examples/nemap_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
############################################################################

if(CONFIG_EXAMPLES_NEMAP_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_NEMAP_DEMO_PROGNAME}
    SRCS
    nemap_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_NEMAP_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_NEMAP_DEMO_PRIORITY})
endif()
