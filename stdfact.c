#include <stdio.h>
#include <stdlib.h>

#define N 80

int factorial(int x) {
  if(x<=1)
    return 1;
  else
    return x*factorial(x-1);
}

int main() {
  char lin[N+1];
  printf("? ");
  fflush(stdout);
  while (fgets(lin, N, stdin)!=NULL) {
    int n= atoi(lin);
    if (n<0)
      break;
    printf("factorial(%d) is %d\n? ", n, factorial(n));
    fflush(stdout);
  }
  return 0;
}
