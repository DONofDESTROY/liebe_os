#include "idt.h"
#include "../config.h"
#include "../io/io.h"
#include "../kernel/kernel.h"
#include "../memory/memory.h"
#include <stdint.h>

extern void idt_load(void *ptr);

extern void int_null_handler();
extern void int_0_handler();
extern void int_21h_handler();

struct idt_desc idt_descriptors[LIEBE_OS_TOTAL_INTERRUPTS];
struct idtr_des idtr_descriptor;

void idt_set(int interrupt_no, void *address) {
  struct idt_desc *descriptor = &idt_descriptors[interrupt_no];
  descriptor->offset_1 = (uintptr_t)address & 0x0000ffff;
  descriptor->selector = KERNEL_CODE_SELECTOR;
  descriptor->zero = 0x00;
  descriptor->type_attr = 0xEE;
  descriptor->offset_2 = (uintptr_t)address >> 16;
}

void idt_init() {
  memset(idt_descriptors, 0, sizeof(idt_descriptors));
  idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
  idtr_descriptor.base = (uintptr_t)idt_descriptors;

  for (int i = 0; i < LIEBE_OS_TOTAL_INTERRUPTS; i++) {
    idt_set(i, int_null_handler);
  }

  idt_set(0, int_0_handler);
  idt_set(0x21, int_21h_handler);
  // load idt
  idt_load(&idtr_descriptor);
}

// int handler functions
void int_null_fn() { outb(0x20, 0x20); }
void int_0_fn() { print("\nDivide by zero found\n"); }
void int_21h_fn() {
  print("\nKeyboard pressed\n");
  outb(0x20, 0x20);
}
