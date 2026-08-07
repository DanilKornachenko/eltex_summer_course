#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

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

  snprintf(mess, 100, "[CON]:user%d", getpid());

  int s = sendto(sockfd, mess, strlen(mess), 0, (struct sockaddr*)&dest_addr,
                 sizeof(dest_addr));

  if (s == -1) {
    perror("first bind ");
  }

  while (1) {
    fgets(mess, sizeof(mess), stdin);
    mess[strcspn(mess, "\n")] = '\0';
    if (!(strlen(mess) == 0)) {
      int send_len = sendto(sockfd, mess, strlen(mess), 0,
                            (struct sockaddr*)&dest_addr, dest_len);
      if (send_len == -1) {
        perror("loop send ");
      }
    }

    int n = recvfrom(sockfd, mess, sizeof(mess), 0,
                     (struct sockaddr*)&dest_addr, &dest_len);

    if (n == -1) {
      perror("recvfrom ");
    } else if (n != 0) {
      fwrite(mess, 1, n, stdout);
      putc('\n', stdout);
    }
  }

  return 0;
}
