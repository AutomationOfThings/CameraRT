// camera_rt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "..\lcm\lcm.h"
#include "lcm\lcm-cpp.hpp"
#include "Types\DiscoveryRequest_t.hpp"
#include "Types\StreamUriRequest_t.hpp"
#include "Types\StreamUriResponse_t.hpp"
#include <iostream>

#include "udp_socket_provider.h"
#include "SunApiRequestResponseTypes\BroadcastTypes.h"

class LcmHandler
{
	public:
		~LcmHandler() {}
		void HandleDiscoveryReq(const lcm::ReceiveBuffer* rbuf,
			const std::string& channel,
			const PtzCamera::DiscoveryRequest_t* req)
		{
			std::cout << "Received message on channel: " << channel << std::endl;
			std::cout << "Sending broadcast request... " << std::endl;

			udp_socket_provider udp_handler;
			udp_handler.create();
			SunApiTypes::BroadcastRequestPacket request_packet;
			memset(&request_packet, 0, sizeof(request_packet));
			request_packet.nMode = DEF_REQ_SCAN_EX;
			char* Id = "camerarruntimetes";
			memcpy(&request_packet.chPacketID, Id, 18);
			udp_handler.send(request_packet);

			std::cout << "Sent broadcast request... " << std::endl;
			
			std::cout << "Listening for broadcast response... " << std::endl;

			SunApiTypes::BroadcastResponsePacket response_packet =
				udp_handler.recv();

		}

		void HandleStreamUriReq(const lcm::ReceiveBuffer* rbuf,
			const std::string& channel,
			const PtzCamera::StreamUriRequest_t* req)
		{
			std::cout << "Received message on channel: " << channel << std::endl;
			std::cout << "Received stream request for camera: " << req->cameraName << std::endl;
		}

	 const  char* _discovery_req_channel = "DISCOVERYREQ";
	 const char* _stream_req_channel = "STREAMURIREQ";
};

int _tmain(int argc, _TCHAR* argv[])
{
	/*lcm_t *lcm = lcm_create(NULL);
	if (!lcm) {*/

	lcm::LCM lcm;

	if (!lcm.good())
	{
		std::cout << "Couldn't allocate lcm_t\n";
	}

	LcmHandler lcmHandler;
	lcm.subscribe("DISCOVERYREQ", &LcmHandler::HandleDiscoveryReq, &lcmHandler);
	lcm.subscribe("STREAMURIREQ", &LcmHandler::HandleStreamUriReq, &lcmHandler);

	while (0 == lcm.handle());

	return 0;
}

