#include "../inc/Request.hpp"
#include "../inc/Server.hpp"
#include "../inc/CGI.hpp"
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

std::string urlDecode(const std::string &str) {
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

std::map<std::string, std::string> parseForm(const std::string &body) {
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

std::string handlePOST_form(Request &req, WsrvServer &config)
{
	if (req.headers["Content-Type"] != "application/x-www-form-urlencoded")
		return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";

	std::map<std::string, std::string> fields = parseForm(req.body);

	if (fields.count("name") == 0 || fields.count("email") == 0 || fields.count("message") == 0)
		return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";

	std::string log_path = config.root + "/forms.log";

	std::ofstream file(log_path.c_str(), std::ios::app);
	if (!file)
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length:  0\r\n\r\n";

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

	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body_resp.size() << "\r\n";
	response << "\r\n";
	response << body_resp;

	return response.str();
}

std::string handlePOST_upload(Request &req, WsrvServer &config)
{
	CGI cgi;

	std::map<std::string,std::string> env;
	env["REQUEST_METHOD"] = "POST";
	env["CONTENT_TYPE"] = req.headers["Content-Type"];
	env["CONTENT_LENGTH"] = toString(req.body.size());
	env["SCRIPT_FILENAME"] = config.cgi_path;  // exemple: "./cgi-bin/upload.py"

	std::string output = cgi.run(config.cgi_path, req.body, env);

	return "HTTP/1.1 200 OK\r\n" + output;
}

std::string handlePOST_delete(Request &req, WsrvServer &config)
{
	CGI cgi;

	std::map<std::string,std::string> env;
	env["REQUEST_METHOD"] = "POST";
	env["CONTENT_TYPE"] = req.headers["Content-Type"];
	env["CONTENT_LENGTH"] = toString(req.body.size());
	env["SCRIPT_FILENAME"] = config.delete_cgi; // "./cgi-bin/delete.py"

	std::string output = cgi.run(config.delete_cgi, req.body, env);

	return "HTTP/1.1 200 OK\r\n" + output;
}

std::string handlePOST(Request &req, WsrvServer &config)
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

	return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
}


// int main()
// {
//     WsrvServer config;
//     config.root = "./website";
//     Request req;

//     req.method = "POST";
//     req.uri = "/form";
//     req.http_version = "HTTP/1.1";

//     req.headers["Content-Type"] = "application/x-www-form-urlencoded";
//     req.headers["Content-Length"] = "52";

//     req.body = "name=Mon+ami&email=monami%40gmail.com&message=Salut+frerot";

//     std::string response = handlePOST_form(req, config);

//     std::cout << "=== RESPONSE ===" << std::endl;
//     std::cout << response << std::endl;

//     return 0;
// }
