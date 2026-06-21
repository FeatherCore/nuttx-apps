# ##############################################################################
# apps/graphics/pinion/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_GRAPHICS_PINION)
  set(PINION_ROOT ${CMAKE_CURRENT_LIST_DIR})

  nuttx_add_library(pinion STATIC)
  target_sources(pinion PRIVATE ${PINION_ROOT}/src/pinion.c)
  target_include_directories(pinion PUBLIC ${PINION_ROOT}/include)

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${PINION_ROOT}/include)
endif()
