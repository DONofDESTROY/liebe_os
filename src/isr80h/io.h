#ifndef ISR80H_IO_H
#define ISR80H_IO_H

#include "../idt/idt.h"

void *isr80h_command1_print(struct interrupt_frame *frame);

void *isr80h_command2_getkey(struct interrupt_frame *frame);

void *isr80h_commad3_putchar(struct interrupt_frame *frame);

#endif // !ISR80H_IO_H
