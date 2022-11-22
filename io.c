#include <stdlib.h>
#include <stdint.h>

int main() {
  volatile char *port= (char*)(intptr_t)0xffff0004;
  int status= 0;

  for (;;) {
    while ((*port & 1) == 0)
      ;
    status= !status;
    *port= status;

    while ((*port & 1) == 1)
      ;
  }
}
