#ifndef PATHPARSER_H
#define PATHPARSER_H

struct path_root {
  int drive_no;
  struct path_part *first;
};

struct path_part {
  const char *part;
  struct path_part *next;
};

// This is used to store the path in form of linked list
// 0:/Projects/summa.c
//
// path_root--->path_part------>path_part
// [0:/] ----->[Projects] ----->[summa.c]----> null ptr

void pathparser_free(struct path_root *root);

struct path_root *pathparser_parse(const char *path,
                                   const char *current_directory_path);

#endif // !PATHPARSER_H
