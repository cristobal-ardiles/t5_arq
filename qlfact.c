#include <stdlib.h>
#include <stdint.h>

#include "term.h"

typedef __int128 BigInt_t;

BigInt_t factorial(int x) {
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
    BigInt_t r= factorial(n);
    showLongLong(r);
    showChar('\n');
  }
}
