#include "User.hpp"
#include "Channel.hpp"
#include "Macro.hpp"
#include <sstream>
#include <ctype.h>

/**
 * ****************************************************************************
 * @brief       send a password to the server during the connection process.
 * 
 * @return      I don't even know why do I need it
 * @link        https://modern.ircdocs.horse/#pass-message
 * 
 * @bug         if multiple params, I take only first
 * @bug         I suppose, the password couldn't passes inside of have_last_param
 * ****************************************************************************
 */
bool	User::commandPASS(t_cmd &cmd)
{
    if (_has_password)
        sendMessage(ERR_ALREADYREGISTERED(ft_itoa(_id)));
    else if (cmd.parameters.size() == 0)
        sendMessage(ERR_NEEDMOREPARAMS(ft_itoa(_id), "PASS"));
    else if (cmd.parameters.front() == server->password)
        _has_password = true;
    else if (cmd.parameters.front() != server->password)
        sendMessage(ERR_PASSWDMISMATCH(ft_itoa(_id)));
    else
        sendMessage("fatal");
    return (true);
}

/**
 * ****************************************************************************
 * @brief       give the client a nickname or change the previous one
 * 
 * @link        https://modern.ircdocs.horse/#nick-message 
 * @link        https://modern.ircdocs.horse/#connection-registration a proper registration
 * 
 * @bug         if multiple params, I take only first
 * @bug         I assume that have_last_param doesn't contain the NICK param
 * @bug         Doesn't understand what does it mean: 
 *                      Servers MAY allow extra characters, as long as they 
 *                      do not introduce ambiguity in other commands
 * ****************************************************************************
 */
bool	User::commandNICK(t_cmd &cmd)
{
	/*  ********************************************************************* */
				/*	Check if password is set	*/
	/*  ********************************************************************* */
	if (!_has_password) {
		sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
		return false;
	}

    /*  ********************************************************************* */
    /*  Check if proper number of parameters    */
    /*  ********************************************************************* */
    if (cmd.parameters.size() == 0){
        if (_has_nick == 1) {
            sendMessage(ERR_NONICKNAMEGIVEN(_nick));
        } else {
            sendMessage(ERR_NONICKNAMEGIVEN(ft_itoa(this->_id)));
        }
        return false;
    }

    /*  ********************************************************************* */
    /*  Check if the Nickname is properly formatted */
    /*  ********************************************************************* */
    std::string param = cmd.parameters.front();
    char        firstChar = param[0];

    /*  Check first */
    if (!(isalpha(firstChar) || firstChar == '[' || firstChar == ']' || firstChar == '{' || firstChar == '}' ||  firstChar == '\\' || firstChar == '|'))
    {
        if (_has_nick == 1) {
            sendMessage(ERR_ERRONEUSNICKNAME(_nick, param));
        } else {
            sendMessage(ERR_ERRONEUSNICKNAME(ft_itoa(this->_id), param));
        }
        return false;
    }

    // /*  Check consecutive */
    // for (size_t i = 1; i < param.length(); ++i)
    // {
    //     char ch = param[i];
    //     if (!(isalnum(ch) || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '\\' || ch == '|')) {
    //         sendMessage(ERR_ERRONEUSNICKNAME(ft_itoa(this->_id), param));
    //         return false;
    //     }
    // }

    /*  ********************************************************************* */
                    /*  Check if the nick already exists    */
    /*  ********************************************************************* */
    for (size_t i = 0; i < g_data_ptr->open_fd.size(); i++)
    {
        if (g_data_ptr->users[g_data_ptr->open_fd[i]]->getNick() == param) {
            if (g_data_ptr->open_fd[i] != this->_fd) {
                param = param + "_";
            }
        }
    }

    /*  ********************************************************************* */
                /*  Inform others about the change of name */
    /*  ********************************************************************* */
    if (this->_has_nick)
    {
        for (size_t i = 0; i < g_data_ptr->open_fd.size(); i++)
        {
            User    *user = g_data_ptr->users[g_data_ptr->open_fd[i]];
            if (user->getIdentification() == true)
                user->sendMessage(NICK(_nick, _user, "localhost", param));
        }
    }

    /*  ********************************************************************* */
                        /*  Assign the nickname */
    /*  ********************************************************************* */
    this->_nick = param;
    _has_nick = true;
    if (this->_has_user && this->_has_password && !this->_is_identified)
    {
        this->_is_identified = true;
        this->sendMessage(RPL_WELCOME(_nick, _user, ft_itoa(g_data_ptr->port), "localhost"));
    }
    return true;
}

/**
 * @brief   specify the username and realname of a new user.
 * 
 * @return  Why do I need it?
 * @link    https://modern.ircdocs.horse/#user-message
 * 
 * @bug     In the protocol, the username could be stated by IDENT protocol
 *          https://datatracker.ietf.org/doc/html/rfc1413 I didn't implement it
 * @bug		It is said that the cmd should contain 0 or *, but I didn't understand why
 */
bool	User::USER(t_cmd &cmd)
{
	/*	********************************************************************* */
								/*	Small checks	*/
	/*	********************************************************************* */
	if (!_has_password) {
        if (_has_nick == 1) {
		    sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
        return false;
	}
	if (cmd.parameters.size() == 0) {
        if (_has_nick == 1) {
            sendMessage(ERR_NEEDMOREPARAMS(_nick, "USER"));
        } else {
    	    sendMessage(ERR_NEEDMOREPARAMS(ft_itoa(_id), "USER"));
        }
        return false;
	}
	if (_has_user) {
        if (_has_nick == 1) {
            sendMessage(ERR_ALREADYREGISTERED(_nick));
        } else {
    	    sendMessage(ERR_ALREADYREGISTERED(ft_itoa(_id)));
        }
        return false;
	}

	/*	********************************************************************* */
							/*	Set the username	*/
	/*	********************************************************************* */
	this->_user = cmd.parameters.front();
	this->_name = cmd.have_last_param;
	_has_user = true;
	if (_has_user && _has_nick && !_is_identified)
	{
		this->_is_identified = true;
		sendMessage(RPL_WELCOME(_nick, _user, ft_itoa(g_data_ptr->port), "localhost"));
	} 
	return true;
}

/**
 * @brief 	check if the server connection is still connected 
 * 
 * @link	https://modern.ircdocs.horse/#ping-message 
 */
bool	User::commandPING(t_cmd &cmd)
{
	/*	********************************************************************* */
						/*	Answer to the request	*/
	/*	********************************************************************* */
	if (_is_identified == false) {
        if (cmd.parameters.size() == 0) {
            sendMessage(PING(ft_itoa(_id), ""));
        } else {
            sendMessage(PING(ft_itoa(_id), cmd.parameters.front()));
        }
    } else {
        if (cmd.parameters.size() == 0) {
            sendMessage(PONG(user_id(_nick, _user, "localhost"), ""));
        } else {
    		sendMessage(PONG(user_id(_nick, _user, "localhost"), cmd.parameters.front()));
        }
    }
    return true;
}

/**
 * @brief	 obtain IRC operator privileges
 * 
 * @param   <name> <password>
 * 
 * @link	https://modern.ircdocs.horse/#oper-message
 */
bool	User::OPER(t_cmd &cmd)
{
	/*	********************************************************************* */
							/*	Basic checks	*/
	/*	********************************************************************* */
	if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 2) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "OPER"));
		return false;
	}

	/*	********************************************************************* */
						/*	Check the user and password	*/
	/*	********************************************************************* */
	if (cmd.parameters.front() != LOGIN || cmd.parameters.back() != PASSWORD) {
		sendMessage(ERR_PASSWDMISMATCH(_nick));
		return false;
	}

	sendMessage(RPL_YOUREOPER(_nick));
    server->operator_fd.push_back(_fd);
	return true;
}

/**
 * @brief   close the connection between a given client and the server
 * 
 * @param       <target nickname> <comment>
 * @return      the fd of the target user
 * 
 * @link        https://modern.ircdocs.horse/#kill-message
 * @attention   the user has to be deleted from the server side
 */
int		User::commandKILL(t_cmd &cmd)
{
    User    *target_user;
    string  target_nick;

    /*	********************************************************************* */
                                /*  Basic tests */
    /*	********************************************************************* */
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 1) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "KILL"));
		return -1;
	}
    if (isOperator() == false) {
        sendMessage(ERR_NOPRIVILEGES(_nick));
        return -1;
    }

    /*	********************************************************************* */
                /*  Get information about the target user   */
    /*	********************************************************************* */
    target_nick = cmd.parameters.front();
    target_user = User::getUser(target_nick, server);
    if (target_user == NULL) {
        sendMessage(ERR_NOSUCHNICKCHANNEL(target_nick));
        return -1;
    }

    /*	********************************************************************* */
            /*  Go through the channels and remove the target user  */
    /*	********************************************************************* */
    vector<Channel *>::iterator iter_beg = _channels.begin();
    vector<Channel *>::iterator iter_end = _channels.end();
    while (iter_beg != iter_end)
    {
        Channel *myChannel = *iter_beg;

        myChannel->kick_user(target_user->getFd());
        target_user->deleteChannel(myChannel->get_name());
        if (myChannel->get_users().size() != 0)
        {
            if (cmd.last_param == 1) {
                myChannel->broadcast(QUIT2(user_id(target_nick, target_user->getUser(), "localhost"), cmd.have_last_param), -1);
            } else {
                myChannel->broadcast(QUIT2(user_id(target_nick, target_user->getUser(), "localhost"), "default reason"), -1);
            }
        } else {   // No more users left in the channel
            this->server->channels.erase(myChannel->get_name());
            delete myChannel;
        }
        iter_beg++;
    }

    /*	********************************************************************* */
                    /*  Notify the user that he was killed  */
    /*	********************************************************************* */
    if (cmd.last_param == true) {
        target_user->sendMessage(KILL(_nick, cmd.have_last_param));
    } else {
        target_user->sendMessage(KILL(_nick, "default reason"));
    }
    return (target_user->getFd());
}

/**
 * @brief       quit the server
 * 
 * @param       [<reason>]
 * @return      the fd of a user
 * 
 * @link        https://modern.ircdocs.horse/#quit-message
 */
int     User::commandQUIT(t_cmd &cmd)
{
    /*	********************************************************************* */
            /*  Go through the channels and remove the target user  */
    /*	********************************************************************* */
    vector<Channel *>::iterator iter_beg = _channels.begin();
    vector<Channel *>::iterator iter_end = _channels.end();
    while (iter_beg != iter_end)
    {
        Channel *myChannel = *iter_beg;

        myChannel->kick_user(_fd);
        _channels.erase(iter_beg);
        if (myChannel->get_users().size() != 0) // There is someone in the channel
        {
            if (cmd.have_last_param.size() != 0)
                myChannel->broadcast(QUIT2(user_id(_nick, _user, "localhost"), cmd.have_last_param), -1);
            else
                myChannel->broadcast(QUIT2(user_id(_nick, _user, "localhost"), "default reason"), -1);
        }
        else    // No more users left in the channel
        {
            this->server->channels.erase(myChannel->get_name());
            delete myChannel;
        }
        iter_beg++;
    }
    return (_fd);
}

/**
 * @brief   send private messages between users
 * 
 * @param   <receiver>{,<receiver>} <text to be sent> 
 * @example PRIVMSG Angel :yes I'm receiving it !       ; cmd to send a message to Angel.
 * @example PRIVMSG %#bunny :Hi! I have a problem!      ; cmd to send a message to halfops and chanops on #bunny.
 * 
 * @link        https://modern.ircdocs.horse/#privmsg-message 
 * @attention   In general, the irc handles privmsg to users with specific
 *              host mask and server mask. This things wasn't integrated 
 *              in the code. But channel mask were handled
 * @attention   I didn't handle the wildcards
 * @attention   I didn't handle several users
 * 
 * @bug         Do I need to handle the banned from channel cases?
 */
bool	User::commandPRIVMSG(t_cmd &cmd)
{
    /*  Basic tests */
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 1 || (cmd.parameters.size() == 1 && cmd.have_last_param.size() == 0)) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "PRIVMSG"));
		return false;
	}

    std::string target = cmd.parameters.front();

    /*  if param is channel */
    if (target[0] == '#')
    {
        Channel *target_channel = Channel::getChannel(target);

        /*  If channel doesn't exist    */ /* Error */
        if (!target_channel) {
       		sendMessage(ERR_NOSUCHCHANNEL(_nick, target));
            return false;
        }

        //  I need to check here if I am in the channel!!!!!!!!!!!!
        if (target_channel->is_user(_fd) == false) {
            sendMessage(ERR_NOTONCHANNEL(target, _nick));
            return false;
        }
        target_channel->broadcast(PRIVMSG2(_nick, _user, "localhost", target, cmd.have_last_param), _fd);
    }
    /*  else param is user  */
    else
    {
        /*  If the target user doesn't exist */
        User    *target_user = User::getUser(target, server);

        if (!target_user) {
            sendMessage(ERR_NOSUCHNICKCHANNEL(target));
            return false;
        }

        /*  Send the message    */
        target_user->sendMessage(PRIVMSG(_nick, _user, "localhost", target, cmd.have_last_param));
    }
    return true;
}

/**
 * @brief   For our project is fully the same
 * 
 * @param   <target>{,<target>} <text to be sent>
 * 
 * @link        https://modern.ircdocs.horse/#notice-message
 * @attention   I didn't handle several users
 */
bool	User::NOTICE(t_cmd &cmd)
{
    /*  Basic tests */
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
	if (cmd.parameters.size() < 1 || (cmd.parameters.size() == 1 && cmd.have_last_param.size() == 0)) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "NOTICE"));
		return false;
	}

    std::string target = cmd.parameters.front();

    /*  if param is channel */
    if (target[0] == '#')
    {
        Channel *target_channel = Channel::getChannel(target);

        /*  If channel doesn't exist    */ /* Error */
        if (!target_channel) {
       		sendMessage(ERR_NOSUCHCHANNEL(_nick, target));
            return false;
        }
        /*  Broadcast to everybody  */
        target_channel->broadcast(PRIVMSG2(_nick, _user, "localhost", target, cmd.have_last_param), _fd);
    }
    /*  else param is user  */
    else
    {
        /*  If the target user doesn't exist */
        User    *target_user = User::getUser(target, server);

        if (!target_user) {
            sendMessage(ERR_NOSUCHNICKCHANNEL(target));
            return false;
        }
    
        /*  Send the message    */
        target_user->sendMessage(PRIVMSG(_nick, _user, "localhost", target, cmd.have_last_param));
    }
    return true;
}

bool     User::commandUnknown(t_cmd &cmd)
{
	if (cmd.cmd == "CAP")
		return true;
    if (_has_nick)
        sendMessage(ERR_UNKNOWNCOMMAND(_nick, cmd.cmd));
    else
        sendMessage(ERR_UNKNOWNCOMMAND(ft_itoa(_id), cmd.cmd));
    return (false);

}
