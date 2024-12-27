// make sending messages easier

#include "my_udp.h"
#include <time.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdbool.h>

struct sockaddr_in my_address;
struct sockaddr_in their_address;
int my_address_len;
int their_address_len;
struct in_addr my_temp_address;
int my_ip;

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
        fgets(indexString, sizeof(indexString), stdin) != 0;
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
                bool myIpStringValid = inet_aton(ipNumberString, &my_address.sin_addr);
                printf("Network interface selected: %s, %s\n", address->ifa_name, ipNumberString);
                break;
            }
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);
}


// give this an int* to get a socket
void udp_init(int* sock, int my_port, int their_port, char* their_ip_address) {
    // set up my address
    my_address_len = sizeof(my_address);
    memset(&my_address, 0, my_address_len);
    memset(&my_address,  0, my_address_len);
    my_address.sin_family = AF_INET; // IPv4
    my_address.sin_addr.s_addr = INADDR_ANY;
    selectLocalAddress();
    // convert the ip string to an ip number
    //int myIpStringValid = inet_aton(my_ip_address, &my_address.sin_addr);
    int theirIpStringValid = inet_aton(their_ip_address, &their_address.sin_addr);
    if (!theirIpStringValid)
        printf("*** Warning! IP address string in udp_init() was invalid!\n");
    my_ip = my_address.sin_addr.s_addr;
    
    printf("My IP address:    %s\n", inet_ntoa(my_address.sin_addr));
    printf("Their IP address: %s\n", inet_ntoa(their_address.sin_addr));
    
    my_address.sin_port = htons(my_port);
    // set up their address
    their_address_len = sizeof(their_address);
    their_address.sin_port = htons(their_port);
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
            (const struct sockaddr *)&my_address,
            my_address_len
            ) < 0 
       )
    {
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }
    printf("socket bound\n");
}
void udp_shut(int* socket) {
    close(*socket);
}
// send a udp message
void udp_send(char* send, int* sock) {
    DMSG("Sending message '%s' from (%s, %d)", send, inet_ntoa(my_address.sin_addr), ntohs(my_address.sin_port));
    DMSG(" to (%s, %d)\n", inet_ntoa(their_address.sin_addr), ntohs(their_address.sin_port));
    sendto(*sock, 
            (const char *)send, 
            strlen(send),
            MSG_CONFIRM, 
            (const struct sockaddr *) &their_address,
            their_address_len);
}
// send a udp message of fixed size
void udp_send_n(char* send, int n, int* sock) {
    sendto(*sock, 
            (const char *)send, 
            n,
            MSG_CONFIRM, 
            (const struct sockaddr *) &their_address,
            their_address_len);
}
// recv a udp message
int udp_recv(char* recv, int* sock) {
    DMSG("Listening on (%s, %d)...\n", inet_ntoa(my_address.sin_addr), ntohs(my_address.sin_port));
    int recv_msg_len = recvfrom(*sock, 
                                (char *)recv, 
                                MAX_BUF_SIZE,
                                MSG_WAITALL, 
                                0,//(struct sockaddr *) &my_address,
                                0//&my_address_len
                               );
    recv[recv_msg_len] = '\0';
    return recv_msg_len;
}
// recv a udp message of fixed size
int udp_recv_n(char* recv, int* sock, int size) {
    int recv_msg_len = recvfrom(*sock, 
                                (char *)recv, 
                                size,
                                MSG_WAITALL, 
                                (struct sockaddr *) &my_address,
                                &my_address_len
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
void find_server(int* my_socket) {
    printf("Finding server...\n");
    //
    // make a request message
    //
    struct connectMessage myMessage = {
        "+ hello server!\n",
        my_ip,
        my_address_len
    };
    // set the outgoing address to the broadcast address
    their_address.sin_addr.s_addr = INADDR_BROADCAST; //TODO find out why this is so hard to recv
    // spew the message to every device on the LAN
    udp_send_n((char*)&myMessage, sizeof(struct connectMessage), my_socket);
    //
    // listen for the server's reply
    //
    struct connectMessage theirMessage = {0};
    while(1) {
        udp_recv_n((char*)&theirMessage, my_socket, sizeof(struct connectMessage));
        if (theirMessage.label[0] == '!')
            break;
        printf("Got message '%.16s'\n", (char*)&theirMessage);
    }
    my_temp_address.s_addr = theirMessage.address;
    printf("Server's response: %s, IP len: %d, IP: %s\n", theirMessage.label, theirMessage.addressLen, inet_ntoa(my_temp_address));
    // extract the server's ip info from the message
    their_address_len = theirMessage.addressLen;
    their_address.sin_addr.s_addr = theirMessage.address;
    // store their ip in the socket
    connect(*my_socket, (struct sockaddr *)&their_address,
				their_address_len);
    printf("Server's address: len %d, %s\n", their_address_len, inet_ntoa(their_address.sin_addr));
}

// respond to a find_server() call
void get_client(int* my_socket) {
    printf("Listening for clients...\n");
    //
    // listen for a client connection request
    //
    struct connectMessage theirMessage = {0};
    while(1) {
        // listen for any find_server() broadcasts
        udp_recv_n((char*)&theirMessage, my_socket, sizeof(struct connectMessage));
        if (theirMessage.label[0] == '+')
            break;
        printf("Got message '%.16s'\n", (char*)&theirMessage);
    }
    my_temp_address.s_addr = theirMessage.address;
    printf("Client message: %s, IP len %d, IP: %s\n", theirMessage.label, theirMessage.addressLen, inet_ntoa(my_temp_address));
    // extract the client's ip information from their message
    their_address_len = theirMessage.addressLen;
    their_address.sin_addr.s_addr = theirMessage.address;
    //their_address.sin_addr.s_addr = INADDR_BROADCAST;
    // store their ip in the socket
    connect(*my_socket, (struct sockaddr *)&their_address,
				their_address_len);
    //
    // tell the client our ip address
    //
    struct connectMessage myMessage = {
        "! hello client!\n",
        my_ip,
        my_address_len
    };
    udp_send_n((char*)&myMessage, sizeof(struct connectMessage), my_socket);
    printf("Client's address: len %d, %s\n", their_address_len, inet_ntoa(their_address.sin_addr));
}
