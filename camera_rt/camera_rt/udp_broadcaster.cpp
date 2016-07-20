

#include "stdafx.h"

#include <errno.h>
#include <iostream>
#include "udp_broadcaster.h"
#include <vector>

#define BUFLEN 512
#define CLIENT_PORT 7711
#define SERVER_PORT 7701

#pragma comment(lib, "Ws2_32.lib")

void udp_broadcaster::create()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
		std::cout << "WSAStartup failed.\n";
		system("pause");
		return;
	}
	// 0 specified for auto selection of protocol
	_client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (_client_socket == 0)
		perror("Error: client socket not created");

	memset(&_client_address, 0, sizeof(_client_address));
	_client_address.sin_family = AF_INET;
	_client_address.sin_port = htons(CLIENT_PORT);
	_client_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_client_socket, (sockaddr*)&_client_address, sizeof(sockaddr_in)) < 0)
	{
		errno = WSAGetLastError();
		std::cout << "Errno: " << errno << std::endl;
		perror("Error: bind to client socket failed");
		closesocket(_client_socket);
	}	
}

void udp_broadcaster::send(void* request, size_t size)
{
	memset(&_broadcast_address, '\0', sizeof(_broadcast_address));
	_broadcast_address.sin_family = AF_INET;
	_broadcast_address.sin_port = htons(SERVER_PORT);
	_broadcast_address.sin_addr.s_addr = INADDR_BROADCAST;

	int broadcast_on = 1;
	int set_socket_opt = setsockopt(_client_socket, SOL_SOCKET, SO_BROADCAST,
		(char*)&broadcast_on, sizeof(broadcast_on));
	if (set_socket_opt == -1)
		perror("Error: setsocketopt failed");	
	/*std::cout << "size of request: " << sizeof(request) << std::endl;*/
	int send = sendto(_client_socket, (char *)request, size,
		0, (sockaddr *)&_broadcast_address, sizeof(_broadcast_address));

	if (send == -1)
		perror("Error: sendto call failed");
}

std::vector<SunApiTypes::BroadcastResponsePacket> udp_broadcaster::recv()
{	
	std::vector<SunApiTypes::BroadcastResponsePacket> broadcast_responses;

	fd_set fds;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(_client_socket, &fds);

	int result = select(_client_socket + 1, &fds, NULL, NULL, &tv);
	if ( result == -1)
		perror("select failed on client socket");	

	auto timed_out = false;

	while (!timed_out)
		if (FD_ISSET(_client_socket, &fds))
		{
			SunApiTypes::BroadcastResponsePacket broadcast_response;
			int addr_size = sizeof(_client_address);
			int recv = recvfrom(_client_socket, (char*)&broadcast_response,
				sizeof(broadcast_response), 0,
				(sockaddr*)&_client_address, &addr_size);
			broadcast_responses.push_back(broadcast_response);	

			select(_client_socket + 1, &fds, NULL, NULL, &tv);
		}

		else
			timed_out = true;

	shutdown(_client_socket, 2);
	closesocket(_client_socket);	
	return broadcast_responses;
}
