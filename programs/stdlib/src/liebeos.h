#ifndef LIEBEOS_H
#define LIEBEOS_H

#include <stdbool.h>
#include <stddef.h>

struct command_argument {
  char argument[512];
  struct command_argument *next;
};

struct process_arguments {
  int argc;
  char **argv;
};

void print(const char *string_to_print);
int liebeos_getkey();
void *liebeos_malloc(size_t size);
void liebeos_free(void *ptr);

int liebeos_getkey_block();

void liebeos_terminal_readline(char *out, int max, bool output_while_typing);
void liebeos_process_load_start(const char *filename);

struct command_argument *liebeos_parse_command(const char *command, int max);
void liebeos_process_get_arguments(struct process_arguments *arguments);

int liebeos_system(struct command_argument *arguments);

int liebeos_system_run(const char *command);

void liebeos_exit();

#endif // !LIEBEOS_H
