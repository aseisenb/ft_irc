#include "Channel.hpp"
#include <iostream>
#include "User.hpp"
#include "messages.hpp"

/******************************************************************************/
/*																		      */
/*                                START RO3                                   */
/*																		      */
/******************************************************************************/

Channel::Channel()
{
	cout << "Default Channel constructor called" << endl;
}

/*
	The parameter name MUST NOT be empty. It has to be checked in the JOIN cmd
*/
Channel::Channel(string name, int fd_creator) : invite_only(false), topic_is_set(false), topic_is_protected(false), channel_is_locked(false), user_MAX(0), has_user_limit(false) 
{
	if (name[0] == '#')
		this->_name = name;
	else
		this->_name = "#" + name;
	this->add_user(fd_creator);
	cout << "Name, Fd Creator Channel Constructor called" << endl;
}

Channel::~Channel()
{
	cout << "Channel destructor called" << endl;
}

/******************************************************************************/
/*																		      */
/*                                  END RO3                                   */
/*																		      */
/******************************************************************************/

/******************************************************************************/
/*																		      */
/*                               	START CHECKERS                            */
/*																		      */
/******************************************************************************/

bool	Channel::is_user(int fd_user)
{
	vector<int>::iterator it = this->user_fd.begin();
	vector<int>::iterator ite = this->user_fd.end();

	for (; it != ite; it++)
	{
		if (*it == fd_user)
			return true;
	}
	return false;
}

bool	Channel::is_op(int fd_user)
{
	vector<int>::iterator it = this->ops_fd.begin();
	vector<int>::iterator ite = this->ops_fd.end();

	for (; it != ite; it++) {
		if (*it == fd_user)
			return true;
	}

	it = g_data_ptr->operator_fd.begin();
	ite = g_data_ptr->operator_fd.end();
	for (; it != ite; it++) {
		if (*it == fd_user)
			return true;
	}
	return false;
}

bool	Channel::is_invited(int fd_user)
{
	vector<int>::iterator it = this->invited_fd.begin();
	vector<int>::iterator ite = this->invited_fd.end();

	for (; it != ite; it++)
	{
		if (*it == fd_user)
			return true;
	}
	return false;
}

/******************************************************************************/
/*																		      */
/*                               	END CHECKERS                              */
/*																		      */
/******************************************************************************/

/******************************************************************************/
/*																		      */
/*                              START FUNCTIONS                               */
/*																		      */
/******************************************************************************/

void	Channel::add_user(int fd_user)
{
	if (this->user_fd.empty())
		this->ops_fd.push_back(fd_user);
	this->user_fd.push_back(fd_user);
}

void	Channel::kick_user(int fd_to_kick)
{
	this->user_fd.erase(find(this->user_fd.begin(), this->user_fd.end(), fd_to_kick));
	if (is_op(fd_to_kick) == true)
		this->ops_fd.erase(find(this->ops_fd.begin(), this->ops_fd.end(), fd_to_kick));
}

void	Channel::part(int fd_user)
{
	this->user_fd.erase(find(this->user_fd.begin(), this->user_fd.end(), fd_user));
	if (is_op(fd_user) == true)
		this->ops_fd.erase(find(this->ops_fd.begin(), this->ops_fd.end(), fd_user));
	if (is_invited(fd_user) == true)
		this->invited_fd.erase(find(this->invited_fd.begin(), this->invited_fd.end(), fd_user));
}

void	Channel::op_user(int fd_to_op)
{
	if (is_op(fd_to_op) == false)
		this->ops_fd.push_back(fd_to_op);
}

void	Channel::invite_user(int fd_to_invite)
{
	if (is_invited(fd_to_invite) == false)
		this->invited_fd.push_back(fd_to_invite);
}

void	Channel::deop_user(int fd_to_deop)
{
	this->ops_fd.erase(find(this->ops_fd.begin(), this->ops_fd.end(), fd_to_deop));
}

void	Channel::enable_locked_mode(string &key)
{
	this->channel_is_locked = true;;
	this->key = key;
}

void	Channel::disable_locked_mode()
{
	this->channel_is_locked = false;
	this->key.clear();
}

/******************************************************************************/
/*																		      */
/*                              END FUNCTIONS                                 */
/*																		      */
/******************************************************************************/

/******************************************************************************/
/*																		      */
/*                              START SETTERS  	                              */
/*																		      */
/******************************************************************************/

void	Channel::set_invite_only(bool mode)
{
	this->invite_only = mode;
	if (mode == false)
		this->invited_fd.clear();
}

void	Channel::set_topic(string topic)
{
	this->topic_is_set = true;
	this->topic = topic;
}

void	Channel::unset_topic()
{
	this->topic_is_set = false;
	this->topic = "";
}

void	Channel::set_protected_topic(bool mode)
{
	this->topic_is_protected = mode;
}

void	Channel::set_max_users(int max_users)
{
	this->user_MAX = max_users;
}

void	Channel::set_has_user_limit(bool mode)
{ 
	this->has_user_limit = mode;
}

/******************************************************************************/
/*																		      */
/*                              END SETTERS	                                  */
/*																		      */
/******************************************************************************/

/******************************************************************************/
/*																		      */
/*                              START UTILS		                              */
/*																		      */
/******************************************************************************/

void	Channel::broadcast(string message, int fd_emitter)
{
	vector<int>::iterator it = this->user_fd.begin();
	vector<int>::iterator ite = this->user_fd.end();

	if (message.size() > 510)
		message = message.substr(0, 510);
	message += "\r\n";
	while (it != ite)
	{
		if (fd_emitter == -1 || *it != fd_emitter)
			write(*it, message.c_str(), message.size());
		it++;
	}
	message.clear();
}

void	Channel::print_names(int target_fd)
{
	User	*target_user;
	User	*i_user;
	string	users_info = "";

	vector<int>::iterator users_fd_beg = user_fd.begin();
	vector<int>::iterator users_fd_end = user_fd.end();

	target_user = g_data_ptr->users[target_fd];
	while (users_fd_beg != users_fd_end)
	{
		i_user = g_data_ptr->users[*users_fd_beg];
		if (find(ops_fd.begin(), ops_fd.end(), *users_fd_beg) != ops_fd.end())
			users_info += "@";
		users_info += i_user->getNick();
		users_fd_beg++;
		if (users_fd_beg != users_fd_end)
			users_info += " ";
	}
	// cout << "Users info: " << users_info << endl;
	target_user->sendMessage(RPL_NAMREPLY(_name, target_user->getNick(), target_user->getUser(), "localhost", users_info));
	target_user->sendMessage(RPL_ENDOFNAMES(_name, target_user->getNick(), target_user->getUser(), "localhost"));
}

Channel *Channel::getChannel(string name)
{
    Channel *myChannel;

    try {
        myChannel = g_data_ptr->channels.at(name);
    }
    catch (const std::exception& e) {
        myChannel = NULL;
    }
    return (myChannel);
}

/******************************************************************************/
/*																		      */
/*                              END UTILS		                              */
/*																		      */
/******************************************************************************/

/******************************************************************************/
/*																		      */
/*                            START GETTERS		                              */
/*																		      */
/******************************************************************************/

string			Channel::get_name(void) const {return this->_name;};
vector<int>		Channel::get_users(void) const {return this->user_fd;};
vector<int>		Channel::get_ops(void) const {return this->ops_fd;};
vector<int>		Channel::get_invited(void) const {return this->invited_fd;};
bool			Channel::get_invite_only(void) const {return this->invite_only;};
string			Channel::get_topic(void) const {return this->topic;}
bool			Channel::get_topic_set(void) const {return this->topic_is_set;};
bool			Channel::get_topic_protected(void) const {return this->topic_is_protected ;};
string			Channel::get_key(void) const {return this->key ;};
bool			Channel::get_channel_locked(void) const {return this->channel_is_locked ;};
unsigned int	Channel::get_user_limit(void) const {return this->user_MAX ;};
bool			Channel::get_has_user_limit(void) const {return this->has_user_limit ;};

/******************************************************************************/
/*																		      */
/*                            END GETTERS		                              */
/*																		      */
/******************************************************************************/

