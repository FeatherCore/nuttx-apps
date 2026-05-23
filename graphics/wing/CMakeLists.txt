# ##############################################################################
# apps/graphics/wing/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_GRAPHICS_WING)
  set(WING_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../stacks/wing)

  set(
    SRCS
    ${WING_ROOT}/src/wing_draw.c
    ${WING_ROOT}/src/wing_draw_sw.c
    ${WING_ROOT}/src/wing_reconcile.c
    ${WING_ROOT}/src/wing_lvgl.c
    ${WING_ROOT}/src/wing_world.c)

  nuttx_add_library(wing STATIC)
  target_sources(wing PRIVATE ${SRCS})
  target_include_directories(wing PUBLIC ${WING_ROOT}/include)

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${WING_ROOT}/include)
endif()
