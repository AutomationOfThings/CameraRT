#pragma once
#include "Types\ptz_control_request_t.hpp"
#include "Types\stop_ptz_control_request_t.hpp"
#include "Types\position_request_t.hpp"

#include <cpprest\http_client.h>
#include <cpprest\filestream.h>
#include <cpprest\uri.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

class ptz_cgi_request_response_handler
{
	void handle_abs_ptz_control_request(ptz_camera::ptz_control_request_t ptz_control_request, http_client* client);
	void handle_rel_ptz_control_request(ptz_camera::ptz_control_request_t ptz_control_request, http_client* client);
	void handle_con_ptz_control_request(ptz_camera::ptz_control_request_t ptz_control_request, http_client* client);
	void handle_stop_ptz_control_request(ptz_camera::stop_ptz_control_request_t ptz_control_request, http_client* client);
	void handle_ptz_position_request(ptz_camera::position_request_t position_request, http_client* client);
};