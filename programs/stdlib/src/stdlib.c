
#include "stdlib.h"
#include "liebeos.h"

void *malloc(size_t size) { return liebeos_malloc(size); }

void free(void *ptr) { liebeos_free(ptr); }

// convert a number to string ex itoa(123) -> "123"
char *itoa(int i) {
  static char text[12];
  int loc = 11;
  text[11] = 0; // null termination
  char neg = 1;
  if (i >= 0) {
    // number is positive
    neg = 0;
    i = -i;
  }
  while (i) {
    text[--loc] = '0' - (i % 10);
    i /= 10;
  }
  if (loc == 11) {
    text[--loc] = '0';
  }

  if (neg)
    text[--loc] = '-';

  return &text[loc];
}
