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

int pop_equal(booklist** queue, short priority);

int pop_first(booklist** queue);

int pop_greater(booklist** queue, short priority);

void freeContacts(booklist* Booklist);

#endif
