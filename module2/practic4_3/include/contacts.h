#ifndef CONTACTS_H
#define CONTACTS_H

#define BALANCE_THRESHOLD 3

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

typedef struct btree {
  booklist* root;
  int mod_count;
} btree;

typedef enum {
  stay,
  left,
  right,
} move;

void init_btree(btree* tree, char* FIO, char* email, char* number);

void add_contact(btree* tree, char* FIO, char* email, char* number);

void delete_by_id(btree* tree, int id);

booklist* find_contact(booklist* root, int id);

int find_contact_by_name(booklist* root, char* FIO);

void free_contacts(booklist* root);

#endif
