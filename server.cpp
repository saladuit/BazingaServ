#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void handleClient(int clientSocket)
{
	const char *message = "Hello from my first simple server!\n";
	char buffer[1024] = {0};

	send(clientSocket, message, strlen(message), 0);

	recv(clientSocket, buffer, sizeof(buffer), 0);

	std::cout << "Received from client: " << buffer << std::endl;
	close(clientSocket);
}

int main()
{
	int serverSocket;
	int clientSocket;

	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	socklen_t clientAddrLen = sizeof(clientAddr);

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Error creating socket\n";
		return 1;
	}

	serverAddr.sin_family = AF_INET;

	serverAddr.sin_port = htons(7070); // Choose any available port

	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSocket, (struct sockaddr *)&serverAddr,
			 sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error binding to port\n";
		close(serverSocket);
		return 1;
	}

	if (listen(serverSocket, 5) == -1)
	{
		std::cerr << "Error listening\n";
		close(serverSocket);
		return 1;
	}

	std::cout << "Server listening on port 8080...\n";

	while (true)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
							  &clientAddrLen);
		if (clientSocket == -1)
		{
			std::cerr << "Error accepting connection\n";
			continue;
		}
		handleClient(clientSocket);
	}

	close(serverSocket);
	return 0;
}