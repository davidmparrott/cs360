/*
David Parrott
11239947
CS 360
Assignment 4
connect.c
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*
Assumes that input format is of the type
<file1> <options> : <file2> <options>
with no error checking
Replaces colon with null pointer so that execvp can get passed argv[]
colon holds where the colon was originally located in argv[]
*/
int main(int argc, char** argv){
  int colon = 0;
  int fd[2];

/*
find and replace colon with NULL
cache location in colon
*/  
  for(int i = 0; i < argc; i++){
    if(strcmp(argv[i],":") == 0){
      colon = i;
      argv[i] = NULL;
      break;
    }else{
      continue;
    }
  }

  pipe(fd);
  int rdr = fd[0], wtr = fd[1];

  if(fork()){					//child writes
    close(wtr);
    close(0);
    dup(rdr);
    execvp(argv[colon+1], argv+colon+1);
    fprintf(stderr, "%s: exec failed \n", argv[0]);
  }else{						//parent writes
    close(rdr);
    close(1); 
    dup(wtr);
    execvp(argv[1], argv+1);
    fprintf(stderr, "%s: exec failed \n", argv[0]);	
  }
  return (1);
}
