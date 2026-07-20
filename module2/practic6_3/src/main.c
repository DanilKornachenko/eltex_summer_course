#include <stdio.h>
#include <stdlib.h>

#include "../include/calculator.h"

int askAction(func_pair functions[]) {
  printf("==================================================\n");
  printf("Выберите действие:\n");

  for (int i = 0; i < PAIR_LENGH ; i++)
  {
    printf("%d. %s\n", i + 1, functions[i].name);
  }
  printf("==================================================\n");
  int input = 0;
  scanf(" %d", &input);

  return input;
}

void askOperand(int action, func_pair functions[]) {
  printf("Введите 2 операнда:\n");
  double a, b;
  scanf("%lf %lf", &a, &b);

  printf("Ответ: ");
  printf("%.5lf", functions[action - 1].func(a, b));
}

int main(void) {
  int action = -1;
  char ch;

  func_pair functions[PAIR_LENGH] = {0};

  include_lib(functions);

  system("clear");

  while (action != 0) {
    system("clear");

    action = askAction(functions);

    if (action >= 1 && action <= PAIR_LENGH) {
      askOperand(action, functions);
    }
    scanf("%c", &ch);
    scanf("%c", &ch);
  }

  clean_lib(functions);

  return 0;
}
