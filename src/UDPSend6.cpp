/*
 *  UDPSend6.cpp
 *
 *  Created by Helmut Hlavacs (2022).
 *
 */


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


#include "UDPSend6.h"
#include "Logger.h"

extern "C" {
#include <stdio.h>
#include <time.h>
}
int startWinsock(void)
{
	WSADATA wsa;
	auto err = WSAStartup(MAKEWORD(2, 2), &wsa);
	return err;
}


typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;



void UDPSend6::init(char *address, int port ) {	
	sock = socket( AF_INET6, SOCK_DGRAM, 0 );

    struct addrinfo hints;

    memset(&addr, 0, sizeof(addr));
    memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;

    struct addrinfo *result = NULL;
    auto dwRetval = getaddrinfo(address, nullptr, &hints, &result);
    if ( dwRetval != 0 ) {
    	LOG_ERROR("getaddrinfo failed with error: " + std::to_string(dwRetval));
        return;
    }
	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		if (ptr->ai_family == AF_INET6) {
			memcpy(&addr, ptr->ai_addr, ptr->ai_addrlen);
			addr.sin6_port = htons(port);
			addr.sin6_family = AF_INET6;
		}
	}
	freeaddrinfo(result);
	LOG_INFO("Sender initialized - sending on port " +std::to_string(port));
}



int UDPSend6::send( char *buffer, int len ) {
	char sendbuffer[65000];
	
	packetnum++;
	
	if( len>65000 ) {
		return 0;
	}
		
	RTHeader_t header;
	header.time = clock() / (double)CLOCKS_PER_SEC;
	header.packetnum = packetnum;
		
	int ret;
	memcpy( sendbuffer, &header, sizeof( header ) );
    memcpy( sendbuffer + sizeof( header), buffer, len );
		
	ret = sendto( sock, sendbuffer, sizeof( header ) + len, 0, (const struct sockaddr *) &addr, sizeof(addr) );
    return ret;
}


void UDPSend6::closeSock() {
	closesocket(sock);
	sock=0;
}








