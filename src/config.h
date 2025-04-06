#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

#define LIEBE_OS_TOTAL_INTERRUPTS 512

// heap

#define LIEBE_OS_HEAP_SIZE_BYTES 104857600 // size of the heap memory
#define LIEBE_OS_HEAP_BLOCK_SIZE 4096      // block size
#define LIEBE_OS_HEAP_ADDRESS 0x01000000   // start address of the heap memory
#define LIEBE_OS_HEAP_TABLE_ADDRESS                                            \
  0x00007E00 // address of the table for heap status

// Disk
#define LIEBE_OS_SECTOR_SIZE 512

// FS
#define LIEBE_OS_MAX_PATH_LEN 108

#define LIEBE_OS_MAX_FILESYSTEMS 12
#define LIEBE_OS_MAX_FILE_DESCRIPTORS 512

#endif // !CONFIG_H
