#ifndef CALCULATOR_H
#define CALCULATOR_H

#define PAIR_LENGH 4

typedef struct func_pair {
  char* name;
  double (*func)(double, double);
  void* handle;
} func_pair;

void include_lib(func_pair functions[]);

void clean_lib(func_pair functions[]);

double sum(double a, double b);

double sub(double a, double b);

double divide(double a, double b);

double mul(double a, double b);

#endif
