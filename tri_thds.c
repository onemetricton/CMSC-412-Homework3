#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t lock;
pthread_cond_t iteration_done;
int threads_created = 0, thread_counter = 0, num_thds = 3;

void *dispid(void *x) { //function for child processes to execute
	int i, num_iter=5;
	//printf("Testing %d\n",pthread_self());
	pthread_t *older_sib = (pthread_t *) x;
	/*
	if(*older_sib!=0) { //if isn't oldest child thread
		printf("\n\nthread %d waiting on %d addr %x\n",pthread_self(),*older_sib,older_sib);
		//pthread_join(*older_sib,NULL); //thread i+1 waits for thread i
	} //*/
	printf("\n%d threads created\n",++threads_created);

	for(i=0;i<num_iter;i++) {
		//pthread_mutex_lock(&lock);
		//pthread_join(*older_sib,NULL); //thread i+1 waits for thread i
		//printf("\nPreceded by %d",*older_sib);

		printf("\nPID %d, Thread %d, Iteration no. %d, CPU ID %d\n",getpid(),pthread_self(),i+1,sched_getcpu());

		if(++thread_counter<num_thds) {
			pthread_cond_wait(&iteration_done,&lock);
		}

		thread_counter = 0;
		pthread_cond_signal(&iteration_done);
		//pthread_mutex_unlock(&lock);
		//sched_yield();
	}
}

//TODO get round robin working, pass the mutex in to dispid instead of older_sib
//No that didn't work either...

int main(int argc, char* argv[]) {
	int i, max_thds = 100;

	if(argc > 1) num_thds = strtol(argv[1],NULL,10);
	if(num_thds>max_thds) num_thds = max_thds;

	pthread_t workers[num_thds], thd_iter=0, older_sib=0, parent=pthread_self();
	pthread_attr_t tattr;
	pthread_mutex_init(&lock,NULL);
	pthread_attr_init(&tattr);
	pthread_attr_setschedpolicy(&tattr, SCHED_RR);

	printf("Parent thread is %d\n",parent);

	for(i=0;i<num_thds;i++) {
		//printf("Outer loop iteration %d, older sibling %d\n",i,*older_sib);
		if(pthread_create(&workers[i],&tattr,(void*) dispid,(void*) &older_sib)) {
			perror("[*] ERROR [*]: Failed to create/join thread.\n");
			exit(-1-i); //accounting for i==0, error code gives iteration number
		} else {
			if(i>0) older_sib = workers[i-1]; //doesn't work because is passed to pthread_create by reference, changes concurrently as dispid is executed ...
		}
		//AFTER pthread_create() IS CALLED, THAT THREAD IS DONE EXECUTING
		//workers[i] = thd_iter; //just pass in the workers to pthread_create ...
	}

	for(i=0;i<num_thds;i++) {
		pthread_join(workers[i],NULL); //parent waits for workers
	}

	//for(i=0;i<num_thds;i++) {
	//	printf("Thread id of worker#%d is %d\n",i+1,workers[i]);
	//}
	exit(0);
}
