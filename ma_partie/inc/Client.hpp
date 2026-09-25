#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "Request.hpp"

struct Client {
    int fd;
    std::string buffer;
    Request request;

    Client(int fd_) : fd(fd_) {}
};

#endif
