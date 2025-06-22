#ifndef TASK_H
#define TASK_H

#include "../config.h"
#include "../idt/idt.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include <stdint.h>

struct process;

struct registers {
  // registers of x86
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
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

struct task {
  /*
   * Page directory for the current task
   */
  struct page_directory *page_directory;
  /*
   * the registers state of the task
   */
  struct registers registers;

  // the process of the task
  struct process *process;

  /*
   * next task in the order
   */
  struct task *next;

  /*
   * previous task in the order
   */
  struct task *prev;
};

struct task *task_new(struct process *process);
struct task *task_current();
struct task *task_get_next();
int task_free(struct task *task);

int task_switch(struct task *task);
int task_page();

void task_run_first_ever_task();

void task_return(struct registers *regs);
void restore_general_purpose_registers(struct registers *regs);
void user_registers();

void task_current_save_state(struct interrupt_frame *frame);
int copy_string_from_task(struct task *task, void *virt, void *physical,
                          int max);

void *task_get_stack_item(struct task *task, int index);
#endif // !TASK_H
