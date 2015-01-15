#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
  srand(time(NULL));
  int r;
  for(int i = 0; i < 101; i++){
    r = rand();
    printf("%i\n",r);
  }
  return 0;
}
