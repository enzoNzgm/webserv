#include "WsrvTokenizer.hpp"

int	wsrv_open_conf_file(WsrvConfFile& conf, int ac, char** av)
{
	if (ac != 2)
	{
		std::cerr << "Error: expected 1 argument\n";
		return 1;
	}

	std::string	file_name = av[1];
	if ((file_name.size() < 5)
		&& (file_name.substr(file_name.size() - 5) != ".conf"))
	{
		std::cerr << "Error: expected a *.conf file\n";
		return 1;
	}

	conf.fName = file_name;

	conf.fStrm.open(conf.fName.c_str(), std::ios::in);
	if (!conf.fStrm.is_open())
		std::cerr << "Error: " << std::strerror(errno) << "\n";
	return errno;
}

bool	wsrv_set_tokens(WsrvConfFile& conf)
{
	WsrvTokenHelper	thlp;
	int				line = 1;

	while (conf.fStrm.get(thlp.c))
	{
		// Check if new line
		if (thlp.c == '\n')
		{
			thlp.comm = false;
			line++;
		}

		// Skip comments
		if (thlp.comm || ((!thlp.sQ && !thlp.dQ) && (thlp.c == '#')))
		{
			thlp.comm = true;
			continue;
		}

		// Skip spaces outside quotes
		if ((!thlp.sQ && !thlp.dQ) && std::isspace(thlp.c))
			continue;

		// Check if it's a starting or ending quote
		if (!thlp.dQ && (thlp.c == '\''))
			thlp.sQ = !thlp.sQ;
		else if (!thlp.sQ && (thlp.c == '\"'))
			thlp.dQ = !thlp.dQ;

		// Push ending token (open_brace, close_brace or semicolon)
		if ((!thlp.sQ && !thlp.dQ) && ((thlp.c == '{') || (thlp.c == '}') || (thlp.c == ';')))
		{
			conf.tokens.push_back(WsrvToken(std::string(1, thlp.c), line));
			wsrv_set_tokens_types(conf.tokens.back(), thlp);
			thlp.direct = true;
			continue;
		}

		// Push c to word if not a starting or ending quote
		if (!((thlp.c == '\"') && ((!thlp.sQ && thlp.dQ) || (!thlp.sQ && !thlp.dQ)))
				&& !((thlp.c == '\'') && ((thlp.sQ && !thlp.dQ) || (!thlp.sQ && !thlp.dQ))))
				thlp.word.push_back(thlp.c);

		// Push token if completed
		int	next = conf.fStrm.peek();
		if ((!thlp.sQ && !thlp.dQ)
			&& (std::isspace(next) || (next == '{')|| (next == '}')
			|| (next == ';') || (next == EOF)))
		{
			conf.tokens.push_back(WsrvToken(thlp.word, line));
			thlp.word.clear();
			wsrv_set_tokens_types(conf.tokens.back(), thlp);
		}
	}
	if (!conf.fStrm.eof() || !conf.fStrm.is_open())
	{
		std::cerr << "Error: didn't reached the end of " << conf.fName << "\n";
		return 1;
	}
	conf.fStrm.close();
	return 0;
}

void	wsrv_set_tokens_types(WsrvToken& token, WsrvTokenHelper& thlp)
{
	if (thlp.c == '{')
		token.type = open_brace;
	else if (thlp.c == '}')
		token.type = close_brace;
	else if (thlp.c == ';')
		token.type = semicolon;
	else if (thlp.direct)
	{
		token.type = directive;
		thlp.direct = false;
	}
	else
		token.type = parameter;
}

std::string wsrv_get_token_type(TokenType tok_type)
{
	if (tok_type == unknown)
		return std::string("unknown");
	else if (tok_type == directive)
		return std::string("directive");
	else if (tok_type == parameter)
		return std::string("parameter");
	else if (tok_type == open_brace)
		return std::string("open_brace");
	else if (tok_type == close_brace)
		return std::string("close_brace");
	else if (tok_type == semicolon)
		return std::string("semicolon");
	return std::string("NULL");
}

void	wsrv_print_tokens(const WsrvConfFile& conf)
{
		size_t	i = 0;
	while (i < conf.tokens.size())
	{
		std::cout << std::setw(3) << std::setfill('0') << i << " --> ";
		std::cout << std::setw(30) << std::setfill(' ') << conf.tokens[i].name;
		std::cout << std::setw(20) << std::setfill(' ');
		std::cout << wsrv_get_token_type(conf.tokens[i].type);
		std::cout << std::setw(10) << std::setfill(' ') << conf.tokens[i].line;
		std::cout << "\n";
		i++;
	}
	std::cout << "\n";
}
