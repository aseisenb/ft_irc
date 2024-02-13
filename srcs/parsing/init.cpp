#include "Struct.hpp"

/* This function sets up a basic Internet Relay Chat server. */
bool	initialize(int &port, t_data &data)
{
	int	sock_opt_val = 1;

	/* 1. Creatiang a socket 
	(address family AF_INET and socket type SOCK_STREAM for a TCP socket)*/
	data.socket.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (data.socket.fd < 0)
		return error("socket() error");

	/* 2. Setting up socket address structure
	with the specified address family, port, and IP address 
	(INADDR_ANY means that the server will listen on all available interfaces) */
	data.socket.address = sockaddr_in();
	data.socket.address.sin_family = AF_INET;
	data.socket.address.sin_port = htons(port);
	data.socket.address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* 3. Setting socket options to reuse the address 
	(option SO_REUSEADDR is allowing the reuse of the local address. If fails, socket is closing */
	if (setsockopt(data.socket.fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(sock_opt_val)) < 0)
		return close(data.socket.fd), error("setsockopt() error");
	
	/* 4. Binding the socket to the specified address and port */
	if (bind(data.socket.fd, (struct sockaddr *)(&data.socket.address), sizeof(data.socket.address)) < 0)
		return close(data.socket.fd), error("Port already in use. Use \"ps -aux\" to find an unused port");
	
	/* 5. Listening for incoming connections */
	if (listen(data.socket.fd, MAX_CNCTS) < 0)
		return close(data.socket.fd), error("listen() error");
	/* 6. Creating an epoll instance */
	data.epoll.fd = epoll_create1(EPOLL_CLOEXEC);
	if (data.epoll.fd < 0)
		return close(data.socket.fd), error("epoll_create() error");

	/* 7. Adding the socket to epoll for monitoring read events (EPOLLIN) */
	data.socket.events.data.fd = data.socket.fd;
	data.socket.events.events = EPOLLIN;
	if (epoll_ctl(data.epoll.fd, EPOLL_CTL_ADD, data.socket.fd, &data.socket.events) < 0)
		return close(data.epoll.fd), close(data.socket.fd), error("epoll_ctl() fail");
	return true;
}
