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

typedef enum {
  stay,
  left,
  right,
} move;

booklist* firstContact(char* FIO, char* email, char* number);

booklist* addContact(booklist* Booklist, char* FIO, char* email, char* number);

booklist* deleteByID(booklist* Booklist, int id);

booklist* findContact(booklist* Booklist, int id);

int findContactByName(booklist* Booklist, char* FIO);

void freeContacts(booklist* Booklist);

#endif
