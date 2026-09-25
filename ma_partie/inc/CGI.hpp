#pragma once
#include <string>
#include <map>
#include <vector>

class CGI
{
public:
    CGI();
    ~CGI();

    std::string run(
        const std::string& scriptPath,
        const std::string& body,
        const std::map<std::string, std::string>& env
    );

private:
    void buildEnv(const std::map<std::string,std::string>& env,
                  std::vector<char*>& out);

};
