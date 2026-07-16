#include <stdio.h>
#include <stdlib.h>

#include "calculator.h"

int askAction() {
  printf("==================================================\n");
  printf("Выберите действие:\n1. sum\n2. sub\n3. div\n4. mul\n");
  printf("==================================================\n");
  int input = 0;
  scanf(" %d", &input);

  return input;
}

void askOperand(int action) {
  printf("Введите 2 операнда:\n");
  int a, b;
  scanf("%d %d", &a, &b);

  printf("Ответ: ");
  switch (action) {
    case 1:
      printf("%d\n", sum(a, b));
      break;
    case 2:
      printf("%d\n", sub(a, b));
      break;
    case 3:
      printf("%d\n", divide(a, b));
      break;
    case 4:
      printf("%d\n", mul(a, b));
      break;
  }
}

int main(void) {
  int action = -1;
  char ch;

  system("clear");

  while (action != 0) {
    system("clear");

    action = askAction();

    if (action >= 1 && action <= 4) {
      askOperand(action);
    }
    scanf("%c", &ch);
    scanf("%c", &ch);
  }

  return 0;
}
