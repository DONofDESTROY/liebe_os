#include "liebeos.h"
#include "stdio.h"

#include "string.h"

// block the execution untill key press is triggerd
int liebeos_getkey_block() {
  int val = 0;
  do {
    val = liebeos_getkey();
  } while (val == 0);
  return val;
}

void liebeos_terminal_readline(char *out, int max, bool output_while_typing) {
  int i = 0;
  for (i = 0; i < max - 1; i++) {
    char key = liebeos_getkey_block();

    // enter means we have read the line
    if (key == 13) {
      break;
    }

    // only render if the output_while_typing is true
    // some programs like password input wont show the characters typed
    if (output_while_typing) {
      liebeos_putchar(key);
    }

    // Backspace
    if (key == 0x08 && i >= 1) {
      out[i - 1] = 0x00;
      // -2 because we will +1 when we go to the continue
      i -= 2;
      continue;
    }

    out[i] = key;
  }

  // Add the null terminator
  out[i] = 0x00;
}

struct command_argument *liebeos_parse_command(const char *command, int max) {
  struct command_argument *root_command = 0;
  char scommand[1025];
  if (max >= (int)sizeof(scommand)) {
    // cannot be bigger than the size
    return 0;
  }

  // make a copy
  strncpy(scommand, command, sizeof(scommand));
  // split the command by space
  char *token = strtok(scommand, " ");
  if (!token) {
    // failed to get the token
    goto out;
  }

  // create root of the command
  root_command = liebeos_malloc(sizeof(struct command_argument));
  if (!root_command) {
    goto out;
  }

  strncpy(root_command->argument, token, sizeof(root_command->argument));
  root_command->next = 0;

  struct command_argument *current = root_command;
  token = strtok(NULL, " ");
  // for the arguments
  while (token != 0) {
    struct command_argument *new_command =
        liebeos_malloc(sizeof(struct command_argument));
    if (!new_command) {
      break;
    }

    // update
    strncpy(new_command->argument, token, sizeof(new_command->argument));
    new_command->next = 0x00;
    current->next = new_command;
    current = new_command;
    token = strtok(NULL, " ");
  }
out:
  return root_command;
}

int liebeos_system_run(const char *command) {
  char buf[1024];
  strncpy(buf, command, sizeof(buf));
  struct command_argument *root_command_argument =
      liebeos_parse_command(buf, sizeof(buf));
  if (!root_command_argument) {
    return -1;
  }

  return liebeos_system(root_command_argument);
}
