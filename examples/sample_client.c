#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> // exit

// port number and IP address of the server we want to connect to:
const unsigned short int PORT = 1235;
const char *SERVER = "127.0.0.1"; //"192.168.1.1";

int main(void)
{
	int fd;
	struct sockaddr_in remoteAddress;

	//create a socket:
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	//SOCK_STREAM=TCP, 0=IP
	{
		perror("socket");
		exit(1);
	}

	//IP address and port number of the server we would like to connect to:
	remoteAddress.sin_family = AF_INET;				   //AF_INET=IPv4
	remoteAddress.sin_port = htons(PORT);			   //htons=change Host byte order TO Network byte order, Short data
	remoteAddress.sin_addr.s_addr = inet_addr(SERVER); //inet_addr changes string notation
													   //(such as 192.168.1.1) to a long number
	memset(&(remoteAddress.sin_zero), '\0', 8);

	//try to connect to the server:
	if (connect(fd, (struct sockaddr *)&remoteAddress, sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		exit(2);
	}
	printf("Success!\n");

	//now we have a working connection between this client and the server;
	//we can receive and send data

	// char buffer1[255] = "test";
	// send(fd, buffer1, sizeof(buffer1), 0);

	char buffer2[255];
    while (recv(fd, buffer2, sizeof(buffer2), 0) != -1)
    {
        size_t size = recv(fd, buffer2, sizeof(buffer2), 0);
		puts("hello");
		puts(buffer2);
        *buffer2 = NULL;
    }
	close(fd); //close the socket when finished
	return 0;
}
