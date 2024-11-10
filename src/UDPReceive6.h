/*
 *  UDPReceive6.h
 *
 *  Created by Helmut Hlavacs (2022).
 *
 */


extern "C" {
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <in6addr.h>
#include <ws2ipdef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <WS2tcpip.h>
}


class UDPReceive6 {

public:
	int sock;
	struct sockaddr_in6 addr;
	char* recbuffer;

	UDPReceive6();
	~UDPReceive6() { delete recbuffer; };
	void init(int port);
	int receive(char* buffer, int len, double* ptime);
	void closeSock();
};
