#include <stdlib.h>
#include <stdio.h>

#include "dtoa-milo.h"

extern "C"
{
  void showDouble(double x);
  #include "term.h"
}

void showDouble(double x) {
  char buf[80];
  dtoa_milo(x, buf);
  showStr(buf);
}
