#ifndef CONTACTS_H
#define CONTACTS_H

typedef struct data {
  char* number;
  char* email;
} data;

typedef struct phonebook {
  int id;
  char* FIO;
  data Data;
} phonebook;

typedef struct booklist {
  phonebook Phonebook;
  struct booklist* left;
  struct booklist* right;
} booklist;

booklist* firstContact(char* FIO, char* email, char* number);

void addContact(booklist* Booklist, char* FIO, char* email, char* number);

booklist* deleteByID(booklist* Booklist, int id);

booklist* findContact(booklist* Booklist, int id);

#endif
