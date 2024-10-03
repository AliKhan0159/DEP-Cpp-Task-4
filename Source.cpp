#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080

// Initialize Winsock
void initialize_winsock() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		exit(EXIT_FAILURE);
	}
}

// Clean up Winsock
void cleanup_winsock() {
	WSACleanup();
}

int create_server_socket() {
	initialize_winsock();

	int server_fd;
	struct sockaddr_in address;

	// Create server socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
		cleanup_winsock();
		exit(EXIT_FAILURE);
	}

	// Define the address structure
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind the socket to the address and port
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		closesocket(server_fd);
		cleanup_winsock();
		exit(EXIT_FAILURE);
	}

	// Start listening for incoming connections
	if (listen(server_fd, 3) == SOCKET_ERROR) {
		std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(server_fd);
		cleanup_winsock();
		exit(EXIT_FAILURE);
	}

	return server_fd;
}

// Function to handle client requests
void handle_client(SOCKET client_socket) {
	char buffer[1024] = { 0 };
	int bytes_read = recv(client_socket, buffer, 1024, 0);

	if (bytes_read > 0) {
		std::cout << "Received request: " << buffer << std::endl;

		// Serve a simple HTTP response
		const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Welcome! Client the Server is Runing!</h1>";
		send(client_socket, response, strlen(response), 0);
	}

	closesocket(client_socket); // Close the client connection
}

int main() {
	int server_fd = create_server_socket();
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	while (true) {
		SOCKET client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
		if (client_socket == INVALID_SOCKET) {
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		// Create a new thread for each client connection
		std::thread client_thread(handle_client, client_socket);
		client_thread.detach();  // Allow thread to run independently
	}

	closesocket(server_fd);
	cleanup_winsock();

	return 0;
}
