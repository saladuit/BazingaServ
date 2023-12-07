#include "CGI.hpp"
#include "Poll.hpp"
#include "Client.hpp"
#include "ClientException.hpp"
#include "Logger.hpp"

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/poll.h>

Client::Client(const int &server_fd) : _socket(server_fd)
{
	_socket.setupClient();
	_state = ClientState::Receiving;
	cgiBodyIsSent = false;
	cgiHasBeenRead = false;
}

Client::~Client()
{
}

int Client::getFD(void) const
{
	return (_socket.getFD());
}

int	*Client::getCgiToServerFd(void) {
	return (_cgiToServerFd);
}

int	*Client::getServerToCgiFd(void) {
	return (_serverToCgiFd);
}

HTTPRequest	&Client::getRequest(void) {
	return (_request);
}

// implement method Martijn for verifying that we are dealing with a CGI
// also get fileExtension from Martijn and save in HTTPRequest class
ClientState Client::handleConnection(short events, Poll &poll, Client &client, 
		std::unordered_map<int, std::shared_ptr<int>> &active_pipes)
{
	Logger &logger = Logger::getInstance();
	logger.log(INFO, "Handling client connection on fd: " +
						 std::to_string(_socket.getFD()));
	try
	{
		if (events & POLLIN && _state == ClientState::Receiving)
		{
			logger.log(ERROR, "ClientState::Receiving");
			_state = _request.receive(_socket.getFD());
			logger.log(DEBUG, "_request_target: " + _request.getRequestTarget());
			logger.log(DEBUG, "_request.getBodyLength(): %", _request.getBodyLength());
			logger.log(DEBUG, "_request.getBody(): %", _request.getBody());
			_request.setCGIToTrue();
			return (_state);
		}
		else if (events & POLLOUT && _state == ClientState::CGI_Start)
		{
			logger.log(ERROR, "ClientState::CGI_Start");
			_state = _cgi.start(poll, client, _request.getBodyLength(), active_pipes);
			return (_state);
		}
		else if (events & POLLOUT && _state == ClientState::CGI_Write)
		{
			logger.log(ERROR, "ClientState::CGI_Write");
			_state = _cgi.send(client, _request.getBody(), _request.getBodyLength());
			return (_state);
		}
		else if (events & POLLIN && _state == ClientState::CGI_Read)
		{
			logger.log(ERROR, "ClientState::CGI_Read");
			_state = _cgi.receive(client);
			if (client.cgiHasBeenRead == true) {
				_state = _file_manager.manageCgi(_request.getHTTPVersion(), _cgi.body);
				logger.log(DEBUG, "response:\n\n" + _file_manager.getResponse());
			}
	
			// logger.log(DEBUG, "cgiBody: " + _cgi.body);
			
			// _state = ClientState::Sending;
			// int status;
			// waitpid(_cgi.getPid(), &status, 0);
			// WEXITSTATUS(status);
			return (_state);
		}
		else if (events & POLLOUT && _state == ClientState::Loading)
		{
			logger.log(ERROR, "ClientState::Loading");
			if (_request.CGITrue() == true) {
				_state = _cgi.parseURIForCGI(_request.getRequestTarget());
				logger.log(DEBUG, "executable: " + _cgi.getExecutable());
				return (_state);
			}
			// if (_request._cgi == true)
			// _state = _cgi.createResponse();
			// else 
			// logger.log(INFO, "request target from loading else if: %", _request.getRequestTarget());
			_state = _file_manager.manage(
				_request.getMethodType(),
				"./data/www" + _request.getRequestTarget(),
				_request.getBody()); // TODO: resolve location
			return (_state);
		}
		// else if (events & POLLOUT && _state == ClientState::CGI_Load)
		// {
		// 	logger.log(DEBUG, "ClientState::CGI_Load");
		// 	_state = _file_manager.manage(
		// 		_request.getMethodType(),
		// 		"./data/www" + _request.getRequestTarget(),
		// 		_cgi.body);
		// 	return (_state);
		// }
		else if (events & POLLOUT && _state == ClientState::Error)
		{
			_state = _file_manager.loadErrorPage();
			return (_state);
		}
		else if (events & POLLOUT && _state == ClientState::Sending)
		{
			logger.log(ERROR, "ClientState::Sending");
			// if CGI
			// _state =
			//	_response.send(_socket.getFD(), _cgi.getResponse());
			// else
			logger.log(DEBUG, "Response: " + _file_manager.getResponse());
			_state =
				_response.send(_socket.getFD(), _file_manager.getResponse());
			logger.log(DEBUG, "request target from handleConnection: %", _request.getRequestTarget());
			return (_state);
		}
	}
	catch (ClientException &e)
	{
		logger.log(ERROR, "Client exception: " + std::string(e.what()));
		_response.clear();
		_response.append(e.what());
		_state = _file_manager.openErrorPage(
			"./data/errors", e.getStatusCode()); // TODO: resolve location
		return (_state);
	}
	return (ClientState::Unkown);
}
