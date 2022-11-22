#include <stdio.h>
#include <string.h>

#include "dtoa_milo.h"

int main() {
  double pi=3.14159;

  char buf[80];
  dtoa_milo(pi, buf);
  printf("%s\n", buf);
  return 0;
}

