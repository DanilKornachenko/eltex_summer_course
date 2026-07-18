#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "stat.h"

void ask_change(__mode_t base_mode) {
  char str[255];
  printf("Введите команды модификации атрибутов: ");

  scanf(" %s", str);

  parse_rights(str, base_mode);
}

int main(void) {
  char path[255];
  char str[255];
  int choice = 0;
  char shadow;
  __mode_t rules = 0644;

  while (1) {
    system("clear");
    printf("==================================================\n");
    printf(
        "Выберите действие:\n1. Ввести маску доступа (преобразование в "
        "биты)\n");
    printf("2. Ввести путь к файлу (для чтения прав доступа)\n");
    printf("0. Выход\n");
    printf("==================================================\n");

    scanf("%d", &choice);

    if (choice == 1) {
      printf("Введите введите права доступа, как в chmod (umask 644) : ");
      scanf(" %s", (char*)str);
      __mode_t parsed_rules = parse_rights((const char*)str, rules);
      if (parsed_rules != (__mode_t)-1) {
        ask_change(parsed_rules);
      }
    } else if (choice == 2) {
      printf("Путь к файлу: ");
      scanf(" %s", (char*)path);
      __mode_t readed_rules = view_file_rights((const char*)path);
      if (readed_rules != (__mode_t)-1) {
        ask_change(readed_rules);
      }
    } else if (choice == 0) {
      break;
    }
    scanf("%c%c", &shadow, &shadow);
  }

  return 0;
}
