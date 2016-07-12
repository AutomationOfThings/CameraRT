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

private:
	lcm::LCM* lcm;
	std::unordered_map <std::string, http_client*> ip_client_map;

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


};

	

