#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t lock;
pthread_cond_t iteration_done;
int num_thds = 3;
volatile int thread_counter;

//https://docs.oracle.com/cd/E19253-01/816-5137/gfwek/index.html


void *dispid(void *x) { //function for worker threads to execute
	int i=0, num_iter=5;

	if(x != NULL) num_iter = (int *) x;

	//The assignment specified for each thread to display its ID *in-turn*
	//... but I do not track execution order within an iteration
	while(i<num_iter) {
		pthread_mutex_lock(&lock); //No shared resources to worry about
		//... EXCEPT for the global variables modified here
		//... But uncommenting the lock & respective unlock below
		//... somehow reveals unpredictable behavior

		//pthread_cond_init(&iteration_done, NULL);
		++thread_counter;
		pthread_mutex_unlock(&lock);

		printf("\nPID %d, Thread %d, Iteration No. %d, Counter %d, CPU ID %d\n",getpid(),pthread_self(), i+1, thread_counter, sched_getcpu());
		//counter stuck at 1 after the first iteration

		//Wait until the current iteration is done on all worker threads
		//RACE CONDITION!
		pthread_mutex_lock(&lock);
		while(thread_counter < num_thds) {
			pthread_cond_wait(&iteration_done, &lock);
			//Still in the body of the function
			//... is context switching a concern?
		}

		//pthread_cond_signal(&iteration_done);
		pthread_cond_broadcast(&iteration_done);
		thread_counter = 0;

		pthread_mutex_unlock(&lock);

		i++;
	}

	pthread_exit((void *)0);
}

int main(int argc, char* argv[]) {
	int i, max_thds = 100, *num_iter=NULL;

	if(argc > 1) num_thds = strtol(argv[1], NULL, 10);
	if(argc > 2) num_iter = strtol(argv[2], NULL, 10);
	if(num_thds>max_thds) num_thds = max_thds;
	thread_counter = 0;

	pthread_t workers[num_thds], main_thd=pthread_self();
	pthread_attr_t tattr;

	pthread_mutex_init(&lock, NULL);
	pthread_attr_init(&tattr);
	pthread_attr_setschedpolicy(&tattr, SCHED_RR);

	printf("Program format is: tt.o [number of threads [number of iterations]]\n");
	printf("\nMain thread is %d\n", main_thd);

	for(i=0;i<num_thds;i++) {
		if(pthread_create(&workers[i], &tattr, (void*) dispid, num_iter)) {
			perror("[*] ERROR [*]: Failed to create thread.\n");
			exit(-1-i);
		}
	}

	//Each child thread only exists within its respective dispid instance
	//... only the main_thd has scope in main, but others may run concurrently
	for(i=0; i<num_thds; i++) {
		pthread_join(workers[i], NULL); //main_thd waits for workers
	}

	//for(i=0;i<num_thds;i++) {
	//	printf("Thread id of worker#%d is %d\n",i+1,workers[i]);
	//}
	exit(0);
}
