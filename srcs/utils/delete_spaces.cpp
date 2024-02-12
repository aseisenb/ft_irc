#include "IRC.hpp"

string	delete_spaces(const string &to_delete)
{
	size_t	start_pos = to_delete.find_first_not_of(" ");
	size_t	end_pos = to_delete.find_last_not_of(" "); 


	if (string::npos != start_pos && string::npos != end_pos)
		return to_delete.substr(start_pos, end_pos - start_pos + 1);
	return "";
}