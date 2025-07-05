#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../task/process.h"

typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard {

  // init function ptr
  KEYBOARD_INIT_FUNCTION init;
  // name of the keyboard
  char name[20];
  // ptr to the next virtual keyboard layer
  struct keyboard *next;
};

void keyboard_init();

void keyboard_push(char c);

void keyboard_backspace(struct process *process);

char keyboard_pop();

int keyboard_insert(struct keyboard *keyboard);

#endif // !KEYBOARD_H
