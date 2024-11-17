#include "kheap.h"
#include "../../config.h"
#include "../../kernel/kernel.h"
#include "heap.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init() {
  // table block
  // total entries for the heap = 25600 blocks
  int total_table_entries = LIEBE_OS_HEAP_SIZE_BYTES / LIEBE_OS_HEAP_BLOCK_SIZE;
  // set the start address for the heap table
  kernel_heap_table.entries =
      (HEAP_BLOCK_TABLE_ENTRY *)(LIEBE_OS_HEAP_TABLE_ADDRESS);
  // set the total entries for the table
  kernel_heap_table.total = total_table_entries;

  // heap memory block
  void *end = (void *)(LIEBE_OS_HEAP_ADDRESS + LIEBE_OS_HEAP_SIZE_BYTES);
  int res = heap_create(&kernel_heap, (void *)(LIEBE_OS_HEAP_ADDRESS), end,
                        &kernel_heap_table);
  if (res < 0) {
    print("Failed to create heap\n");
  }
}

void *kmalloc(size_t size) { return heap_malloc(&kernel_heap, size); }
void kfree(void *ptr) { heap_free(&kernel_heap, ptr); }
