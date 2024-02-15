#include "Struct.hpp"
#include "User.hpp"

// SIGINT signal (Ctrl+C)
void	signal_handler(int code)
{
	(void)code;
	clear_exit_data(*g_data_ptr, "SIGINT", 0);
}

int main(int argc, char *argv[])
{
	t_data				data;
	int					epoll_fds;
	struct sigaction	sa;

	// 1. Command line arguments check
	if (argc != 3)
		return (error("Usage: ./ircserver <port> <password>"), EXIT_FAILURE);
	// 2. Parsing command line arguments
	if (!parsing(argv, data.port, data.password))
		return (EXIT_FAILURE);
	// 3. Initializing the server
	if (!initialize(data.port, data))
		return (EXIT_FAILURE);
	// 4. Setting up signal handling
	g_data_ptr = &data;
	sa.sa_handler = signal_handler;
	sa.sa_flags = SA_RESTART; 		
	sigfillset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) < 0)
		clear_exit_data(data, "Sigaction error", 1);
	// 5. Print server info
	cout << "Port: " << data.port << endl << "Password: " << data.password << endl << "I am ready" << endl;
	// 6 Main server loop
	while (true)
	{
		// 7. Waiting for events using epoll
		epoll_fds = epoll_wait(data.epoll.fd, data.epoll.events, MAX_CNCTS, -1);
		if (epoll_fds < 0)
			return (clear_data(data), error("epoll_wait function error"), EXIT_FAILURE);
		// 8. Handling events in a loop
		for (int i = 0; i < epoll_fds; i++)
			central_server(data, i);		
	}
	return (EXIT_SUCCESS);
}
