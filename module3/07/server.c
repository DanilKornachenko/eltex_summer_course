#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLI 6
#define BUFFER_SIZE 4096

typedef struct cli {
  int fd;
  struct sockaddr_in addr;
  char name[10];
} client;

client clients[MAX_CLI] = {0};
int running = 1;
int user_num = 0;

static void exit_handler(int sig) {
  (void)sig;
  running = 0;
}

static void broadcast(int fd, char* msg) {
  char name[10] = {0};
  for (int i = 0; i < MAX_CLI; i++) {
    if (clients[i].fd == fd) {
      strcpy(name, clients[i].name);
      break;
    }
  }
  int n = strlen(name) + strlen(msg) + 1;
  char* message = calloc(n, sizeof(char));
  snprintf(message, n + 1, "%s:%s", name, msg);
  for (int i = 0; i < MAX_CLI; i++) {
    if (clients[i].fd == fd) continue;
    send(clients[i].fd, message, n, 0);
  }
  free(message);
}

static int add_client(int fd, struct sockaddr_in addr, char* name) {
  for (int i = 0; i < MAX_CLI; i++) {
    if (clients[i].fd == -1) {
      clients[i].fd = fd;
      clients[i].addr = addr;
      strcpy(clients[i].name, name);
      return i;
    }
  }
  return -1;
}

static void remove_client(int fd) {
  for (int i = 0; i < MAX_CLI; i++) {
    if (clients[i].fd == fd) {
      clients[i].fd = -1;
      close(fd);
      break;
    }
  }
}

int main(void) {
  signal(SIGINT, exit_handler);
  signal(SIGPIPE, SIG_IGN);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server = {0};

  server.sin_family = AF_INET;
  server.sin_port = htons(5000);
  server.sin_addr.s_addr = INADDR_ANY;
  socklen_t servlen = sizeof(server);

  if (bind(sockfd, (struct sockaddr*)&server, servlen) == -1) {
    perror("bind ");
    exit(1);
  }

  if (listen(sockfd, MAX_CLI) == -1) {
    perror("listen ");
    exit(1);
  }

  printf("Сервер запущен...\n");

  for (int i = 0; i < MAX_CLI; i++) {
    clients[i].fd = -1;
  }

  struct pollfd fds[MAX_CLI + 1];
  int nfds = 1;
  fds[0].fd = sockfd;
  fds[0].events = POLLIN;

  while (running) {
    int ret = poll(fds, nfds, -1);

    if (ret == -1) {
      if (errno == EINTR) continue;
      perror("poll ");
      break;
    }

    if (fds[0].revents & POLLIN) {
      struct sockaddr_in client_addr;
      socklen_t len = sizeof(client_addr);
      int new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &len);

      if (new_fd == -1) {
        perror("accept ");
        continue;
      }

      char name[10] = {0};
      snprintf(name, 10, "user%d", user_num);
      user_num++;

      int idx = add_client(new_fd, client_addr, name);
      if (idx == -1) {
        perror("Лимит клиентов ");
        close(new_fd);
      } else {
        fds[nfds].fd = new_fd;
        fds[nfds].events = POLLIN;
        nfds++;

        char msg[100] = {0};
        snprintf(msg, 100, "Подключился %s\n", name);

        broadcast(new_fd, msg);
      }
    }

    for (int i = 1; i < nfds; i++) {
      if (fds[i].revents & POLLIN) {
        int fd = fds[i].fd;
        char buffer[BUFFER_SIZE];
        ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0) {
          for (int j = 0; j < MAX_CLI; j++) {
            if (clients[j].fd == fd) {
              char name[10];
              strcpy(name, clients[j].name);
              remove_client(fd);
              char msg[100] = {0};
              snprintf(msg, 100, "Отключился: %s\n", name);
              broadcast(fd, msg);
              break;
            }
          }
          for (int k = i; k < nfds - 1; k++) {
            fds[k] = fds[k + 1];
          }
          nfds--;
          i--;
          continue;
        }
        buffer[n] = '\0';
        broadcast(fd, buffer);
      }
    }
  }

  close(sockfd);
  for (int i = 0; i < MAX_CLI; i++) {
    if (clients[i].fd != -1) {
      close(clients[i].fd);
    }
  }

  printf("Сервер отключён\n");

  return 0;
}
