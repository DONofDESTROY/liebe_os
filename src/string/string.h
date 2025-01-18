#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
size_t strlen(const char *ptr);

size_t strnlen(const char *ptr, int max);

bool is_digit(char c);

int atoi(char c);

#endif // !STRING_H
