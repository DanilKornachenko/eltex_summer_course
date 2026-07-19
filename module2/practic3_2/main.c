#include <complex.h>
#include <stdio.h>

#include "ping.h"

int main(void) {
  ip_addr gateway = 0;
  char str_gateway[255] = {0};
  int mask = 0;
  int n_count = 0;

  printf("Введите ip адрес шлюза, маску подсети и кол-во пакетов\n");

  scanf("%s %d %d", str_gateway, &mask, &n_count);

  if (!(mask >= 0 && mask <= 32)) {
    printf("Неправильная маска подсети\n");
  } else if (n_count < 0) {
    printf("Некорректное кол-во пакетов\n");
  } else {
    gateway = parse_ip(str_gateway);
    if (gateway == -1) {
      printf("Некорректный шлюз.\n");
    } else {
      print_ip(gateway);
      printf("\n");

      generate_packet(gateway, mask, n_count);
    }
  }

  return 0;
}
