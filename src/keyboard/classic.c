#include "classic.h"
#include "../idt/idt.h"
#include "../io/io.h"
#include "../kernel/kernel.h"
#include "keyboard.h"
#include <stdint.h>

int classic_keyboard_init();
void classic_keyboard_handle_interrupt();

// idx to scan code
static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x1B, '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',
    '-',  '=',  0x08, '\t', 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
    'O',  'P',  '[',  ']',  0x0d, 0x00, 'A',  'S',  'D',  'F',  'G',  'H',
    'J',  'K',  'L',  ';',  '\'', '`',  0x00, '\\', 'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  ',',  '.',  '/',  0x00, '*',  0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '7',  '8',  '9',
    '-',  '4',  '5',  '6',  '+',  '1',  '2',  '3',  '0',  '.'};

struct keyboard classic_keyboard = {.name = {"Classic"},
                                    .init = classic_keyboard_init};

int classic_keyboard_init() {

  idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT,
                                  classic_keyboard_handle_interrupt);
  // command to enable first ps2 port
  outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
  return 0;
}

uint8_t classic_keyboard_scancode_to_char(uint8_t scancode) {
  size_t size_of_keyboard_set_one =
      sizeof(keyboard_scan_set_one) / sizeof(uint8_t);

  if (scancode > size_of_keyboard_set_one) {
    return 0;
  }
  char c = keyboard_scan_set_one[scancode];
  return c;
}

void classic_keyboard_handle_interrupt() {
  kernel_page();

  uint8_t scancode = 0;
  scancode = insb(KEYBOARD_INPUT_PORT);
  // ignore irq
  insb(KEYBOARD_INPUT_PORT);

  if (scancode & CLASSIC_KEYBOARD_KEY_RELEASED) {
    // ignore key relase for now
    return;
  }

  uint8_t c = classic_keyboard_scancode_to_char(scancode);
  if (c != 0) {
    keyboard_push(c);
  }

  task_page();
}

struct keyboard *classic_init() { return &classic_keyboard; }
