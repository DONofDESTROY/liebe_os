[BITS 32]
global _start
global kernel_registers
extern kernel_main


CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; enable a20 line
    in al, 0x92
    or al, 2
    out 0x92, al
    cli

    ; Remap the master PIC
    mov al, 00010001b ; Tell master pic
    out 0x20, al
    
    mov al, 0x20 ; Master ISR starting
    out 0x21, al

    mov al, 00000001b
    out 0x21, al
    ; end of remap of the master PIC
    
    ; call c code
    call kernel_main

    jmp $


kernel_registers:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov gs, ax
  mov fs, ax
  ret


times 512-($ - $$) db 0 ; nasm way of saying fill the rest of 510 bytes with 0

