#include "broker.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

// список подписчиков
sub* subs = NULL;
int capacity_subs = 0;

// список издателей
pub* pubs = NULL;
int capacity_pubs = 0;

// список сообщений
sm* sms = NULL;
int capacity_sms = 0;

int queue_id = -1;

static volatile sig_atomic_t running = 1;

static void signal_handler(int sig) {
  (void)sig;
  running = 0;
}

static void append_subscribe(int pid, char* topic) {
  for (int i = 0; i < capacity_subs; i++) {
    if (subs[i].pid == pid && strcmp(subs[i].topic, topic) == 0) return;
  }

  capacity_subs++;

  subs = realloc(subs, capacity_subs * sizeof(sub));

  subs[capacity_subs - 1].pid = pid;
  subs[capacity_subs - 1].topic = strdup(topic);
}

static void remove_subscribe(int pid, const char* topic) {
  for (int i = 0; i < capacity_subs; i++) {
    if (subs[i].pid == pid && strcmp(subs[i].topic, topic) == 0) {
      free(subs[i].topic);
      subs[i] = subs[capacity_subs - 1];
      capacity_subs--;
      return;
    }
  }
}

static void append_publisher(int pid) {
  for (int i = 0; i < capacity_pubs; i++) {
    if (pubs[i].pid == pid) return;
  }

  capacity_pubs++;

  pubs = realloc(pubs, capacity_pubs * sizeof(pub));

  pubs[capacity_pubs - 1].pid = pid;
}

static void append_message(char* topic, char* message) {
  capacity_sms++;
  sms = realloc(sms, capacity_sms * sizeof(sm));

  sms[capacity_sms - 1].topic = strdup(topic);
  sms[capacity_sms - 1].message = strdup(message);
}

static void send_to_subscriber(int pid, const char* text) {
  message msg;
  msg.type = pid;
  strncpy(msg.text, text, sizeof(msg.text) - 1);
  msg.text[sizeof(msg.text) - 1] = '\0';

  if (msgsnd(queue_id, &msg, sizeof(msg.text), 0) == -1) {
    perror("msgsnd to subscriber ");
  }
}

static void parse_pubs(char* text) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "%s", text);

  char* token = strtok(buffer, ",");
  if (!token || strcmp(token, "send") != 0) return;

  char* pid_str = strtok(NULL, ",");
  char* topic = strtok(NULL, ",");
  char* payload = strtok(NULL, "");

  if (!pid_str || !topic) return;

  int pid = atoi(pid_str);
  append_publisher(pid);

  append_message(topic, text);

  for (int i = 0; i < capacity_subs; i++) {
    if (strcmp(subs[i].topic, topic) == 0) {
      send_to_subscriber(subs[i].pid, text);
    }
  }
}

static void parse_subs(char* text) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "%s", text);

  char* token = strtok(buffer, ",");
  if (!token || strcmp(token, "subscribe") != 0) return;

  char* pid_str = strtok(NULL, ",");
  char* topic = strtok(NULL, ",");

  if (!pid_str || !topic) return;

  int pid = atoi(pid_str);
  append_subscribe(pid, topic);

  for (int i = 0; i < capacity_sms; i++) {
    if (strcmp(sms[i].topic, topic) == 0) {
      send_to_subscriber(pid, sms[i].message);
    }
  }
}

static void parse_unsub(char* text) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "%s", text);

  char* token = strtok(buffer, ",");

  if (!token || strcmp(token, "unsubscribe") != 0) return;

  char* pid_str = strtok(NULL, ",");
  char* topic = strtok(NULL, ",");

  if (!pid_str || !topic) return;

  int pid = atoi(pid_str);
  remove_subscribe(pid, topic);
}

static void process_message(message* msg) {
  if (strncmp(msg->text, "send,", 5) == 0) {
    parse_pubs(msg->text);
  } else if (strncmp(msg->text, "subscribe,", 10) == 0) {
    parse_subs(msg->text);
  } else if (strncmp(msg->text, "unsubscribe,", 12) == 0) {
    parse_unsub(msg->text);
  } else {
    fprintf(stderr, "Corrupted message %s\n", msg->text);
  }
}

void cleanup_broker() {
  for (int i = 0; i < capacity_subs; i++) free(subs[i].topic);
  for (int i = 0; i < capacity_sms; i++) {
    free(sms[i].topic);
    free(sms[i].message);
  }
  free(subs);
  free(pubs);
  free(sms);
  if (queue_id != -1) msgctl(queue_id, IPC_RMID, NULL);
}

static void send_sigint_to_all() {
  for (int i = 0; i < capacity_pubs; i++) {
    kill(pubs[i].pid, SIGINT);
  }
  for (int i = 0; i < capacity_subs; i++) {
    kill(subs[i].pid, SIGINT);
  }
}

void broker_mainloop() {
  key_t key = ftok("queue", 500);
  if (key == -1) {
    perror("ftok ");
    exit(1);
  }

  queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
  if (queue_id == -1) {
    perror("Queue error ");
    _exit(1);
  }

  printf("Брокер запущен очередь создана: %d\n", queue_id);

  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  // subs = (sub*)calloc(sizeof(sub), capacity_subs);

  // pubs = (pub*)calloc(sizeof(pub), capacity_pubs);

  // sms = (sm*)calloc(sizeof(sm), capacity_sms);

  message msg = {0};

  long msgtyp = 1;

  while (running) {
    ssize_t len = msgrcv(queue_id, (void*)&msg, sizeof(msg.text), msgtyp, 0);
    if (len == -1) {
      // perror("msgrcv ");
      break;
    }

    msg.text[len] = '\0';

    process_message(&msg);
  }

  send_sigint_to_all();
  cleanup_broker();
  exit(0);
}
