#include <stdexcept>

using namespace std;

class program_execution_exception : public std::runtime_error 
{
	virtual const char* what() const throw()
	{
		return "Error executing program";
	}

};

class program_parse_exception : public std::runtime_error
{
	virtual const char* what() const throw()
	{
		return "Error parsing program";
	}

};