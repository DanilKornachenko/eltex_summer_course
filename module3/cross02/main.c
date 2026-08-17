#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "drivers.h"

#define MAX_EVENTS 10

int main(void) {
  init_epoll();

  struct epoll_event events[MAX_EVENTS];

  while (1) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("epoll_wait");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO) {
        char input[256];
        if (fgets(input, sizeof(input), stdin) == NULL) {
          printf("\nExit.\n");
          exit(0);
        }
        input[strcspn(input, "\n")] = '\0';

        char cmd[64];
        pid_t pid;
        unsigned int time;

        if (sscanf(input, "%63s", cmd) != 1) continue;

        if (strcmp(cmd, "create_driver") == 0) {
          create_driver();
        } else if (strcmp(cmd, "send_task") == 0) {
          if (sscanf(input, "%*s %d %u", &pid, &time) == 2)
            send_task(pid, time);
          else
            printf("Usage: send_task <pid> <task_timer>\n");
        } else if (strcmp(cmd, "get_status") == 0) {
          if (sscanf(input, "%*s %d", &pid) == 1)
            get_status(pid);
          else
            printf("Usage: get_status <pid>\n");
        } else if (strcmp(cmd, "get_drivers") == 0) {
          get_drivers();
        } else if (strcmp(cmd, "exit") == 0) {
          exit(0);
        } else {
          printf(
              "Unknown command. Available: create_driver, send_task, "
              "get_status, get_drivers, exit\n");
        }
      } else {
        int idx = -1;
        for (int j = 0; j < capacity; j++) {
          if (drivers[j].read_fd == fd) {
            idx = j;
            break;
          }
        }
        if (idx == -1) continue;

        char buf[64];
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
          remove_driver(idx);
          continue;
        }
        buf[n] = '\0';

        if (strncmp(buf, "Available", 9) == 0) {
          drivers[idx].status = AVAILABLE;
          drivers[idx].duration = 0;
          printf("Driver %d is now Available\n", drivers[idx].pid);
        } else if (strncmp(buf, "Busy", 4) == 0) {
          unsigned int rem;
          if (sscanf(buf, "Busy %u", &rem) == 1) {
            drivers[idx].status = BUSY;
            drivers[idx].start_time = time(NULL);
            drivers[idx].duration = rem;
          }
        }
      }
    }
  }
  return 0;
}
