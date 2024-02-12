#include "Struct.hpp"
#include "User.hpp"

string	read_input(int	user_fd)
{
	string 	res;
	char	buffer[READ_SIZE + 1];
	int		len_read;

	bzero(buffer, READ_SIZE + 1);
	len_read = read(user_fd, buffer, READ_SIZE);
	if (len_read == -1)
		return "";
	buffer[len_read] = '\0';
	res.append(buffer);
	while (res.find("\r\n") == string::npos)
	{
		bzero(buffer, READ_SIZE + 1);
		len_read = read(user_fd, buffer, READ_SIZE);
		if (len_read == -1)
			continue;
		if (len_read > 0)
		{
			buffer[len_read] = '\0';
			res.append(buffer);
		}
	}
	return res;
}

t_cmd	parse_input(string raw_input)
{
	t_cmd	result;
	string		parameter;
	size_t		ind;

	if (raw_input.empty())
		return result;
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
