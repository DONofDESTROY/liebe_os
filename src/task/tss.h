#ifndef TASK_SWITCH_SEGMENT_H
#define TASK_SWITCH_SEGMENT_H

#include <stdint.h>

// refer https://wiki.osdev.org/Task_State_Segment for more details
struct tss {
  uint32_t link;   // link to previous task
  uint32_t esp0;   // level -0 stack ptr
  uint32_t ss0;    // level -0 stack segment
  uint32_t esp1;   // level -1 stack ptr
  uint32_t ss1;    // level -1 stack seg
  uint32_t esp2;   // level -2 stack ptr
  uint32_t ss2;    // level -2 stack seg
  uint32_t cr3;    // cr3 register
  uint32_t eip;    // instruction ptr
  uint32_t eflags; // flags register
  uint32_t eax;    // eax register
  uint32_t ecx;    // ebx register
  uint32_t edx;    // edx register
  uint32_t ebx;    // ebx register
  uint32_t esp;    // actual stack ptr
  uint32_t ebp;    // actual base ptr
  uint32_t esi;    // esi register
  uint32_t edi;    // edi register

  // segment registers
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  // end of segment registers

  uint32_t ldtr;
  uint32_t iopb;
} __attribute__((packed));

void tss_load(int tss_segment);

#endif // !TASK_SWITCH_SEGMENT_H
