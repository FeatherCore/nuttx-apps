/****************************************************************************
 * apps/wireless/wifi/common/nuttx_wpa_p384_ecdh.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Simulator-only P-384 ECDH wrapper for OWE group 20.
 ****************************************************************************/

#include "includes.h"

#include <stdint.h>
#include <string.h>

#include "common.h"

#define ECC_CURVE secp384r1
#define ecc_make_key nuttx_wifi_p384_ecc_make_key
#define ecc_make_key_uncomp nuttx_wifi_p384_ecc_make_key_uncomp
#define ecdh_shared_secret nuttx_wifi_p384_ecdh_shared_secret_compressed
#define ecdsa_sign nuttx_wifi_p384_ecdsa_sign
#define ecdsa_verify nuttx_wifi_p384_ecdsa_verify

#include "../../../../nuttx/crypto/ecc.c"

#define NUTTX_WIFI_P384_LEN 48

int nuttx_wifi_p384_make_key(u8 publickey[2 * NUTTX_WIFI_P384_LEN],
                             u8 privatekey[NUTTX_WIFI_P384_LEN])
{
  u8 *x = publickey;
  u8 *y = publickey + NUTTX_WIFI_P384_LEN;

  return nuttx_wifi_p384_ecc_make_key_uncomp(x, y, privatekey) ? 0 : -1;
}

int nuttx_wifi_p384_shared_secret(const u8 *publickey, size_t publickey_len,
                                  int inc_y,
                                  const u8 privatekey[NUTTX_WIFI_P384_LEN],
                                  u8 secret[NUTTX_WIFI_P384_LEN])
{
  u8 compressed[NUTTX_WIFI_P384_LEN + 1];

  if (publickey == NULL || privatekey == NULL || secret == NULL)
    {
      return -1;
    }

  if (inc_y)
    {
      if (publickey_len != 2 * NUTTX_WIFI_P384_LEN)
        {
          return -1;
        }

      compressed[0] = 2 + (publickey[2 * NUTTX_WIFI_P384_LEN - 1] & 1);
    }
  else
    {
      if (publickey_len != NUTTX_WIFI_P384_LEN)
        {
          return -1;
        }

      compressed[0] = 2;
    }

  memcpy(compressed + 1, publickey, NUTTX_WIFI_P384_LEN);

  return nuttx_wifi_p384_ecdh_shared_secret_compressed(compressed, privatekey,
                                                       secret) ? 0 : -1;
}
