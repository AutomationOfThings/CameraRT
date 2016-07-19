#pragma once
#include <lcm\lcm-cpp.hpp>
#include "Types\discovery_request_t.hpp"
#include "Types\discovery_response_t.hpp"
#include "Types\init_session_request_t.hpp"
#include "Types\end_session_request_t.hpp"
#include "Types\stream_uri_request_t.hpp"
#include "Types\ptz_control_request_t.hpp"
#include "Types\position_request_t.hpp"
#include "Types\stop_ptz_control_request_t.hpp"
#include "Types\preset_config_request_t.hpp"
#include "Types\preset_move_request_t.hpp"
#include "Types\start_program_request_t.hpp"
#include "Types\stop_program_request_t.hpp"
#include "Types\position_response_t.hpp"

#include <cpprest\http_client.h>
#include <cpprest\filestream.h>
#include <cpprest\uri.h>

#include <unordered_map>
#include <string>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

class lcm_handler
{

public:	
	lcm_handler(lcm::LCM* lcm) 
	{ 
		this->lcm = lcm;		
	}

	~lcm_handler() {}

	void on_init_session_req(const lcm::ReceiveBuffer* rbuff,
		const std::string& channel,
		const ptz_camera::init_session_request_t* req);

	void on_end_session_req(const lcm::ReceiveBuffer* rbuff,
		const std::string& channel,
		const ptz_camera::end_session_request_t* req);


	void on_discovery_req(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::discovery_request_t* req);


	void on_stream_uri_req(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::stream_uri_request_t* req);


	void on_ptz_conrol_req(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::ptz_control_request_t* req);

	void on_stop_ptz_control_req(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::stop_ptz_control_request_t* req);

	void on_position_req(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::position_request_t* req);

	void on_preset_config_request(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::preset_config_request_t* req);

	void on_preset_move_request(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::preset_move_request_t* req);

	void on_start_program_request(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::start_program_request_t* req);

	void on_stop_program_request(const lcm::ReceiveBuffer* rbuf,
		const std::string& channel,
		const ptz_camera::stop_program_request_t* req);	


private:
	lcm::LCM* lcm;
	Concurrency::cancellation_token_source program_cts;
	bool program_is_executing;	

	std::unordered_map <std::string, http_client*> ip_client_map;

	void lcm_handler::send_ptz_control_request
		(ptz_camera::ptz_control_request_t req);

	pplx::task<ptz_camera::position_response_t> get_camera_position
		(http_client* client);

	std::string const ok_message = "OK\n";

	lcm::Subscription *uri_req_sub, *ptz_control_req_sub, 
		*stop_ptz_control_req_sub,
		*discovery_req_sub, *position_req_sub;

	std::wstring convert_to_wstring(std::string value)
	{
		return std::wstring(value.begin(), value.end());
	}

	std::string convert_to_string(std::wstring value)
	{
		return std::string(value.begin(), value.end());
	}

	void handle_wait_in_program(std::string value);

	void handle_preset_in_program(std::string value);

	void handle_output_in_program(std::string value);

	void handle_unrecognized_key_in_program(std::string value);

	std::queue<std::pair <std::string, std::string> > parse_program(std::string program_text);

	pplx::task<void> execute_program(std::queue<std::pair <std::string, std::string> >* program);
};

	

