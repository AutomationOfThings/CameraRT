#include "stdafx.h"
#include "lcm_handler.h"
#include "channel_names.h"
#include "udp_socket_provider.h"
#include "uri_constants.h"
#include "sun_api_response_parser.h"
#include "Types\ptz_control_response_t.hpp"
#include <ppltasks.h>

using namespace std;
using namespace concurrency;

void lcm_handler::on_init_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::init_session_request_t* req)
{
	std::cout << "Received init session req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;
	
	this->session_ip = req->ip_address;
	auto key_position = this->ip_client_map.find(session_ip);
	if (key_position == ip_client_map.end())
	{
		wstring username = this->convert_to_wstring(req->username);
		wstring password = this->convert_to_wstring(req->password);

		wstring main_uri = L"http://" + convert_to_wstring(session_ip);
		auto config = http_client_config();
		auto login = credentials(username, password);
		config.set_credentials(login);

		http_client* client = new http_client(main_uri, config);
		this->ip_client_map[session_ip] = client;
	}

	this->uri_req_sub = this->lcm->subscribe(ptz_camera_channels::stream_req_channel,
		&lcm_handler::on_stream_uri_req, this);
	this->ptz_control_req_sub = 
		this->lcm->subscribe(ptz_camera_channels::ptz_control_req_channel,
		&lcm_handler::on_ptz_conrol_req, this);
	this->position_req_sub = 
		this->lcm->subscribe(ptz_camera_channels::position_req_channel, &lcm_handler::on_position_req, this);
}

void lcm_handler::on_end_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::end_session_request_t* req)
{
	this->ip_client_map.erase(req->ip_address);

	this->lcm->unsubscribe(uri_req_sub);
	this->lcm->unsubscribe(ptz_control_req_sub);

	this->session_ip = "";
}

void lcm_handler::on_discovery_req(const lcm::ReceiveBuffer* rbuf,
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
	udp_handler.send(&request_packet, sizeof(request_packet));

	std::cout << "Sent broadcast request... " << std::endl;
	std::cout << "Listening for broadcast response... " << std::endl;

	auto broadcast_responses = udp_handler.recv();

	std::cout << "Received broadcast response... " << std::endl;

	ptz_camera::discovery_response_t discovery_response;

	discovery_response.total_cams = broadcast_responses.size();
	for (int i = 0; i < broadcast_responses.size(); i++)
		discovery_response.camera_names.push_back(broadcast_responses[i].chIP);
	
	this->lcm->publish(ptz_camera_channels::discovery_res_channel, &discovery_response);
}

void lcm_handler::on_stream_uri_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stream_uri_request_t* req)
{
	std::cout << "Received get_stream_uri req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;	
	ptz_camera::stream_uri_response_t response;

	if (req->ip_address != this->session_ip)
	{
		string message = "Session not initialized for this ip... request dropped";
		response.response_message = message;
		return;
	}

	else
	{
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

		response.ip_address = convert_to_string(camera_ip);
		response.uri = convert_to_string(uri.to_string());
		response.response_message = "OK";
	}

	lcm->publish(ptz_camera_channels::stream_res_channel, &response);
}


void lcm_handler::on_ptz_conrol_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::ptz_control_request_t* req)
{
	std::cout << "Received ptz control req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto req_copy = new ptz_camera::ptz_control_request_t;
	*req_copy = *req;

	http_client* client = this->ip_client_map[req->ip_address];
	auto camera_pan = to_wstring(req->pan_value);
	auto camera_tilt = to_wstring(req->tilt_value);
	auto camera_zoom = to_wstring(req->zoom_value);

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

	client->request(methods::GET, request)
		.then( [] (http_response response) -> pplx::task<wstring>
		{		
			if (response.status_code() == status_codes::OK)
				return create_task([]() -> wstring { return L"OK"; });

			else
				return response.extract_string();
		
		})
		.then( [=] (wstring response)
		{
			ptz_camera::ptz_control_response_t control_response;

			control_response.ip_address = req_copy->ip_address;
			control_response.response_message = 
				this->convert_to_string(response);

			this->lcm->publish(ptz_camera_channels::ptz_control_res_channel,
				&control_response);

			delete req_copy;
		});

}

void lcm_handler::on_position_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::position_request_t* req)
{
	std::cout << "Received position req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto req_copy = new ptz_camera::position_request_t;
	*req_copy = *req;

	http_client* client = this->ip_client_map[req->ip_address];

	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_query).
		append_query(uri_constants::action, uri_constants::action_view).
		append_query(uri_constants::channel, L"0").
		append_query(uri_constants::query, L"Pan,Tilt,Zoom");
	
	auto request = uri.to_string();

	client->request(methods::GET, request)
		.then(
			[=](http_response response) -> pplx::task<wstring>
		{
			if (response.status_code() == status_codes::OK)
				return response.extract_string();

			else
				return response.extract_string();

		})
		.then([=](wstring response_t)
		{
			sun_api::response_parser parser;
			auto response_map = parser.parse_string(convert_to_string(response_t));

			ptz_camera::position_response_t position_response;
			position_response.ip_address = req_copy->ip_address;
			position_response.pan_value = response_map["Pan"];
			position_response.tilt_value = response_map["Tilt"];
			position_response.zoom_value = response_map["Zoom"];
			position_response.response_message = "OK";

			this->lcm->publish(ptz_camera_channels::position_res_channel, 
				&position_response);

		});
	}