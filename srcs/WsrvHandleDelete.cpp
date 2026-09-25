#include "WsrvRequest.hpp"
#include "WsrvStruct.hpp"
#include "WsrvResponse.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

template <typename T>
std::string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

static bool fileExists(const std::string &path)
{
	struct stat info;
	return (stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

static bool isSafePath(const std::string &path, const std::string &root)
{
	// Interdire les path traversal (..)
	if (path.find("..") != std::string::npos)
		return false;

	// Le chemin doit commencer par le root
	if (path.find(root) != 0)
		return false;

	return true;
}

static std::string resolvePath(const std::string &uri, const std::string &root)
{
	std::string path = uri;

	// Supprimer le / initial
	if (!path.empty() && path[0] == '/')
		path.erase(0, 1);

	// Construire le chemin complet
	std::string full_path;
	if (!root.empty() && root[root.size() - 1] == '/')
		full_path = root + path;
	else
		full_path = root + "/" + path;

	return full_path;
}

WsrvResponse wsrv_handle_delete(WsrvRequest &req, WsrvServer &config)
{
	WsrvResponse res;

	// Résoudre le chemin du fichier à supprimer
	std::string file_path = resolvePath(req.uri, config.root);

	// Vérifier la sécurité du chemin
	if (!isSafePath(file_path, config.root))
	{
		res.status = 403;
		res.body = "<html><body><h1>403 Forbidden</h1><p>Access denied.</p></body></html>";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString(res.body.size());
		return res;
	}

	// Vérifier que le fichier existe
	if (!fileExists(file_path))
	{
		res.status = 404;
		res.body = "<html><body><h1>404 Not Found</h1><p>File does not exist.</p></body></html>";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString(res.body.size());
		return res;
	}

	// Supprimer le fichier
	if (unlink(file_path.c_str()) == 0)
	{
		res.status = 200;
		res.body = "<html><body><h1>200 OK</h1><p>File deleted successfully.</p></body></html>";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString(res.body.size());
	}
	else
	{
		res.status = 500;
		res.body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to delete file.</p></body></html>";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString(res.body.size());
	}

	return res;
}
