#ifndef FILE_H
#define FILE_H

#include "./pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum { SEEK_SET, SEEK_CUR, SEEK_END };

typedef unsigned int FILE_MODE;
enum { FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID };

struct disk;

typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, struct path_part *path,
                                  FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk *disk);

struct filesystem {
  // filesystem should return 0 from resolve if the provided disk matches the fs
  FS_RESOLVE_FUNCTION resolve;
  FS_OPEN_FUNCTION open;

  // name for the fs
  char name[20];
};

struct file_descriptor {
  // descriptor index
  int index;
  // actual file sytem ref
  struct filesystem *filesystem;

  // private data for internal file descriptor
  void *pvt;

  // the disk that file descriptor should be used
  struct disk *disk;
};

// initailize the filesystem
void fs_init();
int fopen(const char *filename, const char *mode);
// insert a fs into fielsystem array
void fs_insert_filesystem(struct filesystem *filesystem);

struct filesystem *fs_resolve(struct disk *disk);

#endif // !FILE_H
