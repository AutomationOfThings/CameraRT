#include "stdafx.h"
#include "lcm_handler.h"
#include "channel_names.h"
#include "udp_broadcaster.h"
#include "uri_constants.h"
#include "sun_api_response_parser.h"
#include "Types\init_session_response_t.hpp"
#include "Types\end_session_response_t.hpp"
#include "Types\stream_uri_response_t.hpp"
#include "Types\ptz_control_response_t.hpp"
#include "Types\position_response_t.hpp"
#include "Types\stop_ptz_control_response_t.hpp"
#include "Types\status_codes_t.hpp"

#include <ppltasks.h>

using namespace std;
using namespace concurrency;

void lcm_handler::on_init_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::init_session_request_t* req)
{
	std::cout << "Received init session req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;
	
	auto key_position = this->ip_client_map.find(req->ip_address);
	if (key_position == ip_client_map.end())
	{
		wstring username = this->convert_to_wstring(req->username);
		wstring password = this->convert_to_wstring(req->password);

		wstring main_uri = L"http://" + convert_to_wstring(req->ip_address);
		auto config = http_client_config();
		auto login = credentials(username, password);
		config.set_credentials(login);

		http_client* client = new http_client(main_uri, config);

		ptz_camera::init_session_response_t init_session_response;

		client->request(methods::GET)
			.then([&init_session_response, this, client, req]
				(http_response response) -> pplx::task<wstring>
		{
			if (response.status_code() == status_codes::OK)
			{
				this->ip_client_map[req->ip_address] = client;
				init_session_response.status_code =
				ptz_camera::status_codes_t::OK;
			}

			else
				init_session_response.status_code =
				ptz_camera::status_codes_t::ERR;

			return response.extract_string();

		})
			.then([&init_session_response, this](wstring response)
		{
			init_session_response.response_message =
				this->convert_to_string(response);

			this->lcm->publish(ptz_camera_channels::init_session_req_channel,
				&init_session_response);
		});

	}

	// at least one init session request was received
	// so subscribe for other request types

	this->uri_req_sub = this->lcm->subscribe(
		ptz_camera_channels::stream_req_channel,
		&lcm_handler::on_stream_uri_req, this);

	this->ptz_control_req_sub = 
		this->lcm->subscribe(
			ptz_camera_channels::ptz_control_req_channel,
		&lcm_handler::on_ptz_conrol_req, this);

	this->stop_ptz_control_req_sub = this->lcm->subscribe(
		ptz_camera_channels::stop_ptz_control_req_channel,
		&lcm_handler::on_stop_ptz_control_req, this);

	this->position_req_sub = this->lcm->subscribe(
		ptz_camera_channels::position_req_channel, 
		&lcm_handler::on_position_req, this);

	this->lcm->subscribe(ptz_camera_channels::end_session_req_channel, 
		&lcm_handler::on_end_session_req, this);
}

void lcm_handler::on_end_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::end_session_request_t* req)
{
	std::cout << "Received end_session_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	
	this->ip_client_map.erase(req->ip_address);
}

void lcm_handler::on_discovery_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::discovery_request_t* req)
{
	std::cout << "Received discovery_req on channel: " << channel << std::endl;
	std::cout << "Sending broadcast request... " << std::endl;

	udp_broadcaster udp_handler;
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
	discovery_response.status_code = ptz_camera::status_codes_t::OK;
	discovery_response.response_message = "OK";

	for (int i = 0; i < broadcast_responses.size(); i++)
		discovery_response.ip_addresses.push_back(broadcast_responses[i].chIP);
	
	this->lcm->publish(ptz_camera_channels::discovery_res_channel, &discovery_response);
}

void lcm_handler::on_stream_uri_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stream_uri_request_t* req)
{
	std::cout << "Received get_stream_uri req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::stream_uri_response_t response;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for this ip... request dropped";
		response.status_code = ptz_camera::status_codes_t::ERR;
		response.response_message = message;		
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
		response.status_code = ptz_camera::status_codes_t::OK;
		response.response_message = "OK";
	}

	lcm->publish(ptz_camera_channels::stream_res_channel, &response);
}


void lcm_handler::on_ptz_conrol_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::ptz_control_request_t* req)
{
	std::cout << "Received ptz_control_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::ptz_control_response_t control_response;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for this ip... request dropped";
		cout << message << endl;
		control_response.status_code = ptz_camera::status_codes_t::ERR;
		control_response.response_message = message;
	}

	auto camera_pan = convert_to_wstring(req->pan_value);
	auto camera_tilt = convert_to_wstring(req->tilt_value);
	auto camera_zoom = convert_to_wstring(req->zoom_value);

	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	if (camera_pan == L"" && camera_tilt == L"" && camera_zoom == L"")
		return; //TODO send warning to client since none of ptz was provided

	if (req->mode == ptz_camera::ptz_control_request_t::CON)
		uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_relative);

	if (req->mode == ABSOLUTE)
		uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_absolute);

	uri.append_query(uri_constants::action, uri_constants::action_control);

	if (camera_pan != L"")
		uri.append_query(uri_constants::pan, camera_pan);

	if (camera_tilt != L"")
		uri.append_query(uri_constants::tilt, camera_tilt);

	if (camera_zoom != L"")
		uri.append_query(uri_constants::zoom, camera_zoom);

	auto request = uri.to_string();
	
	control_response.ip_address = req->ip_address;

	ip_client_pair->second->request(methods::GET, request)
		.then( [&control_response] (http_response response) -> pplx::task<wstring>
		{
			if (response.status_code() == status_codes::OK)
				control_response.status_code = 
					ptz_camera::status_codes_t::OK;

			else
				control_response.status_code = 
					ptz_camera::status_codes_t::ERR;			

			return response.extract_string();
		
		})
		.then( [&control_response, this] (wstring response)
		{					
			control_response.response_message = 
				this->convert_to_string(response);

			this->lcm->publish(ptz_camera_channels::ptz_control_res_channel,
				&control_response);
		});

}

void lcm_handler::on_stop_ptz_control_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stop_ptz_control_request_t* req)
{
	std::cout << "Received stop_ptz_control_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::stop_ptz_control_response_t stop_ptz_control_response;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for this ip... request dropped";
		cout << message << endl;
		stop_ptz_control_response.status_code = ptz_camera::status_codes_t::ERR;
		stop_ptz_control_response.response_message = message;
	}

	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_stop).
		append_query(uri_constants::action, uri_constants::action_control);

	switch (req->operation_type)
	{
		case ptz_camera::stop_ptz_control_request_t::ALL:
			uri.append_query(uri_constants::operation_type, uri_constants::all);
			break;

		case ptz_camera::stop_ptz_control_request_t::PAN:
			uri.append_query(uri_constants::operation_type, uri_constants::pan);
			break;

		case ptz_camera::stop_ptz_control_request_t::TILT:
			uri.append_query(uri_constants::operation_type, uri_constants::tilt);
			break;

		case ptz_camera::stop_ptz_control_request_t::ZOOM:
			uri.append_query(uri_constants::operation_type, uri_constants::zoom);
			break;
	}

	auto stop_ptz_control_request = uri.to_string();	
	stop_ptz_control_response.ip_address = req->ip_address;

	ip_client_pair->second->request(methods::GET, stop_ptz_control_request)
		.then(
			[&stop_ptz_control_response](http_response response) -> pplx::task<wstring>
	{
		if (response.status_code() == status_codes::OK)
			stop_ptz_control_response.status_code =
			ptz_camera::status_codes_t::OK;

		else
			stop_ptz_control_response.status_code = 
			ptz_camera::status_codes_t::ERR;

		return response.extract_string();

	})
		.then([&stop_ptz_control_response, this](wstring response)
	{		
		stop_ptz_control_response.response_message = convert_to_string(response);
		this->lcm->publish(ptz_camera_channels::stop_ptz_control_res_channel,
			&stop_ptz_control_response);
	});


}


void lcm_handler::on_position_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::position_request_t* req)
{
	std::cout << "Received position_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::position_response_t position_response;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for this ip... request dropped";
		cout << message << endl;
		position_response.status_code = ptz_camera::status_codes_t::ERR;
		position_response.response_message = message;
	}

	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_query).
		append_query(uri_constants::action, uri_constants::action_view).
		append_query(uri_constants::channel, L"0").
		append_query(uri_constants::query, L"Pan,Tilt,Zoom");
	
	auto request = uri.to_string();

	
	position_response.ip_address = req->ip_address;

	ip_client_pair->second->request(methods::GET, request)
		.then(
			[&position_response](http_response response) -> pplx::task<wstring>
		{
			if (response.status_code() == status_codes::OK)
				position_response.status_code = 
					ptz_camera::status_codes_t::OK;			

			else
				position_response.status_code = 
					ptz_camera::status_codes_t::ERR;			

			return response.extract_string();

		})
		.then([&position_response, this](wstring response)
		{
			sun_api::response_parser parser;
			auto response_map = parser.parse_string(convert_to_string(response));
			position_response.pan_value = response_map["Pan"];
			position_response.tilt_value = response_map["Tilt"];
			position_response.zoom_value = response_map["Zoom"];
			position_response.response_message = "OK";

			this->lcm->publish(ptz_camera_channels::position_res_channel, 
				&position_response);
		});
	}