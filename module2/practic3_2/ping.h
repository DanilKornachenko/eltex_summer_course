#ifndef PING_H
#define PING_H

typedef int ip_addr;

ip_addr parse_ip(char* str_gateway);

void generate_packet(ip_addr gateway, int mask, int n_count);

int check_mask(ip_addr src, ip_addr desc, ip_addr mask);

void print_ip(ip_addr addr);

#endif
