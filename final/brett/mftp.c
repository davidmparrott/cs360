/**
Brett Carter
brett.carter@email.wsu.edu
CS360 - final project
MFTP 
 **/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#define MY_PORT_NUMBER 49999

void GP(int source, int dest);
int setupDataConnection( int fd, char* hostname);

int main(int argc, char** argv){
  int socketfd;
  struct sockaddr_in servaddr;
  struct hostent* hostEntry;
  struct in_addr **pptr;
  struct sockaddr_in dataaddr;
  char* path;
  char* instr;
  char* argument;
  
  //make an internet socket using TCP protocol.
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if(socketfd < 0){
    perror("ERROR: ");
    exit(1);
  }
  printf("created the socket\n");

  //set-up server address. 
  memset( &servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(MY_PORT_NUMBER);
  //get the numeric server address and store it in the address struct
  hostEntry = gethostbyname(argv[1]);
  if(hostEntry < 0 ){
    perror("Error: ");
    exit(1);
  }
  printf("have address\n");
  pptr = (struct in_addr **) hostEntry->h_addr_list;
  memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
  int cn = connect(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
  if(cn < 0){
    perror("Error: ");
    exit(1);
  }

  //give user command for input.
  //use the pipe connection to send the command.
  while(1){
    char buff[255];
    for(int i = 0; i < 255; i++){
      buff[i] = NULL;
    }
    printf("input command: ");
    fgets(buff, 255, stdin);
    instr = strtok(buff, " \n\t\v\f\r");
    
    /* EXIT COMMAND */ //(done)//
    if( strcmp(instr,"exit") == 0 ){
      write(socketfd, "Q\n", 2);
      char check[1];
      read(socketfd,check, 1);
      if(check[0] == "E"){
	printf("error exiting\n");
	exit(1);
      }
      printf("exiting\n");
      close(socketfd);
      exit(1);
    }
    
    /* CD COMMAND */ //(done)//
    else if( strcmp(instr,"cd") == 0){
      argument = strtok(NULL, "\n ");
      if(chdir(argument) < 0){
	perror("Error: ");
      }
    }
    
    /* RCD COMMAND */ //(done)//
    else if( strcmp(instr,"rcd") == 0){
      argument = strtok(NULL, "\n");
      char buff[100];
      strcpy(buff, "C");
      strcat(buff,argument);
      strcat(buff,"\n");
      write(socketfd, buff, strlen(buff) );
      char temp1[256];
      read(socketfd, temp1, 256);
      if(temp1[0] == 'E'){
	printf("error: %s\n", temp1+1);
	continue;
      }
      printf("directory changed\n");
    }
    
    /* LS COMMAND */ //(done)//
    else if( strcmp(instr,"ls") == 0){
      system("ls -l | more -20");
    }
    
    /* RLS COMMAND */ //(done)//
    else if( strcmp(instr,"rls") == 0){
      //make data connection.
      int datasockfd = setupDataConnection(socketfd, argv[1]);
      printf("made data connection\n");
      write(socketfd, "L\n", 2);
      //send the signal to do an ls remotely.
      char check[1];
      char N[1];
      read(socketfd, check, 1);
      read(socketfd, N, 1);
      printf("response: %s\n", check);
      if(check[0] == 'E'){
	char errbuff[512];
	read(socketfd, errbuff, 512);
	printf("error: %s\n", errbuff);
	continue;
      }
      if(check[0] == 'A'){
	if( fork() ){
	  wait(NULL);
	}else{
	  dup2(datasockfd, 0);
	  execlp("more","more","-20", (char *)NULL);
	}
      }
      printf("done printing\n");
      close(datasockfd);
    }
    /* GET COMMAND */
    else if( strcmp(instr,"get") == 0){
      argument = strtok(NULL, "\n \t\v\f\r");
      //set-up data connection.
      int datasockfd = setupDataConnection(socketfd, argv[1]);
      char buff[100];
      strcpy(buff, "G");
      strcat(buff, argument);
      strcat(buff, "\n");
      write(socketfd, buff, strlen(buff));
      
      char check[1];
      read(socketfd, check, 1);
      
      if(check[0] == 'E'){
	char errbuff[512];
	printf("error: %s\n", errbuff);
	continue;
      }
      char* next;
      char* prev;
      //get the file name.
      prev = strtok(argument, "/ \n\t\v\f\r");
      while( (next = strtok(NULL, "/ \n\t\v\f\r")) ){
	prev = next;
      }
      int file;
      if((file = open(prev, O_EXCL|O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR )) < 0){
	perror("Error: ");
	continue;
      }
      GP(datasockfd, file);
      printf("file transfered\n");
      close(datasockfd);
    }
    
    /* SHOW COMMAND */
    else if( strcmp(instr,"show") == 0){
      argument = strtok(NULL, "\n \t\v\f\r");
      //set-up data connection.
      int datasockfd = setupDataConnection(socketfd, argv[1]);
      char buff[100];
      strcpy(buff, "G");
      strcat(buff, argument);
      strcat(buff, "\n");
      write(socketfd, buff, strlen(buff));
      
      char check[1];
      read(socketfd, check, 1);
      if(check[0] == 'E'){
	char errbuff[512];
	printf("error: %s\n", errbuff);
	continue;
      }
      if( check[0] == 'A'){
	if( fork() ){
	  wait(NULL);
	}else{
	  dup2(datasockfd, 0);
	  execlp("more", "more", "-20", (char*)NULL);
	}
      }
      
      close(datasockfd);
    }
    
    /* PUT COMMAND */
    else if( strcmp(instr,"put") == 0){
      int file;
      //get the argument and open the file.
      argument = strtok(NULL, "\n \t\v\f\r");
      if((file = open(argument, O_RDONLY, S_IRUSR )) < 0){
	perror("Error: ");
	continue;
      }
      //set-up the data socket.
      int datasockfd = setupDataConnection(socketfd, argv[1]);
      char* next;
      char* prev;
      //get the file name to send.
      prev = strtok(argument, "/ \n\t\v\f\r");
      while( (next = strtok(NULL, "/ \n\t\v\f\r")) != NULL ){
	prev = next;
      }
      char buff[100];
      strcpy(buff, "P");
      strcat(buff, prev);
      strcat(buff, "\n");
      write(socketfd, buff, strlen(buff));
      char check[1];
      read(socketfd, check,1);
      if(check[0] == 'E'){
	perror("Error: ");
	continue;
      } 
      //transfer the file data.
      GP(file,datasockfd);
      printf("file transfered\n");
      close(datasockfd);
    }
    
    /* UNKNOWN COMMAND */
    else{
      fprintf(stderr, "unknown command\n");
    }
  }
}

  


/*
  GP (get/put) is a function that takes two file descriptors as arguments.
  It will read from the source descriptor and write the information into the 
  destination descriptor.
*/
void GP(int source, int dest){
  char buff[512];
  int bytes;
  while((bytes = read(source, &buff, 512)) > 0){
    write(dest, buff, bytes);
  } 
}


/*
setupDataConnection takes a file descriptor and the host name of the socket
connection you wish to set-up. and returns the socket filedescription to
be read from or written to.
 */
int setupDataConnection( int fd, char* hostname ){
  struct sockaddr_in dataaddr;
  struct hostent* hostEntry;
  struct in_addr **pptr;
  char hname[512];
  char D[3];
  strcpy(D, "D\n");
  write(fd, D, strlen(D));
  read(fd, hname, 512);
  printf("data connection: %s\n", hname);
  
  
  int datasockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset( &dataaddr, 0, sizeof(dataaddr));
  dataaddr.sin_family = AF_INET;
  dataaddr.sin_port = htons(atoi(hname+1));
  hostEntry = gethostbyname(hostname);
  pptr = (struct in_addr **) hostEntry->h_addr_list;
  memcpy(&dataaddr.sin_addr, *pptr, sizeof(struct in_addr));
  connect(datasockfd, (struct sockaddr *) &dataaddr, sizeof(dataaddr));
  printf("made data connection\n");
  
  return datasockfd;
}

