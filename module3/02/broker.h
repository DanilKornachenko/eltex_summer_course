#ifndef BROKER_H
#define BROKER_H

#include <stdlib.h>

typedef struct {
  int pid;
  char* topic;
} sub;

typedef struct {
  int pid;
} pub;

typedef struct message {
  long type;
  char text[100];
} message;

typedef struct saved_message {
  char* topic;
  char* message;
} sm;

void broker_mainloop();

void cleanup_broker();

#endif
