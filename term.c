#include <unistd.h>
#include "term.h"

volatile signed char *_term= (signed char*)0x1ffffffc;
signed char _dummy_term;

void showChar(char c) {
  *_term= c;
}

void showStr(const char *s) {
  volatile signed char *term= _term;
  for (const char *r= s; *r; r++)
    *term= *r;
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
  if (num<0) {
    *_term= '-';
    num= -num;
  }
  showUInt(num);
}

void showUInt(unsigned num) {
  volatile signed char *term= _term;
  char buf[20];
  char *p= buf;
  unsigned r= num;
  do {
    int rem= r%10;
    r /= 10;
    char c= '0'+rem;
    *p++= c;
  } while (r!=0);
  do {
    p--;
    *term= *p;
  } while (p>buf);
}

void showLongLong(long long num) {
  if (num<0) {
    *_term= '-';
    num= -num;
  }
  showULongLong(num);
}

void showULongLong(unsigned long long num) {
  volatile signed char *term= _term;
  char buf[30];
  char *p= buf;
  unsigned long long r= num;
  do {
    int rem= r%10;
    r /= 10;
    char c= '0'+rem;
    *p++= c;
  } while (r!=0);
  do {
    p--;
    *term= *p;
  } while (p>buf);
}

char readChar() {
  volatile signed char *term= _term;
  signed char c;
  do {
    c= *term;
  } while (c<0);
  *term= c;
  return (char)c;
}

int readLine(char *lin, int tam) {
  char c;
  int cnt= 0;
  tam--;
  do {
    c= readChar();
    *lin++= c;
    cnt++;
  } while (c!='\n' && cnt<tam);
  *lin= 0;
  return cnt;
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
  while (1);
    ;
}

#if 0

void _exit(int rc);

void exit(int rc) {
  _exit(rc);
}

void _exit(int rc) {
  char *msg= "Program aborted\n";
  char *rcmsg= "Return code= ";
  showStr(msg);
  showStr(rcmsg);
  showInt(rc);
  showChar('\n');
  while (1);
    ;
}

#endif

ssize_t write(int fd, const void *buf, size_t num) {
  showStr("writing\n");
  if (fd==1 || fd==2) {
    for (int i= 0; i<num; i++) {
      showChar(((char*)buf)[i]);
    }
  }
  return num;
}
