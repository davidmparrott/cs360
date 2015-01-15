/*
David Parrott
11239947
CS 360 assignment 8
Day/Time server

daytime.c connects to a remote host specified in argv[1] then
saves whatever is sent in a character buffer that is sent 
to STDOUT
*/
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*
port that daytime will listen for a connection on
*/
#define MY_PORT_NUMBER		49999

/*
takes a char* that is assumed to be the adderss of a remote
machine listening for connections. Sets up the connection
and assigns it to socketfd which is returned for use by main
*/
int makeCon(char* arg){
/*
variable declarations
socketfd gets bound to the socket
*/
	int socketfd;
	struct sockaddr_in servAddr;
	struct hostent* hostEntry;
	struct in_addr **pptr;
/*
initialize structs
*/
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(MY_PORT_NUMBER);
/*
bind socketfd and get host name (with error checking)
*/
	if( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}
	if((hostEntry = gethostbyname(arg)) < 0){
		herror("host name");
		exit(1);
	}
	pptr = (struct in_addr **) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));
/*
open connection to remote machine
*/
	if((connect(socketfd, (struct sockaddr *) &servAddr, sizeof(servAddr))) < 0){
		perror("connect");
		exit(1);
	}
/*
return the now initialized socketfd
*/
	return(socketfd);
}

int main(int argc, char* argv[]){
/*
variable declarations
socketfd will get an initialized connection from makeCon
daytime is a character buffer to store information from 
the remote machine
*/
	int socketfd;
	char daytime[255];
	socketfd = makeCon(argv[1]);						//initialize socketfd
	read(socketfd, daytime, 255);						//read from remote machine
	printf("%s", daytime);								//print information from remote machine
	close(socketfd);									//close socketfd
	return 0;
}