#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>

int sockfd = -1;
int running = 1;

int main(void) {
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (sockfd == -1) {
    perror("socket ");
    exit(1);
  }

  struct sockaddr_in dest_ip = {0};
  dest_ip.sin_family = AF_INET;
  dest_ip.sin_port = htons(5000);
  dest_ip.sin_addr.s_addr = INADDR_BROADCAST;
  socklen_t dest_len = sizeof(dest_ip);

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5000);
  addr.sin_addr.s_addr = INADDR_ANY;

  int res = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

  if (res == -1) {
    perror("bind ");
    exit(1);
  }

  unsigned char buffer[65535] = {0};

  while (running) {
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&dest_ip, &dest_len);

    if (n < 0) {
      perror("recvfrom ");
      break;
    }

    struct iphdr* ip = (struct iphdr*)buffer;
    if (ip->protocol != IPPROTO_UDP) {
      continue;
    }

    size_t ip_header_len = ip->ihl * 4;
    if (n < ip_header_len + sizeof(struct udphdr)) {
      continue;
    }

    struct udphdr* udp = (struct udphdr*)(buffer + ip_header_len);

    uint16_t src_port = ntohs(udp->source);
    uint16_t dest_port = ntohs(udp->dest);
    uint16_t udp_len = ntohs(udp->len);
    uint16_t checksum = ntohs(udp->check);

    printf("UDP пакет:\nSRC_PORT:%u\nDEST_PORT:%u\nUDP_LEN:%u\nCHECK:%u\n", src_port, dest_port, udp_len, checksum);

    if (udp_len - sizeof(struct udphdr) > 0) {
      char* msg = calloc(udp_len - sizeof(struct udphdr), sizeof(char));

      memcpy(msg, buffer + ip_header_len + sizeof(struct udphdr), udp_len - sizeof(struct udphdr));

      printf("msg: %s\n", msg);
      free(msg);
    }

  }

  close(sockfd);
  return 0;
}
