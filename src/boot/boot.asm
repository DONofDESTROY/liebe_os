; simple bootloader boots and swithes to 32 bit protected mode and loads kernel
ORG 0x7C00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; FAT 16 file system essentials
; jmp to start and ignore the fat16 header 
; refer https://wiki.osdev.org/FAT for more details
jmp short start
nop

; FAT16 Header
OEMIdentifier         db 'LIEBEOS '        ; should be 8 bytes in size
BytesPerSector        dw 0x200             ; no of bytes per sector
SectorsPerCluster     db 0x80              ; no of sectors per cluster 
ReservedSectors       dw 0x200             ; where kernel is stored dont change for file
FATCopies             db 0x02              ; 2 FAT tables, 1 original 1 copy
RootDirEntries        dw 0x40              ; how many folders/files in root dir
NumSectors            dw 0x00              ; total sectors ie > 65535 sectors in volume
MediaType             db 0xF8              ; media descriptor type 0xf8 = fixed disk
SectorsPerFat         dw 0x100             ; no of sectors per fat
; chs stuff
SectorsPerTrack       dw 0x20              ; no of sectors per track
NumberOfHeads         dw 0x40              ; no of heads in hdd
HiddenSectors         dd 0x00              ; no of hidden sectors (LBA addressing)
; end chs stuff
SectorsBig            dd 0x773594          ; total sectors if > 65535

; Extended BPB (DOS 4.0)
DriveNumber           db 0x80              ; 0x80 for hdd
WinNTBit              db 0x00              ; flags for windows nt
Signature             db 0x29              ; signature must be 0x28 or 0x29
VolumeId              dd 0xD105            ; id used  for tracking volumnes in computer
VolumeIDString        db 'LIEBE BOOT '     ; label for volume 11 bytes long
SystemIDString        db 'FAT16   '        ; fat type




start:
  jmp 0:step2 

step2:
   ; disable interrupts on changing the seg registers
    cli                            ; clear interrupts
    mov ax,0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti                           ; enable intterupts

.load_protected:
    cli
    ; load gdt
    lgdt[gdt_descriptor]
    ; switch to 32 bit protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax    
    ; far jump (jump to other segment)
    jmp CODE_SEG:load32

; GDT
gdt_start:
gdt_null: ; should start with  empty descriptor
    dd 0x0
    dd 0x0
; offset 0x8
gdt_code:                       ; CS SHOULD POINT TO THIS
    dw 0xffff                   ; Segment limit first 0-15 bits
    dw 0                        ; Base first 0-15 bits
    db 0                        ; Base 16-23 bits
    db 0x9a ; 10011010          ; Access byte   
    db 11001111b                ; High 4 bit flags and the low 4 bit flags
    db 0                        ; Base 24-31 bits
; offset 0x10
gdt_data:                       ; DS, SS, ES, FS, GS
    dw 0xffff                   ; Segment limit first 0-15 bits
    dw 0                        ; Base first 0-15 bits
    db 0                        ; Base 16-23 bits
    db 0x92 ; 1001010           ; Access byte
    db 11001111b                ; High 4 bit flags and the low 4 bit flags
    db 0                        ; Base 24-31 bits
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start-1
    dd gdt_start

; dummy driver to load the kernel
[BITS 32]
 load32:
    ; load further sectors into 0x0100000
    mov eax, 1              ; starting sector to load
    mov ecx, 100            ; total number of sectors
    mov edi, 0x0100000      ; load into
    call ata_lba_read

    ; jmp to kernel.asm which is 2nd sector
    jmp CODE_SEG:0x0100000

ata_lba_read:
    ; refer 28bit pio on  https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
    mov ebx, eax, ; Backup the LBA
    ; Send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; Select the  master drive
    mov dx, 0x1F6
    out dx, al
    ; Finished sending the highest 8 bits of the lba
    ; Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read
    ; Send more bits of the LBA
    mov eax, ebx ; Restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA
    ; Send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx ; Restore the backup LBA
    shr eax, 8
    out dx, al
    ; Finished sending more bits of the LBA
    ; Send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; Restore the backup LBA
    shr eax, 16
    out dx, al
    ; Finished sending upper 16 bits of the LBA
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to read
.try_again:
    ; waiting/polling he status
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

; We need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; End of reading sectors into memory

    ret
 
times 510-($ - $$) db 0 ; nasm way of saying fill the rest of 510 bytes with 0
; boot signature
dw 0xaa55

