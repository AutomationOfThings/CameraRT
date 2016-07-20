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
#include "Types\preset_config_response_t.hpp"
#include "Types\preset_move_response_t.hpp"
#include "Types\start_program_response_t.hpp"
#include "Types\stop_program_response_t.hpp"
#include "Types\output_request_t.hpp"
#include "Types\status_codes_t.hpp"
#include "messages.h"
#include "exceptions.h"
#include <ppltasks.h>
#include <chrono>
#include <thread>


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
		std::cout << lcm_handler::ok_message;

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
					cout << "GET request to server failed! HTTP Error: "
						<< response.status_code() << std::endl;

					init_session_response->status_code =
						ptz_camera::status_codes_t::ERR;
				}

				return response.extract_string();
			}

			catch (const http_exception& e)
			{
				cout << "Caught http exception: " << e.what() << endl;
				return create_task([e, this]() -> wstring
				{
					return convert_to_wstring(e.what());
				});
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
			this->lcm->publish(ptz_camera_res_channels::init_session_res_channel,
				init_session_response.get());
		});
	}
	
	else
	{
		string message = "Client for ip: " + req->ip_address + " already exists\n";
		cout << message << endl;
		init_session_response->response_message = message;
		init_session_response->status_code = ptz_camera::status_codes_t::OK;

		this->lcm->publish(ptz_camera_res_channels::init_session_res_channel,
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


	this->lcm->publish(ptz_camera_res_channels::end_session_res_channel,
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
	this->lcm->publish(ptz_camera_res_channels::discovery_res_channel, &discovery_response);
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
	lcm->publish(ptz_camera_res_channels::stream_res_channel, &stream_uri_response);
	std::cout << lcm_handler::ok_message;
}


void lcm_handler::on_ptz_conrol_req(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::ptz_control_request_t* req)
{
	std::cout << "Received ptz_control_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto control_response = std::make_shared
		<ptz_camera::ptz_control_response_t>();

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: "
			+ req->ip_address + "... request dropped!";
		cout << message << endl;
		control_response->status_code = ptz_camera::status_codes_t::ERR;
		control_response->response_message = message;
		std::cout << "Sending ptz_control response... ";
		this->lcm->publish(ptz_camera_res_channels::ptz_control_res_channel,
			control_response.get());
		std::cout << lcm_handler::ok_message;
	}
	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		auto ptz_control_response_task = this->send_ptz_control_request(
			ip_client_pair->second, req).
			then([this](ptz_camera::ptz_control_response_t control_response)
			{
				std::cout << "Sending ptz_control response... ";
				this->lcm->publish(ptz_camera_res_channels::ptz_control_res_channel,
					&control_response);
				std::cout << lcm_handler::ok_message;
			});

	}
}

task<ptz_camera::ptz_control_response_t> 
	lcm_handler::send_ptz_control_request(http_client* client, 
		const ptz_camera::ptz_control_request_t* req)
{
	auto control_response = make_shared<ptz_camera::ptz_control_response_t>();
	control_response->ip_address = req->ip_address;	

	std::cout << "Building request... ";

	auto camera_pan = convert_to_wstring(req->pan_value);
	auto camera_tilt = convert_to_wstring(req->tilt_value);
	auto camera_zoom = convert_to_wstring(req->zoom_value);

	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	if (camera_pan == L"" && camera_tilt == L"" && camera_zoom == L"")
		throw camera_request_exception("Invalid ptz value"); 

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
	auto control_request_task = client->request(methods::GET, request)
		.then([control_response, this](
			pplx::task<http_response> request_task) -> pplx::task<wstring>
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
					std::cout << "GET request to server failed! HTTP Error: "
						<< response.status_code() << std::endl;
					control_response->status_code =
						ptz_camera::status_codes_t::ERR;
				}

				return response.extract_string();
			}

			catch (const http_exception& e)
			{
				cout << "Caught exception: " << e.what() << endl;
				return create_task([e, this]() -> wstring
				{
					return this->convert_to_wstring(e.what());
				});
			}

		})
		.then([control_response, this](wstring response) -> 
			ptz_camera::ptz_control_response_t
		{
			control_response->response_message =
				this->convert_to_string(response);	
			return *(control_response.get());
		});

		return control_request_task;	
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
		this->lcm->publish(ptz_camera_res_channels::stop_ptz_control_res_channel,
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
					std::cout << "GET request to server failed! HTTP Error: "
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
			this->lcm->publish(ptz_camera_res_channels::stop_ptz_control_res_channel,
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
		this->lcm->publish(ptz_camera_res_channels::position_res_channel,
			position_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		auto position_task = get_camera_position(ip_client_pair->second);
		auto response = position_task.get();
		response.ip_address = req->ip_address;

		this->lcm->publish(ptz_camera_res_channels::position_res_channel, &response);
	}
}

pplx::task<ptz_camera::position_response_t> lcm_handler::
	get_camera_position(http_client* client)
{
	auto position_response = make_shared<ptz_camera::position_response_t>();
	
	std::cout << "Building request... ";
	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
		append_path(uri_constants::ptz_control_cgi);

	uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_query).
		append_query(uri_constants::action, uri_constants::action_view).
		append_query(uri_constants::channel, L"0").
		append_query(uri_constants::query, L"Pan,Tilt,Zoom");


	auto request = uri.to_string();
	std::cout << messages::ok_message;
	std::cout << "Request: " << convert_to_string(request) << std::endl;

	std::cout << "Sending position request... ";
	return client->request(methods::GET, request)
		.then(
			[position_response, this](
				pplx::task<http_response> request_task) -> pplx::task<wstring>
	{
		try
		{
			auto response = request_task.get();
			if (response.status_code() == status_codes::OK)
			{
				std::cout << messages::ok_message;
				position_response->status_code =
					ptz_camera::status_codes_t::OK;
			}

			else
			{
				std::cout << "GET request to server failed! HTTP Error: "
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
	.then([position_response, this](wstring response) -> ptz_camera::position_response_t
	{
		sun_api::response_parser parser;
		auto response_map = parser.parse_string(convert_to_string(response));
		position_response->pan_value = response_map["Pan"];
		position_response->tilt_value = response_map["Tilt"];
		position_response->zoom_value = response_map["Zoom"];
		position_response->response_message = "OK";
		return *position_response.get();
	});
}


void lcm_handler::on_preset_config_request(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::preset_config_request_t* req)
{
	std::cout << "Received preset_config_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto config_response = make_shared<ptz_camera::preset_config_response_t>();
	config_response->ip_address = req->ip_address;
	config_response->preset_number = req->preset_number;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: "
			+ req->ip_address + "... request dropped!";
		cout << message << endl;
		config_response->status_code = ptz_camera::status_codes_t::ERR;
		config_response->response_message = message;
		std::cout << "Sending preset_config response... ";
		this->lcm->publish(ptz_camera_res_channels::preset_config_res_channel,
			config_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;		
		std::cout << "Building request... ";

		uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
			append_path(uri_constants::ptz_config_cgi);

		uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_preset);

		switch (req->mode)
		{
			case ptz_camera::preset_config_request_t::ADD:
			{
				uri.append_query(uri_constants::action, uri_constants::action_add);
				uri.append_query(uri_constants::preset, convert_to_wstring(req->preset_number));
				uri.append_query(uri_constants::name, convert_to_wstring(req->preset_name));
				break;
			}

			case ptz_camera::preset_config_request_t::UDPATE:
			{
				uri.append_query(uri_constants::action, uri_constants::action_update);
				uri.append_query(uri_constants::preset, convert_to_wstring(req->preset_number));
				uri.append_query(uri_constants::name, convert_to_wstring(req->preset_name));
				break;
			}

			case ptz_camera::preset_config_request_t::REMOVE:
			{
				uri.append_query(uri_constants::action, uri_constants::action_remove);
				uri.append_query(uri_constants::preset, convert_to_wstring(req->preset_number));
				break;
			}
		}

		auto request = uri.to_string();
		std::cout << lcm_handler::ok_message;
		std::cout << "Request: " << convert_to_string(request) << std::endl;

		std::cout << "Sending preset_config request... ";
		ip_client_pair->second->request(methods::GET, request)
		.then([config_response, this](
			pplx::task<http_response> request_task) -> pplx::task<wstring>
		{
			try
			{
				auto response = request_task.get();
				if (response.status_code() == status_codes::OK)
				{
					std::cout << lcm_handler::ok_message;
					config_response->status_code =
						ptz_camera::status_codes_t::OK;
				}

				else
				{
					std::cout << "GET request to server failed! HTTP Error: "
						<< response.status_code() << std::endl;
					config_response->status_code =
						ptz_camera::status_codes_t::ERR;
				}

				return response.extract_string();
			}

			catch (const exception& e)
			{
				cout << "Caught exception: " << e.what() << std::endl;
				return create_task([e, this]() -> wstring
				{
					return this->convert_to_wstring(e.what());
				});
			}

		})
		.then([config_response, this](wstring response)
		{
			config_response->
				response_message = convert_to_string(response);
			std::cout << "Sending preset_config response... ";
			this->lcm->publish(ptz_camera_res_channels::preset_config_res_channel,
				config_response.get());
			std::cout << lcm_handler::ok_message;
		});
	}
}

void lcm_handler::on_preset_move_request(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::preset_move_request_t* req)
{
	std::cout << "Received preset_move_req on channel: " << channel <<
		"; Camera: " << req->ip_address << std::endl;

	auto move_response = make_shared<ptz_camera::preset_move_response_t>();
	move_response->ip_address = req->ip_address;
	move_response->preset_number = req->preset_number;

	auto ip_client_pair = this->ip_client_map.find(req->ip_address);
	if (ip_client_pair == ip_client_map.end())
	{
		string message = "Session not initialized for ip: "
			+ req->ip_address + "... request dropped!";
		cout << message << endl;
		move_response->status_code = ptz_camera::status_codes_t::ERR;
		move_response->response_message = message;
		std::cout << "Sending preset_move response... ";
		this->lcm->publish(ptz_camera_res_channels::preset_move_res_channel,
			move_response.get());
		std::cout << lcm_handler::ok_message;
	}

	else
	{
		std::cout << "Client located for ip: " << req->ip_address << std::endl;
		std::cout << "Building request... ";

		uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
			append_path(uri_constants::ptz_control_cgi);

		uri.append_query(uri_constants::sub_menu, uri_constants::sub_menu_preset).
			append_query(uri_constants::preset, convert_to_wstring(req->preset_number));

		auto request = uri.to_string();
		std::cout << lcm_handler::ok_message;
		std::cout << "Request: " << convert_to_string(request) << std::endl;

		std::cout << "Sending preset_move request... ";
		ip_client_pair->second->request(methods::GET, request)
		.then(
				[move_response, this](
					pplx::task<http_response> request_task) -> pplx::task<wstring>
		{
			try
			{
				auto response = request_task.get();
				if (response.status_code() == status_codes::OK)
				{
					std::cout << lcm_handler::ok_message;
					move_response->status_code =
						ptz_camera::status_codes_t::OK;
				}

				else
				{
					std::cout << "GET request to server failed! HTTP Error: "
						<< response.status_code() << std::endl;
					move_response->status_code =
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
		.then([move_response, this](wstring response)
		{
			move_response->
				response_message = convert_to_string(response);
			std::cout << "Sending preset_move response... ";
			this->lcm->publish(ptz_camera_res_channels::preset_move_res_channel,
				move_response.get());
			std::cout << lcm_handler::ok_message;
		});
	}
}

void lcm_handler::on_start_program_request(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::start_program_request_t* req)
{
	std::cout << "Received start_program request on channel: " << channel
		 << std::endl;

	auto start_program_response = make_shared<ptz_camera::start_program_response_t>();
	
	if (this->program_is_executing)
	{
		start_program_response->status_code = ptz_camera::status_codes_t::ERR;
		auto error_message = "A program is already running, request dropped!";
		std::cout << error_message << std::endl;

		start_program_response->response_message = error_message;
		this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
		return;
	}


	// Reset cts
	auto cts = std::make_shared<cancellation_token_source>();
	this->program_cts = *cts;

	auto program_pairs = make_shared<std::queue<std::pair <std::string, std::vector<std::string>> > > ();
	auto program = make_shared<std::string>();

	*program.get() = req->program;

	auto program_task = create_task([start_program_response, program, program_pairs, this]()
	{
		try
		{
			std::cout << "Parsing program... ";
			*program_pairs = this->parse_program(*(program.get()));
			std:cout << lcm_handler::ok_message;
			start_program_response->status_code = ptz_camera::status_codes_t::OK;
			start_program_response->response_message = "OK";
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
		}
		
		catch (const program_parse_exception& e)
		{
			std::cout << e.what() << std::endl;
			start_program_response->status_code = ptz_camera::status_codes_t::ERR;
			start_program_response->response_message = e.what();
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
			return;
		}

		catch (const logic_error& e)
		{
			std::cout << e.what() << std::endl;
			start_program_response->status_code = ptz_camera::status_codes_t::ERR;
			start_program_response->response_message = e.what();
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
			return;
		}

		try
		{
			std::cout << "Executing program... \n";
			this->program_is_executing = true;
			this->execute_program(program_pairs.get()).wait();			
			this->program_is_executing = false;
		}

		catch (const program_execution_exception& e)
		{
			std::cout << e.what() << std::endl;
			this->program_is_executing = false;
			start_program_response->status_code = ptz_camera::status_codes_t::ERR;
			start_program_response->response_message = e.what();
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
			return;
		}	

		catch (const camera_request_exception& e)
		{
			std::cout << e.what() << std::endl;
			this->program_is_executing = false;
			start_program_response->status_code = ptz_camera::status_codes_t::ERR;
			start_program_response->response_message = e.what();
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
			return;
		}

		catch (const exception& e)
		{
			std::cout << e.what() << std::endl;
			this->program_is_executing = false;
			start_program_response->status_code = ptz_camera::status_codes_t::ERR;
			start_program_response->response_message = e.what();
			std::cout << "Sending start_program response... ";
			this->lcm->publish(ptz_camera_res_channels::start_program_res_channel, start_program_response.get());
			std::cout << lcm_handler::ok_message;
			return;
		}				

		std::cout << "Sending program end notification...";
		ptz_camera::output_request_t program_end_request;
		program_end_request.ip_address = "";
		this->lcm->publish(
			ptz_camera_req_channels::output_req_channel, &program_end_request);
		std::cout << lcm_handler::ok_message;
	});	
}

std::queue<std::pair <std::string, std::vector<string>> > 
lcm_handler::parse_program(std::string program_text)
{
	std::queue<std::pair <std::string, std::vector<std::string>> > program_pairs;
	std::istringstream is_program(program_text);

	std::string line;
	int line_num = 1;

	while (std::getline(is_program, line))
	{
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '='))
		{
			if ((key != "WAIT") && (key != "OUTPUT") && (key != "PRESET"))
				throw program_parse_exception(
					"Unrecognized token in program: \"" + key + "\" Line:" + std::to_string(line_num));

			std::string value;
			if (std::getline(is_line, value))
			{
				if (key == "WAIT")
				{
					std::vector<std::string> value_vector;
					value_vector.push_back(value);
					program_pairs.push(std::pair<std::string, std::vector<std::string> >("WAIT", value_vector));
				}

				else if (key == "OUTPUT")
				{
					std::vector<std::string> value_vector;
					value_vector.push_back(value);
					program_pairs.push(std::pair<std::string, std::vector<std::string> >("OUTPUT", value_vector));
				}

				else if (key == "PRESET")
				{
					std::vector<string> preset_vector;
					std::stringstream preset_stream(value);
					string value;

					while (getline(preset_stream, value, ','))
					{
						preset_vector.push_back(value);
					}

					if (preset_vector.size() != 4)
						throw program_parse_exception("Error parsing "
							"preset token in program: value was " + value + " Line: " + std::to_string(line_num));

					program_pairs.push(std::pair<std::string, std::vector<std::string> >("PRESET", preset_vector));
				}
			}

			else				
				throw program_parse_exception(
					"Missing token after: \"" + key + "\" Line: " + std::to_string(line_num));
		}

		else			
			throw program_parse_exception(
				"Unable to locate token '='. Line: " + std::to_string(line_num));

		line_num += 1;
		
	}

	return program_pairs;
}

pplx::task<void> lcm_handler::execute_program(std::queue<std::pair <std::string, std::vector<std::string> > >* program)
{
	auto program_task = create_task([this, program]()
	{
		while ((*program).size() > 0)
		{
			if (this->program_cts.get_token().is_canceled())
				cancel_current_task();

			auto current_pair = (*program).front();
			(*program).pop();

			if (current_pair.first == "WAIT")
				this->handle_wait_in_program(&(current_pair.second[0])).wait();

			if (current_pair.first == "PRESET")
				this->handle_preset_in_program(&current_pair.second).wait();

			if (current_pair.first == "OUTPUT")
				this->handle_output_in_program(&current_pair.second[0]);
		}
	}, this->program_cts.get_token());

	return program_task;
}


task<void> lcm_handler::handle_wait_in_program(std::string* wait_value)
{
	return create_task([wait_value]()
	{
		std::cout << "Waiting for: " << *wait_value << std::endl;
		try
		{
			std::this_thread::sleep_for(std::chrono::
				milliseconds(atoi((*wait_value).c_str())));
		}

		catch (const exception& e)
		{
			throw program_execution_exception(
				"Error executing wait in program: value was" + *wait_value);
		}
	}, this->program_cts.get_token());
}

task<void> lcm_handler::handle_preset_in_program(std::vector<std::string>* preset_value)
{
	return create_task([preset_value, this]()
	{
		string message = "Executing preset for camera: " + (*preset_value)[0] +
			" pan: " + (*preset_value)[1] + " tilt: " + (*preset_value)[2] +
			" zoom: " + (*preset_value)[3] + "\n";
		std::cout << message;


		auto ip_client_pair = this->ip_client_map.find((*preset_value)[0]);
		if (ip_client_pair == ip_client_map.end())
			throw program_execution_exception("Cannot execute preset. "
				"No session initialized for ip: " + (*preset_value)[0]);

		ptz_camera::ptz_control_request_t ptz_control_request;

		ptz_control_request.mode = ptz_camera::ptz_control_request_t::ABS;
		ptz_control_request.ip_address = (*preset_value)[0];
		ptz_control_request.pan_value = (*preset_value)[1];
		ptz_control_request.tilt_value = (*preset_value)[2];
		ptz_control_request.zoom_value = (*preset_value)[3];

		auto control_response = send_ptz_control_request(ip_client_pair->second, &ptz_control_request).get();

		if (control_response.status_code == ptz_camera::status_codes_t::ERR || 
			control_response.response_message != "OK")
			throw camera_request_exception(control_response.response_message);
	});
}

void lcm_handler::handle_output_in_program(string* value)
{
	std::cout << "Switching output to camera: " << *value << std::endl;

	auto output_request = make_shared<ptz_camera::output_request_t>();

	output_request->ip_address = *value;

	lcm->publish(ptz_camera_req_channels::output_req_channel, output_request.get());
}

void lcm_handler::handle_unrecognized_key_in_program(string* value)
{
	std::cout << "Unrecognized key in program: " << *value << std::endl;
}


void lcm_handler::on_stop_program_request(const lcm::ReceiveBuffer* rbuf,
	const std::string& channel,
	const ptz_camera::stop_program_request_t* req)
{
	std::cout << "Received stop_program request on channel: " << channel
		<< std::endl;	

	ptz_camera::stop_program_response_t stop_program_response;
	
	if (this->program_is_executing)
	{
		std::cout << "Stopping running program... ";
		this->program_cts.cancel();		
		std::cout << lcm_handler::ok_message;
		stop_program_response.status_code = ptz_camera::status_codes_t::OK;
		stop_program_response.response_message = "OK";		
	}

	else
	{
		stop_program_response.status_code = ptz_camera::status_codes_t::ERR;
		auto error_message = "No running program to stop\n";
		stop_program_response.response_message = error_message;
	}

	this->lcm->publish(ptz_camera_res_channels::stop_program_res_channel, &stop_program_response);
}