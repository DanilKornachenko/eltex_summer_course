#ifndef CALCULATOR_H
#define CALCULATOR_H

#define PAIR_LENGH 5

typedef struct func_pair {
  char sign;
  double (*func)(double, double);
} func_pair;

void init_pair(func_pair functions[]);

double sum(double a, double b);

double sub(double a, double b);

double divide(double a, double b);

double mul(double a, double b);

#endif
