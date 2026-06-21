# ##############################################################################
# apps/examples/wing_gui_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_WING_GUI_DEMO)
  include_directories(${CMAKE_CURRENT_LIST_DIR}/../../graphics/wing/include)

  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_WING_GUI_DEMO_PROGNAME}
    SRCS
    wing_gui_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_WING_GUI_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_WING_GUI_DEMO_PRIORITY})
endif()
