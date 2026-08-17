#ifndef DRIVERS_H
#define DRIVERS_H

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_DRIVERS 100

#define AVAILABLE 0
#define BUSY 1

typedef struct {
  pid_t pid;
  int write_fd;
  int read_fd;
  int status;
  time_t start_time;
  unsigned int duration;
} driver;

extern driver drivers[MAX_DRIVERS];
extern int capacity;
extern int epoll_fd;

void create_driver(void);
void send_task(pid_t pid, unsigned int task_time);
void get_status(pid_t pid);
void get_drivers(void);
void init_epoll(void);
void remove_driver(int idx);

#endif
