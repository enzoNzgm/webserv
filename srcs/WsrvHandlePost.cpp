#include "WsrvRequest.hpp"
#include "WsrvStruct.hpp"
#include "WsrvCGI.hpp"
#include "WsrvResponse.hpp"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstdlib>

// Helper function for C++98 compatibility
template <typename T>
std::string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

static std::string urlDecode(const std::string &str) {
	std::string decoded;
	char hex[3] = {0};
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%') {
			hex[0] = str[i + 1];
			hex[1] = str[i + 2];
			decoded += static_cast<char>(strtol(hex, NULL, 16));
			i += 2;
		} else if (str[i] == '+') {
			decoded += ' ';
		} else {
			decoded += str[i];
		}
	}

	return decoded;
}

static std::map<std::string, std::string> parseForm(const std::string &body) {
	std::map<std::string, std::string> result;

	std::stringstream ss(body);
	std::string pair;
	while (std::getline(ss, pair, '&')) {
		size_t pos = pair.find('=');
		if (pos != std::string::npos) {
			std::string key = urlDecode(pair.substr(0, pos));
			std::string value = urlDecode(pair.substr(pos + 1));
			result[key] = value;
		}
	}
	return result;
}

static std::string buildHttpResponse(int status, const std::string& body, const std::string& contentType)
{
	std::stringstream response;
	response << "HTTP/1.1 " << status;

	if (status == 200) response << " OK\r\n";
	else if (status == 400) response << " Bad Request\r\n";
	else if (status == 404) response << " Not Found\r\n";
	else if (status == 500) response << " Internal Server Error\r\n";
	else response << "\r\n";

	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "\r\n";
	response << body;

	return response.str();
}

static std::string handlePOST_form(WsrvRequest &req, WsrvServer &config)
{
	std::string contentType = req.headers["Content-Type"];
	if (contentType.find("application/x-www-form-urlencoded") == std::string::npos)
		return buildHttpResponse(400, "", "text/html");

	std::map<std::string, std::string> fields = parseForm(req.body);

	if (fields.count("name") == 0 || fields.count("email") == 0 || fields.count("message") == 0)
		return buildHttpResponse(400, "", "text/html");

	std::string log_path = config.root + "/forms.log";

	std::ofstream file(log_path.c_str(), std::ios::app);
	if (!file)
		return buildHttpResponse(500, "", "text/html");

	std::time_t t = std::time(0);
	std::tm* tm = std::localtime(&t);
	char timeBuffer[80];
	std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tm);

	file << "--- Nouveau formulaire ---\n";
	file << "Nom: " << fields["name"] << "\n";
	file << "Email: " << fields["email"] << "\n";
	file << "Message: " << fields["message"] << "\n";
	file << "Date: " << timeBuffer << "\n";
	file << "--------------------------\n\n";

	file.close();

	std::string body_resp = "<h1>Formulaire reçu ! Merci.</h1>";
	return buildHttpResponse(200, body_resp, "text/html");
}

static std::string handlePOST_upload(WsrvRequest &req, WsrvServer &config)
{
	WsrvCGI cgi;

	std::map<std::string,std::string> env;
	env["REQUEST_METHOD"] = "POST";
	env["CONTENT_TYPE"] = req.headers["Content-Type"];
	env["CONTENT_LENGTH"] = toString(req.body.size());
	env["SCRIPT_FILENAME"] = config.cgi_path;

	std::string output = cgi.run(config.cgi_path, req.body, env);

	return "HTTP/1.1 200 OK\r\n" + output;
}

static std::string handlePOST_delete(WsrvRequest &req, WsrvServer &config)
{
	WsrvCGI cgi;

	std::map<std::string,std::string> env;
	env["REQUEST_METHOD"] = "POST";
	env["CONTENT_TYPE"] = req.headers["Content-Type"];
	env["CONTENT_LENGTH"] = toString(req.body.size());
	env["SCRIPT_FILENAME"] = config.delete_cgi;

	std::string output = cgi.run(config.delete_cgi, req.body, env);

	return "HTTP/1.1 200 OK\r\n" + output;
}

std::string wsrv_handle_post(WsrvRequest &req, WsrvServer &config)
{
	// 1) Formulaire simple
	if (req.uri == "/contact" || req.uri == "/contact.html")
		return handlePOST_form(req, config);

	// 2) Upload CGI
	if (req.uri == "/upload")
		return handlePOST_upload(req, config);

	// 3) Delete CGI
	if (req.uri == "/delete")
		return handlePOST_delete(req, config);

	return buildHttpResponse(404, "", "text/html");
}
