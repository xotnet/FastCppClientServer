#include "net.cpp"
#include <iostream>
#include <thread>
#include <unistd.h>
#include "BigInt.hpp"
#include <random>
void timer() {
	while (true) {sleep(1);}
}


struct cryptoMethods {
	BigInt sharedKey = std::string("0");
	
	int genRandom(int from, int upto) {
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<> dist(from, upto);
		return dist(gen);
	}

	BigInt genSharedKey(int accepted) {
		BigInt generator = 2;
		BigInt mod = std::string("42571564251765425471355154626165452431571334621325420624815730163446612426143167826542156263");
		std::cout << "generating and exchanging public key...\n";
		int exponent = genRandom(11, 999999);
		int expCopy = exponent;
		BigInt res = 1;
		while (exponent > 0) {
			if (exponent % 2 == 1) {
				res = (res * generator) % mod;
			}

			generator = (generator * generator) % mod;
			exponent /= 2;
		}
		char resChar[res.to_string().length()+1] = "";
		char friChar[500] = "";
		strcpy(resChar, res.to_string().c_str());
		send_net(accepted, resChar, sizeof(resChar));
		BigInt friendsKey(0);
		recv_net(accepted, friChar, sizeof(friChar));
		std::cout << "generating session key...\n";
		BigInt result = 1;
		friendsKey = friChar;
		while (expCopy > 0) {
			if (expCopy % 2 == 1) {
				result = (result * friendsKey) % mod;
			}

			friendsKey = (friendsKey * friendsKey) % mod;
			expCopy /= 2;
		}
		return result;
	}
};


void messageHandler(int accepted, char* buffer) {
	// server response any message
	std::string clientMessage = std::string(buffer);
	if (clientMessage == "help") {
		char buf[] = "help - get command list\nversion - get server time\n";
		send_net(accepted, buf, sizeof(buf));
	} else if (clientMessage == "version") {
		char buf[] = "Server version: 1.7 (With test encryption)\n";
		send_net(accepted, buf, sizeof(buf));
	} else {
		char buf[] = "Unknown command\n";
		send_net(accepted, buf, sizeof(buf));
	}
}

void messageCatcher(int accepted, bool firstConnect) {
	// Encryption
	cryptoMethods crm;
	crm.sharedKey = crm.genSharedKey(accepted);
	std::cout << crm.sharedKey.to_string();
	
	
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