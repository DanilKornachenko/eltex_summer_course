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

void* search_pid(void* strcts, size_t size, int capacity, int pid);

void parse_pubs(int result, message msg);

void parse_subs(int result, message msg);

void process_message(int result, message msg);

void send_all();

void broker_mainloop();

#endif
