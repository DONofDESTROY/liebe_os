[BITS 32]
global _start
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
    
    ; call c code
    call kernel_main

    jmp $

times 510-($ - $$) db 0 ; nasm way of saying fill the rest of 510 bytes with 0

