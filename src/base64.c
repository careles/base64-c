#include "base64.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifdef BASE64_USE_OPENSSL
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif  // BASE64_USE_OPENSSL

#ifndef BASE64_USE_OPENSSL
static unsigned char *baseStr = (unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                 "abcdefghijklmnopqrstuvwxyz"
                                                 "0123456789+/";

static void encode3to4(const unsigned char *src, unsigned char *dest) {
  unsigned int b32 = (unsigned int)0;
  int j = 18;

  for (int i = 0; i < 3; i++) {
    b32 <<= 8;
    b32 |= (unsigned int)src[i];
  }

  for (int i = 0; i < 4; i++) {
    dest[i] = baseStr[(unsigned int)((b32 >> j) & 0x3F)];
    j -= 6;
  }
}

static void encode2to4(const unsigned char *src, unsigned char *dest) {
  dest[0] = baseStr[(unsigned int)((src[0] >> 2) & 0x3F)];
  dest[1] =
      baseStr[(unsigned int)(((src[0] & 3) << 4) | ((src[1] >> 4) & 0xF))];
  dest[2] = baseStr[(unsigned int)((src[1] & 0xF) << 2)];
  dest[3] = (unsigned char)'=';
}

static void encode1to4(const unsigned char *src, unsigned char *dest) {
  dest[0] = baseStr[(unsigned int)((src[0] >> 2) & 0x3F)];
  dest[1] = baseStr[(unsigned int)((src[0] & 3) << 4)];
  dest[2] = (unsigned char)'=';
  dest[3] = (unsigned char)'=';
}

static void encode(const unsigned char *src, unsigned int srclen,
                   unsigned char *dest) {
  while (3 <= srclen) {
    encode3to4(src, dest);
    src += 3;
    dest += 4;
    srclen -= 3;
  }

  switch (srclen) {
    case 2:
      encode2to4(src, dest);
      break;
    case 1:
      encode1to4(src, dest);
      break;
    default:
      break;
  }
}
#endif  // BASE64_USE_OPENSSL

size_t base64_encode(char *dest, const char *src, size_t srclen) {
  if (!src) return 0;

  if (!srclen) srclen = strlen(src);

  if (!srclen) return 0;
  if (!dest) {
    fprintf(stderr, "encode buffer cache not exist!\n");
    return 0;
  }

  size_t desclen = (srclen + 2) / 3 << 2;
#ifdef BASE64_USE_OPENSSL
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
  BIO_write(bio, src, srclen);
  BIO_flush(bio);
  BIO_get_mem_ptr(bio, &bufferPtr);
  BIO_set_close(bio, BIO_NOCLOSE);
  BIO_free_all(bio);
  memcpy(dest, bufferPtr->data, desclen);
#else
  encode(src, srclen, dest);
#endif  // BASE64_USE_OPENSSL
  dest[desclen] = 0;
  return desclen;
}

#ifndef BASE64_USE_OPENSSL
static int code2value(unsigned char code) {
  if (43 == (int)code)  // +
    return 62;
  else if (47 == (int)code)  // /
    return 63;
  else if ((48 <= (int)code) && ((int)code <= 57))  // 0~9
    return ((int)code + 4);
  else if ((65 <= (int)code) && ((int)code <= 90))  // A~Z
    return ((int)code - 65);
  else if ((97 <= (int)code) && ((int)code <= 122))  // a~z
    return ((int)code - 71);
  else
    return -1;
}

static bool decode4to3(const unsigned char *src, unsigned char *dest) {
  unsigned int b32 = (unsigned int)0;

  for (int i = 0; i < 4; i++) {
    int bits = code2value(src[i]);
    if (bits < 0) return false;

    b32 <<= 6;
    b32 |= bits;
  }

  dest[0] = (unsigned char)((b32 >> 16) & 0xFF);
  dest[1] = (unsigned char)((b32 >> 8) & 0xFF);
  dest[2] = (unsigned char)(b32 & 0xFF);

  return true;
}

static bool decode3to2(const unsigned char *src, unsigned char *dest) {
  int bits = code2value(src[0]);
  if (bits < 0) return false;

  unsigned int b32 = (unsigned int)bits;
  b32 <<= 6;

  bits = code2value(src[1]);
  if (bits < 0) return true;

  b32 |= (unsigned int)bits;
  b32 <<= 4;

  bits = code2value(src[2]);
  if (bits < 0) return false;

  unsigned int ubits = (unsigned int)bits;
  b32 |= (ubits >> 2);

  dest[0] = (unsigned char)((b32 >> 8) & 0xFF);
  dest[1] = (unsigned char)(b32 & 0xFF);

  return true;
}

static bool decode2to1(const unsigned char *src, unsigned char *dest) {
  int bits = code2value(src[0]);
  if (bits < 0) return false;

  unsigned int ubits = (unsigned int)bits;
  unsigned int b32 = (ubits << 2);

  bits = code2value(src[1]);
  if (bits < 0) return false;

  ubits = (unsigned int)bits;
  b32 |= (ubits >> 4);

  dest[0] = (unsigned char)b32;

  return true;
}

static bool decode(const unsigned char *src, unsigned int srclen,
                   unsigned char *dest) {
  bool status;

  while (4 <= srclen) {
    status = decode4to3(src, dest);
    if (!status) return false;

    src += 4;
    dest += 3;
    srclen -= 4;
  }

  switch (srclen) {
    case 3:
      status = decode3to2(src, dest);
      break;
    case 2:
      status = decode2to1(src, dest);
      break;
    case 1:
      status = false;
      break;
    case 0:
      status = true;
      break;
    default:
      break;
  }

  return status;
}
#endif  // BASE64_USE_OPENSSL

size_t base64_decode(char *dest, const char *src, size_t srclen) {
  if (!src) return 0;

  if (!srclen) srclen = strlen(src);

  if (!srclen) return 0;

  if (!dest) {
    fprintf(stderr, "decode buffer cache not exist!\n");
    return 0;
  }

  size_t destlen = (srclen >> 2) * 3 + (((srclen % 4) * 3) >> 2);

#ifdef BASE64_USE_OPENSSL
  BIO *bio, *b64;
  bio = BIO_new_mem_buf(src, -1);
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_push(b64, bio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
  BIO_read(bio, dest, srclen);
  BIO_free_all(bio);
#else
  bool status =
      decode((const unsigned char *)src, srclen, (unsigned char *)dest);
  if (!status) {
    if (dest) free(dest);
    return 0;
  }
#endif  // BASE64_USE_OPENSSL

  dest[destlen] = 0;
  return destlen;
}
