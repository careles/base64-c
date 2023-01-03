#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"

int main() {
  char *plainBuf = "This is a plain sting for base64 encode test.";
  char *encBuf = base64_encode(plainBuf, strlen(plainBuf), 0);
  char *decBuf = base64_decode(encBuf, strlen(encBuf), 0);
  printf("Plain Buff: %s\n", plainBuf);
  printf("Encoded Buff: %s\n", encBuf);
  printf("Decoded Buff: %s\n", decBuf);
  free(encBuf);
  free(decBuf);
  return 0;
}
