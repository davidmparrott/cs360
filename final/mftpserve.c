/*
David Parrott
11239947
CS 360 Final
mftp server/client

mftpserve.c opens a socket and listens for a connection then
responds to commands from the client.
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
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

/*
port that dayserve will listen for a connection on
*/
#define MY_PORT_NUMBER 		49999
/*
create and bind a new socket with the specified port
*/
int makeCon(int port){
	int socketfd, length;
	length = sizeof(struct sockaddr_in);
	struct sockaddr_in servAddr;

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if( (socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}
	if ( bind( socketfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind error");
		exit(1);
	}
	return socketfd;	
}

int main(){
	int ctrl_listenfd, ctrl_connectfd, data_listenfd, data_connectfd, pid, length, ecode;
	char commandBuff[1];
	char* readBuff[256];
	char* replyBuff[256];
	char* err;
	char* hostName;
	struct hostent *hostEntry;
	struct sockaddr_in clientAddr;

	ctrl_listenfd = ctrl_connectfd = data_listenfd = data_connectfd = length = ecode = 0;
	length = sizeof(struct sockaddr_in);
	ctrl_listenfd = makeCon(MY_PORT_NUMBER);
	listen(ctrl_listenfd, 4);
	printf("listening for connection of port 49999\n");

	while(1){
		if( (ctrl_connectfd = accept(ctrl_listenfd, (struct sockaddr *) &clientAddr, (socklen_t *) &length) ) < 0) {
			perror("accept error");
			ecode = -1;
			exit(ecode);
		}
		if( (hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL){
			herror("gethostbyaddr error");
			ecode = -1;
			exit(ecode);
		}else{
			hostName = hostEntry->h_name;						//cache name of connection
			time_t seconds;										//get and store time info
			char* date;
			seconds = time(NULL);
			date = ctime(&seconds);
			printf("%s connected on %s", hostName, date);		//log connection info to STDOUT
		}		
		if(pid = fork()){
			wait(NULL);
		}else{									//child
			while(1){													//child process
				read(ctrl_connectfd, commandBuff, 1);
				switch(commandBuff[0]){
					case 'D':
					{	
						read(ctrl_connectfd, commandBuff, 1);
						data_listenfd = makeCon(0);
						char data_comBuff[16];
						int port, data_pid;

						struct sockaddr_in data_servAddr;
						struct sockaddr_in data_clientAddr;
						memset(&data_servAddr, 0, sizeof(data_servAddr));

						listen(data_listenfd, 1);
						getsockname(data_listenfd, (struct sockaddr*) &data_servAddr, &length);

						port = ntohs(data_servAddr.sin_port);
						printf("%s opened data port: %d\n",hostName, port);
						sprintf(&data_comBuff[0], "A%d\n\0", port);
						write(ctrl_connectfd, data_comBuff, strlen(&data_comBuff[0]));
						if( (data_connectfd = accept(data_listenfd, (struct sockaddr *) &data_clientAddr, (socklen_t *) &length) ) < 0) {
							perror("accept error");
							ecode = -1;
							exit(ecode);
						}
						continue;												
					}

					case 'C':
					{
						char reply[256];
						char path[1024];
						int i, bytes;
						i = 0;
						while((bytes = read(ctrl_connectfd, commandBuff, 1)) > 0){
							if(commandBuff[0] == '\n'){
								path[i] = '\0';
								break;
							}else{
								path[i] = commandBuff[0];
								i++;
							}
						}
						if(chdir(path) < 0){
							err = strerror(errno);
							strcpy(reply, "Edirectory change unsucessful\n");
							write(ctrl_connectfd, reply, strlen(reply));
						}else{
							write(ctrl_connectfd, "A\n", 2);
						}
						printf("%s changed directory to: %s\n", hostName, path);
						continue;
					}

					case 'L':
					{
						if(!data_connectfd){
							write(ctrl_connectfd, "Edata connection no established\n", 32);							
						}
						int ls_pid;
						write(ctrl_connectfd, "A\n", 2);
						printf("%s requesting ls -l\n", hostName);
						if(ls_pid = fork()){
							wait(NULL);
						}else{
							close(1);
							dup2(data_connectfd, 1);
							execlp("ls", "ls", "-l",  NULL);
							ecode = 3;
							exit(ecode);
						}						
						close(data_connectfd);
						continue;
					}

					case 'G':
					{
						if(!data_connectfd){
							write(ctrl_connectfd, "Edata connection no established\n", 32);							
						}
						char reply[256];
						char path[1024];
						char get_buff[512];
						char* err;
						int i, bytes, filefd;
						i = 0;
						while((bytes = read(ctrl_connectfd, commandBuff, 1)) > 0){
							if(commandBuff[0] == '\n'){
								path[i] = '\0';
								break;
							}else{
								path[i] = commandBuff[0];
								i++;
							}
						}
						if((filefd = open(path, O_RDONLY, S_IRUSR)) < 0){
							perror("error ");
							strcpy(reply, "Efile does not exist\n");
							write(ctrl_connectfd, reply, strlen(reply));
							ecode = -1;
							exit(ecode);
						}else{
							write(ctrl_connectfd, "A\n", 2);
							printf("%s getting file: %s\n", hostName, path);
							while((bytes = read(filefd, get_buff, 512)) > 0){
								write(data_connectfd, get_buff, bytes);
							}
							close(filefd);
							close(data_connectfd);
						}
						continue; 
					}

					case 'P':
					{
						if(!data_connectfd){
							write(ctrl_connectfd, "Edata connection no established\n", 32);							
						}
						char reply[256];
						char path[1024];
						char put_buff[512];
						int i, bytes, filefd;
						i = 0;
						while((bytes = read(ctrl_connectfd, commandBuff, 1)) > 0){
							if(commandBuff[0] == '\n'){
								path[i] = '\0';
								break;
							}else{
								path[i] = commandBuff[0];
								i++;
							}
						}
						if((filefd = open(path, O_CREAT | O_EXCL | O_WRONLY | O_APPEND, 
												  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0){
							perror("put error");
							strcpy(reply, "Efile exists\n");
							write(ctrl_connectfd, reply, strlen(reply));
							close(data_connectfd);
							ecode = -1;
							exit(ecode);
						}
						write(ctrl_connectfd, "A\n", 2);
						printf("%s putting file: %s\n", hostName, path);
						while((bytes = read(data_connectfd, put_buff, 512)) > 0){
							write(filefd, put_buff, bytes);
						}						
						close(filefd);
						close(data_connectfd);
						continue;																		
					}

					case 'Q':
						write(ctrl_connectfd, "A\n", 2);
						close(ctrl_connectfd);
						printf("%s exiting\n", hostName);
						ecode = 0;
						exit(ecode);
						break;
				}
			}
		}
	}
}