#include "WsrvHandler.hpp"
#include <sstream>

std::string wsrv_response_to_string(const WsrvResponse &res)
{
	std::stringstream ss;

	// Status line
	ss << "HTTP/1.1 " << res.status;
	if (res.status == 200) ss << " OK\r\n";
	else if (res.status == 400) ss << " Bad Request\r\n";
	else if (res.status == 403) ss << " Forbidden\r\n";
	else if (res.status == 404) ss << " Not Found\r\n";
	else if (res.status == 500) ss << " Internal Server Error\r\n";
	else ss << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = res.headers.begin();
		 it != res.headers.end(); ++it)
	{
		ss << it->first << ": " << it->second << "\r\n";
	}

	// Empty line + body
	ss << "\r\n";
	ss << res.body;

	return ss.str();
}
