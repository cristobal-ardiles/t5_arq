// Compilar con: make -B XTRA=term-d.o fast-rfact.ram
#include <stdlib.h>
#include "term.h"

void showDouble(double x);

int main() {
  for (;;) {
    char lin[80];
    showStr("Number? ");
    readLine(lin, 80);
    int n= atoi(lin);
    showStr("Computing factorial\n");
    double r= 1.0;
    for (int i=2; i<=n; i++) {
      showChar('.');
      r= r*i;
    }
    showChar('\n');
    showDouble(r);
    showChar('\n');
  }
}
