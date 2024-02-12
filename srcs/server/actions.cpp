#include "Struct.hpp"
#include "User.hpp"
#include "Macro.hpp"

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
	fd_new_con = accept(data.socket.fd, (struct sockaddr *)&socket_new_con, &size_socket_new_con);
	if (fd_new_con < 0)
		clear_exit_data(data, "accept() failed", 1);
	new_user = new User(user_id, fd_new_con);
	data.users.insert(make_pair<int, User *>(fd_new_con, new_user));
	data.open_fd.push_back(fd_new_con);
	epoll_event_new_con.events = EPOLLIN | EPOLLRDHUP;
	epoll_event_new_con.data.fd = fd_new_con;
	fcntl(fd_new_con, F_SETFL, O_NONBLOCK); //Imposed by the subject
	if (epoll_ctl(data.epoll.fd, EPOLL_CTL_ADD, fd_new_con, &epoll_event_new_con) < 0)
		clear_exit_data(data, "epoll_ctl() failed", 1);
	cout << "\033[0;" << 31 + user_id % 7 << "m" << "User " << user_id << " connected :)" << endl;
	++user_id;
}

static void	user_disconnection(t_data &data, int fd)
{
	int						id_disc_user;
	User					*disc_user;
	vector<int>::iterator	it;
	vector<int>::iterator	ite;

	epoll_ctl(data.epoll.fd, EPOLL_CTL_DEL, fd, &data.socket.events);
	close(fd);
	try
	{
		disc_user = data.users.at(fd);
	}
	catch (out_of_range)
	{
		cerr << "Couldn't disconnect User with fd " << fd << "; there is no such user" << endl;
		return ;
	}
	id_disc_user = disc_user->getId();
	data.users.erase(fd);
	delete disc_user;
	it = data.open_fd.begin();
	ite = data.open_fd.end();
	for (; it != ite; it++)
	{
		if (*it == fd)
		{
			data.open_fd.erase(it);
			break;
		}
	}
	cout << "\033[0;" << 31 + id_disc_user % 7 << "m" << "User " << id_disc_user << " disconnected :(" << endl;
}

/*
	executeCommands() will execute all the commands in the list of the user
*/
void	executeCommands(t_cmd &cmd, User *user)
{
	while (user->cmds.empty() == false)
	{
		cmd = user->cmds.front();
		user->cmds.pop_front(); 
		bool    result;
		int     quit_fd;

		if (cmd.cmd == "PASS") {
			if (user->commandPASS(cmd) == false)
				user_disconnection(*g_data_ptr, user->getFd());
		} else if (cmd.cmd == "NICK") {
			result = user->commandNICK(cmd);
		} else if (cmd.cmd == "USER") {
			result = user->USER(cmd);
		} else if (cmd.cmd == "PING") {
			result = user->commandPING(cmd);
		} else if (cmd.cmd == "JOIN") {
			result = user->commandJOIN(cmd);
		} else if (cmd.cmd == "NAMES") {
			result = user->NAMES(cmd);
		} else if (cmd.cmd == "PRIVMSG") {
			result = user->commandPRIVMSG(cmd);
		} else if (cmd.cmd == "QUIT") {
			quit_fd = user->commandQUIT(cmd);
			user_disconnection(*g_data_ptr, quit_fd);
			break;
		} else if (cmd.cmd == "KILL" || cmd.cmd == "kill") {
			quit_fd = user->commandKILL(cmd);
			if (quit_fd != -1)
				user_disconnection(*g_data_ptr, quit_fd);
		} else if (cmd.cmd == "PART") {
			result = user->PART(cmd);
		} else if (cmd.cmd == "KICK") {
			result = user->KICK(cmd);
		} else if (cmd.cmd == "OPER") {
			result = user->OPER(cmd);
		} else if (cmd.cmd == "MODE") {
			result = user->commandMODE(cmd);
		} else if (cmd.cmd == "TOPIC") {
			result = user->TOPIC(cmd);
		} else if (cmd.cmd == "NOTICE") {
			result = user->NOTICE(cmd);
		} else if (cmd.cmd == "INVITE") {
			result = user->commandINVITE(cmd);
		} else {
			user->commandUnknown(cmd);
		}
	}
}

static void	user_command(int user_fd, t_data &data)
{
	string	raw_input = read_input(user_fd);
	string	current;
	t_cmd cmd;
	t_users::iterator it, ite;
	vector<string>::iterator it_param, ite_param;
	string	tmp;
	
	while (raw_input.empty() == false)
	{
		current = raw_input.substr(0, raw_input.find_first_of("\n"));
		cmd = parse_input(current);
		if (raw_input.size() >= current.size() + 1)
			raw_input = raw_input.substr(current.size() + 1, raw_input.size());
		else
			raw_input.clear();
		data.users[user_fd]->pushCommand(cmd);
	}
	if (cmd.cmd.size() == 0) {
        data.users[user_fd]->sendMessage("Where is my cmd, frere?!\r\n");
        return ;
    }
	executeCommands(cmd, data.users[user_fd]);
}

/*
	Server Actions

	Perform connection, deconnection and execution of commands issued by users
	
	Input:
		t_data	&data: reference to the structure containing all the variables
		int		i: index of the "for" loop in the "main.cpp" file. Represents one fd but ISN'T one
	
*/
void	server_actions(t_data &data, int i)
{
	const int user_fd = find_user_fd(data.epoll.events[i].data.fd, data);

	if (data.epoll.events[i].data.fd == data.socket.fd)				//Check if the user 
		user_connection(data);										//Connect a user
	if ((data.epoll.events[i].events & EPOLLERR)
		|| (data.epoll.events[i].events & EPOLLHUP)
		|| (data.epoll.events[i].events & EPOLLRDHUP)
		|| !(data.epoll.events[i].events & EPOLLIN))				//Check if the user has shutdown the connection
		user_disconnection(data, data.epoll.events[i].data.fd);		//Disconnect a user
	else if (user_fd != -1 && data.users[user_fd]->getId() != -1) 	//Issue a cmd if the user hasn't been disconnected
		user_command(user_fd, data);
} 
