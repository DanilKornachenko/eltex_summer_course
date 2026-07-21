#include "ping.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ip_addr parse_ip(char* str_gateway) {
  int len = strlen(str_gateway);
  char nums[10] = {0};
  int j = 0;
  int bit = 3;
  ip_addr addr = 0;

  for (int i = 0; i < len; i++) {
    if (isdigit(str_gateway[i]) || str_gateway[i] == '.') {
      if (isdigit(str_gateway[i])) {
        nums[j] = str_gateway[i];
        j++;
      } else {
        int num = atoi(nums);
        if (num < 256) {
          addr |= num << (8 * bit);
          bit--;
          for (int nj = 0; nj <= j; nj++) {
            nums[nj] = '\0';
          }
          j = 0;
        } else {
          return -1;
        }
      }
    } else {
      return -1;
    }
  }

  if (j == 0 || bit != 0) return -1;
  int num = atoi(nums);
  if (num > 256) return -1;
  addr |= num;

  return addr;
}

int check_mask(ip_addr src, ip_addr desc, ip_addr mask) {
  if (!((src & mask) ^ (desc & mask))) return 1;

  return 0;
}

void generate_packet(ip_addr gateway, int mask, int n_count) {
  double currect = 0;
  double incurrect = 0;

  ip_addr ip_mask = 0;

  for (int i = 31; i >= 32 - mask; i--) {
    ip_mask |= (1u << i);
  }

  ip_addr generated[n_count + 1];
  for (int i = 0; i < n_count; i++) {
    generated[i] = rand();
  }

  for (int i = 0; i < n_count; i++) {
    if (check_mask(gateway, generated[i], ip_mask)) {
      print_ip(generated[i]);
      printf(" : эта подсеть\n");
      currect++;
    } else {
      print_ip(generated[i]);
      printf(" : не эта подсеть\n");
      incurrect++;
    }
  }

  printf("Сгенерированно в подсети: %d\nСгенерированно не в подсети: %d\n",
         (int)currect, (int)incurrect);
  printf("%.2lf %% packet loss\n",
         100.0 - (currect / (currect + incurrect) * 100.0));
}

void print_ip(ip_addr addr) {
  int bits[4] = {0};
  bits[0] |= addr & 0xff;
  bits[1] |= (addr >> 8) & 0xff;
  bits[2] |= (addr >> 16) & 0xff;
  bits[3] |= (addr >> 24) & 0xff;

  printf("%d.%d.%d.%d\t", bits[3], bits[2], bits[1], bits[0]);
}
