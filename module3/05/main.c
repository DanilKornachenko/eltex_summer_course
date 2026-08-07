#include "workers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
  if (argc == 2) {
    if (strcmp(argv[1], "-m") == 0) {
      printf("Start manufacturer\n");
      manufacturer_loop();
    } else if (strcmp(argv[1], "-p") == 0) {
      printf("Start consumer\n");
      consumer_loop();
    }
  } else {
    perror("flags: -m | -c");
    exit(1);
  }

  return 0;
}
