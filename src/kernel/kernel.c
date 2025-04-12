#include "kernel.h"
#include "../disk/disk.h"
#include "../fs/pparser.h"
#include "../idt/idt.h"
#include "../io/io.h"
#include "../memory/heap/kheap.h"
#include "../memory/paging/paging.h"
#include "../string/string.h"
#include <stddef.h>
#include <stdint.h>

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;
static struct page_directory *page_directory_obj = 0;

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

void print(const char *str) {
  size_t len = strlen(str);
  for (int i = 0; i < len; i++) {
    terminal_writechar(str[i], 15);
  }
}

void kernel_main() {
  // clear terminal
  terminal_initialize();

  // print stuff for debugging
  print("test1\ntest2\ntest3");

  // initalize heap memory
  kheap_init();

  // initialize the filesystems
  fs_init();

  // search and initalize the disks
  disk_search_and_init();

  // initialize interrupts
  idt_init();

  // create page directory
  page_directory_obj = create_new_page_directory(
      PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

  page_switch(get_page_directory_entry(page_directory_obj));

  enable_paging();

  // enable interrupts
  enable_interrupts();

  struct path_root *path = pathparser_parse("0:/usr/bin/summa.out", NULL);

  if (path) {
    // do nothing
  }

  int fd = fopen("0:/folder", "r");
  if (!fd) {
    print("\n failed to load the text");
  } else {
    print("\n loaded successfully");
  }
  while (1) {
  }
}
