#include "kernel.h"
#include "../idt/idt.h"
#include "../io/io.h"
#include "../memory/heap/kheap.h"
#include <stddef.h>
#include <stdint.h>

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

/*
 * creates a 16 bit int with char at first half and color on second half
 */
uint16_t terminal_make_char(char c, char color) { return (color << 8) | c; }

/*
 * writes the char/text into video memory
 */
void terminal_putchar(int x, int y, char c, char color) {
  video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color);
}

/*
 * handles the x and y and write to the video mem
 */
void terminal_writechar(char c, char color) {
  if (c == '\n') {
    // handle new line char
    terminal_row += 1;
    terminal_col = 0;
    return;
  }

  // write the char into mem
  terminal_putchar(terminal_col, terminal_row, c, color);
  terminal_col += 1;
  if (terminal_col >= VGA_WIDTH) {
    // if overflow happens wrap
    terminal_col = 0;
    terminal_row += 1;
  }
}

/* initalizes the video memory and
 * clears the screen
 */
void terminal_initialize() {
  video_mem = (uint16_t *)(0xB8000);
  terminal_row = 0;
  terminal_col = 0;
  for (int y = 0; y < VGA_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      terminal_putchar(x, y, ' ', 0);
    }
  }
}

/*returns the size of string
 */
size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

void print(const char *str) {
  size_t len = strlen(str);
  for (int i = 0; i < len; i++) {
    terminal_writechar(str[i], 15);
  }
}

void kernel_main() {
  terminal_initialize();

  print("Hello world!\ntest");

  kheap_init();

  idt_init();

  enable_interrupts();
}
