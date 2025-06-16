// make sending messages easier

#include "netcode.h"
#include "../defs.h"
#include <time.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdbool.h>
#ifndef fatal
    // Print an error message with file/lineNum, then kill the program.
    #define fatal(message) { fprintf(stderr, "*** %s:%d *** " message "\n", __FILE__, __LINE__); perror(""); exit(1); }
#endif

void makeAddress(struct sockaddr_in* a, int port, char* addressString) {
    memset(&a, 0, myAddressLen);
    memset(&a,  0, myAddressLen);
    a->sin_family = AF_INET; // IPv4
    a->sin_addr.s_addr = INADDR_ANY;
    a->sin_port = htons(port);
    int ipStringValid = inet_aton(addressString, &a->sin_addr);
    if (!ipStringValid)
        fprintf(stderr, "*** Warning! IP address string in makeAddress() was invalid!\n");
}
void outboxCreate(struct outbox* ob, int port, char* addressString, int id) {
    // Set the outbox's address:
    makeAddress(&ob->address, port, addressString);
    ob->id = id;
}
// Make an inbox to send/receive messages at myAddressString/myPort.
void inboxCreate(struct inbox* myInbox, int myPort, char* myAddressString) {
    // Set the inbox's address:
    makeAddress(&myInbox->address, myPort, myAddressString);
    // get a socket for my address
    if ( (myInbox->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 )
        fatal("inboxCreate() socket creation failed.");
    // set the socket options
    int enableBroadcast = 1;
    setsockopt(myInbox->sock, SOL_SOCKET, SO_BROADCAST,
            &enableBroadcast, sizeof(enableBroadcast));
    
    //bind(*sock,  (const struct sockaddr *)&myAddress, myAddressLen )
    
    if ( bind(myInbox->sock, (const struct sockaddr *)&myInbox->address, sizeof(myInbox->address)) < 0 ) {
        fatal("inboxCreate() socket binding failed.");
    }
}
void inboxDestroy(struct inbox* myInbox) {
    if (!myInbox || !myInbox->sock)
        return;
    close(myInbox->sock);
    memset(myInbox, 0, sizeof(struct inbox));
}
void inboxSend(struct inbox* in, struct outbox* out, int messageLen) {
    if (!in->sendBuffer)
        fatal("Inbox did not have a sendBuffer set!");
    dlog(NETCODE, "Sending message from (%s, %d)", inet_ntoa(in->address.sin_addr), ntohs(in->address.sin_port));
    dlog(NETCODE, " to (%s, %d) (id=%d)\n", inet_ntoa(out->address.sin_addr), ntohs(out->address.sin_port), out->id);
    if (messageLen > MAX_UDP_PAYLOAD)
        fatal("Tried to send more than MAX_UDP_PAYLOAD bytes!\n");
    sendto(in->sock, 
            (const char *)in->sendBuffer, 
            messageLen,
            MSG_CONFIRM, 
            (const struct sockaddr *)&out->address,
            sizeof(out->address));
}
void printEventPacket(char* packet, int numEvents) {
    if (numEvents < 0 || numEvents > MAX_PACKET_EVENTS)
        fatal("Can't print packet with invalid size.");
    printf("Packet with %d events:\n", numEvents);
    struct event* e = (struct event*)packet;
    for (int i=0; i<numEvents; i++) {
        printf("    '%s'\n", eventName(e->type));
        e++;
    }
}
void inboxSendAllEvents(struct inbox* in, struct outbox* out, int numEventsToSend) {
    if (!in->sendBuffer)
        fatal("Inbox did not have a sendBuffer set!");
    if (numEventsToSend*sizeof(serverEventBuffer.buffer[0]) > MAX_UDP_PAYLOAD)
        fatal("Tried to send more than MAX_UDP_PAYLOAD bytes!\n");
    dlog(NETCODE, "Sending message from (%s, %d)", inet_ntoa(in->address.sin_addr), ntohs(in->address.sin_port));
    dlog(NETCODE, " to (%s, %d) (id=%d)\n", inet_ntoa(out->address.sin_addr), ntohs(out->address.sin_port), out->id);
    dlog(EVENT_TRANSMISSION, "inboxSendAllEvents: sending ");
    for (int i=0; i<serverEventBuffer.count; i++) {
        dlog(EVENT_TRANSMISSION, "%d:%s, ", serverEventBuffer.buffer[i].type, eventName(serverEventBuffer.buffer[i].type));
    }
    dlog(EVENT_TRANSMISSION, "\n");
    int numEventsSent = 0;
    int numPacketEvents = 0;
    while (numEventsSent < numEventsToSend) {
        numPacketEvents = numEventsToSend - numEventsSent;
        if (numPacketEvents > MAX_PACKET_EVENTS)
            numPacketEvents = MAX_PACKET_EVENTS;
        int messageLen = sizeof(serverEventBuffer.buffer[0]) * numPacketEvents;
        if (LOG_PACKETS) {
            printf("Sending -> ");
            printEventPacket(in->sendBuffer + numEventsSent * sizeof(struct event), numPacketEvents);
        }
        sendto(in->sock, 
            (const char *) ( in->sendBuffer + numEventsSent * sizeof(serverEventBuffer.buffer[0]) ), 
            messageLen,
            MSG_CONFIRM, 
            (const struct sockaddr *)&out->address,
            sizeof(out->address));
        numEventsSent += numPacketEvents;
    }
}
int inboxRecv(struct inbox* in, int bufferSize) {
    if (!in->recvBuffer)
        fatal("Inbox did not have a recvBuffer set!");
    int messageLen = recvfrom(in->sock, 
                                (char *)in->recvBuffer, 
                                bufferSize,
                                MSG_WAITALL, 
                                0,/*(struct sockaddr *)&senderAddress,*/
                                0/*&senderAddressLen*/
                               );
    if (LOG_PACKETS) {
        printf("Received -> ");
        printEventPacket(in->recvBuffer, messageLen / sizeof(struct event));
    }
    return messageLen;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// old, dead code below ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct sockaddr_in myAddress;
struct sockaddr_in theirAddress;
unsigned int myAddressLen;
unsigned int theirAddressLen;
struct in_addr myTempAddress;
int myIp;

void findAddresses() {
   struct ifaddrs *addresses;
    if (getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        exit(-1);
    }

    struct ifaddrs *address = addresses;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            printf("%s\t", address->ifa_name);
            printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");
            
            char ipNumberString[100];
            const int family_size = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            getnameinfo(address->ifa_addr,family_size, ipNumberString, sizeof(ipNumberString), 0, 0, NI_NUMERICHOST);
            printf("\t%s\n", ipNumberString);
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);
}
void selectLocalAddress() {
   struct ifaddrs *addresses;
    if (getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        exit(-1);
    }
    struct ifaddrs *address = addresses;
    printf("Index\tName\tAddress\n");
    int i = 0;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET) {
            i++;
            printf("%d\t", i);
            printf("%s\t", address->ifa_name);
            char ipNumberString[100];
            getnameinfo(address->ifa_addr, sizeof(struct sockaddr_in), ipNumberString, sizeof(ipNumberString), 0, 0, NI_NUMERICHOST);
            printf("\t%s\n", ipNumberString);
        }
        address = address->ifa_next;
    }
    printf("Choose an address (1/2/3/etc.)\n");
    #define INDEX_STRING_SIZE 16
    char indexString[INDEX_STRING_SIZE] = {0};
    bool indexWasValid = false;
    int selectedIndex = -1;
    while (!indexWasValid && i > 0) {
        fgets(indexString, sizeof(indexString), stdin);
        selectedIndex = atoi(indexString); // atoi() returns 0 on invalid input
        indexWasValid = (selectedIndex > 0) && (selectedIndex <= i);
        if (!indexWasValid)
            printf("Invalid index, try again.\n");
        printf("Got '%s' = %d.\n", indexString, selectedIndex);
    }
    i = 0;
    address = addresses;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET) {
            i++;
            if (i == selectedIndex) {
                char ipNumberString[100];
                getnameinfo(address->ifa_addr, sizeof(struct sockaddr_in), ipNumberString, sizeof(ipNumberString), 0, 0, NI_NUMERICHOST);
                //bool myIpStringValid = inet_aton(ipNumberString, &myAddress.sin_addr);
                printf("Network interface selected: %s, %s\n", address->ifa_name, ipNumberString);
                break;
            }
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);
}
// Get an address of a specified type: 'e' for ethernet, 'l' for loopback, 'w' for wifi
void getDefaultAddress(char c, char* addressString, int stringLen) {
   struct ifaddrs *addresses;
    if (getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        exit(-1);
    }
    struct ifaddrs *address = addresses;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_LOCAL) {
            getnameinfo(address->ifa_addr, sizeof(struct sockaddr_in), addressString, stringLen, 0, 0, NI_NUMERICHOST);
            if (address->ifa_name[0] == c) {
                printf("%s, '%s' <-- SELECTED!\n", address->ifa_name, addressString);
                break;
            }
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);
}


// give this an int* to get a socket
void udpInit(int* sock, int my_port, int their_port, char* their_ip_address) {
    // set up my address
    myAddressLen = sizeof(myAddress);
    memset(&myAddress, 0, myAddressLen);
    memset(&myAddress,  0, myAddressLen);
    myAddress.sin_family = AF_INET; // IPv4
    myAddress.sin_addr.s_addr = INADDR_ANY;
    //selectLocalAddress();
    // convert the ip string to an ip number
    //int myIpStringValid = inet_aton(myIp_address, &myAddress.sin_addr);
    
    /*
    int theirIpStringValid = inet_aton(their_ip_address, &theirAddress.sin_addr);
    if (!theirIpStringValid)
        printf("*** Warning! IP address string in udpInit() was invalid!\n");
    */
    makeAddress(&theirAddress, their_port, their_ip_address);
    
    myIp = myAddress.sin_addr.s_addr;
    
    printf("My IP address:    %s\n", inet_ntoa(myAddress.sin_addr));
    printf("Their IP address: %s\n", inet_ntoa(theirAddress.sin_addr));
    
    myAddress.sin_port = htons(my_port);
    // set up their address
    theirAddressLen = sizeof(theirAddress);
    theirAddress.sin_port = htons(their_port);
    // get a socket for my address
    if ((*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("socket created\n");
    // set the socket options
    int enableBroadcast = 1;
    setsockopt(*sock, SOL_SOCKET, SO_BROADCAST,
            &enableBroadcast, sizeof(enableBroadcast));
    if (bind(*sock, 
            (const struct sockaddr *)&myAddress,
            myAddressLen
            ) < 0 
       )
    {
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }
    printf("socket bound\n");
}
void udpShut(int* socket) {
    close(*socket);
}

// send a udp message (assumes message is null-terminated)
void udpSend(char* send, int* sock) {
    dlog(NETCODE, "Sending message '%s' from (%s, %d)", send, inet_ntoa(myAddress.sin_addr), ntohs(myAddress.sin_port));
    dlog(NETCODE, " to (%s, %d)\n", inet_ntoa(theirAddress.sin_addr), ntohs(theirAddress.sin_port));
    int messageLen = strlen(send);
    if (messageLen > MAX_UDP_PAYLOAD) {
        fprintf(stderr, "*** udpSend() tried to send more than MAX_UDP_PAYLOAD bytes!\n");
    }
    sendto(*sock, 
            (const char *)send, 
            messageLen,
            MSG_CONFIRM, 
            (const struct sockaddr *) &theirAddress,
            theirAddressLen);
}
// send a udp message of fixed size
void udpSendN(char* send, int n, int* sock) {
    if (n > MAX_UDP_PAYLOAD) {
        fprintf(stderr, "*** udpSendN() tried to send more than MAX_UDP_PAYLOAD bytes!\n");
    }
    sendto(*sock, 
            (const char *)send, 
            n,
            MSG_CONFIRM, 
            (const struct sockaddr *) &theirAddress,
            theirAddressLen);
}
// recv a udp message
int udpRecv(char* recv, int* sock) {
    dlog(NETCODE, "Listening on (%s, %d)...\n", inet_ntoa(myAddress.sin_addr), ntohs(myAddress.sin_port));
    int recv_msg_len = recvfrom(*sock, 
                                (char *)recv, 
                                MAX_UDP_PAYLOAD,
                                MSG_WAITALL, 
                                0,//(struct sockaddr *) &myAddress,
                                0//&myAddressLen
                               );
    recv[recv_msg_len] = '\0';
    return recv_msg_len;
}
// recv a udp message of fixed size
int udpRecv_n(char* recv, int* sock, int size) {
    int recv_msg_len = recvfrom(*sock, 
                                (char *)recv, 
                                size,
                                MSG_WAITALL, 
                                (struct sockaddr *) &myAddress,
                                &myAddressLen
                               );
    recv[recv_msg_len] = '\0';
    return recv_msg_len;
}

#define LABEL_LEN 16
struct connectMessage {
    char label[LABEL_LEN];
    int address;
    int addressLen;
};

// find and connect to a server in the LAN
void findServer(int* my_socket) {
    printf("Finding server...\n");
    //
    // make a request message
    //
    struct connectMessage myMessage = {
        "+ hello server!\n",
        myIp,
        myAddressLen
    };
    // set the outgoing address to the broadcast address
    theirAddress.sin_addr.s_addr = INADDR_BROADCAST; //TODO find out why this is so hard to recv
    // spew the message to every device on the LAN
    udpSendN((char*)&myMessage, sizeof(struct connectMessage), my_socket);
    //
    // listen for the server's reply
    //
    struct connectMessage theirMessage = {0};
    while(1) {
        udpRecv_n((char*)&theirMessage, my_socket, sizeof(struct connectMessage));
        if (theirMessage.label[0] == '!')
            break;
        printf("Got message '%.16s'\n", (char*)&theirMessage);
    }
    myTempAddress.s_addr = theirMessage.address;
    printf("Server's response: %s, IP len: %d, IP: %s\n", theirMessage.label, theirMessage.addressLen, inet_ntoa(myTempAddress));
    // extract the server's ip info from the message
    theirAddressLen = theirMessage.addressLen;
    theirAddress.sin_addr.s_addr = theirMessage.address;
    // store their ip in the socket
    connect(*my_socket, (struct sockaddr *)&theirAddress,
				theirAddressLen);
    printf("Server's address: len %d, %s\n", theirAddressLen, inet_ntoa(theirAddress.sin_addr));
}

// respond to a findServer() call
void getClient(int* my_socket) {
    printf("Listening for clients...\n");
    //
    // listen for a client connection request
    //
    struct connectMessage theirMessage = {0};
    while(1) {
        // listen for any findServer() broadcasts
        udpRecv_n((char*)&theirMessage, my_socket, sizeof(struct connectMessage));
        if (theirMessage.label[0] == '+')
            break;
        printf("Got message '%.16s'\n", (char*)&theirMessage);
    }
    myTempAddress.s_addr = theirMessage.address;
    printf("Client message: %s, IP len %d, IP: %s\n", theirMessage.label, theirMessage.addressLen, inet_ntoa(myTempAddress));
    // extract the client's ip information from their message
    theirAddressLen = theirMessage.addressLen;
    theirAddress.sin_addr.s_addr = theirMessage.address;
    //theirAddress.sin_addr.s_addr = INADDR_BROADCAST;
    // store their ip in the socket
    connect(*my_socket, (struct sockaddr *)&theirAddress,
				theirAddressLen);
    //
    // tell the client our ip address
    //
    struct connectMessage myMessage = {
        "! hello client!\n",
        myIp,
        myAddressLen
    };
    udpSendN((char*)&myMessage, sizeof(struct connectMessage), my_socket);
    printf("Client's address: len %d, %s\n", theirAddressLen, inet_ntoa(theirAddress.sin_addr));
}
