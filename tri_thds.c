#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t lock;
pthread_cond_t iteration_done;
int threads_created = 0, thread_counter = 0, num_thds = 3;

void *dispid(void *x) { //function for child threads to execute
	int i, num_iter=5;

	if(x != NULL) num_iter = (int *) x;

	//printf("Testing %d\n",pthread_self());
	//printf("\n%d threads created\n",++threads_created); //Shared resource

	//The assignment specified for each thread to display its ID *in-turn*
	//... but I do not track execution order within an iteration
	for(i=0;i<num_iter;i++) {
		printf("\nPID %d, Thread %d, Iteration no. %d, CPU ID %d\n",getpid(),pthread_self(),i+1,sched_getcpu());

		//pthread_mutex_lock(&lock); //No shared resources to worry about
		//... EXCEPT for the global variables modified here
		//... But uncommenting the lock & respective unlock below
		//... somehow causes unpredicatable behavior

		//Wait until the current iteration is done on all worker threads
		if(++thread_counter<num_thds) {
			pthread_cond_wait(&iteration_done,&lock);
			//Still in the body of the function
			//... is context switching a concern?
		}

		thread_counter = 0;
		//pthread_mutex_unlock(&lock);
		pthread_cond_signal(&iteration_done);
	}
}

int main(int argc, char* argv[]) {
	int i, max_thds = 100, *num_iter=NULL;

	if(argc > 1) num_thds = strtol(argv[1],NULL,10);
	if(argc > 2) num_iter = strtol(argv[2],NULL,10);
	if(num_thds>max_thds) num_thds = max_thds;

	pthread_t workers[num_thds], parent=pthread_self();
	pthread_attr_t tattr;

	pthread_mutex_init(&lock,NULL);
	pthread_attr_init(&tattr);
	pthread_attr_setschedpolicy(&tattr, SCHED_RR);

	printf("Program format is: tt.o [number of threads [number of iterations]]")
	printf("\nParent thread is %d\n",parent);

	for(i=0;i<num_thds;i++) {
		if(pthread_create(&workers[i],&tattr,(void*) dispid,num_iter)) {
			perror("[*] ERROR [*]: Failed to create thread.\n");
			exit(-1-i); //accounting for i==0, error code gives iteration number
		}
	}

	//Each child thread only exists within its respective dispid instance
	//... only the parent has scope in main, but others may run concurrently
	for(i=0;i<num_thds;i++) {
		pthread_join(workers[i],NULL); //parent waits for workers
	}

	//for(i=0;i<num_thds;i++) {
	//	printf("Thread id of worker#%d is %d\n",i+1,workers[i]);
	//}
	exit(0);
}
