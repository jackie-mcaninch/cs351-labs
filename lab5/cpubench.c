#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define MSG "* running cpubench %s using %s with size %s and %s threads...\n"

#define USAGE "usage: ./cpubench <mode> <type> <size> <threads> \n" \
"     - mode: flops / matrix \n" \
"     - type: single / double \n" \
"     - size: 10 / 100 / 1000 / 1024 / 4096 / 16386 \n" \
"     - threads: 1 / 2 / 4 \n"

#define GIGAFLOPS 1000000000
#define GIGABYTES 1024*1024*1024

struct d_args {
	double *a;
	double *b;
	double *res;
	int size;
	int l_bound;
	int u_bound;
};


struct i_args {
	int *a;
	int *b;
	int *res;
	int size;
	int l_bound;
	int u_bound;
};


void print_matrix_int(int* mat, int size)
{
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			printf(" %d ", mat[(i*size)+j]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_matrix_double(double* mat, int size)
{
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			printf(" %lf ", mat[(i*size)+j]);
		}
		printf("\n");
	}
	printf("\n");
}

void fill_matrix_int(int* mat, int size)
{
	int offset;
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			offset = (i*size) + j;
			mat[offset] = rand();
		}
	}
}

void fill_matrix_double(double* mat, int size)
{
	int offset;
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			offset = (i*size) + j;
			mat[offset] = (double)rand()/RAND_MAX;
		}
	}
}

void transpose_matrix_int(int* mat, int size)
{
	int tmp;
	for (int i=0; i<size; i++) {
		for (int j=i; j<size; j++) {
			tmp = mat[(i*size)+j];
			mat[(i*size)+j] = mat[(j*size)+i];
			mat[(j*size)+i] = tmp;
		}
	}
}


void transpose_matrix_double(double* mat, int size)
{
	double tmp;
	for (int i=0; i<size; i++) {
		for (int j=i; j<size; j++) {
			tmp = mat[(i*size)+j];
			mat[(i*size)+j] = mat[(j*size)+i];
			mat[(j*size)+i] = tmp;
		}
	}
}


void *multiply_int(void *arg)
{
	pthread_detach(pthread_self());
	struct i_args *m = (struct i_args *)arg;
	int l = m->l_bound;
	int u = m->u_bound;
	int size = m->size;
	int sum, i, j, k;
	for (i=l; i<u; i++) {				//i is row counter for matrix A
		for (j=0; j<size; j++) {		//j is row counter for trans matrix B
			sum = 0;			//k is column counter for both
			for (k=0; k<size; k++) {
				sum += ((m->a)[(i*size)+k] * (m->b)[(j*size)+k]);
			}
			(m->res)[(i*size)+j] = sum;
		}		
	}
	return NULL;
}


void *multiply_double(void *arg)
{
	pthread_detach(pthread_self());
	struct d_args *m = (struct d_args *)arg;
	int l = m->l_bound;
	int u = m->u_bound;
	int size = m->size;
	int i, j, k;
	double sum;
	for (i=l; i<u; i++) {				//i is row counter for matrix A
		for (j=0; j<size; j++) {		//j is row counter for trans matrix B
			sum = 0;			//k is column counter for both
			for (k=0; k<size; k++) {
				sum += ((m->a)[(i*size)+k] * (m->b)[(j*size)+k]);
			}
			(m->res)[(i*size)+j] = sum;
		}		
	}
	return NULL;
}


void *compute_flops_int(void *arg)
{
	int result = 0;
	unsigned long long int *loops = (unsigned long long int *)arg;
	for (unsigned long long int i=0;i<*loops;i++)
	{
		result = i+1;
		asm("");
	}
	return NULL;
}

void *compute_flops_double(void *arg)
{
	double result = 0;
	unsigned long long int *loops = (unsigned long long int *)arg;
	for (unsigned long long int i=0;i<*loops;i++)
	{
		result = i+1.0;
		asm("");
	}
	return NULL;
}


int main(int argc, char **argv)
{
	//ASSESS ARGUMENTS
	time_t t;
	srand((unsigned) time(&t));
	if (argc != 5) 
	{
		printf(USAGE);
		exit(1);
	} 
	
	int mode = -1;
	if(strcmp(argv[1],"flops") == 0)
		mode = 0;
	else if(strcmp(argv[1],"matrix") == 0)
		mode = 1;
	else
		mode = -1;

	int type = -1;
	if(strcmp(argv[2],"single") == 0)
		type = 0;
	else if(strcmp(argv[2],"double") == 0)
		type = 1;
	else
		type = -1;

	int size = atoi(argv[3]);
	int num_threads = atoi(argv[4]);
	

	//PERFORM METHOD
	struct timeval start, end;
	
	//FLOPS SINGLE
	if (mode == 0 && type == 0)
	{
		//start timer
		gettimeofday(&start, NULL);
		
		//split into threads
		unsigned long long int *num_loops = malloc(sizeof(int *));
		*num_loops = (unsigned long long int)(size*((unsigned long long int)GIGAFLOPS/num_threads));
		pthread_t tid[num_threads];
		for (int i=0; i<num_threads; i++) {
			pthread_create(&tid[i], NULL, &compute_flops_int, num_loops);
		}
		
		//rejoin all threads
		for (int i=0; i<num_threads; i++) {
			pthread_join(tid[i], NULL);
		}
		
		//stop timer
		gettimeofday(&end, NULL);
	}
	
	
	//FLOPS DOUBLE
	else if (mode == 0 && type == 1)
	{
		//start timer
		gettimeofday(&start, NULL);
		
		//split into threads
		unsigned long long int *num_loops = malloc(sizeof(int *));
		*num_loops = (unsigned long long int)(size*((unsigned long long int)GIGAFLOPS/num_threads));
		pthread_t tid[num_threads];
		for (int i=0; i<num_threads; i++) {
			pthread_create(&tid[i], NULL, &compute_flops_double, num_loops);
		}
		
		//rejoin all threads
		for (int i=0; i<num_threads; i++) {
			pthread_join(tid[i], NULL);
		}
		
		//stop timer
		gettimeofday(&end, NULL);
	}		
	
	
	//MATRIX SINGLE
	else if (mode == 1 && type == 0)
	{		
		//allocate memory for rows and columns
		int *matA = (int *)malloc(size*size*sizeof(int));
		int *matB = (int *)malloc(size*size*sizeof(int));
		int *matRES = (int *)calloc(size*size,sizeof(int));
		
		//initialize matrices with random values
		fill_matrix_int(matA, size);
		fill_matrix_int(matB, size);
		
		
		//partition matrices for multithreading
		struct i_args **matrices = (struct i_args **)malloc(num_threads*sizeof(struct i_args *));
		matrices[0] = (struct i_args *)malloc(num_threads*sizeof(struct i_args));
		for (int i=0; i<num_threads; i++) {
			if (i>0)
				matrices[i] = matrices[i-1] + sizeof(struct i_args);
			matrices[i]->a = matA;
			matrices[i]->b = matB;
			matrices[i]->res = matRES;
			matrices[i]->size = size;
			matrices[i]->l_bound = i*(size/num_threads);
			matrices[i]->u_bound = (i+1)*(size/num_threads);
		}
		
		//start timer		
		gettimeofday(&start, NULL);		
		
		//transpose matrix B for row-major access
		transpose_matrix_int(matB, size);
		
		//split into threads
		pthread_t tid[num_threads];
		for (int i=0; i<num_threads; i++) {
			pthread_create(&tid[i], NULL, &multiply_int, (void *)matrices[i]);
		}
		
		//rejoin all threads
		for (int i=0; i<num_threads; i++) {
			pthread_join(tid[i], NULL);
		}
		
		//free memory
		free(matA);
		free(matB);
		free(matRES);
		
		//stop timer
		gettimeofday(&end, NULL);
	}
	
	
	//MATRIX DOUBLE
	else if (mode == 1 && type == 1)
	{
		//allocate memory for rows and columns
		double *matA = (double *)malloc(size*size*sizeof(double));
		double *matB = (double *)malloc(size*size*sizeof(double));
		double *matRES = (double *)calloc(size*size,sizeof(double));
		
		//initialize matrices with random values
		fill_matrix_double(matA, size);
		fill_matrix_double(matB, size);
		
		//partition matrices for multithreading
		struct d_args **matrices = (struct d_args **)malloc(num_threads*sizeof(struct d_args *));
		matrices[0] = (struct d_args *)malloc(num_threads*sizeof(struct d_args));
		for (int i=0; i<num_threads; i++) {
			if (i>0)
				matrices[i] = matrices[i-1] + sizeof(struct d_args);
			matrices[i]->a = matA;
			matrices[i]->b = matB;
			matrices[i]->res = matRES;
			matrices[i]->size = size;
			matrices[i]->l_bound = i*(size/num_threads);
			matrices[i]->u_bound = (i+1)*(size/num_threads);
		}
		
		//start timer		
		gettimeofday(&start, NULL);		
		
		//transpose matrix B for row-major access
		transpose_matrix_double(matB, size);
		
		//split into threads
		pthread_t tid[num_threads];
		for (int i=0; i<num_threads; i++) {
			pthread_create(&tid[i], NULL, &multiply_double, (void *)matrices[i]);
		}
		
		//rejoin all threads
		for (int i=0; i<num_threads; i++) {
			pthread_join(tid[i], NULL);
		}
		
		//free memory
		free(matA);
		free(matB);
		free(matRES);
		
		//stop timer
		gettimeofday(&end, NULL);
	}
	
	
	else
	{
		printf(USAGE);
		printf("unrecognized option, exiting...\n");
		exit(1);
	}


	//CALCULATE TIME
	double elapsed_time_sec = (((end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec)))/1000000.0;
	unsigned long long int num_giga_ops = 0;
	
	
	//ERROR HANDLING
	if (size*GIGAFLOPS < 0)
	{
		printf("error in size, check for overflow; exiting...\n");
		exit(1);
	}
	if (elapsed_time_sec < 0)
	{
		printf("error in elapsed time, check for proper timing; exiting...\n");
		exit(1);
	}
	if (elapsed_time_sec == 0)
	{
		printf("elapsed time is 0; exiting...\n");
		exit(1);
	}
		

	//FINAL STATS
	if (mode == 0)
		num_giga_ops = size;
	else if (mode == 1) {
		num_giga_ops = (size*size*size)/(GIGABYTES);
	}
	double throughput = num_giga_ops/elapsed_time_sec;
	printf("mode=%s type=%s size=%d threads=%d time=%lf throughput=%lf\n",argv[1],argv[2],size,num_threads,elapsed_time_sec,throughput);

    return 0;
}
