#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

char user[50] = {0};

int global_sock = -1;
struct sockaddr_in global_dest_sockaddr = {0};

void* recv_thread(void* arg) {
  int sockfd = *(int*)arg;
  char mess[100] = {0};
  struct sockaddr_in dest_addr;
  socklen_t dest_len = sizeof(dest_addr);

  while (1) {
    int n = recvfrom(sockfd, mess, sizeof(mess), 0,
                     (struct sockaddr*)&dest_addr, &dest_len);

    if (n == -1) {
      perror("recvfrom ");
    } else if (n != 0) {
      mess[n] = '\0';
      pthread_mutex_lock(&stdout_mutex);
      if (!(strncmp(mess, user, strlen(user)) == 0)) {
        fwrite(mess, 1, n, stdout);
        putc('\n', stdout);
        fflush(stdout);
      }
      pthread_mutex_unlock(&stdout_mutex);
    }
  }
  return NULL;
}

static void kind_exit(int sig)
{
  char send_msg[100];
  snprintf(send_msg, 100, "%s:Disconnected", user);
  sendto(global_sock, send_msg, strlen(send_msg), 0, (struct sockaddr*)&global_dest_sockaddr, sizeof(global_dest_sockaddr));
  exit(0);
}

int main(void) {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in addr = {0};

  addr.sin_family = AF_INET;
  addr.sin_port = htons(5000);

  struct sockaddr_in dest_addr = {0};
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(5000);
  dest_addr.sin_addr.s_addr = INADDR_BROADCAST;
  socklen_t dest_len = sizeof(dest_addr);

  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ==
      -1) {
    perror("setsockopt");
    exit(1);
  }

  int broadcast = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
                 sizeof(broadcast)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  int res = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

  if (res == -1) {
    perror("bind ");
    exit(1);
  }

  char mess[100] = {0};
  char send_msg[100] = {0};
  snprintf(user, 50, "user%d", getpid());

  snprintf(mess, 100, "[CON]:%s", user);

  int s = sendto(sockfd, mess, strlen(mess), 0, (struct sockaddr*)&dest_addr,
                 sizeof(dest_addr));

  if (s == -1) {
    perror("first bind ");
  }

  pthread_t tid;
  pthread_create(&tid, NULL, recv_thread, &sockfd);

  global_sock = sockfd;
  global_dest_sockaddr = dest_addr;

  signal(SIGINT, kind_exit);

  while (1) {
    fgets(mess, sizeof(mess), stdin);
    mess[strcspn(mess, "\n")] = '\0';
    if (!(strlen(mess) == 0)) {
      snprintf(send_msg, 100, "%s:%s", user, mess);
      int send_len = sendto(sockfd, send_msg, strlen(send_msg), 0,
                            (struct sockaddr*)&dest_addr, dest_len);
      if (send_len == -1) {
        perror("loop send ");
      }
    }
  }

  return 0;
}
