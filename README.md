# tcp-server
TCP server and client with C

- Create a multiplexing TCP server
    - Where multiplexing means the ability to handle `n` number of clients at the same time

Client and server based model
- The server is any machine which receives the request, and optionally returns back the result
- Client is any machine which initiates the request

```

+-----+        ________
|  C  |----> ( internet )-----> 
+-----+       ```````````
```

## Socket Programming

- Linux OS provides2 system calls to enable us to write efficient network applications:
    1. `select()`
    2. `accept()`
- In C it is difficult to write a socket based application without the use of Select and Accept system calls

| Msg type | Client side API | Server side API |
| ------ | ---------| -------- |
| Connection initiation request messages | `connect()` | `accept()` |
| Service request messages | `sendmsg()`, `sendto()` | `recvmsg()`, `recvfrom()` |


### Server Designing
- Messages (or requests) sent by the client to the server can be categorized into 2 types:
    1. Connection inititation request messages
        - This message is used by the client to request the server to establish a dedicated connection
        - Only after the connection has been established, the client can send Service request messages to the server
    2. Service Request messages
        - Client can send these messages to server once the connection is fully established
        - Client requests server to provide a service

### Lifecycle of a Server
- When a server boots up, it creates a **master socket** using `socket()`
    - Master socket (M) has special privileges
    - M is the mother of all client handles
        - M gives birth to all Client handles
    - `accept()` is the system call used on the server side to create client handles
- A client handle is created by the server and master socket from a connection initiation request
- Once the client handles are created for each client, the Server carries out communication with the client using the client handle
    - The master socket is only used for new connection requests
    - M is not used for data exchange with already connected clients

### `accept` System Call
- The purpose of accept is to establish the connection between 2 machines (client and server)
- Carry out TCP 3-way handshake
- `accept` returns the handle to the connection with the client
    - The handle is just an integer value, representing a dedicated connection
- The handle is called a **communication file descriptor**
    - Because in Unix everything is a file
    - and the file descriptor is a non-negative integer to an open file or I/O resource
    - Treating the socket connection as an I/O stream allows aplications to use the same functions (`read`, `write`, `close`) for files and network sockets

```c
int comm_sock_fd = accept(master_sock_tcp_fd, (struct sockaddr*)&client_addr, &addr_len)
```
- `master_sock_tcp_fd`: the server creates this master socket on startup
    - This is the integer file descriptor for the master socket
- `client_addr`: pointer to the struct of type `sockaddr`
    - The struct is defined by standard C API
    - This is an empty structure that is passed by reference
    - It gets populated with the new client's identity (IP address and TCP port number)
- `addr_len`: size of predefined struct `sockaddr` (const)

### `select` System Call
- Implementing a server with multiplexing capabilities requires use of `select`
- Linux provides a built-in data structure to maintain the set of socket file descriptors
    - `fd_set` data structure is a set of file descriptors
- `select()` system call monitors all socket FDs present in `fd_set`
- it blocks code execution until either of the 2 options happens:
    1. New connection request from a client arrives
    2. Data request from existing connected client arrives


#### Example
- A client (C1) makes a new connection request to the server
- The connection request goes through the master socket
- The server then creates a **handle** for the new connection to C1
    - The master socket is used to create this handle from the connection request
    - A handle is an object created by the server that is used to carry out all communication between the client and server
    - The handle is just an object that represents a connection with a client and the server
- The created handle is then used to handle the service request messages sent from the client
    - The handle only processes the service request messages from the client it is conencted with
- The handle then (optionally) sends a service response back to the client

#### Linux Terminology
- handles = file descriptors
- client handles = communication file descriptors
- M = master socket file descriptor