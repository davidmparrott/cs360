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
#include <math.h>
#include "./hashtable.h"

/*
  hash uses Bernstein's hashing algorithm
 */
unsigned hash ( hashTable * table, void * key ) {
  unsigned char * hash = key;
  unsigned h = 0;
  for ( int i = 0; i <  strlen( key ); i++ ) {
    h = 33 * h + hash[i];
  }
  return h % table -> size;
}

/*
  init creates an empty hash table of size numbuckets
 */
hashTable * init ( int numBuckets ) {
  if ( numBuckets < 1 )
    return NULL;
  hashTable * table;
  if ( ( table = malloc ( sizeof ( hashTable ) ) ) == NULL ) {
    return NULL;
  }
  if ( ( table -> PTR = malloc ( sizeof ( node * ) * numBuckets ) ) == NULL ) {
    return NULL;
  }
  for ( int i = 0; i < numBuckets; i++ ) {
    table -> PTR[i] = NULL;
  }
  table -> size = numBuckets;
  table -> entries = 0;
  return table;
}

/*
  search returns a pointer to a node that contains a key found
  in a hash table. If the key is not found it returns NULL
 */
void * search ( hashTable * table, char * key ) {
  unsigned hc = ( hash ( table, key ) );
  node * n = ( table -> PTR[ hc ] );
  while ( n != NULL ) {
    if ( strcmp ( key, n -> key ) == 0 ) {
      return &n -> val;
    } else {
      n = n-> child;
    }
  }
  return NULL;
}

/*
  insert adds a key/value pair into a hash table. val may
  be any type but key is assumed to be a string
 */
void insert ( hashTable * table, char * key, void * val ) {

	float lf;
	if ((lf = loadFactor(table )) >= 0.75) {
		reHash ( table );
	} 

	int hc = ( hash ( table, key ) );
	node * temp1 = NULL;
	node * temp2 = NULL;

	if ( table -> PTR[ hc ] == NULL ) {
		node * new = (node *) malloc(sizeof(node));		
		new -> val = val;
		new -> key = key;
		new -> child = NULL;		
		table -> PTR[ hc ] = new;
  		( table -> entries )++;
	}

	temp1 = table -> PTR[hc];
	while( temp1 != NULL){
		temp2 = temp1;
		temp1 = temp1 -> child;
	}

	node * insert = (node *) malloc(sizeof(node));
	insert -> val = val;
	insert -> key = key;
	insert -> child = NULL;
	temp2 -> child = insert;
	/*
  else {
    temp -> child = table -> PTR[ hc ];
    table -> PTR[ hc ] = new;
    ( table -> entries )++;
  }
*/
}

/*
  calculates the load factor of a hash table by comparing the number
  of entries to the number of buckets. 
 */
float loadFactor ( hashTable * table ) {
  if ( ( table -> entries ) == 0 ) {
    return 0;
  } 
  else {
    float LF = ( (double)( table -> entries ) / ( table -> size ) );   
    return LF;
  }
  return 0;
}

/*
  Checks an int p for primeness. Used by isPrime()
 */
int isPrime ( int p ) {
  int prime = 0;
  if ( p % 2 == 0 )
    prime = 1;
  else {
    for ( int i = 3; i <= sqrt ( p ) && prime == 0; i += 2 ) {
      if ( p % i == 0 ) 
	prime = 1;
    }
  }
  return prime;
}

/*
  returns the next largest prime
 */
int nextPrime ( int old ) {
  int temp = ( old + old );
  if ( temp % 2 == 0 ) {
    temp++;
  }
  while ( isPrime ( temp ) == 1 ) {
    temp += 2;
  }
  return temp;
}

/*
  creates a new hash table with size being the next largest prime
  number up from the table's current size. Does not return
  anything because the information at the pointer is changed
 */
void reHash ( hashTable * table ) {
  int size = nextPrime ( table -> size );
  hashTable * old = table;
  hashTable * new = init ( size );
  node * temp = NULL;
  for ( int i = 0; i < table -> size; i++ ) {
    temp = old -> PTR[i];
    while ( temp != NULL ) {
      insert ( new, temp -> key, temp -> val );
      temp = temp -> child;
    }
  }
  free ( temp );
  free ( old -> PTR );
  *old = *new;
  free ( new );
}

/*
  applies some function to every node in a hash table
 */
void apply ( hashTable * table, void ( * f ) ( node * ) ) {
  node * temp;
  temp = malloc ( sizeof ( node ) );
  for ( int i = 0; i < table -> size; i++ ) {
    temp = table -> PTR[i] ;
    while ( temp != NULL ) {
      f ( temp );
      temp = temp -> child;
    }
  }
  free ( temp );
}
