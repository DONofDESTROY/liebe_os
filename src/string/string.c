#include "string.h"
#include <stdbool.h>

/*
 * returns the size of string passed
 */
size_t strlen(const char *ptr) {
  size_t len = 0;
  while (*ptr != 0) {
    len++;
    ptr++;
  }
  return len;
}

/*
 * returns the max(strlen, max_count)
 */
size_t strnlen(const char *ptr, int max) {
  size_t len = 0;
  int i = 0;
  for (i = 0; i < max; i++) {
    if (ptr[i] == 0)
      break;
    len++;
  }
  return len;
}

/*
 * checks whether the passed char is a digit
 */
bool is_digit(char c) { return c >= 48 && c <= 57; }

/*
 * converts ascii to int
 */
int atoi(char c) { return c - 48; }
