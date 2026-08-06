#include <fcntl.h>
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern bool isHost;

#include "p2p.h"

int main(int argc, char** argv) {
  if (argc == 2) {
    char* name = strdup(argv[1]);
    free(create_open_queue(name));

    if (isHost)
      printf("is Host\n");
    else
      printf("is Client\n");

    chat();

    close_queue();
  } else {
    perror("Expected arg name");
  }

  return 0;
}
