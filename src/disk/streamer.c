#include "streamer.h"
#include "../config.h"
#include "../memory/heap/kheap.h"
#include "disk.h"
#include <stdbool.h>

// creates a new streamer
struct disk_stream *get_new_disk_stream(int disk_id) {
  // get the disk
  struct disk *disk = disk_get(disk_id);
  if (!disk) {
    return 0;
  }

  // create stream obj
  struct disk_stream *stream_obj = kzalloc(sizeof(struct disk_stream));
  stream_obj->pos = 0;
  stream_obj->disk = disk;
  return stream_obj;
}

int diskstreamer_seek(struct disk_stream *stream_obj, int pos) {
  stream_obj->pos = pos;
  return 0;
}

int diskstreamer_read(struct disk_stream *stream_obj, void *out, int total) {
  // calc
  int sector = stream_obj->pos / LIEBE_OS_SECTOR_SIZE;
  int offset = stream_obj->pos % LIEBE_OS_SECTOR_SIZE;
  int total_to_read = total;
  bool overflow = (offset + total_to_read) >= LIEBE_OS_SECTOR_SIZE;
  char buffer[LIEBE_OS_SECTOR_SIZE];

  if (overflow) {
    total_to_read -= (offset + total_to_read) - LIEBE_OS_SECTOR_SIZE;
  }

  int res = disk_read_block(stream_obj->disk, sector, 1, buffer);
  if (res < 0) {
    goto out;
  }

  // transfer the data from buffer into the passed ref
  for (int i = 0; i < total_to_read; i++) {
    *(char *)out++ = buffer[offset + i];
  }

  // adjust the stream
  stream_obj->pos += total_to_read;
  if (overflow) {
    // recursively call read for reading the next sector
    res = diskstreamer_read(stream_obj, out, total - LIEBE_OS_SECTOR_SIZE);
  }

out:
  return res;
}

// clears the stream sturct memory
void free_diskstreamer(struct disk_stream *stream_obj) { kfree(stream_obj); }
