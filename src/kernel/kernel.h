#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20
void kernel_main();

void print(const char *str);

void panic(const char *msg);

// switches the page to kernel page
void kernel_page();
// handles the kernel segment registers
void kernel_registers();

void terminal_writechar(char c, char color);
#endif // !KERNEL_H
