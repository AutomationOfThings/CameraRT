#pragma once

#ifndef __PtzCamera_Constants__
#define __PtzCamera_Constants__

#include <string>
using namespace std;

namespace uri_constants
{
	const wstring stw_cgi = L"stw-cgi";
	const wstring video_cgi = L"video.cgi";
	const wstring ptz_control_cgi = L"ptzcontrol.cgi";
	const wstring sub_menu = L"msubmenu";

	const wstring action = L"action";
	const wstring action_control = L"control";
	const wstring action_view = L"view";

	const wstring sub_menu_stream = L"stream";
	const wstring sub_menu_query = L"query";
	const wstring sub_menu_relative = L"relative";
	const wstring sub_menu_absolute = L"absolute";

	const wstring profile = L"Profile";
	const wstring codec_type = L"CodecType";
	const wstring resolution = L"Resolution";
	const wstring frame_rate = L"FrameRate";
	const wstring compression_level = L"CompressionLevel";
	const wstring channel = L"Channel";
	const wstring query = L"Query";
	
	const wstring pan = L"Pan";
	const wstring tilt = L"Tilt";
	const wstring zoom = L"Zoom";
}

#endif // !__PtzCamera_Constants__
