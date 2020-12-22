#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

int detached = 0;
int port = -1;

const int QLEN = 10;
pid_t id;
pid_t sid;

int fd, connectionfd;
struct sockaddr_in ownAddress;
struct sockaddr_in remAddress;
int addrlen;
char str[INET_ADDRSTRLEN];

void signal_handler(int s)
{
    return;
}

void sendMsg(char *message, int connectionfd)
{
    int messlen = strlen(message) - 1;
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

void detach_process(){
    id = fork();

    if (id < 0)
    {
        // Failed to fork
        perror(errno);
        exit(1);
    }
    else if (id > 0)
    {
        // This is parrent process
        exit(0);
    }

    umask(0);
    openlog("cn-server-deamon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "'cn-server-daemon' starting.\n");

    sid = setsid();
    if (sid < 0)
    {
        //Failure to obtain PID
        syslog(LOG_ERR, "Failed to create cn-server-daemon PID - setsid(): %d\n", sid);
        exit(1);
    }

    if ((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Failed to change directory to '/'\n");
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void echo(int connectionfd){
    char buffer[255];
    size_t size = recv(connectionfd, buffer, sizeof(buffer), 0);
    if (size != -1)
    {
        puts(buffer);

        sendMsg(buffer, connectionfd);
        *buffer = NULL;
    }
}

void handle_connection(int connectionfd){
    if (!fork()){
        echo(connectionfd);
    }
    close(connectionfd);
}

void listenAndServe(int fd) {
    //listen (wait) for incoming connections from clients:
    if (listen(fd, 5) == -1) //5=backlog (waiting queue length), usually between 5 and 10
    {
        perror("Failed listen()");
        exit(3);
    }
    puts("Waiting for incoming connections...");

    //accept the incoming connection, ie. establish communication with a client:
    addrlen = sizeof(struct sockaddr_in);
    if ((connectionfd = accept(fd, (struct sockaddr *)&remAddress, &addrlen)) == -1)
    {
        perror("Failed accept()");
        exit(4);
    }

    //now we have a working connection between this server and some client;
    //we can receive and send data

    //we can also check the address of the connected client:
    inet_ntop(AF_INET, &(remAddress.sin_addr), str, INET_ADDRSTRLEN);
    //(ntop=Network byte order TO (textual) Presentation);
    //a similar function, inet_ntoa(remAddress.sin_addr), is now deprecated
    printf("Handling connection from: %s\n", str);

    handle_connection(connectionfd);
}

void parse_arguments(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "-d")){
                detached = 1;
            }
            if (atoi(argv[i]))
                port = atoi(argv[i]);
        }
    }
    
    if (port == -1) {
        printf("Please provide a port number.\n");
        exit(1);
    }
    if (detached) {
        printf("Detaching server to run on port %d...\n", port);
        detach_process();
    }
}

int main(int argc, char* argv[])
{
    parse_arguments(argc, argv);

    //create a socket
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(2);
    }

    //IP address and port number of the server:
    ownAddress.sin_family = AF_INET;         //AF_INET= IPv4
    ownAddress.sin_port = htons(port);       //htons=change Host byte order TO Network byte order, Short data
    ownAddress.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY=listen on each
    //available network interface;
    //can also listen on a specific address
    memset(&(ownAddress.sin_zero), '\0', 8);

    //bind to the address and port:
    if (bind(fd, (struct sockaddr*)&ownAddress, sizeof(struct sockaddr)) == -1)
    {
        perror("Failed bind()");
        exit(3);
    }
    printf("Server started at localhost:%d\n", port);    
    while(1) {
        listenAndServe(fd);
    }
    close(fd); //close server's socket when all server work is finished

    return 0;
}