#ifndef PROCESS_H
#define PROCESS_H

#include "../config.h"
#include "task.h"
#include <stdint.h>

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;

struct process {
  // id of the process
  uint16_t id;

  char filename[LIEBE_OS_MAX_PATH_LEN];

  struct task *task;

  // the address of memory allocated if the process killed
  // need to clean the assigned memory
  void *allocations[LIEBE_OS_MAX_PROGRAM_ALLOCATIONS];

  PROCESS_FILETYPE filetype;

  union {
    // The physical pointer to the process memory.
    void *ptr;
    struct elf_file *elf_file;
  };

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

int process_switch(struct process *process);
int process_load_switch(const char *filename, struct process **process);
#endif // !PROCESS_H
