// camera_rt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lcm\lcm-cpp.hpp"
#include "Types\DiscoveryRequest_t.hpp"
#include "Types\StreamUriRequest_t.hpp"
#include "Types\StreamUriResponse_t.hpp"
#include <cpprest\http_client.h>
#include <cpprest\filestream.h>
#include <cpprest/uri.h> 
#include <iostream>
#include <string>

#include "udp_socket_provider.h"
#include "SunApiRequestResponseTypes\BroadcastTypes.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

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
			char get_req[400];
			memcpy(get_req, _main_get_req, 51);
			memcpy(get_req + 51, _stream_get_req, 77);
			char buff[200];
			sprintf_s(get_req, "192.168.0.148", "video", 
				"stream", "view", "0", "MJPEG", "600x450", "15", "10");
			
			//http_client client(U(""));

		}

	 const  char* _discovery_req_channel = "DISCOVERYREQ";
	 const char* _stream_req_channel = "STREAMURIREQ";
	 const char* _main_get_req = "http://%s%/stw-cgi/%s%.cgi?msubmenu=%s%&action=%s%";
	 const char* _stream_get_req = "&Profile=%s%&CodecType=%s%&Resolution=%s%&FrameRate=%s%&CompressionLevel=%s%";
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

