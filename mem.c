#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include "term.h"

int main() {
  int *p= malloc(sizeof(int));
  showHexa(&p, sizeof(p));
  showChar('\n');
  stop(0);
}
