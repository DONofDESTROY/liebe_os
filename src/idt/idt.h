#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_desc {
  uint16_t offset_1; // address low
  uint16_t selector; // selector in gdt
  uint8_t zero;      // just zero unused
  uint8_t type_attr; // gate type, dpl, and p flags
  uint16_t offset_2; // address high
} __attribute__((packed));

struct idtr_des {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

void idt_init();

void enable_interrupts();
void disable_interrupts();

#endif // !IDT_H
