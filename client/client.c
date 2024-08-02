#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>

#define DEST_PORT 3001
#define SERVER_IP "127.0.0.1"

typedef struct test_data        // struct to hold the 2 values sent by the client
{       
    unsigned int a;
    unsigned int b;
} test_data;


typedef struct result_data      // struct to store the result of adding a + b from test_data
{
    unsigned int c;             // c = a + b
} result_data;

void setup_tcp_connection()
{

    test_data client_data;
    result_data result;
    int sockfd = 0, sent_recv_bytes = 0;
    
    int addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest;                                            // to store socket addresses

    dest.sin_family = AF_INET;                                          // specify server is using IPv4 addressing

    dest.sin_port = DEST_PORT;                                          // specify port number of the server

    struct hostent *host = (
        struct hostent*)gethostbyname(
            SERVER_IP
    );   // convert IP address string to int

    dest.sin_addr = *(
        (struct in_addr*)host->h_addr_list[0]);           // specify (int) IP address of server


    sockfd = socket(        // create a communication socket to connect to server
        AF_INET,            // IPv4 address family
        SOCK_STREAM,        // creating a TCP socket
        IPPROTO_TCP         // transport layer protocol is TCP
    );                 

    /* Connect with the server */
    if (connect(                        // open a connection on socket FD to (sockaddr) dest server
        sockfd,                     // socket file descriptor
        (struct sockaddr*)&dest,    // server address and port
        sizeof(struct sockaddr)
    ) == -1)
    {
        printf("Failed to connect to server\n");
        return;
    }

    /* OS dynamically assigns a port number to the client when sending a connection request */
    do {
        printf("Enter a: ");
        scanf("%u", &client_data.a);
        printf("Enter b: ");
        scanf("%u", &client_data.b);

        sent_recv_bytes = sendto(       // send data to the server, returns number of bytes sent to the server
            sockfd,                     // comm FD socket
            &client_data,               // structure which carries the data to be sent (uint a and b)
            sizeof(test_data),          // size of the data being sent to the server
            0,                          //
            (struct sockaddr*)&dest,    // server identity
            sizeof(struct sockaddr)     // size of sockaddr
        );

        printf("%d bytes sent to the server.\n", sent_recv_bytes);

        sent_recv_bytes = recvfrom(     // a blocking system call untilreceived data on the specified socket FD
            sockfd,                     // comm FD socket
            (char *)&result,            // structure to store the received data
            sizeof(result_data),        // size of the data structure to hold data being received
            0,                          //
            (struct sockaddr*)&dest,    // server identity
            sizeof(struct sockaddr)     // size of sockaddr
        );

        printf("%d bytes received from the server.\n", sent_recv_bytes);

        printf("%u + %u = %u\n", client_data.a, client_data.b, result.c);

    } while(client_data.a != 0 && client_data.b != 0);
}


int main(int argc, char** argv)
{
    printf("TCP Addition!\n");
    printf("\t*Assign 'a' and 'b' both to 0 to end the session\n");
    
    setup_tcp_connection();

    printf("Ending application\n");
    return 0;
}
