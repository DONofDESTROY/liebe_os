#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct interrupt_frame;

typedef void *(*ISR80H_COMMAND)(struct interrupt_frame *frame);

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

struct interrupt_frame {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t reserved;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t ip;
  uint32_t cs;
  uint32_t flags;
  uint32_t esp;
  uint32_t ss;
} __attribute__((packed));

void idt_init();

void enable_interrupts();
void disable_interrupts();

void isr80h_register_command(int command_id, ISR80H_COMMAND command);

#endif // !IDT_H
