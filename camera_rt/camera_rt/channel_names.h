#pragma once
#ifndef  __ptz_camera_channels__
#define  __ptz_camera_channels__

namespace ptz_camera_channels
{
	
	extern const char* discovery_req_channel;
	extern const char* discovery_res_channel;
	extern const char* init_session_req_channel;
	extern const char* end_session_req_channel;
	extern const char* end_session_res_chanel;
	extern const char* stream_res_channel;
	extern const char* stream_req_channel;
	extern const char* ptz_control_req_channel;
	extern const char* ptz_control_res_channel;
	extern const char* position_req_channel;
	extern const char* position_res_channel;
}

#endif // ! __ptz_camera_channels__
