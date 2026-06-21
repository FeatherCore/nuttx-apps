# ##############################################################################
# apps/examples/frender_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_FRENDER_DEMO)
  include_directories(${CMAKE_CURRENT_LIST_DIR}/../../graphics/frender/include)

  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_FRENDER_DEMO_PROGNAME}
    SRCS
    frender_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_FRENDER_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_FRENDER_DEMO_PRIORITY})
endif()
