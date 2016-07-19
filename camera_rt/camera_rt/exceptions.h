#include "stdafx.h"
#include <stdexcept>

using namespace std;

class program_execution_exception : public std::runtime_error 
{
public:
	program_execution_exception(const std::string &what)
		: std::runtime_error(what){}
};

class program_parse_exception : public std::runtime_error
{
public:
	program_parse_exception(const std::string &what)
		: std::runtime_error(what) {}
};

