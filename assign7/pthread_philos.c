/*
David Parrott
11239947
CS 360 assignment 7
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "random_r.h"

#define N 		5		//number of philosophers
#define ESTDDEV 3 		//Standard Deviation for eating
#define TSTDDEV 7 		//Stand Deviation for thinking
#define EMEAN	9 		//Mean value for eating
#define TMEAN	11 		//mean value for thinking

/*
Definition of globals
chopsticks_mutex is semaphore to 'guard' forks
state is used by randomGaussian_r as a random seed
*/
pthread_mutex_t chopsticks_mutex;
unsigned int state = 0;

/*
eat() governs eating behavior of philosophers.
id corresponds to which philosopher is trying to eat.
Borrowed code segment makes some philosophers reach
'left' first and some reach 'right' first to avoid
deadlocks.

Philosopher attempts to pick up the forks left and right.
Timer is set to the return value from randomGaussian_r.
Status information is sent to STDOUT.
sleep(timer) causes the philosopher to wait, holding on
to the semaphore the entire time, signifying the time spent
eating.
Semaphore is released.
timer is returned to accumulate the time spent eating.
*/
int eat(int id){
/*
The following code segment was taken from Rosetta Code
http://rosettacode.org/wiki/Dining_philosophers#C
*/
	int f[2], ration, i; /* forks */
	f[0] = f[1] = id;
	/* make some (but not all) philosophers leftie.
	   could have been f[!id] = (id + 1) %N; for example */
	f[id & 1] = (id + 1) % N;

	for (i = 0; i < 2; i++) {
		pthread_mutex_lock(&chopsticks_mutex + f[i]);
	}
/*
End segment from Rosetta Code
*/

	int timer = abs(randomGaussian_r(EMEAN, ESTDDEV, &state));

	printf("Philosopher %i is eating   for %i units\n", id, timer);
	sleep(timer);
	for(i = 0; i < 2; i++){
		pthread_mutex_unlock(&chopsticks_mutex + f[i]);
	}
	return timer;
}

/*
think() sets timer to the return value from randomGaussian_R(),
displays status information and sleeps for the return value in timer.
*/
void think(int id){
	int timer = abs(randomGaussian_r(TMEAN, TSTDDEV, &state));
	printf("Philosopher %i is thinking for %i units \n", id, timer);
	sleep(timer);
}

/*
goPhilo() governs the behavior of the philosophers.
eaten is declared and accumulates return values from eat().
Each philosopher starts out thinking to decrease the 
chances of different philosophers trying to reach for the same
fork initially.
Once eaten >= 100 status information sent to STDOUT and the
call to pthread_exit(0) terminates the thread.
*/
void* goPhilo(int* id){
	int eaten = 0;
	think(*id);
	while(eaten < 100){
		eaten += eat(*id);
		think(*id);
	}
	printf("Philosopher %i leaving table having eaten %i units\n", *id, eaten);
	pthread_exit(0);
}

/*
Creates philos[] using number of philosophers speicifed in the header.
Initializes semaphore to guard chopsticks.
Creates N threads.
When each thread terminates, pthread_join cleans up after the party
and closes the restaurant.
*/
int main(){
	pthread_t philos[N];
	int* id;

	pthread_mutex_init(&chopsticks_mutex, NULL);

	for(int i = 0; i < N; i++){
		id = (int*)malloc(sizeof(int));
		*id = i;
		pthread_create(&philos[i], NULL, (void*) goPhilo, (void*) id);
	}

	for(int i = 0; i < N; i++){
		pthread_join(philos[i], NULL);
	}
	return 0;
}