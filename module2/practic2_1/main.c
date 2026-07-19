#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contacts.h"

void printContacts(booklist* Booklist) {
  booklist* head = Booklist;

  system("clear");

  printf("==================ALL=CONTACTS=====================\n");

  while (head) {
    int id = head->Phonebook.id;
    char* FIO = head->Phonebook.FIO;
    char* email = head->Phonebook.Data.email;
    char* number = head->Phonebook.Data.number;

    printf("ID: %d\nФИО: %s\n", id, FIO);

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

  system("clear");

  while (run) {
    if (Booklist == NULL)
      printf("Нет контактов, добавьте\n");
    else {
      printContacts(Booklist);
    }

    // Меню
    printf("==================================================\n");
    printf("1. Добавить контакт\n");
    printf("2. Удалить контакт (выберите его ID)\n");
    printf("3. Изменить контакт (выберите ID)\n");
    printf("4. Поиск контакта по имени\n");
    printf("0. Выход\n");
    printf("==================================================\n");

    int choice = 0;
    scanf("%d", &choice);

    char* FIO = NULL;
    char* email = NULL;
    char* number = NULL;

    // Сами действия
    if (choice == 1) {
      char answer;
      printf("==================================================\n");
      printf("Введите ФИО: ");

      FIO = (char*)calloc(255, sizeof(char));

      scanf("%s", FIO);

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

      if (Booklist != NULL)
        Booklist = addContact(Booklist, FIO, email, number);
      else
        Booklist = firstContact(FIO, email, number);

      free(FIO);
      free(email);
      free(number);
    }
    if (choice == 2) {
      int id = 0;

      printf("==================================================\n");

      printf("Выберите контакт для удаления: ");
      scanf("%d", &id);

      printf("==================================================\n");

      Booklist = deleteByID(Booklist, id);
    }
    if (choice == 3) {
      int id = 0;

      char answer;

      printf("==================================================\n");

      printf("Выберите контакт для изменения: ");
      scanf("%d", &id);

      printf("==================================================\n");

      booklist* changedContact = findContact(Booklist, id);

      if (changedContact) {
        printf("Поменять ФИО? (y/n) : ");
        scanf(" %c", &answer);

        if (answer == 'y') {
          printf("Введите новое имя: ");
          FIO = (char*)calloc(255, sizeof(char));
          scanf("%s", FIO);
          free(changedContact->Phonebook.FIO);
          changedContact->Phonebook.FIO =
              (char*)malloc(sizeof(char) * strlen(FIO) + 1);
          strcpy(changedContact->Phonebook.FIO, FIO);
        }

        printf("Поменять email? (y/n) : ");
        scanf(" %c", &answer);

        if (answer == 'y') {
          printf("Введите новый email: ");
          email = (char*)calloc(255, sizeof(char));
          scanf("%s", email);

          free(changedContact->Phonebook.Data.email);

          changedContact->Phonebook.Data.email =
              (char*)malloc(sizeof(char) * strlen(email) + 1);
          strcpy(changedContact->Phonebook.Data.email, email);
        }

        printf("Поменять number? (y/n) : ");
        scanf(" %c", &answer);

        if (answer == 'y') {
          printf("Введите новый number: ");
          number = (char*)calloc(255, sizeof(char));
          scanf("%s", number);

          free(changedContact->Phonebook.Data.number);

          if (strlen(number) == 11) {
            changedContact->Phonebook.Data.number =
                (char*)malloc(sizeof(char) * strlen(number) + 1);
            strcpy(changedContact->Phonebook.Data.number, number);
          }
        }
      } else {
        printf("Контакт не найден :(\n");
      }

      printf("==================================================\n");
    }
    if (choice == 4)
    {
      char ch;
      printf("Введите имя: ");
      FIO = (char*)calloc(255, sizeof(char));
      scanf("%s", FIO);
      int id = findContactByName(Booklist, FIO);
      if (id == -1)
      {
        printf("Такого контакта нет :(\n");
      }
      else
      {
        printf("ID контакта: %d", id);
      }
      free(FIO);
      scanf("%c", &ch);
      scanf("%c", &ch);
    }
    if (choice == 0) break;
  }

  freeContacts(Booklist);

  return 0;
}
