#include "io.h"
#include "../kernel/kernel.h"
#include "../keyboard/keyboard.h"
#include "../task/task.h"
#include <stdint.h>

void *isr80h_command1_print(struct interrupt_frame *frame) {
  // take the ptr for the string in stack of the task
  void *user_space_msg_buff = task_get_stack_item(task_current(), 0);
  // tmp buffer to hold the copy form the task stack
  char buf[1024];
  // copy form the task page to the tmp buf
  copy_string_from_task(task_current(), user_space_msg_buff, buf, sizeof(buf));
  // print the copied buf
  print(buf);

  return 0;
}

void *isr80h_command2_getkey(struct interrupt_frame *frame) {
  char c = keyboard_pop();
  return (void *)((uintptr_t)c);
}

void *isr80h_commad3_putchar(struct interrupt_frame *frame) {
  char c = (char)(uintptr_t)task_get_stack_item(task_current(), 0);
  terminal_writechar(c, 15);
  return 0;
}
