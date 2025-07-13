[BITS 32]

section .asm

global print:function
global liebeos_getkey:function
global liebeos_malloc:function
global liebeos_free:function
global liebeos_putchar:function
global liebeos_process_load_start:function
global liebeos_process_get_arguments:function
global liebeos_system:function
global liebeos_exit:function

; void print(const char * <string to be printed>)
print:
  push ebp
  mov ebp, esp
  push dword[ebp+8] ; pass the poiter 
  mov eax,1  ; command to print
  int 0x80
  add esp, 4 ; restore the stack
  pop ebp
  ret


; int liebeos_getkey()
liebeos_getkey:
  push ebp
  mov ebp, esp
  mov eax, 2 ; Command getkey
  int 0x80
  pop ebp
  ret


; void liebeos_putchar(char c)
liebeos_putchar:
  push ebp
  mov ebp, esp
  mov eax, 3 ; Command putchar
  push dword [ebp+8] ; variable 'c' ; 4 for stack frame and 4 for return of the caller
  int 0x80
  add esp, 4
  pop ebp
  ret

; void* liebeos_malloc(size_t size)
liebeos_malloc:
  push ebp
  mov ebp, esp
  mov eax, 4 ; Command malloc (Allocates memory for the process)
  push dword[ebp+8] ; Variable "size"
  int 0x80
  add esp, 4
  pop ebp
  ret

; void liebeos_free(void* ptr)
liebeos_free:
  push ebp
  mov ebp, esp
  mov eax, 5 ; Command 5 free (Frees the allocated memory for this process)
  push dword[ebp+8] ; Variable "ptr"
  int 0x80
  add esp, 4
  pop ebp
  ret

; void liebeos_process_load_start(const char* filename)
liebeos_process_load_start:
  push ebp
  mov ebp, esp
  mov eax, 6 ; Command 6 process load start ( stars a process )
  push dword[ebp+8] ; Variable "filename"
  int 0x80
  add esp, 4
  pop ebp
  ret

; void liebeos_process_get_arguments(struct process_arguments* arguments)
liebeos_process_get_arguments:
  push ebp
  mov ebp, esp
  mov eax, 8 ; Command 8 Gets the process arguments
  push dword[ebp+8] ; Variable arguments
  int 0x80
  add esp, 4
  pop ebp
  ret


; int liebeos_system(struct command_argument* arguments)
liebeos_system:
  push ebp
  mov ebp, esp
  mov eax, 7 ; Command 7 process_system ( runs a system command based on the arguments)
  push dword[ebp+8] ; Variable "arguments"
  int 0x80
  add esp, 4
  pop ebp
  ret

; void liebeos_exit()
liebeos_exit:
  push ebp
  mov ebp, esp
  mov eax, 9 ; Command 9 process exit
  int 0x80
  pop ebp
  ret
