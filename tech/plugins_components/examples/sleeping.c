/* sleeping.c */

#include "garfield.h"
#include <stdio.h>

mood sleep (int hours) {
  printf ("Garfield is sleeping for %d hours.\n", hours);
  return HUNGRY;
}
