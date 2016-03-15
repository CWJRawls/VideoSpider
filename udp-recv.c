/*

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "osc-parse.h"
#include "tcp_client.h"

#define BUFSIZE 2048
#define SERVICE_PORT 21234 /* hard-coded port number */
#define SERVER_PORT 21235 /* hard-coded port for central server */


int main(int argc, char **argv)
{
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int fd;				/* our socket */
	int msgcnt = 0;			/* count # of messages we received */
	unsigned char buf[BUFSIZE];	/* receive buffer */


	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */
	int endwhile = 1;
	
	while(endwhile != 0) {
		printf("waiting on port %d\n", SERVICE_PORT);
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);
			int i;
			for(i = 0; i < recvlen; i++)
			{
				printf("%2x %c\t", buf[i],buf[i]);
				if(i % 4 == 3)
					printf("\n");
			}
			meth** msg = unpackMethods(buf, recvlen);
			
			i = 0;
			printf("msg.type = %d\n", msg[i]->data[0].type);
			
			
			while(msg[i]->data[0].type != no_type)
			{
				//do stuff
				int cmd = getCommand(msg[i]->data[0]);
				printf("command = %d\n", cmd);
				
				switch (cmd) 
				{
					case refresh:
						printf("freshen it up\n");
						// TODO: ask server for new ip address
					break;
					case stop:
						// TODO: check disconnect from server.
						endwhile = 0;
					break;
					case disconnect:
						// TODO: implement a disconnect from the server
					break;
					case give_ip:
						// TODO: give the ip address to the server
					break;
					default:
						printf("unknown command %d received. Doing nothing.\n", cmd);
					break;
				}
				
				i++;
			}
			
			// TODO: send the rest of the package without the commands to the server
			int k = recvlen/ 4;
			while(msg[k] == NULL)
			{
				k--;
			}
			
			char* mod_buf;
			int mod_size = recvlen;
			
			for(; k > -1; k--) //remove commands in reverse order to avoid upsetting indeces
			{
				mod_buf = buf;
				remove_command(mod_buf, recvlen, msg[k]->path, msg[k]->jitt);
				mod_size = mod_size - (msg[k]->jitt.end - msg[k]->path.start);
			}
			//free the msg array to avoid mem leaks
			free_meth(msg, recvlen);
		}
		else{
			printf("uh oh - something went wrong!\n");
			break;
		}
		sprintf(buf, "ack %d", msgcnt++);
		printf("sending response \"%s\"\n", buf);
		if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
			perror("sendto");
	}
	/* never exits */
}
