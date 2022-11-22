#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "term.h"

#include "sort.h"

void showBin(uint x) {
  printf("0b");
  int bits[sizeof(uint)*8];
  int i= 0;
  for (;;) {
    bits[i]= x&1;
    x >>= 1;
    if (x==0)
      break;
    i++;
  }
  do {
    showChar('0'+bits[i--]);
  } while (i>=0);
}

int main() {
  uint refs[ ]= { 0b1011011101110001011, 0b1101111110000, 0b101111111110 };
  uint nums[ ]= { refs[2], refs[1], refs[0] };
  showStr("Desordenado:\n");
  for (int i= 0; i<3; i++) {
    showBin( nums[i] );
    showChar(' ');
  }
  showChar('\n');
  sort(nums, 3);
  showStr("Ordenado:\n");
  for (int i= 0; i<3; i++) {
    showBin( nums[i] );
    showChar(' ');
  }
  showChar('\n');
  for (int i= 0; i<3; i++) {
    if (nums[i]!=refs[i]) {
      showInt(i);
      showStr("-esimo elemento es incorrecto\n");
      exit(1);
    }
  }
  showStr("Felicitaciones!\n");
  return 0;
}
