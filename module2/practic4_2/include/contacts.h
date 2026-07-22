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
  short priority;
  phonebook Phonebook;
  struct booklist* right;
} booklist;

typedef enum {
  stay,
  left,
  right,
} move;

void push_equal(booklist** queue, short priority, char* FIO, char* email,
                char* number);

booklist* pop_equal(booklist** queue, short priority);

booklist* pop_first(booklist** queue);

booklist* pop_greater(booklist** queue, short priority);

void freeContacts(booklist* Booklist);

#endif
