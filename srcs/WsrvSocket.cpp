#include "WsrvSocket.hpp"

static int wsrv_check_seen_ip_port_server(const wsrv_pair_str_t& listen, const int server_i)
{
	static wsrv_vec_pair_str_t				seen_ip_port;
	static std::vector<std::vector<int> >	seen_server;

	for (size_t i = 0; i < seen_ip_port.size(); i++)
	{
		if (listen.first == seen_ip_port[i].first &&
			listen.second == seen_ip_port[i].second)
		{
			// Check if this server index is already listed
			for (size_t j = 0; j < seen_server[i].size(); j++)
			{
				if (server_i == seen_server[i][j])
					return 2;
			}
			// Add new server index
			seen_server[i].push_back(server_i);
			return 1;
		}
	}
	seen_ip_port.push_back(listen); // New ip::port pair
	seen_server.push_back(std::vector<int>(1, server_i)); // Add server index

	return 0;
}


static int	wsrv_find_listen_socket(const std::vector<WsrvListenSocket>& vecLstnSock,
				const wsrv_pair_str_t& listen)
{
	for (size_t i = 0; i < vecLstnSock.size(); i++)
	{
		if ((listen.first == vecLstnSock[i].ip)
			&& (listen.second == vecLstnSock[i].port))
			return i;
	}
	return -1;
}

void	wsrv_listen_socket_filler(std::vector<WsrvListenSocket>& vecLstnSock,
			const WsrvHttp& http)
{
	int	status, ind;

	for (size_t i = 0; i < http.server.size(); i++)
	{
		const WsrvServer&	server = http.server[i];
		for (size_t j = 0; j < server.listen.size(); j++)
		{
			wsrv_pair_str_t	listen = server.listen[j];
			status = wsrv_check_seen_ip_port_server(listen, i);
			if (status == 1)
			{
				ind = wsrv_find_listen_socket(vecLstnSock, listen);
				vecLstnSock[ind].server_ids.push_back(i);
			}
			if (status == 0)
			{
				vecLstnSock.push_back(WsrvListenSocket(listen));
				vecLstnSock.back().server_ids.push_back(i);
			}
		}
	}
}

bool	wsrv_socket_bind_listen(std::vector<WsrvListenSocket>& vecLstnSock)
{
	for (size_t i = 0; i < vecLstnSock.size(); i++)
	{
		WsrvListenSocket& lstnSock = vecLstnSock[i];

		struct addrinfo hints, *res = NULL;

		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = ((lstnSock.ip.empty()) ? AI_PASSIVE : AI_NUMERICHOST);

		// hostname: NULL for wildcard binding
		const char* host = lstnSock.ip.empty() ? NULL : lstnSock.ip.c_str();

		int status = getaddrinfo(host, lstnSock.port.c_str(), &hints, &res);
		if (status != 0)
		{
			std::cerr << "Error (getaddrinfo): " << gai_strerror(status)
				<< " for " << (host ? host : "0.0.0.0") << ":"
				<< lstnSock.port << "\n";
			return 1;
		}

		lstnSock.fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (lstnSock.fd == -1)
		{
			std::cerr << "Error (socket): " << strerror(errno) << "\n";
			freeaddrinfo(res);
			return 1;
		}

		int yes = 1;
		if (setsockopt(lstnSock.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			std::cerr << "Error (setsockopt SO_REUSEADDR): " << strerror(errno) << "\n";
			close(lstnSock.fd);
			freeaddrinfo(res);
			return 1;
		}

		if (bind(lstnSock.fd, res->ai_addr, res->ai_addrlen) == -1)
		{
			std::cerr << "Error (bind): " << strerror(errno) << "\n";
			close(lstnSock.fd);
			freeaddrinfo(res);
			return 1;
		}

		if (listen(lstnSock.fd, SOMAXCONN) == -1)
		{
			std::cerr << "Error (listen): " << strerror(errno) << "\n";
			close(lstnSock.fd);
			freeaddrinfo(res);
			return 1;
		}

		// Save socket infos
		std::memcpy(&lstnSock.addr, res->ai_addr, res->ai_addrlen);
		lstnSock.addrlen = res->ai_addrlen;

		freeaddrinfo(res);

		// Make listen socket non-blocking
		int flags = fcntl(lstnSock.fd, F_GETFL, 0);
		if (flags == -1 || fcntl(lstnSock.fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			std::cerr << "Error (fcntl): " << strerror(errno) << "\n";
			close(lstnSock.fd);
			return 1;
		}

		std::cout << "Listening on "
			<< (lstnSock.ip.empty() ? "0.0.0.0" : lstnSock.ip) << ":"
			<< lstnSock.port << "\n";
	}
	return 0;
}
