/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_eir_object_probe.c
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

#define eir_get_service_data bluez_upstream_object_eir_get_service_data

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "upstream/src/eir.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_eir_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/eir.c role=%s linked=1 "
         "source=third/bluez/src/eir.c api=eir\n",
         role);
}
