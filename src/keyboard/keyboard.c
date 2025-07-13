#include "keyboard.h"
#include "../kernel/kernel.h"
#include "../status.h"
#include "classic.h"

static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_last = 0;

void keyboard_init() { keyboard_insert(classic_init()); }

int keyboard_insert(struct keyboard *keyboard) {
  int res = 0;
  if (keyboard->init == 0) {
    // there is no init fn ptr hence exiting
    res = -EINVARG;
    goto exit_fn;
  }

  // there exist an last keyboard then make the current keyboard as last
  // ie insert into linked list as short
  if (keyboard_list_last) {
    keyboard_list_last->next = keyboard;
    keyboard_list_last = keyboard;
  } else {
    keyboard_list_head = keyboard;
    keyboard_list_last = keyboard;
  }
  res = keyboard->init();

exit_fn:
  return res;
}

static int keyboard_get_tail_index(struct process *process) {
  // returns the item wrapped within th buffer size
  return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

void keyboard_push(char c) {
  // pushes the char into the process keyboard buffer
  struct process *process_obj = process_current();
  if (!process_obj) {
    return;
  }

  if (c == 0) {
    // so far no null characters
    return;
  }

  int real_index = keyboard_get_tail_index(process_obj);
  process_obj->keyboard.buffer[real_index] = c;
  process_obj->keyboard.tail++;
}

void keyboard_backspace(struct process *process_obj) {
  // handle backspace key on the buffer
  process_obj->keyboard.tail -= 1;
  int real_index = keyboard_get_tail_index(process_obj);
  process_obj->keyboard.buffer[real_index] = 0x00;
}

char keyboard_pop() {
  if (!task_current()) {
    return 0;
  }

  struct process *process = task_current()->process;
  int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
  char c = process->keyboard.buffer[real_index];
  if (c == 0x00) {
    // Nothing to pop return zero.
    return 0;
  }

  process->keyboard.buffer[real_index] = 0;
  process->keyboard.head++;
  return c;
}

void keyboard_set_capslock(struct keyboard *keyboard,
                           KEYBOARD_CAPS_LOCK_STATE state) {
  keyboard->capslock_state = state;
}

KEYBOARD_CAPS_LOCK_STATE keyboard_get_capslock(struct keyboard *keyboard) {
  return keyboard->capslock_state;
}
