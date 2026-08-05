#include "p2p.h"

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ds1, ds2;

void create_open_queue(char* name) {
  mqd_t ds;
  struct mq_attr queue_attr;
  queue_attr.mq_maxmsg = 32;
  queue_attr.mq_msgsize = 256;
  if ((ds = mq_open(strcat(name, "_1"), O_RDWR, 0666)) == (mqd_t)-1) {
    if ((ds = mq_open(strcat(name, "_1"), O_CREAT | O_RDWR, 0666)) ==
        (mqd_t)-1) {
      perror("Cannot may create queue");
      exit(1);
    }
  }
  ds1 = ds;
  if ((ds = mq_open(strcat(name, "_2"), O_RDWR, 0666)) == (mqd_t)-1) {
    if ((ds = mq_open(strcat(name, "_2"), O_CREAT | O_RDWR, 0666)) ==
        (mqd_t)-1) {
      perror("Cannot may create queue");
      exit(1);
    }
  }
  ds2 = ds;
}
