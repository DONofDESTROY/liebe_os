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

void paging_free_4gb(struct page_directory *chunk) {
  for (int i = 0; i < 1024; i++) {
    uintptr_t entry = chunk->directory_entry[i];
    uintptr_t *table = (uintptr_t *)(entry & 0xfffff000);
    kfree(table);
  }
  kfree(chunk->directory_entry);
  kfree(chunk);
}

void *paging_align_address(void *ptr) {
  if ((uintptr_t)ptr % PAGE_SIZE) {
    return (void *)((uintptr_t)ptr + PAGE_SIZE - ((uintptr_t)ptr % PAGE_SIZE));
  }

  return ptr;
}

int paging_map(uintptr_t *directory, void *virt, void *phys, int flags) {
  if (((uintptr_t)virt % PAGE_SIZE) || ((uintptr_t)phys % PAGE_SIZE)) {
    // is not page aligned
    return -EINVARG;
  }

  return paging_set(directory, virt, (uintptr_t)phys | flags);
}

int paging_map_range(uintptr_t *directory, void *virt, void *phys, int count,
                     int flags) {
  int res = 0;
  for (int i = 0; i < count; i++) {
    res = paging_map(directory, virt, phys, flags);
    if (res < 0)
      break;
    virt += PAGE_SIZE;
    phys += PAGE_SIZE;
  }

  return res;
}

int paging_map_to(uintptr_t *direcotry, void *virt, void *phys, void *phys_end,
                  int flags) {
  int res = 0;
  if ((uintptr_t)virt % PAGE_SIZE) {
    // if the virt is not aligned
    res = -EINVARG;
    goto exit_fn;
  }

  if ((uintptr_t)phys % PAGE_SIZE) {
    // if the phys is not aligned
    res = -EINVARG;
    goto exit_fn;
  }

  if ((uintptr_t)phys_end % PAGE_SIZE) {
    // if the phys_end is not aligned
    res = -EINVARG;
    goto exit_fn;
  }

  if ((uintptr_t)phys_end < (uintptr_t)phys) {
    // if the end > start
    res = -EINVARG;
    goto exit_fn;
  }

  uint32_t total_bytes = phys_end - phys;
  int total_pages = total_bytes / PAGE_SIZE;
  res = paging_map_range(direcotry, virt, phys, total_pages, flags);

exit_fn:
  return res;
}

// returns the physical address for the passed virtual address
uintptr_t paging_get(uintptr_t *directory, void *virtual) {
  struct page_location page_location_obj = get_page_indexs(virtual);
  uint32_t directory_index = page_location_obj.directory_index;
  uint32_t table_index = page_location_obj.table_index;
  uintptr_t entry = directory[directory_index];
  uintptr_t *table = (uintptr_t *)(entry & 0xfffff000);
  return table[table_index];
}

void *paging_align_to_lower_page(void *addr) {
  uint32_t _addr = (uint32_t)(uintptr_t)addr;
  _addr -= (_addr % PAGE_SIZE);
  return (void *)(uintptr_t)_addr;
}

void *paging_get_physical_address(uintptr_t *directory, void *virt) {
  void *virt_addr_new = (void *)paging_align_to_lower_page(virt);
  void *diff = (void *)((uintptr_t)virt - (uintptr_t)virt_addr_new);
  return (void *)((paging_get(directory, virt_addr_new) & 0xfffff000) + diff);
}
