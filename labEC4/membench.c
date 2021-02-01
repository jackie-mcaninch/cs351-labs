#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <x86intrin.h>

#define USAGE "usage: ./membench <function> <size (in kB)>"

//DEFINE THREAD ARGS STRUCTURE
typedef struct thread_args {
	char *buf;
	char *str;
	char c;
	long int size;	
}targs;


//DEFINE FUNCTIONS
void *my_memset(void *arg) {
	targs *t = (targs *)arg;
	char *target = t->str;
	char c = t->c;
	int s = (t->size)/4;
	for (int i=0; i<s; i++) {
		target[i] = c;
	}
	return NULL;
}

void *my_memcpy(void *arg) {
	targs *t = (targs *)arg;
	char *to = t->buf;
	char *from = t->str;
	int s = (t->size)/4;
	for (int i=0; i<s; i++) {
		to[i] = from[i];
	}
	return NULL;
}

void my_memset_exec(void *ptr, int x, size_t n) {
	pthread_t tid[4];
	targs **t_args = (targs **)malloc(4*sizeof(targs *));
	t_args[0] = (targs *)malloc(4*sizeof(targs));
	for (int i=0; i<4; i++) {
		if (i>0)
			t_args[i] = t_args[i-1] + sizeof(targs);
		//create arguments for each thread
		t_args[i]->str = ptr + (i*(n/4));
		t_args[i]->c = x;
		t_args[i]->size = n;
		pthread_create(&tid[i], NULL, &my_memset, t_args[i]);
	}
	for (int i=0; i<4; i++) {
		pthread_join(tid[i], NULL);
	}
	free(t_args[0]);
	free(t_args);
}

void my_memcpy_exec(void *to, void *from, size_t n) {
	pthread_t tid[4];
	targs **t_args = (targs **)malloc(4*sizeof(targs *));
	t_args[0] = (targs *)malloc(4*sizeof(targs));
	for (int i=0; i<4; i++) {
		if (i>0)
			t_args[i] = t_args[i-1] + sizeof(targs);
		t_args[i]->buf = to +(i*(n/4));
		t_args[i]->str = from + (i*(n/4));
		t_args[i]->size = n;
		pthread_create(&tid[i], NULL, &my_memcpy, t_args[i]);
	}
	for (int i=0; i<4; i++) {
		pthread_join(tid[i], NULL);
	}
	free(t_args[0]);
	free(t_args);
}

//MAIN FUNCTION
int main(int argc, char **argv)
{
	printf("Running %s mode on %s bytes...\n", argv[1], argv[2]);
	printf("=============================================\n");
	
	//ASSESS ARGUMENTS
	if (argc != 3) 
	{
		printf(USAGE);
		exit(1);
	} 
	
	int mode = -1;
	long int size = strtol(argv[2], NULL, 0);
	
	if (strcmp(argv[1],"memset") == 0) {
		mode = 0;
	}
	else if(strcmp(argv[1],"memcpy") == 0) {
		mode = 1;
	}
	else if(strcmp(argv[1],"my_memset") == 0) {
		mode = 2;
	}
	else if(strcmp(argv[1],"my_memcpy") == 0) {
		mode = 3;
	}
	
	//CREATE BUFFERS
	char *buf = malloc(size);	//destination for memcpy
	char *str = malloc(size);	//string for memset
		
	//START TIMER
	struct timeval start, end;	//measures time
	gettimeofday(&start, NULL);
	
	u_int64_t st, fin;		//measures cycles
	st = __rdtsc();
	
	//CALL FUNCTION
	switch (mode) {
	case 0: {								//MEMSET
		memset(str, '.', size);
		break;
	}
	case 1: {								//MEMCPY
		memcpy(buf, str, size);
		break;
	}
	case 2: {								//MY_MEMSET
		my_memset_exec(str, '.', size);
		break;
	}
	case 3: {								//MY_MEMCPY
		my_memcpy_exec(buf, str, size);
		break;
	}
	default: {
		printf(USAGE);
		exit(1);
	}		
	}
	
	//END TIMER
	fin = __rdtsc();
	gettimeofday(&end, NULL);
	long int total_cycles = fin-st;
	double time = (((end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec)))/1000000.0;
	double throughput = size/time;
	printf("Time elapsed: %f seconds\n", time);
	printf("CPU ticks: %ld cycles\n", total_cycles);
	printf("Throughput: %f ops/sec\n\n", throughput);	
	
	//FREE MEMORY
	free(buf);
	free(str);
}
