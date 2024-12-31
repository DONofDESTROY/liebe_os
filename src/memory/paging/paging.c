#include "paging.h"
#include "../../status.h"
#include "../heap/kheap.h"
#include <stdbool.h>
#include <stdint.h>

void paging_load_directory(uintptr_t *directory);

static uintptr_t *current_directory = 0;

struct page_directory *create_new_page_directory(uint8_t flags) {

  uintptr_t *directory = kzalloc(sizeof(uint32_t) * TABLE_ENTRIES_COUNT);
  int offset = 0;

  // for page descriptor
  for (int i = 0; i < TABLE_ENTRIES_COUNT; i++) {
    uintptr_t *table = kzalloc(sizeof(uint32_t) * TABLE_ENTRIES_COUNT);
    // for page table
    for (int b = 0; b < TABLE_ENTRIES_COUNT; b++) {
      table[b] = (offset + (b * PAGE_SIZE)) | flags;
    }
    // increase for next table note (each table contains 1024 entries and 4096
    // bytes for each entry)
    offset += (TABLE_ENTRIES_COUNT * PAGE_SIZE);
    // add that table to direcotry
    directory[i] = (uintptr_t)table | flags | PAGING_IS_WRITEABLE;
  }
  struct page_directory *page_directory_obj =
      kzalloc(sizeof(struct page_directory));
  page_directory_obj->directory_entry = directory;
  return page_directory_obj;
}

void page_switch(uintptr_t *directory) {
  // calls the asm function to load the directory into cr3
  paging_load_directory(directory);
  // for future use
  current_directory = directory;
}

uintptr_t *get_page_directory_entry(struct page_directory *chunk) {
  return chunk->directory_entry;
}

bool is_page_aligned(void *addr) { return ((uintptr_t)addr % PAGE_SIZE) == 0; }

/*
 * returns the page directory and page table index
 */
struct page_location get_page_indexs(void *virt_addr) {
  struct page_location page_location_obj = {.directory_index = -1,
                                            .table_index = -1};
  if (!is_page_aligned(virt_addr))
    return page_location_obj;

  page_location_obj.directory_index =
      (uintptr_t)virt_addr / (TABLE_ENTRIES_COUNT * PAGE_SIZE);
  page_location_obj.table_index =
      ((uintptr_t)virt_addr % (TABLE_ENTRIES_COUNT * PAGE_SIZE) / PAGE_SIZE);
  return page_location_obj;
}

int paging_set(uintptr_t *directory, void *virt, uint32_t val) {
  if (!is_page_aligned(virt)) {
    return -EINVARG;
  }
  struct page_location page_location_obj = get_page_indexs(virt);
  if (page_location_obj.table_index == -1 ||
      page_location_obj.directory_index == -1) {
    return -EINVARG;
  }
  uintptr_t entry = directory[page_location_obj.directory_index];
  uintptr_t *table = (uintptr_t *)(entry & 0xfffff000);
  table[page_location_obj.table_index] = val;
  return 0;
}
