#include "Struct.hpp"

t_data	*g_data_ptr;

static bool parse_port(char *port_str, int &port)
{
	char *end_ptr = NULL;

	errno = 0;
	port = strtod(port_str, &end_ptr);
	if (errno == ERANGE)
		return error("port is out of range");
	if (*end_ptr != '\0')
		return error("invalid char in \"port\" argument: " + string(1, *end_ptr));
	if (port < 0)
		return error("port number can't be negative");
	if (port <= 1023)
		return error("port is system reserved port range (0-1023)");
	if (port > 65535)
		return error("port number is too big");
	return true;	
}

static bool	parse_password(char *password_str, string &password)
{
	password = password_str;
	if (password.empty() || password.size() < 1)
		return error("password can't be empty");
	return true;
}

bool parsing(char **argv, int &port, string &password)
{
	return (parse_port(argv[1], port) && parse_password(argv[2], password));
}