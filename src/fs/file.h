#ifndef FILE_H
#define FILE_H

#include <stdint.h>

#include "./pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum { SEEK_SET, SEEK_CUR, SEEK_END };

typedef unsigned int FILE_MODE;
enum { FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID };

enum { FILE_STAT_READ_ONLY = 0b00000001 };
typedef unsigned int FILE_STAT_FLAGS;

struct disk;

struct file_stat {
  FILE_STAT_FLAGS flags;
  uint32_t filesize;
};

typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, struct path_part *path,
                                  FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk *disk);
typedef int (*FS_READ_FUNCTION)(struct disk *disk, void *pvt, uint32_t size,
                                uint32_t nmemb, char *out);

typedef int (*FS_SEEK_FUNCTION)(void *pvt, uint32_t offset,
                                FILE_SEEK_MODE seek_mode);

typedef int (*FS_STAT_FUNCTION)(struct disk *disk, void *pvt,
                                struct file_stat *stat);

typedef int (*FS_CLOSE_FUNCTION)(void *pvt);

struct filesystem {
  // filesystem should return 0 from resolve if the provided disk matches the fs
  FS_RESOLVE_FUNCTION resolve;
  FS_OPEN_FUNCTION open;
  FS_READ_FUNCTION read;
  FS_SEEK_FUNCTION seek;
  FS_STAT_FUNCTION stat;
  FS_CLOSE_FUNCTION close;

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
int fread(void *ptr, uint32_t size, uint32_t memb, int fd);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fstat(int fd, struct file_stat *stat);
int fclose(int fd);
// insert a fs into fielsystem array
void fs_insert_filesystem(struct filesystem *filesystem);

struct filesystem *fs_resolve(struct disk *disk);

#endif // !FILE_H
