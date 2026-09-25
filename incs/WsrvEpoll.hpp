#ifndef EPOLL_HPP
# define EPOLL_HPP

# include <sys/epoll.h>
# include <map>
# include "WsrvCore.hpp"
# include "WsrvRequest.hpp"

# define MAX_EVENTS	1024
# define TIME_OUT	5000

struct WsrvListenSocket;

struct WsrvClientSocket
{
	int						fd;
	std::string				in_buff;
	std::string				out_buff;
	const std::vector<int>&	server_ids;
	WsrvRequest				request;

	WsrvClientSocket(int client_fd, const std::vector<int>& server_ids_) :
		fd(client_fd), server_ids(server_ids_) {};
};

struct WsrvEpoll
{
	int								fd;
	int								ev_i;
	struct epoll_event				events[MAX_EVENTS];
	std::vector<WsrvListenSocket>	lstnSock;
	std::map<int, WsrvClientSocket>	cliSock;
	const WsrvHttp&					http;

	WsrvEpoll(const WsrvHttp& http_) : ev_i(0), http(http_) {};
};

bool	wsrv_epoll(WsrvEpoll& epoll);
void	wsrv_epoll_close(WsrvEpoll& epoll);


#endif
