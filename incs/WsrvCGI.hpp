#ifndef WSRV_CGI_HPP
# define WSRV_CGI_HPP

# include <string>
# include <map>
# include <vector>

class WsrvCGI
{
public:
	WsrvCGI();
	~WsrvCGI();

	std::string run(
		const std::string& scriptPath,
		const std::string& body,
		const std::map<std::string, std::string>& env
	);

private:
	void buildEnv(const std::map<std::string,std::string>& env,
				  std::vector<char*>& out);
};

#endif
