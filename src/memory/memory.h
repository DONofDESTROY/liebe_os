#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/*
 * sets memory of size from the given base with char
 * @param *prt pointer to memory
 * @param c  character to be filled
 * @param size  size of the memory to be filled
 */
void *memset(void *ptr, int c, size_t size);

/*
 * compares 2 mem
 * @param *s1 ptr to first mem
 * @param *s2 ptr to sec mem
 * @param count count to be compared
 * @return int 0 if both match, 1 if s1 is greater and -1 if s2 is greater
 */
int memcmp(void *s1, void *s2, int count);

#endif // !MEMORY_H
