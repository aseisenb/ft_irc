#include "Struct.hpp"

bool	initialize(int &port, t_data &data)
{
	int	sock_opt_val = 1;

	data.socket.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (data.socket.fd < 0)
		return error("socket() error");
	data.socket.address = sockaddr_in();
	data.socket.address.sin_family = AF_INET;
	data.socket.address.sin_port = htons(port);
	data.socket.address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(data.socket.fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(sock_opt_val)) < 0)
		return close(data.socket.fd), error("setsockopt() error");
	if (bind(data.socket.fd, (struct sockaddr *)(&data.socket.address), sizeof(data.socket.address)) < 0)
		return close(data.socket.fd), error("Port already in use. Use \"ps -aux\" to find an unused port");
	if (listen(data.socket.fd, MAX_CNCTS) < 0)
		return close(data.socket.fd), error("listen() error");
	data.epoll.fd = epoll_create1(EPOLL_CLOEXEC);
	if (data.epoll.fd < 0)
		return close(data.socket.fd), error("epoll_create() error");
	data.socket.events.data.fd = data.socket.fd;
	data.socket.events.events = EPOLLIN;
	if (epoll_ctl(data.epoll.fd, EPOLL_CTL_ADD, data.socket.fd, &data.socket.events) < 0)
		return close(data.epoll.fd), close(data.socket.fd), error("epoll_ctl() fail");
	return true;
}
