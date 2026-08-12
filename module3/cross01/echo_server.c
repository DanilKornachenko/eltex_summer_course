#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 65512

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
  addr.sin_addr.s_addr = INADDR_ANY;
  socklen_t socklen = sizeof(addr);

  char buffer[BUFFER_SIZE] = {0};

  while (running) {
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, &socklen);

    if (n == -1) {
      perror("recvfrom ");
      break;
    }

    struct iphdr* iphdr = (struct iphdr*)buffer;

    if (iphdr->protocol != IPPROTO_UDP) continue;

    int ip_size = iphdr->ihl * 4;

    struct udphdr* udphdr = (struct udphdr*)(buffer + ip_size);

    uint16_t src_port = ntohs(udphdr->source);
    uint16_t dest_port = ntohs(udphdr->dest);

    char src_ip[INET_ADDRSTRLEN], dest_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &iphdr->saddr, src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iphdr->daddr, dest_ip, INET_ADDRSTRLEN);

    char* data = buffer + sizeof(struct udphdr) + ip_size;
    int data_len = n - ip_size - sizeof(struct udphdr);

    printf("Source:\t%s:%u\nDest:\t%s:%u\nData: %s\nData Length: %d", src_ip, src_port, dest_ip, dest_port, data, data_len);

  }

  close(sockfd);

  return 0;
}
