#include <check.h>
#include <stdlib.h>
#include "contacts.h"
#include <string.h>

static char* FIO;
static char* email;
static char* number;
static char* error_number;

START_TEST(delete_from_head)
{
  booklist* book = NULL;
  book = firstContact(FIO, email, number);

  book = deleteByID(book, 1);

  ck_assert_ptr_eq(book, NULL);
} END_TEST

START_TEST(delete_from_tail)
{
  booklist* book = NULL;
  book = firstContact(FIO, email, number);
  addContact(book, FIO, NULL, error_number);

  book = deleteByID(book, 2);

  ck_assert_ptr_eq(book->right, NULL);

  freeContacts(book);
} END_TEST

START_TEST(delete_from_middle)
{
  booklist* book = NULL;
  book = firstContact(FIO, email, number);
  addContact(book, FIO, NULL, error_number);
  addContact(book, FIO, NULL, NULL);

  book = deleteByID(book, 2);

  ck_assert_int_eq(book->right->Phonebook.id, 3);

  freeContacts(book);
} END_TEST

Suite* contacts_suite(void)
{
  Suite* s;
  TCase* tc_delete_from_head;
  TCase* tc_delete_from_tail;
  TCase* tc_delete_from_middle;

  s = suite_create("Contacts Test");

  tc_delete_from_head = tcase_create("Delete contact from head");
  tc_delete_from_tail = tcase_create("Delete contact from tail");
  tc_delete_from_middle = tcase_create("Delete contact from middle");

  tcase_add_test(tc_delete_from_head, delete_from_head);
  tcase_add_test(tc_delete_from_tail, delete_from_tail);
  tcase_add_test(tc_delete_from_middle, delete_from_middle);

  suite_add_tcase(s, tc_delete_from_head);
  suite_add_tcase(s, tc_delete_from_tail);
  suite_add_tcase(s, tc_delete_from_middle);

  return s;
}

int main(void)
{
  int fails;
  Suite *s;
  SRunner *sr;

  FIO = (char*)malloc(sizeof(char) * 255);
  strcpy(FIO, "KornachenkoDA");

  email = (char*)malloc(sizeof(char) * 255);
  strcpy(email, "mail@mail.ru");

  number = (char*)malloc(sizeof(char) * 255);
  strcpy(number, "89130605442");

  error_number = (char*)malloc(sizeof(char) * 255);
  strcpy(number, "8913442");

  s = contacts_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  fails = srunner_ntests_failed(sr);
  srunner_free(sr);

  free(FIO);
  free(email);
  free(number);
  free(error_number);

  return (fails == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
