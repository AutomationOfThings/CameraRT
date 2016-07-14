// camera_rt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "lcm_handler.h"
#include "channel_names.h"



int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << " _____                       _____ _____ \n";
	std::cout << "|     |___ _____ ___ ___ ___| __  |_   _|\n";
	std::cout << "|   --| .'|     | -_|  _| .'|    -| | |  \n";
	std::cout << "|_____|__,|_|_|_|___|_| |__,|__|__| |_|  \n\n";	


	lcm::LCM lcm;

	std::cout << "Initializing lcm... ";

	if (!lcm.good())
	{
		std::cout << "Couldn't allocate lcm\n";
	}

	std::cout << "OK\n";

	lcm_handler handler(&lcm);
	
	std::cout << "Subscribing for requests... ";
	lcm.subscribe(
		ptz_camera_req_channels::discovery_req_channel, 
		&lcm_handler::on_discovery_req, &handler);

	lcm.subscribe(ptz_camera_req_channels::init_session_req_channel, 
		&lcm_handler::on_init_session_req, &handler);

	lcm.subscribe(
		ptz_camera_req_channels::stream_req_channel,
		&lcm_handler::on_stream_uri_req, &handler);

	lcm.subscribe(
			ptz_camera_req_channels::ptz_control_req_channel,
			&lcm_handler::on_ptz_conrol_req, &handler);

	lcm.subscribe(
		ptz_camera_req_channels::stop_ptz_control_req_channel,
		&lcm_handler::on_stop_ptz_control_req, &handler);

	lcm.subscribe(
		ptz_camera_req_channels::position_req_channel,
		&lcm_handler::on_position_req, &handler);

	lcm.subscribe(ptz_camera_req_channels::end_session_req_channel,
		&lcm_handler::on_end_session_req, &handler);

	lcm.subscribe(ptz_camera_req_channels::preset_config_req_channel,
		&lcm_handler::on_preset_config_request, &handler);

	lcm.subscribe(ptz_camera_req_channels::preset_move_req_channel,
		&lcm_handler::on_preset_move_request, &handler);
	
	std::cout << "OK\n";

	while (0 == lcm.handle());

	return 0;
}

