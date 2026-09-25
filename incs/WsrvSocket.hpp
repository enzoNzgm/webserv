#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <arpa/inet.h>
# include <sys/socket.h>
# include <fcntl.h>
# include <netdb.h>
# include <set>
# include <unistd.h>
# include "WsrvCore.hpp"

struct WsrvListenSocket {
	int					fd;
	std::string			ip;
	std::string			port;
	sockaddr_storage	addr;
	socklen_t			addrlen;
	std::vector<int>	server_ids;

	WsrvListenSocket(const std::string& ip_, const std::string& port_): fd(-1),
		ip(ip_), port(port_), addrlen(sizeof(addr))
	{
		std::memset(&addr, 0, sizeof(addr));
	}

	WsrvListenSocket(const wsrv_pair_str_t& listen): fd(-1),
		ip(listen.first), port(listen.second), addrlen(sizeof(addr))
	{
		std::memset(&addr, 0, sizeof(addr));
	}

	WsrvListenSocket() : fd(-1), ip(""), port(""), addrlen(sizeof(addr))
	{
		std::memset(&addr, 0, sizeof(addr));
	}
};

void	wsrv_listen_socket_filler(std::vector<WsrvListenSocket>& vecLstnSock,
			const WsrvHttp& http);
bool	wsrv_socket_bind_listen(std::vector<WsrvListenSocket>& vecLstnSock);

#endif
