#include "contacts.h"

#include <stdlib.h>
#include <string.h>

static phonebook createUser(int id, char* FIO, char* email, char* number) {
  phonebook user = {0};
  user.id = id;
  user.FIO = (char*)malloc(sizeof(char) * strlen(FIO) + 1);
  strcpy(user.FIO, FIO);
  data Data = {0};
  if (email) {
    if (email[0] != '\0') {
      Data.email = (char*)malloc(sizeof(char) * strlen(email) + 1);
      strcpy(Data.email, email);
    }
  }

  if (number) {
    if (strlen(number) == 11) {
      Data.number = (char*)malloc(sizeof(char) * 12);
      strcpy(Data.number, number);
    }
  }

  if (Data.email != NULL || Data.number != NULL) {
    user.Data = Data;
  }

  return user;
}

booklist* firstContact(char* FIO, char* email, char* number) {
  phonebook user = createUser(1, FIO, email, number);

  booklist* Booklist = (booklist*)malloc(sizeof(booklist));
  Booklist->left = NULL;
  Booklist->right = NULL;

  Booklist->Phonebook = user;

  return Booklist;
}

void addContact(booklist* Booklist, char* FIO, char* email, char* number) {
  booklist* head = Booklist;
  int id = head->Phonebook.id;
  while (head->right) {
    head = head->right;
  }

  id = head->Phonebook.id;

  phonebook user = createUser(id + 1, FIO, email, number);

  head->right = (booklist*)malloc(sizeof(booklist));

  head->right->Phonebook = user;

  head->right->left = head;
  head->right->right = NULL;
}

booklist* deleteByID(booklist* Booklist, int id) {
  if (Booklist == NULL) return Booklist;

  booklist* head = Booklist;
  booklist* current = Booklist;

  if (head->Phonebook.id == id) {
    if (head->right != NULL) {
      booklist* temp = head->right;
      temp->left = NULL;

      free(head->Phonebook.Data.email);

      free(head->Phonebook.Data.number);

      free(head->Phonebook.FIO);

      free(head);
      return temp;
    } else {
      free(head->Phonebook.Data.email);

      free(head->Phonebook.Data.number);

      free(head->Phonebook.FIO);

      free(head);
      head = NULL;
      return head;
    }
  }

  while (current) {
    if (current->Phonebook.id == id) {
      booklist* temp = current->left;
      temp->right = current->right;
      if (temp->right) temp->right->left = temp;

      current->left = NULL;
      current->right = NULL;

      free(current->Phonebook.Data.email);

      free(current->Phonebook.Data.number);

      free(current->Phonebook.FIO);

      free(current);

      break;
    } else {
      current = current->right;
    }
  }

  return head;
}

booklist* findContact(booklist* Booklist, int id) {
  booklist* head = Booklist;

  while (head) {
    if (head->Phonebook.id == id) return head;

    head = head->right;
  }

  return NULL;
}

void freeContacts(booklist* Booklist) {
  if (Booklist == NULL) return;

  booklist* head = Booklist;
  while (head) {
    booklist* next = head->right;

    free(head->Phonebook.FIO);
    free(head->Phonebook.Data.email);
    free(head->Phonebook.Data.number);

    free(head);

    head = next;
  }
}
