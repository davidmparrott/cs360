/*
David Parrott
11239947
CS 360 assignment 8
Day/Time server

dayserve.c listens for a connection and when one is received sends the
date and time information then closes.
*/
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*
port that dayserve will listen for a connection on
*/
#define MY_PORT_NUMBER 		49999

/*
opens a connection on port defined in MY_PORT_NUMBER then sends the
day and time over the connection. When a connection is detected a new
process forks off to send the date/time. Parent process waits for the
child to die then continutes in a while loop waiting for a new connection
*/
int main(){
/*
variable declarations
listenfd gets bound to the socket
connectfd gets bound to the incoming connection
length holds the size of the sockaddr_in struct
hostName gets the name of the machine connecting
pid gets the process id
*/
	int listenfd, connectfd, length, ecode;
	struct sockaddr_in servAddr;
	struct sockaddr_in clientAddr;
	struct hostent* hostEntry;
	char* hostName;
	pid_t pid;
/*
initialize structs
*/
	length = sizeof(struct sockaddr_in);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(MY_PORT_NUMBER);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

/*
opens socket
*/
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket error");
		exit(1);
	}
/*
binds socket
*/
	if ( bind( listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind error");
		exit(1);
	}
/*
begin listening on the socket
*/
	listen(listenfd, 1);
/*
continue to listen for new connections indefinitely
*/
	while(1){
/*
extract the first connection request on the queue of pending connections 
for the listening socket, listenfd, creates a new connected socket, and 
returns a new file descriptor referring to that socket bound to connectfd
*/
		if( (connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, (socklen_t *) &length) ) < 0) {
			perror("accept error");
			exit(1);
		}
/*
forks a child process
*/
		switch(pid = fork()){
			case -1:								//fork had an error 
				perror("fork error");
				exit(1);

			case 0:									//child process
				if( (hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL){
					herror("gethostbyaddr error");
					exit(1);
				}
				hostName = hostEntry->h_name;		//cache name of connection		
				time_t seconds;						//get and store time info
				char* date;
				seconds = time(NULL);
				date = ctime(&seconds);
				printf("%s connected on %s", hostName, date);	//log connection info to STDOUT
				write(connectfd, date, strlen(date));			//write date/time info
				close(connectfd);					//close connectfd
/*
ecode is set to 3 to avoid conflicts with any other exit codes
*/
				ecode = 3;							
				exit(ecode);

			default:								//parent process
				wait(&ecode);						//wait for child to die
		}
	}
	return 0;
}