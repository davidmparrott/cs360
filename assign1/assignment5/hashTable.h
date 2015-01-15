#ifndef HASHTABLE_H_INCLUDED
#define HASHTABLE_H_INCLUDED

/*each node must contain a key, a value and a pointer to the
next node in the list */
typedef struct Node {
  char * key;
  void * val;
  struct Node * child;
} node;
/*the table must have size for the number of buckets available,
 entries for the number of entries contained in the table and
and array that contains individual nodes, or even lists of nodes*/
typedef struct hashTable {
  int size;
  int entries;
  struct Node **PTR;
} hashTable;
/*protoypes of functions for hash table operations. */
unsigned hash( hashTable *, void * );
hashTable * init ( int );
void * search ( hashTable * , char * );
void insert ( hashTable *, char *, void * );
float loadFactor ( hashTable * );
int isPrime ( int );
int nextPrime ( int );
void reHash ( hashTable * );
void apply ( hashTable *, void ( node * ) );
#endif
