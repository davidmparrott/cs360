/*
David Parrott
11239947
CS 360
Assignment 5
warn.c
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

void pauseHandler(int code);
void ctrlHandler(int code);


int main(int argc, char** argv){
  int fd[2];

  pipe(fd);
  int rdr = fd[0], wtr = fd[1];

	int childID;

  if(childID = fork()){					//child writes
    char buffer[256];
    close(wtr);
    while(1){
      signal(SIGINT, pauseHandler);
      pause();
      printf("> \n");
      scanf("%[^\n]s", &buffer);
      write(wtr, buffer, 256);
      if(strcmp(buffer, "exit")){exit(1);}
      kill(childID, SIGFPE);
    }
  }else{						//parent writes
    close(wtr);
    char buffer[256];
    signal(SIGINT, SIG_IGN);
    signal(SIGFPE, pauseHandler);
    signal(SIGLRM, pauseHandler);
    pause();
    while(1){
      read(rdr, buffer, 256);
      printf("%s\n", buffer);
      controler = 1;
      while(controler){
        signal(SIGINT, ctrlHandler);
        printf("%s\n", buffer);
        alarm(5);
        pause();
      }
      signal(SIGFPE, pauseHandler);

      pause();
    }
  }
  return (1);
}

void ctrlHandler(int code){

}

void pauseHandler(int code){
  signal(SIGALRM, pauseHandler);
  return;
}