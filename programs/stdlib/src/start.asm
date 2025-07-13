[BITS 32]


global _start
extern main_wrapper
extern liebeos_exit

section .asm

_start:
    call main_wrapper
    call liebeos_exit
    ret
