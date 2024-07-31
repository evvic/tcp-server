#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
// #include "common.h"

/* Define the port which the client has to send data to */
#define SERVER_PORT 3000
#define CONNECTIONS_QUEUE 5

char DATA_BUFFER[1024];

void init_tcp_server()
{
    /* Initialize objects */
    int master_socket_fd = 0,   // master socket file descriptor
        sent_recv_bytes = 0,
        addr_len = 0,
        opt = 1;

    int comm_socket_fd = 0;     // client specific comm socket file descriptor

    fd_set readfds;             // set of FDs which select() polls

    struct sockaddr             // struct to store server and client info
        server_addr,            
        client_addr;

    /* TCP master socket creation */

    if ((master_socket_fd = socket(             // create a master socket for the server (get the FD)
            AF_INET,                            // specifies the address family -> IPv4 addressing
            SOCK_STREAM,                        // specify socket is for TCP connections
            IPPROTO_TCP)                        // specify TCP is the protocol to run in this socket
        ) == -1)
    {
        printf("Socket creation failed\n");
        exit(1);
    }

    /* Specify server information */
    server_addr.sin_family = AF_INET;           // this socket will only process IPv4 network packets
    server_addr.sin_port = SERVER_PORT;         // this server will process any data arriving on SERVER_PORT
    server_addr.sin_addr.s_addre = INADDR_ANY;  // set the server IP address to any/all interfaces on the server (INADDR_ANY)

    addr_len = sizeof(struct sockaddr);

    if (bind(                                   // get OS to send packets to this server program if they match server_addr info
            master_socket_fd,                   // provide socket FD
            (struct sockaddr *)&server_addr,    // provide server information (address & port)
            sizeof(struct sockaddr))
        == -1)
    {
        printf("Socket bind failed\n");
        return;
    }

    if (listen(                                 // tell the OS to maintain a queue of length CONNECTIONS_QUEUE
            master_socket_fd,                   // receiving conenctions on master socket
            CONNECTIONS_QUEUE)                  // the queue will hold no more than (5) CONNECTIONS_QUEUE
        < 0) 
    {
        printf("Listen failed\n");
        return;
    }

    while(1)
    {
        
    }

    close(master_socket_fd);
}

int main(int argc, char **argv)
{
    init_tcp_server();
    return 0;
}