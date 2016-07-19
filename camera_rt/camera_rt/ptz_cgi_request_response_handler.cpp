#include "stdafx.h"
//#include "ptz_cgi_request_response_handler.h"
//#include "Types\ptz_control_response_t.hpp"
//#include "Types\status_codes_t.hpp"
//#include "utility_functions.h"
//#include "uri_constants.h"
//#include "lcm_handler.h"
//#include "messages.h"
//#include "channel_names.h"
//#include <ppltasks.h>
//
//using namespace utility_functions;
//
//void ptz_cgi_request_response_handler::
//	handle_abs_ptz_control_request(ptz_camera::ptz_control_request_t ptz_control_request, http_client* client)
//{
//	auto control_response = std::make_shared<ptz_camera::ptz_control_response_t>();
//	control_response->ip_address = ptz_control_request.ip_address;
//
//	std::cout << "Client located for ip: " << ptz_control_request.ip_address << std::endl;
//	std::cout << "Building request... ";
//
//	auto camera_pan = convert_to_wstring(ptz_control_request.pan_value);
//	auto camera_tilt = convert_to_wstring(ptz_control_request.tilt_value);
//	auto camera_zoom = convert_to_wstring(ptz_control_request.zoom_value);
//
//	uri_builder uri = uri_builder(U("/") + uri_constants::stw_cgi).
//		append_path(uri_constants::ptz_control_cgi);
//
//	if (camera_pan == L"" && camera_tilt == L"" && camera_zoom == L"")
//		return; //TODO send warning to client since none of ptz was provided
//	
//		uri.append_query(uri_constants::sub_menu,
//			uri_constants::sub_menu_absolute).append_query(
//				uri_constants::action, uri_constants::action_control);
//
//	if (camera_pan != L"")
//		uri.append_query(uri_constants::pan, camera_pan);
//
//	if (camera_tilt != L"")
//		uri.append_query(uri_constants::tilt, camera_tilt);
//
//	if (camera_zoom != L"")
//		uri.append_query(uri_constants::zoom, camera_zoom);
//
//	auto request = uri.to_string();
//	std::cout << messages::ok_message;
//
//	std::cout << "Request: " << convert_to_string(request) << std::endl;
//
//	std::cout << "Sending ptz_control request... ";
//	client->request(methods::GET, request)
//		.then([control_response, this](
//			pplx::task<http_response> request_task) -> pplx::task<wstring>
//	{
//		try
//		{
//			auto response = request_task.get();
//			if (response.status_code() == status_codes::OK)
//			{
//				std::cout << messages::ok_message;
//				control_response->status_code =
//					ptz_camera::status_codes_t::OK;
//			}
//
//			else
//			{
//				std::cout << "GET request to server failed! HTTP Error: "
//					<< response.status_code() << std::endl;
//				control_response->status_code =
//					ptz_camera::status_codes_t::ERR;
//			}
//
//			return response.extract_string();
//		}
//
//		catch (const exception& e)
//		{
//			cout << "Caught exception: " << e.what() << endl;
//			return concurrency::create_task([e, this]() -> wstring
//			{
//				return utility_functions::convert_to_wstring(e.what());
//			});
//		}
//
//	})
//		.then([control_response, this](wstring response)
//		{
//			control_response->response_message =
//				utility_functions::convert_to_string(response);
//
//			std::cout << "Sending ptz_control response... ";
//			auto lcm_p = lcm_handler::get_lcm_p()
//				lcm_p->publish(ptz_camera_res_channels::ptz_control_res_channel,
//				control_response.get());
//			std::cout << messages::ok_message;
//		});
//}
//
//void build_request_abs_control_request()
//{
//
//}
