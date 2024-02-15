#include "Struct.hpp"
#include "User.hpp"
#include "Macro.hpp"

/* This functions accepts a new connection, creates a User object, 
configures epoll for monitoring and updates data structures */
static void	user_connection(t_data &data)
{
	int					fd_new_con;
	struct	sockaddr_in socket_new_con;
	struct	epoll_event epoll_event_new_con;
	socklen_t			size_socket_new_con;
	User				*new_user;
	static int			user_id = 1;

	size_socket_new_con = sizeof(socket_new_con);
	socket_new_con = sockaddr_in();
	epoll_event_new_con = epoll_event();

	// 1. Accepting new connection from a client on the server's listening socket (data.socket.fd)
	fd_new_con = accept(data.socket.fd, (struct sockaddr *)&socket_new_con, &size_socket_new_con); // established connection
	if (fd_new_con < 0)
		clear_exit_data(data, "accept() failed", 1);
	
	// 2. Creating a new user object with a unique user_id and the file descriptor for the new connection
	new_user = new User(user_id, fd_new_con);
	
	// 3. Adding new User to data structures (to a map data.users using fd as the key)
	data.users.insert(make_pair<int, User *>(fd_new_con, new_user));
	data.open_fd.push_back(fd_new_con); // the file descriptor is added to a list
	
	// 4. Configuring epoll for new connection
	epoll_event_new_con.events = EPOLLIN | EPOLLRDHUP; // incoming data (EPOLLIN) and connection closure (EPOLLRDHUP)
	epoll_event_new_con.data.fd = fd_new_con;
	fcntl(fd_new_con, F_SETFL, O_NONBLOCK); // sets the new file descriptor to non-blocking mode
	
	// 5. Adding new connection(new fd) to epoll(data.epoll.fd)
	if (epoll_ctl(data.epoll.fd, EPOLL_CTL_ADD, fd_new_con, &epoll_event_new_con) < 0)
		clear_exit_data(data, "epoll_ctl() failed", 1);
	
	// 6. Printing connection message:
	cout << "\033[0;" << 31 + user_id % 7 << "m" << "User " << user_id << " connected" << endl;
	++user_id; // 	// ensuring each user gets a unique id
}

/* This function handles user disconnection */
static void	user_disconnection(t_data &data, int fd)
{
	int						id_disc_user;
	User					*disc_user;
	vector<int>::iterator	it;
	vector<int>::iterator	ite;

    // 1. Removing fd from epoll monitoring
	epoll_ctl(data.epoll.fd, EPOLL_CTL_DEL, fd, &data.socket.events);
	close(fd); // closing the socket associated with fd
	// 2. Finding the user associated with the given fd in the data.users map
	try
	{
		disc_user = data.users.at(fd);
	}
	catch (out_of_range)
	{
		cerr << "Error: failed disconnection of User with fd " << fd << ". Reason: there is no such user" << endl;
		return ;
	}
	// 3. Cleaning
	id_disc_user = disc_user->getId(); // getting the ID of the disconnected user
	data.users.erase(fd); // removing the user from the data structures
	delete disc_user; // deleting the User object
	
	// 4. Removing fd from the list of open fd
	it = data.open_fd.begin();
	ite = data.open_fd.end();
	// 5. Iterating over the list of open fd to find and remove the disconnected fd
	for (; it != ite; it++)
	{
		if (*it == fd)
		{
			data.open_fd.erase(it);
			break;
		}
	}
	// 6. Printing a message indicating that a user has disconnected
	cout << "\033[0;" << 31 + id_disc_user % 7 << "m" << "User " << id_disc_user << " disconnected" << endl;
}

/* This is the command handler, it executes all the commands */
void	commandsHandler(t_cmd &cmd, User *user)
{
	// 1. Loop that continues as long as the user's command queue is not empty
	while (user->cmds.empty() == false)
	{
		// 2. Retrieving the first command from the queue, assigning it to the command variable and removing it from the queue
		cmd = user->cmds.front();
		user->cmds.pop_front(); 
		bool    result;
		int     quit_fd;

		// 3. Command execution
		if (cmd.cmd == "PASS") {
			if (user->commandPASS(cmd) == false)
				user_disconnection(*g_data_ptr, user->getFd());
		} else if (cmd.cmd == "NICK") {
			result = user->commandNICK(cmd);
		} else if (cmd.cmd == "USER") {
			result = user->commandUSER(cmd);
		} else if (cmd.cmd == "PING") {
			result = user->commandPING(cmd);
		} else if (cmd.cmd == "JOIN") {
			result = user->commandJOIN(cmd);
		} else if (cmd.cmd == "NAMES") {
			result = user->commandNAMES(cmd);
		} else if (cmd.cmd == "PRIVMSG") {
			result = user->commandPRIVMSG(cmd);
		}else if (cmd.cmd == "PART") {
			result = user->commandPART(cmd);
		} else if (cmd.cmd == "KICK") {
			result = user->commandKICK(cmd);
		} else if (cmd.cmd == "OPER") {
			result = user->commandOPER(cmd);
		} else if (cmd.cmd == "MODE") {
			result = user->commandMODE(cmd);
		} else if (cmd.cmd == "TOPIC") {
			result = user->commandTOPIC(cmd);
		} else if (cmd.cmd == "NOTICE") {
			result = user->commandNOTICE(cmd);
		} else if (cmd.cmd == "INVITE") {
			result = user->commandINVITE(cmd);
		}else if (cmd.cmd == "QUIT") {
			quit_fd = user->commandQUIT(cmd);
			user_disconnection(*g_data_ptr, quit_fd);
			break;
		}else if (cmd.cmd == "KILL" || cmd.cmd == "kill") {
			quit_fd = user->commandKILL(cmd);
			if (quit_fd != -1)
				user_disconnection(*g_data_ptr, quit_fd);
		} else {
			user->commandUnknown(cmd);
		}
	}
}

/* This function reads raw input from a user, processes, parses and executes multiple commands */
static void	user_command(int user_fd, t_data &data)
{
	// 1. Reading raw input from the user associated with the file descriptor user_fd and storing it in the raw_input string
	string	raw_input = read_input(user_fd);
	string	current;
	t_cmd cmd;
	t_users::iterator it, ite;
	vector<string>::iterator it_param, ite_param;
	string	tmp;
	
	// 2. Loop continues as long as raw_input is not empty
	while (raw_input.empty() == false)
	{
		// 1. Extracting the current command from the beginning of raw_input up to the first newline character and stores it in the current string
		current = raw_input.substr(0, raw_input.find_first_of("\n"));
		// 2. Parsing the current command
		cmd = parse_input(current);
		// 3. Adjusting the raw_input string to remove the processed command
		if (raw_input.size() >= current.size() + 1) // If there are characters remaining after the current command and a newline character, updating raw_input to exclude the processed part
			raw_input = raw_input.substr(current.size() + 1, raw_input.size());
		else
			raw_input.clear(); // If there are no remaining characters, clearing raw_input
		// 4. Pushing command to user's command queue
		data.users[user_fd]->pushCommand(cmd);
	}
	// 3. Checking if the parsed command has an empty command string
	if (cmd.cmd.size() == 0) {
        data.users[user_fd]->sendMessage("Error: missing command\r\n");
        return ;
    }
	// 4. Execute nice command 
	commandsHandler(cmd, data.users[user_fd]);
}

/*  This function performs different server related actions based on events detected by epoll. It checks for new connections, disconnections and executes commands */
void	server_actions(t_data &data, int i)
{
	// 1. Finding user file descriptor based on the epoll event's fd
	const int user_fd = find_user_fd(data.epoll.events[i].data.fd, data);

	// 2. Checking for new connection (checks if the event's file descriptor is the same as the server's listening socket (data.socket.fd). If true it means a new user is trying to connect
	if (data.epoll.events[i].data.fd == data.socket.fd)
		user_connection(data); // connect a user
	// 3. Checking if the user wants to disconnect or if the user perfroms errors:
	if ((data.epoll.events[i].events & EPOLLERR)
		|| (data.epoll.events[i].events & EPOLLHUP)
		|| (data.epoll.events[i].events & EPOLLRDHUP)
		|| !(data.epoll.events[i].events & EPOLLIN))
		user_disconnection(data, data.epoll.events[i].data.fd);// disconnect the user
	// 4. If the user hasn't been disconnected check for command issuing
	else if (user_fd != -1 && data.users[user_fd]->getId() != -1)
		user_command(user_fd, data);
} 
