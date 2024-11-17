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

#endif // !MEMORY_H
