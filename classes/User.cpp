#include "User.hpp"

User::User(): server(g_data_ptr)
{
	cout << "Default User constructor called" << endl;
}

User::User(const User &copy): server(g_data_ptr)
{
	cout << "Copy User constructor called" << endl; //TODO
	(void)copy;
}

User& User::operator=(const User &cpy)
{
	cout << "User Assignment operator called" << endl; //TODO
	if (this == &cpy)
		return (*this);
	(void)cpy;
	return (*this);
}

User::~User() {}

User::User(int id, int fd) : _id(id), _fd(fd), _has_password(false), _has_nick(false), _has_user(false), _is_identified(false), server(g_data_ptr) {}

void	User::pushCommand(t_cmd &cmd)
{
	this->cmds.push_back(cmd);
}


bool    User::sendMessage(const string &message)
{
	string truncated = message;
	
	if (truncated.size() > 510)
	{
		truncated = truncated.substr(0, 510) + "\r\n";
	}
    if (write(this->_fd, truncated.c_str(), truncated.length()) < 1)
		return false;
	cout << "\033[0;" << 31 + this->_id % 7 << "m" << this->_id << " > " << truncated << "\033[0m";
	return true;
}


/*	************************************************************************* */
						/*	Static functions	*/
/*	************************************************************************* */
User *User::getUser(std::string nick, t_data *server)
{
    User *myUser;

    std::vector<int>::iterator user_begin = server->open_fd.begin();
    std::vector<int>::iterator user_end = server->open_fd.end();
    while (user_begin != user_end)
    {
        myUser = server->users.at(*user_begin);
        if (nick == myUser->getNick())
            return (myUser);
        user_begin++;
    }
    return (NULL);
}

bool    User::isOperator(void)
{
    vector<int>  operators;
    vector<int>::iterator beg;
    vector<int>::iterator end;

    operators = this->server->operator_fd; 
    beg = operators.begin();
    end = operators.end();
    while (beg != end)
    {
        if (*beg == _fd)
            return true;
        beg++;
    }
    return false;
}

bool	User::deleteChannel(string channel_name)
{
    vector<Channel *>::iterator beg;
    vector<Channel *>::iterator end;

    beg = _channels.begin();
    end = _channels.end();
    while (beg != end)
    {
        if ((*beg)->get_name() == channel_name)
        {
            _channels.erase(beg);
            return true;
        }
        beg++;
    }
    return false;
}


/*
				GETTERS
*/
int	User::getFd(void) const {return this->_fd;};
int User::getId(void) const {return this->_id;};
string User::getNick(void) const {return this->_nick;};
string	User::getUser(void) const {return this->_user;};
bool	User::getIdentification(void) const {return this->_is_identified;};
