#include <stdlib.h>
#include "term.h"

long long factorial(int x) {
  if(x<=1)
    return 1;
  else
    return x*factorial(x-1);
}

int main() {
  for (;;) {
    char lin[81];
    showStr("? ");
    readLine(lin, 80);
    int n= atoi(lin);
    showStr("fact(");
    showInt(n);
    showStr(")=");
    long long r= factorial(n);
    showLongLong(r);
    showChar('\n');
  }
}
