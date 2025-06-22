[BITS 32]

section .asm

global _start

_start:

label:
  push 20
  push 30
  mov eax, 0; command 0 sum
  int 0x80
  add esp, 8 ; adjust the stack ptr to reset the pushed values
  ; alternative way
  ; pop eax
  ; pop eax ; pop 2 times since pushed 2 times
  push message
  mov eax, 1 ; print command
  int 0x80   ; call the interrupt for
  add esp, 4 ; reset the stack


  jmp $

section .data
message: db 'From the blank',0
