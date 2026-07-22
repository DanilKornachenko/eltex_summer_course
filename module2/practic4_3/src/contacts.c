#include "../include/contacts.h"

#include <stdlib.h>
#include <string.h>

int maxID = 0;

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

static int cmpName(char* main, char* other) { return strcmp(main, other); }

static void add_node_to_array(booklist* node, booklist*** array, int* size,
                              int* capacity) {
  if (*size >= *capacity) {
    *capacity *= 2;
    *array = realloc(*array, (*capacity) * sizeof(booklist*));
  }
  (*array)[(*size)++] = node;
}

static void store_inorder(booklist* node, booklist*** array, int* size,
                          int* capacity) {
  if (!node) return;
  store_inorder(node->left, array, size, capacity);
  add_node_to_array(node, array, size, capacity);
  store_inorder(node->right, array, size, capacity);
}

static booklist* build_balanced(booklist** nodes, int start, int end) {
  if (start > end) return NULL;
  int mid = start + (end - start) / 2;
  booklist* root = nodes[mid];
  root->left = build_balanced(nodes, start, mid - 1);
  root->right = build_balanced(nodes, mid + 1, end);
  return root;
}

static void balance_tree(btree* tree) {
  if (!tree->root) return;

  int size = 0;
  int capacity = BALANCE_THRESHOLD;
  booklist** nodes = malloc(capacity * sizeof(booklist*));

  store_inorder(tree->root, &nodes, &size, &capacity);
  tree->root = build_balanced(nodes, 0, size - 1);
  tree->mod_count = 0;

  free(nodes);
}

void init_btree(btree* tree, char* FIO, char* email, char* number) {
  if (!tree->root) {
    phonebook user = createUser(1, FIO, email, number);

    booklist* Booklist = (booklist*)malloc(sizeof(booklist));
    Booklist->left = NULL;
    Booklist->right = NULL;

    Booklist->Phonebook = user;
    maxID = 1;

    tree->root = Booklist;
    tree->mod_count = 1;
  }
}

void add_contact(btree* tree, char* FIO, char* email, char* number) {
  int id = maxID + 1;
  maxID = id;

  booklist* tmp = (booklist*)malloc(sizeof(booklist));
  tmp->left = NULL;
  tmp->right = NULL;
  tmp->Phonebook = createUser(id, FIO, email, number);

  if (tree->root == NULL) {
    tree->root = tmp;
    return;
  }

  booklist* current = tree->root;

  while (1) {
    if (current->Phonebook.id > tmp->Phonebook.id) {
      if (current->left) {
        current = current->left;
        continue;
      } else {
        current->left = tmp;
        break;
      }
    } else {
      if (current->right) {
        current = current->right;
        continue;
      } else {
        current->right = tmp;
        break;
      }
    }
  }

  tree->mod_count++;
  if (tree->mod_count >= BALANCE_THRESHOLD) {
    balance_tree(tree);
  }
}

static booklist* find_min(booklist* node) {
  while (node->left) node = node->left;
  return node;
}

static booklist* delete_node(booklist* root, int id) {
  if (root == NULL) return NULL;

  if (id < root->Phonebook.id)
    root->left = delete_node(root->left, id);
  else if (id > root->Phonebook.id)
    root->right = delete_node(root->right, id);
  else {
    // Узел найден
    if (root->left == NULL) {
      booklist* temp = root->right;
      free(root->Phonebook.FIO);
      free(root->Phonebook.Data.email);
      free(root->Phonebook.Data.number);
      free(root);
      return temp;
    } else if (root->right == NULL) {
      booklist* temp = root->left;
      free(root->Phonebook.FIO);
      free(root->Phonebook.Data.email);
      free(root->Phonebook.Data.number);
      free(root);
      return temp;
    }
    // Случай с двумя потомками
    booklist* min_right = find_min(root->right);
    root->Phonebook.id = min_right->Phonebook.id;
    free(root->Phonebook.FIO);
    root->Phonebook.FIO = malloc(strlen(min_right->Phonebook.FIO) + 1);
    strcpy(root->Phonebook.FIO, min_right->Phonebook.FIO);
    free(root->Phonebook.Data.email);
    root->Phonebook.Data.email = min_right->Phonebook.Data.email
                                     ? strdup(min_right->Phonebook.Data.email)
                                     : NULL;
    free(root->Phonebook.Data.number);
    root->Phonebook.Data.number = min_right->Phonebook.Data.number
                                      ? strdup(min_right->Phonebook.Data.number)
                                      : NULL;

    root->right = delete_node(root->right, min_right->Phonebook.id);
  }
  return root;
}

void delete_by_id(btree* tree, int id) {
  tree->root = delete_node(tree->root, id);
  tree->mod_count++;
  if (tree->mod_count >= BALANCE_THRESHOLD) {
    balance_tree(tree);
  }
}

booklist* find_contact(booklist* root, int id) {
  if (!root) return NULL;

  if (id == root->Phonebook.id) return root;
  if (id < root->Phonebook.id)
    return find_contact(root->left, id);
  else
    return find_contact(root->right, id);
}

int find_contact_by_name(booklist* root, char* FIO) {
  if (!root) return -1;
  if (strstr(root->Phonebook.FIO, FIO)) return root->Phonebook.id;
  int left = find_contact_by_name(root->left, FIO);
  if (left != -1) return left;
  return find_contact_by_name(root->right, FIO);
}

void free_contacts(booklist* root) {
  if (!root) return;

  free_contacts(root->left);
  free_contacts(root->right);
  free(root->Phonebook.FIO);
  free(root->Phonebook.Data.email);
  free(root->Phonebook.Data.number);
  free(root);
}
