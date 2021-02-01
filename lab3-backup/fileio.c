#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>

#define MSG "\n\n\n%s WORKLOAD WITH %sKB BLOCKS AND %s PROCESS(ES)\n\n"

#define USAGE "usage: ./fileio <workload> <block_size> <num_procs> [datasets...]\n" \
"     - workload: WS / WR / RS / RR \n" \
"     - block_size: 64KB / 1MB / 1GB \n" \
"     - num_procs: 1 / 2 / 4 / 8 \n" \
"     - WS = write-sequential \n" \
"     - WR = write-random \n" \
"     - RS = read-sequential \n" \
"     - RR = read-random \n"

int main(int argc, char **argv)
{

//handle faulty arguments...

    if (argc != atoi(argv[3])+4) {
	printf(USAGE);
        exit(1);
    } 
    else {
        printf(MSG, argv[1], argv[2], argv[3]);
    }
   
//handle arguments and store into variables
    
    //set record size
    int rec_size = atoi(argv[2])*1024;

    //set concurrency
    int num_procs = atoi(argv[3]);
    
    //set type of test
    int workload;
    if (strcmp(argv[1], "WS") == 0) {	
        workload = 0;
    }
    else if (strcmp(argv[1], "RS") == 0) {	
    	    workload = 1;
    }
    else if (strcmp(argv[1], "WR") == 0) {	
	workload = 2;
    }
    else if (strcmp(argv[1], "RR") == 0) {	
    	    workload = 3;
    }
    else {
	printf(USAGE);
	exit(1);
    }
    
    //set size of the dataset (1GB or 4MB)
    int dataset_size;
    if (rec_size == 4096) {
	dataset_size = 4194304;
    }
    else {
	dataset_size = 1073741824;
    }


//prepare for reading and writing
  
    //initialize and align buffer
    char *buf;
    posix_memalign((void*)&buf, 4096, rec_size);
    
    //initialize shared memory			 	     
    int *num_bytes = mmap(NULL, num_procs*sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    int *op_count = mmap(NULL, num_procs*sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    //initialize files and file stats
    int fd[num_procs];
    int offset; //for random read/write
    for (int i=0; i<num_procs; i++) {
	fd[i] = open(argv[i+4], O_DIRECT | O_RDWR);
	num_bytes[i] = 0;
	op_count[i] = 0;
    }
 

//identify workload and time the execution
    
    //start timer
    double total_time;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    double t = (double)((start.tv_sec*1000000)+start.tv_usec);

    //fork processes
    pid_t child;
    int stat = 0;
    for (int i=0; i<num_procs; i++) {
	if ((child = fork()) == 0) {	
	    switch(workload) {
	
	    //SEQUENTIAL WRITE TEST	    
		case 0:
		    while (num_bytes[i] < (dataset_size/num_procs)) {
			num_bytes[i] += write(fd[i], buf, rec_size);
		    }
		    break;

   	    //SEQUENTIAL READ TEST
   	        case 1:
		    while (num_bytes[i] < (dataset_size/num_procs)) {
			num_bytes[i] += read(fd[i], buf, rec_size);
		    }
		    break;

	    //RANDOM WRITE TEST
 	       case 2:
		    while (num_bytes[i] < (dataset_size/num_procs)) {
			offset = (rand() % (dataset_size/num_procs/rec_size))*rec_size;
			lseek(fd[i], offset, SEEK_SET);
			num_bytes[i] += write(fd[i], buf, rec_size);
		        op_count[i]++;
		    }
		    break;
	
	    //RANDOM READ TEST
       	       case 3:
		    while (num_bytes[i] < (dataset_size/num_procs)) {
			offset = (rand() % (dataset_size/num_procs/rec_size))*rec_size;
			lseek(fd[i], offset, SEEK_SET);
			num_bytes[i] += read(fd[i], buf, rec_size);
		        op_count[i]++;
		    }
		    break;
   	    }
        exit(0);
        }
    }
    while (wait(&stat) > 0);
    
    //stop timer and calculate elapsed time
    gettimeofday(&end, NULL);
    t = ((double)((end.tv_sec*1000000)+end.tv_usec)-t);
    total_time = t/1000000;
    printf("TIME ELAPSED: %f seconds\n", total_time);


//calculate throughput/latency

    //4MB DATASET: LATENCY
    if (rec_size == 4096) {
	int total_ops = 0;
	for (int i=0; i<num_procs; i++) {
	    total_ops += op_count[i];
	}
	printf("OPERATIONS PERFORMED: %d operations\n\n", total_ops);
	double latency = total_ops/total_time;
	printf("LATENCY: %f operations per second.\n", latency);
    }

    //1GB DATASET: THROUGHPUT
    else {
	int total_bytes = 0;
	for (int i=0; i<num_procs; i++) {
	    total_bytes += num_bytes[i];
	}
	printf("BYTES READ/WRITTEN: %d bytes\n\n", total_bytes);
	double throughput = total_bytes/total_time/1048576;
	printf("THROUGHPUT: %f megabytes per second.\n", throughput);
    } 
    printf("\n\n=================================================");    
    return 0;
}
