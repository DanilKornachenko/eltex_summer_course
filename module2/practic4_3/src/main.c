#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/contacts.h"

static void print_contacts(booklist* node) {
  if (!node) return;

  print_contacts(node->left);

  int id = node->Phonebook.id;
  char* FIO = node->Phonebook.FIO;
  char* email = node->Phonebook.Data.email;
  char* number = node->Phonebook.Data.number;

  printf("ID: %d\nФИО: %s\n", id, FIO);

  if (email)
    printf("Email: %s\n", email);
  else
    printf("Email: None\n");

  if (number)
    printf("Number: %s\n", number);
  else
    printf("Number: None\n");

  printf("--------------------------------------------------\n");

  print_contacts(node->right);
}

static void print_tree_view(booklist* node, const char* prefix, int is_left) {
  if (!node) return;

  char new_prefix[256];

  snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix,
           is_left ? "|   " : "    ");
  print_tree_view(node->right, new_prefix, 0);

  printf("%s%sID:%d", prefix, is_left ? "|-- " : "^-- ", node->Phonebook.id);
  if (node->Phonebook.FIO) printf(" [%s]", node->Phonebook.FIO);
  printf("\n");

  snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix,
           is_left ? "    " : "|   ");
  print_tree_view(node->left, new_prefix, 1);
}

int main(void) {
  btree tree = {0};

  bool run = true;

  system("clear");

  while (run) {
    system("clear");
    if (tree.root == NULL)
      printf("Нет контактов, добавьте\n");
    else {
      print_contacts(tree.root);
    }

    // Меню
    printf("==================================================\n");
    printf("1. Добавить контакт\n");
    printf("2. Удалить контакт (выберите его ID)\n");
    printf("3. Поиск контакта по имени\n");
    printf("4. Визуализация дерева\n");
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

      if (tree.root != NULL)
        add_contact(&tree, FIO, email, number);
      else
        init_btree(&tree, FIO, email, number);

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

      delete_by_id(&tree, id);
    }
    if (choice == 3) {
      char ch;
      printf("Введите имя: ");
      FIO = (char*)calloc(255, sizeof(char));
      scanf("%s", FIO);
      int id = find_contact_by_name(tree.root, FIO);
      if (id == -1) {
        printf("Такого контакта нет :(\n");
      } else {
        printf("ID контакта: %d", id);
      }
      free(FIO);
      scanf("%c", &ch);
      scanf("%c", &ch);
    }
    if (choice == 4) {
      char ch;
      if (!tree.root) {
        printf("Дерево пусто (((\n");
      } else {
        print_tree_view(tree.root, "", 0);
      }
      scanf("%c", &ch);
      scanf("%c", &ch);
    }
    if (choice == 0) break;
  }

  free_contacts(tree.root);

  return 0;
}
