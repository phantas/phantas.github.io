/* eating.c */

#include "garfield.h"
#include <stdio.h>

mood eat (char* food) {
  printf ("Garfield is eating %s.\n", food);
  return SLEEPY;
}

