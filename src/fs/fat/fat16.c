#include "fat16.h"
#include "../../config.h"
#include "../../disk/disk.h"
#include "../../disk/streamer.h"
#include "../../macros.h"
#include "../../memory/heap/kheap.h"
#include "../../memory/memory.h"
#include "../../status.h"
#include "../../string/string.h"
#include "../file.h"
#include "stdint.h"

int fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode);
int fat16_read(struct disk *disk, void *descriptor, uint32_t size,
               uint32_t nmemb, char *out_ptr);

int fat16_seek(void *pvt, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_stat(struct disk *disk, void *private, struct file_stat *stat);

int fat16_close(void *private);

struct filesystem fat16_fs = {.resolve = fat16_resolve,
                              .open = fat16_open,
                              .read = fat16_read,
                              .seek = fat16_seek,
                              .stat = fat16_stat,
                              .close = fat16_close};

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

void fat16_to_proper_string(char **out, const char *in) {
  // it should not be either null terminator or space (0x20)
  while (*in != 0x00 && *in != 0x20) {
    **out = *in;
    *out += 1;
    in += 1;
  }
  if (*in == 0x20) {
    **out = 0x00;
  }
}

void fat16_get_full_relative_filename(struct fat_directory_item *item,
                                      char *out, int max_len) {
  memset(out, 0x00, max_len);
  char *out_tmp = out;
  fat16_to_proper_string(&out_tmp, (const char *)item->filename);
  if (item->ext[0] != 0x00 && item->ext[0] != 0x20) {
    *out_tmp++ = '.';
    fat16_to_proper_string(&out_tmp, (const char *)item->ext);
  }
}

struct fat_directory_item *
fat16_clone_direcotry_item(struct fat_directory_item *item) {
  // copy function for file
  int size = sizeof(struct fat_directory_item);
  struct fat_directory_item *dir_item_copy = kzalloc(size);
  if (!dir_item_copy) {
    goto exit_fn;
  }

  memcpy(dir_item_copy, item, size);

exit_fn:
  return dir_item_copy;
}

// positionn of passed cluster on data sector
static int fat16_cluster_to_sector(struct fat_private *private, int cluster) {
  return private->root_directory.ending_sector_pos +
         ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item *item) {
  return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
}

static uint32_t fat16_get_first_fat_sector(struct fat_private *private) {
  return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk *disk, int cluster) {
  int res = -1;
  struct fat_private *private = disk->fs_private;
  struct disk_stream *stream = private->fat_read_stream;
  if (!stream) {
    goto exit_fn;
  }

  uint32_t fat_table_position =
      fat16_get_first_fat_sector(private) * disk->sector_size;
  res = diskstreamer_seek(stream, fat_table_position +
                                      (cluster * LIEBE_OS_FAT16_ENTRY_SIZE));
  if (res < 0) {
    goto exit_fn;
  }

  uint16_t result = 0;
  res = diskstreamer_read(stream, &result, sizeof(result));

  if (res < 0) {
    goto exit_fn;
  }

  res = result;

exit_fn:
  return res;
}

static int fat16_get_cluster_for_offset(struct disk *disk, int starting_cluster,
                                        int offset) {

  int res = 0;
  struct fat_private *private = disk->fs_private;
  int size_of_cluster_bytes =
      private->header.primary_header.sectors_per_cluster * disk->sector_size;
  int cluster_to_use = starting_cluster;
  int cluster_ahead = offset / size_of_cluster_bytes;
  for (int i = 0; i < cluster_ahead; i++) {
    int entry = fat16_get_fat_entry(disk, cluster_to_use);
    if (entry == 0xFF8 || entry == 0xFFF) {
      res = -EIO;
      goto exit_fn;
    }

    if (entry == LIEBE_OS_FAT16_BAD_SECTOR) {
      res = -EIO;
      goto exit_fn;
    }

    if (entry == 0xFF0 || entry == 0XFF6) {
      res = -EIO;
      goto exit_fn;
    }

    if (entry == 0x00) {
      res = -EIO;
      goto exit_fn;
    }

    cluster_to_use = entry;
  }

  res = cluster_to_use;
exit_fn:
  return res;
}

void fat16_free_directory(struct fat_directory *directroy) {
  if (!directroy) {
    return;
  }
  if (directroy->item) {
    kfree(directroy->item);
  }
  kfree(directroy);
}

static int fat16_read_internal_from_stream(struct disk *disk,
                                           struct disk_stream *stream,
                                           int cluster, int offset, int total,
                                           void *out) {
  int res = 0;
  struct fat_private *private = disk->fs_private;
  int size_of_cluster_bytes =
      private->header.primary_header.sectors_per_cluster * disk->sector_size;
  int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
  if (cluster_to_use < 0) {
    res = cluster_to_use;
    goto exit_fn;
  }

  int offset_from_cluster = offset % size_of_cluster_bytes;
  int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
  int starting_pos =
      (starting_sector * disk->sector_size) + offset_from_cluster;
  int total_to_read =
      total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
  res = diskstreamer_seek(stream, starting_pos);
  if (res != 0) {
    goto exit_fn;
  }

  res = diskstreamer_read(stream, out, total_to_read);
  if (res != OK) {
    goto exit_fn;
  }

  total -= total_to_read;
  if (total > 0) {
    res = fat16_read_internal_from_stream(disk, stream, cluster,
                                          offset + total_to_read, total,
                                          out + total_to_read);
  }

exit_fn:
  return res;
}

static int fat16_read_internal(struct disk *disk, int starting_cluster,
                               int offset, int total, void *out) {
  struct fat_private *fs_private = disk->fs_private;
  struct disk_stream *stream = fs_private->cluster_read_stream;
  return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset,
                                         total, out);
}

struct fat_directory *
fat16_load_fat_directory(struct disk *disk, struct fat_directory_item *item) {

  int res = 0;
  struct fat_directory *directory = 0;
  struct fat_private *fat_private = disk->fs_private;
  if (!(item->attribute & FAT_FILE_SUBDIRECTORY)) {
    // if the function somehow got called with file return
    res = -EINVARG;
    goto exit_fn;
  }

  directory = kzalloc(sizeof(struct fat_directory));
  if (!directory) {
    // no memory
    res = -ENOMEM;
    goto exit_fn;
  }

  // find the sub directory in the fat table
  int cluster = fat16_get_first_cluster(item);
  int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
  // find the total no items for the directory
  int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
  directory->total = total_items;
  int directory_size = directory->total * sizeof(struct fat_directory_item);
  directory->item = kzalloc(directory_size);
  if (!directory->item) {
    res = -ENOMEM;
    goto exit_fn;
  }

  res =
      fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
  if (res != OK) {
    goto exit_fn;
  }

exit_fn:
  return directory;
}

struct fat_item *
fat16_new_fat_item_for_directory_item(struct disk *disk,
                                      struct fat_directory_item *item) {
  // since we cannot trust the memory handled to us copy it
  struct fat_item *item_copy = kzalloc(sizeof(struct fat_item));
  if (!item_copy) {
    // failed to allocate memoery
    return 0;
  }

  if (item->attribute & FAT_FILE_SUBDIRECTORY) {
    // item is a sub directory
    item_copy->directory = fat16_load_fat_directory(disk, item);
    item_copy->type = FAT_ITEM_TYPE_DIRECTORY;
  } else {
    // item is a file
    item_copy->type = FAT_ITEM_TYPE_FILE;
    item_copy->item = fat16_clone_direcotry_item(item);
  }

  return item_copy;
}

struct fat_item *fat16_find_item_in_directory(struct disk *disk,
                                              struct fat_directory *directory,
                                              const char *name) {
  struct fat_item *tmp_item = 0;
  char tmp_filename[LIEBE_OS_MAX_PATH_LEN];
  for (int i = 0; i < directory->total; i++) {
    // convert the descriptor string to search file name
    // remove extra spaces and concats file name + extension
    // ex name= 'HELLO   ' ext='txt' -> 'hello.txt'
    fat16_get_full_relative_filename(&directory->item[i], tmp_filename,
                                     sizeof(tmp_filename));

    // check if the path file name matches with the descriptor
    if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0) {
      // found the file/directory
      tmp_item =
          fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
    }
  }
  return tmp_item;
}

void fat16_fat_item_free(struct fat_item *item) {
  if ((FAT_ITEM_TYPE)(FAT_ITEM_TYPE_DIRECTORY) == item->type) {
    fat16_free_directory(item->directory);
  } else if (item->type == (int)FAT_ITEM_TYPE_FILE) {
    kfree(item->item);
  }

  kfree(item);
}

struct fat_item *fat16_get_directory_entry(struct disk *disk,
                                           struct path_part *path) {
  struct fat_private *fat_private = disk->fs_private;
  struct fat_item *current_item = 0;
  // get first item
  // !NOTE this can be a single functin instead of calling twice
  struct fat_item *root_item = fat16_find_item_in_directory(
      disk, &fat_private->root_directory, path->part);

  if (!root_item) {
    goto exit_fn;
  }

  struct path_part *next_part = path->next;
  current_item = root_item;
  while (next_part != 0) {
    if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
      // if file not found then it should be sub directory if it is a file then
      // return with error
      current_item = 0;
      break;
    }

    // read the next sub-directory or file
    struct fat_item *tmp_item = fat16_find_item_in_directory(
        disk, current_item->directory, next_part->part);

    // free the previous mem and make the current item as permanent
    fat16_fat_item_free(current_item);
    current_item = tmp_item;
    // go to next part of the path if exist
    next_part = next_part->next;
  }

exit_fn:
  return current_item;
}

/*
 *  main fs open function
 */
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode) {
  if (mode != FILE_MODE_READ) {
    // this fn supports only read
    return ERROR(-ERDONLY);
  }

  struct fat_file_descriptor *descriptor =
      kzalloc(sizeof(struct fat_file_descriptor));
  if (!descriptor) {
    // failed to create memory for descriptor
    return ERROR(-ENOMEM);
  }

  descriptor->item = fat16_get_directory_entry(disk, path);
  if (!descriptor->item) {
    // failed to identify the item
    return ERROR(-EIO);
  }

  // file should start from the begining
  descriptor->pos = 0;

  return descriptor;
}

/*
 * copies the file into the passed buffer for size with nmemb times
 */
int fat16_read(struct disk *disk, void *descriptor, uint32_t size,
               uint32_t nmemb, char *out_ptr) {
  int res = 0;

  struct fat_file_descriptor *fat_desc = descriptor;
  struct fat_directory_item *item = fat_desc->item->item;
  int offset = fat_desc->pos;
  for (uint32_t i = 0; i < nmemb; i++) {
    // copy the file data into the buffer
    res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size,
                              out_ptr);
    if (ISERR(res)) {
      goto exit_fn;
    }
    out_ptr += size;
    offset += size;
  }

  res = nmemb;

exit_fn:
  return res;
}

int fat16_seek(void *pvt, uint32_t offset, FILE_SEEK_MODE seek_mode) {
  int res = 0;
  struct fat_file_descriptor *desc = pvt;
  struct fat_item *desc_item = desc->item;
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    res = -EINVARG;
    goto exit_fn;
  }

  struct fat_directory_item *ritem = desc_item->item;
  if (offset >= ritem->filesize) {
    res = -EIO;
    goto exit_fn;
  }

  switch (seek_mode) {
  case SEEK_SET:
    // passed offset is current
    desc->pos = offset;
    break;
  case SEEK_END:
    // passed offset from the end of the file
    res = ritem->filesize - offset;
    break;
  case SEEK_CUR:
    // current psosition plus the offset
    desc->pos += offset;
    break;
  default:
    res = -EINVARG;
  }

exit_fn:
  return res;
}

int fat16_stat(struct disk *disk, void *private, struct file_stat *stat) {
  int res = 0;

  struct fat_file_descriptor *desc = (struct fat_file_descriptor *)private;

  struct fat_item *desc_item = desc->item;
  if (desc_item->type != FAT_ITEM_TYPE_FILE) {
    res = -EINVARG;
    goto exit_fn;
  }

  struct fat_directory_item *ritem = desc_item->item;
  stat->filesize = ritem->filesize;
  stat->flags = 0x00;

  if (ritem->attribute & FAT_FILE_READ_ONLY) {
    // read only file flag
    stat->flags |= FILE_STAT_READ_ONLY;
  }

exit_fn:
  return res;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor *desc) {
  fat16_fat_item_free(desc->item);
  kfree(desc);
}

int fat16_close(void *private) {
  fat16_free_file_descriptor((struct fat_file_descriptor *)private);
  return 0;
}
