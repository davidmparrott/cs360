/*
David Parrott
11239947
CS 360 Final
mftp server/client

mftpserve.c opens a socket and listens for a connection then
responds to commands from the client.
*/
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


/*
port that dayserve will listen for a connection on
*/
#define MY_PORT_NUMBER 		49999

/*
function prototypes
*/
int makeCon();
int readbale(char* path);
int getCommand(char* string);
/*
opens a connection on port defined in MY_PORT_NUMBER then sends the
day and time over the connection. When a connection is detected a new
process forks off to send the date/time. Parent process waits for the
child to die then continutes in a while loop waiting for a new connection
*/
int main(){
	int ctrl_listenfd, ctrl_connectfd, data_listenfd, data_connectfd, pid, length, ecode;
	char comBuff[1];
	char readBuff[4096] = {0};
	char replyBuff[4096] = {0};
	
	char* hostName;
	struct hostent *hostEntry;
	struct sockaddr_in clientAddr;

	ctrl_listenfd = ctrl_connectfd = data_listenfd = data_connectfd = length = ecode = -1;
	length = sizeof(struct sockaddr_in);
	ctrl_listenfd = makeCon(MY_PORT_NUMBER);
	listen(ctrl_listenfd, 4);
/*
continue to listen for new connections indefinitely
*/
	while(1){
/*
extract the first connection request on the queue of pending connections 
for the listening socket, listenfd, creates a new connected socket, and 
returns a new file descriptor referring to that socket bound to connectfd
*/
		if( (ctrl_connectfd = accept(ctrl_listenfd, (struct sockaddr *) &clientAddr, (socklen_t *) &length) ) < 0) {
			perror("accept error");
			ecode = -1;
			exit(ecode);
		}
/*
forks a child process
*/
		switch(pid = fork()){
			case -1:												//fork had an error 
				perror("fork error");
				ecode = -1;
				exit(ecode);

			case 1:
				printf("Parent\n");

			case 0:
			{
				printf("Child: %d\n", pid);
																	//child process
				if( (hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL){
					herror("gethostbyaddr error");
					ecode = -1;
					exit(ecode);
				}
				hostName = hostEntry->h_name;						//cache name of connection
				time_t seconds;										//get and store time info
				char* date;
				seconds = time(NULL);
				date = ctime(&seconds);
				printf("%s connected on %s", hostName, date);		//log connection info to STDOUT
				read(ctrlConnectfd, comBuff, 1);
				int action = getCommand(comBuff);
				switch(action){
					case 1:
					{											//D
						dataListenfd = makeCon();
						char dataBuff[7];
						int port;
						int lengthD; 
						int dataPID;
						lengthD = sizeof(struct sockaddr_in);

						struct sockaddr_in dataAddr;
						struct sockaddr_in clientAddr2;
						memset(&dataAddr, 0, sizeof(dataAddr));

						getsockname(dataListenfd, &dataAddr, &lengthD);
						port = ntohs(dataAddr.sin_port);
						printf("%d\n", port):
						sprintf(&dataBuff[0], "A %d\n\0", port);

						write(ctrlConnectfd, dataBuff, strlen(dataBuff));

						if( (dataConnectfd = accept(dataListenfd, (struct sockaddr *) &clientAddr2, (socklen_t *) &lengthD) ) < 0) {
							perror("accept error");
							ecode = -1;
							exit(ecode);
						}

						switch(dataPID = fork()){
							case -1:								//fork had an error 
								perror("fork error");
								ecode = -1;
								exit(ecode);

							case 0:
								continue;

							default:
								wait(NULL);
								close(dataConnectfd);
						}
					}
					case 2:
					{											//C
						printf("GOT THE C!\n");
						char msg[512] = {0};
						int i; 
						int readLength;
						char* err;
						i = 0;
						while((readLength = read(ctrlConnectfd, comBuff, 1)) > 0){
							if(comBuff[0] == '\n'){
								readBuff[i] = '\0';
								continue;
							}else{
								readBuff[i] = comBuff[0];
								i++;
							}
						}
						if(chdir(readBuff) < 0){
							err = strerror(errno);
							msg[0] = 'E';
							msg[1] = '\0';
							strcat(&msg, err);
							strcat(&msg, "\n");
							write(ctrlConnectfd, msg, strlen(&msg[0]));
						}else{
							write(ctrlConnectfd, "A\n", 2);
						}
						printf("Client: %d \n", pid);
						printf("New dir: %s \n", readBuff);
					}

					case 3:
					{											//L
						if(dataConnectfd == -1)
							write(ctrlConnectfd, "E data connection no established\n", 33);
						int lsPID;
						write(ctrlConnectfd, "A\n", 2);
						switch(lsPID = fork()){
							case -1:
								perror("fork error");
								exit(1);

							case 0:
								close(1);
								dup2(dataConnectfd, 1);
								close(dataConnectfd);
								close(ctrlConnectfd);
								execlp("ls", "ls", "-l",  NULL);

							case 1:
								wait(NULL);
								close(dataConnectfd);
								ecode = 3;
								exit(ecode);
						}
					}

					case 4:
					{											//G
						if(dataConnectfd == -1)
							write(ctrlConnectfd, "E data connection no established\n", 33);
						int i, FILE, data;
						char transferBuff[4096] = {0};
						i = 0;
						while((data = read(ctrlConnectfd, comBuff, 1)) > 0){
							if(comBuff[0] == '\n'){
								readBuff[i] = '\0';
								continue;
							}else{
								readBuff[i] = comBuff[0];
								i++;
							}
						}
						if((FILE = open(readBuff, O_RDONLY)) < 0){
							sprintf(&replyBuff[0], "E %s\n\0", strerror(errno));
							write(ctrlConnectfd, replyBuff, strlen(replyBuff));
							ecode = -1;
							exit(ecode);
						}
						if(readable(readBuff)){
							write(ctrlConnectfd, "A\n", 2);
							printf("Client: %d \n", pid);
							printf("Getting file: %s \n", readBuff);
							while((data = read(FILE, transferBuff, 4096)) > 0){
								write(dataConnectfd, transferBuff, data);
							}
							close(FILE);
							close(dataConnectfd);
							ecode = 3;
							exit(ecode);
						}else{
							write(ctrlConnectfd, "E failed to open file\n", 22);
							close(FILE);
							close(dataConnectfd);
							ecode = -1;
							exit(ecode);
						}
					}

					case 5:
					{											//P
						if(dataConnectfd == -1)
							write(ctrlConnectfd, "E data connection no established\n", 33);
						int i, FILE, data;
						char transferBuff[4096] = {0};
						i = 0;
						while((data = read(ctrlConnectfd, comBuff, 1)) > 0){
							if(comBuff[0] == '\n'){
								readBuff[i] = '\0';
								continue;
							}else{
								readBuff[i] = comBuff[0];
								i++;
							}
						}
						printf("Client: %d \n", pid);
						printf("Putting file: %s \n", readBuff);
						if((FILE = open(readBuff, O_CREAT | O_EXCL | O_WRONLY | O_APPEND, 
												  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0){
							sprintf(&replyBuff[0], "E %s\n\0", strlen(errno));
							write(ctrlConnectfd, replyBuff, strlen(replyBuff));
							close(dataConnectfd);
							ecode = -1;
							exit(ecode);
						}				
						write(ctrlConnectfd, "A\n", 2);
						while((data = read(dataConnectfd, transferBuff, 4096)) > 0){
							write(FILE, transferBuff, data);
						}								
						close(FILE);
						close(dataConnectfd);
						ecode = 3;
						exit(ecode);
					}

					case 6:
					{											//Q
						write(ctrlConnectfd, "A\n", 2);
						close(ctrlConnectfd);
						printf("Client: %d \n", pid);
						printf("quit\n");
						ecode = 0;
						exit(ecode);	
					}

					default:										//unknown command
						printf("Unknown command '%s' from client %i\n", comBuff, pid);
						close(dataConnectfd);
						ecode = -1;
						exit(ecode);
				}

				pid = getpid();
				//close(ctrlListenfd);
			}

			default:
				printf("WTF eh?\n");
				close(ctrlConnectfd);
		}
	}


}

/*
creates a new socket to be used by a data connection
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

/*
checks to see if a file is readable
*/
int readable(char* path){
	DIR * dir;
  	dir = opendir(path);
  	struct dirent* entry;
  	char* name;
  	if(dir == NULL){
    	fprintf(stderr, "Failed to open directory '%s'\n", path);
    	return(-1);
  	}
    entry = readdir(dir);
    if(!entry){
    	return(-1);
    }
    if(entry->d_type == 8){
    	return(1);
    }
    return(0);
}

/*
gets the next command token
*/
int getCommand(char* string){
	if(string == 'D'){return 1;}
	else if(string == 'C'){return 2;}
	else if(string == 'L'){return 3;}
	else if(string == 'G'){return 4;}
	else if(string == 'P'){return 5;}
	else if(string == 'Q'){return 6;}
	return -1;
}