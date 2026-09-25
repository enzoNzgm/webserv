#ifndef CORE_HPP
# define CORE_HPP

# include <sys/epoll.h>
# include <signal.h>

# include "WsrvTokenizer.hpp"
# include "WsrvStruct.hpp"
# include "WsrvFiller.hpp"
# include "WsrvUtils.hpp"
# include "WsrvSocket.hpp"
# include "WsrvEpoll.hpp"

struct WsrvCore
{
	WsrvConfFile	conf;
	WsrvHttp		http;
};

#endif
