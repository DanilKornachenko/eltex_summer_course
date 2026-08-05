#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#include "p2p.h"

int main(int argc, char** argv) {
  if (argc == 2) {
    char* name = strdup(argv[1]);
    create_open_queue(name);

  } else {
    perror("Expected arg name");
  }

  return 0;
}
