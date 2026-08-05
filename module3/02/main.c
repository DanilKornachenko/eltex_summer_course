#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "broker.h"

typedef enum role {
  nothing,
  broker,
  publisher,
  subscruber,
} role;

static int get_queue_id() {
  key_t key = ftok("queue", 500);
  if (key == -1) {
    perror("ftok");
    exit(1);
  }
  int qid = msgget(key, 0);
  if (qid == -1) {
    perror("msgget ");
    exit(1);
  };
  return qid;
}

static volatile sig_atomic_t running = 1;
void handler(int sig) {
  (void)sig;
  running = 0;
}

void publisher_loop(char* topic) {
  int qid = get_queue_id();
  int pid = getpid();

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  printf("Издатель (тема: %s, PID=%d), Введите сообщение:\n", topic, pid);

  char buffer[256];
  while (running) {
    if (!fgets(buffer, sizeof(buffer), stdin)) break;
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strlen(buffer) == 0) break;

    message msg;
    msg.type = 1;
    snprintf(msg.text, sizeof(msg.text), "send,%d,%s,%s", pid, topic, buffer);

    if (msgsnd(qid, &msg, sizeof(msg.text), 0) == -1) {
      perror("msgsnd ");
      break;
    }
    printf("Отправлено: %s\n", msg.text);
  }
  printf("Издатель завершён.\n");
}

void subscriber_loop(int num_topics, char** topics) {
  int qid = get_queue_id();
  int pid = getpid();

  for (int i = 0; i < num_topics; i++) {
    message msg;
    msg.type = 1;
    snprintf(msg.text, sizeof(msg.text), "subscribe,%d,%s", pid, topics[i]);
    if (msgsnd(qid, &msg, sizeof(msg.text), 0) == -1) {
      perror("msgsnd ");
      exit(1);
    }
    printf("Подписка на тему '%s' отправлена.\n", topics[i]);
  }

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  // volatile sig_atomic_t running = 1;
  // void handler(int sig) { (void)sig; running = 0; }

  message msg;
  printf("Подписчик (PID=%d) ожидает сообщения...\n", pid);
  while (running) {
    ssize_t len = msgrcv(qid, &msg, sizeof(msg.text), pid, 0);
    if (len == -1) {
      perror("msgrcv ");
      break;
    }
    msg.text[len] = '\0';
    printf("Получинл: %s\n", msg.text);
  }

  for (int i = 0; i < num_topics; i++) {
    message msg;
    msg.type = 1;
    snprintf(msg.text, sizeof(msg.text), "unsubscribe,%d,%s", pid, topics[i]);
    if (msgsnd(qid, &msg, sizeof(msg.text), 0) == -1) {
      perror("msgsnd ");
    }
    printf("Отписка от темы '%s' отправлена.\n", topics[i]);
  }
  printf("Подписчик завершён.\n");
}

int main(int argc, char* argv[]) {
  int opt;
  int queue_id = 0;
  role _role = nothing;
  char* topic = NULL;
  char** topics = NULL;
  int num_topics = 0;

  while ((opt = getopt(argc, argv, "bp:s")) != -1) {
    switch (opt) {
      case 'b':
        _role = broker;
        break;
      case 'p':
        _role = publisher;
        topic = optarg;
        break;
      case 's':
        _role = subscruber;
        break;
      default:
        errno = 13;
        perror("Недопустимый флаг (ввод аргументов только с флагом) ");
        _exit(1);
    }
  }

  if (_role == subscruber) {
    num_topics = argc - optind;
    topics = &argv[optind];
    if (num_topics == 0) {
      fprintf(stderr, "Нужга хотя бы одна тема\n");
      exit(1);
    }
  }

  switch (_role) {
    case broker:
      broker_mainloop();
      break;
    case publisher:
      if (!topic) {
        fprintf(stderr, "Нужно указать тему!\n");
        exit(1);
      }
      publisher_loop(topic);
      break;
    case subscruber:
      subscriber_loop(num_topics, topics);
      break;
    default:
      fprintf(stderr, "Не указана роль!\n");
      exit(1);
  }

  return 0;
}
