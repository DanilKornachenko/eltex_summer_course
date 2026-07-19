#include "calculator.h"

void init_pair(func_pair functions[])
{
  func_pair sum_f = {'+', sum};

  functions[0] = sum_f;

  func_pair sub_f = {'-', sub};

  functions[1] = sub_f;

  func_pair div_f = {'/', divide};

  functions[2] = div_f;

  func_pair mul_f = {'*', mul};

  functions[3] = mul_f;

  func_pair susum = {'-', sum};

  functions[4] = susum;
}

double sum(double a, double b) { return a + b; }

double sub(double a, double b) { return a - b; }

double divide(double a, double b) { return a / b; }

double mul(double a, double b) { return a * b; }
