#include "base64.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

char *base64_encode(const char *src, unsigned int srclen, char *dest) {
  if (!src) return 0;

  if (!srclen) srclen = strlen(src);

  if (!srclen) return 0;
  size_t desclen = (srclen + 2) / 3 << 2;
  if (!dest) dest = (char *)malloc(desclen + 1);
  encode(src, srclen, dest);
  dest[desclen] = 0;
  return dest;
}

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

char *base64_decode(const char *src, unsigned int srclen, char *dest) {
  if (!src) return 0;

  if (!srclen) srclen = strlen(src);

  if (!srclen) return 0;

  unsigned int destlen = (srclen >> 2) * 3 + (((srclen % 4) * 3) >> 2);
  if (!dest) dest = (char *)malloc(destlen + 1);

  if (!dest) return 0;

  bool status =
      decode((const unsigned char *)src, srclen, (unsigned char *)dest);
  if (!status) {
    if (dest) free(dest);
    return 0;
  }

  dest[destlen] = 0;
  return dest;
}
