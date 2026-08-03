#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>

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

  int fds[4] = {0};
  make_pipe(pipepath, fds);

  pid_t pid = fork();

  if (pid < 0)
  {
    perror("Ошибка при создании форка! ");
    // TODO: Нужно ли что-то отчищать?
    exit(1);
  }

  if (pid > 0)
  {
    int file;
    for (int i = 0; i < strs_size; i++)
    {
      file = open(strs[i], O_RDONLY, 0222);
      if (file == -1)
      {
        perror("file not enable : ");
        continue;
      }
      else
      {
        char filepath[100] = {0};
        strcpy(filepath, "FILE:");
        strcat(filepath, strs[i]);
        write(fds[3], filepath, strlen(filepath));
        char* message[100] = {0};
        while (read(file, message, 100))
        {
          write(fds[1], message, 100);
          write(fds[3], "continue", 9);
        }
        close(file);
        write(fds[3], "close", 6);
      }
    }
    write(fds[3], "END", 4);
    int status;
    wait(&status);
  }
  else
  {
    // TODO: Переработать обработку read, так как read ждёт сообщения до талого
    // TODO: Принимать запись в файл из pipe
    int file;
    bool fileopen = false;
    char message[100] = {0};
    while(true)
    {
      if (read(fds[2], message, 100))
      {
        if (strstr(message, "FILE:"))
        {
          //TODO: create file
          char filepath[100];
          strcpy(filepath, strstr(message, "FILE:"));
          strcat(filepath, ".copy");
          file = open(filepath, O_CREAT | O_APPEND, 0444);
          fileopen = true;
        } else if (strstr(message, "close"))
        {
          // TODO: close file
          close(file);
          fileopen = false;
        }
        else if (strstr(message, "END"))
        {
          //TODO: End program
          exit(0);
        }
      }
      if (fileopen)
      {
        //TODO: loop write into file descriptor
        char message[100] = {0};
        size_t n = 0;
        while ((n = read(file, message, 100)))
        {
          write(file, message, n);
        }
      }
    }
  }

  for (int i = 0; i < strs_size; i++)
  {
    printf("FILE: %s\n", strs[i]);
  }
  if (pipepath[0] != '\0')
    printf("PIPE: %s\n", pipepath);

  return 0;
}
