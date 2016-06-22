#pragma once
#ifndef  __ptz_camera_channels__
#define __ptz_camera_channels__

namespace ptz_camera_channels
{
	const char* discovery_req_channel = "DISCOVERYREQ";
	const char* stream_req_channel = "STREAMURIREQ";
	const char* stream_res_channel = "STREAMURIRES";
	const char* ptz_control_req_channel = "PTZCONTROLREQ";
	const char* ptz_control_res_channel = "PTZCONTROLRES";
}

#endif // ! __ptz_camera_channels__
