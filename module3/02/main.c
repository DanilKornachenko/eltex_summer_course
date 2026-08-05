#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/msg.h>
#include "broker.h"

typedef enum role {
  nothing,
  broker,
  publisher,
  subscruber,
} role;

int main(int argc, char* argv[])
{
  int opt;
  int queue_id = 0;
  role _role;
  while ((opt = getopt(argc, argv, "bp:s")) != -1)
  {
    switch (opt)
    {
      case 'b':
        // TODO: Создаётся брокер
        _role = broker;
        break;
      case 'p':
        // TODO: Создаётся издатель
        _role = publisher;
        break;
      case 's':
        // TODO: Создаётся подписчик
        _role = subscruber;
        break;
      default:
        errno = 13;
        perror("Недопустимый флаг (ввод аргументов только с флагом) ");
        _exit(1);

    }
  }

  switch (_role)
  {
    case broker:
      broker_mainloop();
      break;
    case publisher:
      break;
    case subscruber:
      break;
    default:
      _exit(1);
  }

  return 0;
}
