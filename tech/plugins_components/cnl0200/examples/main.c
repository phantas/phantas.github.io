/* main.c */

#include "garfield.h"
#include <stdio.h>

#define ACTIONS 6

int main(int argc, char **argv) {
  int i;
  mood gmood;

  gmood = HUNGRY;
  for (i = 0; i < ACTIONS; i++) {
    if (gmood == HUNGRY)
      gmood = eat("lasagne");
    else if  (gmood == SLEEPY)
      gmood = sleep(12);
    else 
      gmood = HUNGRY;
  }
}
