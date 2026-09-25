#include "WsrvFiller.hpp"

wsrv_vec_tok_cit_t WsrvFiller::prev()
{
	wsrv_vec_tok_cit_t	prev = it;

	if (prev == itBegin)
		return prev;
	return --prev;
}

// --------------- Main context ---------------

bool	wsrv_main_filler(WsrvCore& core)
{
	WsrvFiller	fil(core.conf.tokens);
	bool		res = 0;

	fil.ctx.push(ctx_main);
	while (fil.it != fil.itEnd)
	{
		if (fil.it->name == "http")
			res = wsrv_http_filler(core.http, fil);
		else
			res = wsrv__error_token(fil);
		if (res)
			return res;
		fil.it++;
	}
	fil.ctx.pop();
	return 0;
}

// -------------------- Http context --------------------

bool	wsrv_http_filler(WsrvHttp& http, WsrvFiller& fil)
{
	bool	res = 0;

	fil.ctx.push(ctx_http);
	fil.it++;
	if (wsrv_ckeck_open_brace(fil))
		return 1;
	fil.it++;
	while ((fil.it != fil.itEnd) && (fil.it->type != close_brace))
	{
		if (wsrv_ckeck_close_brace(fil))
			return 1;
		else if (fil.it->name == "root")
			res = wsrv_fill_root(http, fil);
		else if (fil.it->name == "client_max_body_size")
			res = wsrv_fill_client_max_body_size(http, fil);
		else if (fil.it->name == "error_page")
			res = wsrv_fill_error_page(http, fil);
		else if (fil.it->name == "server")
			res = wsrv_server_filler(http, fil);
		else
			res = wsrv__error_token(fil);
		if (res)
			return res;
		fil.it++;
	}
	if (wsrv_ckeck_close_brace(fil))
		return 1;
	fil.ctx.pop();
	return 0;
}

// -------------------- Server context --------------------

bool	wsrv_server_filler(WsrvHttp& http, WsrvFiller& fil)
{
	http.server.push_back(WsrvServer(http));
	WsrvServer&	server = http.server.back();
	bool		res = 0;

	fil.ctx.push(ctx_server);
	fil.it++;
	if (wsrv_ckeck_open_brace(fil))
		return 1;
	fil.it++;
	while ((fil.it != fil.itEnd) && (fil.it->type != close_brace))
	{
		if (wsrv_ckeck_close_brace(fil))
			return 1;
		else if (fil.it->name == "listen")
			res = wsrv_server_fill_listen(server, fil);
		else if (fil.it->name == "server_name")
			res = wsrv_server_fill_server_name(server, fil);
		else if (fil.it->name == "root")
			res = wsrv_fill_root(server, fil);
		else if (fil.it->name == "index")
			res = wsrv_fill_index(server, fil);
		else if (fil.it->name == "autoindex")
			res = wsrv_fill_autoindex(server, fil);
		else if (fil.it->name == "client_max_body_size")
			res = wsrv_fill_client_max_body_size(server, fil);
		else if (fil.it->name == "error_page")
			res = wsrv_fill_error_page(server, fil);
		else if (fil.it->name == "location")
			res = wsrv_location_filler(server, fil);
		else
			res = wsrv__error_token(fil);
		if (res)
			return res;
		fil.it++;
	}
	if (wsrv_ckeck_close_brace(fil))
		return 1;
	fil.ctx.pop();
	return 0;
}

bool	wsrv_server_fill_listen(WsrvServer& server, WsrvFiller& fil)
{
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != parameter))
		return wsrv__print_msg_error(fil, "a 'port' or 'address:port'");
	wsrv_server_split_listen(server, fil);
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

bool	wsrv_server_fill_server_name(WsrvServer& server, WsrvFiller& fil)
{
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != parameter))
		return wsrv__print_msg_error(fil, "a 'name'");
	while ((fil.it != fil.itEnd) && (fil.it->type != semicolon))
	{
		server.server_name.push_back(fil.it->name);
		fil.it++;
	}
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

void	wsrv_server_split_listen(WsrvServer& server, WsrvFiller& fil)
{
	server.listen.push_back(wsrv_pair_str_t());
	wsrv_pair_str_t&	listen = server.listen.back();
	size_t				col_pos = fil.it->name.rfind(':');

	if (col_pos != std::string::npos)
	{
		listen.first = fil.it->name.substr(0, col_pos);
		listen.second = fil.it->name.substr(col_pos + 1);
	}
	else
	{
		listen.first = "";
		listen.second = fil.it->name;
	}
}

// -------------------- Location context --------------------

bool	wsrv_location_filler(WsrvServer& server, WsrvFiller& fil)
{
	server.location.push_back(WsrvLocation(server));
	WsrvLocation&	location = server.location.back();
	bool			res = 0;

	fil.ctx.push(ctx_location);
	fil.it++;
	if (wsrv_location_fill_uri(location, fil))
		return 1;
	if (wsrv_ckeck_open_brace(fil))
		return 1;
	fil.it++;
	while ((fil.it != fil.itEnd) && (fil.it->type != close_brace))
	{
		if (wsrv_ckeck_close_brace(fil))
			return 1;
		else if (fil.it->name == "root")
			res = wsrv_fill_root(server, fil);
		else if (fil.it->name == "autoindex")
			res = wsrv_fill_autoindex(location, fil);
		else if (fil.it->name == "client_max_body_size")
			res = wsrv_fill_client_max_body_size(location, fil);
		else if (fil.it->name == "error_page")
			res = wsrv_fill_error_page(location, fil);
		else if(fil.it->name == "limit_except")
			res = wsrv_location_fill_limit_except(location, fil);
		else
			res = wsrv__error_token(fil);
		if (res)
			return res;
		fil.it++;
	}
	if (wsrv_ckeck_close_brace(fil))
		return 1;
	fil.ctx.pop();
	return 0;
}

bool	wsrv_location_fill_uri(WsrvLocation& location, WsrvFiller& fil)
{
	if (fil.it->type != parameter)
		return wsrv__print_msg_error(fil, "a 'uri'");
	location.uri = fil.it->name;
	fil.it++;
	return 0;
}

bool	wsrv_location_fill_limit_except(WsrvLocation& location, WsrvFiller& fil)
{
	fil.it++;
	while ((fil.it != fil.itEnd) && (fil.it->type != semicolon)
		&& (fil.it->type == parameter))
	{
		location.limit_except.push_back(fil.it->name);
		fil.it++;
	}
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

// -------------------- Limit_except context --------------------

// bool	wsrv_limit_except_filler(WsrvLocation& location, WsrvFiller& fil)
// {
// 	WsrvLimitExcept&	limit_except = location.limit_except;
// 	bool				res = 0;

// 	fil.ctx.push(ctx_limit_except);
// 	fil.it++;
// 	if (wsrv_limit_except_fill_method(limit_except, fil))
// 		return 1;
// 	if (wsrv_ckeck_open_brace(fil))
// 		return 1;
// 	fil.it++;
// 	while ((fil.it != fil.itEnd) && (fil.it->type != close_brace))
// 	{
// 		if (wsrv_ckeck_close_brace(fil))
// 			return 1;
// 		else if ((fil.it->name == "allow") || (fil.it->name == "deny"))
// 			res = wsrv_limit_except_fill_right(limit_except, fil);
// 		else
// 			res = wsrv__error_token(fil);
// 		if (res)
// 			return res;
// 		fil.it++;
// 	}
// 	if (wsrv_ckeck_close_brace(fil))
// 		return 1;
// 	fil.ctx.pop();
// 	return 0;
// }

// bool	wsrv_limit_except_fill_method(WsrvLimitExcept& limit_except, WsrvFiller& fil)
// {
// 	while ((fil.it != fil.itEnd) && (fil.it->type != open_brace))
// 	{
// 		if ((fil.it->name != "GET") && (fil.it->name != "POST")
// 			&& (fil.it->name != "DELETE"))
// 			return wsrv__print_msg_error(fil, "'GET', 'POST' or 'DELETE'");
// 		limit_except.method.push_back(fil.it->name);
// 		fil.it++;
// 	}
// 	return 0;
// }

// bool	wsrv_limit_except_fill_right(WsrvLimitExcept& limit_except, WsrvFiller& fil)
// {
// 	wsrv_pair_str_t	right;

// 	right.first = fil.it->name;
// 	fil.it++;
// 	if (fil.it->type != parameter)
// 		return wsrv__print_msg_error(fil, "an ip address");
// 	right.second = fil.it->name;
// 	limit_except.right.push_back(right);
// 	fil.it++;
// 	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
// 		return wsrv__print_msg_error(fil, "';'");
// 	return 0;
// }

// -------------------- Utils functions --------------------

bool		wsrv__print_msg_error(WsrvFiller& fil, std::string msg)
{
	std::cerr << "Error: expected " << msg << " after '" << fil.prev()->name
		<< "' (line: " << fil.prev()->line << ")\n";
	return 1;
}

bool	wsrv__error_token(WsrvFiller& fil)
{
	if (fil.it->type == directive)
	{
		std::cerr << "Error: use of '" << fil.it->name << "' directive "
			<< "is unsupported in " << wsrv__get_context(fil) << " context "
			<< "(line: "<< fil.it->line << ")\n";
	}
	else
	{
		std::cerr << "Error: use of '" << fil.it->name << "' is not expected here "
			<< "(line: "<< fil.it->line << ")\n";
	}
	return 1;
}

bool	wsrv__pass_directive(WsrvFiller& fil)
{
	std::cerr << "Warning: " << fil.it->name << " directive passed in "
		<< wsrv__get_context(fil) << " context (line: " << fil.it->line << ")\n";

	while ((fil.it != fil.itEnd) && (fil.it->type != semicolon)
		&& (fil.it->type != close_brace))
		fil.it++;
	return 0;
}

std::string	wsrv__get_context(WsrvFiller& fil)
{
	switch (fil.ctx.top())
	{
		case ctx_main:
			return std::string("main");
		case ctx_http:
			return std::string("http");
		case ctx_server:
			return std::string("server");
		case ctx_location:
			return std::string("location");
		case ctx_limit_except:
			return std::string("limit_except");
	}
	return std::string("...");
}
