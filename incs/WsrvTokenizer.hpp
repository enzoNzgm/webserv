#ifndef TOKENIZER_HPP
# define TOKENIZER_HPP

# include <cstring>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <vector>

enum TokenType
{
	unknown = -1,
	directive,
	parameter,
	open_brace = '{',
	close_brace = '}',
	semicolon = ';'
};

struct WsrvTokenHelper
{
	std::string	word;
	char	c;
	bool	direct;	// (directive) true if encounter ';', '{' or '}'
	bool	dQ;		// true if enter in double quotes
	bool	sQ;		// true if enter in simple quotes
	bool	comm;	// true if in comment

	WsrvTokenHelper() : word(""), c('\0'), direct(true), dQ(false), sQ(false),
		comm(false) {};
};

struct WsrvToken
{
	std::string	name;
	TokenType	type;
	int			line;

	WsrvToken() : name(""), type(unknown), line(-1) {};
	WsrvToken(std::string name) : name(name), type(unknown), line(-1) {};
	WsrvToken(std::string name, int line) : name(name),type(unknown), line(line) {};
};

struct WsrvConfFile
{
	std::string				fName;
	std::ifstream			fStrm;
	std::vector<WsrvToken>	tokens;
};

int			wsrv_open_conf_file(WsrvConfFile& conf, int ac, char**av);

bool		wsrv_set_tokens(WsrvConfFile& conf);
void		wsrv_set_tokens_types(WsrvToken& token, WsrvTokenHelper& thlp);
std::string	wsrv_get_token_type(const TokenType tok_type);
void		wsrv_print_tokens(const WsrvConfFile& conf);

#endif