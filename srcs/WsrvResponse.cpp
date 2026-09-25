#include "WsrvResponse.hpp"
#include <sstream>
#include <sys/stat.h>
#include <fstream>

// Helper function for C++98 compatibility
template <typename T>
std::string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

static std::string readFile(const std::string &path)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return "";

	std::string content;

	file.seekg(0, std::ios::end);
	content.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&content[0], content.size());
	file.close();

	return content;
}

static std::string getMimeType(const std::string &path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot);

	if (ext == ".html") return "text/html";
	if (ext == ".htm") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".png") return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif") return "image/gif";
	if (ext == ".svg") return "image/svg+xml";
	if (ext == ".txt") return "text/plain";

	return "application/octet-stream";
}

WsrvResponse serveFile(const std::string &path)
{
	WsrvResponse res;

	res.status = 200;
	res.body = readFile(path);
	res.headers["Content-Length"] = toString(res.body.size());
	res.headers["Content-Type"] = getMimeType(path);

	return res;
}

WsrvResponse serveHTML(const std::string &html, int status)
{
	WsrvResponse res;

	res.status = status;
	res.body = html;
	res.headers["Content-Length"] = toString(html.size());
	res.headers["Content-Type"] = "text/html";

	return res;
}
