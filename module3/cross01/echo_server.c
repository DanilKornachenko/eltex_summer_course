#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

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

    uint16_t source = htons(udphdr->source);
    uint16_t dest = htons(udphdr->dest);
    uint16_t dport = htons(udphdr->uh_dport);
    uint16_t sport = htons(udphdr->uh_sport);

    char source_ip[7] = {0};
    char dest_ip[7] = {0};

    inet_ntop(AF_INET, &source, source_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &dest, dest_ip, INET_ADDRSTRLEN);

    source_ip[6] = '\0';
    dest_ip[6] = '\0';

    printf("Source:\t%s:%u\nDest:\t%s:%u\n",source_ip, sport, dest_ip, dport);

  }


  return 0;
}
