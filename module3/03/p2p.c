#include "p2p.h"

#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int ds1, ds2;
bool isHost;
int otherpid = 0;
static volatile sig_atomic_t running = 1;

static void signal_handler(int sig) {
  (void)sig;
  printf("Captured signal :D\n");
  printf("PID: %d\nOther PID: %d\n", getpid(), otherpid);
  running = 0;
  kill(otherpid, SIGINT);
  if (isHost) {
    close_queue();
  }
  exit(0);
}

char* create_open_queue(char* name) {
  mqd_t ds;
  struct mq_attr queue_attr;
  queue_attr.mq_maxmsg = 32;
  queue_attr.mq_msgsize = 256;

  size_t len = strlen(name);
  char* name1 = calloc(len + 3, sizeof(char));
  char* name2 = calloc(len + 3, sizeof(char));
  snprintf(name1, len + 3, "%s_1", name);
  snprintf(name2, len + 3, "%s_2", name);

  if ((ds = mq_open(name1, O_RDWR | O_NONBLOCK, 0600, &queue_attr)) ==
      (mqd_t)-1) {
    if ((ds = mq_open(name1, O_CREAT | O_RDWR, 0600, &queue_attr)) ==
        (mqd_t)-1) {
      perror("Cannot may create queue");
      exit(1);
    }
  }
  isHost = false;
  ds1 = ds;
  if ((ds = mq_open(name2, O_RDWR | O_NONBLOCK, 0600, &queue_attr)) ==
      (mqd_t)-1) {
    if ((ds = mq_open(name2, O_CREAT | O_RDWR, 0600, &queue_attr)) ==
        (mqd_t)-1) {
      perror("Cannot may create queue");
      exit(1);
    }
    isHost = true;
  }
  ds2 = ds;

  if (isHost)
    return name1;
  else
    return name2;
}

void chat() {
  char text[256] = {0};
  // struct mq_attr attr, old_attr;
  unsigned int priority = 1;
  // attr.mq_flags = 0;

  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  // sigaction(SIGINT, &sa, NULL);

  signal(SIGINT, signal_handler);

  char info[50] = {0};

  snprintf(info, 50, "%d|pid", getpid());

  if (isHost) {
    mq_send(ds1, info, 50, priority);
    mq_receive(ds2, info, 50, &priority);
    if (strstr(info, "pid") != NULL) {
      if (atoi(info) != getpid()) otherpid = atoi(info);
    }
  } else {
    mq_send(ds2, info, 50, priority);
    mq_receive(ds1, info, 50, &priority);
    if (strstr(info, "pid") != NULL) {
      if (atoi(info) != getpid()) otherpid = atoi(info);
    }
  }

  while (running) {
    if (isHost) {
      if (mq_receive(ds2, text, 256, &priority) == -1) {
        // perror("cannot recived ");
      } else {
        printf("user2: %s\n", text);
        if (strstr(text, "pid") != NULL) {
          if (atoi(text) != getpid()) otherpid = atoi(text);
        }
      }
      fgets(text, sizeof(text), stdin);
      if (mq_send(ds1, text, 256, priority) == -1) {
        perror("cannot send ");
      }
    } else {
      if (mq_receive(ds1, text, 256, &priority) == -1) {
        // perror("cannot recived ");
      } else {
        printf("user1: %s\n", text);
        if (strstr(text, "pid") != NULL) {
          if (atoi(text) != getpid()) otherpid = atoi(text);
        }
      }
      fgets(text, sizeof(text), stdin);
      if (mq_send(ds2, text, 256, priority) == -1) {
        perror("cannot send ");
      }
    }
  }
}

void close_queue() {
  if (isHost) {
    if (mq_close(ds1) == -1) perror("Error close ");

    if (mq_close(ds2) == -1) perror("Error close ");
  }
}
