#include "fat16.h"
#include "../../disk/disk.h"
#include "../../disk/streamer.h"
#include "../../memory/heap/kheap.h"
#include "../../memory/memory.h"
#include "../../status.h"
#include "../../string/string.h"
#include "../file.h"

int fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode);

struct filesystem fat16_fs = {.resolve = fat16_resolve, .open = fat16_open};

struct filesystem *fat16_init() {
  // copy the name into fat16 struct
  strcpy(fat16_fs.name, "FAT16");
  // return the add of the fat16_fs to push into the arr
  return &fat16_fs;
}

// initalize the default private data
static void fat16_init_private(struct disk *disk,
                               struct fat_private *private_obj) {
  // make the memory as 0 as default
  memset(private_obj, 0, sizeof(struct fat_private));
  private_obj->cluster_read_stream = get_new_disk_stream(disk->id);
  private_obj->fat_read_stream = get_new_disk_stream(disk->id);
  private_obj->directory_stream = get_new_disk_stream(disk->id);
}

int fat16_sector_to_absolute(struct disk *disk, int sector) {
  return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct disk *disk,
                                        uint32_t directory_start_sector) {

  struct fat_directory_item item;
  struct fat_directory_item empty_item;

  memset(&empty_item, 0, sizeof(empty_item));

  struct fat_private *fat_private_obj = disk->fs_private;

  int res = 0;
  int count = 0;
  int directory_start_pos = directory_start_sector * disk->sector_size;
  struct disk_stream *stream = fat_private_obj->directory_stream;
  // set the streammer to starting of the directroy passed
  if (diskstreamer_seek(stream, directory_start_pos) != OK) {
    res = -EIO;
    goto exit_fn;
  }
  while (1) {
    // read items one by one from the start pos
    if (diskstreamer_read(stream, &item, sizeof(item)) != OK) {
      res = -EIO;
      goto exit_fn;
    }

    // empty item ie end of the directrory found
    if (item.filename[0] == 0x00) {
      break;
    }

    // item is unsued so dont count it
    if (item.filename[0] == 0xE5) {
      continue;
    }
    count++;
  }

  res = count;

exit_fn:
  return res;
}

int fat16_get_root_directory(struct disk *disk,
                             struct fat_private *fat_private_obj,
                             struct fat_directory *directory) {
  int res = 0;

  struct fat_header *primary_header = &fat_private_obj->header.primary_header;

  // pos in the disk
  int root_direcotry_pos =
      (primary_header->fat_copies * primary_header->sectors_per_fat) +
      primary_header->reserved_sectors;
  // no of directory entnries
  int root_dir_entries =
      fat_private_obj->header.primary_header.root_dir_entries;
  // size of the directory
  int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
  // sector size
  int total_sectors = root_dir_size / disk->sector_size;

  // ceiling
  if (root_dir_size % disk->sector_size) {
    total_sectors += 1;
  }

  int total_items =
      fat16_get_total_items_for_directory(disk, root_direcotry_pos);

  struct fat_directory_item *dir = kzalloc(root_dir_size);

  if (!dir) {
    res = -ENOMEM;
    goto exit_fn;
  }

  struct disk_stream *stream = fat_private_obj->directory_stream;
  if (diskstreamer_seek(
          stream, fat16_sector_to_absolute(disk, root_direcotry_pos)) != OK) {
    res = -EIO;
    goto exit_fn;
  }

  if (diskstreamer_read(stream, dir, root_dir_size) != OK) {
    res = -EIO;
    goto exit_fn;
  }

  // set the directory in priavte meta
  directory->item = dir;
  directory->total = total_items;
  directory->sector_pos = root_direcotry_pos;
  directory->ending_sector_pos =
      root_direcotry_pos + (root_dir_size / disk->sector_size);

exit_fn:
  return res;
}

int fat16_resolve(struct disk *disk) {

  int res = 0;
  struct fat_private *fat_private_obj = kzalloc(sizeof(struct fat_private));
  // init some default data
  fat16_init_private(disk, fat_private_obj);

  // bind the fat16 meta to disk
  disk->fs_private = fat_private_obj;
  disk->filesystem = &fat16_fs;

  struct disk_stream *stream = get_new_disk_stream(disk->id);
  if (!stream) {
    // failed to create a stream
    res = -ENOMEM;
    goto exit_fn;
  }

  if (diskstreamer_read(stream, &fat_private_obj->header,
                        sizeof(fat_private_obj->header)) != OK) {
    // failed to read the fat header
    res = -EIO;
    goto exit_fn;
  }

  if (fat_private_obj->header.shared.extended_header.signature !=
      FAT16_SIGNATURE) {
    res = -EFSNOTUS;
    goto exit_fn;
  }

  if (fat16_get_root_directory(disk, fat_private_obj,
                               &fat_private_obj->root_directory) != OK) {
    // failed to read root directory
    res = -EIO;
    goto exit_fn;
  }

exit_fn:
  return res;
}

void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode) {
  return 0;
}
