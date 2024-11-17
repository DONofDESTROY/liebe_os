#include "heap.h"

#include "../../kernel/kernel.h"
#include "../../memory/memory.h"
#include "../../status.h"
#include "heap.h"
#include <stdbool.h>
#include <stdint.h>

/*
 * validate the heap table obj
 */
static int heap_validate_table(void *ptr, void *end, struct heap_table *table) {
  int res = 0;
  size_t table_size = (size_t)(end - ptr);
  size_t total_blocks = table_size / LIEBE_OS_HEAP_BLOCK_SIZE;
  if (table->total != total_blocks) {
    res = -EINVARG;
    goto out;
  }
out:
  return res;
}

/*
 * validate the ptr ie multiples of 4096
 */
static bool heap_validate_alignment(void *ptr) {
  return ((uintptr_t)ptr % LIEBE_OS_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct heap *heap, void *ptr, void *end,
                struct heap_table *table) {
  int res = 0;
  if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end)) {
    res = -EINVARG;
    goto out;
  }
  memset(heap, 0, sizeof(struct heap));
  heap->saddr = ptr;
  heap->table = table;
  res = heap_validate_table(ptr, end, table);
  if (res < 0) {
    goto out;
  }
  size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
  memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);
out:
  return res;
}

/*
 * makes the req size eq to multiples of 4096
 *  always ceils
 */
static uint32_t heap_align_value_to_upper(uint32_t val) {
  if ((val % LIEBE_OS_HEAP_BLOCK_SIZE) == 0) {
    return val;
  }
  val = (val - (val % LIEBE_OS_HEAP_BLOCK_SIZE));
  val += LIEBE_OS_HEAP_BLOCK_SIZE;
  return val;
}

/*
 * returns starting block number for req size on the heap table
 * @param *heap heap obj
 * @param total_blocks total number of blocks requested
 */
int heap_get_start_block(struct heap *heap, uint32_t total_blocks) {
  struct heap_table *table = heap->table;
  int block_count = 0;
  int block_start = -1;
  for (size_t i = 0; i < table->total; i++) {
    // last bit is set
    if (table->entries[i] & HEAP_BLOCK_TABLE_ENTRY_TAKEN) {
      block_count = 0;
      block_start = -1;
      continue;
    }
    // If this is the first block
    if (block_start == -1) {
      block_start = i;
    }
    // inc the block count
    block_count++;
    // if the block count found break the loop
    if (block_count == total_blocks) {
      break;
    }
  }
  // if still it points to -1 then there is no mem
  if (block_start == -1) {
    return -ENOMEM;
  }

  // return the block start
  return block_start;
}

/*
 * converts block to heap memory
 * note 1 block = 4096 bytes
 * @param heap heap obj
 * @param block count of blocks
 */
void *heap_block_to_address(struct heap *heap, int block) {
  return heap->saddr + (block * LIEBE_OS_HEAP_BLOCK_SIZE);
}

/*
 * convert heap address to table block index
 * @param heap heap obj
 * @param address address on the heap mem
 */
int heap_address_to_block(struct heap *heap, void *address) {
  return ((int)(address - heap->saddr)) / LIEBE_OS_HEAP_BLOCK_SIZE;
}

/*
 * mark the blocks that have been used based on the block type
 * uses bit operations to mark the table entries
 * @param *heap heap obj
 * @param start_block start block on the table
 * @param total_block end block on the table
 */
void heap_mark_blocks_taken(struct heap *heap, int start_block,
                            int total_blocks) {
  int end_block = (start_block + total_blocks) - 1;

  HEAP_BLOCK_TABLE_ENTRY entry =
      HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
  if (total_blocks > 1) {
    entry |= HEAP_BLOCK_HAS_NEXT;
  }
  for (int i = start_block; i <= end_block; i++) {
    heap->table->entries[i] = entry;
    entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
    if (i != end_block - 1) {
      entry |= HEAP_BLOCK_HAS_NEXT;
    }
  }
}

void *heap_malloc_blocks(struct heap *heap, uint32_t total_blocks) {
  void *address = 0;
  // get free starting block
  int start_block = heap_get_start_block(heap, total_blocks);
  if (start_block < 0) {
    goto out;
  }
  // convert to heap address
  address = heap_block_to_address(heap, start_block);
  // mark the blocks as taken
  heap_mark_blocks_taken(heap, start_block, total_blocks);
out:
  return address;
}

/*
 * marks the block as free ie 0
 * @param *heap heap obj
 * @param starting_block starting block on the table
 */
void heap_mark_blocks_free(struct heap *heap, int starting_block) {
  struct heap_table *table = heap->table;
  for (int i = starting_block; i < (int)table->total; i++) {
    HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
    table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
    if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
      break;
    }
  }
}

void *heap_malloc(struct heap *heap, size_t size) {
  // always ceil
  // min block size is 4096
  // malloc(50) marks 1 block of memeory on table ie 4096 bytes
  // malloc(5000) marks 2 blocks of memory on the table ie 4096 * 2
  size_t aligned_size = heap_align_value_to_upper(size);
  uint32_t total_blocks = aligned_size / LIEBE_OS_HEAP_BLOCK_SIZE;

  return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap *heap, void *ptr) {
  heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}
