#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char* argv[])
{
  int opt;
  bool broker = false;
  while ((opt = getopt(argc, argv, "bp:s")) != -1)
  {
    switch (opt)
    {
      case 'b':
        // TODO: Создаётся брокер
        break;
      case 'p':
        // TODO: Создаётся издатель
        break;
      case 's':
        // TODO: Создаётся подписчик
        break;
      default:
        errno = 13;
        perror("Недопустимый флаг (ввод аргументов только с флагом) ");
        _exit(3);

    }
  }

  return 0;
}
