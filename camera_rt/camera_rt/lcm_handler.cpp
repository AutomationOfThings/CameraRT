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
#include <set>

using namespace std;
using namespace concurrency;

void lcm_handler::on_init_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::init_session_request_t* req)
{
	std::cout << "Received init session req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;

	auto ip_address = req->ip_address;

	auto init_session_response = make_shared<ptz_camera::init_session_response_t>();
	init_session_response->ip_address = req->ip_address;
																				
	auto key_position = this->ip_client_map.find(ip_address);
	if (key_position == ip_client_map.end())
	{
		std::cout << "Creating a new client for the ip: "
			<< req->ip_address << endl;

		wstring username = this->convert_to_wstring(req->username);
		wstring password = this->convert_to_wstring(req->password);

		wstring main_uri = L"http://" + convert_to_wstring(ip_address);
		auto config = http_client_config();
		auto login = credentials(username, password);
		config.set_credentials(login);
		/*config.set_timeout(std::chrono::milliseconds(1000));*/

		http_client* client = new http_client(main_uri, config);
		std::cout << "Client created...\n";

		uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
			append_path(uri_constants::attributes_cgi).append_path(uri_constants::attributes);

		auto request = uri.to_string();
	
		client->request(methods::GET, request)
			.then([this, ip_address, client, init_session_response]
				(pplx::task<http_response> request_task) -> pplx::task<wstring>
		{
			try 
			{
				auto response = request_task.get();
				if (response.status_code() == status_codes::OK)
				{
					std::cout << "Saving client...";
					this->ip_client_map[ip_address] = client;
					std::cout << lcm_handler::ok_message;

					init_session_response->status_code =
					ptz_camera::status_codes_t::OK;
				}

				else
				{
					cout << "GET request to client failed! HTTP Error: "
						<< response.status_code() << std::endl;

					init_session_response->status_code =
						ptz_camera::status_codes_t::ERR;
				}

				return response.extract_string();
			}
				
			catch (const exception& e)
			{
				cout << "Caught exception: " << e.what() << endl;
				return create_task([e, this]() -> wstring
				{
					return convert_to_wstring(e.what());
				});
			}				

		})
			.then([init_session_response, this](wstring response)
		{
			string string_response = this->convert_to_string(response);
			init_session_response->response_message = string_response;
			this->lcm->publish(ptz_camera_channels::init_session_res_channel,
				init_session_response.get());
		});
	}


	else
	{
		string message = "Client for ip: " + req->ip_address + " already exists\n";
		cout << message << endl;
		init_session_response->response_message = message;
		init_session_response->status_code = ptz_camera::status_codes_t::OK;

		this->lcm->publish(ptz_camera_channels::init_session_res_channel,
			init_session_response.get());
	}
	
}

void lcm_handler::on_end_session_req(const lcm::ReceiveBuffer* rbuff,
	const std::string& channel,
	const ptz_camera::end_session_request_t* req)
{
	std::cout << "Received end_session_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::end_session_response_t end_session_response;

	auto key_position = this->ip_client_map.find(req->ip_address);
	if (key_position != ip_client_map.end())
	{
		cout << "Removing session for ip: " + req->ip_address + "...";
		delete key_position->second;
		this->ip_client_map.erase(req->ip_address);
		std::cout << lcm_handler::ok_message;

		end_session_response.status_code = ptz_camera::status_codes_t::OK;
		end_session_response.response_message = "OK";

	}

	else
	{		
		string message =  "No session initialized for ip: " + req->ip_address + "... request dropped!\n";
		cout << message;
		end_session_response.response_message = message;
		end_session_response.status_code = ptz_camera::status_codes_t::ERR;
	}


	this->lcm->publish(ptz_camera_channels::end_session_res_channel,
		&end_session_response);
	
}

void lcm_handler::on_discovery_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::discovery_request_t* req)
{
	std::cout << "Received discovery_req on channel: " << channel << std::endl;
	std::cout << "Sending broadcast request... ";

	udp_broadcaster udp_handler;
	udp_handler.create();
	SunApiTypes::BroadcastRequestPacket request_packet;
	memset(&request_packet, 0, sizeof(request_packet));
	request_packet.nMode = DEF_REQ_SCAN_EX;
	char* Id = "camerarruntimetes";
	memcpy(&request_packet.chPacketID, Id, 18);
	udp_handler.send(&request_packet, sizeof(request_packet));

	std::cout << lcm_handler::ok_message;

	std::cout << "Listening for broadcast response... ";
	auto broadcast_responses = udp_handler.recv();	
	std::cout << lcm_handler::ok_message;

	ptz_camera::discovery_response_t discovery_response;

	discovery_response.total_cams = broadcast_responses.size();
	std::cout << "Total cams discovered: " << discovery_response.total_cams << std::endl;
	discovery_response.status_code = ptz_camera::status_codes_t::OK;
	discovery_response.response_message = "OK";

	for (int i = 0; i < broadcast_responses.size(); i++)
		discovery_response.ip_addresses.push_back(broadcast_responses[i].chIP);
	
	std::cout << "Sending brodcast response... ";
	this->lcm->publish(ptz_camera_channels::discovery_res_channel, &discovery_response);
	std::cout << lcm_handler::ok_message;
}

void lcm_handler::on_stream_uri_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stream_uri_request_t* req)
{
	std::cout << "Received get_stream_uri req on channel: " << channel << 
		"; Camera: " << req->ip_address << std::endl;

	ptz_camera::stream_uri_response_t stream_uri_response;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: " 
			+ req->ip_address + "... request dropped!";
		cout << message << endl;
		stream_uri_response.status_code = ptz_camera::status_codes_t::ERR;
		stream_uri_response.response_message = message;		
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		
		std::cout << "Building uri... ";
		std::wstring camera_ip = convert_to_wstring(req->ip_address);

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

		std::cout << lcm_handler::ok_message;

		stream_uri_response.ip_address = convert_to_string(camera_ip);
		stream_uri_response.uri = convert_to_string(uri.to_string());
		stream_uri_response.status_code = ptz_camera::status_codes_t::OK;
		stream_uri_response.response_message = "OK";

	}

	std::cout << "Sending get_stream_uri response... ";
	lcm->publish(ptz_camera_channels::stream_res_channel, &stream_uri_response);
	std::cout << lcm_handler::ok_message;
}


void lcm_handler::on_ptz_conrol_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::ptz_control_request_t* req)
{
	std::cout << "Received ptz_control_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto control_response = make_shared<ptz_camera::ptz_control_response_t>();
	control_response->ip_address = req->ip_address;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: " 
			+ req->ip_address + "... request dropped!";
		cout << message << endl;
		control_response->status_code = ptz_camera::status_codes_t::ERR;
		control_response->response_message = message;

		std::cout << "Sending ptz_control response... ";
		this->lcm->publish(ptz_camera_channels::ptz_control_res_channel,
			control_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		std::cout << "Building request... ";

 		auto camera_pan = convert_to_wstring(req->pan_value);
		auto camera_tilt = convert_to_wstring(req->tilt_value);
		auto camera_zoom = convert_to_wstring(req->zoom_value);

		uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
			append_path(uri_constants::ptz_control_cgi);

		if (camera_pan == L"" && camera_tilt == L"" && camera_zoom == L"")
			return; //TODO send warning to client since none of ptz was provided

		if (req->mode == ptz_camera::ptz_control_request_t::CON)
			uri.append_query(uri_constants::sub_menu,
				uri_constants::sub_menu_continuous);

		if (req->mode == ptz_camera::ptz_control_request_t::ABS)
			uri.append_query(uri_constants::sub_menu, 
				uri_constants::sub_menu_absolute);

		if (req->mode == ptz_camera::ptz_control_request_t::REL)
			uri.append_query(uri_constants::sub_menu,
				uri_constants::sub_menu_relative);

		uri.append_query(uri_constants::action, uri_constants::action_control);

		if (camera_pan != L"")
			uri.append_query(uri_constants::pan, camera_pan);

		if (camera_tilt != L"")
			uri.append_query(uri_constants::tilt, camera_tilt);

		if (camera_zoom != L"")
			uri.append_query(uri_constants::zoom, camera_zoom);

		auto request = uri.to_string();

		std::cout << lcm_handler::ok_message;
		std::cout << "Request: " << convert_to_string(request) << std::endl;		

		std::cout << "Sending ptz_control request... ";
		ip_client_pair->second->request(methods::GET, request)
			.then([control_response, this](pplx::task<http_response> request_task) -> pplx::task<wstring>
			{
				try
				{
					auto response = request_task.get();
					if (response.status_code() == status_codes::OK)
					{
						std::cout << lcm_handler::ok_message;
						control_response->status_code =
							ptz_camera::status_codes_t::OK;
					}

					else
					{
						std::cout << "GET request to client failed! HTTP Error: "
							<< response.status_code() << std::endl;
						control_response->status_code =
							ptz_camera::status_codes_t::ERR;
					}

					return response.extract_string();
				}

				catch (const exception& e)
				{
					cout << "Caught exception: " << e.what() << endl;
					return create_task([e, this]() -> wstring
					{
						return this->convert_to_wstring(e.what());
					});
				}

			})
			.then([control_response, this](wstring response)
			{
				control_response->response_message =
					this->convert_to_string(response);

				std::cout << "Sending ptz_control response... ";
				this->lcm->publish(ptz_camera_channels::ptz_control_res_channel,
					control_response.get());
				std::cout << lcm_handler::ok_message;
			});
	}	
}

void lcm_handler::on_stop_ptz_control_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stop_ptz_control_request_t* req)
{
	std::cout << "Received stop_ptz_control_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto stop_ptz_control_response = make_shared<ptz_camera::stop_ptz_control_response_t>();
	stop_ptz_control_response->ip_address = req->ip_address;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: "
			+ req->ip_address + "... request DROPPED!";
		cout << message << endl;
		stop_ptz_control_response->status_code = ptz_camera::status_codes_t::ERR;
		stop_ptz_control_response->response_message = message;
		std::cout << "Sending stop_ptz_control response... ";
		this->lcm->publish(ptz_camera_channels::stop_ptz_control_res_channel,
			stop_ptz_control_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		std::cout << "Building request... ";

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
		std::cout << lcm_handler::ok_message;
		std::cout << "Request: " << convert_to_string(stop_ptz_control_request) << std::endl;
		
		std::cout << "Sending stop_ptz_control request... ";
		ip_client_pair->second->request(methods::GET, stop_ptz_control_request)
			.then(
				[stop_ptz_control_response, this](
					pplx::task<http_response> request_task) -> pplx::task<wstring>
		{
			try
			{
				auto response = request_task.get();
				if (response.status_code() == status_codes::OK)
				{
					std::cout << lcm_handler::ok_message;
					stop_ptz_control_response->status_code =
						ptz_camera::status_codes_t::OK;
				}

				else
				{
					std::cout << "GET request to client failed! HTTP Error: "
						<< response.status_code() << std::endl;
					stop_ptz_control_response->status_code =
						ptz_camera::status_codes_t::ERR;
				}

				return response.extract_string();
			}

			catch (const exception& e)
			{
				cout << "Caught exception: " << e.what() << endl;
				return create_task([e, this]() -> wstring
				{
					return this->convert_to_wstring(e.what());
				});
			}

		})
			.then([stop_ptz_control_response, this](wstring response)
		{
			stop_ptz_control_response->
				response_message = convert_to_string(response);	
			std::cout << "Sending stop_ptz_control response... ";
			this->lcm->publish(ptz_camera_channels::stop_ptz_control_res_channel,
				stop_ptz_control_response.get());
			std::cout << lcm_handler::ok_message;
		});
	}
}


void lcm_handler::on_position_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::position_request_t* req)
{
	std::cout << "Received position_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto position_response = make_shared<ptz_camera::position_response_t>();
	position_response->ip_address = req->ip_address;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: "
			+ req->ip_address + "... request dropped";
		cout << message << endl;
		position_response->status_code = ptz_camera::status_codes_t::ERR;
		position_response->response_message = message;
		std::cout << "Sending get_position response... ";
		this->lcm->publish(ptz_camera_channels::position_res_channel,
			position_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		std::cout << "Building request... ";

		uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
			append_path(uri_constants::ptz_control_cgi);

		uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_query).
			append_query(uri_constants::action, uri_constants::action_view).
			append_query(uri_constants::channel, L"0").
			append_query(uri_constants::query, L"Pan,Tilt,Zoom");

		auto request = uri.to_string();
		std::cout << lcm_handler::ok_message;
		std::cout << "Request: " << convert_to_string(request) << std::endl;

		std::cout << "Sending position request... ";
		ip_client_pair->second->request(methods::GET, request)
			.then(
				[position_response, this](
					pplx::task<http_response> request_task) -> pplx::task<wstring>
			{
				try
				{
					auto response = request_task.get();
					if (response.status_code() == status_codes::OK)
					{
						std::cout << lcm_handler::ok_message;
						position_response->status_code =
							ptz_camera::status_codes_t::OK;
					}

					else
					{
						std::cout << "GET request to client failed! HTTP Error: "
							<< response.status_code() << std::endl;
						position_response->status_code =
							ptz_camera::status_codes_t::ERR;
					}

					return response.extract_string();
				}

				catch (const exception& e)
				{
					cout << "Caught exception: " << e.what() << endl;
					return create_task([e, this]() -> wstring
					{
						return this->convert_to_wstring(e.what());
					});
				}

			})
			.then([position_response, this](wstring response)
			{
				sun_api::response_parser parser;
				auto response_map = parser.parse_string(convert_to_string(response));
				position_response->pan_value = response_map["Pan"];
				position_response->tilt_value = response_map["Tilt"];
				position_response->zoom_value = response_map["Zoom"];
				position_response->response_message = "OK";

				std::cout << "Sending get_position response... ";
				this->lcm->publish(ptz_camera_channels::position_res_channel,
					position_response.get());
				std::cout << lcm_handler::ok_message;
			});
	}
}