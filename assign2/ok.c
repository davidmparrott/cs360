/*
  David Parrott
  11239947
  CS 360 Assignment 2
  ok.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*
Smaller dictionary 'tiny' used for testing
*/
#define DICTIONARY "/cs_Share/class/cs360/lib/webster"
//#define DICTIONARY "/cs_Share/class/cs360/lab/io/tiny"

int ok(int fp, char * word);

/*
Opens the selected dictionary and stores that location as
fp. Error checks are performed. If all checks are passed
the search string enetered is stored in word[]. Any 
remaining space in word is filled with space characters
and a null terminator is appended to the end.
*/
int main(int argc, char* argv[]){
  int fp;
  char word[15];

  if((fp = open(DICTIONARY, O_RDONLY, 0)) < 0){
    fprintf(stderr, "unable to open dictionary\n");
    exit(1);
  }
  if(argc != 2){
    fprintf(stderr, "usage: ./ok <word>\n");
    exit(1);
  }else{
    int length = strlen(argv[1]);
    if(length > 15){
      fprintf(stderr, "word length can not exceed 15 characters\n");
      exit(1);
    }
    int spaces = 16 - length;
    for(int i = 0; i < length; i++){
      word[i] = argv[1][i];
    }
    word[15] = '\0';
    for(int j = length; j < 15; j++){
      word[j] = ' ';
    }
    int check = ok(fp, word);
    if(check == 1){
      fprintf(stdout, "yes\n");
    }else{
      fprintf(stdout, "no\n");
    } 
  }
  return(0);
}

/*
fp points to a location in the dictionary. word is the search
string. Binary search is used to efficiently search the
dictionary, assumed to be in lexagraphic order. 
*/
int ok(int fp, char * word){
  int top, bot, end, check, mid, lines;
  top = bot = end = check = mid = lines = 0; 
  char buff[15];
  
  end = lseek(fp, end, SEEK_END);
  lines = end / 16;
  top = lines;
/*
Binary search loop. Continues until location boundaries
pass each other.
*/  
  while( top > bot){
    mid = (top + bot) / 2;
    lseek(fp, mid*16, SEEK_SET);
    read(fp, buff, 16);
    buff[15] = '\0';
    check = strcmp(buff, word);
    if(check == 0){
      return 1 ;
    }
    else if(check < 0){
      bot = mid + 1;
    }else{
      top = mid -1;
    }
  }
  return 0;
}
