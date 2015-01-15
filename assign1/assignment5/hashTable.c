#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "./hashtable.h"
/*hash generates a unique number based on Bernstein's hash code
  implementation. */
unsigned hash ( hashTable * table, void * key ) {
  unsigned char * hash = key;
  unsigned h = 0;
  for ( int i = 0; i <  strlen( key ); i++ ) {
    h = 33 * h + hash[i];
  }
  return h % table -> size;
}
/*init is a method of initializing a new table with null pointers.
  an int is passed into init, which will be the number of buckets
  in the hash table.  */
hashTable * init ( int numBuckets ) {
  /*If you try to form a table with less than 1 bucket a NULL is 
    returned. NULL is chosen instead of non-zero into since the
    method must return a hash table.*/
  if ( numBuckets < 1 )
    return NULL;
  /*create a pointer to a new hash table */
  hashTable * table;
  /*allocate memory on the heap for the table. If you are unable
    to malloc then a NULL is returned.*/
  if ( ( table = malloc ( sizeof ( hashTable ) ) ) == NULL ) {
    return NULL;
  }
  /*allocate space on the heap for enough nodes to fill the array
    with nodes equal to the number of buckets asked for. If this fails
    return a NULL.*/
  if ( ( table -> PTR = malloc ( sizeof ( node * ) * numBuckets ) ) == NULL ) {
    return NULL;
  }
  /*walk the array and put NULL pointers into each bucket */
  for ( int i = 0; i < numBuckets; i++ ) {
    table -> PTR[i] = NULL;
  }
  /*set the size attribute equal to the number of buckets */
  table -> size = numBuckets;
  /*set the number of entries to 0 since nothing has been
    inserted into the table yet. */
  table -> entries = 0;
  return table;
}

/*search returns a pointer to the value associated with a given key.
void * is used to enable a more generic implementation of hashTable */
void * search ( hashTable * table, char * key ) {
  /*hc will contain the hash code for a given key */
  unsigned hc = ( hash ( table, key ) );
  /*n will be a pointed to the root node of the list rooted
    in a bucket determined by the hash code*/
  node * n = ( table -> PTR[ hc ] );
  /*if n != NULL then that bucket already contains a list
   of nodes. to check is a key is already contained in the
   list (and the hash table by enxtension) we must walk
   the list comparing the key passed into search with the
   key at each node. A pointer is passed back if the key
   is found.*/
  while ( n != NULL ) {
    if ( strcmp ( key, n -> key ) == 0 ) {
      return &n -> val;
    } else {
      /*step to the next node */
      n = n-> child;
    }
  }
  /*if the key is not contained within the table return NULL */
  return NULL;
}
/*insert takes a has table, key and value, but returns nothing
a node will be created and space allocated on the heap for it.
the key and value passed to insert will be assigned to the
new node. 
IMPORTANT!! insert does no check to see if the node is
already in the table. It is assumed that by calling insert
you have already verified that the key value pair is
unique. This is done to avoid redundant checks*/
void insert ( hashTable * table, char * key, void * val ) {
  /*create a pointer to a new node */
  node * new;
  /*get a hash code for the key */
  int hc = ( hash ( table, key ) );
  /*create space on the heap and return NULL if that fails */
  if ( (  new = malloc ( sizeof ( node ) ) ) == NULL )
    return;
  /*set the new node's key and value to be those passed to insert */
  new -> key = strdup ( key ) ;
  new -> val = val;
  /*if no list exists in the array then set the new node to be
    the root of a new list and increment the tables number of
    entries*/
  if ( table -> PTR[ hc ] == NULL ) {
    table -> PTR[ hc ] = new;
    ( table -> entries )++;
    ( table -> PTR[ hc ] ) -> child = NULL; 
  }
  /*if there is already a list in that bucket insert
    will set the new node as the root and the old root
    becomes the child of that node, then increments entries*/ 
  else {
    new -> child = table -> PTR[ hc ];
    table -> PTR[ hc ] = new;
    ( table -> entries )++;
  }
}
/*loadfactor returns a float that corresponds to the ratio of
  the number of entries to the size of the table.  */
float loadFactor ( hashTable * table ) {
  /*if there are no entries just return 0 */
  if ( ( table -> entries ) == 0 ) {
    return 0;
  } 
  /*the number of entries is cast as a double so that precision
    is not lost by dividing two ints */
  else {
    float LF = ( (double)( table -> entries ) / ( table -> size ) );   
    return LF;
  }
  return 0;
}
/*function to get the next prime number greater than the number
  passed to the function */
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
/*helper for isPrime */
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
/*reHash is passed a hash table and creates a new hash table
that has twice the number of buckets, rounded up to the nearest
prime number. The function return type is void because reHash
updates the pointer to the old table*/
void reHash ( hashTable * table ) {
  /*get a number for the new table size */
  int size = nextPrime ( table -> size );
  /*old will hold on to the previous hash table while data
    is re-hashed*/
  hashTable * old = table;
  /*gets an empty table with the new size that we can insert into */
  hashTable * new = init ( size );
  /*creates a pointer to a new node that will be used as a vehicle
    to move data from the old hash table to the new one */
  node * temp = NULL;
  /*in order traversal of the old table */
  for ( int i = 0; i < table -> size; i++ ) {
    /*temp gets the root of each list in the old table */
    temp = old -> PTR[i];
    /*if temp != NULL then a list exists and must be inserted into
      the new table */
    while ( temp != NULL ) {
      insert ( new, temp -> key, temp -> val );
      temp = temp -> child;
    }
  }
  /*free the temp node and the old table pointer array */
  free ( temp );
  free ( old -> PTR );
  /*update pointers */
  *old = *new;
  /*free the memory allocated during the rehashing process. */
  free ( new );
}
/*apply performs an in order traversal of a hash table and applies
  a function passed as an argument to each node individually*/
void apply ( hashTable * table, void ( * f ) ( node * ) ) {
  /*pointer to a temporary node */
  node * temp;
  /*allocated space on the heap */
  temp = malloc ( sizeof ( node ) );
  /*check each list in the table. If it is not null then walk
    the list and pass the nodes one at a time to the function */
  for ( int i = 0; i < table -> size; i++ ) {
    temp = table -> PTR[i] ;
    while ( temp != NULL ) {
      f ( temp );
      temp = temp -> child;
    }
  }
  /*free temp */
  free ( temp );
}
