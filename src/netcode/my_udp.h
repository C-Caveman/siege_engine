// easily send and recv udp messages!
// more info at: https://docs.oracle.com/cd/E19620-01/805-4041/6j3r8iu2l/index.html

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MY_PORT 8080
#define MAX_BUF_SIZE 1024
#define MINI_BUF_SIZE 16
#define DEBUG 1
#define DMSG(...) if (DEBUG) { printf(__VA_ARGS__ ); }

extern struct sockaddr_in my_address;
extern struct sockaddr_in their_address;
extern int my_address_len;
extern int their_address_len;
//int socket_file_descriptor;
extern int my_ip;

// List available network interfaces:
void findAddresses();
// Select which valid network interface to use.
void selectLocalAddress();

// init vars used to send messages
// (use different port numbers if both parties are on the same machine)
void udp_init(int* socket, int my_port, int their_port, char* their_ip_address);

// give up the socket when done using udp
void udp_shut(int* socket);

// send a string of data to their_address
// (you can change who you're sending it to by changing their_address)
void udp_send(char* send, int* socket);

// send a specific number of bytes
void udp_send_n(char* send, int n, int* sock);

// recv a string of data
// (returns the len of the message)
int udp_recv(char* recv, int* socket);

// connect to a server on the LAN
void find_server(int* my_socket);

// listen for a find_server() request
void get_client(int* my_socket);
