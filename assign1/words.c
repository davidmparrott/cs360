/*
  David Parrott
  cs360 Spring 2014
  assignment 1
  11239947
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"
#include "getWord.h"

#define MAX_WORD_SIZE 256
/*MAX_WORD_SIZE can be chaged at any time to accomodate strings or any size */

/*
helper function to print key value pairs	
*/
void print(node * node) {
  printf("Val: %10d \t\tKey: %s\n", (  node -> val ), ( node -> key ) );
}

/*
helper function to convert hashtable to a linear array
*/
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

/*
helper function to compare values
assumes they are ints
*/
int compareCount ( node * a, node * b ) {
  return ( b -> val - a -> val );
}

int main ( int argc, char *argv[] ) {
  node * root;
  int * val, * start;
  root = ( node * ) malloc ( sizeof ( node ) );
  
/*set initial values */
  val = start = 1;
  root -> val = start ;
  root -> child = NULL;
  
  char * prevWord = (char *) malloc ( sizeof ( char *));
  char * currWord = (char *) malloc ( sizeof ( char *));
  char * STRING[MAX_WORD_SIZE];
  
/*
wordpairs requires a particular input format and checks that first
*/

  if ( argc <= 2 ){
    printf ( "usage: %s filename\n", argv[0] );
  }
  else{

    char * check = * ( argv [ 1 ] );
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
	prevWord = getNextWord( file );	  
	while ( ( currWord = getNextWord( file ) ) != NULL ){
	  float lf;
	  if ((lf = loadFactor(table )) >= 0.75) {
	    reHash ( table );
	  } 
	  if ( currWord[0] != '\0' ) {
	    sprintf( STRING, "%s %s", prevWord, currWord );
	    prevWord = currWord;
	    val = search ( table, strdup ( STRING ) );
	    if ( val == NULL) {
	      insert ( table, strdup ( STRING ), start );
	    }
	    else {
	      ( *val ) ++;
	    }
	  }
	}
	free(prevWord);
	free(currWord);
	fclose( file );
      }
    }
/*
  create flat array from table for sorting
*/
    node * array = toArray ( table );
    qsort ( array, table -> entries, sizeof ( node ), compareCount );
    node * tempo;
    
/*
  print number of word pairs specified at run time
*/
    for ( int j = 0; j < lines; j++ ) {
      tempo = & array[ j ];
      printf("Val: %10i \t\tKey: %s\n", ( (int) tempo -> val ), ( tempo -> key ) );
    
    }
  }
  return 0;
}
