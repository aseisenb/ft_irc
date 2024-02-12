#ifndef USER_HPP
# define USER_HPP

# include <string>
# include <set>
# include <vector>
# include <iostream>
# include "IRC.hpp"
# include "Channel.hpp"

using namespace std;

class User
{
	private:
		/*	User info	*/
		int		_id, _fd;
		string	_nick, _user, _name;
		
		/*	Authentication level	*/
		bool	_has_password, _has_nick, _has_user, _is_identified;
		
		/*	Pointers to the classes the user is in	*/
		vector<Channel *>	_channels;
		t_data				*server;

	public:
		list<t_cmd>		cmds; //TODO change back  to private
		/*	Constructors and Destructor	*/
		User();
		~User();
		User(int fd, int id);
		User(const  User &copy);
		User& operator=(const User &cpy);


		/*	Class	Methods	*/
		void	pushCommand(t_cmd &cmd);
		bool	sendMessage(const string &message);

		/*	cmd	handler	*/
		void	executeCommands(t_cmd &cmd);

		bool	commandPASS(t_cmd &cmd);
		bool	commandNICK(t_cmd &cmd);
		bool	USER(t_cmd &cmd);
		bool	commandPING(t_cmd &cmd);
		bool	commandJOIN(t_cmd &cmd);
		bool	NAMES(t_cmd &cmd);
		bool	commandPRIVMSG(t_cmd &cmd);
		int		commandQUIT(t_cmd &cmd);
		bool	PART(t_cmd &cmd);
		bool	KICK(t_cmd &cmd);
		int		commandKILL(t_cmd &cmd);
		bool	OPER(t_cmd &cmd);
		bool	commandMODE(t_cmd &cmd);
		bool	TOPIC(t_cmd &cmd);
		bool	NOTICE(t_cmd &cmd);
		bool	commandINVITE(t_cmd &cmd);
		bool	commandUnknown(t_cmd &cmd);

		bool	deleteChannel(string channel_name);


		/*	Static function	*/
		static User *getUser(std::string nick, t_data *server);


		/*	Getters and Setters	*/
		int		getId(void) const;
		int		getFd(void) const;
		string	getNick(void) const;
		string	getUser(void) const;
		bool	getIdentification(void) const;
		bool    isOperator(void);
};

#endif