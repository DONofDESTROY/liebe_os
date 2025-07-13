#include "stdio.h"
#include "./liebeos.h"
#include "stdlib.h"
#include <stdarg.h>

int putchar(int c) {
  liebeos_putchar((char)c);
  return 0;
}

// print format
int printf(const char *fmt, ...) {
  va_list ap;
  const char *p;
  char *sval;
  int ival;

  va_start(
      ap,
      fmt); // init ap after the starting address after the end of fmt variable
  for (p = fmt; *p; p++) {
    if (*p != '%') {
      // not a formatter hence print the character
      putchar(*p);
      continue;
    }

    switch (*++p) {
    case 'd':
      // int formatter ie print the value as number
      ival = va_arg(ap, int);
      print(itoa(ival));
      break;

    case 's':
      // string formatter ie print the value as string
      sval = va_arg(ap, char *);
      print(sval);
      break;

    default:
      putchar(*p);
      break;
    }
  }

  // cleanup the va list after processing
  va_end(ap);

  return 0;
}
