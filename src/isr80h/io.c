#include "io.h"
#include "../kernel/kernel.h"
#include "../task/task.h"

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
