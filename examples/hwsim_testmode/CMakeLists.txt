# ##############################################################################
# apps/examples/hwsim_testmode/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# ##############################################################################

if(CONFIG_EXAMPLES_HWSIM_TESTMODE)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_HWSIM_TESTMODE_PROGNAME}
    SRCS
    hwsim_testmode_main.c
    STACKSIZE
    ${CONFIG_EXAMPLES_HWSIM_TESTMODE_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_HWSIM_TESTMODE_PRIORITY})
endif()
