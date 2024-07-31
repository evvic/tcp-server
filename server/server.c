#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
// #include "common.h"

/* Define the port which the client has to send data to */
#define SERVER_PORT 3000

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

    if ((master_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Socket creation failed\n");
        exit(1);
    }

    close(master_socket_fd);
}

int main(int argc, char **argv)
{
    init_tcp_server();
    return 0;
}