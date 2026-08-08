#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_CLI 6
#define BUFFER_SIZE 4096

int sockfd = -1;
int running = 1;

static void exit_handler(int sig) {
  (void)sig;
  running = 0;
}

static void send_text(const char* line) {
  if (send(sockfd, line, strlen(line), 0) == -1) {
    perror("send ");
  }
}

static void send_file(char* filepath) {
  FILE* fp = fopen(filepath, "rb");
  if (!fp) {
    perror("fopen ");
    return;
  }

  struct stat st;
  if (stat(filepath, &st) != 0) {
    perror("stat ");
    fclose(fp);
    return;
  }

  char* filename = strrchr(filepath, '/');
  if (filename)
    filename++;
  else
    filename = (char*)filepath;

  char header[256];
  snprintf(header, sizeof(header), "FILE_START %s %ld\n", filename, st.st_size);

  if (send(sockfd, header, strlen(header), 0) == -1) {
    perror("send header ");
    fclose(fp);
    return;
  }

  char buffer[BUFFER_SIZE] = {0};
  ssize_t bytes;
  while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
    if (send(sockfd, buffer, bytes, 0) == -1) {
      perror("send file data ");
      break;
    }
  }
  fclose(fp);
  printf("Файл отправлен: %s\n", filename);
}

static void handle_server_data() {
  char buffer[BUFFER_SIZE] = {0};
  ssize_t n = recv(sockfd, buffer, sizeof(buffer), 0);
  if (n <= 0) {
    printf("Сервер закрыл соединение\n");
    running = 0;
    return;
  }
  buffer[n] = '\0';

  if (strncmp(buffer, "FILE_START ", 11) == 0) {
    char fname[256];
    long fsize;
    if (sscanf(buffer, "FILE_START %255s %ld", fname, &fsize) == 2) {
      printf("Получен файл %s, размер %ld\n", fname, fsize);
      char outname[300];
      snprintf(outname, sizeof(outname), "recived_%s", fname);
      FILE* fp = fopen(outname, "wb");
      if (!fp) {
        perror("open receive ");
        return;
      }
      long received = 0;
      while (received < fsize) {
        char chunk[BUFFER_SIZE];
        ssize_t chunk_size = recv(sockfd, chunk, sizeof(chunk), 0);
        if (chunk_size <= 0) {
          perror("recv file data");
          break;
        }
        fwrite(chunk, 1, chunk_size, fp);
        received += chunk_size;
      }
      fclose(fp);
      printf("Файл сохранён как %s", outname);
    } else {
      printf("Неверный формат FILE_START\n");
    }
  } else {
    fputs(buffer, stdout);
    fflush(stdout);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "%s <IP> <PORT>", argv[0]);
    exit(1);
  }

  signal(SIGINT, exit_handler);

  char* server_ip = argv[1];
  int server_port = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    perror("socket ");
    exit(1);
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip);

  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("connect ");
    exit(1);
  }

  printf("Введите сообщение или /file путь\n");

  fd_set readfd;

  while (running) {
    FD_ZERO(&readfd);
    FD_SET(0, &readfd);
    FD_SET(sockfd, &readfd);

    int max_fd = (sockfd > 0) ? sockfd : 0;
    int ret = select(max_fd + 1, &readfd, NULL, NULL, NULL);

    if (ret == -1) {
      if (errno == EINTR) continue;
      perror("select ");
      break;
    }

    if (FD_ISSET(0, &readfd)) {
      char line[BUFFER_SIZE] = {0};
      if (!fgets(line, sizeof(line), stdin)) {
        running = 0;
        break;
      }
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "/file ", 6) == 0) {
        send_file(line + 6);
      } else {
        strcat(line, "\n");
        send_text(line);
      }
    }

    if (FD_ISSET(sockfd, &readfd)) {
      handle_server_data();
    }
  }

  close(sockfd);
  printf("Клиент закрылся\n");
  return 0;
}
