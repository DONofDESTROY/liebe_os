#include "liebeos.h"

int main(int argc, char **argv) {
  if (argc == 1)
    return 0; // no string to print

  print("\n");
  for (int i = 1; i < argc; i++) {
    print(argv[i]);
    if (i + 1 < argc) {
      print(" ");
    }
  }
  print("\n");
  return 0;
}
