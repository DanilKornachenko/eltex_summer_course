#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/contacts.h"

void printContacts(booklist* Booklist) {
  booklist* head = Booklist;

  system("clear");

  printf("==================ALL=CONTACTS=====================\n");

  while (head) {
    int id = head->Phonebook.id;
    short priority = head->priority;
    char* FIO = head->Phonebook.FIO;
    char* email = head->Phonebook.Data.email;
    char* number = head->Phonebook.Data.number;

    printf("ID: %d\nФИО: %s\npriority: %hd\n", id, FIO, priority);

    if (email)
      printf("Email: %s\n", email);
    else
      printf("Email: None\n");

    if (number)
      printf("Number: %s\n", number);
    else
      printf("Number: None\n");

    head = head->right;

    printf("--------------------------------------------------\n");
  }

  printf("==================================================\n");
}

int main(void) {
  booklist* Booklist = NULL;

  bool run = true;

  char ch;

  system("clear");

  while (run) {
    if (Booklist == NULL)
      printf("Нет контактов, добавьте\n");
    else {
      printContacts(Booklist);
    }

    // Меню
    printf("==================================================\n");
    printf("1. Добавить контакт(с приоритетом)\n");
    printf("2. Удалить контакт\n");
    printf("3. Удалить контакт (выберите его priority)\n");
    printf("4. Удалить контакт выше приоритета (выберите его priority)\n");
    printf("0. Выход\n");
    printf("==================================================\n");

    int choice = 0;
    scanf("%d", &choice);

    char* FIO = NULL;
    short priority = 255;
    char* email = NULL;
    char* number = NULL;

    // Сами действия
    if (choice == 1) {
      char answer;
      printf("==================================================\n");
      printf("Введите ФИО: ");

      FIO = (char*)calloc(255, sizeof(char));

      scanf("%s", FIO);

      printf("Введите приоритет: ");

      scanf("%hd", &priority);

      printf("Хотите ввести email? (y/n) ");

      scanf(" %c", &answer);

      if (answer == 'y') {
        printf("Введите email: ");
        email = (char*)calloc(255, sizeof(char));
        scanf("%s", email);
      }
      printf("Хотите ввести number? (y/n) ");
      scanf(" %c", &answer);

      if (answer == 'y') {
        printf("Введите number: ");
        number = (char*)calloc(255, sizeof(char));
        scanf("%s", number);
      }

      printf("==================================================\n");

      push_equal(&Booklist, priority, FIO, email, number);

      free(FIO);
      free(email);
      free(number);
    }
    if (choice == 2) {
      printf("==================================================\n");

      printf("Удаление первого контакта из очереди \n");

      printf("==================================================\n");

      if (!pop_first(&Booklist)) {
        printf("==================================================\n");

        printf("Rонтактов нету");
        scanf("%c", &ch);
        scanf("%c", &ch);
      }
    }
    if (choice == 3) {
      printf("==================================================\n");

      printf("Выберите контакт для удаления (по приоритету): ");
      scanf("%hd", &priority);

      printf("==================================================\n");

      if (!pop_equal(&Booklist, priority)) {
        printf("==================================================\n");

        printf("Такого контакта нету");
        scanf("%c", &ch);
        scanf("%c", &ch);
      }
    }
    if (choice == 4) {
      printf("==================================================\n");

      printf("Выберите контакт для удаления (выше приоритета): ");
      scanf("%hd", &priority);

      printf("==================================================\n");

      if (!pop_greater(&Booklist, priority)) {
        printf("==================================================\n");

        printf("Такого контакта нету");
        scanf("%c", &ch);
        scanf("%c", &ch);
      }
    }
    if (choice == 0) break;
  }

  freeContacts(Booklist);

  return 0;
}
