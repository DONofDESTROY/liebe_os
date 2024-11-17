#ifndef HEAP_H
#define HEAP_H
#include "../../config.h"
#include <stddef.h>
#include <stdint.h>

// last bit represet wheather it is taken or not
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0b00000001
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0b00000000

// fist bit marks wheather it is a first block in set of blocks
#define HEAP_BLOCK_IS_FIRST 0b10000000
// second bit marks wheather it has blocks next to it
#define HEAP_BLOCK_HAS_NEXT 0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table {
  // ptr actual table
  HEAP_BLOCK_TABLE_ENTRY *entries;
  // size of the table
  size_t total;
};

struct heap {
  struct heap_table *table;
  // start address of the heap pool
  void *saddr;
};

/*
 * creates a heap memory
 * @param heap heap obj
 * @param  *start ptr for the heap start
 * @param  *end ptr for the heap end
 * @param  *heap_table table obj
 * @returns  res response < 0 for fail
 */
int heap_create(struct heap *heap, void *start, void *end,
                struct heap_table *table);

/*
 * marks the blocks on table and returns the pointer on the memory
 * @param heap heap obj for allocation
 * @param size size req from the heap
 * @returns ptr ptr to the memory
 */
void *heap_malloc(struct heap *heap, size_t size);

/*
 * gets an address and marks the blocks as free on heap table
 * @param heap heap obj
 * @param *ptr ptr on the heap memory to free
 */
void heap_free(struct heap *heap, void *ptr);

#endif // !HEAP_H
