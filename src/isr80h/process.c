#include "process.h"
#include "../config.h"
#include "../macros.h"
#include "../status.h"
#include "../string/string.h"
#include "../task/process.h"
#include "../task/task.h"
#include <stdint.h>

void *isr80h_command6_process_load_start(struct interrupt_frame *frame) {
  void *filename_user_ptr = task_get_stack_item(task_current(), 0);
  char filename[LIEBE_OS_MAX_PATH_LEN];
  int res = copy_string_from_task(task_current(), filename_user_ptr, filename,
                                  sizeof(filename));
  if (res < 0) {
    // cant get the string form the process
    goto exit_fn;
  }

  char path[LIEBE_OS_MAX_PATH_LEN];
  // copyt he drive path
  strcpy(path, "0:/");
  // copy the filename
  strcpy(path + 3, filename);

  struct process *process = 0;
  res = process_load_switch(path, &process);
  if (res < 0) {
    goto exit_fn;
  }

  // swith to the task
  task_switch(process->task);
  // do the registers stuff
  task_return(&process->task->registers);

exit_fn:
  return 0;
}

void *isr80h_command7_invoke_system_command(struct interrupt_frame *frame) {

  struct command_argument *arguments = task_virtual_address_to_physical(
      task_current(), task_get_stack_item(task_current(), 0));
  if (!arguments || strlen(arguments[0].argument) == 0) {
    return ERROR(-EINVARG);
  }

  // get the filename form the root
  struct command_argument *root_command_argument = &arguments[0];
  const char *program_name = root_command_argument->argument;

  // create a path from the filename
  // !NOTE create a env and cd commands
  char path[LIEBE_OS_MAX_PATH_LEN];
  strcpy(path, "0:/");
  strncpy(path + 3, program_name, sizeof(path));

  struct process *process = 0;
  // load it into the process list
  int res = process_load_switch(path, &process);
  if (res < 0) {
    return ERROR((uintptr_t)res);
  }

  // inject the argv and argc
  res = process_inject_arguments(process, root_command_argument);
  if (res < 0) {
    return ERROR((uintptr_t)res);
  }

  // switch to the new task
  task_switch(process->task);
  task_return(&process->task->registers);

  return 0;
}

void *isr80h_command8_get_program_arguments(struct interrupt_frame *frame) {
  struct process *process = task_current()->process;
  struct process_arguments *arguments = task_virtual_address_to_physical(
      task_current(), task_get_stack_item(task_current(), 0));

  process_get_arguments(process, &arguments->argc, &arguments->argv);
  return 0;
}

void *isr80h_command9_exit(struct interrupt_frame *frame) {
  struct process *process = task_current()->process;
  process_terminate(process);
  task_next();
  return 0;
}
