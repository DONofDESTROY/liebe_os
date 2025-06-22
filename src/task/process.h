#ifndef PROCESS_H
#define PROCESS_H

#include "../config.h"
#include "task.h"
#include <stdint.h>

struct process {
  // id of the process
  uint16_t id;

  char filename[LIEBE_OS_MAX_PATH_LEN];

  struct task *task;

  // the address of memory allocated if the process killed
  // need to clean the assigned memory
  void *allocations[LIEBE_OS_MAX_PROGRAM_ALLOCATIONS];

  // physical ptr to process memory
  void *ptr;

  // physical ptr to stack memory
  void *stack;

  // the size of the data pointed by the ptr
  uint32_t size;

  struct keyboard_buffer {
    char buffer[LIEBE_OS_KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
  } keyboard;

} __attribute__((packed));

int process_load_for_slot(const char *fileName, struct process **process,
                          int process_slot);

int process_load(const char *filename, struct process **process);

struct process *process_current();
#endif // !PROCESS_H
