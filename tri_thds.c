#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *dispid(void *x) { //function for child processes to execute
	int i;//, x = *(int *) num_iter;
	int num_iter=5;
	//printf("Testing %d\n",pthread_self());
	pthread_t *older_sib = (pthread_t *) x;
	if(*older_sib!=0) { //if isn't oldest child thread
		printf("\n\nthread %d waiting on %d addr %x\n",pthread_self(),*older_sib,older_sib);
		pthread_join(*older_sib,NULL); //thread i+1 waits for thread i
	}
	for(i=0;i<num_iter;i++) {
		printf("\nPID %d, Thread %d, Iteration no. %d\n",getpid(),pthread_self(),i+1);
	}
}

int main(int argc, char* argv[]) {
	int i, num_iter = 5, num_thds = 3, max_thds = 100;
	if(argc > 1) num_thds = strtol(argv[1],NULL,10);
	if(num_thds>max_thds) num_thds = max_thds;
	pthread_t workers[num_thds], thd_iter=0, older_sib=0, parent=pthread_self();
	printf("Parent thread is %d\n",pthread_self());
	for(i=0;i<num_thds;i++) {
		//printf("Outer loop iteration %d, older sibling %d\n",i,*older_sib);
		if(pthread_create(&thd_iter,NULL,(void*) dispid,(void*) &older_sib)) {
			perror("[*] ERROR [*]: Failed to create/join thread.\n");
			exit(-i+1); //accounting for i==0, error code gives iteration number
		} else {
			if(i>0) older_sib = workers[i-1]; //could probably be placed anywhere in the for loop
		}
		workers[i] = thd_iter;
	}
	for(i=0;i<num_thds;i++) {
		pthread_join(workers[i],NULL); //parent waits for workers
	}
	//for(i=0;i<num_thds;i++) {
	//	printf("Thread id of worker#%d is %d\n",i+1,workers[i]);
	//}
	exit(0);
}
