#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "./hashtable.h"

#define MAX_WORD_SIZE 256
/*MAX_WORD_SIZE can be chaged at any time to accomodate strings or any size */

char* getNextWord ( FILE *fd ) {
  /*we need a couple variables. j for a counter. c to hold individual
    characters. string to hold the string array*/
  int j = 0;
  char c;
  char string[MAX_WORD_SIZE];
  /*a hierarchy of checks is necessary for this to funciton properly.
    We first check for EOF to ensure there is more text to read.*/
  while ( ( c = fgetc( fd ) ) != EOF ){
    /*We know that there must be more to read so we next test it c
      is a whitespace character. If so we've reached the end of a
      potential word and will return string using strdup()*/
    if ( isspace( c ) ) {
      string[j] = '\0';
      return ( char* ) strdup ( string );
    }
    /*The final check will remove punctuation and seperate words
      into chunks of MAX_WORD_SIZE - 1*/
    if ( isalnum ( c ) && ( j <= ( MAX_WORD_SIZE - 1 ) ) ) {
      /*We now know we have we have an alpha numeric character. We
	place it in the next available slot in the string array 
	(which is being tracked by the counter, j) and increment j*/
      string[j] = tolower ( c );
      j++;
    }else {
      //j++;
      string[j] = '\0';
      return ( char* ) strdup ( string );
    }
  }
  /*The NULL return will be utilized by main to know when EOF has 
    been reached*/
  return NULL;
}
/*print is a sinple function that has a node passed to it. We
assume that the key is a string and the val is an int. This
will print the val (we are interpreting it as a frequency
count) and that the key (string). this function exists primarily
as a simple way to test the unctionality of the apply method.
Since the node will be passing a void * print casts the val
as an int */
void print(node * node) {
  printf("Val: %10i \t\tKey: %s\n", ( (int) node -> val ), ( node -> key ) );
}
/*
int cmpKeys ( node * a, node * b ) {
  return strcmp ( a -> key, b -> key );
}
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

int compareCount ( node * a, node * b ) {
  return ( ( int ) b -> val - ( int ) a -> val );
}

int main ( int argc, char *argv[] ) {
  /*We must create a root node*/
  node * root;
  /*and pointers to ints for passing the count */
  int * val, * start;
  /*make space on the heap for them all*/
  root = ( node * ) malloc ( sizeof ( node ) );
  val = ( int * ) malloc ( sizeof ( int ) );
  start = ( int * ) malloc ( sizeof ( int ) );
  /*set initial values */
  val = start = 1;
  root -> val = start ;
  root -> child = NULL;
  /*We have a framework to build our table out of now. By calling search,
    insert, etc we can build up the table*/
  
  char * STRING = NULL;
  /*STRING will hold the string passed from getNextWord */
  if ( argc <= 2 ) 
    /* argc should be >= 3 for correct execution */
    {
      /* We print argv[0] assuming it is the program name */
      printf ( "usage: %s filename", argv[0] );
      /*just in case you forgot to put in a number of lines and filename */
    }
  else
    {
      /*We will now check to make certain that we have formatted the command
       properly. If it is not of the anticipated format we exit with non-0
       check gets the first character of the first argument passed to main*/
      char * check = * ( argv [ 1 ] );
      /*if it is no '-' then exit with usage statement */
      if ( check != '-' ) {
	printf("usage: words -<number of lines to print> <filename.txt>\n");
	exit ( -1 );
      }
      /*We now know that it is formatted correctly and can strip out the
	number of lines we will attempt to read from the file. This will
	not be used until the entire table has been created*/
      int lines;
      /*skip over the '-' and read whatever numbers follow it. This will
       strip off any letters or special characters that follow numbers
       ie -54bacon*/
      sscanf ( argv [ 1 ], "-%d", &lines );
      /*idiot check to make sure 0 lines was not selected*/
      if ( ! ( lines > 0 ) ) {
	printf("number of lines to print must be > 0\n");
	exit ( -1 );
      }
      hashTable * table = init ( 101 );
      /*create a new hashTable. 101 is used initially since it is a prime
	number. Rehashing will attempt to keep the size as a prime*/
      for (int i = 2; i < argc; i++){
        /* We assume argv[1] is a filename to open*/
        FILE *file = fopen( argv[i], "r" );
        /* fopen returns 0, the NULL pointer, on failure */
        if ( file == 0 ) {
	  printf ( "Could not open file\n" );
	  exit (-1);
        }
        else {
	  /*We know that we have opened a file successfully. Now we must iterate
	    through it. getNextWord is designed to return null if EOF is reached
	    so a while loop testing for that suffices to iterate through a file*/
	  while ( ( STRING = getNextWord( file ) ) != NULL ){
	    /*before each insertion the load on the Hash Table will be checked.
	     This is done by calling a function within the hashTable() method
	     which will return a float of the number of entries / the size of
	     the table. 0.75 is chosen but can easily be adjusted below. 
	     IMPORTANT!! A load factor of 1 means that the table has 1 entry for
	     every available slot. This is not a problem for our implementation
	     of a Hash Table because we use seperate chaining. I have chosen
	     to keep the number low, regardless, in the event that this code
	     will be reused for a Hash Table to uses a different method to deal 
	     with collisions*/
	    float lf;
	    if ( ( lf = loadFactor ( table ) ) >= 0.75 ) {
	      reHash ( table );
	    }
	    /*getNextWord can return 'empty' strings ie a string that
	      only has \0 in it when it encounters more than one white
	      space in a row. As such we want to test that STRING 
	      contains something more than just \0*/
	    if ( STRING[0] != '\0' ) {
	      val = search ( table, strdup ( STRING ) );
	      /*Search returns a pointer to the value associatied with
		a key that is passed to it. If the key is not found
		search returns NULL. This check is then used to determine
		if an insert needs to be performed.*/
	      if ( val == NULL) {
		insert ( table, strdup ( STRING ), start );
	      }
	      /*If a key has a value associated with it already, 
		increment the key. Recall we are assuming that the values
		associated with keys are frequency counts. This behavior
		could be easily altered since hashTable() stores vals as void **/
	      else {
		( * val ) ++;
	      }
	      /*don't forget to free things up */
	      free ( STRING );
	    }
	  }
	  /*we will create an array lineraization of our hash table using
	    the toArray method created for this purpose*/
	  node * array = toArray ( table );
	  /*the array is passed to qsort, taking the number of elements 
	    directly from the table we started with
	    compareCount is a very basic method that has two nodes passed
	    to it. It subtracts the (int)value of the second from the first
	    and returns that number*/
	  qsort ( array, table -> entries, sizeof ( node ), compareCount );
	  /*holds nodes for printing*/
	  node * tempo;
	  /*recall that lines was stored before the table was even read
	    and is a number given from the command line
	    this small for loop will print the specified number of lines*/
	  for ( int j = 0; j < lines; j++ ) {
	    tempo = & array[ j ];
	    printf("Val: %10i \t\tKey: %s\n", ( (int) tempo -> val ), ( tempo -> key ) );
	  }
	  fclose( file );
        }
      }
    }
  return 0;
}
