#include "User.hpp"
#include "Channel.hpp"
#include "Macro.hpp"
#include <sstream>
#include <ctype.h>

/* Command Pass sends a password to the server */
bool	User::commandPASS(t_cmd &cmd)
{
    /* 1. Check if the user already has a password. If true sends an error message
    and returns true to signify that the command has been processed */
    if (_has_password)
        sendMessage(ERR_ALREADYREGISTERED(ft_itoa(_id)));
    // 2. Check for sufficient number of parameters
    else if (cmd.parameters.size() == 0)
        sendMessage(ERR_NEEDMOREPARAMS(ft_itoa(_id), "PASS"));
    // 3. Checks if the provided password matches the server's password
    else if (cmd.parameters.front() == server->password)
        _has_password = true;
    // 4. Checks if the provided password does not match the server's password
    else if (cmd.parameters.front() != server->password)
        sendMessage(ERR_PASSWDMISMATCH(ft_itoa(_id)));
    else
        sendMessage("fatal");
    return (true);
}

/* Command NICK gives the client a nickname or can change the previous one */
bool	User::commandNICK(t_cmd &cmd)
{
    // 1. Check if the user already has a password
	if (!_has_password) {
		sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
		return false;
	}
    /* 2. Check if the nickname was given.
    If the user already has a nickname, it includes the current nickname in the error message */
    if (cmd.parameters.size() == 0){
        if (_has_nick == 1) {
            sendMessage(ERR_NONICKNAMEGIVEN(_nick));
        } else {
            sendMessage(ERR_NONICKNAMEGIVEN(ft_itoa(this->_id)));
        }
        return false;
    }

    // 3. Check the proper format of the nickname
    // Retrieve the first character of the proposed nickname
    std::string param = cmd.parameters.front();
    char        firstChar = param[0];

    // Check if the 1st char is a letter or one of the specified special characters 
    if (!(isalpha(firstChar) || firstChar == '[' || firstChar == ']' || firstChar == '{' || firstChar == '}' ||  firstChar == '\\' || firstChar == '|'))
    {
        if (_has_nick == 1) {
            sendMessage(ERR_ERRONEUSNICKNAME(_nick, param));
        } else {
            sendMessage(ERR_ERRONEUSNICKNAME(ft_itoa(this->_id), param));
        }
        return false;
    }

    // //  Check consecutive
    // for (size_t i = 1; i < param.length(); ++i)
    // {
    //     char ch = param[i];
    //     if (!(isalnum(ch) || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '\\' || ch == '|')) {
    //         sendMessage(ERR_ERRONEUSNICKNAME(ft_itoa(this->_id), param));
    //         return false;
    //     }
    // }

    // 4. Check if the nick already exists. If it does append an underscore to the nickname
    for (size_t i = 0; i < g_data_ptr->open_fd.size(); i++)
    {
        if (g_data_ptr->users[g_data_ptr->open_fd[i]]->getNick() == param) {
            if (g_data_ptr->open_fd[i] != this->_fd) {
                param = param + "_";
            }
        }
    }

    // 5. Inform others about the change of name
    if (this->_has_nick)
    {
        for (size_t i = 0; i < g_data_ptr->open_fd.size(); i++)
        {
            User    *user = g_data_ptr->users[g_data_ptr->open_fd[i]];
            if (user->getIdentification() == true)
                // If the user already had a nickname, it sends a message to all users notifying them about the change
                user->sendMessage(NICK(_nick, _user, "localhost", param));
        }
    }

    // 6. Assign the nickname to the user
    this->_nick = param;
    _has_nick = true;
    // if the user has a username, password and is not identified, sets _is_identified to true and sends a welcome message
    if (this->_has_user && this->_has_password && !this->_is_identified)
    {
        this->_is_identified = true;
        this->sendMessage(RPL_WELCOME(_nick, _user, ft_itoa(g_data_ptr->port), "localhost"));
    }
    return true;
}

/* specify the username and realname of a new user */
bool	User::commandUSER(t_cmd &cmd)
{
	// 1. Check if the user has a password
	if (!_has_password) {
        if (_has_nick == 1) {
		    sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
        return false;
	}
    // 2. Check if there are parameters provided
	if (cmd.parameters.size() == 0) {
        if (_has_nick == 1) {
            sendMessage(ERR_NEEDMOREPARAMS(_nick, "USER"));
        } else {
    	    sendMessage(ERR_NEEDMOREPARAMS(ft_itoa(_id), "USER"));
        }
        return false;
	}
    // 3. Check if the user is already registered
	if (_has_user) {
        if (_has_nick == 1) {
            sendMessage(ERR_ALREADYREGISTERED(_nick));
        } else {
    	    sendMessage(ERR_ALREADYREGISTERED(ft_itoa(_id)));
        }
        return false;
	}

    // 4. Set the username
	this->_user = cmd.parameters.front();
	this->_name = cmd.have_last_param;
	_has_user = true;
    // 5. Check for identification and send welcome message
	if (_has_user && _has_nick && !_is_identified)
	{
		this->_is_identified = true;
		sendMessage(RPL_WELCOME(_nick, _user, ft_itoa(g_data_ptr->port), "localhost"));
	} 
	return true;
}

/* PING command is used to check the connectivity between the client and the server.
It checks whether the user is identified and sends an appropriate PING or PONG message */
bool	User::commandPING(t_cmd &cmd)
{
	/* 1. Check whether the user is identified.
    For an unidentified user, check if PING command has parameters. If no parameters, 
    send a PING message to the user with the user's ID. If parameters are provided, it includes 
    the parameters in the PING message.
    */
    if (_is_identified == false) {
        if (cmd.parameters.size() == 0) {
            sendMessage(PING(ft_itoa(_id), ""));
        } else {
            sendMessage(PING(ft_itoa(_id), cmd.parameters.front()));
        }
    } 
    /* 2. Process PONG for Identified User
    Constructs the user ID using _nick, _user, and "localhost" as the parameters for user_id(), 
    and include the parameters from the PING command in the PONG message if they exist */
    else {
        if (cmd.parameters.size() == 0) {
            sendMessage(PONG(user_id(_nick, _user, "localhost"), ""));
        } else {
    		sendMessage(PONG(user_id(_nick, _user, "localhost"), cmd.parameters.front()));
        }
    }
    return true;
}

/* OPER command used to obtain operator privileges. It checks for user identification, number of parameters and the correctness of the LOGIN and PASSWORD parameters */
bool	User::commandOPER(t_cmd &cmd)
{
	/* 1. Check User Identification
    If the user is not identified, send an error message no privileges (ERR_NOPRIVILEGES) */
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
    // 2. Check Command Parameters
	if (cmd.parameters.size() < 2) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "OPER"));
		return false;
	}

	// 3. Check if login and password are correct one
	if (cmd.parameters.front() != LOGIN || cmd.parameters.back() != PASSWORD) {
		sendMessage(ERR_PASSWDMISMATCH(_nick));
		return false;
	}

    // 4. Grant Operator Privileges if all the checks pass
	sendMessage(RPL_YOUREOPER(_nick)); // send a message that the user is now an operator
    server->operator_fd.push_back(_fd); // add the user's fd to the list of operator file descriptors
	return true;
}

/* Remove the client from the server */
int		User::commandKILL(t_cmd &cmd)
{
    User    *target_user;
    string  target_nick;

    // 1. Check user identification
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
    // 2. Check command parameters
	if (cmd.parameters.size() < 1) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "KILL"));
		return -1;
	}
    // 3. Check operator privileges:
    if (isOperator() == false) {
        sendMessage(ERR_NOPRIVILEGES(_nick));
        return -1;
    }

    // 4. Get targeted user:
    target_nick = cmd.parameters.front(); // extract target user's nickname
    target_user = User::getUser(target_nick, server); // get the user object
    if (target_user == NULL) {
        sendMessage(ERR_NOSUCHNICKCHANNEL(target_nick)); // error message in user doesn't exist
        return -1;
    }

    // 5. Kick target user from all channels
    vector<Channel *>::iterator iter_beg = _channels.begin();
    vector<Channel *>::iterator iter_end = _channels.end();
    while (iter_beg != iter_end)
    {
        Channel *myChannel = *iter_beg;

        myChannel->kick_user(target_user->getFd()); // kick the target user from each channel
        target_user->deleteChannel(myChannel->get_name());
        /* Send quit message to each channel to notify other users about target user leaving the channel. 
        Plus check if the message should include add info */
        if (myChannel->get_users().size() != 0)
        {
            if (cmd.last_param == 1) {
                myChannel->broadcast(QUIT2(user_id(target_nick, target_user->getUser(), "localhost"), cmd.have_last_param), -1);
            } else {
                myChannel->broadcast(QUIT2(user_id(target_nick, target_user->getUser(), "localhost"), "default reason"), -1);
            }
        } else {   // if no more users left in the channel delete the channel
            this->server->channels.erase(myChannel->get_name());
            delete myChannel;
        }
        iter_beg++;
    }

    // 6. Notify the target user that he was deleted from the channel
    if (cmd.last_param == true) {
        target_user->sendMessage(KILL(_nick, cmd.have_last_param));
    } else {
        target_user->sendMessage(KILL(_nick, "default reason"));
    }
    return (target_user->getFd());
}

/* Command that allows you to quit the server */
int     User::commandQUIT(t_cmd &cmd)
{
    // 1. Go through the channels and remove the target user
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

/* Command msg sends private messages between users
 * 
 * @param   <receiver>{,<receiver>} <text to be sent> 
 * @example PRIVMSG Angel :yes I'm receiving it !       ; cmd to send a message to Angel.
 * @example PRIVMSG %#bunny :Hi! I have a problem!      ; cmd to send a message to halfops and chanops on #bunny
 */
bool	User::commandPRIVMSG(t_cmd &cmd)
{
    // 1. Check user identification
    if (_is_identified == false) {
        if (_has_nick == 1) {
    		sendMessage(ERR_NOPRIVILEGES(_nick));
        } else {
            sendMessage(ERR_NOPRIVILEGES(ft_itoa(_id)));
        }
    	return false;
	}
    // 2. Check command parameters
	if (cmd.parameters.size() < 1 || (cmd.parameters.size() == 1 && cmd.have_last_param.size() == 0)) {
		sendMessage(ERR_NEEDMOREPARAMS(_nick, "PRIVMSG"));
		return false;
	}

    // 3. Extract the target (channel or user) from the command parameters
    std::string target = cmd.parameters.front();

    // 4. If it's channel than process the message (# means it's a channel)
    if (target[0] == '#')
    {
        Channel *target_channel = Channel::getChannel(target);

        //  If channel doesn't exist send an error
        if (!target_channel) {
       		sendMessage(ERR_NOSUCHCHANNEL(_nick, target));
            return false;
        }
        //  If the use is not in the channel send an error
        if (target_channel->is_user(_fd) == false) {
            sendMessage(ERR_NOTONCHANNEL(target, _nick));
            return false;
        }
        // if everythin ok broadcasts the PRIVMSG2 message to the channel
        target_channel->broadcast(PRIVMSG2(_nick, _user, "localhost", target, cmd.have_last_param), _fd);
    }
    // 4. If it's user than process the message
    else
    {
        // Get the corresponding user object
        User    *target_user = User::getUser(target, server);
        // If the target user doesn't exist send an error
        if (!target_user) {
            sendMessage(ERR_NOSUCHNICKCHANNEL(target));
            return false;
        }
        // If user exists sends the PRIVMSG message to him
        target_user->sendMessage(PRIVMSG(_nick, _user, "localhost", target, cmd.have_last_param));
    }
    return true;
}

/* Command Notice is similar to PRIVMSG but typically used for server notifications or information. It checks user identification, 
the number of parameters, and whether the target is a channel or user. Depending on the target, it broadcasts the notice to the channel or sends the notice to the user. If any check fails, it sends an appropriate error message and returns false.
*/
bool	User::commandNOTICE(t_cmd &cmd)
{
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

    if (target[0] == '#')
    {
        Channel *target_channel = Channel::getChannel(target);

        if (!target_channel) {
       		sendMessage(ERR_NOSUCHCHANNEL(_nick, target));
            return false;
        }
        target_channel->broadcast(PRIVMSG2(_nick, _user, "localhost", target, cmd.have_last_param), _fd);
    }
    else
    {
        User    *target_user = User::getUser(target, server);

        if (!target_user) {
            sendMessage(ERR_NOSUCHNICKCHANNEL(target));
            return false;
        }
        target_user->sendMessage(PRIVMSG(_nick, _user, "localhost", target, cmd.have_last_param));
    }
    return true;
}

/* Handles unknown commands. If the command is "CAP," it returns true, meaning that this command is recognized and handled elsewhere. If the command is not 
"CAP," it sends an error message meaning an unknown command and returns false */
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
