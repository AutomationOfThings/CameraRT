#pragma once

#ifndef __services_udp_socket_provider__
#define __services_udp_socket_provider__

#include "SunApiRequestResponseTypes\BroadcastTypes.h"
#include <winsock2.h>
#include <vector>

class udp_broadcaster
{
public:
	void create();
	std::vector<SunApiTypes::BroadcastResponsePacket> recv();	
	void send(void* request, size_t size);
	
private:
	int _client_socket;
	sockaddr_in _broadcast_address;
	sockaddr_in _client_address;
};

#endif