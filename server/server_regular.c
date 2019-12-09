#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> // exit
#include <unistd.h>
#include <sys/errno.h>

const unsigned short int PORT = 1235;
const int QLEN = 10;

void sigchld_handler(int s)
{
    while (wait(NULL) > 0)
        ;
}

void sendMsg(char *message, int connectionfd)
{
    int messlen;
    messlen = strlen(message);
    if (sendall(connectionfd, message, &messlen) == -1)
    {
        fprintf(stderr, "Only %d bytes was transferred because of an error!\n", messlen);
    }
    else
    {
        puts("Transfer completed successfully!");
    }
}

int sendall(int fd, const char *data, int *len)
{
    int transfered = 0;
    // Bytes already sent
    int left = *len;
    // Bytes still to send
    int i;

    while (transfered < *len)
    {
        i = send(fd, data + transfered, left, 0);
        if (i == -1)
        {
            break;
        }; // Error!
        transfered += i;
        left -= i;
    };

    *len = transfered;
    return i == -1 ? -1 : 0;
}

int main(void)
{
    int fd, connectionfd;
    struct sockaddr_in ownAddress;
    struct sockaddr_in remAddress;
    int addrlen;
    char str[INET_ADDRSTRLEN];

    //create a socket:
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    //IP address and port number of the server:
    ownAddress.sin_family = AF_INET;         //AF_INET= IPv4
    ownAddress.sin_port = htons(PORT);       //htons=change Host byte order TO Network byte order, Short data
    ownAddress.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY=listen on each
    //available network interface;
    //can also listen on a specific address
    memset(&(ownAddress.sin_zero), '\0', 8);

    //bind to the address and port:
    if (bind(fd, (struct sockaddr *)&ownAddress, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(2);
    }

    //listen (wait) for incoming connections from clients:
    if (listen(fd, 5) == -1) //5=backlog (waiting queue length), usually between 5 and 10
    {
        perror("listen");
        exit(3);
    }
    puts("Waiting for incoming connections...");

    //accept the incoming connection, ie. establish communication with a client:
    addrlen = sizeof(struct sockaddr_in);
    if ((connectionfd = accept(fd, (struct sockaddr *)&remAddress, &addrlen)) == -1)
    {
        perror("accept");
        exit(4);
    }

    //now we have a working connection between this server and some client;
    //we can receive and send data

    //we can also check the address of the connected client:
    inet_ntop(AF_INET, &(remAddress.sin_addr), str, INET_ADDRSTRLEN);
    //(ntop=Network byte order TO (textual) Presentation);
    //a similar function, inet_ntoa(remAddress.sin_addr), is now deprecated
    printf("Connection from: %s\n", str);

    sendMsg("Hello! it's my welcome message \n Tell me something! \n", connectionfd);

    char buffer[255];
    while (recv(connectionfd, buffer, sizeof(buffer), 0) != -1)
    {
        size_t size = recv(connectionfd, buffer, sizeof(buffer), 0);
        puts(buffer);

        sendMsg(buffer, connectionfd);
        *buffer = NULL;
        close(connectionfd); //close connection when finished with this client
    }

    close(fd); //close server's socket when all server work is finished11

    return 0;
}