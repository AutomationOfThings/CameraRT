#pragma once
#include <string>
namespace utility_functions
{
	std::wstring convert_to_wstring(std::string value)
	{
		return std::wstring(value.begin(), value.end());
	}

	std::string convert_to_string(std::wstring value)
	{
		return std::string(value.begin(), value.end());
	}
}