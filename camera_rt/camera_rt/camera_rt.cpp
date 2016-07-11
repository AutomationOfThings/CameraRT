// camera_rt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "lcm_handler.h"
#include "channel_names.h"



int _tmain(int argc, _TCHAR* argv[])
{	
	std::cout << "╔╗┌─┐┌┬┐┌─┐┬─┐┌─┐╦═╗╔╦╗" << std::endl;
	std::cout << "║  ├─┤│││├┤ ├┬┘├─┤╠╦╝ ║ " << std::endl;
	std::cout << "╚═╝┴ ┴┴ ┴└─┘┴└─┴ ┴╩╚═ ╩ " << std::endl;

	lcm::LCM lcm;

	if (!lcm.good())
	{
		std::cout << "Couldn't allocate lcm\n";
	}

	lcm_handler handler(&lcm);
	
	lcm.subscribe(ptz_camera_channels::discovery_req_channel, &lcm_handler::on_discovery_req, &handler);
	lcm.subscribe(ptz_camera_channels::init_session_req_channel, &lcm_handler::on_init_session_req, &handler);

	while (0 == lcm.handle());

	return 0;
}

