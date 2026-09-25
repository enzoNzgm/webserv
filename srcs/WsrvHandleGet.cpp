#include "WsrvStruct.hpp"
#include "WsrvRequest.hpp"
#include "WsrvResponse.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Helper function for C++98 compatibility
template <typename T>
std::string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

static std::string resolvePath(WsrvRequest &req, WsrvServer &config)
{
	std::string root = config.root;
	std::string uri = req.uri;

	if (uri.find("..") != std::string::npos)
		return "";

	if (!uri.empty() && uri[0] == '/')
		uri.erase(0, 1);

	std::string full_path;

	if (!root.empty() && root[root.size() - 1] == '/')
		full_path = root + uri;
	else
		full_path = root + "/" + uri;

	return full_path;
}

static bool fileExists(const std::string &path)
{
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

static bool isDirectory(const std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

static std::string findIndexFile(const std::string &directoryPath, WsrvServer &config)
{
	for (size_t i = 0; i < config.index.size(); i++)
	{
		std::string candidate = directoryPath;

		if (candidate[candidate.size() - 1] != '/')
			candidate += "/";

		candidate += config.index[i];

		if (fileExists(candidate))
			return candidate;
	}
	return "";
}

static std::string generateAutoindex(const std::string &directoryPath, const std::string &uri)
{
	std::string html;

	html += "<html><head><title>Index of ";
	html += uri;
	html += "</title></head><body>\n";
	html += "<h1>Index of ";
	html += uri;
	html += "</h1><hr><pre>\n";

	DIR *dir = opendir(directoryPath.c_str());
	if (!dir)
		return ""; // erreur interne → handleGET renverra 500

	struct dirent *entry;
	while ((entry = readdir(dir)))
	{
		std::string name = entry->d_name;

		if (name == "." || name == "..")
			continue;

		html += "<a href=\"";
		html += uri;

		if (!uri.empty() && uri[uri.size() - 1] != '/')
			html += "/";

		html += name;
		html += "\">";
		html += name;
		html += "</a>\n";
	}

	closedir(dir);
	html += "</pre><hr></body></html>";
	return html;
}

static std::string findErrorPage(int code, WsrvServer &config)
{
	std::string code_str = toString(code);

	// Chercher dans les pages d'erreur configurées
	for (size_t i = 0; i < config.error_page.size(); i++)
	{
		if (config.error_page[i].first == code_str)
		{
			std::string error_path = config.root + config.error_page[i].second;
			if (fileExists(error_path))
				return error_path;
		}
	}
	return "";
}

static WsrvResponse errorResponse(int code, WsrvServer &config)
{
	WsrvResponse res;
	res.status = code;

	// Essayer d'utiliser une page d'erreur personnalisée
	std::string error_page_path = findErrorPage(code, config);
	if (!error_page_path.empty())
	{
		res = serveFile(error_page_path);
		res.status = code; // Corriger le status (serveFile retourne toujours 200)
		return res;
	}

	// Sinon utiliser le HTML par défaut
	res.body = "<html><body><h1>Error " + toString(code) + "</h1></body></html>";
	res.headers["Content-Length"] = toString(res.body.size());
	res.headers["Content-Type"] = "text/html";
	return res;
}

WsrvResponse wsrv_handle_get(WsrvRequest &req, WsrvServer &config)
{
	std::string path = resolvePath(req, config);
	if (!fileExists(path) || path.empty())
		return errorResponse(404, config);

	if (isDirectory(path))
	{
		std::string indexFile = findIndexFile(path, config);
		if (!indexFile.empty())
			return serveFile(indexFile);
		if (config.autoindex == "on")
		{
			std::string html = generateAutoindex(path, req.uri);
			return serveHTML(html, 200);
		}
		return errorResponse(403, config);

	}

	return serveFile(path);
}
