section .asm

global idt_load


; macro definition wrapper
%macro interrupt_wrapper 1
  cli                       ; clear the interrupts
    pushad                  ; push the general purpose registers to stack
      call %1               ; call the fnction
    popad                   ; pop the general purpose registers from the stack
  sti                       ; set the interrupts
  iret                      ; return from the interrupts
%endmacro

; from c files 
extern int_null_fn
extern int_0_fn
extern int_21h_fn
extern isr80h_handler

; for c files
global int_null_handler
global int_0_handler
global int_21h_handler
global enable_interrupts
global disable_interrupts
global isr80h_wrapper


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

; Kernel interrupts
; I am not using the wrapper because it has having a slighlty different structrue and I'm lazy to modify the wrapper
isr80h_wrapper:
  ; interrupt frame start
  ; already pushed to us by the processor upon entry of interrupt
  ; uint32_t ip     [instruction ptr]
  ; uint32_t cs     [code segment]
  ; uint32_t flags  [flags register]
  ; uint32_t sp     [stack ptr]
  ; uint32_t ss     [stack segment]

  pushad            ; pushes the general purpose registers to the sack

  ; interrupt frame end

  push esp                 ; push the stack pointer so that we are pointing to the interrupt frame

  push eax                 ; push the command code of the kernel command 

  call isr80h_handler
  mov dword[tmp_res], eax  ; the interrupt retuns on the eax register similar to c func

  add esp, 8               ; for ignoring the push esp and push eax

  popad                    ; restore general purpose registers
  mov eax, [tmp_res]       ; pass the output of the interrupt to the caller
  iretd


section .data
tmp_res: dd 0           ; ptr to store eax value 
  

  


