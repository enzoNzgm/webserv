#include <cctype>
#include <vector>
#include <string>
#include <cstdlib>
#include "../inc/Request.hpp"
#include "../inc/Client.hpp"

std::vector<std::string> split(const std::string &s, const std::string &delim)
{
    std::vector<std::string> result;
    size_t start = 0, pos;

    while ((pos = s.find(delim, start)) != std::string::npos) {
        result.push_back(s.substr(start, pos - start));
        start = pos + delim.length();
    }
    result.push_back(s.substr(start));
    return result;
}

std::string trim(const std::string &s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start]))
        start++;

    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1]))
        end--;

    return s.substr(start, end - start);
}


bool parseRequest(Client &client)
{
    Request &req = client.request;

    // 1) Vérifier si on a fini les headers
    size_t header_end = client.buffer.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    // 2) Récupérer la partie header
    std::string header_part = client.buffer.substr(0, header_end);
    std::vector<std::string> lines = split(header_part, "\r\n");

    // 3) Parse Start-Line
    if (lines.size() < 1)
        return false;

    std::vector<std::string> tokens = split(lines[0], " ");
    if (tokens.size() != 3)
        return false;

    req.method       = tokens[0];
    req.uri          = tokens[1];
    req.http_version = tokens[2];

    // 4) Parse headers
    for (size_t i = 1; i < lines.size(); i++)
    {
        size_t sep = lines[i].find(":");
        if (sep == std::string::npos)
            continue;

        std::string key = trim(lines[i].substr(0, sep));
        std::string val = trim(lines[i].substr(sep + 1));
        req.headers[key] = val;
    }

    // 5) Gestion du body
    std::string body_part = client.buffer.substr(header_end + 4);

    if (req.headers.find("Content-Length") != req.headers.end())
    {
        int len = atoi(req.headers["Content-Length"].c_str());

        // Le body n'est pas encore au complet → attendre
        if (body_part.size() < (size_t)len)
            return false;

        // Body complet → on copie
        req.body = body_part.substr(0, len);
    }
    else
        req.body = "";

    return true;
}


