#include "stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

static void print_bits(__mode_t rules)
{
  for (int i = 8; i >= 0; i--)
  {
    printf("%x", (rules >> i) & 01);
  }
  printf("\n");
}

static void print_human(__mode_t rules)
{
  const char bits[] = "rwxrwxrwx";
  char out[10] = {0};
  for (int i = 0; i < 9; i++)
    out[i] = (rules >> (8 - i)) & 01 ? bits[i] : '-';
  out[9] = '\0';
  printf("%s\n", out);
}

static void print_octal(__mode_t rules)
{
  int user = ((rules >> 6) & 7);
  int group = ((rules >> 3) & 7);
  int other = (rules & 7);
  printf("%d%d%d\n", user, group, other);
}

__mode_t view_file_rights(const char* path)
{
  struct stat sb;

  if (stat(path, &sb) != 0)
  {
    perror("stat");
    return -1;
  }

  __mode_t rules = sb.st_mode & 0777;

  printf("Битовое представление: ");
  print_bits(rules);
  printf("Буквенное представление: ");
  print_human(rules);
  printf("Цифровое представление: ");
  print_octal(rules);
  return rules;
}

static int parse_octal_mode(const char* str, __mode_t* rules)
{
  if (strlen(str) != 3) return false;
  for (int i = 0; i < 3; i++)
  {
    if (str[i] < '0' || str[i] > '7') return false;
  }
  *rules = ((str[0] - '0') << 6) |
          ((str[1] - '0') << 3) |
          (str[2] - '0');
  return true;
}

static int parse_human_mode(const char* str, __mode_t* rules)
{
  const char* p = str;
  int who = 0;

  while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a')
  {
    if (*p == 'u') who |= 4;
    else if (*p == 'g') who |= 2;
    else if (*p == 'o') who |= 1;
    else if (*p == 'a') who = 7;
    p++;
  }

  if (who == 0) who = 7;

  if (*p != '+' && *p != '-' && *p != '=')
    return false;
  char op = *p;
  p++;

  __mode_t mask = 0;
  int has_perm = false;

  while (*p != '\0')
  {
    if (*p == 'r')
    {
      mask |= 0444;
      has_perm = true;
    }
    else if (*p == 'w')
    {
      mask |= 0222;
      has_perm = true;
    }
    else if (*p == 'x')
    {
      mask |= 0111;
      has_perm = true;
    }
    else
    {
      return false;
    }
    p++;
  }
  if (!has_perm) return false;

  if (op == '=')
  {
    __mode_t clear_mask = 0;
    if (who & 4) clear_mask |= 0700;
    if (who & 2) clear_mask |= 0070;
    if (who & 1) clear_mask |= 0007;
    *rules &= ~clear_mask;
    op = '+';
  }

  if (op == '+')
  {
    __mode_t add = 0;
    if (who & 4) add |= ((mask >> 6) & 0x7) << 6;
    if (who & 2) add |= ((mask >> 3) & 0x7) << 3;
    if (who & 1) add |= ((mask >> 0) & 0x7) << 0;
    *rules |= add;
  }
  else if (op == '-')
  {
    __mode_t rem = 0;
    if (who & 4) rem |= ((mask >> 6) & 0x7) << 6;
    if (who & 2) rem |= ((mask >> 3) & 0x7) << 3;
    if (who & 1) rem |= ((mask >> 0) & 0x7) << 0;
    *rules &= ~rem;
  }

  return true;
}

__mode_t parse_rights(const char* str, __mode_t base_mode)
{
  __mode_t rules = base_mode;

  if (isdigit(str[0]))
  {
    if (!parse_octal_mode(str, &rules))
    {
      printf("Ошибка: Некоректный ввод\n");
      return -1;
    }
  }
  else if (!parse_human_mode(str, &rules))
  {
    printf("Ошибка: Некоректный ввод\n");
    return -1;
  }

  print_bits(rules);
  print_human(rules);
  print_octal(rules);
  return rules;
}
