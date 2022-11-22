#define _DEFAULT_SOURCE 1

#include <stdlib.h>
#include <stdio.h>

#include "term.h"

int main() {
  for (;;) {
    char lin[80];
    sprintf(lin,"\n%s", "Number= ");
    showStr("Number? ");
    readLine(lin, 80);
    int n= atoi(lin);
    showStr("Computing factorial\n");
    double r= 1.0;
    for (int i=2; i<=n; i++) {
      // showStr(".");
      r= r*i;
    }
    // showStr("\nHex dump of the result is:\n");
    // showHexa(&r, sizeof(r));
    // showStr("\nWait patiently for the\nprintable result:\n");
    // The only way to convert a number to string in standard C
    // is by using sprintf:
    sprintf(lin,"%e\n", r);
    showStr(lin);  // Took 18 minutes to show 7.200000e+02 (LRV32IM-piped)
                   // 33 minutes with LRV32IM
  }
}
