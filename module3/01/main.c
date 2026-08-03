#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void make_pipe(char* pipe_name, int* fildes)
{
  int fd[2];
  int fd2[2];
  if (pipe_name != NULL)
  {
    if (pipe_name[0] == '\0')
    {
      if (pipe(fd) == 0)
      {
        fildes[0] = fd[0];
        fildes[1] = fd[1];
      }
      else
      {
        perror("Pipe create error ");
      }
    }
    else
    {

    }
    if (pipe(fd2) == 0)
    {
      fildes[2] = fd2[0];
      fildes[3] = fd2[1];
    }
    else
    {
      perror("Pipe create error ");
    }
  }
  else
  {
    perror("Error arguments in 'make_pipe' ");
  }
}

int main(int argc, char** argv)
{
  char** strs = (char**)malloc(sizeof(char*));
  int strs_size = 0;

  char pipepath[100] = {0};

  int skip_val = -1;
  int skip_path = -1;

  for (int i = 1; i < argc; i++)
  {
    char* arg = argv[i];
    if (strstr(arg, "-p") != NULL)
    {
      skip_val = i;
      if (i + 1 != argc)
      {
        strcpy(pipepath, argv[i+1]);
        skip_path = i+1;
      }
      else
      {
        perror("Error: -n [path-to-pipe] ");
        exit(1);
      }
    }
    else
    {
      if (i == skip_val || i == skip_path)
        continue;

      size_t len = strlen(arg);
      strs[strs_size] = (char*)malloc(sizeof(char) * len + 1);
      strcpy(strs[strs_size], argv[i]);
      strs_size++;
      strs = realloc(strs, sizeof(char**) * strs_size + 1);
    }
  }

  pid_t pid = fork();

  if (pid < 0)
  {
    perror("Ошибка при создании форка! ");
  }

  int fds[4] = {0};
  make_pipe(pipepath, fds);

  if (pid == 0)
  {
    char message[100] = {0};
    do
    {
      if (read(fds[2], message, 100) != 0)
      {
        if (strstr(message, "FILE:") != NULL)
        {
          // TODO: Сделать чтение и запись файлов форками
        }
      }
    } while (strstr(message, "end") == NULL)
  }
  else
  {

  }

  for (int i = 0; i < strs_size; i++)
  {
    printf("FILE: %s\n", strs[i]);
  }
  if (pipepath[0] != '\0')
    printf("PIPE: %s\n", pipepath);

  return 0;
}
