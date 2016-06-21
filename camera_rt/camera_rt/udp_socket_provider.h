#pragma once

#ifndef __services_udp_socket_provider__
#define _services_udp_socket_provider

#include "SunApiRequestResponseTypes\BroadcastTypes.h"
#include <winsock2.h>

class udp_socket_provider
{
public:
	void create();
	void send(SunApiTypes::BroadcastRequestPacket request);
	SunApiTypes::BroadcastResponsePacket recv();
private:
	int _client_socket;
	sockaddr_in _broadcast_address;
	sockaddr_in _server_address;
};

#endif