#include "liebeos.h"

int shell() {

  print("LIEBEOS v1.0.0\n");
  while (1) {
    print("$ ");
    char buf[1024];
    liebeos_terminal_readline(buf, sizeof(buf), true);
    print("\n");
    liebeos_system_run(buf);
  }
  return 0;
}

int main(int argc, char **argv) { return shell(); }
