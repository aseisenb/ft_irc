#include "Struct.hpp"
#include "User.hpp"

/* This function reads data from a file descriptor (user_fd) until it finds a newline ("\r\n"). 
It handles read errors and appends data to the result string in chunks. 
The final result is the string read from the file descriptor up to the first occurrence of a newline */
string	read_input(int	user_fd)
{
	string 	res;
	char	buffer[READ_SIZE + 1]; // to store the data read from user_fd
	int		len_read; // to store the number of bytes read by the read function

	// 1. Initialize buffer to zeros
	bzero(buffer, READ_SIZE + 1);
	// 2. Read data from user_fd into buffer. The maximum number of bytes to read is READ_SIZE. The actual number of bytes read is stored in len_read
	len_read = read(user_fd, buffer, READ_SIZE);
	// 3. Check for read error. If any the function returns an empty string
	if (len_read == -1)
		return "";
	// 4. Null-terminate the buffer and append it to the result string
	buffer[len_read] = '\0';
	res.append(buffer);
	// 5. Read until a newline ("\r\n") is found in the result string
	while (res.find("\r\n") == string::npos)
	{
		// Initialize buffer to zeros
		bzero(buffer, READ_SIZE + 1);
		// Read more data from user_fd into buffer
		len_read = read(user_fd, buffer, READ_SIZE);
		// Continue to the next iteration if there is a read error
		if (len_read == -1)
			continue;
		// Null-terminate the buffer and append it to the result string
		if (len_read > 0)
		{
			buffer[len_read] = '\0';
			res.append(buffer);
		}
	}
	// 6. Return the accumulated result string
	return res;
}

/* This functions akes a raw input string and parses it into a t_cmd structure.
The info is than stored in the result structure, which includes prefix, cmd, parameters and flags indicating the presence of a last parameter */
t_cmd	parse_input(string raw_input)
{
	t_cmd		result;
	string		parameter;
	size_t		ind;

	// 1. Check if the input is empty
	if (raw_input.empty())
		return result;
	// 2. Check if input has a prefix ":" and parse it
	if (raw_input[0] == ':' && raw_input.size() > 1)
	{
		ind = raw_input.find_first_of(" \r\n");
		result.prefix = raw_input.substr(1, ind);
		raw_input = raw_input.substr(result.prefix.size() + 1, raw_input.size() - result.prefix.size());
		if (raw_input[ind - result.prefix.size()] == '\n')
			return result;
		result.prefix = delete_spaces(result.prefix);
		if (raw_input.size() == 1 && string(" \r\n", 3).find(raw_input[0]))
			raw_input.clear();		
	}

	// 3. Parse command based on spaces and newline characters
	if (raw_input.empty() == false)
	{
		ind = raw_input.find_first_of(" \r\n");
		result.cmd = raw_input.substr(0, ind);
		raw_input = raw_input.substr(result.cmd.size(), raw_input.size() - result.cmd.size());
		if (raw_input[ind - result.cmd.size()] == '\n')
			return result;
		raw_input = delete_spaces(raw_input);
		if (raw_input.size() == 1 && string(" \r\n", 3).find(raw_input[0]))
			raw_input.clear();
	}

	// 3. Parse parameters based on spaces and newline characters
	if (raw_input.empty() == false)
	{
		while (raw_input.empty() == false && !(raw_input.size() > 1 && raw_input[0] == ':'))
		{
			ind = raw_input.find_first_of(" \r\n");
			parameter = raw_input.substr(0, ind);
			result.parameters.push_back(parameter);
			raw_input = raw_input.substr(parameter.size(), raw_input.size() - parameter.size());
			if (ind - result.cmd.size() < raw_input.size() && raw_input[ind - result.cmd.size()] == '\n')
				return result;
		
			raw_input = delete_spaces(raw_input);
			if (raw_input.size() == 1 && string(" \r\n", 3).find(raw_input[0]))
				raw_input.clear();
		}
		if (raw_input.size() > 1 && raw_input[0] == ':')
		{
			result.have_last_param = raw_input.substr(1, raw_input.size() - 2);
			result.last_param = true;
		}
		else
			result.last_param = false;
	}
	return result;
}
