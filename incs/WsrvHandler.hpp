#ifndef WSRV_HANDLER_HPP
# define WSRV_HANDLER_HPP

# include "WsrvRequest.hpp"
# include "WsrvResponse.hpp"
# include "WsrvStruct.hpp"
# include "WsrvEpoll.hpp"

// Parse request from client buffer
bool wsrv_parse_request(WsrvClientSocket &client);

// Handle GET method
WsrvResponse wsrv_handle_get(WsrvRequest &req, WsrvServer &config);

// Handle POST method (returns raw HTTP response string)
std::string wsrv_handle_post(WsrvRequest &req, WsrvServer &config);

// Handle DELETE method
WsrvResponse wsrv_handle_delete(WsrvRequest &req, WsrvServer &config);

// Convert WsrvResponse to HTTP response string
std::string wsrv_response_to_string(const WsrvResponse &res);

#endif
