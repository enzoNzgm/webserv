// Response.hpp
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

struct Response {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;
};

Response serveFile(const std::string &path);
Response serveHTML(const std::string &html, int status);

#endif
