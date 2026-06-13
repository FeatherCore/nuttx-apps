# ##############################################################################
# apps/graphics/frender/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_GRAPHICS_FRENDER)
  set(FRENDER_ROOT ${CMAKE_CURRENT_LIST_DIR})

  nuttx_add_library(frender STATIC)
  target_sources(frender PRIVATE ${FRENDER_ROOT}/src/frender.c)
  target_include_directories(frender PUBLIC ${FRENDER_ROOT}/include)

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${FRENDER_ROOT}/include)
endif()
