/*********************************************************************
** A multiplexing TCP server that can hold multiple connections.
** The server takes 2 (unsigned) integers and adds them together.
** The result is then retuned to the client.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>

/* Define the port which the client has to send data to */
#define SERVER_PORT 3001
#define CONNECTIONS_QUEUE 5
#define MAX_SUPPORTED_CLIENTS 32

typedef struct test_data        // struct to hold the 2 values sent by the client
{       
    unsigned int a;
    unsigned int b;
} test_data;


typedef struct result_data      // struct to store the result of adding a + b from test_data
{
    unsigned int c;             // c = a + b
} result_data;

char DATA_BUFFER[1024];         // data buffer for data from client
char ip_str[INET_ADDRSTRLEN];   // client IP address buffer (formatted string)

int monitored_fd_set[MAX_SUPPORTED_CLIENTS];

void init_monitor_fd_set()
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        monitored_fd_set[i] = -1;
    }
}

void add_to_monitored_fd_set(int fd)
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i] != -1)
        {
            continue;
        }
        monitored_fd_set[i] = fd;
        break;
    }
}

void remove_from_monitored_fd_set(int fd)
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i] != fd)
        {
            continue;
        }
        monitored_fd_set[i] = -1;
        break;
    }
}

void re_init_fds(fd_set *fdset)
{
    FD_ZERO(fdset);
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i] != -1)
        {
            FD_SET(monitored_fd_set[i], fdset);
        }
    }
}

int get_max_fd()
{
    int max = -1;
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i] != -1)
        {
            max = (monitored_fd_set[i] > max) ? monitored_fd_set[i] : max;
        }
    }
    printf("Max FD: %d\n", max);
    return max;
}

void init_tcp_server()
{
    /* Initialize objects */
    int master_socket_fd = 0,   // master socket file descriptor
        sent_recv_bytes = 0,
        addr_len = 0,
        opt = 1;

    // int comm_socket_fd = 0;     // client specific comm socket file descriptor

    fd_set readfds;             // set of FDs which select() polls

    struct sockaddr_in          // struct to store server and client info
        server_addr,            
        client_addr;

    init_monitor_fd_set();      //

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
    server_addr.sin_addr.s_addr = INADDR_ANY;  // set the server IP address to any/all interfaces on the server (INADDR_ANY)

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

    add_to_monitored_fd_set(master_socket_fd);  // Add master socket DF to set being monitored

    /* Server loop for servicing clients */

    while(1)
    {

        re_init_fds(&readfds);                     // does the same thing as the 2 below lines but for all FDs
        // FD_ZERO(&readfds);                      // initialize the FD set to empty
        // FD_SET(master_socket_fd, &readfds);     // adding only the master FD to the set

        /* Wait for client connection */

        select(                                 // process waits for any request from a FD in the readfds set
            get_max_fd() + 1,                   // provide the number for creating the next FD
            &readfds,                           // provide the set of FDs
            NULL, NULL, NULL
        );

        if (FD_ISSET(                           // check which FD in readfds set is activated
                master_socket_fd,               // provide the master FD to check if that one is active
                &readfds))                      // within the FD set
        {                                       // master socket FD being active means a new connection request
            printf("New connection received! Accepting the connection..\n");

            /* Create a temp file descriptor for the rest of the connections life */

            int comm_socket_fd = accept(        // accept the connection and return the FD
                master_socket_fd,               // master FD only used for accepting the new clients connection
                (struct sockaddr*)&client_addr, // pass empty client_addr to be populated with IP address & port
                &addr_len                       // const size of sockaddr
            );

            if (comm_socket_fd < 0)             // check accept didn't fail creating a FD
            {
                printf("accept error: errno=%d\n", errno);
                exit(0);
            }

            if (inet_ntop(                      // Convert uint32_t IP address into readable string, return NULL on error
                    AF_INET,                    // specify IPv4 address
                    &(client_addr.sin_addr),    // provide the clients uint32_t IP address
                    ip_str,                     // the ip buffer to be assigned the IP string
                    sizeof(ip_str)              // size of the buffer
                ) == NULL)
            {                        // NULL means it failed to convert address to string
                perror("inet_ntop");
                printf("Error converting network address uint to a strin\n");
                strcpy(ip_str, "null");
            }

            add_to_monitored_fd_set(comm_socket_fd);
            printf("Connection accepted from client: %s:%u\n",
                inet_ntoa(client_addr.sin_addr),                         // print IP address with "x.x.x.x" format
                ntohs(client_addr.sin_port));   // convert port into readable integer
        }
        else                                    // data arrived on a client communication socket FD
        {
            int comm_socket_fd = -1;
            for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
            {
                if (FD_ISSET(                   // find which comm socket FD is active in monitored_fd_set
                    monitored_fd_set[i],        // use the array of FD integers to check which one
                    &readfds                    // is active in readfds
                ))
                {
                    comm_socket_fd = monitored_fd_set[i];
                    memset(                         // prepare memory space for server to store data
                        DATA_BUFFER,                // received from the client of size DATA_BUFFER
                        0,
                        sizeof(DATA_BUFFER));

                    /* Server receiving data from the client */

                    sent_recv_bytes = recvfrom(     // a blocking system call until data arrives at comm_socket_fd
                        comm_socket_fd,             // all communciation happens on the communication FD (not master FD)
                        (char*)DATA_BUFFER,         // the location which the data is going to be stored
                        sizeof(DATA_BUFFER),        // how many bytes long is this data buffer
                        0,
                        (struct sockaddr*)&client_addr,
                        sizeof(struct sockaddr)
                    );

                    printf("Server received %d bytes from client %s:%u\n",
                        sent_recv_bytes,
                        ip_str, // uint32_t or unsigned int
                        ntohs(client_addr.sin_port));

                    if (sent_recv_bytes == 0)       // if server received 0 bytes from client
                    {                               // server may close the connection and wait for
                        close(comm_socket_fd);      // a new conenction
                        remove_from_monitored_fd_set(comm_socket_fd);
                        break;
                    }

                    test_data *client_data = (test_data *)DATA_BUFFER;

                    if (client_data->a == 0 &&      // End connection with client
                        client_data->b == 0)        // if both a and b values are 0
                    {
                        close(comm_socket_fd);
                        remove_from_monitored_fd_set(comm_socket_fd);
                        printf("Server closed connection with client %s:%u\n",
                            ip_str,
                            ntohs(client_addr.sin_port));

                        break;
                    }

                    result_data result;

                    /* Server is calculating the sum of values a + b */
                    result.c = client_data->a + client_data->b;

                    /* Server replying back to client */

                    sent_recv_bytes = sendto(       // server is sending the result back to the client
                        comm_socket_fd,             // invoked on the comm_socket_fd to reply to cleint
                        (char*)&result,
                        sizeof(result_data),
                        0,
                        (struct sockaddr*)&client_addr,
                        sizeof(struct sockaddr)
                    );

                    printf("Server send %d bytes in reply to client\n",
                        sent_recv_bytes
                    );

                }
            }
        }
    }

    close(master_socket_fd);
}

int main(int argc, char **argv)
{
    init_tcp_server();
    return 0;
}
