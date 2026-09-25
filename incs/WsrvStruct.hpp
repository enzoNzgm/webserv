#ifndef STRUCT_HPP
# define STRUCT_HPP

# include <vector>
# include <string>
# include <map>

typedef std::vector<std::string>			wsrv_vec_str_t;
typedef std::pair<std::string, std::string>	wsrv_pair_str_t;
typedef std::vector<wsrv_pair_str_t >		wsrv_vec_pair_str_t;

struct WsrvHttp;
struct WsrvServer;
struct WsrvLocation;

// struct WsrvLimitExcept
// {
// 	wsrv_vec_str_t	method; // GET, POST or DELETE
// 	wsrv_vec_pair_str_t			right; // allow or deny
// };

struct WsrvLocation
{
	WsrvServer&			server;
	std::string			uri;
	std::string			autoindex;
	std::string			client_max_body_size;
	wsrv_vec_pair_str_t	error_page;
	wsrv_vec_str_t		limit_except;

	WsrvLocation(WsrvServer& server_) : server(server_),
		autoindex("off"), client_max_body_size("1m") {};
};

struct WsrvServer
{
	WsrvHttp&					http;
	wsrv_vec_pair_str_t			listen;
	wsrv_vec_str_t				server_name;
	std::string					root;
	wsrv_vec_str_t				index;
	std::string					autoindex;
	wsrv_vec_pair_str_t			error_page;
	std::string					client_max_body_size;
	std::vector<WsrvLocation>	location;
	std::map<std::string, std::string> cgi;
	std::string					cgi_path;
	std::string					delete_cgi;

	WsrvServer(WsrvHttp& http_) : http(http_), root(""), autoindex("off"),
		client_max_body_size("1m"), cgi_path("./cgi-bin/upload.py"),
		delete_cgi("./cgi-bin/delete.py") {}
};

struct WsrvHttp
{
	std::string			root;
	std::string			client_max_body_size;
	wsrv_vec_pair_str_t	error_page;

	std::vector<WsrvServer>	server;
};

#endif

