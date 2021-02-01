#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>

#define MSG "* running netio with method %s operation %s for %s number of ops...\n"

#define USAGE "usage: ./netio <method> <operation> <num_ops> \n" \
"     - method: function / pipe / socket / rpc \n" \
"     - operation: add / subtract / multiply / divide \n" \
"     - num_calls: [int num_calls] \n"

double multiply(double a, double b)
{
	return a*b;
}

double divide(double a, double b)
{
	if(b == 0.0)
		return 0;
	else
		return a/b;
}

double add(double a, double b)
{
	return a+b;
}

double subtract(double a, double b)
{
	return a-b;
}


int main(int argc, char **argv)
{
	//initialize variables
	time_t t;
	srand((unsigned) time(&t));
	int method = -1; 		//used to store what method to test
	int operation = -1; 		//used to store what operation to test
	int tmp;			//used as a dummy variable to store to avoid warnings
	pid_t pid;
	double val1, val2;		//used to hold the random operands
	double ret_value = 0.0;	//used to hold the result of the operation
	double time_elapsed = 0;
    	

	//handle arguments
	if (argc != 4) {
        	printf(USAGE);
        	exit(1);
    	} 
    
	printf(MSG, argv[1], argv[2], argv[3]);
        
	if(strcmp(argv[1],"function") == 0)
        	method = 0;
        else if(strcmp(argv[1],"pipe") == 0)
        	method = 1;
        else if(strcmp(argv[1],"socket") == 0)
        	method = 2;
        else if(strcmp(argv[1],"rpc") == 0)
        	method = 3;
        else {
        	printf(USAGE);
        	exit(1);
	}
        if(strcmp(argv[2],"add") == 0)
        	operation = 0;
        else if(strcmp(argv[2],"subtract") == 0)
        	operation = 1;
        else if(strcmp(argv[2],"multiply") == 0)
        	operation = 2;
        else if(strcmp(argv[2],"divide") == 0)
        	operation = 3;
        else {
        	printf(USAGE);
        	exit(1);
	}
        int num_ops = atoi(argv[3]);


	//start timer
	struct timeval start, end;
  	gettimeofday(&start, NULL);
	switch (method) {
        
        
	//perform operations
        
	//FUNCTION
	case 0:;
   		switch (operation) {
        	case 0:
           		for (int i=0;i<num_ops;i++)
       			ret_value = add((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
        		break;
		case 1:
           		for (int i=0;i<num_ops;i++)
           			ret_value = subtract((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
			break;
        	case 2:
           		for (int i=0;i<num_ops;i++)
           			ret_value = multiply((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           		break;
        	case 3:
           		for (int i=0;i<num_ops;i++)
           			ret_value = divide((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           		break;
     		}
		break;
        
	//PIPE
	case 1:;		
		//create pipes
		int fd[4];
		tmp = pipe(fd); 	//pipe from parent to child
		tmp = pipe(fd+2); 	//pipe from child to parent
		
		//align memory
		tmp = posix_memalign((void*)&val1, 4096, sizeof(double));
		tmp = posix_memalign((void*)&val2, 4096, sizeof(double));
		tmp = posix_memalign((void*)&ret_value, 4096, sizeof(double));
		
		//create remote process
		if ((pid = fork()) == 0) {
			
			//child process: read operands and perform operations
			close(fd[1]);
			close(fd[2]);
			for (int i=0;i<num_ops;i++) {
				
				//read values piped from parent
				tmp = read(fd[0], &val1, sizeof(double));
				tmp = read(fd[0], &val2, sizeof(double));
				
				//perform designated operation
				switch (operation) {
				case 0:
					ret_value = add(val1, val2);
					break;
				case 1:
					ret_value = subtract(val1, val2);
					break;
				case 2:
					ret_value = multiply(val1, val2);
					break;
				case 3:
					ret_value = divide(val1, val2);
					break;
				}
				
				//pipe result back to parent
				tmp = write(fd[3], &ret_value, sizeof(double));
			}
			exit(0);
		}
		else {
			
			//parent process: generate random nums and pipe to child
			close(fd[0]);
			close(fd[3]);
			for (int i=0;i<num_ops;i++) {
				
				//generate randoms
				val1 = (double)rand()/RAND_MAX;
				val2 = (double)rand()/RAND_MAX;
				
				//pipe to child
				tmp = write(fd[1], &val1, sizeof(double));
				tmp = write(fd[1], &val2, sizeof(double));
				
				//receive result piped from child
				tmp = read(fd[2], &ret_value, sizeof(double));
			}
     		}
		break;
        
	//SOCKET	
	case 2:;
   		//call the server program
   		char cmd[30];
   		snprintf(cmd, 30, "./server %d %d", operation, num_ops);		
		if ((pid = fork()) == 0) {
			
			//child process: run remote program
			tmp = system(cmd);
			exit(0);
		}
		else {
			
			//parent process: send raw values and receive result
			
			//allow slight time for server to accept connection
			sleep(.001);
			waitpid(-1, NULL, WNOHANG);
			
			//create socket and establish connection
			int sock = 0;
			struct sockaddr_in serv_addr;
			sock = socket(AF_INET, SOCK_STREAM, 0);
			inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
		
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(8000);
			do {
				connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
			} while(errno == ECONNREFUSED);
			//exit(0);
			//send and receive values
			for (int i=0; i<num_ops; i++) {
				//generate two random doubles
				val1 = (double)rand()/RAND_MAX;
				val2 = (double)rand()/RAND_MAX;
				
				//send two doubles
				send(sock, &val1, sizeof(double), 0);
				send(sock, &val2, sizeof(double), 0);
			
		   			
		   		//receive result
				recv(sock, &ret_value, sizeof(double), 0);
			}
			close(sock);
		}
     		break;
     		
     	//ERROR	
        default:
        	printf("method not supported, exit...\n");
           	return -1;
     	}		

     	//stop timer and calculate time elapsed
	gettimeofday(&end, NULL);
	time_elapsed = ((end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec));
	printf("==> %f ops/sec\n",(num_ops/time_elapsed)*1000000);
	return 0;
}
