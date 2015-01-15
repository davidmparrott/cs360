David Parrott
cs360 Spring 2014
Assignment 1

Project Explanation:
wordpairs reads text files int a hash table for frequency counting.
After all word pairs are read into the hash table, the table is
converted to a flat array and passed to the c-library qsort routine.
wordpairs assumes a routine exists to retrieve the next word at
/cs_Share/class/cs360

To Compile:
make

To Execute:
./wordpairs -<number of word pairs> <filename1> <filename2> ...