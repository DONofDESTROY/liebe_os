
#include "string.h"
#include <stdbool.h>

// !Note copy of file from the kerenel

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

char *strcpy(char *dest, const char *src) {
  char *res = dest;
  while (*src != 0) {
    *dest = *src;
    src += 1;
    dest += 1;
  }
  return res;
}

/*
 * copy src to dest for max of count digits
 */
char *strncpy(char *dest, const char *src, int count) {
  int i = 0;
  for (i = 0; i < count - 1; i++) {
    if (src[i] == 0x00)
      break;

    dest[i] = src[i];
  }

  dest[i] = 0x00;
  return dest;
}

/*
 * compares 2 string for n char
 */
int strncmp(const char *str1, const char *str2, int n) {
  unsigned char ptr1, ptr2;

  // iterate for n times
  while (n-- > 0) {
    // take 1 byte from the string
    ptr1 = (unsigned char)*str1++;
    ptr2 = (unsigned char)*str2++;
    if (ptr1 != ptr2) {
      return ptr1 - ptr2;
    } else if (ptr1 == '\0') {
      return 0;
    }
  }

  return 0;
}

/*
 * conerts to lower case
 */
char to_lower(char c) {
  char convert = c;
  if (c >= 65 && c <= 90) {
    convert += 32;
  }
  return convert;
}

/*
 * compares 2 string for n char (case insensitive)
 */
int istrncmp(const char *str1, const char *str2, int n) {
  unsigned char ptr1, ptr2;

  // iterate for n times
  while (n-- > 0) {
    // take 1 byte from the string
    ptr1 = (unsigned char)*str1++;
    ptr2 = (unsigned char)*str2++;
    if (ptr1 != ptr2 && to_lower(ptr1) != to_lower(ptr2)) {
      return ptr1 - ptr2;
    } else if (ptr1 == '\0') {
      return 0;
    }
  }

  return 0;
}

/*
 * returns the strlen up untill null terminator or passed char
 */
int strnlen_terminator(const char *str, int max, char terminator) {
  int i = 0;
  for (i = 0; i <= max; i++) {
    if (str[i] == '\0' || str[i] == terminator) {
      break;
    }
  }

  return i;
}

char *sp = 0;
bool single_quote_started = false;
bool double_quote_started = false;
// char *strtok(char *str, const char *delimiters) {
//   int i = 0;
//   int len = strlen(delimiters);
//   if (!str && !sp)
//     return 0;
//
//   if (str && !sp) {
//     sp = str;
//   }
//
//   char *p_start = sp;
//   while (1) {
//     for (i = 0; i < len; i++) {
//       if (*p_start == delimiters[i]) {
//         p_start++;
//         break;
//       }
//     }
//
//     if (i == len) {
//       sp = p_start;
//       break;
//     }
//   }
//
//   if (*sp == '\0') {
//     sp = 0;
//     return sp;
//   }
//
//   // Find end of substring
//   while (*sp != '\0') {
//     for (i = 0; i < len; i++) {
//       if (*sp == delimiters[i]) {
//         *sp = '\0';
//         break;
//       }
//     }
//
//     sp++;
//     if (i < len)
//       break;
//   }
//
//   return p_start;
// }

char *strtok(char *str, const char *delimiters) {
  int i = 0;
  int len = strlen(delimiters);
  if (!str && !sp)
    return 0;

  if (str && !sp) {
    sp = str;
  }

  char *p_start = sp;
  while (1) {
    for (i = 0; i < len; i++) {
      if (*p_start == delimiters[i]) {
        p_start++;
        break;
      }
    }

    if (i == len) {
      sp = p_start;
      break;
    }
  }

  if (*sp == '\0') {
    sp = 0;
    return sp;
  }

  // Find end of substring
  while (*sp != '\0') {

    if (*sp == '\'') {
      single_quote_started = !single_quote_started;
    }
    if (*sp == '\"') {
      double_quote_started = !double_quote_started;
    }

    if (single_quote_started || double_quote_started) {

      for (i = 0; i < len; i++) {
        if (*sp == delimiters[i]) {
          *sp = '\0';
          break;
        }
      }
    }

    sp++;
    if (i < len)
      break;
  }

  return p_start;
}
