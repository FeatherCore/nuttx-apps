# ##############################################################################
# apps/examples/lsm6dsox_demo/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_LSM6DSOX_DEMO)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_LSM6DSOX_DEMO_PROGNAME}
    SRCS
    lsm6dsox_demo_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_LSM6DSOX_DEMO_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_LSM6DSOX_DEMO_PRIORITY})
endif()
