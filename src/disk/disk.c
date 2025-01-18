#include "disk.h"
#include "../config.h"
#include "../io/io.h"
#include "../memory/memory.h"
#include "../status.h"

struct disk disk_obj;

// ATA driver simialr to the one on boot.asm expect in c
int disk_read_sector(int lba, int total, void *buff) {
  outb(0x1F6, (lba >> 24) | 0xE0);
  outb(0x1F2, total);
  outb(0x1F3, (unsigned char)(lba & 0XFF));
  outb(0X1F4, (unsigned char)(lba >> 8));
  outb(0X1F5, (unsigned char)(lba >> 16));
  outb(0X1F7, 0x20);

  // read 2 bytes at a time
  unsigned short *ptr = (unsigned short *)buff;
  for (int b = 0; b < total; b++) {

    do {
      // wait for the flag
    } while (!(insb(0X1F7) & 0x08));

    // copy from hard disk to memory
    for (int i = 0; i < 256; i++) {
      *ptr = insw(0X1F0);
      ptr++;
    }
  }

  return 0;
}

// initalize the disk obj
void disk_search_and_init() {
  memset(&disk_obj, 0, sizeof(disk_obj));
  disk_obj.type = LIEBE_OS_DISK_TYPE_REAL;
  disk_obj.sector_size = LIEBE_OS_SECTOR_SIZE;
}

// get disk obj
struct disk *disk_get(int index) {
  // for the time being only disk 0 is allowed
  if (index != 0) {
    return 0;
  }
  return &disk_obj;
}

// fn to read from the disk
int disk_read_block(struct disk *idisk, unsigned int lba, int total,
                    void *buff) {
  if (idisk != &disk_obj) {
    return -EIO;
  }
  return disk_read_sector(lba, total, buff);
}
