#include "../inc/CGI.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

CGI::CGI() {}
CGI::~CGI() {}

void CGI::buildEnv(const std::map<std::string,std::string>& envMap,
					std::vector<char*>& out)
{
    static std::vector<std::string> buffer;

    buffer.clear();
    for (std::map<std::string,std::string>::const_iterator it = envMap.begin();
         it != envMap.end(); ++it)
        buffer.push_back(it->first + "=" + it->second);

    out.clear();
    for (size_t i = 0; i < buffer.size(); ++i)
        out.push_back(const_cast<char*>(buffer[i].c_str()));
    out.push_back(NULL);
}

std::string CGI::run(
    const std::string& scriptPath,
    const std::string& body,
    const std::map<std::string,std::string>& envMap
)
{
    int inPipe[2];
    int outPipe[2];

    if (pipe(inPipe) < 0 || pipe(outPipe) < 0)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
    {
        close(inPipe[0]);
        close(inPipe[1]);
        close(outPipe[0]);
        close(outPipe[1]);
        throw std::runtime_error("fork failed");
    }

    if (pid == 0)
    {
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);

        close(inPipe[0]);
        close(inPipe[1]);
        close(outPipe[0]);
        close(outPipe[1]);

        std::vector<char*> envp;
        buildEnv(envMap, envp);

        char* const argv[] = {
            (char*)"/usr/bin/python3",
            (char*)scriptPath.c_str(),
            NULL
        };

        execve("/usr/bin/python3", argv, envp.data());
        exit(1);
    }

    close(inPipe[0]);
    close(outPipe[1]);

    write(inPipe[1], body.c_str(), body.size());
    close(inPipe[1]);

    std::string output;
    char buffer[4096];
    ssize_t n = 0;

    while ((n = read(outPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, n);

    close(outPipe[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0 && output.empty())
        throw std::runtime_error("CGI script execution failed");

    return output;
}

