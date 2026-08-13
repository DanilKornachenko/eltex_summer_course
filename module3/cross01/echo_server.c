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
#include <unistd.h>

#define BUFFER_SIZE 65512
#define MAX_CLIENTS 100

int sockfd = -1;
int running = 1;

typedef struct cli {
  struct sockaddr_in addr;
  int count;
} client_t;

int clients_count = 0;

client_t clients[MAX_CLIENTS] = {0};

static void signal_handler(int sig) {
  (void)sig;
  running = 0;
  close(sockfd);
  exit(0);
}

static client_t* add_client(client_t client) {
  for (int i = 0; i < clients_count; i++) {
    if (client.addr.sin_port == clients[i].addr.sin_port &&
        client.addr.sin_addr.s_addr == clients[i].addr.sin_addr.s_addr) {
      return &clients[i];
    }
  }
  if (clients_count >= MAX_CLIENTS) {
    fprintf(stderr, "Слишком много клиентов\n");
    return NULL;
  }
  clients[clients_count] = client;
  clients_count++;
  return &clients[clients_count - 1];
}

static int del_client(client_t client) {
  for (int i = 0; i < clients_count; i++) {
    if (client.addr.sin_port == clients[i].addr.sin_port &&
        client.addr.sin_addr.s_addr == clients[i].addr.sin_addr.s_addr) {
      clients[i].addr.sin_addr.s_addr = 0;
      clients[i].addr.sin_port = 0;
      clients[i].count = 0;
      return 1;
    }
  }
  return 0;
}

static void send_client(client_t client, char* message, int size) {
  if (size >= 5) {
    if (strncmp(message, "EXIT", 5) == 0) {
      int res = del_client(client);
      if (res) {
        printf("Client exit\n");
      }
      return;
    }
  }

  client_t* cli = add_client(client);

  char mess[1024] = {0};

  cli->count++;

  snprintf(mess, sizeof(mess), "%s %d", message, cli->count);

  char buffer[1024] = {0};

  struct udphdr* udphdr = (struct udphdr*)buffer;

  udphdr->dest = cli->addr.sin_port;
  udphdr->source = htons(5000);
  udphdr->check = 0;
  udphdr->len = htons(sizeof(struct udphdr) + strlen(mess));

  char* data = buffer + sizeof(struct udphdr);
  strcpy(data, mess);

  sendto(sockfd, buffer, sizeof(struct udphdr) + strlen(mess), 0,
         (struct sockaddr*)&cli->addr, sizeof(cli->addr));
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
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr,
                     &socklen);

    if (n == -1) {
      perror("recvfrom ");
      break;
    }

    buffer[n] = '\0';

    struct iphdr* iphdr = (struct iphdr*)buffer;

    if (iphdr->protocol != IPPROTO_UDP) continue;

    int ip_size = iphdr->ihl * 4;

    struct udphdr* udphdr = (struct udphdr*)(buffer + ip_size);

    uint16_t src_port = ntohs(udphdr->source);
    uint16_t dest_port = ntohs(udphdr->dest);

    if (dest_port != 5000) continue;

    if (iphdr->saddr == iphdr->daddr && udphdr->source == udphdr->dest) {
      printf("Один и тот же адрес :(\n");
      continue;
    }

    char src_ip[INET_ADDRSTRLEN], dest_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &iphdr->saddr, src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iphdr->daddr, dest_ip, INET_ADDRSTRLEN);

    char* data = buffer + sizeof(struct udphdr) + ip_size;
    int data_len = n - ip_size - sizeof(struct udphdr);

    printf("Source:\t%s:%u\nDest:\t%s:%u\nData: %.*s\nData Length: %d\n",
           src_ip, src_port, dest_ip, dest_port, data_len, data, data_len);

    // TODO: sendto client
    struct sockaddr_in cli_addr = {0};
    cli_addr.sin_port = udphdr->source;
    cli_addr.sin_addr.s_addr = iphdr->saddr;
    cli_addr.sin_family = AF_INET;
    client_t cli = {0};
    cli.addr = cli_addr;

    send_client(cli, data, n - ip_size - sizeof(struct udphdr));
  }

  close(sockfd);

  return 0;
}
