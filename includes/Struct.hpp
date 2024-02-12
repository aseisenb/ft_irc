#ifndef STRUCT_HPP
# define STRUCT_HPP
# include <iostream>
# include <string>
# include <map>
# include <list>
# include <vector>
# include <algorithm>
# include <errno.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <signal.h>
# include <sstream>
# include <ctime>
# include <cstdlib>
# define MAX_CNCTS 16
# define LOCAL_HOST "127.0.0.1"
# define READ_SIZE	512 			//Max message size. 510 "usable characters". 2 last ones MUST be \r\n
# define LOGIN "login"
# define PASSWORD "password"

class User;
class Channel;

using namespace std;


typedef map<int, User*> 		t_users; 	//Users will be mapped key: fd; User*: User corresponding to the fd
typedef map<string, Channel *>	t_channels; //Key: channel name; Channel*: corresponding Channel

typedef struct s_cmd
{
	string				prefix;
	string				cmd;
	vector<string>		parameters;
	string				have_last_param;
	bool				last_param;
}						t_cmd;

typedef struct s_socket
{
	int					fd;
	epoll_event			events;
	struct sockaddr_in	address;
}						t_socket;

typedef struct s_epoll
{
	int					fd;
	epoll_event			events[MAX_CNCTS];
}						t_epoll;

typedef struct s_data
{
        t_epoll			epoll;
        t_socket		socket;
        int				port;
        string			password;
        t_users			users;
        vector<int>		open_fd;
		vector<int>		operator_fd;
        t_channels		channels;
}						t_data;

extern t_data	*g_data_ptr;

/* INITIALIAZATION */
bool		initialize(int &port, t_data &data);

/* PARSING */
bool		parsing(char **argv, int &port, string &password);
t_cmd		parse_input(string raw_input);
string		read_input(int	user_fd);

/* SERVER */
void		server_actions(t_data &data, int i);

void		clear_data(t_data &data);
void		clear_exit_data(t_data &data, string message, int err_code);

/* UTILS */
int			find_user_fd(int fd, t_data &data);
string		ft_itoa(int n);
int			ft_atoi(string &number);
string		delete_spaces(const string &to_delete);
bool		error(const string error_msg);

#endif