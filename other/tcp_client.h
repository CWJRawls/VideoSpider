#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h> 

#define SERV_PORT 21235

typedef struct{
	char* in_data; //data to be sent to the server, from max
	char out_data[1024]; //data from the server for max
	char* s_addr; //address of the server as a string
	int* in_size; //size of the char array in_data refers to
	int out_size; //size of the array referred to by out_data
	int* cmd; //pointer to any commands from max. (Disconnect Mainly)
	int* s_addr_size;//size of the address string
	int* in_state; //whether or not there is data to be sent
	int* out_state; //whether or not there is data to be read
} tcp_sock_data;

//function to open and run a socket to the server over tcp
void run_socket(tcp_sock_data* ts_d);

