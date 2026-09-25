#ifndef WSRV_RESPONSE_HPP
# define WSRV_RESPONSE_HPP

# include <string>
# include <map>

struct WsrvResponse
{
	int status;
	std::map<std::string, std::string> headers;
	std::string body;

	WsrvResponse() : status(200) {}
};

WsrvResponse serveFile(const std::string &path);
WsrvResponse serveHTML(const std::string &html, int status);

#endif
