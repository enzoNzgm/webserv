#include "WsrvCore.hpp"

#define DEBUG false

static void	handle_sigint(int sig)
{
	(void)sig;
	std::cout << "\r \r";
};

static void	wsrv_signal(void)
{
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigint);
}

int	main(int ac, char **av)
{
	// ---------- Parsing ----------
	WsrvCore	core;

	// Check the arguments and open the configuration file
	if (wsrv_open_conf_file(core.conf, ac, av))
		return 1;

	// Split the configuration file into tokens and close it
	if (wsrv_set_tokens(core.conf))
		return 1;

	// Print the tokens name, type and line
	if (DEBUG)
		wsrv_print_tokens(core.conf);

	// Fill the http, server(s), location(s) and limit_except(s) structures
	if (wsrv_main_filler(core))
		return 1;

	// Print the filled structures
	if (DEBUG)
		wsrv_print_http(core.http);

	// ---------- Connection ----------

	WsrvEpoll	epoll(core.http);

	// Fill the needed infos for each unique listen ip:port
	wsrv_listen_socket_filler(epoll.lstnSock, core.http);

	// Create servers sockets for listening
	if (wsrv_socket_bind_listen(epoll.lstnSock))
		return 1;

	// Handle the incoming signals
	wsrv_signal();

	// Manage the client
	wsrv_epoll(epoll);

	// Close all the open fd
	wsrv_epoll_close(epoll);

	return 0;
}
