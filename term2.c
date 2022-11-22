#include <unistd.h>
#include "term.h"
#include "intr-cntl.h"

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

#define KBDBUFSIZ 80

static char kbd_buf[KBDBUFSIZ];
static int kbd_first= 0, kbd_last= 0, kbd_cnt= 0;

void kbd_hdlr(void) {
  volatile signed char *term= _term;
  while (kbd_cnt<KBDBUFSIZ) {
    signed char c= *term;
    if (c<0)
      break;
    showChar(c); // Echo character on display
    kbd_buf[kbd_last]= c;
    kbd_last= (kbd_last+1)%KBDBUFSIZ;
    kbd_cnt++;
  }
  if (kbd_cnt>=KBDBUFSIZ) {
    // Disable keyboard interrupt if buffer is full
    set_irq_handler(1, NULL);
  }
}

static char fastReadChar() {
  volatile signed char *term= _term;
  signed char c;
  if (kbd_cnt>0) {
    c= kbd_buf[kbd_first];  // Buffer not empty: get char from buffer
    kbd_first= (kbd_first+1)%KBDBUFSIZ;
    kbd_cnt--;
  }
  else {
    do {                    // Buffer empty: do busy waiting until
      c= *term;             // the user types a key
    } while (c<0);
    *term= c;               // Echo character on display
  }
  return (char)c;
}

char readChar() {
  set_irq_handler(1, NULL); // Disable keyboard interrupts
  char c= fastReadChar();
  set_irq_handler(1, kbd_hdlr); // Enable keyboard interrupts
  return c;
}

int readLine(char *lin, int tam) {
  set_irq_handler(1, NULL); // Disable keyboard interrupts
  char c;
  int cnt= 0;
  tam--;
  do {
    c= fastReadChar();
    *lin++= c;
    cnt++;
  } while (c!='\n' && cnt<tam);
  *lin= 0;
  set_irq_handler(1, kbd_hdlr); // Enable keyboard interrupts
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
