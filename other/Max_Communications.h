#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9930


void diep(char *s)
{
	perror(s);
	exit(1);
}