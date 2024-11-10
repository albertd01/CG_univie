/*
 *  UDPReceive6.cpp
 *
 *  Created by Helmut Hlavacs (2022).
 *
 */


#include "UDPReceive6.h"
#include "Logger.h"
#include "UDPSend6.h"

extern "C" {
#include <stdio.h>
#include <time.h>
}

int startWinsockReceive(void)
{
	WSADATA wsa;
	auto err = WSAStartup(MAKEWORD(2, 2), &wsa);
	return err;
}

typedef struct RTHeader {
	double		  time;
	unsigned long packetnum;
} RTHeader_t;


UDPReceive6::UDPReceive6() {
	recbuffer = new char[65000];
}


void UDPReceive6::init(int port) {
	LOG_INFO("trying to initialize receiver socket with port " + std::to_string(port));
	sock = socket(AF_INET6, SOCK_DGRAM, 0);

	if(sock < 0)
	{
		LOG_ERROR("Socket creation failed");
	}

	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0) {
		LOG_ERROR("Failed to set socket options: " + std::to_string(WSAGetLastError()));
		return;
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	addr.sin6_addr = in6addr_any;
	auto ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	LOG_WARNING("Result after binding " + std::to_string(ret));
	LOG_INFO("Frame Receiver initialized - listening on port: " + std::to_string(port));
}


int UDPReceive6::receive(char* buffer, int len, double* ptime) {
	struct sockaddr_in6 si_other;
	socklen_t slen = sizeof(si_other);
	char recbuffer[65000];  // Adjust buffer size if needed

	while (true) {
		LOG_INFO("Trying to receive data");

		auto ret = recvfrom(sock, recbuffer, sizeof(recbuffer), 0, (sockaddr*)&si_other, &slen);
		if (ret < 0) {
			int error_code = WSAGetLastError();
			if (error_code == WSAEWOULDBLOCK || error_code == WSAEINTR) {
				LOG_WARNING("Non-fatal error while receiving data: " + std::to_string(error_code));
				continue;
			} else {
				LOG_ERROR("Failed to receive data. Error code: " + std::to_string(error_code));
				return -1;
			}

		}
		if (ret > (int)sizeof(RTHeader_t)) {
			int payload_len = ret - sizeof(RTHeader_t);
			if (payload_len > len) {
				LOG_ERROR("Buffer overflow. Received payload size exceeds buffer length.");
				return -1;
			}

			memcpy(buffer, recbuffer + sizeof(RTHeader_t), payload_len);
			return payload_len;
		}
	}
	return 0;
}


void UDPReceive6::closeSock() {
	closesocket(sock);
	sock = 0;
}




