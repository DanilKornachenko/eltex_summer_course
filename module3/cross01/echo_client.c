#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int sockfd = -1;
int running = 1;
struct sockaddr_in addr = {0};
int client_port = -1;

static void signal_handler(int sig) {
  (void)sig;
  running = 0;
  char buffer[1024] = {0};
  struct udphdr* udp = (struct udphdr*)buffer;
  udp->source = htons(client_port);
  udp->dest = htons(5000);
  udp->len = htons(sizeof(struct udphdr) + 4);
  udp->check = 0;
  strcpy(buffer + sizeof(struct udphdr), "EXIT");
  sendto(sockfd, buffer, sizeof(struct udphdr) + 4, 0, (struct sockaddr*)&addr,
         sizeof(addr));
  close(sockfd);
  exit(0);
}

int main(void) {
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

  if (sockfd == -1) {
    perror("socket ");
    exit(1);
  }

  signal(SIGINT, signal_handler);

  // struct sockaddr_in addr = {0};

  addr.sin_family = AF_INET;
  addr.sin_port = htons(5000);
  addr.sin_addr.s_addr = INADDR_ANY;
  socklen_t len = sizeof(addr);

  srand(time(NULL));

  client_port = (rand() % 40000) + 20000;

  char buffer[1024] = {0};

  while (running) {
    char* message = NULL;
    size_t bufsize = 0;
    ssize_t length = getline(&message, &bufsize, stdin);
    if (length == -1) {
      free(message);
      break;
    }
    if (length > 0 && message[length - 1] == '\n') {
      message[length - 1] = '\0';
      length--;
    }

    struct udphdr* udphdr = (struct udphdr*)buffer;

    udphdr->source = htons(client_port);
    udphdr->dest = htons(5000);
    udphdr->len = htons(sizeof(struct udphdr) + strlen(message));
    udphdr->check = 0;

    char* data = buffer + sizeof(struct udphdr);
    strcpy(data, message);

    ssize_t sent =
        sendto(sockfd, buffer, sizeof(struct udphdr) + strlen(message), 0,
               (struct sockaddr*)&addr, len);

    if (sent == -1) {
      perror("sendto ");
    } else {
      printf("Отправлено\n");
    }

    uint16_t src_port = 0;
    uint16_t dest_port = 0;
    struct iphdr* iphdr = NULL;
    struct udphdr* recvudp = NULL;
    int ip_size = 0;
    int n = -1;
    while (!(src_port == 5000 && dest_port == client_port)) {
      n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, &len);

      if (n == -1) {
        perror("recvfrom ");
        continue;
      }

      iphdr = (struct iphdr*)buffer;

      if (iphdr->protocol != IPPROTO_UDP) continue;

      ip_size = iphdr->ihl * 4;

      recvudp = (struct udphdr*)(buffer + ip_size);

      src_port = ntohs(recvudp->source);
      dest_port = ntohs(recvudp->dest);
    }
    if (iphdr == NULL) continue;
    if (recvudp == NULL) continue;

    if (iphdr->saddr == iphdr->daddr && recvudp->source == recvudp->dest) {
      printf("Один и тот же адрес:(\n");
      continue;
    }

    char* recvdata = buffer + sizeof(struct udphdr) + ip_size;

    int data_len = n - ip_size - sizeof(struct udphdr);

    printf("response: %.*s\n", data_len, recvdata);
    free(message);
    src_port = -1;
  }

  close(sockfd);

  return 0;
}
