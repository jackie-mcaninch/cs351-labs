#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv) 
{
	//set arguments
	int operation = (int)atoi(argv[1]);
	int num_ops = (int)atoi(argv[2]);
	
	//initialize variables
	int serv_fd, new_socket;
	struct sockaddr_in client_addr;
	int opt = 1;
	int len = sizeof(client_addr);
	double op1, op2, result;
	
	//create socket
	serv_fd = socket(AF_INET, SOCK_STREAM , 0);
	
	//setsockopt
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(8000);
	
	//bind socket
	bind(serv_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	
	//listen for connection
	listen(serv_fd, 3);

	//run accept loop until connection occurs
	new_socket = accept(serv_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);
	
	//receive and send data
	for (int i=0; i<num_ops; i++) {
			
		//receive operands from socket
		recv(new_socket, &op1, sizeof(double), 0);
		recv(new_socket, &op2, sizeof(double), 0);
		
		//perform operation
		switch (operation) {
			case 0:
				result = op1 + op2;
				break;
			case 1:
				result = op1 - op2;
				break;
			case 2:
				result = op1 * op2;
				break;
			case 3:
				if(op2 == 0.0)
					result = 0;
				else
					result = op1 / op2;
				break;				
		}
	
		//send result back through the socket
		send(new_socket, &result, sizeof(double), 0);
	}
	
	close(serv_fd);
	close(new_socket);
	exit(0);
}
