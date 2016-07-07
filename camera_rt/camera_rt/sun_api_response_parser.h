#pragma once

#include <map>
#include <string>

namespace sun_api
{
	class response_parser
	{
	public:
		std::map<std::string, std::string> parse_string(std::string response);
	};

}