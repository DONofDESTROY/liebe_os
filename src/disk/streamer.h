#ifndef DISKSTREAMER_H
#define DISKSTREAMER_H

#include "disk.h"

struct disk_stream {
  int pos;
  struct disk *disk;
};

// functions
struct disk_stream *get_new_disk_stream(int disk_id);
int diskstreamer_seek(struct disk_stream *stream_obj, int pos);
int diskstreamer_read(struct disk_stream *stream_obj, void *out, int total);
void free_diskstreamer(struct disk_stream *stream_obj);

#endif // !DISKSTREAMER_H
