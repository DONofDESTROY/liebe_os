# A toy os in the making
<img width="772" height="516" alt="image" src="https://github.com/user-attachments/assets/88723786-2308-4c11-99ac-b42f87fc6bfa" />

⚠️Note: I am learning about this, this may contain wrong information. ⚠️

⚠️Note: Not tested on real hardware use it on your own risk ⚠️

# Plan to add these
- [X] ~Memory management.~
- [X] ~file system~(still have write).
- [X] ~process management.~
- [ ] GUI(Maybe)
- [X] ~shell~(Base)
- [ ] fswrite

# DEBUGGING
run gbd
### and run these command to add symbols
add-symbol-file ./build/kernelfull.o 0x100000
### for debugging user programs 
break *0x400000
### run qemu and attach gdb
target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio


# build
0. Make the scripts in the scripts folder as executable 
1. need a cross compiler (without std libs and os specific interrupts), there is a script on scripts folder for arch linux run it!
2. run build.sh
# run 
run ` make run ` on the root of the repo to run through qemu.

## some scribbles that I drew to understand this,
⚠️Note: these logic might not be used in the code or updated its just for understanding the process ⚠️
![image](https://github.com/user-attachments/assets/30e8c316-8652-4c3a-966f-3ad3f4920f23)
![image](https://github.com/user-attachments/assets/133aceda-8dec-44b2-aeb6-c8d70412998e)
![image](https://github.com/user-attachments/assets/34ba65ff-f23e-42c0-a48b-0be627c312cd)
![image](https://github.com/user-attachments/assets/f4d41a09-b167-4127-aad5-80178e15da96)
![image](https://github.com/user-attachments/assets/2b51df3a-2431-4abf-b533-af5c8c79feee)
![image](https://github.com/user-attachments/assets/b5527996-0592-4b58-ad60-78faf978f62f)
![image](https://github.com/user-attachments/assets/e5a2c860-e25d-472d-b619-d7ce9b057faa)

# Resources used for learning
- https://www.youtube.com/watch?v=MwPjvJ9ulSc&list=PLm3B56ql_akNcvH8vvJRYOc7TbYhRs19M
- https://wiki.osdev.org/Expanded_Main_Page
- https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/
