#include "idt.h"
#include "../config.h"
#include "../io/io.h"
#include "../kernel/kernel.h"
#include "../memory/memory.h"
#include "../status.h"
#include "../task/task.h"
#include <stdint.h>

extern void idt_load(void *ptr);

extern void int_null_handler();
extern void int_0_handler();
extern void isr80h_wrapper();

extern void *interrupt_pointer_table[LIEBE_OS_TOTAL_INTERRUPTS];
struct idt_desc idt_descriptors[LIEBE_OS_TOTAL_INTERRUPTS];
struct idtr_des idtr_descriptor;

static ISR80H_COMMAND isr80h_commnads[LIEBE_OS_ISR80H_MAX_COMMANDS];
static INTERRUPT_CALLBACK_FUNCTION
    interrupt_callbacks[LIEBE_OS_TOTAL_INTERRUPTS];

void interrupt_handler(int interrupt, struct interrupt_frame *frame) {
  kernel_page();
  if (interrupt_callbacks[interrupt] != 0) {
    task_current_save_state(frame);
    interrupt_callbacks[interrupt](frame);
  }
  task_page();
  outb(0x20, 0x20);
}

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
    idt_set(i, interrupt_pointer_table[i]);
  }

  idt_set(0, int_0_handler);
  idt_set(0x80, isr80h_wrapper);
  // load idt
  idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(
    int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback) {
  if (interrupt < 0 || interrupt >= LIEBE_OS_TOTAL_INTERRUPTS) {
    return -EINVARG;
  }

  interrupt_callbacks[interrupt] = interrupt_callback;
  return 0;
}

// int handler functions
void int_null_fn() { outb(0x20, 0x20); }
void int_0_fn() { print("\nDivide by zero found\n"); }

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
