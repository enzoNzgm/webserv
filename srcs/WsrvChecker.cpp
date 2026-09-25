#include "WsrvChecker.hpp"

bool	wsrv_ckeck_open_brace(WsrvFiller& fil)
{
	if ((fil.it != fil.itEnd) && (fil.it->type == open_brace))
		fil.depth++;
	else
		return wsrv__print_msg_error(fil, "'{'");
	return 0;
}

bool	wsrv_ckeck_close_brace(WsrvFiller& fil)
{
	if (fil.it == fil.itEnd)
	{
		std::cerr << "Error: expected '}' before EOF";
		return 1;
	}
	else if (fil.depth != fil.ctx.top())
	{
		std::cerr << "Error: expected '}' before '" << fil.it->name
			<< "' (line: " << fil.it->line << ")\n";
		return 1;
	}
	else if (fil.it->type == close_brace)
		fil.depth--;
	return 0;
}