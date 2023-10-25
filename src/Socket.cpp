#include <Logger.hpp>
#include <Socket.hpp>
#include <SystemException.hpp>

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>

Socket::Socket(const int fd)
	: _addr_len(sizeof(_addr)),
	  _fd(accept(fd, (t_sockaddr *)&_addr, &_addr_len))
{
	Logger &logger = Logger::getInstance();

	if (getFD() == SYSTEM_ERROR)
		throw SystemException("Accept");
	assert(fcntl(getFD(), F_SETFL, O_NONBLOCK) != SYSTEM_ERROR);
	char address[INET_ADDRSTRLEN];
	assert(inet_ntop(AF_INET, &_addr.sin_addr, address, sizeof(address)) !=
		   NULL);
	logger.log(INFO,
			   "Connection received from " + std::string(address) +
				   " created client socket on fd: " + std::to_string(getFD()));
}

Socket::Socket() : _fd(socket(AF_INET, SOCK_STREAM, 0))
{
	Logger &logger = Logger::getInstance();

	if (getFD() == SYSTEM_ERROR)
		throw SystemException("socket creation failed");
	assert(fcntl(getFD(), F_SETFL, O_NONBLOCK) != SYSTEM_ERROR);
	logger.log(DEBUG,
			   "Created server socket on fd: " + std::to_string(getFD()));
}

Socket::~Socket()
{
	assert(close(getFD()) != SYSTEM_ERROR);
}

void Socket::initSockaddrIn(t_sockaddr_in &addr, const std::string &_port)
{
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(std::stoi(_port));
	std::fill_n(addr.sin_zero, sizeof(addr.sin_zero), '\0');
}

void Socket::setupServer(const std::string &port)
{
	int option = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) ==
		SYSTEM_ERROR)
		throw SystemException("setsockopt failed");
	initSockaddrIn(_addr, port);
	if (bind(getFD(), (t_sockaddr *)&_addr, sizeof(t_sockaddr_in)) ==
		SYSTEM_ERROR)
		throw SystemException("Bind");
	if (listen(getFD(), MAX_PENDING_CONNECTIONS) == SYSTEM_ERROR)
		throw SystemException("Listen");
}

int Socket::getFD() const
{
	return (_fd);
}
