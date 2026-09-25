#ifndef WSRV_REQUEST_HPP
# define WSRV_REQUEST_HPP

# include <string>
# include <map>

struct WsrvRequest
{
	std::string method;
	std::string uri;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;
	bool is_complete;
	int error_code;

	WsrvRequest() : is_complete(false), error_code(0) {}
};

#endif
