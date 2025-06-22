#include "misc.h"
#include "../idt/idt.h"
#include "../task/task.h"
#include <stdint.h>

void *isr80h_command0_sum(struct interrupt_frame *frame) {
  int v2 = (int)(uintptr_t)task_get_stack_item(task_current(), 1);
  int v1 = (int)(uintptr_t)task_get_stack_item(task_current(), 0);
  return (void *)(uintptr_t)(v1 + v2);
}
