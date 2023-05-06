#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"

int main() {
  const char *plainBuf = "This is a plain sting for base64 encode test.";
  printf("Plain Buff: %s\n", plainBuf);
  char encBuf[1024] = {0};
  size_t enclen = base64_encode(encBuf, plainBuf, strlen(plainBuf));
  if (!enclen) exit(1);
  printf("Encoded Buff: %s\n", encBuf);

  char decBuf[1024] = {0};
  size_t declen = base64_decode(decBuf, encBuf, strlen(encBuf));
  if (!declen) exit(1);
  printf("Decoded Buff: %s\n", decBuf);
  return 0;
}
