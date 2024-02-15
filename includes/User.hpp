#ifndef USER_HPP
# define USER_HPP

# include <string>
# include <set>
# include <vector>
# include <iostream>
# include "Struct.hpp"
# include "Channel.hpp"

using namespace std;

class User
{
	private:
		// User info
		int		_id, _fd;
		string	_nick, _user, _name;
		
		// Authentication
		bool	_has_password, _has_nick, _has_user, _is_identified;
		
		// Pointers to user classes
		vector<Channel *>	_channels;
		t_data				*server;

	public:
		list<t_cmd>		cmds;

		User();
		~User();
		User(int fd, int id);
		User(const  User &copy);
		User& operator=(const User &cpy);


		// Methods
		void	pushCommand(t_cmd &cmd);
		bool	sendMessage(const string &message);
		void	commandsHandler(t_cmd &cmd);
		bool	commandPASS(t_cmd &cmd);
		bool	commandNICK(t_cmd &cmd);
		bool	commandUSER(t_cmd &cmd);
		bool	commandPING(t_cmd &cmd);
		bool	commandJOIN(t_cmd &cmd);
		bool	commandNAMES(t_cmd &cmd);
		bool	commandPRIVMSG(t_cmd &cmd);
		int		commandQUIT(t_cmd &cmd);
		bool	commandPART(t_cmd &cmd);
		bool	commandKICK(t_cmd &cmd);
		int		commandKILL(t_cmd &cmd);
		bool	commandOPER(t_cmd &cmd);
		bool	commandMODE(t_cmd &cmd);
		bool	commandTOPIC(t_cmd &cmd);
		bool	commandNOTICE(t_cmd &cmd);
		bool	commandINVITE(t_cmd &cmd);
		bool	commandUnknown(t_cmd &cmd);
		bool	deleteChannel(string channel_name);

		static User *getUser(std::string nick, t_data *server);

		//Getters
		int		getId(void) const;
		int		getFd(void) const;
		string	getNick(void) const;
		string	getUser(void) const;
		bool	getIdentification(void) const;
		bool    isOperator(void);
};

#endif