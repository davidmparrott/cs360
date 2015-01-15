/*
David Parrott
11239947
CS 360 final
mftp client

mftp.c connects to a remote host specified in argv[1]
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

/*
port that daytime will listen for a connection on
*/
#define MY_PORT_NUMBER		49999
/*
takes a char* that is assumed to be the adderss of a remote
machine listening for connections and int that is assumed to
be the port that machine is listening on. Sets up the connection
and assigns it to socketfd which is returned for use by main
*/
int makeCon(char* arg, int port){
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
	servAddr.sin_port = htons(port);
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
/*
checks a file to verify it is a regular, non directory file
*/
int isDir(char* path){
	printf("%s\n",path);
  	DIR * dir;
	dir = opendir(path);

	if(dir == NULL){
    	//fprintf(stderr, "Failed to open directory '%s'\n", path);
    	return 1;
  	}else{return 0;}
}
/*
changes the client directory
*/
void cd(char* path){
	if(chdir(path) < 0){
		perror("cd error");
	}
	return;
}
/*
changes the server directory
*/
void rcd(char* path, int fd){
	char* commandBuff = (char*)calloc(256, sizeof(char));
	char reply[256];
	strcpy(commandBuff, "C");
	strcat(commandBuff, path);
	strcat(commandBuff, "\n");
	write(fd, commandBuff, strlen(commandBuff));
	read(fd, reply, 256);
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
	}
	free(commandBuff);
}
/*
performs ls -l in client directory
*/
void ls(){
	system("ls -l | more -20");
	return;
}
/*
gets the port for a data connection
*/
int newPort(int fd){
	int port;
	char* sendTheD = "D\n";
	char reply[256];
	write(fd, sendTheD, 2);
	read(fd, reply, 256);
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
		return(-1);
	}else{
		sscanf(reply, "A%d", &port);
	}
	return port;
}
/*
performs ls -l in server directory
*/
void rls(int datafd, int socketfd){
	char reply[256];
	int ecode;
	write(socketfd, "L\n", 2);
	read(socketfd, reply, 256);
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
	}else{
		char rlsBuff;
		int lineCount = 0;
		while(read(datafd, &rlsBuff, 1)){
			if(rlsBuff == '\n'){
				lineCount++;
				if(lineCount == 20){
					getchar();
					lineCount = 0;
				}
			}
			if(rlsBuff == EOF){
				break;
			}else{
				printf("%c", rlsBuff);
			}
		}
	}
}
/*
performs get
*/
void get(char* path, char* fileName, int datafd, int socketfd){
	char* fileBuff[512];
	char reply[256];	
	char* gpath = (char*)calloc(256, sizeof(char));	
	int bytes, filefd;
	strcpy(gpath, "G");
	strcat(gpath, path);
	strcat(gpath, "\n");
	write(socketfd, gpath, strlen(gpath));
	read(socketfd, reply, 256);
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
		return;
	}else if((filefd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 
									S_IRUSR | S_IWUSR )) < 0){
			
			perror("get file open error");
			return;
	}else{
		while((bytes = read(datafd, fileBuff, 512)) > 0){
			write(filefd, fileBuff, bytes);
		}
		close(filefd);
	}
}
/*
performs show 20 lines at a time
*/
void show(char* path, int socketfd, int datafd){
	char* commandBuff = (char*)calloc(256, sizeof(char));
	char reply[256];
	strcpy(commandBuff, "G");
	strcat(commandBuff, path);
	strcat(commandBuff, "\n");
	write(socketfd, commandBuff, strlen(commandBuff));
	read(socketfd, reply, 256);
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
	}else{
		char showBuff;
		int lineCount = 0;
		while(read(datafd, &showBuff, 1)){
			if(showBuff == '\n'){
				lineCount++;
				if(lineCount == 20){
					getchar();
					lineCount = 0;
				}
			}
			if(showBuff == EOF){
				break;
			}else{
				printf("%c", showBuff);
			}
		}
	}
	free(commandBuff);
}
/*
performs put if file exists and is regular
*/
void put(char* path, int filefd, int socketfd, int datafd){
	char* fileBuff[512];
	char reply[256];	
	char* putPath = (char*)calloc(256, sizeof(char));	
	int bytes;
	strcpy(putPath, "P");
	strcat(putPath, path);
	strcat(putPath, "\n");
	if(!isDir(path)){
		printf("file was not regular\n");
		return;
	}
	write(socketfd, putPath, strlen(putPath));
	read(socketfd, reply, 256);			
	if(reply[0] == 'E'){
		printf("error: %s", reply+1);
	}else{
		while((bytes = read(filefd, fileBuff, 512)) > 0){
			write(datafd, fileBuff, bytes);
		}
	}
}


int main(int argc, char* argv[]){
/*
variable declarations
socketfd will get an initialized connection from makeCon
daytime is a character buffer to store information from 
the remote machine
*/
	int socketfd, PID;
	char* inputBuffer = (char*)calloc(256, sizeof(char));
	char* instructionToken = (char*)calloc(256, sizeof(char));
	char* commandToken = (char*)calloc(256, sizeof(char));
	socketfd = makeCon(argv[1], MY_PORT_NUMBER);						//initialize socketfd

	while(1){
		printf("/:> ");
		fgets(inputBuffer, 256, stdin);
		instructionToken = strtok(inputBuffer, " \n\t\v\f\r");

		if(!strcmp(instructionToken, "exit")){
			write(socketfd, "Q\n", 2);
			printf("process terminated\n");
			close(socketfd);
			exit(0);
		}else if(!strcmp(instructionToken, "cd")){
			commandToken = strtok(NULL, "\n");
			cd(commandToken);
		}else if(!strcmp(instructionToken, "rcd")){
			commandToken = strtok(NULL, "\n");
			rcd(commandToken, socketfd);
		}else if(!strcmp(instructionToken, "ls")){
			commandToken = strtok(NULL, "\n");
			ls();
		}else if(!strcmp(instructionToken, "rls")){
			int port;
			int datafd;
			if((port = newPort(socketfd)) < 0){
				perror("rls data port error");
				continue;	
			}
			if((datafd = makeCon(argv[1], port)) < 0){
				perror("rls socket error");
				continue;
			}
			rls(datafd, socketfd);
			close(datafd);
		}else if(!strcmp(instructionToken, "get")){
			int port, datafd;
			char getBuff[256];

			commandToken = strtok(NULL, "\n\t\v\f\r");
			char* tempFileName = strtok(commandToken, "/");
			char* fileName = tempFileName;

			while((tempFileName = strtok(NULL, "/")) != NULL){
				fileName = tempFileName;
			}
			if((port = newPort(socketfd)) < 0){
				perror("get data port error");
				continue;	
			}
			if((datafd = makeCon(argv[1], port)) < 0){
				perror("get socket error");
				continue;
			}
			get(commandToken, fileName, datafd, socketfd);
			close(datafd);
		}else if(!strcmp(instructionToken, "show")){
			int datafd, port;
			commandToken = strtok(NULL, "\n\t\v\f\r");
			if((port = newPort(socketfd)) < 0){
				perror("show data port error");
				continue;	
			}
			if((datafd = makeCon(argv[1], port)) < 0){
				perror("show socket error");
				continue;
			}
			show(commandToken, socketfd, datafd);
			close(datafd);
		}else if(!strcmp(instructionToken, "put")){
			int port, datafd, filefd;
			char putBuff[256];
			commandToken = strtok(NULL, "\n\t\v\f\r");

			char* tempFileName = strtok(commandToken, "/");
			char* fileName = tempFileName;
			if((port = newPort(socketfd)) < 0){
				perror("get data port error");
				continue;	
			}
			if((datafd = makeCon(argv[1], port)) < 0){
				perror("get socket error");
				continue;
			}			
			while((tempFileName = strtok(NULL, "/")) != NULL){
				fileName = tempFileName;
			}
			if((filefd = open(commandToken, O_RDONLY)) < 0){
				perror("put file open error");
				continue;
			}
			put(fileName, filefd, socketfd, datafd);
			close(filefd);
			close(datafd);
		}

	}
	close(socketfd);									//close socketfd
	return 0;
}
