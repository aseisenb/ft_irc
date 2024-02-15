#include "User.hpp"
#include "Macro.hpp"
#include <sstream>
#include <ctype.h>

/* allows yser to create/join a channel (the user that cerates the channel is given operator privilages)*/ 
bool	User::commandJOIN(t_cmd &cmd)
{
	Channel	*channel;
	string	channel_name;
	string	channel_key;

    /*  Basic checks  */
	if (_is_identified == false) 
	{
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 1) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "JOIN"));
		return false;
	}

	/*	Parse the channel info */
	channel_name = cmd.parameters.front();
    if (channel_name[0] != '#') 
	{
        sendMessage(ERR_BADCHANMASK(channel_name));
        return false;
    }
	if (cmd.parameters.size() != 2)
		channel_key = "";
	else
		channel_key = cmd.parameters.back();
	channel = Channel::getChannel(channel_name);

	/*	Interract with the channel	*/
    if (channel)
	{
		if (channel->get_invite_only() == true && channel->is_invited(_fd) == false) 
		{
			sendMessage(ERR_INVITEONLYCHAN(_nick, channel_name)); // user_id(_nick, _user, "localhost"), 
			return false;
		}
		if (channel->get_has_user_limit() == true && channel->get_user_limit() == channel->get_users().size()) 
		{
			sendMessage(ERR_CHANNELISFULL(_nick, channel_name));
			return false;
		}
		if (channel->get_channel_locked() == true && channel->get_key() != channel_key) 
		{
			sendMessage(ERR_BADCHANNELKEY(_nick, channel_name));
			return false;
		}
		if (channel->is_user(_fd) == true)
			return true;
		channel->add_user(_fd);
	}
	else
	{
		channel = new Channel(channel_name, _fd);
		sendMessage(CREATEDCHANNEL(channel_name));
		g_data_ptr->channels[channel_name] = channel;
	}
	channel->transmit(JOIN(_nick, _user, "localhost",  channel_name), -1);
	if (channel->get_make_topic() == true) 
	{
		sendMessage(RPL_TOPIC(_nick, _user, "localhost", channel_name, channel->get_topic()));
	}
	channel->print_names(_fd);
	_channels.push_back(channel);
	return true;
}

/*This command allows user so set a topic of a chanel*/
bool	User::commandTOPIC(t_cmd &cmd)
{
	/*	General checks	*/
	if (_is_identified == false)
	{
        if (_has_nick == 1)
		{
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else 
		{
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	} else if (cmd.parameters.size() == 0) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "TOPIC"));
		return false;
	}


	/*	Get user information	*/
	string	channel_name = cmd.parameters.front();
	Channel	*channel = Channel::getChannel(channel_name);

	if (channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, channel_name));
		return false;
	} 
	else if (channel->is_user(_fd) == false) 
	{
		sendMessage(ERR_NOTONCHANNEL(channel_name, _nick));
		return false;
	}

	/*	Reading the topic	*/
	if (cmd.last_param == false)
	{
		if (channel->get_make_topic() == false) 
		{
			sendMessage(RPL_NOTOPIC(_nick, _user, "localhost", channel_name));
		} 
		else 
		{
			sendMessage(RPL_TOPIC(_nick, _user, "localhost", channel_name, channel->get_topic()));
		}
		return true;
	}

	/*	Write the topic	*/
	if (channel->get_protected_topic() == true && channel->is_operator(_fd) == false) 
	{
		sendMessage(ERR_CHANOPRIVSNEEDED(user_id(_nick, _user, "localhost"), channel_name));
		return false;
	}
	if (cmd.have_last_param.size() <= 1)
	{
		channel->delete_topic();
		channel->transmit(RPL_TOPIC2(_nick, _user, "localhost", channel_name, ""), -1);
	} 
	else 
	{
		channel->create_topic(cmd.have_last_param);
		channel->transmit(RPL_TOPIC2(_nick, _user, "localhost", channel_name, cmd.have_last_param), -1);
	}
	return true;
}

/*give a list of names that are in the channel*/
bool	User::commandNAMES(t_cmd &cmd)
{
	Channel	*target_channel;

	/*	Basic tests	*/
	if (_is_identified == false) 
	{
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() == 0) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "NAMES"));
		return false;
	}
	
	target_channel = Channel::getChannel(cmd.parameters.front());
	if (target_channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, target_channel->get_name()));
		return false;
	}
	if (target_channel->is_user(_fd) == false)
	{
		sendMessage(ERR_NOTONCHANNEL(target_channel->get_name(), _nick));
		return false;
	}
	target_channel->print_names(_fd);
	return true;
}


  /*	List cmd?	*/


/*invite a user to a channel*/
bool	User::commandINVITE(t_cmd &cmd)
{
	/*	Basic tests	*/
	if (_is_identified == false) 
	{
        if (_has_nick == 1) 
		{
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } 
		else 
		{
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	} 
	else if (cmd.parameters.size() != 2) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "INVITE"));
		return false;
	}

	/*	Get the information about the user	*/
	string	channel_name;
	string	user_name;
	Channel	*channel;
	User	*user;

	user_name = cmd.parameters.at(0);
	channel_name = cmd.parameters.at(1);
	user = User::getUser(user_name, server);
	channel	= Channel::getChannel(channel_name);

	if (channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, channel_name));
		return false;
	} 
	else if (user == NULL) 
	{
		sendMessage(ERR_NOSUCHNICKCHANNEL(user_name));
		return false;
	} 
	else if (channel->is_user(_fd) == false) 
	{
		sendMessage(ERR_NOTONCHANNEL(channel_name, _nick));
		return false;
	} 
	else if (channel->get_invite_only() == true && channel->is_operator(_fd) == false) 
	{
		sendMessage(ERR_CHANOPRIVSNEEDED(user_id(_nick, _user, "localhost"), channel_name));
		return false;	
	} 
	else if (channel->is_user(user->getFd()) == true) 
	{
		sendMessage(ERR_USERONCHANNEL(_nick, user->getNick(), channel_name));
		return false;
	}

	/*	Invite the user	*/
	sendMessage(RPL_INVITING(user_id(_nick, _user, "localhost"), _nick, user->getNick(), channel_name));
	channel->invite_user(user->getFd());
	user->sendMessage(INVITE(user_id(_nick, _user, "localhost"), user->getNick(), channel_name));
	return true;
}

/*removes the user from the given channel(s)*/
bool	User::commandPART(t_cmd &cmd)
{
	
    /*  Basic checks  */
    
	/*	Basic tests	*/
		/*	If not authenticated	*/
	if (_is_identified == false) 
	{
        if (_has_nick == 1) 
		{
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } 
		else 
		{
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
		/*	If not enough parameters	*/
	if (cmd.parameters.size() == 0) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "PART"));
		return false;
	}

	/*	Check the channel	*/

		/*	If channel doesn't exist  */
			/*	ERR_NOSUCHCHANNEL	*/
	string	channel_name = cmd.parameters.front(); 
	Channel	*channel = Channel::getChannel(channel_name);
	
	if (channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, channel_name));
		return false;
	}
	if (channel->is_user(_fd) == false) 
	{
		sendMessage(ERR_NOTONCHANNEL(channel_name, _nick));
		return false;
	}
	/*	If OK	*/
		/*	Notify everybody that client quitted the channel  */
	if (cmd.last_param == false) 
	{
		channel->transmit(PART_WOREASON(_nick, _user, "localhost", channel_name), -1);
	} 
	else 
	{
		// cout << "I was in WREASON! " << PART_WREASON(_nick, _user, "localhost", channel_name, cmd.have_last_param) << endl;
		channel->transmit(PART_WREASON(_nick, _user, "localhost", channel_name, cmd.have_last_param), -1);
	}
	channel->part(_fd);
	_channels.erase(find(_channels.begin(), _channels.end(), channel));
	/* If channel empty, remove it */
	if (channel->get_users().empty() == true)
	{
		g_data_ptr->channels.erase(channel->get_name());
		delete channel;
	}
	return true;
}

/**
 * @brief	request the forced removal of a user from a channel
 * 
 * @example		KICK <channel> <user> *( "," <user> ) [<comment>]
 * 
 * @attention	One user at a time
 * @link		https://modern.ircdocs.horse/#kick-message
 */
bool	User::commandKICK(t_cmd &cmd)
{
	/*	********************************************************************* */
								/*	Basic tests	*/
	/*	********************************************************************* */
	if (_is_identified == false) 
	{
        if (_has_nick == 1) 
		{
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } 
		else 
		{
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 2) 
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "KICK"));
		return false;
	}

	/*	********************************************************************* */
				/*	Get the information from the user	*/
	/*	********************************************************************* */
	string	channel_name = cmd.parameters.at(0);
	string	target_name	= cmd.parameters.at(1);
	Channel	*channel = Channel::getChannel(channel_name);
	User	*target_user = User::getUser(target_name, server);

	if (channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, channel_name));
		return false;
	}
	if (target_user == NULL) 
	{
		sendMessage(ERR_NOSUCHNICKCHANNEL(cmd.parameters.at(1)));
		return false;
	}
	if (channel->is_user(_fd) == false) 
	{
		sendMessage(ERR_NOTONCHANNEL(channel_name, _nick));
		return false;
	}
	if (channel->is_operator(_fd) == false) 
	{
		sendMessage(ERR_CHANOPRIVSNEEDED(user_id(_nick, _user, "localhost"), channel_name));
		return false;
	}
	if (channel->is_user(target_user->getFd()) == false) 
	{
		sendMessage(ERR_USERNOTINCHANNEL(target_user->getNick(), channel_name));
		return false;
	}
	
	/*	********************************************************************* */
					/*	Kick the user from the channel	*/
	/*	********************************************************************* */

	if (cmd.last_param == true) 
	{
		channel->transmit(RPL_KICK2(user_id(_nick, _user, "localhost"), channel_name, target_name, cmd.have_last_param), -1);
	} else 
	{
		channel->transmit(RPL_KICK2(user_id(_nick, _user, "localhost"), channel_name, target_name, "Don't like your name"), -1);
	}
	channel->kick_user(target_user->getFd());
	target_user->deleteChannel(channel_name);

	//If channel empty, remove Channel
	if (channel->get_users().empty() == true)
	{
		_channels.erase(find(_channels.begin(), _channels.end(), channel));
		delete channel;
	}
	return true;
}

/**
 * @brief	set or remove options (or modes) from a given target
 * 
 * @example	<target> [<modestring> [<mode arguments>...]]
 * 
 * @attention	The User mode has only +o mode
 * @attention	The channel mode has +i, 
 */
bool	User::commandMODE(t_cmd &cmd)
{
	/*	Basic tests	*/
		/*	If not authenticated	*/
	if (_is_identified == false)		// Not identified
	{
        if (_has_nick == 1) 
		{
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } 
		else 
		{
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	} 
	else if (cmd.parameters.size() < 1)			// Bad num of params
	{
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "MODE"));
		return false;
	} 
	else if (cmd.parameters.size() == 1)		// specific cmd
	{
		string	myBeautifulString;

		myBeautifulString = cmd.parameters.front();
		if (myBeautifulString[0] == '#')
		{
			Channel *smartChannel;

			smartChannel = Channel::getChannel(myBeautifulString);
			if (smartChannel == NULL) 
			{
				sendMessage(ERR_NOSUCHCHANNEL(_nick, cmd.parameters.front()));
				return false;
			}
			sendMessage(RPL_CHANNELMODEIS(_nick, smartChannel->get_name()));
		}
		else
		{
			User	*sportyUser;

			sportyUser = User::getUser(myBeautifulString, g_data_ptr);
			if (sportyUser == NULL) 
			{
				sendMessage(ERR_NOSUCHNICKCHANNEL(myBeautifulString));
				return false;
			}
			sendMessage(RPL_UMODEIS(myBeautifulString));
		}
		return true;
	}
	if (cmd.parameters.front()[0] != '#')
		return true;

	/*	********************************************************************* */
							/*	Get user information	*/
	/*	********************************************************************* */
	string	channel_name;
	Channel	*channel;
	
	channel_name = cmd.parameters.front();
	channel = Channel::getChannel(channel_name);
	if (channel == NULL) 
	{
		sendMessage(ERR_NOSUCHCHANNEL(_nick, channel_name));
		return false;
	}
	if (channel->is_operator(_fd) == false) 
	{
		sendMessage(ERR_NOPRIVILEGES(_nick));
		return false;
	}

	/*	********************************************************************* */
							/*	Work with modes	*/
	/*	********************************************************************* */
	string mode;
	
	mode = cmd.parameters.at(1);
	if (mode == "+i") 
	{
		channel->make_invite_only(true);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "+i", "is now invite-only."), -1);
	}
	else if (mode == "-i") 
	{
		channel->make_invite_only(false);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "-i", "is no longer invite-only."), -1);
	}
	else if (mode == "+t") 
	{
		channel->make_topic_protected(true);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "+t", "topic is now protected."), -1);
	}
	else if (mode == "-t") 
	{
		channel->make_topic_protected(false);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "-t", "topic is no longer protected."), -1);
	}
	else if (mode == "+k")
	{
		if (cmd.parameters.size() != 3) 
		{
			sendMessage(ERR_NEEDMOREPARAMS(_nick, "MODE"));
			return false;
		} 
		else if (channel->get_channel_locked() == true)
		{
			sendMessage(ERR_KEYSET(channel_name));
			return false;
		}

		if (cmd.parameters.at(2).empty()) 
		{
			return false;
		} 
		else 
		{
			channel->set_locked_mode(cmd.parameters.at(2));
			channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "+k", "is now locked."), -1);
		}
	}
	else if (mode == "-k")
	{
		channel->unset_locked_mode();
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "-k", "is no longer locked."), -1);
	}
	else if (mode == "+o")
	{
		if (cmd.parameters.size() < 3) 
		{
			ERR_NEEDMOREPARAMS(_nick, "MODE");
			return false;
		}

		string	user_name;
		User	*user;

		user_name = cmd.parameters.at(2);
		user = User::getUser(user_name, server);
		if (user == NULL) 
		{
			sendMessage(ERR_NOSUCHNICKCHANNEL(cmd.parameters.at(2)));
			return false;
		} else if (channel->is_user(user->getFd()) == false) 
		{
			sendMessage(ERR_NOTONCHANNEL(channel_name, user->getNick()));
			return false;
		}

		if (channel->is_operator(user->getFd()) == false) 
		{
			channel->make_operator(user->getFd());
			channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "+o", user->getNick() + " is now channel operator."), -1);
		}
		return true;
	}
	else if (mode == "-o")
	{
		if (cmd.parameters.size() != 3) 
		{
			ERR_NEEDMOREPARAMS(_nick, "MODE");
			return false;
		}

		string	user_name;
		User	*user;

		user_name = cmd.parameters.at(2);
		user = User::getUser(user_name, server);
		if (user == NULL) 
		{
			sendMessage(ERR_NOSUCHNICKCHANNEL(cmd.parameters.at(2)));
			return false;
		} 
		else if (channel->is_user(user->getFd()) == false) 
		{
			sendMessage(ERR_NOTONCHANNEL(channel_name, user->getNick()));
			return false;
		}

		if (channel->is_operator(user->getFd()) == true){
			channel->take_operator(user->getFd());
			channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "-o", user->getNick() + " is no longer channel operator."), -1);
		}
		return true;
	}
	else if (mode == "+l")
	{
		if (cmd.parameters.size() != 3) 
		{
			sendMessage(ERR_NEEDMOREPARAMS(_nick, "MODE"));
			return false;
		}

		int value;
		
		value = atoi(cmd.parameters.at(2).c_str());
		if (value < 1)
			return false;
		channel->set_max_users(value);
		channel->set_has_user_limit(true);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "+l", "is now limited in members ") + ft_itoa(value) + ".", -1);
	}
	else if (mode == "-l")
	{
		channel->set_has_user_limit(false);
		channel->transmit(MODE2(user_id(_nick, _user, "localhost"), channel_name, "-l", "is no longer limited in members."), -1);
	}
	else
	{
		sendMessage(ERR_UNKNOWNMODE(mode));
		return false;
	}
	return true;
}
