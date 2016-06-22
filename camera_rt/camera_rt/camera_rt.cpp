// camera_rt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "lcm\lcm-cpp.hpp"
#include "Types\discovery_request_t.hpp"
#include "Types\stream_uri_request_t.hpp"
#include "Types\stream_uri_response_t.hpp"
#include "Types\ptz_control_request_t.hpp"
#include <cpprest\http_client.h>
#include <cpprest\filestream.h>
#include <cpprest\uri.h> 
#include <iostream>
#include <string>

#include "udp_socket_provider.h"
#include "SunApiRequestResponseTypes\BroadcastTypes.h"
#include "uri_constants.h"
#include "channels.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

class LcmHandler
{
	public:
		~LcmHandler() {}		
		lcm::LCM* lcm;
		void HandleDiscoveryReq(const lcm::ReceiveBuffer* rbuf,
			const std::string& channel,
			const ptz_camera::discovery_request_t* req)
		{
			std::cout << "Received discovery req on channel: " << channel << std::endl;
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
			const ptz_camera::stream_uri_request_t* req)
		{
			std::cout << "Received get_stream_uri req on channel: " << channel << std::endl;
			std::cout << "Received stream req for camera: " << req->ip_address << std::endl;

			std::wstring camera_ip = std::wstring(req->ip_address.begin(),
				req->ip_address.end());

			wstring main_uri = L"http://" + camera_ip;

			auto camera_profile = convert_to_wstring(req->profile);
			auto camera_codec_type = convert_to_wstring(req->codec_type);
			auto camera_resolution = convert_to_wstring(req->resolution);
			auto camera_frame_rate = convert_to_wstring(req->frame_rate);
			auto camera_compression_level = convert_to_wstring(req->compression_level);
			auto camera_channel = convert_to_wstring(req->channel);		

			auto uri = uri_builder(main_uri).append_path(uri_constants::stw_cgi).
				append_path(uri_constants::video_cgi).
				append_query(uri_constants::sub_menu, uri_constants::sub_menu_stream).
				append_query(uri_constants::action, U("view"));

			if (camera_profile != L"")
				uri.append_query(uri_constants::profile, camera_profile);

			if (camera_codec_type != L"")
				uri.append_query(uri_constants::codec_type, camera_codec_type);

			if (camera_resolution != L"")
				uri.append_query(uri_constants::resolution, camera_resolution);

			if (camera_frame_rate != L"")
				uri.append_query(uri_constants::frame_rate, camera_frame_rate);

			if (camera_compression_level != L"")
				uri.append_query(uri_constants::compression_level, camera_compression_level);

			if (camera_channel != L"")
				uri.append_query(uri_constants::channel, camera_channel);	

			ptz_camera::stream_uri_response_t response;

			response.ip_address = convert_to_string(camera_ip);
			response.uri = convert_to_string(uri.to_string());
			lcm->publish(ptz_camera_channels::stream_res_channel, &response);
	}

		void HandlePtzControlRequest(const lcm::ReceiveBuffer* rbuf,
			const std::string& channel,
			const ptz_camera::ptz_control_request_t* req)
		{
			std::cout << "Received ptz control req on channel: " << channel << std::endl;
			std::cout << "Received ptz control req for camera: " << req->ip_address << std::endl;

			std::wstring camera_ip = std::wstring(req->ip_address.begin(),
				req->ip_address.end());

			wstring main_uri = L"http://" + camera_ip;


			auto camera_pan = to_wstring(req->pan_value);

			if (req->pan_value == 200)
				camera_pan = L"-100";
			
			auto camera_tilt = to_wstring(req->tilt_value);
			auto camera_zoom = to_wstring(req->zoom_value);

			auto config = http_client_config();			
			auto login = credentials(U("admin"), U("aotcamera01"));
			config.set_credentials(login);

			http_client client(main_uri, config);

			uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
				append_path(uri_constants::ptz_control_cgi);

			if (camera_pan == L"" && camera_tilt == L"" && camera_zoom == L"")
				return; //TODO send warning to client since none of ptz was provided

			uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_relative).
				append_query(uri_constants::action, uri_constants::action_control);

			if (camera_pan != L"")
				uri.append_query(uri_constants::pan, camera_pan);

			if (camera_tilt != L"")
				uri.append_query(uri_constants::tilt, camera_tilt);

			if (camera_zoom != L"")
				uri.append_query(uri_constants::zoom, camera_zoom);

			auto request = uri.to_string();

			client.request(methods::GET, request);


		}
	
private:
	std::wstring convert_to_wstring(string value)
	 {		
		 return std::wstring(value.begin(), value.end());
	 }

	std::string convert_to_string(wstring value)
	{
		return std::string (value.begin(), value.end());
	}
};

int _tmain(int argc, _TCHAR* argv[])
{	
	lcm::LCM lcm;

	if (!lcm.good())
	{
		std::cout << "Couldn't allocate lcm_t\n";
	}

	LcmHandler lcmHandler;
	lcmHandler.lcm = &lcm;

	lcm.subscribe(ptz_camera_channels::discovery_req_channel, &LcmHandler::HandleDiscoveryReq, &lcmHandler);
	lcm.subscribe(ptz_camera_channels::stream_req_channel, &LcmHandler::HandleStreamUriReq, &lcmHandler);
	lcm.subscribe(ptz_camera_channels::ptz_control_req_channel, &LcmHandler::HandlePtzControlRequest, &lcmHandler);

	while (0 == lcm.handle());

	return 0;
}

