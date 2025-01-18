#include "pparser.h"
#include "../config.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../status.h"
#include "../string/string.h"

static int is_valid_path(const char *filename) {
  size_t len = strnlen(filename, LIEBE_OS_MAX_PATH_LEN);
  return (len >= 3 && is_digit(filename[0]) &&
          memcmp((void *)&filename[1], ":/", 2) == 0);
}

/*
 * returns drive number(int) from the path passed
 */
static int get_drive_number(const char **path) {
  if (!is_valid_path(*path)) {
    return -EBADPATH;
  }
  int drive_no = atoi(*path[0]);
  return drive_no;
}

/*
 * creates a root node
 */
static struct path_root *create_root_path(int drive_number) {
  struct path_root *root = kzalloc(sizeof(struct path_root));
  root->drive_no = drive_number;
  root->first = 0;
  return root;
}

/*
 * recursive fn to create rest of the paths
 * supports upto n level
 */
struct path_part *create_path_nodes(const char **path,
                                    struct path_part *last_path) {
  const char *tmp_path = *path;
  if (tmp_path[0] == 0x00) {
    return 0;
  }
  char *path_str = kzalloc(LIEBE_OS_MAX_PATH_LEN);
  int idx = 0;
  while (*tmp_path != '/' && *tmp_path != 0x00) {
    path_str[idx] = *tmp_path;
    tmp_path++;
    idx++;
  }

  struct path_part *part = kzalloc(sizeof(struct path_part));
  part->part = path_str;
  part->next = 0x00;
  if (last_path) {
    last_path->next = part;
  }
  if (*tmp_path == '/') {
    tmp_path++;
  }
  create_path_nodes(&tmp_path, part);
  return part;
}

struct path_root *pathparser_parse(const char *path,
                                   const char *current_directory_path) {

  struct path_root *root = 0;
  if (!is_valid_path(path)) {
    goto end;
  }
  int drive_number = get_drive_number(&path);
  root = create_root_path(drive_number);

  const char *tmp_path = path;
  tmp_path += 3;
  struct path_part *path_start_obj_ref = create_path_nodes(&tmp_path, NULL);
  root->first = path_start_obj_ref;

end:
  return root;
}

void pathparser_free(struct path_root *root) {
  struct path_part *part = root->first;
  while (part) {
    struct path_part *next_part = part->next;
    kfree((void *)part->part);
    kfree(part);
    part = next_part;
  }
  kfree(root);
}
