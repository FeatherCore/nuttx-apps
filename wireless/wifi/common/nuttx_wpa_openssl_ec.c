/****************************************************************************
 * apps/wireless/wifi/common/nuttx_wpa_openssl_ec.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Simulator-only bignum/ECC glue for WPA3 SAE while the rest of the staged
 * wpa_supplicant/hostapd port continues to use the existing internal crypto
 * and TLS sources.
 ****************************************************************************/

#include "includes.h"

#include <stdint.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/encoder.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>
#include <openssl/x509.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "crypto/crypto.h"
#include "crypto/random.h"

struct crypto_ec
{
  int nid;
  EC_GROUP *group;
  BN_CTX *bnctx;
  BIGNUM *prime;
  BIGNUM *order;
  BIGNUM *a;
  BIGNUM *b;
};

struct nuttx_wpa_ec_key
{
  int group;
  int nid;
  struct crypto_ec *ec;
  BIGNUM *priv;
  EC_POINT *pub;
};

static void nuttx_wpa_mix_sim_entropy(u8 *buf, size_t len)
{
  static uint64_t sequence;
  struct timeval tv;
  uint64_t state;
  size_t i;

  if (gettimeofday(&tv, NULL) < 0)
    {
      tv.tv_sec = 0;
      tv.tv_usec = 0;
    }

  state = ((uint64_t)(uintptr_t)buf) ^
          ((uint64_t)(uintptr_t)&buf << 7) ^
          ((uint64_t)(uintptr_t)&sequence >> 3) ^
          ((uint64_t)getpid() << 32) ^
          ((uint64_t)tv.tv_sec << 16) ^
          (uint64_t)tv.tv_usec ^
          ++sequence;

  /* Multiple NuttX sim instances can start with identical RNG state. Mix in
   * process-local data so SAE peers do not generate reflected commits.
   */

  for (i = 0; i < len; i++)
    {
      state ^= state >> 12;
      state ^= state << 25;
      state ^= state >> 27;
      state *= UINT64_C(2685821657736338717);
      buf[i] ^= (u8)(state >> ((i & 7) * 8));
    }
}

struct crypto_bignum *crypto_bignum_init(void)
{
  return (struct crypto_bignum *)BN_new();
}

struct crypto_bignum *crypto_bignum_init_set(const u8 *buf, size_t len)
{
  return (struct crypto_bignum *)BN_bin2bn(buf, len, NULL);
}

struct crypto_bignum *crypto_bignum_init_uint(unsigned int val)
{
  BIGNUM *bn = BN_new();

  if (bn == NULL)
    {
      return NULL;
    }

  if (BN_set_word(bn, val) != 1)
    {
      BN_free(bn);
      return NULL;
    }

  return (struct crypto_bignum *)bn;
}

void crypto_bignum_deinit(struct crypto_bignum *n, int clear)
{
  if (clear)
    {
      BN_clear_free((BIGNUM *)n);
    }
  else
    {
      BN_free((BIGNUM *)n);
    }
}

int crypto_bignum_to_bin(const struct crypto_bignum *a,
                         u8 *buf, size_t buflen, size_t padlen)
{
  int num_bytes;
  size_t offset;

  if (a == NULL || buf == NULL || padlen > buflen)
    {
      return -1;
    }

  if (padlen)
    {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
      return BN_bn2binpad((const BIGNUM *)a, buf, padlen);
#endif
    }

  num_bytes = BN_num_bytes((const BIGNUM *)a);
  if ((size_t)num_bytes > buflen)
    {
      return -1;
    }

  offset = padlen > (size_t)num_bytes ? padlen - (size_t)num_bytes : 0;
  memset(buf, 0, offset);
  BN_bn2bin((const BIGNUM *)a, buf + offset);

  return num_bytes + offset;
}

int crypto_bignum_rand(struct crypto_bignum *r,
                       const struct crypto_bignum *m)
{
  size_t len;
  unsigned int top_bits;
  u8 *buf;
  int ret = -1;
  int i;

  if (r == NULL || m == NULL || BN_is_zero((const BIGNUM *)m) ||
      BN_is_negative((const BIGNUM *)m))
    {
      return -1;
    }

  len = BN_num_bytes((const BIGNUM *)m);
  top_bits = BN_num_bits((const BIGNUM *)m) % 8;
  buf = os_malloc(len);
  if (buf == NULL)
    {
      return -1;
    }

  for (i = 0; i < 100; i++)
    {
      if (random_get_bytes(buf, len) < 0)
        {
          goto out;
        }

      nuttx_wpa_mix_sim_entropy(buf, len);

      if (top_bits != 0)
        {
          buf[0] &= (1u << top_bits) - 1u;
        }

      if (BN_bin2bn(buf, len, (BIGNUM *)r) == NULL)
        {
          goto out;
        }

      if (BN_cmp((BIGNUM *)r, (const BIGNUM *)m) < 0)
        {
          ret = 0;
          goto out;
        }
    }

out:
  bin_clear_free(buf, len);
  return ret;
}

int crypto_bignum_add(const struct crypto_bignum *a,
                      const struct crypto_bignum *b,
                      struct crypto_bignum *c)
{
  return BN_add((BIGNUM *)c, (const BIGNUM *)a, (const BIGNUM *)b) ? 0 : -1;
}

int crypto_bignum_mod(const struct crypto_bignum *a,
                      const struct crypto_bignum *b,
                      struct crypto_bignum *c)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod((BIGNUM *)c, (const BIGNUM *)a, (const BIGNUM *)b, bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_exptmod(const struct crypto_bignum *a,
                          const struct crypto_bignum *b,
                          const struct crypto_bignum *c,
                          struct crypto_bignum *d)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod_exp((BIGNUM *)d, (const BIGNUM *)a, (const BIGNUM *)b,
                   (const BIGNUM *)c, bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_inverse(const struct crypto_bignum *a,
                          const struct crypto_bignum *b,
                          struct crypto_bignum *c)
{
  BIGNUM *ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod_inverse((BIGNUM *)c, (const BIGNUM *)a, (const BIGNUM *)b,
                       bnctx);
  BN_CTX_free(bnctx);
  return ret != NULL ? 0 : -1;
}

int crypto_bignum_sub(const struct crypto_bignum *a,
                      const struct crypto_bignum *b,
                      struct crypto_bignum *c)
{
  return BN_sub((BIGNUM *)c, (const BIGNUM *)a, (const BIGNUM *)b) ? 0 : -1;
}

int crypto_bignum_div(const struct crypto_bignum *a,
                      const struct crypto_bignum *b,
                      struct crypto_bignum *c)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_div((BIGNUM *)c, NULL, (const BIGNUM *)a, (const BIGNUM *)b,
               bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_addmod(const struct crypto_bignum *a,
                         const struct crypto_bignum *b,
                         const struct crypto_bignum *c,
                         struct crypto_bignum *d)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod_add((BIGNUM *)d, (const BIGNUM *)a, (const BIGNUM *)b,
                   (const BIGNUM *)c, bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_mulmod(const struct crypto_bignum *a,
                         const struct crypto_bignum *b,
                         const struct crypto_bignum *c,
                         struct crypto_bignum *d)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod_mul((BIGNUM *)d, (const BIGNUM *)a, (const BIGNUM *)b,
                   (const BIGNUM *)c, bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_sqrmod(const struct crypto_bignum *a,
                         const struct crypto_bignum *b,
                         struct crypto_bignum *c)
{
  int ret;
  BN_CTX *bnctx = BN_CTX_new();

  if (bnctx == NULL)
    {
      return -1;
    }

  ret = BN_mod_sqr((BIGNUM *)c, (const BIGNUM *)a, (const BIGNUM *)b,
                   bnctx);
  BN_CTX_free(bnctx);
  return ret == 1 ? 0 : -1;
}

int crypto_bignum_rshift(const struct crypto_bignum *a, int n,
                         struct crypto_bignum *r)
{
  return BN_rshift((BIGNUM *)r, (const BIGNUM *)a, n) == 1 ? 0 : -1;
}

int crypto_bignum_cmp(const struct crypto_bignum *a,
                      const struct crypto_bignum *b)
{
  return BN_cmp((const BIGNUM *)a, (const BIGNUM *)b);
}

int crypto_bignum_is_zero(const struct crypto_bignum *a)
{
  return BN_is_zero((const BIGNUM *)a);
}

int crypto_bignum_is_one(const struct crypto_bignum *a)
{
  return BN_is_one((const BIGNUM *)a);
}

int crypto_bignum_is_odd(const struct crypto_bignum *a)
{
  return BN_is_odd((const BIGNUM *)a);
}

int crypto_bignum_legendre(const struct crypto_bignum *a,
                           const struct crypto_bignum *p)
{
  int ret = -2;
  BIGNUM *exp = BN_new();
  BIGNUM *tmp = BN_new();
  BN_CTX *bnctx = BN_CTX_new();

  if (exp == NULL || tmp == NULL || bnctx == NULL)
    {
      goto out;
    }

  if (BN_sub(exp, (const BIGNUM *)p, BN_value_one()) != 1 ||
      BN_rshift1(exp, exp) != 1 ||
      BN_mod_exp(tmp, (const BIGNUM *)a, exp, (const BIGNUM *)p,
                 bnctx) != 1)
    {
      goto out;
    }

  if (BN_is_zero(tmp))
    {
      ret = 0;
    }
  else if (BN_is_one(tmp))
    {
      ret = 1;
    }
  else
    {
      ret = -1;
    }

out:
  BN_clear_free(tmp);
  BN_clear_free(exp);
  BN_CTX_free(bnctx);
  return ret;
}

static int crypto_ec_group_to_nid(int group)
{
  switch (group)
    {
      case 19:
        return NID_X9_62_prime256v1;
      case 20:
        return NID_secp384r1;
      case 21:
        return NID_secp521r1;
      case 25:
        return NID_X9_62_prime192v1;
      case 26:
        return NID_secp224r1;
#ifdef NID_brainpoolP224r1
      case 27:
        return NID_brainpoolP224r1;
#endif
#ifdef NID_brainpoolP256r1
      case 28:
        return NID_brainpoolP256r1;
#endif
#ifdef NID_brainpoolP384r1
      case 29:
        return NID_brainpoolP384r1;
#endif
#ifdef NID_brainpoolP512r1
      case 30:
        return NID_brainpoolP512r1;
#endif
      default:
        return -1;
    }
}

struct crypto_ec *crypto_ec_init(int group)
{
  struct crypto_ec *e;
  int nid = crypto_ec_group_to_nid(group);

  if (nid < 0)
    {
      return NULL;
    }

  e = os_zalloc(sizeof(*e));
  if (e == NULL)
    {
      return NULL;
    }

  e->bnctx = BN_CTX_new();
  e->nid = nid;
  e->group = EC_GROUP_new_by_curve_name(nid);
  e->prime = BN_new();
  e->order = BN_new();
  e->a = BN_new();
  e->b = BN_new();

  if (e->bnctx == NULL || e->group == NULL || e->prime == NULL ||
      e->order == NULL || e->a == NULL || e->b == NULL ||
      EC_GROUP_get_curve(e->group, e->prime, e->a, e->b, e->bnctx) != 1 ||
      EC_GROUP_get_order(e->group, e->order, e->bnctx) != 1)
    {
      crypto_ec_deinit(e);
      return NULL;
    }

  return e;
}

void crypto_ec_deinit(struct crypto_ec *e)
{
  if (e == NULL)
    {
      return;
    }

  BN_clear_free(e->b);
  BN_clear_free(e->a);
  BN_clear_free(e->order);
  BN_clear_free(e->prime);
  EC_GROUP_free(e->group);
  BN_CTX_free(e->bnctx);
  os_free(e);
}

size_t crypto_ec_prime_len(struct crypto_ec *e)
{
  return BN_num_bytes(e->prime);
}

size_t crypto_ec_prime_len_bits(struct crypto_ec *e)
{
  return BN_num_bits(e->prime);
}

size_t crypto_ec_order_len(struct crypto_ec *e)
{
  return BN_num_bytes(e->order);
}

const struct crypto_bignum *crypto_ec_get_prime(struct crypto_ec *e)
{
  return (const struct crypto_bignum *)e->prime;
}

const struct crypto_bignum *crypto_ec_get_order(struct crypto_ec *e)
{
  return (const struct crypto_bignum *)e->order;
}

const struct crypto_bignum *crypto_ec_get_a(struct crypto_ec *e)
{
  return (const struct crypto_bignum *)e->a;
}

const struct crypto_bignum *crypto_ec_get_b(struct crypto_ec *e)
{
  return (const struct crypto_bignum *)e->b;
}

const struct crypto_ec_point *crypto_ec_get_generator(struct crypto_ec *e)
{
  return (const struct crypto_ec_point *)EC_GROUP_get0_generator(e->group);
}

struct crypto_ec_point *crypto_ec_point_init(struct crypto_ec *e)
{
  return e == NULL ? NULL : (struct crypto_ec_point *)EC_POINT_new(e->group);
}

void crypto_ec_point_deinit(struct crypto_ec_point *p, int clear)
{
  if (clear)
    {
      EC_POINT_clear_free((EC_POINT *)p);
    }
  else
    {
      EC_POINT_free((EC_POINT *)p);
    }
}

int crypto_ec_point_x(struct crypto_ec *e, const struct crypto_ec_point *p,
                      struct crypto_bignum *x)
{
  return EC_POINT_get_affine_coordinates(e->group, (const EC_POINT *)p,
                                         (BIGNUM *)x, NULL,
                                         e->bnctx) == 1 ? 0 : -1;
}

int crypto_ec_point_to_bin(struct crypto_ec *e,
                           const struct crypto_ec_point *point,
                           u8 *x, u8 *y)
{
  int ret = -1;
  int len = BN_num_bytes(e->prime);
  BIGNUM *x_bn = BN_new();
  BIGNUM *y_bn = BN_new();

  if (x_bn == NULL || y_bn == NULL)
    {
      goto out;
    }

  if (EC_POINT_get_affine_coordinates(e->group, (const EC_POINT *)point,
                                      x_bn, y_bn, e->bnctx) != 1)
    {
      goto out;
    }

  ret = 0;
  if (x != NULL &&
      crypto_bignum_to_bin((struct crypto_bignum *)x_bn, x, len, len) < 0)
    {
      ret = -1;
    }

  if (ret == 0 && y != NULL &&
      crypto_bignum_to_bin((struct crypto_bignum *)y_bn, y, len, len) < 0)
    {
      ret = -1;
    }

out:
  BN_clear_free(x_bn);
  BN_clear_free(y_bn);
  return ret;
}

struct crypto_ec_point *crypto_ec_point_from_bin(struct crypto_ec *e,
                                                 const u8 *val)
{
  int len = BN_num_bytes(e->prime);
  EC_POINT *point = EC_POINT_new(e->group);
  BIGNUM *x = BN_bin2bn(val, len, NULL);
  BIGNUM *y = BN_bin2bn(val + len, len, NULL);

  if (point == NULL || x == NULL || y == NULL ||
      EC_POINT_set_affine_coordinates(e->group, point, x, y,
                                      e->bnctx) != 1)
    {
      EC_POINT_clear_free(point);
      point = NULL;
    }

  BN_clear_free(x);
  BN_clear_free(y);
  return (struct crypto_ec_point *)point;
}

int crypto_ec_point_add(struct crypto_ec *e, const struct crypto_ec_point *a,
                        const struct crypto_ec_point *b,
                        struct crypto_ec_point *c)
{
  return EC_POINT_add(e->group, (EC_POINT *)c, (const EC_POINT *)a,
                      (const EC_POINT *)b, e->bnctx) == 1 ? 0 : -1;
}

int crypto_ec_point_mul(struct crypto_ec *e, const struct crypto_ec_point *p,
                        const struct crypto_bignum *b,
                        struct crypto_ec_point *res)
{
  return EC_POINT_mul(e->group, (EC_POINT *)res, NULL, (const EC_POINT *)p,
                      (const BIGNUM *)b, e->bnctx) == 1 ? 0 : -1;
}

int crypto_ec_point_invert(struct crypto_ec *e, struct crypto_ec_point *p)
{
  return EC_POINT_invert(e->group, (EC_POINT *)p, e->bnctx) == 1 ? 0 : -1;
}

struct crypto_bignum *
crypto_ec_point_compute_y_sqr(struct crypto_ec *e,
                              const struct crypto_bignum *x)
{
  BIGNUM *tmp = BN_new();

  if (tmp != NULL &&
      BN_mod_sqr(tmp, (const BIGNUM *)x, e->prime, e->bnctx) == 1 &&
      BN_mod_add(tmp, tmp, e->a, e->prime, e->bnctx) == 1 &&
      BN_mod_mul(tmp, tmp, (const BIGNUM *)x, e->prime, e->bnctx) == 1 &&
      BN_mod_add(tmp, tmp, e->b, e->prime, e->bnctx) == 1)
    {
      return (struct crypto_bignum *)tmp;
    }

  BN_clear_free(tmp);
  return NULL;
}

int crypto_ec_point_is_at_infinity(struct crypto_ec *e,
                                   const struct crypto_ec_point *p)
{
  return EC_POINT_is_at_infinity(e->group, (const EC_POINT *)p);
}

int crypto_ec_point_is_on_curve(struct crypto_ec *e,
                                const struct crypto_ec_point *p)
{
  return EC_POINT_is_on_curve(e->group, (const EC_POINT *)p,
                              e->bnctx) == 1;
}

int crypto_ec_point_cmp(const struct crypto_ec *e,
                        const struct crypto_ec_point *a,
                        const struct crypto_ec_point *b)
{
  return EC_POINT_cmp(e->group, (const EC_POINT *)a, (const EC_POINT *)b,
                      e->bnctx);
}

void crypto_ec_point_debug_print(const struct crypto_ec *e,
                                 const struct crypto_ec_point *p,
                                 const char *title)
{
  wpa_printf(MSG_DEBUG, "%s: %s", title,
             crypto_ec_point_is_at_infinity((struct crypto_ec *)e, p) ?
             "infinity" : "finite");
}

#define NUTTX_WIFI_P384_LEN 48

int nuttx_wifi_p384_make_key(u8 publickey[2 * NUTTX_WIFI_P384_LEN],
                             u8 privatekey[NUTTX_WIFI_P384_LEN]);
int nuttx_wifi_p384_shared_secret(const u8 *publickey, size_t publickey_len,
                                  int inc_y,
                                  const u8 privatekey[NUTTX_WIFI_P384_LEN],
                                  u8 secret[NUTTX_WIFI_P384_LEN]);

struct crypto_ecdh
{
  struct crypto_ec *ec;
  BIGNUM *priv;
  EC_POINT *pub;
  int use_p384;
  u8 p384_priv[NUTTX_WIFI_P384_LEN];
  u8 p384_pub[2 * NUTTX_WIFI_P384_LEN];
};

struct crypto_ecdh *crypto_ecdh_init(int group)
{
  struct crypto_ecdh *ecdh;
  u8 *priv_buf = NULL;
  size_t priv_len = 0;
  int i;

  ecdh = os_zalloc(sizeof(*ecdh));
  if (ecdh == NULL)
    {
      return NULL;
    }

  ecdh->ec = crypto_ec_init(group);
  if (ecdh->ec == NULL)
    {
      goto fail;
    }

  if (group == 20)
    {
      ecdh->use_p384 = 1;
      if (nuttx_wifi_p384_make_key(ecdh->p384_pub, ecdh->p384_priv) < 0)
        {
          goto fail;
        }

      return ecdh;
    }

  ecdh->pub = EC_POINT_new(ecdh->ec->group);
  ecdh->priv = BN_new();
  if (ecdh->pub == NULL || ecdh->priv == NULL)
    {
      goto fail;
    }

  priv_len = BN_num_bytes(ecdh->ec->order);
  priv_buf = os_zalloc(priv_len);
  if (priv_buf == NULL)
    {
      goto fail;
    }

  for (i = 0; i < 100; i++)
    {
      memset(priv_buf, 0, priv_len);
      nuttx_wpa_mix_sim_entropy(priv_buf, priv_len);
      if (BN_bin2bn(priv_buf, priv_len, ecdh->priv) == NULL ||
          BN_mod(ecdh->priv, ecdh->priv, ecdh->ec->order,
                 ecdh->ec->bnctx) != 1)
        {
          goto fail;
        }

      if (!BN_is_zero(ecdh->priv))
        {
          break;
        }
    }

  if (BN_is_zero(ecdh->priv) ||
      EC_POINT_mul(ecdh->ec->group, ecdh->pub, ecdh->priv, NULL, NULL,
                   ecdh->ec->bnctx) != 1)
    {
      goto fail;
    }

  bin_clear_free(priv_buf, priv_len);
  return ecdh;

fail:
  bin_clear_free(priv_buf, priv_len);
  crypto_ecdh_deinit(ecdh);
  return NULL;
}

struct crypto_ecdh *crypto_ecdh_init2(int group,
                                      struct crypto_ec_key *own_key)
{
  struct crypto_ecdh *ecdh;
  struct nuttx_wpa_ec_key *key = (struct nuttx_wpa_ec_key *)own_key;

  if (key == NULL || key->priv == NULL || key->pub == NULL)
    {
      return NULL;
    }

  ecdh = os_zalloc(sizeof(*ecdh));
  if (ecdh == NULL)
    {
      return NULL;
    }

  ecdh->ec = crypto_ec_init(group);
  if (ecdh->ec == NULL)
    {
      goto fail;
    }

  if (key->group != group)
    {
      goto fail;
    }

  ecdh->priv = BN_dup(key->priv);
  ecdh->pub = EC_POINT_dup(key->pub, ecdh->ec->group);
  if (ecdh->priv == NULL || ecdh->pub == NULL)
    {
      goto fail;
    }

  return ecdh;

fail:
  crypto_ecdh_deinit(ecdh);
  return NULL;
}

struct wpabuf *crypto_ecdh_get_pubkey(struct crypto_ecdh *ecdh, int inc_y)
{
  struct wpabuf *buf = NULL;
  BIGNUM *x = NULL;
  BIGNUM *y = NULL;
  int len;

  if (ecdh == NULL || ecdh->ec == NULL || ecdh->pub == NULL)
    {
      if (ecdh == NULL || !ecdh->use_p384)
        {
          return NULL;
        }
    }

  if (ecdh->use_p384)
    {
      return wpabuf_alloc_copy(ecdh->p384_pub,
                               inc_y ? 2 * NUTTX_WIFI_P384_LEN :
                               NUTTX_WIFI_P384_LEN);
    }

  len = BN_num_bytes(ecdh->ec->prime);
  x = BN_new();
  if (inc_y)
    {
      y = BN_new();
    }

  buf = wpabuf_alloc(inc_y ? 2 * len : len);
  if (x == NULL || (inc_y && y == NULL) || buf == NULL)
    {
      goto fail;
    }

  if (EC_POINT_get_affine_coordinates(ecdh->ec->group, ecdh->pub, x, y,
                                      ecdh->ec->bnctx) != 1)
    {
      goto fail;
    }

  if (crypto_bignum_to_bin((struct crypto_bignum *)x,
                           wpabuf_put(buf, len), len, len) < 0)
    {
      goto fail;
    }

  if (inc_y &&
      crypto_bignum_to_bin((struct crypto_bignum *)y,
                           wpabuf_put(buf, len), len, len) < 0)
    {
      goto fail;
    }

  BN_clear_free(x);
  BN_clear_free(y);
  return buf;

fail:
  wpabuf_free(buf);
  BN_clear_free(x);
  BN_clear_free(y);
  return NULL;
}

struct wpabuf *crypto_ecdh_set_peerkey(struct crypto_ecdh *ecdh, int inc_y,
                                       const u8 *key, size_t len)
{
  BIGNUM *x = NULL;
  BIGNUM *y = NULL;
  BIGNUM *secret_bn = NULL;
  BIGNUM *exp = NULL;
  struct wpabuf *secret = NULL;
  size_t secret_len;
  EC_POINT *pub = NULL;
  EC_POINT *secret_point = NULL;
  u8 *pos;

  if (ecdh == NULL || ecdh->ec == NULL ||
      key == NULL || len == 0)
    {
      return NULL;
    }

  if (ecdh->use_p384)
    {
      u8 secret_buf[NUTTX_WIFI_P384_LEN];

      if (nuttx_wifi_p384_shared_secret(key, len, inc_y, ecdh->p384_priv,
                                        secret_buf) < 0)
        {
          return NULL;
        }

      secret = wpabuf_alloc_copy(secret_buf, sizeof(secret_buf));
      forced_memzero(secret_buf, sizeof(secret_buf));
      return secret;
    }

  if (ecdh->priv == NULL)
    {
      return NULL;
    }

  x = BN_bin2bn(key, inc_y ? len / 2 : len, NULL);
  pub = EC_POINT_new(ecdh->ec->group);
  if (x == NULL || pub == NULL)
    {
      goto fail;
    }

  if (inc_y)
    {
      y = BN_bin2bn(key + len / 2, len / 2, NULL);
      if (y == NULL ||
          EC_POINT_set_affine_coordinates(ecdh->ec->group, pub, x, y,
                                          ecdh->ec->bnctx) != 1)
        {
          goto fail;
        }
    }
  else
    {
      y = (BIGNUM *)crypto_ec_point_compute_y_sqr(ecdh->ec,
                                                  (struct crypto_bignum *)x);
      exp = BN_dup(ecdh->ec->prime);
      if (y == NULL || exp == NULL ||
          BN_add_word(exp, 1) != 1 ||
          BN_rshift(exp, exp, 2) != 1 ||
          BN_mod_exp(y, y, exp, ecdh->ec->prime, ecdh->ec->bnctx) != 1 ||
          EC_POINT_set_affine_coordinates(ecdh->ec->group, pub, x, y,
                                          ecdh->ec->bnctx) != 1)
        {
          goto fail;
        }
    }

  if (EC_POINT_is_on_curve(ecdh->ec->group, pub, ecdh->ec->bnctx) != 1)
    {
      goto fail;
    }

  secret_point = EC_POINT_new(ecdh->ec->group);
  secret_bn = BN_new();
  if (secret_point == NULL || secret_bn == NULL ||
      EC_POINT_mul(ecdh->ec->group, secret_point, NULL, pub, ecdh->priv,
                   ecdh->ec->bnctx) != 1 ||
      EC_POINT_is_at_infinity(ecdh->ec->group, secret_point))
    {
      goto fail;
    }

  if (EC_POINT_get_affine_coordinates(ecdh->ec->group, secret_point,
                                      secret_bn, NULL,
                                      ecdh->ec->bnctx) != 1)
    {
      goto fail;
    }

  secret_len = BN_num_bytes(ecdh->ec->prime);
  secret = wpabuf_alloc(secret_len);
  if (secret == NULL)
    {
      goto fail;
    }

  pos = wpabuf_put(secret, secret_len);
  if (crypto_bignum_to_bin((struct crypto_bignum *)secret_bn, pos,
                           secret_len, secret_len) < 0)
    {
      goto fail;
    }

  BN_clear_free(x);
  BN_clear_free(y);
  BN_clear_free(secret_bn);
  BN_clear_free(exp);
  EC_POINT_free(pub);
  EC_POINT_clear_free(secret_point);
  return secret;

fail:
  wpabuf_free(secret);
  BN_clear_free(x);
  BN_clear_free(y);
  BN_clear_free(secret_bn);
  BN_clear_free(exp);
  EC_POINT_free(pub);
  EC_POINT_clear_free(secret_point);
  return NULL;
}

void crypto_ecdh_deinit(struct crypto_ecdh *ecdh)
{
  if (ecdh == NULL)
    {
      return;
    }

  crypto_ec_deinit(ecdh->ec);
  BN_clear_free(ecdh->priv);
  EC_POINT_clear_free(ecdh->pub);
  forced_memzero(ecdh->p384_priv, sizeof(ecdh->p384_priv));
  os_free(ecdh);
}

size_t crypto_ecdh_prime_len(struct crypto_ecdh *ecdh)
{
  return ecdh == NULL ? 0 : crypto_ec_prime_len(ecdh->ec);
}

static int nuttx_wpa_asn1_get_tlv(const u8 **pos, const u8 *end, u8 *tag,
                                  const u8 **value, size_t *len)
{
  const u8 *p = *pos;
  size_t l;
  u8 len_byte;
  int i;
  int len_len;

  if (p >= end)
    {
      return -1;
    }

  *tag = *p++;
  if (p >= end)
    {
      return -1;
    }

  len_byte = *p++;
  if ((len_byte & 0x80) == 0)
    {
      l = len_byte;
    }
  else
    {
      len_len = len_byte & 0x7f;
      if (len_len == 0 || len_len > (int)sizeof(size_t) ||
          (size_t)(end - p) < (size_t)len_len)
        {
          return -1;
        }

      l = 0;
      for (i = 0; i < len_len; i++)
        {
          l = (l << 8) | *p++;
        }
    }

  if ((size_t)(end - p) < l)
    {
      return -1;
    }

  *value = p;
  *len = l;
  *pos = p + l;
  return 0;
}

static int nuttx_wpa_ec_curve_nid_from_oid(const u8 *oid, size_t oid_len)
{
  static const u8 prime256v1_oid[] =
  {
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07
  };
  static const u8 secp384r1_oid[] =
  {
    0x2b, 0x81, 0x04, 0x00, 0x22
  };
  static const u8 secp521r1_oid[] =
  {
    0x2b, 0x81, 0x04, 0x00, 0x23
  };

  if (oid_len == sizeof(prime256v1_oid) &&
      os_memcmp(oid, prime256v1_oid, oid_len) == 0)
    {
      return NID_X9_62_prime256v1;
    }

  if (oid_len == sizeof(secp384r1_oid) &&
      os_memcmp(oid, secp384r1_oid, oid_len) == 0)
    {
      return NID_secp384r1;
    }

  if (oid_len == sizeof(secp521r1_oid) &&
      os_memcmp(oid, secp521r1_oid, oid_len) == 0)
    {
      return NID_secp521r1;
    }

  return 0;
}

static int nuttx_wpa_ec_nid_to_group(int nid)
{
  switch (nid)
    {
      case NID_X9_62_prime256v1:
        return 19;
      case NID_secp384r1:
        return 20;
      case NID_secp521r1:
        return 21;
#ifdef NID_brainpoolP224r1
      case NID_brainpoolP224r1:
        return 27;
#endif
#ifdef NID_brainpoolP256r1
      case NID_brainpoolP256r1:
        return 28;
#endif
#ifdef NID_brainpoolP384r1
      case NID_brainpoolP384r1:
        return 29;
#endif
#ifdef NID_brainpoolP512r1
      case NID_brainpoolP512r1:
        return 30;
#endif
      default:
        return -1;
    }
}

static struct nuttx_wpa_ec_key *nuttx_wpa_ec_key_alloc(int group)
{
  struct nuttx_wpa_ec_key *key;
  int nid = crypto_ec_group_to_nid(group);

  if (nid < 0)
    {
      return NULL;
    }

  key = os_zalloc(sizeof(*key));
  if (key == NULL)
    {
      return NULL;
    }

  key->group = group;
  key->nid = nid;
  key->ec = crypto_ec_init(group);
  if (key->ec == NULL)
    {
      os_free(key);
      return NULL;
    }

  return key;
}

static int nuttx_wpa_ec_key_set_pub_octets(struct nuttx_wpa_ec_key *key,
                                           const u8 *pub, size_t pub_len)
{
  size_t prime_len;
  BIGNUM *x = NULL;
  BIGNUM *y = NULL;
  BIGNUM *exp = NULL;
  int ret = -1;

  if (key == NULL || pub == NULL || pub_len == 0)
    {
      return -1;
    }

  EC_POINT_clear_free(key->pub);
  key->pub = EC_POINT_new(key->ec->group);
  if (key->pub == NULL)
    {
      return -1;
    }

  prime_len = crypto_ec_prime_len(key->ec);
  if (pub[0] == 0x04 && pub_len == 1 + 2 * prime_len)
    {
      x = BN_bin2bn(pub + 1, prime_len, NULL);
      y = BN_bin2bn(pub + 1 + prime_len, prime_len, NULL);
      if (x == NULL || y == NULL ||
          EC_POINT_set_affine_coordinates(key->ec->group, key->pub, x, y,
                                          key->ec->bnctx) != 1)
        {
          goto out;
        }
    }
  else if ((pub[0] == 0x02 || pub[0] == 0x03) &&
           pub_len == 1 + prime_len)
    {
      x = BN_bin2bn(pub + 1, prime_len, NULL);
      y = (BIGNUM *)crypto_ec_point_compute_y_sqr(key->ec,
                                                  (struct crypto_bignum *)x);
      exp = BN_dup(key->ec->prime);
      if (x == NULL || y == NULL || exp == NULL ||
          BN_add_word(exp, 1) != 1 ||
          BN_rshift(exp, exp, 2) != 1 ||
          BN_mod_exp(y, y, exp, key->ec->prime, key->ec->bnctx) != 1)
        {
          goto out;
        }

      if (BN_is_odd(y) != (pub[0] & 1) &&
          BN_sub(y, key->ec->prime, y) != 1)
        {
          goto out;
        }

      if (EC_POINT_set_affine_coordinates(key->ec->group, key->pub, x, y,
                                          key->ec->bnctx) != 1)
        {
          goto out;
        }
    }
  else
    {
      goto out;
    }

  if (EC_POINT_is_on_curve(key->ec->group, key->pub,
                           key->ec->bnctx) != 1)
    {
      goto out;
    }

  ret = 0;

out:
  BN_clear_free(x);
  BN_clear_free(y);
  BN_clear_free(exp);
  return ret;
}

static size_t nuttx_wpa_asn1_len_size(size_t len)
{
  if (len < 128)
    {
      return 1;
    }

  if (len <= 0xff)
    {
      return 2;
    }

  return 3;
}

static void nuttx_wpa_asn1_put_len(struct wpabuf *buf, size_t len)
{
  if (len < 128)
    {
      wpabuf_put_u8(buf, len);
    }
  else if (len <= 0xff)
    {
      wpabuf_put_u8(buf, 0x81);
      wpabuf_put_u8(buf, len);
    }
  else
    {
      wpabuf_put_u8(buf, 0x82);
      wpabuf_put_u8(buf, len >> 8);
      wpabuf_put_u8(buf, len);
    }
}

static size_t nuttx_wpa_asn1_int_len(const BIGNUM *bn)
{
  u8 tmp[72];
  int len;

  if (BN_is_zero(bn))
    {
      return 1;
    }

  len = BN_num_bytes(bn);
  if (len <= 0 || (size_t)len > sizeof(tmp))
    {
      return 0;
    }

  BN_bn2bin(bn, tmp);
  return len + ((tmp[0] & 0x80) ? 1 : 0);
}

static void nuttx_wpa_asn1_put_int(struct wpabuf *buf, const BIGNUM *bn)
{
  u8 tmp[72];
  size_t enc_len;
  int len;

  wpabuf_put_u8(buf, 0x02);
  enc_len = nuttx_wpa_asn1_int_len(bn);
  nuttx_wpa_asn1_put_len(buf, enc_len);

  if (BN_is_zero(bn))
    {
      wpabuf_put_u8(buf, 0);
      return;
    }

  len = BN_num_bytes(bn);
  BN_bn2bin(bn, tmp);
  if (tmp[0] & 0x80)
    {
      wpabuf_put_u8(buf, 0);
    }

  wpabuf_put_data(buf, tmp, len);
}

static BIGNUM *nuttx_wpa_ecdsa_hash_to_bn(const u8 *data, size_t len,
                                          const BIGNUM *order)
{
  BIGNUM *z;
  int order_bits;
  int data_bits;

  z = BN_bin2bn(data, len, NULL);
  if (z == NULL)
    {
      return NULL;
    }

  order_bits = BN_num_bits(order);
  data_bits = 8 * len;
  if (data_bits > order_bits &&
      BN_rshift(z, z, data_bits - order_bits) != 1)
    {
      BN_clear_free(z);
      return NULL;
    }

  return z;
}

static int nuttx_wpa_asn1_get_int_bn(const u8 **pos, const u8 *end,
                                     BIGNUM **bn)
{
  const u8 *value;
  size_t len;
  u8 tag;

  if (nuttx_wpa_asn1_get_tlv(pos, end, &tag, &value, &len) < 0 ||
      tag != 0x02 || len == 0)
    {
      return -1;
    }

  while (len > 1 && value[0] == 0)
    {
      value++;
      len--;
    }

  *bn = BN_bin2bn(value, len, NULL);
  return *bn == NULL ? -1 : 0;
}

struct crypto_ec_key *crypto_ec_key_parse_priv(const u8 *der, size_t der_len)
{
  struct nuttx_wpa_ec_key *key = NULL;
  const u8 *pos = der;
  const u8 *end = der + der_len;
  const u8 *seq;
  const u8 *seq_end;
  const u8 *value;
  const u8 *priv = NULL;
  const u8 *pub = NULL;
  size_t len;
  size_t priv_len = 0;
  size_t pub_len = 0;
  u8 tag;
  int nid = 0;
  int group;

  if (nuttx_wpa_asn1_get_tlv(&pos, end, &tag, &seq, &len) < 0 ||
      tag != 0x30 || pos != end)
    {
      wpa_printf(MSG_INFO, "OpenSSL: Invalid ECPrivateKey DER sequence");
      return NULL;
    }

  pos = seq;
  seq_end = seq + len;
  if (nuttx_wpa_asn1_get_tlv(&pos, seq_end, &tag, &value, &len) < 0 ||
      tag != 0x02)
    {
      wpa_printf(MSG_INFO, "OpenSSL: Invalid ECPrivateKey version");
      return NULL;
    }

  if (nuttx_wpa_asn1_get_tlv(&pos, seq_end, &tag, &priv, &priv_len) < 0 ||
      tag != 0x04)
    {
      wpa_printf(MSG_INFO, "OpenSSL: Missing ECPrivateKey private key");
      return NULL;
    }

  while (pos < seq_end)
    {
      const u8 *inner;
      const u8 *inner_end;
      size_t inner_len;
      u8 inner_tag;

      if (nuttx_wpa_asn1_get_tlv(&pos, seq_end, &tag, &value, &len) < 0)
        {
          return NULL;
        }

      inner = value;
      inner_end = value + len;
      if (tag == 0xa0 &&
          nuttx_wpa_asn1_get_tlv(&inner, inner_end, &inner_tag,
                                 &value, &inner_len) == 0 &&
          inner_tag == 0x06)
        {
          nid = nuttx_wpa_ec_curve_nid_from_oid(value, inner_len);
        }
      else if (tag == 0xa1 &&
               nuttx_wpa_asn1_get_tlv(&inner, inner_end, &inner_tag,
                                      &value, &inner_len) == 0 &&
               inner_tag == 0x03 && inner_len >= 2 && value[0] == 0)
        {
          pub = value + 1;
          pub_len = inner_len - 1;
        }
    }

  if (nid == 0)
    {
      if (priv_len == 32)
        {
          nid = NID_X9_62_prime256v1;
        }
      else if (priv_len == 48)
        {
          nid = NID_secp384r1;
        }
      else if (priv_len == 66)
        {
          nid = NID_secp521r1;
        }
    }

  group = nuttx_wpa_ec_nid_to_group(nid);
  if (group < 0)
    {
      wpa_printf(MSG_INFO, "OpenSSL: Unsupported ECPrivateKey curve");
      return NULL;
    }

  key = nuttx_wpa_ec_key_alloc(group);
  if (key == NULL)
    {
      return NULL;
    }

  key->priv = BN_bin2bn(priv, priv_len, NULL);
  key->pub = EC_POINT_new(key->ec->group);
  if (key->priv == NULL || key->pub == NULL)
    {
      goto fail;
    }

  if (pub != NULL)
    {
      if (nuttx_wpa_ec_key_set_pub_octets(key, pub, pub_len) < 0)
        {
          goto fail;
        }
    }
  else if (EC_POINT_mul(key->ec->group, key->pub, key->priv, NULL, NULL,
                        key->ec->bnctx) != 1)
    {
      goto fail;
    }

  return (struct crypto_ec_key *)key;

fail:
  crypto_ec_key_deinit((struct crypto_ec_key *)key);
  return NULL;
}

struct crypto_ec_key *crypto_ec_key_parse_pub(const u8 *der, size_t der_len)
{
  struct nuttx_wpa_ec_key *key = NULL;
  const u8 *pos = der;
  const u8 *end = der + der_len;
  const u8 *seq;
  const u8 *seq_end;
  const u8 *alg;
  const u8 *alg_end;
  const u8 *value;
  const u8 *pub;
  size_t len;
  size_t pub_len;
  u8 tag;
  int nid = 0;
  int group;

  if (nuttx_wpa_asn1_get_tlv(&pos, end, &tag, &seq, &len) < 0 ||
      tag != 0x30 || pos != end)
    {
      return NULL;
    }

  pos = seq;
  seq_end = seq + len;
  if (nuttx_wpa_asn1_get_tlv(&pos, seq_end, &tag, &alg, &len) < 0 ||
      tag != 0x30)
    {
      return NULL;
    }

  alg_end = alg + len;
  if (nuttx_wpa_asn1_get_tlv(&alg, alg_end, &tag, &value, &len) < 0 ||
      tag != 0x06)
    {
      return NULL;
    }

  if (nuttx_wpa_asn1_get_tlv(&alg, alg_end, &tag, &value, &len) < 0 ||
      tag != 0x06)
    {
      return NULL;
    }

  nid = nuttx_wpa_ec_curve_nid_from_oid(value, len);
  group = nuttx_wpa_ec_nid_to_group(nid);
  if (group < 0)
    {
      return NULL;
    }

  if (nuttx_wpa_asn1_get_tlv(&pos, seq_end, &tag, &value, &len) < 0 ||
      tag != 0x03 || len < 2 || value[0] != 0 || pos != seq_end)
    {
      return NULL;
    }

  pub = value + 1;
  pub_len = len - 1;
  key = nuttx_wpa_ec_key_alloc(group);
  if (key == NULL ||
      nuttx_wpa_ec_key_set_pub_octets(key, pub, pub_len) < 0)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      return NULL;
    }

  return (struct crypto_ec_key *)key;
}

struct crypto_ec_key *crypto_ec_key_set_priv(int group,
                                             const u8 *raw, size_t raw_len)
{
  struct nuttx_wpa_ec_key *key;
  size_t prime_len;

  key = nuttx_wpa_ec_key_alloc(group);
  if (key == NULL || raw == NULL)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      return NULL;
    }

  prime_len = crypto_ec_prime_len(key->ec);
  if (raw_len > prime_len)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      return NULL;
    }

  key->priv = BN_bin2bn(raw, raw_len, NULL);
  key->pub = EC_POINT_new(key->ec->group);
  if (key->priv == NULL || key->pub == NULL ||
      BN_is_zero(key->priv) || BN_is_negative(key->priv) ||
      BN_cmp(key->priv, key->ec->order) >= 0 ||
      EC_POINT_mul(key->ec->group, key->pub, key->priv, NULL, NULL,
                   key->ec->bnctx) != 1)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      return NULL;
    }

  return (struct crypto_ec_key *)key;
}

struct crypto_ec_key *crypto_ec_key_set_pub(int group, const u8 *x,
                                            const u8 *y, size_t len)
{
  struct nuttx_wpa_ec_key *key;
  u8 *point;

  if (x == NULL || y == NULL)
    {
      return NULL;
    }

  key = nuttx_wpa_ec_key_alloc(group);
  point = os_malloc(1 + 2 * len);
  if (key == NULL || point == NULL)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      os_free(point);
      return NULL;
    }

  point[0] = 0x04;
  os_memcpy(point + 1, x, len);
  os_memcpy(point + 1 + len, y, len);
  if (nuttx_wpa_ec_key_set_pub_octets(key, point, 1 + 2 * len) < 0)
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      key = NULL;
    }

  bin_clear_free(point, 1 + 2 * len);
  return (struct crypto_ec_key *)key;
}

struct crypto_ec_key *
crypto_ec_key_set_pub_point(struct crypto_ec *e,
                            const struct crypto_ec_point *pub)
{
  struct nuttx_wpa_ec_key *key;
  int group;

  if (e == NULL || pub == NULL)
    {
      return NULL;
    }

  group = nuttx_wpa_ec_nid_to_group(e->nid);
  key = nuttx_wpa_ec_key_alloc(group);
  if (key == NULL)
    {
      return NULL;
    }

  key->pub = EC_POINT_dup((const EC_POINT *)pub, e->group);
  if (key->pub == NULL ||
      !EC_POINT_is_on_curve(key->ec->group, key->pub, key->ec->bnctx) ||
      EC_POINT_is_at_infinity(key->ec->group, key->pub))
    {
      crypto_ec_key_deinit((struct crypto_ec_key *)key);
      return NULL;
    }

  return (struct crypto_ec_key *)key;
}

struct crypto_ec_key *crypto_ec_key_gen(int group)
{
  struct nuttx_wpa_ec_key *key;
  int i;

  key = nuttx_wpa_ec_key_alloc(group);
  if (key == NULL)
    {
      return NULL;
    }

  key->priv = BN_new();
  key->pub = EC_POINT_new(key->ec->group);
  if (key->priv == NULL || key->pub == NULL)
    {
      goto fail;
    }

  for (i = 0; i < 100; i++)
    {
      if (crypto_bignum_rand((struct crypto_bignum *)key->priv,
                             (struct crypto_bignum *)key->ec->order) == 0 &&
          !BN_is_zero(key->priv) &&
          EC_POINT_mul(key->ec->group, key->pub, key->priv, NULL, NULL,
                       key->ec->bnctx) == 1)
        {
          return (struct crypto_ec_key *)key;
        }
    }

fail:
  crypto_ec_key_deinit((struct crypto_ec_key *)key);
  return NULL;
}

void crypto_ec_key_deinit(struct crypto_ec_key *key)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;

  if (ec_key == NULL)
    {
      return;
    }

  EC_POINT_clear_free(ec_key->pub);
  BN_clear_free(ec_key->priv);
  crypto_ec_deinit(ec_key->ec);
  os_free(ec_key);
}

struct wpabuf *crypto_ec_key_get_subject_public_key(struct crypto_ec_key *key)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  struct wpabuf *buf = NULL;
  u8 point[1 + 66];
  const u8 *curve_oid;
  size_t curve_oid_len;
  size_t point_len;
  size_t alg_len;
  size_t bitstr_len;
  size_t seq_len;

  static const u8 oid_id_ec_public_key[] =
  {
    0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01
  };

  static const u8 oid_prime256v1[] =
  {
    0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07
  };

  static const u8 oid_secp384r1[] =
  {
    0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22
  };

  static const u8 oid_secp521r1[] =
  {
    0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x23
  };

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->pub == NULL)
    {
      return NULL;
    }

  switch (ec_key->nid)
    {
      case NID_X9_62_prime256v1:
        curve_oid = oid_prime256v1;
        curve_oid_len = sizeof(oid_prime256v1);
        break;
      case NID_secp384r1:
        curve_oid = oid_secp384r1;
        curve_oid_len = sizeof(oid_secp384r1);
        break;
      case NID_secp521r1:
        curve_oid = oid_secp521r1;
        curve_oid_len = sizeof(oid_secp521r1);
        break;
      default:
        return NULL;
    }

  point_len = EC_POINT_point2oct(ec_key->ec->group, ec_key->pub,
                                POINT_CONVERSION_COMPRESSED, point,
                                sizeof(point), ec_key->ec->bnctx);
  if (point_len == 0)
    {
      return NULL;
    }

  alg_len = sizeof(oid_id_ec_public_key) + curve_oid_len;
  bitstr_len = 1 + point_len;
  seq_len = 1 + nuttx_wpa_asn1_len_size(alg_len) + alg_len +
            1 + nuttx_wpa_asn1_len_size(bitstr_len) + bitstr_len;
  buf = wpabuf_alloc(1 + nuttx_wpa_asn1_len_size(seq_len) + seq_len);
  if (buf == NULL)
    {
      return NULL;
    }

  wpabuf_put_u8(buf, 0x30);
  nuttx_wpa_asn1_put_len(buf, seq_len);
  wpabuf_put_u8(buf, 0x30);
  nuttx_wpa_asn1_put_len(buf, alg_len);
  wpabuf_put_data(buf, oid_id_ec_public_key, sizeof(oid_id_ec_public_key));
  wpabuf_put_data(buf, curve_oid, curve_oid_len);
  wpabuf_put_u8(buf, 0x03);
  nuttx_wpa_asn1_put_len(buf, bitstr_len);
  wpabuf_put_u8(buf, 0);
  wpabuf_put_data(buf, point, point_len);

  return buf;
}

struct wpabuf *crypto_ec_key_get_ecprivate_key(struct crypto_ec_key *key,
                                               bool include_pub)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  struct wpabuf *buf;
  u8 priv[66];
  u8 pub[1 + 2 * 66];
  const u8 *curve_oid;
  size_t curve_oid_len;
  size_t prime_len;
  size_t priv_len;
  size_t param_len;
  size_t pub_len = 0;
  size_t bitstr_len = 0;
  size_t seq_len;

  static const u8 oid_prime256v1[] =
  {
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07
  };

  static const u8 oid_secp384r1[] =
  {
    0x2b, 0x81, 0x04, 0x00, 0x22
  };

  static const u8 oid_secp521r1[] =
  {
    0x2b, 0x81, 0x04, 0x00, 0x23
  };

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->priv == NULL)
    {
      return NULL;
    }

  switch (ec_key->nid)
    {
      case NID_X9_62_prime256v1:
        curve_oid = oid_prime256v1;
        curve_oid_len = sizeof(oid_prime256v1);
        break;
      case NID_secp384r1:
        curve_oid = oid_secp384r1;
        curve_oid_len = sizeof(oid_secp384r1);
        break;
      case NID_secp521r1:
        curve_oid = oid_secp521r1;
        curve_oid_len = sizeof(oid_secp521r1);
        break;
      default:
        return NULL;
    }

  prime_len = crypto_ec_prime_len(ec_key->ec);
  if (prime_len > sizeof(priv) ||
      crypto_bignum_to_bin((const struct crypto_bignum *)ec_key->priv,
                           priv, sizeof(priv), prime_len) < 0)
    {
      return NULL;
    }

  if (include_pub && ec_key->pub != NULL)
    {
      pub_len = EC_POINT_point2oct(ec_key->ec->group, ec_key->pub,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   pub, sizeof(pub), ec_key->ec->bnctx);
      if (pub_len == 0)
        {
          forced_memzero(priv, sizeof(priv));
          return NULL;
        }

      bitstr_len = 1 + pub_len;
    }

  priv_len = prime_len;
  param_len = 1 + nuttx_wpa_asn1_len_size(curve_oid_len) + curve_oid_len;
  seq_len = 3 + 1 + nuttx_wpa_asn1_len_size(priv_len) + priv_len +
            1 + nuttx_wpa_asn1_len_size(param_len) + param_len;
  if (include_pub)
    {
      size_t pub_field_len = 1 + nuttx_wpa_asn1_len_size(bitstr_len) +
                             bitstr_len;

      seq_len += 1 + nuttx_wpa_asn1_len_size(pub_field_len) + pub_field_len;
    }

  buf = wpabuf_alloc(1 + nuttx_wpa_asn1_len_size(seq_len) + seq_len);
  if (buf == NULL)
    {
      forced_memzero(priv, sizeof(priv));
      return NULL;
    }

  wpabuf_put_u8(buf, 0x30);
  nuttx_wpa_asn1_put_len(buf, seq_len);
  wpabuf_put_u8(buf, 0x02);
  wpabuf_put_u8(buf, 0x01);
  wpabuf_put_u8(buf, 0x01);
  wpabuf_put_u8(buf, 0x04);
  nuttx_wpa_asn1_put_len(buf, priv_len);
  wpabuf_put_data(buf, priv, priv_len);
  wpabuf_put_u8(buf, 0xa0);
  nuttx_wpa_asn1_put_len(buf, param_len);
  wpabuf_put_u8(buf, 0x06);
  nuttx_wpa_asn1_put_len(buf, curve_oid_len);
  wpabuf_put_data(buf, curve_oid, curve_oid_len);
  if (include_pub)
    {
      size_t pub_field_len = 1 + nuttx_wpa_asn1_len_size(bitstr_len) +
                             bitstr_len;

      wpabuf_put_u8(buf, 0xa1);
      nuttx_wpa_asn1_put_len(buf, pub_field_len);
      wpabuf_put_u8(buf, 0x03);
      nuttx_wpa_asn1_put_len(buf, bitstr_len);
      wpabuf_put_u8(buf, 0);
      wpabuf_put_data(buf, pub, pub_len);
    }

  forced_memzero(priv, sizeof(priv));
  forced_memzero(pub, sizeof(pub));
  return buf;
}

struct wpabuf *crypto_ec_key_get_pubkey_point(struct crypto_ec_key *key,
                                              int prefix)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  struct wpabuf *buf;
  size_t prime_len;
  size_t point_len;

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->pub == NULL)
    {
      return NULL;
    }

  prime_len = crypto_ec_prime_len(ec_key->ec);
  point_len = 1 + 2 * prime_len;
  buf = wpabuf_alloc(point_len);
  if (buf == NULL)
    {
      return NULL;
    }

  if (EC_POINT_point2oct(ec_key->ec->group, ec_key->pub,
                         POINT_CONVERSION_UNCOMPRESSED,
                         wpabuf_put(buf, point_len), point_len,
                         ec_key->ec->bnctx) != point_len)
    {
      wpabuf_free(buf);
      return NULL;
    }

  if (!prefix)
    {
      u8 *pos = wpabuf_mhead(buf);

      os_memmove(pos, pos + 1, point_len - 1);
      buf->used--;
    }

  return buf;
}

struct crypto_ec_point *
crypto_ec_key_get_public_key(struct crypto_ec_key *key)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->pub == NULL)
    {
      return NULL;
    }

  return (struct crypto_ec_point *)EC_POINT_dup(ec_key->pub,
                                                ec_key->ec->group);
}

struct crypto_bignum *
crypto_ec_key_get_private_key(struct crypto_ec_key *key)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;

  if (ec_key == NULL || ec_key->priv == NULL)
    {
      return NULL;
    }

  return (struct crypto_bignum *)BN_dup(ec_key->priv);
}

struct wpabuf *crypto_ec_key_sign(struct crypto_ec_key *key, const u8 *data,
                                  size_t len)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  BIGNUM *z = NULL;
  BIGNUM *k = NULL;
  BIGNUM *kinv = NULL;
  BIGNUM *x = NULL;
  BIGNUM *r = NULL;
  BIGNUM *s = NULL;
  BIGNUM *tmp = NULL;
  EC_POINT *point = NULL;
  struct wpabuf *sig = NULL;
  size_t r_len;
  size_t s_len;
  size_t seq_len;
  int i;

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->priv == NULL)
    {
      return NULL;
    }

  z = nuttx_wpa_ecdsa_hash_to_bn(data, len, ec_key->ec->order);
  k = BN_new();
  x = BN_new();
  r = BN_new();
  s = BN_new();
  tmp = BN_new();
  point = EC_POINT_new(ec_key->ec->group);
  if (z == NULL || k == NULL || x == NULL || r == NULL || s == NULL ||
      tmp == NULL || point == NULL)
    {
      goto out;
    }

  for (i = 0; i < 100; i++)
    {
      BN_clear_free(kinv);
      kinv = NULL;

      if (crypto_bignum_rand((struct crypto_bignum *)k,
                             (struct crypto_bignum *)ec_key->ec->order) < 0 ||
          BN_is_zero(k) ||
          EC_POINT_mul(ec_key->ec->group, point, k, NULL, NULL,
                       ec_key->ec->bnctx) != 1 ||
          EC_POINT_get_affine_coordinates(ec_key->ec->group, point, x, NULL,
                                          ec_key->ec->bnctx) != 1 ||
          BN_mod(r, x, ec_key->ec->order, ec_key->ec->bnctx) != 1 ||
          BN_is_zero(r))
        {
          continue;
        }

      kinv = BN_mod_inverse(NULL, k, ec_key->ec->order, ec_key->ec->bnctx);
      if (kinv == NULL ||
          BN_mod_mul(tmp, r, ec_key->priv, ec_key->ec->order,
                     ec_key->ec->bnctx) != 1 ||
          BN_mod_add(tmp, tmp, z, ec_key->ec->order,
                     ec_key->ec->bnctx) != 1 ||
          BN_mod_mul(s, kinv, tmp, ec_key->ec->order,
                     ec_key->ec->bnctx) != 1 ||
          BN_is_zero(s))
        {
          continue;
        }

      r_len = nuttx_wpa_asn1_int_len(r);
      s_len = nuttx_wpa_asn1_int_len(s);
      if (r_len == 0 || s_len == 0)
        {
          goto out;
        }

      seq_len = 1 + nuttx_wpa_asn1_len_size(r_len) + r_len +
                1 + nuttx_wpa_asn1_len_size(s_len) + s_len;
      sig = wpabuf_alloc(1 + nuttx_wpa_asn1_len_size(seq_len) + seq_len);
      if (sig == NULL)
        {
          goto out;
        }

      wpabuf_put_u8(sig, 0x30);
      nuttx_wpa_asn1_put_len(sig, seq_len);
      nuttx_wpa_asn1_put_int(sig, r);
      nuttx_wpa_asn1_put_int(sig, s);
      goto out;
    }

out:
  BN_clear_free(kinv);
  BN_clear_free(tmp);
  BN_clear_free(s);
  BN_clear_free(r);
  BN_clear_free(x);
  BN_clear_free(k);
  BN_clear_free(z);
  EC_POINT_clear_free(point);
  return sig;
}

struct wpabuf *crypto_ec_key_sign_r_s(struct crypto_ec_key *key,
                                      const u8 *data, size_t len)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  const u8 *pos;
  const u8 *end;
  const u8 *seq;
  const u8 *seq_end;
  struct wpabuf *der = NULL;
  struct wpabuf *raw = NULL;
  BIGNUM *r = NULL;
  BIGNUM *s = NULL;
  size_t seq_len;
  size_t prime_len;
  u8 tag;
  u8 *r_buf;
  u8 *s_buf;

  if (ec_key == NULL || ec_key->ec == NULL)
    {
      return NULL;
    }

  der = crypto_ec_key_sign(key, data, len);
  if (der == NULL)
    {
      return NULL;
    }

  pos = wpabuf_head(der);
  end = pos + wpabuf_len(der);
  if (nuttx_wpa_asn1_get_tlv(&pos, end, &tag, &seq, &seq_len) < 0 ||
      tag != 0x30 || pos != end)
    {
      goto out;
    }

  pos = seq;
  seq_end = seq + seq_len;
  if (nuttx_wpa_asn1_get_int_bn(&pos, seq_end, &r) < 0 ||
      nuttx_wpa_asn1_get_int_bn(&pos, seq_end, &s) < 0 ||
      pos != seq_end)
    {
      goto out;
    }

  prime_len = crypto_ec_prime_len(ec_key->ec);
  raw = wpabuf_alloc(2 * prime_len);
  if (raw == NULL)
    {
      goto out;
    }

  r_buf = wpabuf_put(raw, prime_len);
  s_buf = wpabuf_put(raw, prime_len);
  if (crypto_bignum_to_bin((const struct crypto_bignum *)r, r_buf,
                           prime_len, prime_len) < 0 ||
      crypto_bignum_to_bin((const struct crypto_bignum *)s, s_buf,
                           prime_len, prime_len) < 0)
    {
      wpabuf_clear_free(raw);
      raw = NULL;
    }

out:
  BN_clear_free(r);
  BN_clear_free(s);
  wpabuf_clear_free(der);
  return raw;
}

int crypto_ec_key_verify_signature(struct crypto_ec_key *key, const u8 *data,
                                   size_t len, const u8 *sig, size_t sig_len)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;
  const u8 *pos = sig;
  const u8 *end = sig + sig_len;
  const u8 *seq;
  const u8 *seq_end;
  size_t seq_len;
  u8 tag;
  BIGNUM *r = NULL;
  BIGNUM *s = NULL;
  BIGNUM *z = NULL;
  BIGNUM *w = NULL;
  BIGNUM *u1 = NULL;
  BIGNUM *u2 = NULL;
  BIGNUM *x = NULL;
  BIGNUM *v = NULL;
  EC_POINT *point = NULL;
  int ret = -1;

  if (ec_key == NULL || ec_key->ec == NULL || ec_key->pub == NULL ||
      nuttx_wpa_asn1_get_tlv(&pos, end, &tag, &seq, &seq_len) < 0 ||
      tag != 0x30 || pos != end)
    {
      return -1;
    }

  pos = seq;
  seq_end = seq + seq_len;
  if (nuttx_wpa_asn1_get_int_bn(&pos, seq_end, &r) < 0 ||
      nuttx_wpa_asn1_get_int_bn(&pos, seq_end, &s) < 0 ||
      pos != seq_end)
    {
      goto out;
    }

  if (BN_is_zero(r) || BN_is_negative(r) ||
      BN_cmp(r, ec_key->ec->order) >= 0 ||
      BN_is_zero(s) || BN_is_negative(s) ||
      BN_cmp(s, ec_key->ec->order) >= 0)
    {
      ret = 0;
      goto out;
    }

  z = nuttx_wpa_ecdsa_hash_to_bn(data, len, ec_key->ec->order);
  w = BN_mod_inverse(NULL, s, ec_key->ec->order, ec_key->ec->bnctx);
  u1 = BN_new();
  u2 = BN_new();
  x = BN_new();
  v = BN_new();
  point = EC_POINT_new(ec_key->ec->group);
  if (z == NULL || w == NULL || u1 == NULL || u2 == NULL || x == NULL ||
      v == NULL || point == NULL ||
      BN_mod_mul(u1, z, w, ec_key->ec->order, ec_key->ec->bnctx) != 1 ||
      BN_mod_mul(u2, r, w, ec_key->ec->order, ec_key->ec->bnctx) != 1 ||
      EC_POINT_mul(ec_key->ec->group, point, u1, ec_key->pub, u2,
                   ec_key->ec->bnctx) != 1 ||
      EC_POINT_is_at_infinity(ec_key->ec->group, point) ||
      EC_POINT_get_affine_coordinates(ec_key->ec->group, point, x, NULL,
                                      ec_key->ec->bnctx) != 1 ||
      BN_mod(v, x, ec_key->ec->order, ec_key->ec->bnctx) != 1)
    {
      goto out;
    }

  ret = BN_cmp(v, r) == 0 ? 1 : 0;

out:
  BN_clear_free(v);
  BN_clear_free(x);
  BN_clear_free(u2);
  BN_clear_free(u1);
  BN_clear_free(w);
  BN_clear_free(z);
  BN_clear_free(s);
  BN_clear_free(r);
  EC_POINT_clear_free(point);
  return ret;
}

int crypto_ec_key_verify_signature_r_s(struct crypto_ec_key *key,
                                       const u8 *data, size_t len,
                                       const u8 *r, size_t r_len,
                                       const u8 *s, size_t s_len)
{
  BIGNUM *r_bn = NULL;
  BIGNUM *s_bn = NULL;
  struct wpabuf *der = NULL;
  size_t r_der_len;
  size_t s_der_len;
  size_t seq_len;
  int ret = -1;

  r_bn = BN_bin2bn(r, r_len, NULL);
  s_bn = BN_bin2bn(s, s_len, NULL);
  if (r_bn == NULL || s_bn == NULL)
    {
      goto out;
    }

  r_der_len = nuttx_wpa_asn1_int_len(r_bn);
  s_der_len = nuttx_wpa_asn1_int_len(s_bn);
  if (r_der_len == 0 || s_der_len == 0)
    {
      goto out;
    }

  seq_len = 1 + nuttx_wpa_asn1_len_size(r_der_len) + r_der_len +
            1 + nuttx_wpa_asn1_len_size(s_der_len) + s_der_len;
  der = wpabuf_alloc(1 + nuttx_wpa_asn1_len_size(seq_len) + seq_len);
  if (der == NULL)
    {
      goto out;
    }

  wpabuf_put_u8(der, 0x30);
  nuttx_wpa_asn1_put_len(der, seq_len);
  nuttx_wpa_asn1_put_int(der, r_bn);
  nuttx_wpa_asn1_put_int(der, s_bn);
  ret = crypto_ec_key_verify_signature(key, data, len,
                                       wpabuf_head(der), wpabuf_len(der));

out:
  wpabuf_clear_free(der);
  BN_clear_free(r_bn);
  BN_clear_free(s_bn);
  return ret;
}

int crypto_ec_key_group(struct crypto_ec_key *key)
{
  struct nuttx_wpa_ec_key *ec_key = (struct nuttx_wpa_ec_key *)key;

  return ec_key == NULL ? -1 : ec_key->group;
}

int crypto_ec_key_cmp(struct crypto_ec_key *key1, struct crypto_ec_key *key2)
{
  struct nuttx_wpa_ec_key *ec_key1 = (struct nuttx_wpa_ec_key *)key1;
  struct nuttx_wpa_ec_key *ec_key2 = (struct nuttx_wpa_ec_key *)key2;

  if (ec_key1 == NULL || ec_key2 == NULL || ec_key1->ec == NULL ||
      ec_key2->ec == NULL || ec_key1->group != ec_key2->group ||
      ec_key1->pub == NULL || ec_key2->pub == NULL)
    {
      return -1;
    }

  return EC_POINT_cmp(ec_key1->ec->group, ec_key1->pub, ec_key2->pub,
                      ec_key1->ec->bnctx) == 0 ? 0 : -1;
}

void crypto_ec_key_debug_print(const struct crypto_ec_key *key,
                               const char *title)
{
  struct wpabuf *pub;

  pub = crypto_ec_key_get_pubkey_point((struct crypto_ec_key *)key, 1);
  if (pub == NULL)
    {
      return;
    }

  wpa_hexdump_buf_key(MSG_DEBUG, title, pub);
  wpabuf_clear_free(pub);
}

struct wpabuf *crypto_pkcs7_get_certificates(const struct wpabuf *pkcs7)
{
  wpa_printf(MSG_INFO, "NuttX WPA EC: PKCS#7 certificate extraction is not supported");
  return NULL;
}

struct crypto_csr *crypto_csr_init(void)
{
  wpa_printf(MSG_INFO, "NuttX WPA EC: DPP CSR generation is not supported");
  return NULL;
}

struct crypto_csr *crypto_csr_verify(const struct wpabuf *req)
{
  wpa_printf(MSG_INFO, "NuttX WPA EC: DPP CSR verification is not supported");
  return NULL;
}

void crypto_csr_deinit(struct crypto_csr *csr)
{
}

int crypto_csr_set_ec_public_key(struct crypto_csr *csr,
                                 struct crypto_ec_key *key)
{
  return -1;
}

int crypto_csr_set_name(struct crypto_csr *csr, enum crypto_csr_name type,
                        const char *name)
{
  return -1;
}

int crypto_csr_set_attribute(struct crypto_csr *csr, enum crypto_csr_attr attr,
                             int attr_type, const u8 *value, size_t len)
{
  return -1;
}

const u8 *crypto_csr_get_attribute(struct crypto_csr *csr,
                                   enum crypto_csr_attr attr,
                                   size_t *len, int *type)
{
  return NULL;
}

struct wpabuf *crypto_csr_sign(struct crypto_csr *csr,
                               struct crypto_ec_key *key,
                               enum crypto_hash_alg algo)
{
  return NULL;
}
