#include "IRC.hpp"

int	ft_atoi(string &number)
{
	int value;

	istringstream iss(number);
	iss >> value;
	return value;
}