#include "Struct.hpp"

t_data	*g_data_ptr;

/* Function returns true if both parsing operations are successful */
bool parsing(char **argv, int &port, string &password)
{
	return (parse_port(argv[1], port) && parse_password(argv[2], password));
}

/* Parse command-line arguments for a password */
static bool	parse_password(char *password_str, string &password)
{
	// 1. Copy password_str to the password string
	password = password_str;
	// 2. Check if the password is empty or has a length less than 1
	if (password.empty() || password.size() < 1)
		return error("password can't be empty");
	// 3. If all checks pass, return true
	return true;
}

/* Parse command-line arguments for a port */
static bool parse_port(char *port_str, int &port)
{
	char *end_ptr = NULL;

	// 1. Convert port_str to an integer, set end_ptr to the first invalid character
	errno = 0;
	port = strtod(port_str, &end_ptr);
	// 2. Check for conversion errors
	if (errno == ERANGE)
		return error("Error: port is out of range");
	if (*end_ptr != '\0')
		return error("Error: invalid char in \"port\" argument: " + string(1, *end_ptr));
	if (port < 0)
		return error("Error: port number can't be negative");
	if (port <= 1023)
		return error("Error: port is system reserved port range (0-1023)");
	if (port > 65535)
		return error("Error: port number is too big");
	return true;	
}
