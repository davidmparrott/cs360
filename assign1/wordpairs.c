#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"
#include "getWord.h"



/*
helper function to compare values
assumes they are ints
*/

int compareCount( const void * a, const void * b){
	int* aVal = (*(node**)a) -> val;
	int* bVal = (*(node**)b) -> val;
	printf("a: %i b: %i \n", aVal, bVal);
	return bVal - aVal;
}

/*
helper function to print key value pairs	
*/
void print(node * node) {
  printf("Val: %10d \t\tKey: %s\n", (  node -> val ), ( node -> key ) );
}

/*
helper function to convert hashtable to a linear array
*/
void sortPrintArray ( hashTable * table, int lines ) {
	node ** array = malloc(table -> entries * sizeof(node *) + 1);	
	node * temp1;
	node * temp2;
	int j = 0;

	for(int i = 0; i < table -> entries; i++){
		array[i] = NULL;
	}
  	for ( int i = 0; i < table -> size; i++ ) {
    	temp1 = table -> PTR[i];
    	while ( temp1 != NULL ) {
    		array[j] = temp1;
			temp1 = temp1 -> child;
    		j++;
    	}
  	}

	qsort ( array, table -> entries, sizeof ( node *), compareCount );
    for ( int j = 0; j < lines; j++ ) {
      printf("Key: %10s \t\tVal: %d\n", array[j] -> key, *((int*)array[j]->val) );
    
    }
}

node * toArray ( hashTable * table ) {
  node * array; 
  if ( ( array = malloc ( sizeof (  node ) * table -> entries ) ) == NULL ) {
    return NULL;
  }
  node * temp;
  temp = malloc ( sizeof ( node ) );
  int j = 0;
  for ( int i = 0; i < table -> size; i++ ) {
    temp = table -> PTR[i];
    while ( temp != NULL ) {
      array[j] = * temp;
      temp = temp -> child;
      j++;
    }
  }
  return array;
}


int main ( int argc, char *argv[] ) {
 
/*set initial values */
	char * prevWord = NULL;
	char * currWord = NULL;
	char * nextWord = NULL;
	int * val = NULL;
  
/*
wordpairs requires a particular input format and checks that first
*/

  if ( argc <= 2 ){
    printf ( "usage: %s filename\n", argv[0] );
  }
  else{

    int check = * ( argv [ 1 ] );
    if ( check != '-' ) {
      printf("usage: words -<number of lines to print> <filename.txt>\n");
      exit ( -1 );
    }
    int lines;
    sscanf ( argv [ 1 ], "-%d", &lines );
    if ( ! ( lines > 0 ) ) {
      printf("number of lines to print must be > 0\n");
      exit ( -1 );
    }
    hashTable * table = init ( 101 );
/*
  start of code that actually opens files and reads words
  into a hash table. Will step through file names passed 
  in argv[] and output top word pairs after all have
  files have been read in.
*/    
    for (int i = 2; i < argc; i++){
      FILE *file = fopen( argv[i], "r" );
      if ( file == 0 ) {
	printf ( "Could not open file\n" );
	exit (-1);
      }
      else {
/*
  first time through a file we get the first word into 
  prevWord since we are reading in word pairs
*/
	nextWord = getNextWord(file);
	currWord = nextWord;
	while ( ( nextWord = getNextWord( file ) ) != NULL ){

	  if ( currWord[0] != '\0' ) {
		prevWord = currWord;
		currWord = nextWord;

		char tempStr[strlen(prevWord) + strlen(currWord) + 2];
		strcpy(tempStr, prevWord);
		strcat(tempStr, " ");
		strcat(tempStr, currWord);

		char * key = malloc(sizeof(char)*(strlen(tempStr)+1));

		snprintf(key, strlen(tempStr) + 1, "%s", tempStr);
		free(prevWord);

	    val = (int *)search ( table, key );
	    if ( val == NULL) {
			int * new = (int *) malloc(sizeof(int));
			*new = 1;
	      insert ( table, key , new );
	    }
	    else {
	      ( *val ) ++;
	    }
	  }
	}
	free(currWord);
	fclose( file );
      }
    }
	//sortPrintArray(table, lines);
	node * array = toArray(table);
	qsort(array, table -> entries, sizeof(node), compareCount); 

	free(table);
/*
  print number of word pairs specified at run time
*/

  }
  return 0;
}
