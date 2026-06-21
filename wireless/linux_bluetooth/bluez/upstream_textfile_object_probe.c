/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_textfile_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef STORAGEDIR
#  define STORAGEDIR "/tmp/bluez"
#endif

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "upstream/src/textfile.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_textfile_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/textfile.c role=%s linked=1 "
         "source=third/bluez/src/textfile.c api=textfile\n",
         role);
}
