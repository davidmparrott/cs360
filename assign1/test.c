#include <stdlib.h>
#include <stdio.h>


int main (int argc, char * argv[] ) {
  for ( int i = 1; i < argc; i++ ) {
    /*
    int opt = atoi( argv[i] );
    if ( opt < 0 ) {
      opt = opt * -1;
    }
    printf("arg %i: %i \n",i, opt );
    */
    /*
    int opt = strtol ( argv[i], '\0', 0 );
    printf("arg %i: %i \n",i, opt );
    */
    char * check = *(argv[i]);
    if ( check != '-'){
      printf("nope\n");
    }
    int opt;
    sscanf( argv[i], "-%d", &opt);
    if (!( opt > 0) ) {
      printf("nope\n");
    }
    printf("arg %i: %i \n",i, opt );
   
  }
  return 0;
}
