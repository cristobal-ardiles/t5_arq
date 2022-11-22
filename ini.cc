#include <stdio.h>

int f() {
  printf("f!\n");
  return 1;
}

int a= f();

int main() {
  printf("Hello World!\n");
  return 0;
}
