#pragma once
#ifndef  __ptz_camera_channels__
#define  __ptz_camera_channels__

namespace ptz_camera_req_channels
{	
	extern const char* discovery_req_channel;
	extern const char* init_session_req_channel;
	extern const char* end_session_req_channel;
	extern const char* stream_req_channel;
	extern const char* ptz_control_req_channel;
	extern const char* position_req_channel;
	extern const char* stop_ptz_control_req_channel;
	extern const char* preset_config_req_channel;
	extern const char* preset_move_req_channel; 
	extern const char* preset_move_req_channel;
	extern const char* start_program_req_channel;
	extern const char* stop_program_req_channel;
	extern const char* output_req_channel;
};


namespace ptz_camera_res_channels
{
	extern const char* discovery_res_channel;
	extern const char* init_session_res_channel;
	extern const char* end_session_res_channel;
	extern const char* stream_res_channel;
	extern const char* ptz_control_res_channel;
	extern const char* position_res_channel;
	extern const char* stop_ptz_control_res_channel;
	extern const char* preset_config_res_channel;
	extern const char* preset_move_res_channel;
	extern const char* start_program_res_channel;
	extern const char* stop_program_res_channel;
};

namespace ptz_camera_message_channels
{
	extern const char* program_status_mes_channel;
	extern const char* end_program_mes_channel;
}

#endif // ! __ptz_camera_channels__
