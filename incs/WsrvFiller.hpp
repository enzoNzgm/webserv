#ifndef FILLER_HPP
# define FILLER_HPP

# include <stack>
# include "WsrvStruct.hpp"
# include "WsrvTokenizer.hpp"
# include "WsrvChecker.hpp"
# include "WsrvCore.hpp"

struct WsrvCore;

typedef std::vector<WsrvToken>					wsrv_vec_tok_t;
typedef std::vector<WsrvToken>::const_iterator	wsrv_vec_tok_cit_t;

enum Context
{
	ctx_main,
	ctx_http,
	ctx_server,
	ctx_location,
	ctx_limit_except
};

struct WsrvFiller
{
	const wsrv_vec_tok_t&		tokens;
	const wsrv_vec_tok_cit_t	itBegin;
	const wsrv_vec_tok_cit_t	itEnd;
	wsrv_vec_tok_cit_t			it;
	std::stack<Context>			ctx;
	int							depth;

	WsrvFiller(const std::vector<WsrvToken>& toks) : tokens(toks),
		itBegin(tokens.begin()), itEnd(tokens.end()), it(tokens.begin()),
		depth(0) {};

	wsrv_vec_tok_cit_t	prev();
};


bool	wsrv_main_filler(WsrvCore& core);

bool	wsrv_http_filler(WsrvHttp& http, WsrvFiller& fil);

bool	wsrv_server_filler(WsrvHttp& http, WsrvFiller& fil);
bool	wsrv_server_fill_listen(WsrvServer& server, WsrvFiller& fil);
void	wsrv_server_split_listen(WsrvServer& server, WsrvFiller& fil);
bool	wsrv_server_fill_server_name(WsrvServer& server, WsrvFiller& fil);

bool	wsrv_location_filler(WsrvServer& server, WsrvFiller& fil);
bool	wsrv_location_fill_uri(WsrvLocation& location, WsrvFiller& fil);
bool	wsrv_location_fill_limit_except(WsrvLocation& location, WsrvFiller& fil);

// bool	wsrv_limit_except_filler(WsrvLocation& location, WsrvFiller& fil);
// bool	wsrv_limit_except_fill_method(WsrvLimitExcept& limit_except, WsrvFiller& fil);
// bool	wsrv_limit_except_fill_right(WsrvLimitExcept& limit_except, WsrvFiller& fil);

// --------------- Utils prototypes ---------------

bool		wsrv__print_msg_error(WsrvFiller& fil, std::string msg);
bool		wsrv__error_token(WsrvFiller& fil);
bool		wsrv__pass_directive(WsrvFiller& fil);
std::string	wsrv__get_context(WsrvFiller& fil);


// --------------- Templates functions ---------------

template <typename T>
bool	wsrv_fill_root(T& wsrvStruct, WsrvFiller& fil)
{
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != parameter))
		return wsrv__print_msg_error(fil, "a path");
	wsrvStruct.root = fil.it->name;
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

template <typename T>
bool	wsrv_fill_index(T& wsrvStruct, WsrvFiller& fil)
{
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != parameter))
		return wsrv__print_msg_error(fil, "a file name");
	while ((fil.it != fil.itEnd) && (fil.it->type != semicolon)
		&& (fil.it->type == parameter))
	{
		wsrvStruct.index.push_back(fil.it->name);
		fil.it++;
	}
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

template <typename T>
bool	wsrv_fill_autoindex(T& wsrvStruct, WsrvFiller& fil)
{
	fil.it++;
	if ((fil.it == fil.itEnd)
		|| ((fil.it->name != "on") && (fil.it->name != "off")))
		return wsrv__print_msg_error(fil, "'on' or 'off'");
	wsrvStruct.autoindex = fil.it->name;
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

template <typename T>
bool	wsrv_fill_client_max_body_size(T& wsrvStruct, WsrvFiller& fil)
{
	fil.it++;
	wsrvStruct.client_max_body_size = fil.it->name;
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

template <typename T>
bool	wsrv_fill_error_page(T& wsrvStruct, WsrvFiller& fil)
{
	wsrvStruct.error_page.push_back(wsrv_pair_str_t());
	wsrv_pair_str_t&	error_page = wsrvStruct.error_page.back();

	fil.it++;
	if ((fil.it == fil.itEnd)
		|| ((fil.it->name.size() != 3)
			|| (fil.it->name.compare(0, 2, "40")
				&& fil.it->name.compare(0, 2, "50"))))
		return wsrv__print_msg_error(fil, "a valid 'code'");
	error_page.first = fil.it->name;
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != parameter))
		return wsrv__print_msg_error(fil, "an 'uri'");
	error_page.second = fil.it->name;
	fil.it++;
	if ((fil.it == fil.itEnd) || (fil.it->type != semicolon))
		return wsrv__print_msg_error(fil, "';'");
	return 0;
}

#endif