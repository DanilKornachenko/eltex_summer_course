#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_PACKETS 1000
#define BUFFER_SIZE 65535

typedef struct {
  double timestamp;
  uint8_t mac_src[6];
  uint8_t mac_dst[6];
  char ip_src[INET_ADDRSTRLEN];
  char ip_dst[INET_ADDRSTRLEN];
  uint16_t port_src;
  uint16_t port_dst;
  uint16_t data_len;
  unsigned char data[BUFFER_SIZE];
} packet_info_t;

packet_info_t packets[MAX_PACKETS];
int packet_count = 0;
int running = 1;
struct timespec start_time;

int filter_mode = 0;

void handle_sigint(int sig) {
  (void)sig;
  running = 0;
}

void print_mac(uint8_t* mac, char* out) {
  sprintf(out, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3],
          mac[4], mac[5]);
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    if (strcmp(argv[1], "-p") == 0) {
      filter_mode = 1;
    } else if (strcmp(argv[1], "-d") == 0) {
      filter_mode = 2;
    }
  }

  signal(SIGINT, handle_sigint);

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sockfd == -1) {
    perror("socket");
    exit(1);
  }

  unsigned char buffer[BUFFER_SIZE];
  struct sockaddr_ll sll;
  socklen_t sll_len = sizeof(sll);

  while (running && packet_count < MAX_PACKETS) {
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr*)&sll, &sll_len);
    if (n < 0) {
      perror("recvfrom");
      break;
    }

    struct ethhdr* eth = (struct ethhdr*)buffer;
    if (ntohs(eth->h_proto) != ETH_P_IP) {
      continue;
    }

    struct iphdr* ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    if (ip->protocol != IPPROTO_UDP) {
      continue;
    }

    size_t ip_header_len = ip->ihl * 4;
    if (n < sizeof(struct ethhdr) + ip_header_len + sizeof(struct udphdr)) {
      continue;
    }

    struct udphdr* udp =
        (struct udphdr*)(buffer + sizeof(struct ethhdr) + ip_header_len);
    uint16_t src_port = ntohs(udp->source);
    uint16_t dst_port = ntohs(udp->dest);
    uint16_t udp_len = ntohs(udp->len);

    if (udp_len < sizeof(struct udphdr)) {
      continue;
    }

    if (filter_mode == 1 && dst_port != 5000) {
      continue;
    }
    if (filter_mode == 2 && dst_port != 53) {
      continue;
    }

    packet_info_t* pkt = &packets[packet_count];

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    pkt->timestamp = (now.tv_sec - start_time.tv_sec) +
                     (now.tv_nsec - start_time.tv_nsec) / 1e9;

    memcpy(pkt->mac_src, eth->h_source, 6);
    memcpy(pkt->mac_dst, eth->h_dest, 6);

    inet_ntop(AF_INET, &(ip->saddr), pkt->ip_src, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip->daddr), pkt->ip_dst, INET_ADDRSTRLEN);

    pkt->port_src = src_port;
    pkt->port_dst = dst_port;

    size_t data_len = udp_len - sizeof(struct udphdr);
    pkt->data_len = data_len;
    if (data_len > 0) {
      memcpy(pkt->data,
             buffer + sizeof(struct ethhdr) + ip_header_len +
                 sizeof(struct udphdr),
             data_len);
    }

    packet_count++;
    printf("\nЗахвачено пакетов: %d", packet_count);
    fflush(stdout);
  }

  close(sockfd);

  printf("\n\nСОБРАННЫЕ ПАКЕТЫ (%d)\n", packet_count);

  FILE* fout = fopen("output.txt", "w");
  if (!fout) {
    perror("fopen output.txt");
  }

  char mac_str[18];
  for (int i = 0; i < packet_count; i++) {
    packet_info_t* p = &packets[i];

    printf("\n--- Пакет #%d ---\n", i + 1);
    printf("Время: %.6f с\n", p->timestamp);

    print_mac(p->mac_src, mac_str);
    printf("MAC отправителя: %s\n", mac_str);
    print_mac(p->mac_dst, mac_str);
    printf("MAC получателя: %s\n", mac_str);

    printf("IP отправителя: %s\n", p->ip_src);
    printf("IP получателя: %s\n", p->ip_dst);
    printf("Порт отправителя: %u\n", p->port_src);
    printf("Порт получателя: %u\n", p->port_dst);
    printf("Длина данных: %u байт\n", p->data_len);

    if (p->data_len > 0) {
      printf("Данные: ");
      if (p->port_dst == 5000 || p->port_src == 5000) {
        unsigned char* data = p->data;
        size_t len = p->data_len;
        size_t str_len = 0;
        while (str_len < len && data[str_len] != '\0') str_len++;
        if (str_len > 0) {
          printf("%.*s", (int)str_len, data);
        } else {
          printf("(пустая строка)");
        }
      } else {
        for (size_t j = 0; j < p->data_len; j++) {
          printf("%02x ", p->data[j]);
          if ((j + 1) % 16 == 0) printf("\n   ");
        }
      }
      printf("\n");
    } else {
      printf("Данные: (пусто)\n");
    }

    if (fout) {
      fprintf(fout, "--- Пакет #%d ---\n", i + 1);
      fprintf(fout, "Время: %.6f с\n", p->timestamp);
      print_mac(p->mac_src, mac_str);
      fprintf(fout, "MAC отправителя: %s\n", mac_str);
      print_mac(p->mac_dst, mac_str);
      fprintf(fout, "MAC получателя: %s\n", mac_str);
      fprintf(fout, "IP отправителя: %s\n", p->ip_src);
      fprintf(fout, "IP получателя: %s\n", p->ip_dst);
      fprintf(fout, "Порт отправителя: %u\n", p->port_src);
      fprintf(fout, "Порт получателя: %u\n", p->port_dst);
      fprintf(fout, "Длина данных: %u байт\n", p->data_len);
      if (p->data_len > 0) {
        fprintf(fout, "Данные: ");
        if (p->port_dst == 5000 || p->port_src == 5000) {
          unsigned char* data = p->data;
          size_t len = p->data_len;
          size_t str_len = 0;
          while (str_len < len && data[str_len] != '\0') str_len++;
          if (str_len > 0)
            fprintf(fout, "%.*s", (int)str_len, data);
          else
            fprintf(fout, "(пустая строка)");
        } else {
          for (size_t j = 0; j < p->data_len; j++) {
            fprintf(fout, "%02x ", p->data[j]);
            if ((j + 1) % 16 == 0) fprintf(fout, "\n   ");
          }
        }
        fprintf(fout, "\n");
      } else {
        fprintf(fout, "Данные: (пусто)\n");
      }
    }
  }

  if (fout) {
    fclose(fout);
    printf("\nРезультаты также сохранены в файл output.txt\n");
  }

  return 0;
}
