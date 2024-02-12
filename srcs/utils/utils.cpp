#include "IRC.hpp"
#include "User.hpp"

string	delete_spaces(const string &to_delete)
{
	size_t	start_pos = to_delete.find_first_not_of(" ");
	size_t	end_pos = to_delete.find_last_not_of(" "); 


	if (string::npos != start_pos && string::npos != end_pos)
		return to_delete.substr(start_pos, end_pos - start_pos + 1);
	return "";
}

int	find_user_fd(int fd, t_data &data)
{
	t_users::const_iterator	it;
	t_users::const_iterator	ite;

	it = data.users.begin();
	ite = data.users.end();
	for (; it != ite; it++)
		if (it->first == fd)
			return (it->first);
	return -1;
}

int	ft_atoi(string &number)
{
	int value;

	istringstream iss(number);
	iss >> value;
	return value;
}

string	ft_itoa(int n)
{
	stringstream ss;

	ss << n;
	return  ss.str();
}