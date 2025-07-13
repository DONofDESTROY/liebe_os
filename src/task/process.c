#include "process.h"
#include "../config.h"
#include "../fs/file.h"
#include "../kernel/kernel.h"
#include "../loader/formats/elfloader.h"
#include "../macros.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../status.h"
#include "../string/string.h"
#include "task.h"
#include <stdint.h>

// ptr to the current process that is running
struct process *current_process = 0;

// all the process
static struct process *processes[LIEBE_OS_MAX_PROCESSES] = {};

static void process_init(struct process *process) {
  memset(process, 0, sizeof(struct process));
}

// returns the current process
struct process *process_current() { return current_process; }

struct process *process_get(int process_id) {
  // returns the process based on id
  if (process_id < 0 || process_id >= LIEBE_OS_MAX_PROCESSES) {
    return (void *)-EINVARG;
  }
  return processes[process_id];
}

// returns the free slot in the processes arr
int process_get_free_slot() {
  for (int i = 0; i < LIEBE_OS_MAX_PROCESSES; i++) {
    if (processes[i] == 0) {
      return i;
    }
  }
  return -EISTKN;
}

// load the process in the memory
int process_load(const char *filename, struct process **process) {
  int res = 0;
  int process_slot = process_get_free_slot();
  if (process_slot < 0) {
    res = -EISTKN;
    goto exit_fn;
  }
  res = process_load_for_slot(filename, process, process_slot);

exit_fn:
  return res;
}

static int process_load_binary(const char *fileName, struct process *process) {
  int res = 0;
  int fd = fopen(fileName, "r");
  if (!fd) {
    // faield to get file descriptor
    res = -EIO;
    goto exit_fn;
  }

  struct file_stat stat;
  res = fstat(fd, &stat);
  if (res != OK) {
    // faile to get stat of the file
    goto exit_fn;
  }

  void *program_data_ptr = kzalloc(stat.filesize);
  if (!program_data_ptr) {
    // not enough mem
    res = -ENOMEM;
    goto exit_fn;
  }

  // read the binary as 1 block of size fd
  if (fread(program_data_ptr, stat.filesize, 1, fd) != 1) {
    // failed to read the whole binary
    res -= EIO;
    goto exit_fn;
  }

  // process is of type binary
  process->filetype = PROCESS_FILETYPE_BINARY;
  process->ptr = program_data_ptr;
  process->size = stat.filesize;

exit_fn:
  fclose(fd);
  return res;
}

static int process_load_elf(const char *filename, struct process *process) {
  int res = 0;
  struct elf_file *elf_file = 0;
  res = elf_load(filename, &elf_file);
  if (ISERR(res)) {
    goto out;
  }

  // file type is elf
  process->filetype = PROCESS_FILETYPE_ELF;
  process->elf_file = elf_file;
out:
  return res;
}

static int process_load_data(const char *fileName, struct process *process) {
  int res = 0;
  res = process_load_elf(fileName, process);
  if (res == -EINFORMAT) {
    // failed to laoad as elf so load it as bin
    res = process_load_binary(fileName, process);
  }
  return res;
}

static int process_map_elf(struct process *process) {
  int res = 0;

  struct elf_file *elf_file = process->elf_file;

  struct elf_header *header = elf_header(elf_file);
  struct elf32_phdr *phdrs = elf_pheader(header);
  for (int i = 0; i < header->e_phnum; i++) {
    struct elf32_phdr *phdr = &phdrs[i];
    void *phdr_phys_address = elf_phdr_phys_address(elf_file, phdr);
    int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
    if (phdr->p_flags & PF_W) {
      flags |= PAGING_IS_WRITEABLE;
    }
    res = paging_map_to(
        process->task->page_directory->directory_entry,
        paging_align_to_lower_page((void *)(uintptr_t)phdr->p_vaddr),
        paging_align_to_lower_page(phdr_phys_address),
        paging_align_address(phdr_phys_address + phdr->p_memsz), flags);
    if (ISERR(res)) {
      break;
    }
  }
  return res;
}

int process_map_binary(struct process *process) {
  int res = 0;
  res = paging_map_to(process->task->page_directory->directory_entry,
                      (void *)LIEBE_OS_PROGRAM_VIRTUAL_ADDRESS, process->ptr,
                      paging_align_address(process->ptr + process->size),
                      PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL |
                          PAGING_IS_WRITEABLE);
  return res;
}

int process_map_memory(struct process *process) {
  int res = 0;

  switch (process->filetype) {
  case PROCESS_FILETYPE_ELF:
    res = process_map_elf(process);
    break;

  case PROCESS_FILETYPE_BINARY:
    res = process_map_binary(process);
    break;

  default:
    panic("process_map_memory: Invalid filetype\n");
  }

  if (res < 0) {
    goto exit_fn;
  }

  paging_map_to(
      process->task->page_directory->directory_entry,
      (void *)LIEBE_OS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack,
      paging_align_address(process->stack + LIEBE_OS_USER_PROGRAM_STACK_SIZE),
      PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
exit_fn:
  return res;
}

// load the process in the provided slot
int process_load_for_slot(const char *fileName, struct process **process,
                          int process_slot) {
  int res = 0;
  struct task *task = 0;
  struct process *_process;
  void *program_stack_ptr = 0;

  if (process_get(process_slot) != 0) {
    // a process exist
    res = -EISTKN;
    goto exit_fn;
  }

  // create mem for process
  _process = kzalloc(sizeof(struct process));
  if (!process) {
    res = -ENOMEM;
    goto exit_fn;
  }

  // clear the mem
  process_init(_process);

  // load the data into the process
  res = process_load_data(fileName, _process);
  if (res < 0) {
    goto exit_fn;
  }

  // create the stack for the process
  program_stack_ptr = kzalloc(LIEBE_OS_USER_PROGRAM_STACK_SIZE);
  if (!program_stack_ptr) {
    // not enough mem
    res = -ENOMEM;
    goto exit_fn;
  }

  // copy the file name to the process
  strncpy(_process->filename, fileName, sizeof(_process->filename));
  // assign the stack ptr
  _process->stack = program_stack_ptr;
  // assign the id on the arr
  _process->id = process_slot;

  // create a new task
  task = task_new(_process);
  if (ERROR_I(task) == 0) {
    // failed to create new task
    res = ERROR_I(task);
    goto exit_fn;
  }

  // assign the task with the process
  _process->task = task;

  res = process_map_memory(_process);
  if (res < 0) {
    goto exit_fn;
  }

  *process = _process;

  processes[process_slot] = _process;

exit_fn:
  if (ISERR(res)) {
    if (_process && _process->task) {
      task_free(_process->task);
    }
  }
  return res;
}

int process_switch(struct process *process) {
  current_process = process;
  return 0;
}

int process_load_switch(const char *filename, struct process **process) {
  int res = process_load(filename, process);
  if (res == 0) {
    process_switch(*process);
  }

  return res;
}

static int process_find_free_allocation_index(struct process *process) {
  int res = -ENOMEM;
  for (int i = 0; i < LIEBE_OS_MAX_PROGRAM_ALLOCATIONS; i++) {
    // findn the free spot on the array and return its index
    if (process->allocations[i].ptr == 0) {
      res = i;
      break;
    }
  }

  return res;
}

void *process_malloc(struct process *process, size_t size) {
  // create mem for the allocation
  void *ptr = kzalloc(size);
  if (!ptr) {
    // no mem
    goto out_err;
  }

  // find the free index to store this allocation
  int index = process_find_free_allocation_index(process);
  if (index < 0) {
    // out of allocations
    goto out_err;
  }

  int res = paging_map_to(process->task->page_directory->directory_entry, ptr,
                          ptr, paging_align_address(ptr + size),
                          PAGING_IS_WRITEABLE | PAGING_IS_PRESENT |
                              PAGING_ACCESS_FROM_ALL);
  if (res < 0) {
    goto out_err;
  }
  // store it so that this can be hadled when program crash
  process->allocations[index].ptr = ptr;
  process->allocations[index].size = size;

  return ptr;

out_err:
  if (ptr) {
    kfree(ptr);
  }
  return 0;
}

static int is_current_process_ptr(struct process *process, void *ptr) {
  for (int i = 0; i < LIEBE_OS_MAX_PROGRAM_ALLOCATIONS; i++) {
    if (process->allocations[i].ptr == ptr) {
      // ptr found in allocation of the current process
      return i;
    }
  }
  // failed to find the allocation in the process ptr ie doesen't belong to
  // current process
  return -1;
}

void process_free(struct process *process, void *ptr) {
  int allocation_idx = is_current_process_ptr(process, ptr);

  if (allocation_idx < 0) {
    // not part of current process
    return;
  }
  struct process_allocation *allocation = &process->allocations[allocation_idx];

  int res = paging_map_to(
      process->task->page_directory->directory_entry, allocation->ptr,
      allocation->ptr, paging_align_address(allocation->ptr + allocation->size),
      0x00);

  if (res < 0) {
    return;
  }

  process->allocations[allocation_idx].ptr = 0x00;
  process->allocations[allocation_idx].size = 0;

  kfree(ptr);
}

void process_get_arguments(struct process *process, int *argc, char ***argv) {
  *argc = process->arguments.argc;
  *argv = process->arguments.argv;
}

int process_count_commnad_arguments(struct command_argument *root_argument) {
  struct command_argument *current = root_argument;
  int count = 0;
  while (current) {
    count++;
    current = current->next;
  }
  return count;
}

int process_inject_arguments(struct process *process,
                             struct command_argument *root_argument) {
  int res = 0;
  struct command_argument *current = root_argument;
  int i = 0;
  int argc = process_count_commnad_arguments(root_argument);
  if (argc == 0) {
    // no argumnets
    res = -EIO;
    goto exit_fn;
  }

  char **argv = process_malloc(process, sizeof(const char *) * argc);
  if (!argv) {
    // no memory
    res = -ENOMEM;
    goto exit_fn;
  }

  while (current) {
    char *argument_str = process_malloc(process, sizeof(current->argument));
    if (!argument_str) {
      res = -ENOMEM;
      goto exit_fn;
    }

    strncpy(argument_str, current->argument, sizeof(current->argument));
    argv[i] = argument_str;
    current = current->next;
    i++;
  }

  process->arguments.argc = argc;
  process->arguments.argv = argv;
exit_fn:
  return res;
}

int process_terminate_allocations(struct process *process) {
  for (int i = 0; i < LIEBE_OS_MAX_PROGRAM_ALLOCATIONS; i++) {
    kfree(process->allocations[i].ptr);
  }
  return 0;
}

int process_free_binary_data(struct process *process) {

  kfree(process->ptr);
  return 0;
}

int process_free_elf_data(struct process *process) {
  elf_close(process->elf_file);
  return 0;
}

int process_free_program_data(struct process *process) {
  int res = 0;
  switch (process->filetype) {
  case PROCESS_FILETYPE_BINARY:
    res = process_free_binary_data(process);
    break;
  case PROCESS_FILETYPE_ELF:
    res = process_free_elf_data(process);
    break;
  default:
    res = -EINVARG;
  }
  return res;
}

void process_switch_to_any() {
  for (int i = 0; i < LIEBE_OS_MAX_PROCESSES; i++) {
    if (processes[i]) {
      process_switch(processes[i]);
      return;
    }
  }
  panic("No process to switch too \n");
}

static void process_unlink(struct process *process) {

  // take the process from the list
  processes[process->id] = 0x00;
  if (current_process == process) {
    process_switch_to_any();
  }
}

int process_terminate(struct process *process) {
  int res = 0;
  // first free the allocations
  res = process_terminate_allocations(process);
  // free the actual data
  res = process_free_program_data(process);
  if (res < 0) {
    goto exit_fn;
  }

  // free the process stack memory
  kfree(process->stack);
  // clear the task
  task_free(process->task);

  process_unlink(process);

exit_fn:
  return res;
}
