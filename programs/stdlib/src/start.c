#include "liebeos.h"

extern int main(int argc, char **argv);

void main_wrapper() {
  // get the argc and argv for the main func
  struct process_arguments arguments;
  liebeos_process_get_arguments(&arguments);

  int res = main(arguments.argc, arguments.argv);
  if (res == 0) {
  }
}
