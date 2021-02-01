#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <x86intrin.h>

#define USAGE "usage: ./syncbench <mode> <size> <threads> \n"

//GLOBAL VARIABLE DEFINITIONS
static int n = 0;
static sem_t *s_lock;
static pthread_mutex_t m_lock;
static pthread_spinlock_t sp_lock;


//FUNCTION DEFINITIONS
void *vanilla(void *arg) {
	int size = *((int *) arg);
	long int total_cycles = 0;
	double *avg_lat = (double *)malloc(sizeof(double));
	u_int64_t st, fin;
	for (int i=0; i<size; i++) {
		st = __rdtsc();
		n++;
		fin = __rdtsc();
		total_cycles += (long int)(fin-st);
	}
	*avg_lat = total_cycles/(double)size;
	return avg_lat;
}

void *mutex(void *arg) {
	int size = *((int *) arg);
	long int total_cycles = 0;
	double *avg_lat = (double *)malloc(sizeof(double));
	u_int64_t st, fin;
	for (int i=0; i<size; i++) {
		st = __rdtsc();
		pthread_mutex_lock(&m_lock);
		n++;
		pthread_mutex_unlock(&m_lock);
		fin = __rdtsc();
		total_cycles += (long int)(fin-st);
	}
	*avg_lat = total_cycles/(double)size;
	return avg_lat;
}

void *semaphore(void *arg) {
	int size = *((int *) arg);
	long int total_cycles = 0;
	double *avg_lat = (double *)malloc(sizeof(double));
	u_int64_t st, fin;
	for (int i=0; i<size; i++) {
		st = __rdtsc();
		sem_wait(s_lock);
		n++;
		sem_post(s_lock);
		fin = __rdtsc();
		total_cycles += (long int)(fin-st);
	}
	*avg_lat = total_cycles/(double)size;
	return avg_lat;
}

void *spinlock(void *arg) {
	int size = *((int *) arg);
	long int total_cycles = 0;
	double *avg_lat = (double *)malloc(sizeof(double));
	u_int64_t st, fin;
	for (int i=0; i<size; i++) {
		st = __rdtsc();
		pthread_spin_lock(&sp_lock);
		n++;
		pthread_spin_unlock(&sp_lock);
		fin = __rdtsc();
		total_cycles += (long int)(fin-st);
	}
	*avg_lat = total_cycles/(double)size;
	return avg_lat;
}

void *atomic(void *arg) {
	int size = *((int *) arg);
	long int total_cycles = 0;
	double *avg_lat = (double *)malloc(sizeof(double));
	u_int64_t st, fin;
	for (int i=0; i<size; i++) {
		st = __rdtsc();
		__atomic_fetch_add(&n, 1, __ATOMIC_SEQ_CST);
		fin = __rdtsc();
		total_cycles += (long int)(fin-st);
	}
	*avg_lat = total_cycles/(double)size;
	return avg_lat;
}


//MAIN FUNCTION
int main(int argc, char **argv)
{
	printf("Running %s for %s operations on %s threads...\n", argv[1], argv[2], argv[3]);
	printf("=============================================\n");
	
	//ASSESS ARGUMENTS
	
	if (argc != 4) 
	{
		printf(USAGE);
		exit(1);
	} 
	
	int mode = -1;
	int size = atoi(argv[2]);
	int threads = atoi(argv[3]);
	
	if(strcmp(argv[1],"vanilla") == 0)
		mode = 0;
	else if(strcmp(argv[1],"mutex") == 0)
		mode = 1;
	else if(strcmp(argv[1],"semaphore") == 0)
		mode = 2;
	else if(strcmp(argv[1],"spinlock") == 0)
		mode = 3;
	else if(strcmp(argv[1],"atomic") == 0)
		mode = 4;

	//PREPARE FOR MULTITHREADING
	pthread_t tid[threads];
	int *arg = malloc(sizeof(*arg));
	*arg = size/threads;
	
	//START TIMER
	struct timeval start, end;
	gettimeofday(&start, NULL);
	
	//SPLIT INTO THREADS
	for (int i=0; i<threads; i++) {
		switch (mode) {
		case 0: {								//VANILLA
			pthread_create(&tid[i], NULL, &vanilla, arg);
			break;
		}
		case 1: {								//MUTEX
			pthread_mutex_init(&m_lock, NULL);
			pthread_create(&tid[i], NULL, &mutex, arg);
			break;
		}
		case 2: {								//SEMAPHORE
			s_lock = sem_open("/sem", O_CREAT, 0600, 1);
			sem_init(s_lock, 1, 1);
			pthread_create(&tid[i], NULL, &semaphore, arg);
			break;
		}
		case 3: {								//SPINLOCK
			pthread_spin_init(&sp_lock, PTHREAD_PROCESS_SHARED);
			pthread_create(&tid[i], NULL, &spinlock, arg);
			break;
		}
		case 4: {								//ATOMIC
			pthread_create(&tid[i], NULL, &atomic, arg);
			break;
		}
		default: {
			printf(USAGE);
			exit(1);
		}		
		}
	}
	
	//REJOIN ALL THREADS AND CALCULATE LATENCY
	void *latency;
	double lat_total = 0;
	for (int i=0; i<threads; i++) {
		pthread_join(tid[i], &latency);
		lat_total += *(double *) latency;
	}
	double lat_avg = lat_total/threads;
	
	//FREE MEMORY
	sem_close(s_lock);
	sem_unlink("/sem");
	pthread_mutex_destroy(&m_lock);
	pthread_spin_destroy(&sp_lock);
	
	//END TIMER
	gettimeofday(&end, NULL);
	double time = (((end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec)))/1000000.0;
	printf("Final value of n: %d\nTime elapsed: %f seconds\n", n, time);
	
	//PRINT THROUGHPUT/LATENCY
	if (size == 1000000000) {
		double throughput = 1000000000/time;
		printf("Throughput: %f operations per second\n\n", throughput);
	}
	else {
		double latency = time*10; //divide by 1E8 operations, multiply by 1E9 nanosec per second
		printf("Latency (time): %f nanoseconds per operation\n", latency);
		printf("Latency (cycles): %f cycles per operation\n", lat_avg);
	}
	printf("\n\n");		
}
