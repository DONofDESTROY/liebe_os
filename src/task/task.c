#include "task.h"
#include "../kernel/kernel.h"
#include "../loader/formats/elfloader.h"
#include "../macros.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../status.h"
#include "../string/string.h"
#include "process.h"
#include <stdint.h>

// ptr for the current excuting task
struct task *current_task = 0;

// task linked list ptrs
struct task *task_tail = 0;
struct task *task_head = 0;

int task_init(struct task *task, struct process *process);

// fn to expose current task var
struct task *task_current() { return current_task; }

// fn to create a new task
struct task *task_new(struct process *process) {
  int res = 0;
  struct task *task = kzalloc(sizeof(struct task));

  if (!task) {
    res = -ENOMEM;
    goto exit_fn;
  }

  res = task_init(task, process);
  if (res != OK) {
    goto exit_fn;
  }

  if (task_head == 0) {
    // create a fresh linked list if not exist
    task_head = task;
    task_tail = task;
    current_task = task;
    goto exit_fn;
  }

  // append the task to task tail
  task_tail->next = task;
  task->prev = task_tail;
  task_tail = task;

exit_fn:
  if (ISERR(res)) {
    task_free(task);
    return ERROR((uintptr_t)res);
  }
  return task;
}

// returns the next executable task
struct task *task_get_next() {
  if (!current_task->next) {
    return task_head;
  }
  return current_task->next;
}

static void task_list_remove(struct task *task) {

  if (task->prev) {
    // remove the task if it is in between
    task->prev->next = task->next;
  }
  if (task == task_head) {
    // remove the task if it is head
    task_head = task->next;
  }
  if (task == task_tail) {
    // change the tail if the task is tail
    task_tail = task->prev;
  }

  if (task == current_task) {
    current_task = task_get_next();
  }
}

int task_free(struct task *task) {
  // free the page table
  paging_free_4gb(task->page_directory);
  // remove it from the linked list
  task_list_remove(task);

  // free the allocated memeory
  kfree(task);
  return 0;
}

int task_switch(struct task *task) {
  current_task = task;
  page_switch(task->page_directory->directory_entry);
  return 0;
}

int task_page() {
  user_registers();
  task_switch(current_task);
  return 0;
}

int task_page_task(struct task *task) {
  user_registers();
  page_switch(task->page_directory->directory_entry);
  return 0;
}

void task_run_first_ever_task() {
  if (!current_task) {
    panic("task_run_first_ever_task(): No current task exists!\n");
  }

  task_switch(task_head);
  task_return(&task_head->registers);
}

int task_init(struct task *task, struct process *process) {
  memset(task, 0, sizeof(struct task));

  task->page_directory =
      create_new_page_directory(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
  if (!task->page_directory) {
    return -EIO;
  }

  task->registers.ip = LIEBE_OS_PROGRAM_VIRTUAL_ADDRESS;
  if (process->filetype == PROCESS_FILETYPE_ELF) {
    task->registers.ip = elf_header(process->elf_file)->e_entry;
  }

  task->registers.ss = USER_DATA_SEGMENT;
  task->registers.cs = USER_CODE_SEGMENT;
  task->registers.esp = LIEBE_OS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

  task->process = process;

  return 0;
}

void task_save_state(struct task *task, struct interrupt_frame *frame) {
  task->registers.ip = frame->ip;
  task->registers.cs = frame->cs;
  task->registers.flags = frame->flags;
  task->registers.esp = frame->esp;
  task->registers.ss = frame->ss;
  task->registers.eax = frame->eax;
  task->registers.ebp = frame->ebp;
  task->registers.ebx = frame->ebx;
  task->registers.ecx = frame->ecx;
  task->registers.edi = frame->edi;
  task->registers.edx = frame->edx;
  task->registers.esi = frame->esi;
}

void task_current_save_state(struct interrupt_frame *frame) {
  if (!task_current()) {
    panic("!No current task to save \n");
  }

  struct task *task = task_current();
  task_save_state(task, frame);
}

int copy_string_from_task(struct task *task, void *virtual, void *physical,
                          int max) {
  if (max >= PAGE_SIZE) {
    // limiting to 1 page size
    return -EINVARG;
  }

  int res = 0;
  // tmp memory to hold entry from task
  char *tmp = kzalloc(max);
  if (!tmp) {
    res = -ENOMEM;
    goto exit_fn;
  }

  uintptr_t *task_directory = task->page_directory->directory_entry;
  uintptr_t old_entry = paging_get(task_directory, tmp);
  paging_map(task->page_directory->directory_entry, tmp, tmp,
             PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
  // switch to the task page
  page_switch(task->page_directory->directory_entry);
  // copy the mem of the task page
  strncpy(tmp, virtual, max);
  // switch back to kernel page
  kernel_page();

  res = paging_set(task_directory, tmp, old_entry);
  if (res < 0) {
    res = -EIO;
    goto out_free;
  }

  strncpy(physical, tmp, max);

out_free:
  kfree(tmp);

exit_fn:
  return res;
}

// get the params from the task's stack
void *task_get_stack_item(struct task *task, int index) {
  void *result = 0;
  uint32_t *sp_ptr = (uint32_t *)(uintptr_t)task->registers.esp;
  // swith to the passed task page
  task_page_task(task);

  result = (void *)(uintptr_t)sp_ptr[index];

  // switch back to kernel page
  kernel_page();
  return result;
}

void *task_virtual_address_to_physical(struct task *task,
                                       void *virtual_address) {
  return paging_get_physical_address(task->page_directory->directory_entry,
                                     virtual_address);
}

void task_next() {
  // swithes to the next task on the list
  struct task *next_task = task_get_next();
  if (!next_task) {
    panic("No more tasks!\n");
  }

  task_switch(next_task);
  task_return(&next_task->registers);
}
