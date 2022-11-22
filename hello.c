#define _DEFAULT_SOURCE 1

#include <stdlib.h>
#include "term.h"

int main() {
  volatile char *term= (char*)0x1ffffffc;
  char *hello= "hello world\n";
  char *s= hello;
  while (*s!=0)
    *term= *s++;
  stop(0);
  return 0;
}
