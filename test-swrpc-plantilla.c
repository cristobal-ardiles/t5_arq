#include "stdlib.h"
#include "stdint.h"
#include "term.h"

int g;

int main(int argc, char *argv[]) {
  if (argc==0) {
    showStr("argc es 0\n");
  }
  volatile int *p= &g;
  for (int i= 0; i<3; i++) {
    *p= i;
    showStr("escr. "); showInt(i); showStr(" en ");
    showHexInt((intptr_t)(int*)p); showChar('\n');
    int j= *p;
    if (i!=j) {
      showStr("*** Error: se escribio "); showInt(j); showStr(" ***\n");
      exit(1);
    }
  }
  showStr("Felicitaciones!\n");
  return 0;
}
