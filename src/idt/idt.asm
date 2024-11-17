section .asm

global idt_load


; macro definition wrapper
%macro interrupt_wrapper 1
  cli
    pushad
      call %1
    popad
  sti
  iret
%endmacro

; from c files 
extern int_null_fn
extern int_0_fn
extern int_21h_fn

; for c files
global int_null_handler
global int_0_handler
global int_21h_handler
global enable_interrupts
global disable_interrupts


idt_load:
  push ebp
  mov ebp, esp
  mov ebx, [ebp+8]

  lidt [ebx]

  pop ebp
  ret

enable_interrupts:
  sti
  ret


disable_interrupts:
  cli
  ret

; individual wrapped label definition for each macros
; mostly for div by 0
int_0_handler:
  interrupt_wrapper int_0_fn


; no interrupt / null interrupts
int_null_handler:
  interrupt_wrapper int_null_fn

; keybaord
int_21h_handler:
  interrupt_wrapper int_21h_fn




