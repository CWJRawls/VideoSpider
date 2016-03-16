#include "tcp_client.h"

struct tcp_sock_data* sock_d;

char buffer[1024];
int clientSocket;
struct sockaddr_in severAddr;


//meant to be called as a part of a new thread. Requires the passing of socket data struct from header file.
void run_socket(tcp_sock_data* ts_d)
{
	sock_d = ts_d; //copy over the pointer.
	sock_d->out_data = buffer; // set the pointer to the buffer
	
	clientSocket = socket(PF_INET, SOCK_STREAM, 0); //create the socket
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERV_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(sock_d->s_addr);
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
	
	//connect to server
	if(connect(clientSocket, serverAddr, sizeof(serverAddr)) > -1)
	{
		int stop = 0; //Used to stop the socket when necessary
		int fail = 0;
		
		while(stop == 0)
		{
			//communicate with server
			int n = write(clientSocket, (struct sockaddr *) serverAddr, sizeof(serverAddr));
			
			if(n < 0) //keep track of the number of consecutive failures
			{
				fail++;
			}
			else //reset on successful write
			{
				fail = 0;
			}
			
			if(fail >= 10) //if we have failed to write to the socket 10+ times, lets close the socket
			{
				stop = 1;
			}
		}
		
		close(client_socket); //close the socket after the loop is complete.
	}
	else
	{
		printf("Socket Connection Failed");
	}
}
