#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// similar to gdt on boot.asm
struct gdt {
  uint16_t segment;
  uint16_t base_first;
  uint8_t base;
  uint8_t access;
  uint8_t high_flags;
  uint8_t base_24_31_bits;
} __attribute__((packed));

// easier way to declare gdt
struct gdt_structured {
  uint32_t base;
  uint32_t limit;
  uint8_t type;
} __attribute__((packed));

void gdt_load(struct gdt *gdt, int size);

void gdt_structured_to_gdt(struct gdt *gdt,
                           struct gdt_structured *structured_gdt,
                           int total_entries);

#endif // !GDT_H
