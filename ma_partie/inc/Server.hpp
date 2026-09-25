#include <string>
#include <vector>
#include <utility>
#include <map>

typedef std::vector<std::string>                            wsrv_vec_str_t;
typedef std::vector<std::pair<std::string, std::string> >    wsrv_vec_pair_str_t;

struct WsrvServer
{
    wsrv_vec_str_t                listen;
    std::string                    server_name;
    std::string                    root;
    wsrv_vec_str_t                index;
    std::string                    autoindex;
    wsrv_vec_pair_str_t            error_page;
    std::string                    client_max_body_size;
    // std::vector<WsrvLocation>    location;
	std::map<std::string, std::string> cgi;
	std::string                    cgi_path;      // Path to upload CGI script
	std::string                    delete_cgi;    // Path to delete CGI script
};
