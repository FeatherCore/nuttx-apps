/****************************************************************************
 * apps/wireless/linux_bluetooth/bluez/upstream_ad_object_probe.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#undef false
#define false 0

#define UINT_TO_PTR(u) ((void *)(uintptr_t)(u))
#define PTR_TO_UINT(p) ((unsigned int)(uintptr_t)(p))

/****************************************************************************
 * Upstream Object Include
 ****************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include "upstream/src/shared/ad.c"

#pragma GCC diagnostic pop

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void bluez_upstream_ad_object_probe_print(const char *role)
{
  printf("bluez-upstream-object: src/shared/ad.c role=%s linked=1 "
         "source=third/bluez/src/shared/ad.c api=bt_ad\n",
         role);
}
