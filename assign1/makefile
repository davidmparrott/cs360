#David Parrott
#cs360 Spring 2014
#assignment 1
#11239947

LIBDIR = -L$(GET_WORD)/lib
INCLUDEDIR = -I$(GET_WORD)/include

words: wordpairs.c hashTable.c hashTable.h 
	gcc -std=c99 $(LIBDIR) $(INCLUDEDIR) -lm -g wordpairs.c hashTable.c -o wordpairs -lget
