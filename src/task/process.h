#ifndef PROCESS_H
#define PROCESS_H

#include "../config.h"
#include "task.h"
#include <stdint.h>

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;

struct process_allocation {
  void *ptr;
  size_t size;
};

struct command_argument {
  char argument[512];
  struct command_argument *next;
};

struct process_arguments {
  int argc;
  char **argv;
};

struct process {
  // id of the process
  uint16_t id;

  char filename[LIEBE_OS_MAX_PATH_LEN];

  struct task *task;

  // the address of memory allocated if the process killed
  // need to clean the assigned memory
  struct process_allocation allocations[LIEBE_OS_MAX_PROGRAM_ALLOCATIONS];

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

  struct process_arguments arguments;
};

int process_load_for_slot(const char *fileName, struct process **process,
                          int process_slot);

int process_load(const char *filename, struct process **process);

struct process *process_current();

int process_switch(struct process *process);
int process_load_switch(const char *filename, struct process **process);
void *process_malloc(struct process *process, size_t size);
void process_free(struct process *process, void *ptr);

void process_get_arguments(struct process *process, int *argc, char ***argv);
int process_inject_arguments(struct process *process,
                             struct command_argument *root_argument);
int process_terminate(struct process *process);
#endif // !PROCESS_H
