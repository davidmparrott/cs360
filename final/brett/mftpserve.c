/**
   Brett Carter
   brett.carter@wsu.edu
   CS360 - Final Project
   MFTPserver
 **/


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>

#define MY_PORT_NUMBER 49998


void GP(int source, int dest);

int main(int argc, char** argv){
  int connectfd;
  socklen_t length = sizeof(struct sockaddr_in);
  struct sockaddr_in servAddr;
  struct sockaddr_in clientAddr;
  struct hostent* hostEntry;
  //create the socket.
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0){
    perror("Error: ");
    exit(1);
  }
  //bind socket to port.
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(MY_PORT_NUMBER);
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if((bind(listenfd,(struct sockaddr*)&servAddr, sizeof(servAddr))) < 0 ){
    perror("bind");
    exit(1);
  }
  if( (listen(listenfd, 4)) < 0 ){
    perror("Error: ");
    exit(1);
  }
  
  while(1){
    int connectfd= accept(listenfd, (struct sockaddr*) &clientAddr, &length);
    printf("connection established\n");
    hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET);
    char* hostName = hostEntry->h_name;
    if(hostName == NULL){
      perror("Error: ");
      exit(1);
    }
    printf("hostname: %s\n", hostName);
    int procID;
    if( (procID = fork()) ){
      //parent
      wait(NULL);
    }
    else{
      //child
      struct sockaddr_in datAddr;
      struct sockaddr_in portAddr;
      int datafd;
      int dfd;
      while(1){
	char arg[2];
	char N;
	char optbuff[512];
	read(connectfd, &N, 1);
	arg[0] = N;
	arg[1] = '\0';
	printf("Got a %s\n", arg);
	
	/* got a Q command */
	if(strcmp(arg, "Q") == 0){
	  read(connectfd, &N, 1);
	  write(connectfd, "A\n", 2);
	  printf("disconnecting\n");
	  close(connectfd);
	  kill(procID, SIGTERM);
	}
	
	/* got a D command */
	else if( strcmp(arg, "D") == 0){
	  printf("in d\n");
	  read(connectfd, &N, 1);
	  datafd = socket(AF_INET, SOCK_STREAM, 0);
	  
	  memset(&datAddr, 0, sizeof(datAddr));
	  datAddr.sin_family = AF_INET;
	  datAddr.sin_port = 0;
	  datAddr.sin_addr.s_addr = htons(INADDR_ANY);
	  bind(datafd, (struct sockaddr*)&datAddr, sizeof(datAddr));
	  //get socket name for client.
	  if(getsockname(datafd, (struct sockaddr*)&portAddr, &length) == -1){
	    printf("could not get the socket name\n");
	    char errbuff[512];
	    strcpy(errbuff, "E");
	    strcat(errbuff, "Error: Could not get socket name\n");
	    write(connectfd, errbuff, strlen(errbuff));
	  }
	  //send client the socket name.
	  char buff[512];
	  sprintf(buff, "A%u\n", ntohs(portAddr.sin_port));
	  printf("portname: %s\n", buff);
	  write(connectfd, buff, strlen(buff));
	  listen(datafd, 1);
	  //accept the data connection.
	  dfd = accept(datafd, (struct sockaddr*)&portAddr, &length);
	}
	
	/* got a L command */
	else if( strcmp(arg, "L") == 0){
	  int CID;
	  read(connectfd, &N, 1);
	  if( (CID = fork()) ){
	    wait(CID);
	  }
	  else{
	    if( dup2(dfd, 1) < 0){
	      perror("Error: ");
	      char errbuff[512];
	      strcpy(errbuff, "E");
	      strcat(errbuff, "Could not change from stdout to socket");
	      strcat(errbuff, "\n");
	      write(connectfd, errbuff, strlen(errbuff));
	      exit(1);
	    }
	    write(connectfd, "A\n", 2);
	    execlp("ls", "ls", "-l", (char*)NULL );
	    
	    exit(1);
	  }
	  close(dfd);
	}

	/* got a C change directory */
	else if( strcmp(arg, "C") == 0 ){
	  char dirPath[512];
	  char* temp;
	  read(connectfd, dirPath, 511);
	  temp = strtok(dirPath,"\n");
	  printf("path: %s\n", dirPath);
	  if(chdir(dirPath) < 0){
	    perror("Error: ");
	    char errbuff[512];
	    strcpy(errbuff, "E");
	    strcat(errbuff, "Error: not a legal pathname\n");
	    write(connectfd, errbuff, strlen(errbuff));
	    exit(1);
	  }
	  write(connectfd, "A\n", 2);
	}
	
	/* got a G get a file from args */
	else if( strcmp(arg, "G") == 0){
	  printf("in G\n");
	  char path[512];
	  char* temp;
	  int file;
	  read(connectfd, path, 511);
	  //remove return char.
	  temp = strtok(path,"\n");
	  printf("path is: %s\n", temp);
	  
	  if( (file = open(temp, O_RDONLY, S_IRUSR)) < 0){
	    perror(" Error: ");
	    char errbuff[512];
	    strcpy(errbuff, "E");
	    strcat(errbuff, "Error could not open file\n");
	    write(connectfd, errbuff, strlen(errbuff));
	    exit(1);
	  }
	  write(connectfd, "A\n", 2);
	  GP(file, dfd);
	  close(file);
	  close(dfd);
	}

	/* got a P. read the file data */
	else if( strcmp(arg, "P") == 0 ){
	  printf("in P\n");
	  char filename[512];
	  char* temp;
	  int file;
	  read(connectfd, filename, 511);
	  temp = strtok(filename, "\n");
	  printf("name of file: %s\n", temp);
	  
	  if( (file = open(temp, O_EXCL|O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0 ){
	    perror("Error: ");
	    char errbuff[512];
	    strcpy(errbuff, "E\n");
	    strcat(errbuff, "file name already exists. could not create\n");
	    write(connectfd, errbuff, strlen(errbuff));
	  }
	  write(connectfd, "A\n", 2);
	  GP(dfd, file);
	  printf("file transfer complete\n");
	  
	  close(dfd);
	}
      }//end of child while loop.
    }
  }//end of while loop.
  close(connectfd);
}



void GP(int source, int dest){
  char buff[512];
  int bytes;
  while((bytes = read(source, &buff, 512)) > 0){
    write(dest, buff, bytes);
  } 
}
