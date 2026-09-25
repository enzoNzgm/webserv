# include "WsrvUtils.hpp"

static std::string indent(int lvl) { return std::string(lvl * 2, ' '); }

// ------------------------------------------------------------
// Vector<string> (simple directive)
// ------------------------------------------------------------
static void wsrv_print_vec_str(const wsrv_vec_str_t& v, const std::string& name, int lvl)
{
	std::cout << indent(lvl) << name << " = ";
	if (v.empty()) { std::cout << "(null)\n"; return; }

	for (size_t i = 0; i < v.size(); ++i)
		std::cout << v[i] << (i + 1 < v.size() ? ", " : "");
	std::cout << "\n";
}

// ------------------------------------------------------------
// error_page (pairs) — simple directive (NOT A BLOCK)
// ------------------------------------------------------------
static void wsrv_print_error_page(const wsrv_vec_pair_str_t& v, int lvl)
{
	std::cout << indent(lvl) << "error_page = ";
	if (v.empty()) { std::cout << "(null)\n"; return; }

	for (size_t i = 0; i < v.size(); ++i)
	{
		std::cout << v[i].first << " " << v[i].second;
		if (i + 1 < v.size()) std::cout << ", ";
	}
	std::cout << "\n";
}

// ------------------------------------------------------------
// Location Block
// ------------------------------------------------------------
static void wsrv_print_location(const WsrvLocation& loc, int lvl)
{
	// Header: "location: <uri>"
	std::cout << "\n" << indent(lvl) << "location: "
	          << (loc.uri.empty() ? "(null)" : loc.uri) << "\n";

	// Remaining directives:
	std::cout << indent(lvl + 1) << "autoindex = "
	          << (loc.autoindex.empty() ? "(null)" : loc.autoindex) << "\n";

	std::cout << indent(lvl + 1) << "client_max_body_size = "
	          << (loc.client_max_body_size.empty() ? "(null)" : loc.client_max_body_size)
	          << "\n";

	wsrv_print_error_page(loc.error_page, lvl + 1);
	wsrv_print_vec_str(loc.limit_except, "limit_except", lvl + 1);
}

// ------------------------------------------------------------
// listen (pairs) — simple directive (NOT A BLOCK)
// ------------------------------------------------------------
static void wsrv_print_listen(const wsrv_vec_pair_str_t& v, int lvl)
{
	std::cout << indent(lvl) << "listen = ";
	if (v.empty()) { std::cout << "(null)\n"; return; }

	for (size_t i = 0; i < v.size(); ++i)
	{
		std::cout << v[i].first << (v[i].first.empty() ? " " : ":") << v[i].second;
		if (i + 1 < v.size()) std::cout << ", ";
	}
	std::cout << "\n";
}

// ------------------------------------------------------------
// Server Block
// ------------------------------------------------------------
static void wsrv_print_server(const WsrvServer& srv, int lvl)
{
	std::cout << "\n" << indent(lvl) << "server:\n";

	wsrv_print_listen(srv.listen, lvl + 1);
	wsrv_print_vec_str(srv.server_name, "server_name", lvl + 1);

	std::cout << indent(lvl + 1) << "root = "
	          << (srv.root.empty() ? "(null)" : srv.root) << "\n";

	wsrv_print_vec_str(srv.index, "index", lvl + 1);

	std::cout << indent(lvl + 1) << "autoindex = "
	          << (srv.autoindex.empty() ? "(null)" : srv.autoindex) << "\n";

	wsrv_print_error_page(srv.error_page, lvl + 1);

	std::cout << indent(lvl + 1) << "client_max_body_size = "
	          << (srv.client_max_body_size.empty() ? "(null)" : srv.client_max_body_size)
	          << "\n";

	for (size_t i = 0; i < srv.location.size(); ++i)
		wsrv_print_location(srv.location[i], lvl + 1);
}

// ------------------------------------------------------------
// HTTP Block
// ------------------------------------------------------------
void wsrv_print_http(const WsrvHttp& http, int lvl)
{
	std::cout << "http:\n";

	std::cout << indent(lvl + 1) << "root = "
	          << (http.root.empty() ? "(null)" : http.root) << "\n";

	std::cout << indent(lvl + 1) << "client_max_body_size = "
	          << (http.client_max_body_size.empty() ? "(null)" : http.client_max_body_size)
	          << "\n";

	wsrv_print_error_page(http.error_page, lvl + 1);

	for (size_t i = 0; i < http.server.size(); ++i)
		wsrv_print_server(http.server[i], lvl + 1);
	std::cout << "\n";
}
