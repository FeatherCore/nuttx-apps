# ##############################################################################
# apps/examples/wingdemo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_WINGDEMO)
  nuttx_add_application(
    NAME
    wingdemo
    PRIORITY
    ${CONFIG_EXAMPLES_WINGDEMO_PRIORITY}
    STACKSIZE
    ${CONFIG_EXAMPLES_WINGDEMO_STACKSIZE}
    MODULE
    ${CONFIG_EXAMPLES_WINGDEMO}
    DEPENDS
    wing
    SRCS
    wingdemo_main.c
    lvgl/demos/widgets/lv_demo_widgets.c
    lvgl/demos/widgets/lv_demo_widgets_components.c
    lvgl/demos/widgets/lv_demo_widgets_profile.c
    lvgl/demos/widgets/lv_demo_widgets_analytics.c
    lvgl/demos/widgets/lv_demo_widgets_shop.c
    lvgl/demos/widgets/assets/img_demo_widgets_avatar.c
    lvgl/demos/widgets/assets/img_demo_widgets_needle.c
    lvgl/demos/widgets/assets/img_lvgl_logo.c
    lvgl/demos/widgets/assets/img_clothes.c
    INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/lvgl
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/include
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/demos
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/demos/widgets)
endif()
