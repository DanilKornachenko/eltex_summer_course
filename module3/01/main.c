#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

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
      if (mkfifo(pipe_name, 0777) == 0)
      {
        struct stat st;
        stat(pipe_name, &st);
        // TODO: how take fd D:
       fildes[0] = open(pipe_name, O_RDWR, 0666);
       fildes[1] = open(pipe_name, O_RDWR, 0666);
      }
      else
      {
        perror("FIFO error : ");
      }
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

  if (pid == 0)
  {
    int file;
    for (int i = 0; i < strs_size; i++)
    {
      file = open(strs[i], O_RDONLY);
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
        strcat(filepath, "|");
        write(fds[3], filepath, strlen(filepath));
        sleep(1);
        char message[100] = {0};
        strcpy(message, "MSGE:");
        size_t n = 0;
        while ((n = read(file, message + 5, 94)))
        {
          strcat(message, "|");
          write(fds[1], message, n + 6);
        sleep(1);
          //write(fds[3], "ACTN:continue|", 14);
        sleep(1);
        }
        close(file);
        sleep(1);
        write(fds[3], "ACTN:close|", 11);
        sleep(1);
      }
    }
    write(fds[3], "ACTN:END|", 9);
        sleep(1);
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
    char ch[1] = {0};
    while(true)
    {
      read(fds[2], message, 5);
      if (strstr(message, "FILE:"))
      {
        if (!fileopen)
        {
          int last = 0;
          *ch = '\0';
          for (int i = 0; *ch != '|'; i++)
          {
            read(fds[2], ch, 1);
            message[i] = *ch;
            last = i;
          }
          message[last] = '\0';
          strcat(message, ".copy");
          file = open(message, O_CREAT | O_RDWR, 0666);
          fileopen = true;
        }
      }
      if (strstr(message, "ACTN:"))
      {
        *ch = '\0';
        int last = 0;
        for (int i = 0; *ch != '|'; i++)
        {
          read(fds[2], ch, 1);
          message[i] = *ch;
          last = i;
        }
        message[last] = '\0';
        if (strstr(message, "close"))
        {
          if (fileopen)
          {
            fileopen = false;
            close(file);
          }
        }
        if (strstr(message, "END"))
        {
          // TODO: maybe clean memory
          exit(0);
        }
      }
      if (fileopen)
      {
        read(fds[0], message, 5);

        if (strstr(message, "MSGE:"))
        {
          *ch = '\0';
          int last = 0;
          for (int i = 0; *ch != '|'; i++)
          {
            read(fds[0], ch, 1);
            message[i] = *ch;
            last = i;
          }
          message[last] = '\0';
          write(file, message, last);
        }
      }
    }
  }

  /*
  for (int i = 0; i < strs_size; i++)
  {
    printf("FILE: %s\n", strs[i]);
  }
  if (pipepath[0] != '\0')
    printf("PIPE: %s\n", pipepath);
  */

  return 0;
}
