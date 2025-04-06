#ifndef DISK_H
#define DISK_H

#include "../fs/file.h"

typedef unsigned int LIEBE_OS_DISK_TYPE;

#define LIEBE_OS_DISK_TYPE_REAL 0

#define LIEBE_OS_DEFAULT_DISK_ID 0

struct disk {
  LIEBE_OS_DISK_TYPE type;
  int sector_size;

  // unique id for the disk
  int id;

  struct filesystem *filesystem;

  // private data for fs
  void *fs_private;
};

void disk_search_and_init();
struct disk *disk_get(int index);
int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf);

#endif // !DISK_H
