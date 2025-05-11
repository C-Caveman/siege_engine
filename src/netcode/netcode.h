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
#define MAX_UDP_PAYLOAD 508
#define MINI_BUF_SIZE 16
#define LOG_NETCODE 0
#define logNetcode(...) if (LOG_NETCODE) { printf(__VA_ARGS__ ); }

struct inbox {
    int sock;
    struct sockaddr_in address;
    char* sendBuffer;
    char* recvBuffer;
};
struct outbox {
    // id is not set by netcode.c, use it for your own purposes:
    int id;
    struct sockaddr_in address;
};
void outboxCreate(struct outbox* ob, int port, char* addressString, int id);
void inboxCreate(struct inbox* myInbox, int myPort, char* myAddressString);
void inboxDestroy(struct inbox* myInbox);
void inboxSend(struct inbox* in, struct outbox* out, int messageLen);
void inboxSendAllEvents(struct inbox* in, struct outbox* out, int numEventsToSend);
int inboxRecv(struct inbox* in, int bufferSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// old, dead code below ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern struct sockaddr_in myAddress;
extern struct sockaddr_in theirAddress;
extern unsigned int myAddressLen;
extern unsigned int theirAddressLen;
//int socket_file_descriptor;
extern int myIp;

// List available network interfaces:
void findAddresses();
// Select which valid network interface to use.
void selectLocalAddress();
// Get an address of a specified type: 'e' for ethernet, 'l' for loopback, 'w' for wifi
void getDefaultAddress(char c, char* addressString, int stringLen);

// init vars used to send messages
// (use different port numbers if both parties are on the same machine)
void udpInit(int* socket, int my_port, int their_port, char* their_ip_address);

// give up the socket when done using udp
void udpShut(int* socket);

// send a string of data to theirAddress
// (you can change who you're sending it to by changing theirAddress)
void udpSend(char* send, int* socket);

// send a specific number of bytes
void udpSendN(char* send, int n, int* sock);

// recv a string of data
// (returns the len of the message)
int udpRecv(char* recv, int* socket);

// connect to a server on the LAN
void findServer(int* my_socket);

// listen for a findServer() request
void getClient(int* my_socket);
