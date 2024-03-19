#include "net.hpp"
#include "misc.hpp"
#include <iostream>
#include <thread>
#include "BigInt.hpp"

struct cryptoMethods {
	BigInt sharedKey = std::string("0");
	BigInt genSharedKey(int accepted) {
		BigInt generator = 2;
		BigInt mod = std::string("42571564251765425471355154626165452431571334621325420624815730163446612426143167826542156263");
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
		// check crypt "protocol" of friChar
		std::string friString = "";
		if (readPiece(friChar, 0, 9) != "EncExchP1") {
			return BigInt(0);
		} else {
			friString = readPiece(friChar, 9);
		}
		BigInt result = 1;
		friendsKey = friString;
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


void messageHandler(int accepted, char* buffer, std::string key) {
	// server response any message
	std::string clientMessage = toStringAndDecrypt(std::string(buffer), readPiece(key, 12, 24));
		std::cout << "[D" << accepted << "] " << clientMessage << '\n';
	if (clientMessage == "help") {
		char buf[] = "help - get command list\nversion - get server time\n";
		encryptMessage(buf, key);
		send_net(accepted, buf, sizeof(buf));
	} else if (clientMessage == "version") {
		char buf[] = "Server version: 1.8 (With encryption)\n";
		encryptMessage(buf, key);
		send_net(accepted, buf, sizeof(buf));
	} else {
		char buf[] = "Unknown command\n";
		encryptMessage(buf, key);
		send_net(accepted, buf, sizeof(buf));
	}
}

void messageCatcher(int accepted, bool firstConnect, cryptoMethods crm) {
	// Encryption
	if (crm.sharedKey == BigInt(0)) {
		crm.sharedKey = crm.genSharedKey(accepted);
	}
	
	if (firstConnect) { // welcome message
		char welcomeMessage[100] = "Welcome from server! Use help for get command list\n";
		encryptMessage(welcomeMessage, crm.sharedKey.to_string());
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
	messageHandler(accepted, buffer, crm.sharedKey.to_string());
	std::thread messageCatcher_thread(messageCatcher, accepted, false, crm);
	messageCatcher_thread.detach();
}

void acceptorOfNewConnections(int listener) {
	sockaddr* addr;
	int* addrLen = NULL;
	int accepted = accept_net(listener);
	std::cout << "[D" << accepted << "] " << "Client connected! \n";
	// Forward this client to message catcher
	cryptoMethods crm;
	std::thread messageHandler_thread(messageCatcher, accepted, true, crm);
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