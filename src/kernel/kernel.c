#include "kernel.h"
#include "../config.h"
#include "../disk/disk.h"
#include "../fs/pparser.h"
#include "../gdt/gdt.h"
#include "../idt/idt.h"
#include "../io/io.h"
#include "../isr80h/isr80h.h"
#include "../keyboard/keyboard.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../string/string.h"
#include "../task/process.h"
#include "../task/task.h"
#include "../task/tss.h"
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
 * Handle the backspace character while printing to the terminal
 */

void terminal_backspace() {

  if (terminal_row == 0 && terminal_col == 0) {
    return;
  }

  if (terminal_col == 0) {
    terminal_row -= 1;
    terminal_col = VGA_WIDTH;
  }

  terminal_col -= 1;
  terminal_putchar(terminal_col, terminal_row, ' ', 15);
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

  // backspace
  if (c == 0x08) {
    terminal_backspace();
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

/*
 * function that keeps the kernel in halt state and prints the passed fn
 */
void panic(const char *msg) {
  print(msg);
  while (1) {
    // do nothing
  }
}

struct tss tss;
// actual gdt format
struct gdt gdt_real[LIEBE_OS_TOTAL_GDT_SEGMENTS];

// gdt declartion for ease of life
struct gdt_structured gdt_structured[LIEBE_OS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},       // NULL SEGMENT
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a}, // KERNEL CODE SEG
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, // KERNEL DATA SEG

    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8}, // USER CODE SEG
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, // USER DATA SEG

    {.base = 0x00, .limit = sizeof(tss), .type = 0xE9}, //  TSS SEG
};

// switches the current page to kernel page
void kernel_page() {
  kernel_registers();
  page_switch((uintptr_t *)page_directory_obj->directory_entry);
}

void kernel_main() {

  // init the address o tss on the segment
  gdt_structured[5].base = (uintptr_t)&tss;

  // init the gdt segments
  // init_gdt();

  // clear terminal
  terminal_initialize();

  memset(gdt_real, 0x00, sizeof(gdt_real));

  gdt_structured_to_gdt(gdt_real, gdt_structured, LIEBE_OS_TOTAL_GDT_SEGMENTS);

  // load the gdt
  gdt_load(gdt_real, sizeof(gdt_real));

  // initalize heap memory
  kheap_init();

  // initialize the filesystems
  fs_init();

  // search and initalize the disks
  disk_search_and_init();

  // initialize interrupts
  idt_init();

  // setup the tss
  memset(&tss, 0x00, sizeof(tss));
  tss.esp0 = 0x600000;
  tss.ss0 = KERNEL_DATA_SELECTOR;

  // load the tss
  tss_load(0x28);

  // create page directory
  page_directory_obj = create_new_page_directory(
      PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

  page_switch(get_page_directory_entry(page_directory_obj));

  enable_paging();

  isr80h_register_commands();

  keyboard_init();

  // enable interrupts
  //  enable_interrupts();
  struct process *process = 0;
  int res = process_load_switch("0:/shell.elf", &process);
  if (res != 0) {
    panic("Failed to load the shell! Panic");
  }

  task_run_first_ever_task();

  while (1) {
  }
}
