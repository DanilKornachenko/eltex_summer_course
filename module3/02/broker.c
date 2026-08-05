#include "broker.h"
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// список подписчиков
sub* subs;
int capacity_subs = 1;

// список издателей
pub* pubs;
int capacity_pubs = 1;

// список сообщений
sm* sms;
int capacity_sms = 1;

int queue_id = -1;

static void append_subscribe(int pid, char* topic)
{
  capacity_subs++;

  subs = realloc(subs, capacity_subs);

  subs[capacity_subs - 1].pid = pid;
  subs[capacity_subs - 1].topic = topic;
}

static void append_message(char* topic, char* message)
{
  capacity_sms++;
  sms = realloc(sms, capacity_sms);

  sms[capacity_sms - 1].topic = topic;
  sms[capacity_sms - 1].message = message;
}

void* search_pid(void *strcts, size_t size, int capacity, int pid)
{
  void* result = NULL;

  for (int i = 0; i < capacity; i++)
  {
    void* strct = strcts + (i * size);

    if ((*(int*)strct) == pid)
    {
      result = strcts + (i * size);
    }
  }

  return result;
}

void parse_pubs(int result, message msg)
{
  char* column2 = strtok(msg.text, ",");
  char* column3 = strtok(msg.text, ",");
  char* end = strtok(msg.text, ",");

  char tmp[100] = {0};

  int pid = atoi(column2);

  int zero = 0;

  for (int i = 0; column3 != end; i++)
  {
    tmp[i] = *column3;
    zero = i;
  }

  tmp[zero] = '\0';

  char* topic = strdup(tmp);

  pub* p = (pub*)search_pid(pubs, sizeof(pub), capacity_pubs, pid);

  capacity_pubs++;
  pubs = realloc(pubs, capacity_pubs);
  pubs[capacity_pubs - 1].pid = pid;
  append_message(topic, msg.text);
}

void parse_subs(int result, message msg)
{
  char* column2 = strtok(msg.text, ",");
  char* column3 = strtok(msg.text, ",");

  int pid = atoi(column2);

  char* topic = strdup(column3);

}

static void send_all_message(int pid, char* topic)
{
  for (int i = 0; i < capacity_sms; i++)
  {
    if (strstr(topic, sms[i].topic) != NULL)
    {
      message msg = {0};
      msg.type = pid;
      strcpy(msg.text, sms[i].message);
      msgsnd(queue_id, &msg, sizeof(msg), 0);
    }
  }
}

void process_message(int result, message msg)
{
  if (strstr("send", msg.text) != NULL)
  {
    // TODO: publisher parser
    parse_pubs(result, msg);
  }
  else if (strstr("subscribe", msg.text) != NULL)
  {
    // TODO: subscribe parser
    parse_subs(result, msg);
  }
  else
  {
    errno = 13;
    perror("Invalide message ");
  }
}

void send_all()
{
  for (int i = 0; i < capacity_subs; i++)
  {
    int pid = subs[i].pid;
    char* topic = subs[i].topic;


  }
}

void broker_mainloop()
{

  queue_id = msgget(ftok("queue", 500), IPC_CREAT | IPC_EXCL | 0666);
  if (queue_id == -1)
  {
    perror("Queue error ");
    _exit(1);
  }

  subs = (sub*)calloc(sizeof(sub), capacity_subs);

  pubs = (pub*)calloc(sizeof(pub), capacity_pubs);

  sms = (sm*)calloc(sizeof(sm), capacity_sms);

  int result;

  message msg = {0};

  long msgtyp = 1;

  while (1)
  {
    result = msgrcv(queue_id, (void*) &msg, sizeof(msg.text), msgtyp, MSG_NOERROR);
    if (result == -1) continue;

    process_message(result, msg);

    send_all();
  }
}
