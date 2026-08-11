#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

int sockfd = -1;
int running = 1;

static void signal_handler(int sig) {
  (void)sig;
  running = 0;
}

int main(void) {
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

  if (sockfd == -1) {
    perror("socket ");
    exit(1);
  }

  signal(SIGINT, signal_handler);

  struct sockaddr_in addr = {0};

  addr.sin_family = AF_INET;

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind ");
    exit(1);
  }



  return 0;
}
