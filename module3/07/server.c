#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(void) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server = {0};

  server.sin_family = AF_INET;
  server.sin_port = htons(5000);
  server.sin_addr.s_addr = inet_addr("192.168.1.5");
  socklen_t servlen = sizeof(server);

  if (bind(sockfd, (struct sockaddr*)&server, servlen) == -1) {
    perror("bind ");
    exit(1);
  }

  if (listen(sockfd, 6) == -1) {
    perror("listen ");
    exit(1);
  }

  int cli = accept(sockfd, (struct sockaddr*)&server, &servlen);
  if (cli == -1) {
    perror("accept ");
  }

  if (cli > 0) {

  }

  return 0;
}
