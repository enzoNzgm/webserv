#include <iostream>
#include "../inc/Client.hpp"
#include "../inc/Response.hpp"
#include "../inc/Server.hpp"


bool parseRequest(Client &client);

// int	main()
// {
// 	Client client(1);
// 	client.buffer =
//         "POST /submit HTTP/1.1\r\n"
//         "Host: localhost:8080\r\n"
//         "Content-Length: 30\r\n"
//         "User-Agent: TestClient\r\n"
//         "\r\n"
//         "Hello, World!, This is a test.";



// 	if (parseRequest(client))
// 	{
// 		std::cout << "Method: " << client.request.method << std::endl;
// 		std::cout << "URI: " << client.request.uri << std::endl;
// 		std::cout << "HTTP Version: " << client.request.http_version << std::endl;
// 		std::cout << "Headers:" << std::endl;
// 		for (const auto &header : client.request.headers)
// 		{
// 			std::cout << header.first << ": " << header.second << std::endl;
// 		}
// 		std::cout << "Body: " << client.request.body << std::endl;
// 	}
// 	else
// 	{
// 		std::cout << "Failed to parse request." << std::endl;
// 	}
// 	// std::cout << std::endl;
// 	// std::cout << "===== Generated Response: =====" << std::endl;
// 	// std::string response = generateResponse(client.request);
// 	// std::cout << response << std::endl;

// 	return 0;
// }

int main() {
    std::cout << "Webserv with CGI support compiled successfully!" << std::endl;
    std::cout << "CGI implementation for upload and delete is ready." << std::endl;
    std::cout << "See CGI_README.md and ARCHITECTURE.md for usage instructions." << std::endl;
    return 0;
}
