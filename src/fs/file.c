#include "./file.h"
#include "../config.h"
#include "../disk/disk.h"
#include "../kernel/kernel.h"
#include "../macros.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../status.h"
#include "../string/string.h"
#include "./fat/fat16.h"
#include "pparser.h"

struct filesystem *filesystems[LIEBE_OS_MAX_FILESYSTEMS];
struct file_descriptor *file_descriptors[LIEBE_OS_MAX_FILE_DESCRIPTORS];

// returns the index of empty filesystem
static struct filesystem **fs_get_free_filesystem() {
  for (int i = 0; i < LIEBE_OS_MAX_FILESYSTEMS; i++) {
    if (filesystems[i] == 0)
      return &filesystems[i];
  }
  return 0;
}

// add the ref of the fs passed into filesystem array
void fs_insert_filesystem(struct filesystem *filesystem) {
  struct filesystem **fs;
  // get free index on the filesystem arr
  fs = fs_get_free_filesystem();
  if (!fs) {
    print("Failed to insert fs");
    while (1) {
      // failed
    }
  }
  *fs = filesystem;
}

static void fs_static_load() {
  // init fat16 file system and insert into vfs
  fs_insert_filesystem(fat16_init());
}

void fs_load() {
  // clean the filesystem array mem
  memset(filesystems, 0, sizeof(filesystems));
  // load default filesystems
  fs_static_load();
}

// init file descripotr arrary
void fs_init() {
  memset(file_descriptors, 0, sizeof(file_descriptors));
  fs_load();
}

// creates a descriptor
static int file_new_decriptor(struct file_descriptor **desc_out) {
  int res = -ENOMEM;
  for (int i = 0; i < LIEBE_OS_MAX_FILE_DESCRIPTORS; i++) {
    if (file_descriptors[i] == 0) {
      struct file_descriptor *desc = kzalloc(sizeof(struct file_descriptor));
      // descriptor idx start at 1
      desc->index = i + 1;
      file_descriptors[i] = desc;
      *desc_out = desc;
      res = 0;
      break;
    }
  }
  return res;
}

// returns the file descriptor based on passed index
static struct file_descriptor *file_get_descriptor(int fd) {
  if (fd <= 0 || fd >= LIEBE_OS_MAX_FILE_DESCRIPTORS) {
    return 0;
  }
  // Descriptors start at 1
  int index = fd - 1;
  return file_descriptors[index];
}

struct filesystem *fs_resolve(struct disk *disk) {
  struct filesystem *fs = 0;
  // iterate through the fs array and call the fs resolve fn ptr
  for (int i = 0; i < LIEBE_OS_MAX_FILESYSTEMS; i++) {
    // if the fs resolve returns 0 then it is a match
    if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
      fs = filesystems[i];
      break;
    }
  }
  return fs;
}

FILE_MODE file_get_mode_by_string(const char *str) {
  FILE_MODE mode = FILE_MODE_INVALID;

  if (strncmp(str, "r", 1) == 0) {
    mode = FILE_MODE_READ;
  }

  if (strncmp(str, "w", 1) == 0) {
    mode = FILE_MODE_WRITE;
  }

  if (strncmp(str, "a", 1) == 0) {
    mode = FILE_MODE_APPEND;
  }

  return mode;
}

int fopen(const char *filename, const char *mode_str) {
  int res = 0;

  struct path_root *root_node = pathparser_parse(filename, NULL);
  if (!root_node) {
    // invalid path
    res = -EINVARG;
    goto exit_fn;
  }

  if (!root_node->first) {
    // does not contain filename/directory apart from disk no
    res = -EINVARG;
    goto exit_fn;
  }

  struct disk *disk_obj = disk_get(root_node->drive_no);
  if (!disk_obj) {
    // can't find the disk
    res = -EIO;
    goto exit_fn;
  }

  if (!disk_obj->filesystem) {
    // filesystem missing for the disk
    res = -EIO;
    goto exit_fn;
  }

  FILE_MODE mode = file_get_mode_by_string(mode_str);

  if (mode == FILE_MODE_INVALID) {
    // passed invlaid fopen mode
    res = -EINVARG;
    goto exit_fn;
  }

  void *descriptor_private_data =
      disk_obj->filesystem->open(disk_obj, root_node->first, mode);

  if (ISERR(descriptor_private_data)) {
    res = ERROR_I(descriptor_private_data);
    goto exit_fn;
  }

  struct file_descriptor *desc = 0;

  res = file_new_decriptor(&desc);

  if (ISERR_I(res)) {
    // cant make a descriptor
    res = -EIO;
    goto exit_fn;
  }

  desc->filesystem = disk_obj->filesystem;
  desc->pvt = descriptor_private_data;
  desc->disk = disk_obj;
  res = desc->index;

exit_fn:
  if (ISERR_I(res))
    res = 0;
  return res;
}
