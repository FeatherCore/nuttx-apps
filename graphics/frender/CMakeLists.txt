# ##############################################################################
# apps/graphics/frender/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_GRAPHICS_FRENDER)
  set(FRENDER_ROOT ${CMAKE_CURRENT_LIST_DIR})

  nuttx_add_library(frender STATIC)

  # Core
  target_sources(frender PRIVATE ${FRENDER_ROOT}/src/frender.c)

  if(CONFIG_GRAPHICS_FRENDER_FB)
    target_sources(frender PRIVATE ${FRENDER_ROOT}/src/fr_backend_framebuffer.c)
  endif()

  if(CONFIG_GRAPHICS_FRENDER_NX)
    target_sources(frender PRIVATE ${FRENDER_ROOT}/src/fr_backend_nx.c)
  endif()

  target_include_directories(frender PUBLIC ${FRENDER_ROOT}/include)
  target_include_directories(frender PRIVATE ${FRENDER_ROOT}/src)

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${FRENDER_ROOT}/include)
endif()
