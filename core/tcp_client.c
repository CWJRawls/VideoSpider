#include "tcp_client.h"

struct tcp_sock_data* sock_d;

char buffer[1024];
int clientSocket;
struct sockaddr_in severAddr;

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
			
		}
	}
	else
	{
		printf("Socket Connection Failed");
	}
}
