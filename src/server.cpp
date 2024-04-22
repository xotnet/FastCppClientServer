#include "misc.hpp"
#include <iostream>
#include <thread>
#include "BigInt.hpp"

struct cryptoMethods {
	BigInt sharedKey = std::string("0");
	BigInt genSharedKey(int accepted) {
		try {
			std::string ip = getPeerIp_net(accepted);
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
			if (send_net(accepted, resChar, sizeof(resChar)) == -1) {std::cout << "[" << ip << "] " << "Client disconnected\n"; return 0;}
			if (recv_net(accepted, friChar, sizeof(friChar)) == -1) {std::cout << "[" << ip << "] " << "Client disconnected\n"; return 0;}
			BigInt friendsKey(0);
			// check crypt "protocol" of friChar
			std::string friString = "";
			if (sizeof(friChar) < 10) {return 0;}
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
		} catch (...) {return 0;}
	}
};

bool BotFilter(int accepted) {
	std::string ip = getPeerIp_net(accepted);
        // Checking
    std::string command = "print                               [BOTFILTER]\n\nWe are checking your connection for legit...\n";
    if (send_net(accepted, &command[0], command.length()) == -1) {std::cout << "[" << ip << "] Disconnected\n"; return false;}
    sleep(3);
        // If legit
    /*command = "print You passed BOTCHECK\n\n\n";
    if (send_net(accepted, &command[0], command.length()) == -1) {std::cout << "[" << ip << "] Disconnected\n"; return false;}*/
    command = "psdbf";
    if (send_net(accepted, &command[0], command.length()) == -1) {std::cout << "[" << ip << "] Disconnected\n"; return false;}
    return true;
}

void messageHandler(int accepted, char* buffer, std::string key) {
	// server response any message
	std::string clientMessage = crySynMethod(std::string(buffer), key);
	std::cout << "[" << getPeerIp_net(accepted) << "] " << clientMessage << '\n';
	if (clientMessage == "help") {
		char buf[] = "help - get command list\nversion - get server time\nencryption - get server encryption protocol\n";
		sendAndEncryptMessage(accepted, buf, key);
	} else if (clientMessage == "version") {
		char buf[] = "Server version: 1.9 (With fast encryption and BotFilter)\n";
		sendAndEncryptMessage(accepted, buf, key);
	} else if (clientMessage == "encryption") {
		char buf[] = "Server use fast server xor encryption behind Diffie Hellman key exchange. This is 1.2 version of server encryption protocol\n";
		sendAndEncryptMessage(accepted, buf, key);
	} else {
		char buf[] = "Unknown command\n";
		sendAndEncryptMessage(accepted, buf, key);
	}
}

void messageCatcher(int accepted, bool firstConnect, cryptoMethods crm, bool makeBotCheck) {
	std::string ip = getPeerIp_net(accepted);
    //BotFilterServer
	bool isLegit = 1;
    if (makeBotCheck) {isLegit = BotFilter(accepted);}
    if (!isLegit) {
        close_net(accepted);
		return;
	}
	// Encryption
	if (crm.sharedKey == BigInt(0)) {
		crm.sharedKey = crm.genSharedKey(accepted);
		if (crm.sharedKey == BigInt(0)) {return;}
	}
	
	if (firstConnect) { // welcome message
		char welcomeMessage[100] = "Welcome from server! Use help for get command list\n";
		sendAndEncryptMessage(accepted, welcomeMessage, crm.sharedKey.to_string());
	}
	char buffer[1024] = "";
	if (int messageBytes = recv_net(accepted, buffer, sizeof(buffer)) == -1) {return;}
	if (buffer[0] == 0) { // if client exit
		std::cout << "[" << ip << "] " << "Client disconnected\n";
		close_net(accepted);
		return;
	}
	// if client online
	messageHandler(accepted, buffer, crm.sharedKey.to_string());
	std::thread messageCatcher_thread(messageCatcher, accepted, false, crm, false);
	messageCatcher_thread.detach();
}

void acceptorOfNewConnections(int listener) {
	int accepted = accept_net(listener);
	std::cout << "[" << getPeerIp_net(accepted) << "] " << "Client connected! \n";
	// Forward this client to message catcher
	cryptoMethods crm;
	std::thread messageHandler_thread(messageCatcher, accepted, true, crm, true);
	messageHandler_thread.detach();
	// Now continue listen new connections
	std::thread accept_thread(acceptorOfNewConnections, listener);
	accept_thread.detach();
}

int main() {
	const char serverPort[] = "1597";
	std::cout << "Server started on port " << serverPort << '\n';
	int listener = listen_net("0.0.0.0", serverPort); // start server on port 1597
	std::thread accept_thread(acceptorOfNewConnections, listener); // start root thread
	accept_thread.detach();
	timer();
}
