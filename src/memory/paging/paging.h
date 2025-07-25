#ifndef PAGING_H
#define PAGING_H

// flags
#include <stdint.h>
#define PAGING_IS_PRESENT 1 << 0
#define PAGING_IS_WRITEABLE 1 << 1
#define PAGING_ACCESS_FROM_ALL 1 << 2
#define PAGING_WRITE_THROUGHT 1 << 3
#define PAGING_CACHE_DISABLED 1 << 4

#define TABLE_ENTRIES_COUNT 1024
#define PAGE_SIZE 4096

struct page_directory {
  uintptr_t *directory_entry;
};

struct page_location {
  int32_t directory_index;
  int32_t table_index;
};

struct page_directory *create_new_page_directory(uint8_t flags);

void page_switch(uintptr_t *directory);
void enable_paging();

uintptr_t *get_page_directory_entry(struct page_directory *);

int paging_set(uintptr_t *directory, void *virt, uint32_t val);

void paging_free_4gb(struct page_directory *chunk);

int paging_map_to(uintptr_t *directory, void *virt, void *phys, void *phys_end,
                  int flags);
int paging_map_range(uintptr_t *directory, void *virt, void *phys, int count,
                     int flags);
int paging_map(uintptr_t *directory, void *virt, void *phys, int flags);
void *paging_align_address(void *ptr);

uintptr_t paging_get(uintptr_t *directory, void *virt);

void *paging_align_to_lower_page(void *addr);
void *paging_get_physical_address(uintptr_t *directory, void *virt);
#endif
