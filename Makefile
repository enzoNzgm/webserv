vpath %.cpp srcs
vpath %.hpp incs

INCSDIR = incs
INCS = WsrvCore.hpp WsrvFiller.hpp WsrvChecker.hpp WsrvStruct.hpp \
		WsrvSocket.hpp WsrvTokenizer.hpp WsrvUtils.hpp WsrvEpoll.hpp \
		WsrvRequest.hpp WsrvResponse.hpp WsrvHandler.hpp WsrvCGI.hpp

SRCS = main.cpp WsrvChecker.cpp WsrvFiller.cpp WsrvSocket.cpp WsrvTokenizer.cpp \
		WsrvUtils.cpp WsrvEpoll.cpp WsrvParseRequest.cpp WsrvResponse.cpp \
		WsrvHandleGet.cpp WsrvHandlePost.cpp WsrvHandleDelete.cpp WsrvHandler.cpp WsrvCGI.cpp

OBJSDIR = objs
OBJS = $(SRCS:%.cpp=%.o)
OBJS := $(addprefix $(OBJSDIR)/, $(OBJS))
DEPS := $(OBJS:.o=.d)

CXX = c++ -g3
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -I$(INCSDIR) -MMD -MP

RM = rm -rf

NAME = webserv

all : $(NAME)

$(OBJSDIR)/%.o : %.cpp | $(OBJSDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJSDIR) :
	mkdir -p $@

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

-include $(DEPS)

clean :
	$(RM) $(OBJSDIR)

fclean : clean
	$(RM) $(NAME)

re : fclean all

.PHONY : all clean fclean re