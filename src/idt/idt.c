#include "idt.h"
#include "../config.h"
#include "../io/io.h"
#include "../kernel/kernel.h"
#include "../memory/memory.h"
#include "../task/task.h"
#include <stdint.h>

extern void idt_load(void *ptr);

extern void int_null_handler();
extern void int_0_handler();
extern void int_21h_handler();
extern void isr80h_wrapper();

struct idt_desc idt_descriptors[LIEBE_OS_TOTAL_INTERRUPTS];
struct idtr_des idtr_descriptor;

static ISR80H_COMMAND isr80h_commnads[LIEBE_OS_ISR80H_MAX_COMMANDS];

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
  idt_set(0x80, isr80h_wrapper);
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

void isr80h_register_command(int command_id, ISR80H_COMMAND command) {
  if (command_id < 0 || command_id >= LIEBE_OS_ISR80H_MAX_COMMANDS) {
    panic("!Command out of bounds");
  }
  if (isr80h_commnads[command_id]) {
    panic("!Trying to set on exisitng command");
  }
  isr80h_commnads[command_id] = command;
}

void *isr80h_handle_command(int command, struct interrupt_frame *frame) {
  void *res = 0;
  if (command < 0 || command >= LIEBE_OS_ISR80H_MAX_COMMANDS) {
    // Invalid command
    return 0;
  }

  ISR80H_COMMAND fn_ptr = isr80h_commnads[command];
  if (!fn_ptr) {
    return 0;
  }

  res = fn_ptr(frame);
  return res;
}

/*
 * returns the fn ptr for the matched command
 */
void *isr80h_handler(int command, struct interrupt_frame *frame) {
  void *res = 0;
  kernel_page(); // switch to kernel page

  task_current_save_state(frame); // saves the current registers

  res = isr80h_handle_command(command, frame);

  task_page(); // switch back to task page

  return res;
}
