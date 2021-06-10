/* garfield.h */

#define HUNGRY 0
#define FULL   1
#define SLEEPY 2
#define AWAKE  3

typedef int mood;

mood eat(char* food);

mood sleep(int hours);
