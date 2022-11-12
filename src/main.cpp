#include "top.hpp"

#include <stdlib.h>

int main(int argc, char *argv[]) {
  pid_t pid = atoi(argv[1]);
  TopWindow(pid);
  return 0;
}
