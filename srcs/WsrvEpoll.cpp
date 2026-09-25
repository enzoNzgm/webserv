#include "WsrvEpoll.hpp"
#include "WsrvHandler.hpp"
#include <sstream>

// Find matching location for a URI
static WsrvLocation* wsrv_find_location(const std::string& uri, WsrvServer& server)
{
	WsrvLocation* best_match = NULL;
	size_t best_match_len = 0;

	for (size_t i = 0; i < server.location.size(); i++)
	{
		const std::string& loc_uri = server.location[i].uri;

		if (uri.find(loc_uri) == 0)
		{
			if (loc_uri.length() > best_match_len)
			{
				best_match = &server.location[i];
				best_match_len = loc_uri.length();
			}
		}
	}

	return best_match;
}

// Check if method is allowed for this location
static bool wsrv_is_method_allowed(const std::string& method, WsrvLocation* location)
{
	if (!location || location->limit_except.empty())
		return true;

	for (size_t i = 0; i < location->limit_except.size(); i++)
	{
		if (location->limit_except[i] == method)
			return true;
	}

	return false;
}

static bool	wsrv_epoll_is_listen_socket(const WsrvEpoll& epoll)
{
	const int	event_fd = epoll.events[epoll.ev_i].data.fd;
	for (size_t i = 0; i < epoll.lstnSock.size(); i++)
	{
		if (event_fd == epoll.lstnSock[i].fd)
			return 0;
	}
	return 1;
}

static int	wsrv_epoll_get_listen_socket_index(const WsrvEpoll& epoll)
{
	const int	event_fd = epoll.events[epoll.ev_i].data.fd;
	for (size_t i = 0; i < epoll.lstnSock.size(); i++)
	{
		if (event_fd == epoll.lstnSock[i].fd)
			return i;
	}
	return -1;
}

static bool	wsrv_epoll_ctl(WsrvEpoll& epoll, int fd, uint32_t evts_, int op_)
{
	struct epoll_event	ev;
	ev.events = evts_;
	ev.data.fd = fd;
	if (epoll_ctl(epoll.fd, op_, fd, &ev) == -1)
	{
		std::cerr << "Error (epoll_ctl): " << strerror(errno) << "\n";
		return 1;
	}
	return 0;
}

static bool	wsrv_epoll_client_register(WsrvEpoll& epoll)
{
	const int listen_fd = epoll.events[epoll.ev_i].data.fd;

	// Create client socket
	int cli_fd = accept(listen_fd, NULL, NULL);
	if (cli_fd == -1)
	{
		std::cerr << "Error (epoll_wait): " << strerror(errno) << "\n";
		return 1;
	}

	int	i = wsrv_epoll_get_listen_socket_index(epoll);
	epoll.cliSock.insert(std::make_pair(cli_fd,
		WsrvClientSocket(cli_fd, epoll.lstnSock[i].server_ids)));
	WsrvClientSocket&	client = epoll.cliSock.at(cli_fd);

	// Make client socket non-blocking
	int flags = fcntl(client.fd, F_GETFL, 0);
	if (flags == -1 || fcntl(client.fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "Error (fcntl): " << strerror(errno) << "\n";
		close(client.fd);
		return 1;
	}

	// Add the client socket to the interest list of epoll
	if (wsrv_epoll_ctl(epoll, client.fd, EPOLLIN, EPOLL_CTL_ADD))
		return 1;
	std::cout << "Client connected: fd -> " << client.fd << "\n";
	return 0;
}

static void	wsrv_epoll_handle_client_epollin(WsrvEpoll& epoll)
{
	char				buffer[4096];
	const int&			fd = epoll.events[epoll.ev_i].data.fd;
	WsrvClientSocket&	client = epoll.cliSock.at(fd);

	// Read the client request and store it in the client in_buff
	int	n = read(fd, buffer, sizeof(buffer));
	if (n > 0)
	{
		client.in_buff.append(buffer, n);

		// Try to parse the request
		if (wsrv_parse_request(client))
		{
			// Request is complete and parsed
			std::cout << "Request parsed: " << client.request.method
					  << " " << client.request.uri << "\n";

			// Change the epoll event of the client to write mode
			if (wsrv_epoll_ctl(epoll, client.fd, EPOLLOUT, EPOLL_CTL_MOD))
				return ;
		}
	}
	else if (n <= 0)
	{
		std::cout << "Client disconnected: fd -> " << client.fd << "\n";
		close(fd);
		epoll.cliSock.erase(fd);
		return ;
	}
}

static void	wsrv_epoll_handle_client_epollout(WsrvEpoll& epoll)
{
	const int&			fd = epoll.events[epoll.ev_i].data.fd;
	WsrvClientSocket&	client = epoll.cliSock.at(fd);

	// Generate response if not already done
	if (client.out_buff.empty())
	{
		// Get the first server config associated with this client
		int server_id = client.server_ids[0];
		WsrvServer& server = const_cast<WsrvServer&>(epoll.http.server[server_id]);

		// Check if there was a parsing error (e.g., body too large)
		if (client.request.error_code != 0)
		{
			if (client.request.error_code == 413)
			{
				std::string body = "<html><body><h1>413 Payload Too Large</h1>"
					"<p>Le fichier est trop volumineux. Taille maximale : 1 MB</p></body></html>";
				std::stringstream ss;
				ss << "HTTP/1.1 413 Payload Too Large\r\n"
				   << "Content-Type: text/html\r\n"
				   << "Content-Length: " << body.size() << "\r\n"
				   << "\r\n"
				   << body;
				client.out_buff = ss.str();
			}
			else
			{
				std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";
				std::stringstream ss;
				ss << "HTTP/1.1 400 Bad Request\r\n"
				   << "Content-Type: text/html\r\n"
				   << "Content-Length: " << body.size() << "\r\n"
				   << "\r\n"
				   << body;
				client.out_buff = ss.str();
			}
		}
		// Find matching location for this URI
		else
		{
			WsrvLocation* location = wsrv_find_location(client.request.uri, server);

			// Check if method is allowed by limit_except
			if (!wsrv_is_method_allowed(client.request.method, location))
			{
				// Method not allowed for this location
				client.out_buff = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
			}
			// Handle request based on method
			else if (client.request.method == "GET")
			{
				WsrvResponse res = wsrv_handle_get(client.request, server);
				client.out_buff = wsrv_response_to_string(res);
			}
			else if (client.request.method == "POST")
			{
				client.out_buff = wsrv_handle_post(client.request, server);
			}
			else if (client.request.method == "DELETE")
			{
				WsrvResponse res = wsrv_handle_delete(client.request, server);
				client.out_buff = wsrv_response_to_string(res);
			}
			else
			{
				// Method not implemented
				client.out_buff = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
			}
		}
	}

	// Send the response
	ssize_t sent = write(fd, client.out_buff.c_str(), client.out_buff.size());
	if (sent > 0)
	{
		client.out_buff.erase(0, sent);

		// If all sent, close connection or reset for keep-alive
		if (client.out_buff.empty())
		{
			std::cout << "Response sent to client: fd -> " << fd << "\n";
			close(fd);
			epoll.cliSock.erase(fd);
			return ;
		}
	}
	else if (sent <= 0)
	{
		std::cout << "Client disconnected during write: fd -> " << fd << "\n";
		close(fd);
		epoll.cliSock.erase(fd);
		return ;
	}
}

bool	wsrv_epoll(WsrvEpoll& epoll)
{
	// Create a new instance of epoll
	epoll.fd = epoll_create1(EPOLL_CLOEXEC);
	if (epoll.fd == -1)
	{
		std::cerr << "Error (epoll_create): " << strerror(errno) << "\n";
		return 1;
	}

	// Add the listen sockets to the interest list of epoll
	for (size_t i = 0; i < epoll.lstnSock.size(); i++)
	{
		const WsrvListenSocket&	listen = epoll.lstnSock[i];

		if (wsrv_epoll_ctl(epoll, listen.fd, EPOLLIN, EPOLL_CTL_ADD))
			return 1;
	}

	// Epoll loop
	while (1)
	{
		int	nfds = epoll_wait(epoll.fd, epoll.events, MAX_EVENTS, TIME_OUT);
		if (nfds == -1)
		{
			std::cout << "Warning (epoll_wait): " << strerror(errno) << "\n";
			return 1;
		}

		epoll.ev_i = 0;
		for (; epoll.ev_i < nfds; epoll.ev_i++)
		{
			struct epoll_event	ev = epoll.events[epoll.ev_i];

			if (!wsrv_epoll_is_listen_socket(epoll))
			{
				// Create client socket
				if (wsrv_epoll_client_register(epoll))
					return 1;
			}
			else if (ev.events & (EPOLLIN))
			{
				wsrv_epoll_handle_client_epollin(epoll);
			}
			else if (ev.events & (EPOLLOUT))
			{
				wsrv_epoll_handle_client_epollout(epoll);
			}
		}
	}
	return 0;
}

void	wsrv_epoll_close(WsrvEpoll& epoll)
{
	for (size_t i = 0; i < epoll.lstnSock.size(); i++)
	{
		close(epoll.lstnSock[i].fd);
	}

	std::map<int, WsrvClientSocket>::iterator it = epoll.cliSock.begin();
	for (; it != epoll.cliSock.end(); ++it)
	{
		close(it->second.fd);
	}

	close(epoll.fd);
}
