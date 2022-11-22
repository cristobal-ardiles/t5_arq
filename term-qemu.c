#include <stdio.h>
#include <string.h>
#include "term.h"

void showChar(char c) {
  putchar(c);
  fflush(stdout);
}

void showStr(const char *s) {
  printf("%s", s);
  fflush(stdout);
}

void showHexInt(unsigned x) {
  char buf[sizeof(x)*2];
  int i= 0;
  do {
    buf[i++]= x & 0x0f;
    x >>= 4;
  } while (x!=0);
  do {
    int h= buf[--i];
    if (h<10)
      showChar(h+'0');
    else
      showChar(h-10+'a');
  } while (i>0);
}

void showHexa(void *ptr, int nbytes) {
  char *p= ptr;
  while (nbytes--) {
    int b= *p++;
    int up= (b & 0xf0) >> 4;
    int down= b & 0xf;
    showChar(up<=9 ? '0'+up : 'a'+up-10);
    showChar(down<=9 ? '0'+down : 'a'+down-10);
    showChar(' ');
  }
}

void showInt(int num) {
  printf("%d", num);
  fflush(stdout);
}

void showUInt(unsigned num) {
  printf("%ud", num);
  fflush(stdout);
}

void showLongLong(long long num) {
  printf("%lld", num);
  fflush(stdout);
}

void showULongLong(unsigned long long num) {
  printf("%llu", num);
  fflush(stdout);
}

int readLine(char *lin, int tam) {
  fgets(lin, tam, stdin);
  return strlen(lin);
}

void stop(int rc) {
  char *msg= "Program finished\n";
  char *rcmsg= "Return code= ";
  showStr(msg);
  if (rc!=0) {
    showStr(rcmsg);
    showInt(rc);
    showChar('\n');
  }
}
