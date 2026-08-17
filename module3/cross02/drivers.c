#include "drivers.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

driver drivers[MAX_DRIVERS];
int capacity = 0;
int epoll_fd = -1;

static driver* find_driver(pid_t pid) {
  for (int i = 0; i < capacity; i++)
    if (drivers[i].pid == pid) return &drivers[i];
  return NULL;
}

static unsigned int remaining_time(driver* d) {
  if (d->status == AVAILABLE) return 0;
  time_t now = time(NULL);
  long elapsed = now - d->start_time;
  if (elapsed >= d->duration) return 0;
  return d->duration - elapsed;
}

void remove_driver(int idx) {
  if (idx < 0 || idx >= capacity) return;
  driver* d = &drivers[idx];
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, d->read_fd, NULL);
  close(d->read_fd);
  close(d->write_fd);
  for (int i = idx; i < capacity - 1; i++) drivers[i] = drivers[i + 1];
  capacity--;
}

static void driver_loop(int read_fd, int write_fd) {
  int epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("driver epoll_create1");
    return;
  }

  struct epoll_event ev, events[2];
  ev.events = EPOLLIN;
  ev.data.fd = read_fd;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, read_fd, &ev) == -1) {
    perror("driver epoll_ctl add read_fd");
    close(epfd);
    return;
  }

  int timer_fd = -1;
  int status = AVAILABLE;
  time_t start_time = 0;
  unsigned int duration = 0;

  while (1) {
    int nfds = epoll_wait(epfd, events, 2, -1);
    if (nfds == -1) {
      perror("driver epoll_wait");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == read_fd) {
        char buf[256];
        int n = read(read_fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
          close(epfd);
          return;
        }
        buf[n] = '\0';

        if (strncmp(buf, "task", 4) == 0) {
          unsigned int sec;
          if (sscanf(buf, "task %u", &sec) == 1) {
            if (status == AVAILABLE) {
              timer_fd = timerfd_create(CLOCK_REALTIME, 0);
              if (timer_fd == -1) {
                perror("timerfd_create");
                continue;
              }
              struct itimerspec its;
              its.it_value.tv_sec = sec;
              its.it_value.tv_nsec = 0;
              its.it_interval.tv_sec = 0;
              its.it_interval.tv_nsec = 0;
              if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
                perror("timerfd_settime");
                close(timer_fd);
                continue;
              }
              ev.events = EPOLLIN;
              ev.data.fd = timer_fd;
              if (epoll_ctl(epfd, EPOLL_CTL_ADD, timer_fd, &ev) == -1) {
                perror("epoll_ctl add timer");
                close(timer_fd);
                continue;
              }
              status = BUSY;
              start_time = time(NULL);
              duration = sec;
              dprintf(write_fd, "Ok\n");
            } else {
              time_t now = time(NULL);
              long elapsed = now - start_time;
              unsigned int rem =
                  (elapsed >= duration) ? 0 : (duration - elapsed);
              dprintf(write_fd, "Busy %u\n", rem);
            }
          }
        }
      } else if (events[i].data.fd == timer_fd) {
        uint64_t exp;
        if (read(timer_fd, &exp, sizeof(exp)) != sizeof(exp))
          perror("read timer");
        status = AVAILABLE;
        close(timer_fd);
        timer_fd = -1;
        dprintf(write_fd, "Available\n");
      }
    }
  }
}

void init_epoll(void) {
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(1);
  }
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = STDIN_FILENO;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
    perror("epoll_ctl stdin");
    exit(1);
  }
}

void create_driver(void) {
  int parent_to_child[2], child_to_parent[2];
  if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
    perror("pipe");
    return;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }

  if (pid == 0) {
    close(parent_to_child[1]);
    close(child_to_parent[0]);
    driver_loop(parent_to_child[0], child_to_parent[1]);
    exit(0);
  } else {
    close(parent_to_child[0]);
    close(child_to_parent[1]);

    if (capacity >= MAX_DRIVERS) {
      fprintf(stderr, "Too many drivers\n");
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      return;
    }

    drivers[capacity].pid = pid;
    drivers[capacity].write_fd = parent_to_child[1];
    drivers[capacity].read_fd = child_to_parent[0];
    drivers[capacity].status = AVAILABLE;
    drivers[capacity].start_time = 0;
    drivers[capacity].duration = 0;
    capacity++;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = child_to_parent[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, child_to_parent[0], &ev) == -1) {
      perror("epoll_ctl add driver");
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      return;
    }
    printf("Driver created with PID %d\n", pid);
  }
}

void send_task(pid_t pid, unsigned int task_time) {
  driver* d = find_driver(pid);
  if (!d) {
    fprintf(stderr, "Driver with PID %d not found\n", pid);
    return;
  }

  if (d->status == BUSY) {
    unsigned int rem = remaining_time(d);
    printf("Busy %u\n", rem);
    return;
  }

  char buf[64];
  snprintf(buf, sizeof(buf), "task %u\n", task_time);
  if (write(d->write_fd, buf, strlen(buf)) == -1) {
    perror("write to driver");
    return;
  }
  d->status = BUSY;
  d->start_time = time(NULL);
  d->duration = task_time;
  printf("Task assigned to driver %d\n", pid);
}

void get_status(pid_t pid) {
  driver* d = find_driver(pid);
  if (!d) {
    fprintf(stderr, "Driver with PID %d not found\n", pid);
    return;
  }
  if (d->status == AVAILABLE)
    printf("Available\n");
  else {
    unsigned int rem = remaining_time(d);
    printf("Busy %u\n", rem);
  }
}

void get_drivers(void) {
  if (capacity == 0) {
    printf("No drivers\n");
    return;
  }
  for (int i = 0; i < capacity; i++) {
    printf("PID %d: ", drivers[i].pid);
    if (drivers[i].status == AVAILABLE)
      printf("Available\n");
    else {
      unsigned int rem = remaining_time(&drivers[i]);
      printf("Busy %u\n", rem);
    }
  }
}
