#include "net.cpp"
#include <iostream>
#include <thread>
#include <unistd.h>

void timer() {
	while (true) {sleep(1);}
}

void messageHandler(int accepted, char* buffer) {
	// server response any message
	std::string clientMessage = std::string(buffer);
	if (clientMessage == "help") {
		char buf[] = "help - get command list\nversion - get server time\n";
		send_net(accepted, buf, sizeof(buf));
	} else if (clientMessage == "version") {
		char buf[] = "Server version: 1.6 (No encrypted)\n";
		send_net(accepted, buf, sizeof(buf));
	} else {
		char buf[] = "Unknown command\n";
		send_net(accepted, buf, sizeof(buf));
	}
}

void messageCatcher(int accepted, bool firstConnect) {
	if (firstConnect) { // welcome message
		char welcomeMessage[] = "Welcome from server! Use help for get command list\n";
		send_net(accepted, welcomeMessage, sizeof(welcomeMessage));
	}
	char buffer[1024] = "";
	int messageBytes = recv_net(accepted, buffer, sizeof(buffer));
	if (buffer[0] == 0) { // if client exit
		std::cout << "[D" << accepted << "] " << "Client disconnected\n";
		close_net(accepted);
		return;
	}
	// if client online
	std::cout << "[D" << accepted << "] " << buffer << '\n';
	messageHandler(accepted, buffer);
	std::thread messageHandler_thread(messageCatcher, accepted, false);
	messageHandler_thread.detach();
}

void acceptorOfNewConnections(int listener) {
	sockaddr* addr;
	int* addrLen = NULL;
	int accepted = accept_net(listener);
	std::cout << "[D" << accepted << "] " << "Client connected! \n";
	// Forward this client to message catcher
	std::thread messageHandler_thread(messageCatcher, accepted, true);
	messageHandler_thread.detach();
	// Now continue listen new connections
	std::thread accept_thread(acceptorOfNewConnections, listener);
	accept_thread.detach();
}

int main() {
	int listener = listen_net("1597"); // start server on port 1597
	std::thread accept_thread(acceptorOfNewConnections, listener); // start root thread
	accept_thread.detach();
	timer();
}