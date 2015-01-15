#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "random.h"

#define N 5

int semID = -1;

union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
};

void stickController(int n, int action){
	struct sembuf signals[2];
	int left = n + 1;
	int right = n;
	if(left >= N){left = 0;}
	
	signals[0].sem_num = left;
	signals[0].sem_op = action;

	signals[1].sem_num = right;
	signals[1].sem_op = action;

	int temp = semop(semID, signals, 2);

	return;
}

void upSticks(int n){
	stickController(n, -1);
}
void dnSticks(int n){
	stickController(n, 1);
}

void startPhilo(int n){
	int eat = 0;
	int tnk = 0;
	int eaten = 0;

	while(eaten < 100){
		eat = randomGaussian(9, 3);
		tnk = randomGaussian(11, 7);

		printf("Philosopher %d thinking for %d seconds\n", n, tnk);
		sleep(tnk);
		upSticks(n);
		printf("Philosopher %d eating for %d seconds. Total eaten %d\n", n, eat, eaten);
		sleep(eat);
		dnSticks(n);
		eaten += eat;
	}
	printf("Philosopher %d leaves table\n", n);
}

int main(int argc, char* argv[]){
	union semun semInfo;
	unsigned short sticks[N];
	int philos[N];
	int pid, status;

	for(int i = 0; i < N; i++){
		sticks[i] = 1;
		philos[i] = 0;
	}

	semID = semget(IPC_PRIVATE, 5, S_IRUSR | S_IWUSR);
	printf("semID set\n");

	semInfo.array = sticks;

	semctl(semID, 0, SETALL, semInfo);
	printf("Stick semaphore set\n");

	for(int i = 0; i < N; i++){
		pid = fork();
		if(pid == 0){
			startPhilo(i);
			return 0;
		}
		philos[i] = pid;
		printf("created philosopher %i \n", i);
	}
	printf("bye bye\n");

}