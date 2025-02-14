	/*
  David Parrott
  cs360 Spring 2014
  assignment 1
  11239947
 */
#ifndef HASHTABLE_H_INCLUDED
#define HASHTABLE_H_INCLUDED

typedef struct Node {
  char * key;
  void * val;
  struct Node * child;
} node;

typedef struct hashTable {
  int size;
  int entries;
  struct Node **PTR;
} hashTable;

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
