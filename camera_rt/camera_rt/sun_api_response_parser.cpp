#include "stdafx.h"
#include "sun_api_response_parser.h"
#include <iostream>
#include <sstream>

namespace sun_api
{
	std::map <std::string, std::string> response_parser::parse_string(std::string response)
	{
		std::map <std::string, std::string> map;
		std::istringstream is_response (response);

		std::string line;
		while (std::getline(is_response, line))
		{
			std::istringstream is_line(line);
			std::string key;
			if (std::getline(is_line, key, '='))
			{
				std::string value;
				if (std::getline(is_line, value))
					map[key] = value;
			}
		}
		return map;
	}
}